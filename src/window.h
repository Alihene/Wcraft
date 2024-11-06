#ifndef _WINDOW_H
#define _WINDOW_H

#include <SDL3/SDL.h>
#include <cglm/struct.h>

#include "util.h"

typedef struct {
    SDL_Window *handle;

    ivec2s dimensions;

    bool cursor_active;

    struct {
        vec2s movement;
    } mouse;

    const u8 *keys;
} Window;

Window init_window(const char *name, ivec2s dimensions);

void destroy_window(Window *window);

void update_keys(Window *window);

#endif