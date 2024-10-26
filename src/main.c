#include <stdio.h>
#include <SDL3/SDL.h>
#include <stb_image/stb_image.h>
#include <stdbool.h>
#include <stdint.h>
#include <math.h>
#include <time.h>
#include <limits.h>
#include <cglm/struct.h>
#include <cglm/struct/cam.h>

#define SCREEN_WIDTH 427
#define SCREEN_HEIGHT 240

#define SET_PIXEL(x, y, color) if(x >= 0 && y >= 0 && x < SCREEN_WIDTH && y < SCREEN_HEIGHT) state.pixels[y * SCREEN_WIDTH + x] = color;

#define NS_PER_SECOND 1000000000

static long ns_now() {
	struct timespec ts;
	timespec_get(&ts, TIME_UTC);
	return ((ts.tv_sec * NS_PER_SECOND) + ts.tv_nsec);
}

typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef float f32;
typedef double f64;

#define DEPTH_PRECISION (1 << 12)

struct {
    SDL_Window *window;
    SDL_Surface *surface;
    SDL_Renderer *renderer;
    SDL_Texture *texture;
    u32 pixels[SCREEN_WIDTH * SCREEN_HEIGHT];
    i32 depth_buffer[SCREEN_WIDTH * SCREEN_HEIGHT];
    bool depth_test;
    bool quit;
} state;

typedef struct {
    vec3s pos;
    f32 w; // For perspective correct interpolation
    vec2s uv;
} Vertex;

typedef struct {
    ivec3s pos;
    f32 w; // For perspective correct interpolation
    vec2s uv;
} RawVertex;

typedef enum {
    FORMAT_RGB,
    FORMAT_RGBA
} TextureFormat;

typedef struct {
    u8 *data;
    TextureFormat format;
    u32 width;
    u32 height;
} Texture;

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

static void sort_cw(Vertex *vertices) {
    if(vertices[0].pos.x == vertices[1].pos.x
        && vertices[1].pos.x == vertices[2].pos.x) {
        return;
    }

    if(vertices[0].pos.y == vertices[1].pos.y
        && vertices[1].pos.y == vertices[2].pos.y) {
        return;
    }

    // Checks if vertices 1 and 2 are clockwise around vertex 0
    f32 a =
        (vertices[0].pos.x - vertices[1].pos.x) * (vertices[2].pos.y - vertices[1].pos.y)
            - (vertices[0].pos.y - vertices[1].pos.y) * (vertices[2].pos.x - vertices[1].pos.x);

    if(a < 0.0f) {
        Vertex t = vertices[1];
        vertices[1] = vertices[2];
        vertices[2] = t;
    }
}

static void clear_color(u8 r, u8 g, u8 b, u8 a) {
    if(SDL_SetRenderDrawColor(state.renderer, r, g, b, a) != 0) {
        fprintf(stderr, "Failed to set clear color %x %x %x %x\n", r, g, b, a); 
    }
}

static void present() {
    void *px;
    u32 pitch;
    SDL_LockTexture(state.texture, NULL, &px, &pitch);
    memcpy(px, state.pixels, sizeof(state.pixels));
    SDL_UnlockTexture(state.texture);

    SDL_SetRenderTarget(state.renderer, NULL);
    SDL_SetRenderDrawColor(state.renderer, 0, 0, 0, 0xFF);
    SDL_SetRenderDrawBlendMode(state.renderer, SDL_BLENDMODE_NONE);

    memset(state.pixels, 0, sizeof(state.pixels));
    if(state.depth_test) {
        memset(state.depth_buffer, 0, sizeof(state.depth_buffer));
    }
    SDL_RenderClear(state.renderer);

    SDL_RenderTexture(state.renderer, state.texture, NULL, NULL);
    SDL_RenderPresent(state.renderer);
}

static Texture load_texture(const char *path, TextureFormat format) {
    stbi_set_flip_vertically_on_load(true);

    u8 *data;
    i32 width, height, channels;

    if(format == FORMAT_RGB) {
        data = stbi_load(path, &width, &height, &channels, 3);
    } else if(format == FORMAT_RGBA) {
        data = stbi_load(path, &width, &height, &channels, 4);
    }

    if(!data) {
        fprintf(stderr, "Failed to load image from path %s\n", path);
    }

    Texture texture;
    texture.width = width;
    texture.height = height;
    texture.data = data;
    texture.format = format;
    return texture;
}

static void destroy_texture(Texture *texture) {
    stbi_image_free(texture->data);
}

static void draw_rect_raw(u32 x1, u32 y1, u32 x2, u32 y2, u32 color) {
    for(u32 x = x1; x <= x2; x++) {
        for(u32 y = y1; y <= y2; y++) {
            SET_PIXEL(x, y, color);
        }
    }
}

static void draw_rect(vec3s v1, vec3s v2, u32 color) {
    ivec3s v1_pixels = ndc_to_pixels(v1);
    ivec3s v2_pixels = ndc_to_pixels(v2);

    draw_rect_raw(
        v1_pixels.x,
        v1_pixels.y,
        v2_pixels.x,
        v2_pixels.y,
        color);
}

static void draw_line_raw(u32 x1, u32 y1, u32 x2, u32 y2, u32 color) {
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

static void draw_line(vec3s v1, vec3s v2, u32 color) {
    ivec3s v1_pixels = ndc_to_pixels(v1);
    ivec3s v2_pixels = ndc_to_pixels(v2);

    draw_line_raw(
        v1_pixels.x,
        v1_pixels.y,
        v2_pixels.x,
        v2_pixels.y,
        color);
}

static i32 edge_function(ivec2s a, ivec2s b, ivec2s c) {
    return ((c.x - a.x) * (b.y - a.y) - (c.y - a.y) * (b.x - a.x));
}

static void draw_triangle_raw(RawVertex *vertices, const Texture *texture) {
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
    i32 min_z = min(z1, z2, z3);

    i32 max_x = max(x1, x2, x3);
    i32 max_y = max(y1, y2, y3);
    i32 max_z = max(z1, z2, z3);

    i32 area = abs(x1 * (y2 - y3) + x2 * (y3 - y1) + x3 * (y1 - y2)) >> 1;
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

    vec2s tex_coords;

    for(i32 y = min_y; y < max_y; y++) {
        i32 e1 = e_row1;
        i32 e2 = e_row2;
        i32 e3 = e_row3;

        for(i32 x = min_x; x < max_x; x++) {
            if((e1 >= 0) && (e2 >= 0) && (e3 >= 0)) {
                i32 area_31 = e1 >> 1;
                i32 area_23 = e2 >> 1;
                i32 area_12 = e3 >> 1;

                f32 u, v, w; // Barycentric coordinates
                u = (f32) area_23 * inverse_area;
                v = (f32) area_12 * inverse_area;
                w = (f32) area_31 * inverse_area;

                // Barycentric uv coordinates
                tex_coords.x = (u * uv1.x + v * uv3.x + w * uv2.x);
                tex_coords.y = (u * uv1.y + v * uv3.y + w * uv2.y);

                u32 index =
                    (u32)((tex_coords.x * (texture->width - 1)))
                    + (u32)(tex_coords.y * (texture->height - 1)) * texture->width;

                u32 color = 0x000000FF;

                if(texture->format == FORMAT_RGBA) {
                    index *= 4;

                    color |= ((u32)(texture->data)[index]) << 8; // Red
                    color |= ((u32)(texture->data)[index + 1]) << 16; // Green
                    color |= ((u32)(texture->data)[index + 2]) << 24; // Blue
                    color |= ((u32)(texture->data)[index + 3]); // Alpha
                } else if(texture->format == FORMAT_RGB) {
                    index *= 3;

                    color |= ((u32)(texture->data)[index]) << 8; // Red
                    color |= ((u32)(texture->data)[index + 1]) << 16; // Green
                    color |= ((u32)(texture->data)[index + 2]) << 24; // Blue
                }

                if(state.depth_test) {
                    i32 depth =
                        (i32) (u * z1)
                        + (i32) (v * z3)
                        + (i32) (w * z2);

                    i32 d = state.depth_buffer[y * SCREEN_WIDTH + x];

                    if(d <= 0 || depth < d) {
                        state.depth_buffer[y * SCREEN_WIDTH + x] = depth;
                        SET_PIXEL(x, y, color);
                    }
                } else {
                    SET_PIXEL(x, y, color);
                }
            }

            e1 += ((i32) y3 - (i32) y1);
            e2 += ((i32) y2 - (i32) y3);
            e3 += ((i32) y1 - (i32) y2);
        }

        e_row1 += ((i32) x1 - (i32) x3);
        e_row2 += ((i32) x3 - (i32) x2);
        e_row3 += ((i32) x2 - (i32) x1);
    }
}

static void draw_triangle(const Vertex *vertices, const Texture *texture) {
    RawVertex raw_vertices[3];

    raw_vertices[0] = (RawVertex) {
        .pos = ndc_to_pixels(vertices[0].pos),
        .w = vertices[0].w,
        .uv = vertices[0].uv
    };
    raw_vertices[1] = (RawVertex) {
        .pos = ndc_to_pixels(vertices[1].pos),
        .w = vertices[1].w,
        .uv = vertices[1].uv
    };
    raw_vertices[2] = (RawVertex) {
        .pos = ndc_to_pixels(vertices[2].pos),
        .w = vertices[2].w,
        .uv = vertices[2].uv
    };

    draw_triangle_raw(
        raw_vertices,
        texture);
}

static void draw_triangles(
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
        }
        sort_cw(local_vertices);
        draw_triangle(local_vertices, texture);
    }
}

static void cleanup() {
    SDL_DestroyTexture(state.texture);
    SDL_DestroyRenderer(state.renderer);
    SDL_DestroyWindow(state.window);
}

int main() {
    state.quit = false;

    if(SDL_Init(SDL_INIT_VIDEO)) {
        fprintf(stderr, "Failed to initialize SDL\n");
        return -1;
    }

    state.window = SDL_CreateWindow("Software Renderer", 854, 480, SDL_WINDOW_RESIZABLE);
    if(!state.window) {
        fprintf(stderr, "Failed to create window\n");
        return -1;
    }

    SDL_ShowWindow(state.window);

    state.renderer = SDL_CreateRenderer(state.window, NULL, SDL_RENDERER_PRESENTVSYNC);

    if(!state.renderer) {
        printf("Failed to create renderer %s\n", SDL_GetError());
    }

    state.texture = SDL_CreateTexture(
        state.renderer,
        SDL_PIXELFORMAT_BGRA8888,
        SDL_TEXTUREACCESS_STREAMING,
        SCREEN_WIDTH,
        SCREEN_HEIGHT);
    SDL_SetTextureScaleMode(state.texture, SDL_SCALEMODE_NEAREST);

    clear_color(0, 0, 0, 0xFF);

    memset(state.depth_buffer, 0, sizeof(state.depth_buffer));
    state.depth_test = true;

    Texture texture = load_texture("resources/texture.png", FORMAT_RGBA);

    u32 counter = 0;

    while(!state.quit) {
        SDL_Event event;
        while(SDL_PollEvent(&event)) {
            switch(event.type) {
                case SDL_EVENT_QUIT:
                    state.quit = true;
                    break;
            }
        }

        long start = ns_now();

        vec3s pos = {0.0f, 0.0f, 3.0f};
        mat4s model = glms_mat4_identity();
        mat4s view = glms_mat4_identity();
        mat4s proj = glms_mat4_identity();
        view = glms_lookat(
            pos,
            glms_vec3_add(pos, (vec3s) {0.0f, 0.0f, -1.0f}),
            (vec3s) {0.0f, 1.0f, 0.0f});
        proj = glms_perspective(glm_rad(90.0f), (f32) SCREEN_WIDTH / (f32) SCREEN_HEIGHT, 0.1f, 100.0f);
        model = glms_rotate(model, glm_rad((f32) counter), (vec3s) {1.0f, 1.0f, 0.0f});

        Vertex vertices[18];
        vertices[0] = (Vertex) {
            (vec3s) {-1.0f, 1.0f, 1.0f},
            1.0f,
            (vec2s) {0.0f, 1.0f / 8.0f}
        };
        vertices[1] = (Vertex) {
            (vec3s) {1.0f, 1.0f, 1.0f},
            1.0f,
            (vec2s) {1.0f / 8.0f, 1.0f / 8.0f}
        };
        vertices[2] = (Vertex) {
            (vec3s){1.0f, -1.0f, 1.0f},
            1.0f,
            (vec2s) {1.0f / 8.0f, 0.0f}
        };
        vertices[3] = (Vertex) {
            (vec3s){1.0f, -1.0f, 1.0f},
            1.0f,
            (vec2s) {1.0f / 8.0f, 0.0f}
        };
        vertices[4] = (Vertex) {
            (vec3s){-1.0f, -1.0f, 1.0f},
            1.0f,
            (vec2s) {0.0f, 0.0f}
        };
        vertices[5] = (Vertex) {
            (vec3s) {-1.0f, 1.0f, 1.0f},
            1.0f,
            (vec2s) {0.0f, 1.0f / 8.0f}
        };
        vertices[6] = (Vertex) {
            (vec3s) {-1.0f, -1.0f, 1.0f},
            1.0f,
            (vec2s) {1.0f, 0.0f}
        };
        vertices[7] = (Vertex) {
            (vec3s) {-1.0f, 1.0f, 1.0f},
            1.0f,
            (vec2s) {1.0f, 1.0f}
        };
        vertices[8] = (Vertex) {
            (vec3s) {-1.0f, 1.0f, -1.0f},
            1.0f,
            (vec2s) {0.0f, 1.0f}
        };
        vertices[9] = (Vertex) {
            (vec3s) {-1.0f, 1.0f, -1.0f},
            1.0f,
            (vec2s) {0.0f, 1.0f}
        };
        vertices[10] = (Vertex) {
            (vec3s) {-1.0f, -1.0f, -1.0f},
            1.0f,
            (vec2s) {0.0f, 0.0f}
        };
        vertices[11] = (Vertex) {
            (vec3s) {-1.0f, -1.0f, 1.0f},
            1.0f,
            (vec2s) {1.0f, 0.0f}
        };

        draw_triangles(4, vertices, &texture, proj, view, model);

        long end = ns_now();
        
        present();

        counter++;
        printf("Took %ld ns to render\n", end-start);
    }

    cleanup();

    return 0;
}