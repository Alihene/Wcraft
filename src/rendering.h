#ifndef _RENDERING_H
#define _RENDERING_H

#include <cglm/struct.h>
#include <SDL3/SDL.h>

#include "util.h"
#include "window.h"

#define SCREEN_WIDTH 427
#define SCREEN_HEIGHT 240

#define DEPTH_PRECISION (1 << 20)

typedef struct {
    SDL_Surface *surface;
    SDL_Renderer *renderer;
    SDL_Texture *texture;
    u32 pixels[SCREEN_WIDTH * SCREEN_HEIGHT];
    i32 depth_buffer[SCREEN_WIDTH * SCREEN_HEIGHT];
    bool depth_test;
} RenderState;

typedef struct {
    u8 *data;
    u32 width;
    u32 height;
} Texture;

typedef struct {
    vec3s pos;
    f32 w; // For perspective correct interpolation
    vec2s uv;
    f32 brightness;
} Vertex;

typedef struct {
    ivec3s pos;
    f32 w; // For perspective correct interpolation
    vec2s uv;
    f32 brightness;
} RawVertex;

RenderState *init_rendering(Window *window);

void cleanup_rendering();

void set_clear_color(u8 r, u8 g, u8 b, u8 a);

void present();

Texture load_texture(const char *path);
void destroy_texture(Texture *texture);

void draw_line(vec3s v1, vec3s v2, u32 color);
void draw_line_raw(u32 x1, u32 y1, u32 x2, u32 y2, u32 color);

void draw_triangles(
    u32 count,
    const Vertex *vertices,
    const Texture *texture,
    mat4s proj,
    mat4s view,
    mat4s model);
void draw_triangle(const Vertex *vertices, const Texture *texture);
void draw_triangle_raw(RawVertex *vertices, const Texture *texture);

vec2s lpc(vec2s tex_coords, f32 delta);
vec2s rpc(vec2s tex_coords, f32 delta);

#endif