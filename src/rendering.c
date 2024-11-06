#include "rendering.h"

#include <stb_image/stb_image.h>
#include <immintrin.h>

#include "player.h"

static RenderState render_state;

#define SET_PIXEL(x, y, color) if(x >= 0 && y >= 0 && x < SCREEN_WIDTH && y < SCREEN_HEIGHT) render_state.pixels[((y) * SCREEN_WIDTH) + x] = color;

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
        SDL_clamp(ndc.z * DEPTH_PRECISION, 0, DEPTH_PRECISION)
    };
}

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
    render_state.depth_test = true;

    return &render_state;
}

void cleanup_rendering() {
    SDL_DestroyTexture(render_state.texture);
    SDL_DestroyRenderer(render_state.renderer);
}

void set_clear_color(u8 r, u8 g, u8 b, u8 a) {
    if(SDL_SetRenderDrawColor(render_state.renderer, r, g, b, a) != 0) {
        fprintf(stderr, "Failed to set clear color %x %x %x %x\n", r, g, b, a); 
    }
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
    if(render_state.depth_test) {
        memset(render_state.depth_buffer, 0, sizeof(render_state.depth_buffer));
    }
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
        bool draw[3] = { false, false, false };

        for(u32 j = 0; j < 3; j++) {
            vec4s v =
                glms_mat4_mulv(
                    m,
                    (vec4s) {
                        local_vertices[j].pos.x,
                        local_vertices[j].pos.y,
                        local_vertices[j].pos.z,
                        local_vertices[j].w});
            
            local_vertices[j].pos.x = v.w == 0.0f ? 0.0f : v.x / v.w;
            local_vertices[j].pos.y = v.w == 0.0f ? 0.0f : v.y / v.w;
            local_vertices[j].pos.z = v.w == 0.0f ? 0.0f : v.z / v.w;
            local_vertices[j].w = v.w;

            // Hack to stop segfaults - NEEDS PROPER FIXING
            if(local_vertices[j].w > 0.0f) {
                draw[j] = true;
            }
        }
        if(draw[0] && draw[1] && draw[2]) {
            sort_cw(local_vertices);
            draw_triangle(local_vertices, texture);
        }
    }
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

    draw_triangle_raw(
        raw_vertices,
        texture);
}

void draw_triangle_raw(RawVertex *vertices, const Texture *texture) {
    i32 x1 = vertices[0].pos.x;
    i32 x2 = vertices[1].pos.x;
    i32 x3 = vertices[2].pos.x;
    i32 y1 = vertices[0].pos.y;
    i32 y2 = vertices[1].pos.y;
    i32 y3 = vertices[2].pos.y;
    i32 z1 = vertices[0].pos.z;
    i32 z2 = vertices[1].pos.z;
    i32 z3 = vertices[2].pos.z;

    vec2s uv1 = vertices[0].uv;
    vec2s uv2 = vertices[1].uv;
    vec2s uv3 = vertices[2].uv;

    i32 min_x = min(x1, x2, x3);
    i32 min_y = min(y1, y2, y3);

    i32 max_x = max(x1, x2, x3);
    i32 max_y = max(y1, y2, y3);

    i32 area = abs(x1 * (y2 - y3) + x2 * (y3 - y1) + x3 * (y1 - y2)) >> 1;
    if(area < 1) {
        area = 1;
    }
    f32 inverse_area = 1.0f / (f32) area;

    min_x = SDL_clamp(min_x, 0, SCREEN_WIDTH);
    max_x = SDL_clamp(max_x, 0, SCREEN_WIDTH);
    min_y = SDL_clamp(min_y, 0, SCREEN_HEIGHT);
    max_y = SDL_clamp(max_y, 0, SCREEN_HEIGHT);
    z1 = SDL_clamp(z1, 0, DEPTH_PRECISION);
    z2 = SDL_clamp(z2, 0, DEPTH_PRECISION);
    z3 = SDL_clamp(z3, 0, DEPTH_PRECISION);

    i32 e_row1, e_row2, e_row3;
    e_row1 = edge_function((ivec2s){x1, y1}, (ivec2s){x3, y3}, (ivec2s){min_x, min_y});
    e_row2 = edge_function((ivec2s){x3, y3}, (ivec2s){x2, y2}, (ivec2s){min_x, min_y});
    e_row3 = edge_function((ivec2s){x2, y2}, (ivec2s){x1, y1}, (ivec2s){min_x, min_y});

    i32 de1_row = x1 - x3;
    i32 de2_row = x3 - x2;
    i32 de3_row = x2 - x1;
    i32 de1 = y3 - y1;
    i32 de2 = y2 - y3;
    i32 de3 = y1 - y2;

    f32 z1f = (f32) z1 / (f32) DEPTH_PRECISION;
    f32 z2f = (f32) z2 / (f32) DEPTH_PRECISION;
    f32 z3f = (f32) z3 / (f32) DEPTH_PRECISION;
    z1f *= vertices[0].w;
    z2f *= vertices[1].w;
    z3f *= vertices[2].w;
    z1f = 1.0f / z1f;
    z2f = 1.0f / z2f;
    z3f = 1.0f / z3f;

    vec2s tex_coords;

    vec3s raw_bc_row;
    raw_bc_row.x = (f32) e_row2 * inverse_area * z1f / 2.0f;
    raw_bc_row.y = (f32) e_row3 * inverse_area * z3f / 2.0f;
    raw_bc_row.z = (f32) e_row1 * inverse_area * z2f / 2.0f;
    f32 du_row = (f32)((x3 - x2)) * inverse_area * z1f / 2.0f;
    f32 dv_row = (f32)((x2 - x1)) * inverse_area * z3f / 2.0f;
    f32 dw_row = (f32)((x1 - x3)) * inverse_area * z2f / 2.0f;
    f32 du = (f32)((y2 - y3)) * inverse_area * z1f / 2.0f;
    f32 dv = (f32)((y1 - y2)) * inverse_area * z3f / 2.0f;
    f32 dw = (f32)((y3 - y1)) * inverse_area * z2f / 2.0f;

    f32 brightness = vertices[0].brightness;

    for(i32 y = min_y; y < max_y; y++) {
        i32 e1 = e_row1;
        i32 e2 = e_row2;
        i32 e3 = e_row3;

        vec3s raw_bc = raw_bc_row;

        for(i32 x = min_x; x < max_x; x++) {
            if((e1 | e2 | e3) >= 0) {
                vec3s bc = raw_bc;
                f32 inverse_sum = 1.0f / (bc.x + bc.y + bc.z);
                bc.x *= inverse_sum;
                bc.y *= inverse_sum;
                bc.z *= inverse_sum;

                i32 depth =
                    bc.x * z1
                    + bc.y * z3
                    + bc.z * z2;

                // Don't do per-pixel calculations if the pixel isn't visible!
                if(render_state.depth_test) {
                    i32 d = render_state.depth_buffer[y * SCREEN_WIDTH + x];
                    if(d <= 0 || depth <= d) {
                        tex_coords.x = (bc.x * uv1.x + bc.y * uv3.x + bc.z * uv2.x);
                        tex_coords.y = (bc.x * uv1.y + bc.y * uv3.y + bc.z * uv2.y);

                        u32 index =
                            (u32)((tex_coords.x * (texture->width - 1)))
                            + (u32)(tex_coords.y * (texture->height - 1)) * texture->width;

                        index = SDL_clamp(index, 0, texture->width * texture->height - 1);
                        u32 color = ((u32*) texture->data)[index];
                        // Don't draw if alpha value is 0
                        if(color & 0xFF000000) {
                            f32 c1 = ((u8*) &color)[2];
                            f32 c2 = ((u8*) &color)[1];
                            f32 c3 = ((u8*) &color)[0];
                            c1 *= brightness;
                            c2 *= brightness;
                            c3 *= brightness;
                            ((u8*) &color)[2] = (u8) c1;
                            ((u8*) &color)[1] = (u8) c2;
                            ((u8*) &color)[0] = (u8) c3;
                        
                            render_state.depth_buffer[y * SCREEN_WIDTH + x] = depth;
                            SET_PIXEL(x, y, color);
                        }
                    }
                } else {
                    tex_coords.x = (bc.x * uv1.x + bc.y * uv3.x + bc.z * uv2.x);
                    tex_coords.y = (bc.x * uv1.y + bc.y * uv3.y + bc.z * uv2.y);

                    u32 index =
                        (u32)((tex_coords.x * (texture->width - 1)))
                        + (u32)(tex_coords.y * (texture->height - 1)) * texture->width;

                    index = SDL_clamp(index, 0, texture->width * texture->height - 1);
                    u32 color = ((u32*) texture->data)[index];
                    // Don't draw if alpha value is 0
                    if(color & 0xFF000000) {
                        f32 c1 = ((u8*) &color)[2];
                        f32 c2 = ((u8*) &color)[1];
                        f32 c3 = ((u8*) &color)[0];
                        c1 *= brightness;
                        c2 *= brightness;
                        c3 *= brightness;
                        ((u8*) &color)[2] = (u8) c1;
                        ((u8*) &color)[1] = (u8) c2;
                        ((u8*) &color)[0] = (u8) c3;
                    
                        SET_PIXEL(x, y, color);
                    }
                }
            }

            e1 += de1;
            e2 += de2;
            e3 += de3;
            raw_bc.x += du;
            raw_bc.y += dv;
            raw_bc.z += dw;
        }

        e_row1 += de1_row;
        e_row2 += de2_row;
        e_row3 += de3_row;
        raw_bc_row.x += du_row;
        raw_bc_row.y += dv_row;
        raw_bc_row.z += dw_row;
    }
}