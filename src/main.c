#include <stdio.h>
#include <SDL3/SDL.h>
#include <stdbool.h>
#include <stdint.h>
#include <math.h>
#include <time.h>
#include <limits.h>

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

const uint8_t gammas[256] = {
    0, 21, 29, 35, 39, 43, 47, 51, 54, 57, 59, 62, 64, 67, 69, 71,
    73, 75, 77, 79, 81, 83, 85, 86, 88, 90, 91, 93, 94, 96, 97, 99,
    100, 102, 103, 104, 106, 107, 108, 110, 111, 112, 113, 114, 116, 117, 118, 119,
    120, 121, 122, 124, 125, 126, 127, 128, 129, 130, 131, 132, 133, 134, 135, 136,
    137, 138, 139, 140, 141, 142, 143, 143, 144, 145, 146, 147, 148, 149, 150, 150,
    151, 152, 153, 154, 155, 156, 156, 157, 158, 159, 160, 160, 161, 162, 163, 164,
    164, 165, 166, 167, 167, 168, 169, 170, 170, 171, 172, 173, 173, 174, 175, 175,
    176, 177, 178, 178, 179, 180, 180, 181, 182, 182, 183, 184, 184, 185, 186, 186,
    187, 188, 188, 189, 190, 190, 191, 192, 192, 193, 193, 194, 195, 195, 196, 197,
    197, 198, 198, 199, 200, 200, 201, 201, 202, 203, 203, 204, 204, 205, 206, 206,
    207, 207, 208, 208, 209, 210, 210, 211, 211, 212, 212, 213, 214, 214, 215, 215,
    216, 216, 217, 217, 218, 219, 219, 220, 220, 221, 221, 222, 222, 223, 223, 224,
    224, 225, 225, 226, 227, 227, 228, 228, 229, 229, 230, 230, 231, 231, 232, 232,
    233, 233, 234, 234, 235, 235, 236, 236, 237, 237, 238, 238, 239, 239, 240, 240,
    241, 241, 242, 242, 242, 243, 243, 244, 244, 245, 245, 246, 246, 247, 247, 248,
    248, 249, 249, 250, 250, 250, 251, 251, 252, 252, 253, 253, 254, 254, 255, 255,
};

#define DEPTH_PRECISION 4096

struct {
    SDL_Window *window;
    SDL_Surface *surface;
    SDL_Renderer *renderer;
    SDL_Texture *texture;
    u32 pixels[SCREEN_WIDTH * SCREEN_HEIGHT];
    u32 depth_buffer[SCREEN_WIDTH * SCREEN_HEIGHT];
    bool depth_test;
    bool quit;
} state;

typedef struct {
    i32 x, y;
} vec2i;

typedef struct {
    i32 x, y, z;
} vec3i;

typedef struct {
    f32 x, y;
} vec2f;

typedef struct {
    f32 x, y, z;
} vec3f;

static u32 max(i32 a, i32 b, i32 c) {
    if(a >= b && a >= c) {
        return a;
    }

    if(b >= a && b >= c) {
        return b;
    }

    return c;
}

static u32 min(i32 a, i32 b, i32 c) {
    if(a <= b && a <= c) {
        return a;
    }

    if(b <= a && b <= c) {
        return b;
    }

    return c;
}

static vec3i ndc_to_pixels(vec3f ndc) {
    return (vec3i) {
        ((ndc.x + 1.0f) / 2.0f) * (SCREEN_WIDTH),
        (1.0f - ((ndc.y + 1.0f) / 2.0f)) * (SCREEN_HEIGHT),
        ndc.z * DEPTH_PRECISION
    };
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

static void draw_rect_raw(u32 x1, u32 y1, u32 x2, u32 y2, u32 color) {
    for(u32 x = x1; x <= x2; x++) {
        for(u32 y = y1; y <= y2; y++) {
            SET_PIXEL(x, y, color);
        }
    }
}

static void draw_rect(vec3f v1, vec3f v2, u32 color) {
    vec3i v1_pixels = ndc_to_pixels(v1);
    vec3i v2_pixels = ndc_to_pixels(v2);

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

static void draw_line(vec3f v1, vec3f v2, u32 color) {
    vec3i v1_pixels = ndc_to_pixels(v1);
    vec3i v2_pixels = ndc_to_pixels(v2);

    draw_line_raw(
        v1_pixels.x,
        v1_pixels.y,
        v2_pixels.x,
        v2_pixels.y,
        color);
}

static i32 edge_function(vec2i a, vec2i b, vec2i c) {
    return ((c.x - a.x) * (b.y - a.y) - (c.y - a.y) * (b.x - a.x));
}

static void draw_triangle_raw(i32 x1, i32 y1, i32 z1, i32 x2, i32 y2, i32 z2, i32 x3, i32 y3, i32 z3) {
    i32 min_x = min(x1, x2, x3);
    i32 min_y = min(y1, y2, y3);

    i32 max_x = max(x1, x2, x3);
    i32 max_y = max(y1, y2, y3);

    SDL_clamp(min_x, 0, SCREEN_WIDTH);
    SDL_clamp(max_x, 0, SCREEN_WIDTH);
    SDL_clamp(min_y, 0, SCREEN_HEIGHT);
    SDL_clamp(max_y, 0, SCREEN_HEIGHT);
    SDL_clamp(z1, 0, DEPTH_PRECISION);
    SDL_clamp(z2, 0, DEPTH_PRECISION);
    SDL_clamp(z3, 0, DEPTH_PRECISION);

    i32 area = (max_x - min_x) * (max_y - min_y);

    i32 e_row1, e_row2, e_row3;
    e_row1 = edge_function((vec2i){x1, y1}, (vec2i){x3, y3}, (vec2i){min_x, min_y});
    e_row2 = edge_function((vec2i){x3, y3}, (vec2i){x2, y2}, (vec2i){min_x, min_y});
    e_row3 = edge_function((vec2i){x2, y2}, (vec2i){x1, y1}, (vec2i){min_x, min_y});

    for(u32 y = min_y; y < max_y; y++) {
        i32 e1 = e_row1;
        i32 e2 = e_row2;
        i32 e3 = e_row3;

        for(u32 x = min_x; x < max_x; x++) {
            if((e1 >= 0) && (e2 >= 0) && (e3 >= 0)) {
                i32 area_31 = e1 >> 1;
                i32 area_23 = e2 >> 1;
                i32 area_12 = e3 >> 1;

                f32 u, v, w; // Barycentric coordinates
                u = (f32) area_23 / (f32) area;
                v = (f32) area_12 / (f32) area;
                w = (f32) area_31 / (f32) area;

                u32 c = 0x000000FF;
                c |= gammas[((u32) (u * 512.0f))] << 8;
                c |= gammas[((u32) (v * 512.0f))] << 16;
                c |= gammas[((u32) (w * 512.0f))] << 24;

                if(state.depth_test) {
                    u32 depth =
                        (u32) floor((u * z1))
                        + (u32) floor(v * z3)
                        + (u32) floor(w * z2);
                    u32 *depth_ptr = state.depth_buffer + (y * SCREEN_WIDTH + x);
                    if(*depth_ptr <= depth) {
                        *depth_ptr = depth;
                        SET_PIXEL(x, y, c);
                    }
                } else {
                    SET_PIXEL(x, y, c);
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

static void draw_triangle(vec3f v1, vec3f v2, vec3f v3, u32 color) {
    vec3i v1_pixels = ndc_to_pixels(v1);
    vec3i v2_pixels = ndc_to_pixels(v2);
    vec3i v3_pixels = ndc_to_pixels(v3);

    draw_triangle_raw(
        v1_pixels.x,
        v1_pixels.y,
        v1_pixels.z,
        v2_pixels.x,
        v2_pixels.y,
        v2_pixels.z,
        v3_pixels.x,
        v3_pixels.y,
        v3_pixels.z);
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
        printf("no renderer %s\n", SDL_GetError());
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

        draw_triangle(
            (vec3f){0.0f, 1.0f, 1.0f},
            (vec3f){1.0f, -1.0f, 1.0f},
            (vec3f){-1.0f, -1.0f, 1.0f},
            0x0000FFFF);

        long end = ns_now();
        
        present();

        counter++;
        printf("Took %ld ns to render\n", end-start);
    }

    cleanup();

    return 0;
}