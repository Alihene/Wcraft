#include "window.h"

Window init_window(const char *name, ivec2s dimensions) {
    Window window = {0};
    window.dimensions = dimensions;

    if(SDL_Init(SDL_INIT_VIDEO)) {
        fprintf(stderr, "Failed to initialize SDL\n");
    }
    
    window.handle = SDL_CreateWindow(
        name,
        dimensions.x,
        dimensions.y,
        SDL_WINDOW_RESIZABLE);
    if(!window.handle) {
        fprintf(stderr, "Failed to create window\n");
    }

    SDL_ShowWindow(window.handle);

    window.cursor_active = false;
    SDL_SetRelativeMouseMode(SDL_TRUE);
    
    return window;
}

void destroy_window(Window *window) {
    SDL_DestroyWindow(window->handle);
}

void update_keys(Window *window) {
    window->keys = SDL_GetKeyboardState(NULL);
}