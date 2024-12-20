#include "rendering.h"

#include <stb_image/stb_image.h>
#include "thread_pool.h"

#include "player.h"

#include <xmmintrin.h>

static RenderState render_state;

// Number of threads to use for rendering, therefore number of sections the screen is split into
#define RENDER_THREAD_COUNT 16

RenderSection render_sections[RENDER_THREAD_COUNT];
ThreadPool thread_pool;

#define SET_PIXEL(x, y, color) \
        render_state.pixels[((y) * SCREEN_WIDTH) + (x)] = (color);

static i32 max(i32 a, i32 b, i32 c) {
    if(a >= b && a >= c) {
        return a;
    }

    if(b >= a && b >= c) {
        return b;
    }

    return c;
}

static i32 min(i32 a, i32 b, i32 c) {
    if(a <= b && a <= c) {
        return a;
    }

    if(b <= a && b <= c) {
        return b;
    }

    return c;
}

static ivec3s ndc_to_pixels(vec3s ndc) {
    return (ivec3s) {
        ((ndc.x + 1.0f) / 2.0f) * (SCREEN_WIDTH),
        (1.0f - ((ndc.y + 1.0f) / 2.0f)) * (SCREEN_HEIGHT),
        ndc.z * DEPTH_PRECISION
    };
}

// Returns true if the vertices had to be sorted, false otherwise
static bool sort_cw(Vertex *vertices) {
    if(vertices[0].pos.x == vertices[1].pos.x
        && vertices[1].pos.x == vertices[2].pos.x) {
        return false;
    }

    if(vertices[0].pos.y == vertices[1].pos.y
        && vertices[1].pos.y == vertices[2].pos.y) {
        return false;
    }

    // Checks if vertices 1 and 2 are clockwise around vertex 0
    f32 a =
        (vertices[0].pos.x - vertices[1].pos.x) * (vertices[2].pos.y - vertices[1].pos.y)
            - (vertices[0].pos.y - vertices[1].pos.y) * (vertices[2].pos.x - vertices[1].pos.x);

    if(a < 0.0f) {
        Vertex t = vertices[1];
        vertices[1] = vertices[2];
        vertices[2] = t;
        return true;
    }
    return false;
}

RenderState *init_rendering(Window *window) {
    render_state.renderer = SDL_CreateRenderer(window->handle, NULL, SDL_RENDERER_PRESENTVSYNC);

    if(!render_state.renderer) {
        printf("Failed to create renderer %s\n", SDL_GetError());
        return NULL;
    }

    render_state.texture = SDL_CreateTexture(
        render_state.renderer,
        SDL_PIXELFORMAT_ABGR8888,
        SDL_TEXTUREACCESS_STREAMING,
        SCREEN_WIDTH,
        SCREEN_HEIGHT);
    SDL_SetTextureScaleMode(render_state.texture, SDL_SCALEMODE_NEAREST);

    memset(render_state.depth_buffer, 0, sizeof(render_state.depth_buffer));

    u32 section_width = floorf((f32) SCREEN_WIDTH / RENDER_THREAD_COUNT);
    u32 x = 0;
    for(u32 i = 0; i < RENDER_THREAD_COUNT; i++) {
        RenderSection section = { 0 };
        if(i < RENDER_THREAD_COUNT - 1) {
            section.bounds = (ivec4s) {
                x,
                0,
                x + section_width - 1,
                SCREEN_HEIGHT - 1
            };
        } else {
            section.bounds = (ivec4s) {
                x,
                0,
                SCREEN_WIDTH - 1,
                SCREEN_HEIGHT - 1
            };
        }
        x += section_width;

        render_sections[i] = section;
    }

    init_thread_pool(&thread_pool, RENDER_THREAD_COUNT);

    return &render_state;
}

void cleanup_rendering() {
    thread_pool.active = false;
    destroy_thread_pool(&thread_pool);

    SDL_DestroyTexture(render_state.texture);
    SDL_DestroyRenderer(render_state.renderer);
}

void set_clear_color(u8 r, u8 g, u8 b, u8 a) {
    if(SDL_SetRenderDrawColor(render_state.renderer, r, g, b, a) != 0) {
        fprintf(stderr, "Failed to set clear color %x %x %x %x\n", r, g, b, a); 
    }
}

static void draw_render_section(RenderSection *section) {
    for(u32 i = 0; i < section->triangle_list.count; i++) {
        draw_triangle_raw(&section->triangle_list.parts[i], section->bounds, section->triangle_list.parts[i].texture);
    }

    section->triangle_list.count = 0;
}

static void draw_render_section_thread_func(void *arg) {
    RenderSection *section = arg;
    draw_render_section(section);
}

void present() {
    void *px;
    i32 pitch;
    SDL_LockTexture(render_state.texture, NULL, &px, &pitch);
    memcpy(px, render_state.pixels, sizeof(render_state.pixels));
    SDL_UnlockTexture(render_state.texture);

    SDL_SetRenderTarget(render_state.renderer, NULL);
    SDL_SetRenderDrawColor(render_state.renderer, 0, 0, 0, 0xFF);
    SDL_SetRenderDrawBlendMode(render_state.renderer, SDL_BLENDMODE_NONE);

    memset32(render_state.pixels, 0xFFFFAE00, sizeof(render_state.pixels));
    memset(render_state.depth_buffer, 0, sizeof(render_state.depth_buffer));

    SDL_RenderClear(render_state.renderer);

    SDL_RenderTexture(render_state.renderer, render_state.texture, NULL, NULL);
    SDL_RenderPresent(render_state.renderer);
}

Texture load_texture(const char *path) {
    stbi_set_flip_vertically_on_load(true);

    u8 *data;
    i32 width, height, channels;

    data = stbi_load(path, &width, &height, &channels, 4);

    if(!data) {
        fprintf(stderr, "Failed to load image from path %s\n", path);
    }

    Texture texture;
    texture.width = width;
    texture.height = height;
    texture.data = data;
    texture.pixel_size = (vec2s) {
        1.0f / texture.width,
        1.0f / texture.height
    };
    return texture;
}

void destroy_texture(Texture *texture) {
    stbi_image_free(texture->data);
}

void draw_line(vec3s v1, vec3s v2, u32 color) {
    ivec3s v1_pixels = ndc_to_pixels(v1);
    ivec3s v2_pixels = ndc_to_pixels(v2);

    draw_line_raw(
        v1_pixels.x,
        v1_pixels.y,
        v2_pixels.x,
        v2_pixels.y,
        color);
}

void draw_line_raw(u32 x1, u32 y1, u32 x2, u32 y2, u32 color) {
    u32 dx = x1 > x2 ? x1 - x2 : x2 - x1;
    u32 dy = y1 > y2 ? y1 - y2 : y2 - y1;
    bool steep = dy > dx;
    
    if(steep) {
        u32 t = x1;
        x1 = y1;
        y1 = t;
        t = x2;
        x2 = y2;
        y2 = t;
    }

    if(x1 > x2) {
        u32 t = x1;
        x1 = x2;
        x2 = t;
        t = y1;
        y1 = y2;
        y2 = t;
    }
    
    u32 y = y1;

    u32 decision_factor = 0;
    for(u32 x = x1; x < x2; x++) {
        if(steep) {
            SET_PIXEL(y, x, color);
            decision_factor += dx;
            if(decision_factor >= dy) {
                if(y2 > y1) {
                    y++;
                } else {
                    y--;
                }
                decision_factor -= dy;
            }
        } else {
            SET_PIXEL(x, y, color);
            decision_factor += dy;
            if(decision_factor >= dx) {
                if(y2 > y1) {
                    y++;
                } else {
                    y--;
                }
                decision_factor -= dx;
            }
        }
    }
}

static i32 edge_function(ivec2s a, ivec2s b, ivec2s c) {
    return ((c.x - a.x) * (b.y - a.y) - (c.y - a.y) * (b.x - a.x));
}

static bool is_vertex_visible(const Vertex *vertex) {
    return fabsf(vertex->pos.z) <= vertex->w;
}

// Returns true if FULLY visible
static bool is_triangle_visible(Vertex *vertices) {
    bool visible[3] = {false};

    for(u32 i = 0; i < 3; i++) {
        Vertex *vertex = &vertices[i];
        if(is_vertex_visible(vertex)) {
            visible[i] = true;
        }
    }

    return visible[0] && visible[1] && visible[2];
}

static void clip_triangle(Vertex *vertices) {
    // TODO: actually make this a real algorithm

    for(u32 i = 0; i < 3; i++) {
        Vertex *vertex = &vertices[i];
        vertex->pos.z = SDL_clamp(vertex->pos.z, 0.0f, fabsf(vertex->w));
    }
}

void draw_triangles(
    u32 count,
    const Vertex *vertices,
    const Texture *texture,
    mat4s proj,
    mat4s view,
    mat4s model) {
    Vertex local_vertices[3];
    mat4s m = glms_mat4_mul(view, model);
    m = glms_mat4_mul(proj, m);

    for(u32 i = 0; i < count; i++) {
        memcpy(local_vertices, &vertices[i * 3], sizeof(local_vertices));
        // Hack to save on performance
        if(local_vertices[0].pos.y <= 0.0f
            && local_vertices[1].pos.y <= 0.0f
            && local_vertices[2].pos.y <= 0.0f
            && player.camera.pos.y >= 0.0f) {
            continue;
        }

        for(u32 j = 0; j < 3; j++) {
            vec4s v =
                glms_mat4_mulv(
                    m,
                    (vec4s) {
                        local_vertices[j].pos.x,
                        local_vertices[j].pos.y,
                        local_vertices[j].pos.z,
                        local_vertices[j].w});

            local_vertices[j].pos.x = v.x;
            local_vertices[j].pos.y = v.y;
            local_vertices[j].pos.z = v.z;
            local_vertices[j].w = v.w;
        }

        if(is_triangle_visible(local_vertices)) {
            clip_triangle(local_vertices);

            for(u32 j = 0; j < 3; j++) {
                vec4s v = (vec4s) {
                    local_vertices[j].pos.x,
                    local_vertices[j].pos.y,
                    local_vertices[j].pos.z,
                    local_vertices[j].w};
                local_vertices[j].pos.x = v.w == 0.0f ? 0.0f : v.x / v.w;
                local_vertices[j].pos.y = v.w == 0.0f ? 0.0f : v.y / v.w;
                local_vertices[j].pos.z = v.w == 0.0f ? 0.0f : v.z / v.w;
            }

            sort_cw(local_vertices);
            draw_triangle(local_vertices, texture);
        }
    }
}

static RenderSection *get_render_section(ivec2s coords) {
    for(u32 i = 0; i < RENDER_THREAD_COUNT; i++) {
        ivec4s bounds = render_sections[i].bounds;
        if(coords.x >= bounds.x && coords.x <= bounds.z
            && coords.y >= bounds.y && coords.y <= bounds.w) {
            return &render_sections[i];
        }
    }
    return NULL;
}

static void push_triangle_to_render_section(RenderSection *section, TrianglePart *triangle) {
    if(!section->triangle_list.parts) {
        section->triangle_list.parts = malloc(1024 * sizeof(TrianglePart));
        section->triangle_list.allocated_size = 1024;
    }

    if(section->triangle_list.count >= section->triangle_list.allocated_size) {
        section->triangle_list.parts =
            realloc(
                section->triangle_list.parts,
                2 * section->triangle_list.allocated_size * sizeof(TrianglePart));
        section->triangle_list.allocated_size *= 2;
    }

    section->triangle_list.parts[section->triangle_list.count] = *triangle;
    section->triangle_list.count++;
}

void draw_triangle(const Vertex *vertices, const Texture *texture) {
    RawVertex raw_vertices[3];

    raw_vertices[0] = (RawVertex) {
        .pos = ndc_to_pixels(vertices[0].pos),
        .w = vertices[0].w,
        .uv = vertices[0].uv,
        .brightness = vertices[0].brightness
    };
    raw_vertices[1] = (RawVertex) {
        .pos = ndc_to_pixels(vertices[1].pos),
        .w = vertices[1].w,
        .uv = vertices[1].uv,
        .brightness = vertices[0].brightness
    };
    raw_vertices[2] = (RawVertex) {
        .pos = ndc_to_pixels(vertices[2].pos),
        .w = vertices[2].w,
        .uv = vertices[2].uv,
        .brightness = vertices[0].brightness
    };

    const i32 x1 = raw_vertices[0].pos.x;
    const i32 x2 = raw_vertices[1].pos.x;
    const i32 x3 = raw_vertices[2].pos.x;
    const i32 y1 = raw_vertices[0].pos.y;
    const i32 y2 = raw_vertices[1].pos.y;
    const i32 y3 = raw_vertices[2].pos.y;

    i32 min_x = min(x1, x2, x3);
    i32 min_y = min(y1, y2, y3);
    i32 max_x = max(x1, x2, x3);
    i32 max_y = max(y1, y2, y3);
    min_x = SDL_clamp(min_x, 0, SCREEN_WIDTH - 1);
    max_x = SDL_clamp(max_x, 0, SCREEN_WIDTH - 1);
    min_y = SDL_clamp(min_y, 0, SCREEN_HEIGHT - 1);
    max_y = SDL_clamp(max_y, 0, SCREEN_HEIGHT - 1);
    i32 size_x = max_x - min_x;
    i32 size_y = max_y - min_y;

    if(size_x < 1 || size_y < 1) {
        return;
    }

    RenderSection *left_render_section = get_render_section((ivec2s) {min_x, 0});
    RenderSection *right_render_section = get_render_section((ivec2s) {max_x, 0});

    // Triangle must be outside of screen for some reason, this shouldn't happen though
    if(!left_render_section || !right_render_section) {
        return;
    }

    if(left_render_section == right_render_section) {
        // Triangle is in the same section
        TrianglePart triangle_part;
        memcpy(triangle_part.vertices, raw_vertices, sizeof(raw_vertices));
        triangle_part.min_x = min_x;
        triangle_part.max_x = max_x;
        triangle_part.min_y = min_y;
        triangle_part.max_y = max_y;
        triangle_part.texture = texture;
        push_triangle_to_render_section(left_render_section, &triangle_part);
    } else {
        // Triangle is split across multiple sections
        for(u32 i = 0; i < RENDER_THREAD_COUNT; i++) {
            RenderSection *section = &render_sections[i];
            bool is_in_section = max_x >= section->bounds.x && min_x <= section->bounds.z;
            if(is_in_section) {
                TrianglePart triangle_part;
                memcpy(triangle_part.vertices, raw_vertices, sizeof(raw_vertices));
                triangle_part.min_x = section->bounds.x;
                triangle_part.max_x = max_x >= section->bounds.z ? section->bounds.z : max_x;
                triangle_part.min_y = min_y;
                triangle_part.max_y = max_y;
                triangle_part.texture = texture;
                
                push_triangle_to_render_section(section, &triangle_part);
            }
        }
    }
}

void draw_triangle_raw(const TrianglePart *part, ivec4s section_bounds, const Texture *texture) {
    const i32 x1 = part->vertices[0].pos.x;
    const i32 x2 = part->vertices[1].pos.x;
    const i32 x3 = part->vertices[2].pos.x;
    const i32 y1 = part->vertices[0].pos.y;
    const i32 y2 = part->vertices[1].pos.y;
    const i32 y3 = part->vertices[2].pos.y;
    i32 z1 = part->vertices[0].pos.z;
    i32 z2 = part->vertices[1].pos.z;
    i32 z3 = part->vertices[2].pos.z;

    const vec2s uv1 = part->vertices[0].uv;
    const vec2s uv2 = part->vertices[1].uv;
    const vec2s uv3 = part->vertices[2].uv;
    const f32 min_uvx = SDL_min(uv1.x, SDL_min(uv2.x, uv3.x));
    const f32 min_uvy = SDL_min(uv1.y, SDL_min(uv2.y, uv3.y));
    const f32 max_uvx = SDL_max(uv1.x, SDL_max(uv2.x, uv3.x)) - texture->pixel_size.x;
    const f32 max_uvy = SDL_max(uv1.y, SDL_max(uv2.y, uv3.y)) - texture->pixel_size.y;

    const __m128 v_uv_x = _mm_setr_ps(uv1.x, uv3.x, uv2.x, 0.0f);
    const __m128 v_uv_y = _mm_setr_ps(uv1.y, uv3.y, uv2.y, 0.0f);

    i32 min_x = part->min_x;
    i32 min_y = part->min_y;
    i32 max_x = part->max_x;
    i32 max_y = part->max_y;

    i32 area = abs(x1 * (y2 - y3) + x2 * (y3 - y1) + x3 * (y1 - y2)) >> 1;
    if(area < 1) {
        area = 1;
    }
    f32 inverse_area = 1.0f / (f32) area;

    i32 section_min_x = section_bounds.x;
    i32 section_min_y = section_bounds.y;
    i32 section_max_x = section_bounds.z;
    i32 section_max_y = section_bounds.w;

    min_x = SDL_clamp(min_x, section_min_x, section_max_x);
    max_x = SDL_clamp(max_x, section_min_x, section_max_x);
    min_y = SDL_clamp(min_y, section_min_y, section_max_y);
    max_y = SDL_clamp(max_y, section_min_y, section_max_y);
    z1 = SDL_clamp(z1, 0, DEPTH_PRECISION);
    z2 = SDL_clamp(z2, 0, DEPTH_PRECISION);
    z3 = SDL_clamp(z3, 0, DEPTH_PRECISION);

    __m128 v_z = _mm_setr_ps(z1, z3, z2, 0.0f);

    i32 e_row1, e_row2, e_row3;
    e_row1 = edge_function((ivec2s){x1, y1}, (ivec2s){x3, y3}, (ivec2s){min_x, min_y});
    e_row2 = edge_function((ivec2s){x3, y3}, (ivec2s){x2, y2}, (ivec2s){min_x, min_y});
    e_row3 = edge_function((ivec2s){x2, y2}, (ivec2s){x1, y1}, (ivec2s){min_x, min_y});

    // Edge function increments
    i32 de1_row = (x1 - x3);
    i32 de2_row = (x3 - x2);
    i32 de3_row = (x2 - x1);
    i32 de1 = (y3 - y1);
    i32 de2 = (y2 - y3);
    i32 de3 = (y1 - y2);

    // Inverse homogenous coordinates
    f32 inv_w1 = 1.0f / part->vertices[0].w;
    f32 inv_w2 = 1.0f / part->vertices[1].w;
    f32 inv_w3 = 1.0f / part->vertices[2].w;

    vec2s tex_coords;

    vec3s raw_bc_row;
    raw_bc_row.x = (f32) e_row2 * inverse_area * inv_w1 / 2.0f;
    raw_bc_row.y = (f32) e_row3 * inverse_area * inv_w3 / 2.0f;
    raw_bc_row.z = (f32) e_row1 * inverse_area * inv_w2 / 2.0f;
    // Barycentric coordinate increments
    const f32 du_row = (f32)((x3 - x2)) * inverse_area * inv_w1 / 2.0f;
    const f32 dv_row = (f32)((x2 - x1)) * inverse_area * inv_w3 / 2.0f;
    const f32 dw_row = (f32)((x1 - x3)) * inverse_area * inv_w2 / 2.0f;
    const f32 du = (f32)((y2 - y3)) * inverse_area * inv_w1 / 2.0f;
    const f32 dv = (f32)((y1 - y2)) * inverse_area * inv_w3 / 2.0f;
    const f32 dw = (f32)((y3 - y1)) * inverse_area * inv_w2 / 2.0f;

    __m128 v_raw_bc_row = _mm_setr_ps(raw_bc_row.x, raw_bc_row.y, raw_bc_row.z, 0.0f);
    __m128 v_dbc_row = _mm_setr_ps(du_row, dv_row, dw_row, 0.0f);
    __m128 v_dbc = _mm_setr_ps(du, dv, dw, 0.0f);

    // Since we are not interpolating brightness we can just take it from the first vertex
    const f32 brightness = part->vertices[0].brightness;
    const u32 fp_brightness = (1 << 10) * brightness;

    for(i32 y = min_y; y <= max_y; y++) {
        i32 e1 = e_row1;
        i32 e2 = e_row2;
        i32 e3 = e_row3;

        __m128 v_raw_bc = v_raw_bc_row;
        i32 *depth_row = &render_state.depth_buffer[y * SCREEN_WIDTH];
        for(i32 x = min_x; x <= max_x; x++) {
            if((e1 | e2 | e3) >= 0) {
                __m128 v_bc = v_raw_bc;
                f32 sum = hsum_ps_sse3(v_bc);
                const __m128 v_inverse_sum = _mm_rcp_ps(_mm_set1_ps(sum));
                v_bc = _mm_mul_ps(v_bc, v_inverse_sum);

                const __m128 v_depth = _mm_mul_ps(v_bc, v_z);
                i32 depth = hsum_ps_sse3(v_depth);

                // Don't do per-pixel calculations if the pixel isn't visible!
                const i32 d = depth_row[x];
                if((depth > 0 && depth <= DEPTH_PRECISION) && (d == 0 || depth <= d)) {
                    tex_coords.x = hsum_ps_sse3(_mm_mul_ps(v_bc, v_uv_x));
                    tex_coords.y = hsum_ps_sse3(_mm_mul_ps(v_bc, v_uv_y));

                    // Floating point precision is a pain in the ass
                    tex_coords.x = SDL_clamp(tex_coords.x, min_uvx, max_uvx);
                    tex_coords.y = SDL_clamp(tex_coords.y, min_uvy, max_uvy);

                    u32 index_width = (tex_coords.x * (texture->width));
                    index_width = SDL_clamp(index_width, 0, texture->width);
                    u32 index_height = tex_coords.y * texture->height;
                    index_height = SDL_clamp(index_height, 0, texture->height);
                    index_height *= texture->width;

                    const u32 index =
                        SDL_clamp(
                            index_width + index_height,
                            0,
                            texture->width * texture->height);
                    u32 color = ((u32*) texture->data)[index];
                    
                    // Don't draw if alpha value is 0
                    if(color & 0xFF000000) {
                        u8 c1 = ((u8*) &color)[2];
                        u8 c2 = ((u8*) &color)[1];
                        u8 c3 = ((u8*) &color)[0];
                        c1 = (c1 * fp_brightness) >> 10;
                        c2 = (c2 * fp_brightness) >> 10;
                        c3 = (c3 * fp_brightness) >> 10;
                        ((u8*) &color)[2] = c1;
                        ((u8*) &color)[1] = c2;
                        ((u8*) &color)[0] = c3;
                    
                        depth_row[x] = depth;
                        SET_PIXEL(x, y, color);
                    }
                }
            }

            e1 += de1;
            e2 += de2;
            e3 += de3;
            v_raw_bc = _mm_add_ps(v_raw_bc, v_dbc);
        }

        e_row1 += de1_row;
        e_row2 += de2_row;
        e_row3 += de3_row;
        v_raw_bc_row = _mm_add_ps(v_raw_bc_row, v_dbc_row);
    }
}

void draw_screen() {
    pthread_mutex_lock(&thread_pool.mutex);
    for(u32 i = 0; i < RENDER_THREAD_COUNT; i++) {
        push_task(&thread_pool, draw_render_section_thread_func, &render_sections[i]);
    }
    pthread_mutex_unlock(&thread_pool.mutex);
}

void render_wait() {
    thread_pool_wait(&thread_pool);
}
