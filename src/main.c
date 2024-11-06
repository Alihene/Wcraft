#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <math.h>
#include <limits.h>
#include <cglm/struct/cam.h>

#include "rendering.h"
#include "camera.h"
#include "world.h"
#include "player.h"

struct {
    RenderState *render_state;
    bool quit;
} state;

int main() {
    state.quit = false;

    Window window = init_window("Wcraft", (ivec2s) {854, 480});

    state.render_state = init_rendering(&window);
    if(!state.render_state) {
        fprintf(stderr, "Failed to initialize rendering\n");
        return -1;
    }

    set_clear_color(0, 0, 0, 0xFF);

    Texture texture = load_texture("resources/texture.png");

    init_player();

    init_blocks();

    World *world = init_world();

    u64 start_time = SDL_GetTicks();
    f32 last_time = 0.0f;

    u64 last_second = ns_now();
    u32 frames = 0;

    while(!state.quit) {
        window.mouse.movement = (vec2s) {0.0f, 0.0f};

        SDL_Event event;
        while(SDL_PollEvent(&event)) {
            switch(event.type) {
                case SDL_EVENT_QUIT:
                    state.quit = true;
                    break;
                case SDL_EVENT_MOUSE_MOTION:
                    window.mouse.movement.x = event.motion.xrel;
                    window.mouse.movement.y = -event.motion.yrel;
                    break;
                case SDL_EVENT_MOUSE_BUTTON_DOWN:
                    if(event.button.button == SDL_BUTTON_LEFT && event.button.state == SDL_PRESSED) {
                        try_break_block();
                    } else if(event.button.button == SDL_BUTTON_RIGHT && event.button.state == SDL_PRESSED) {
                        try_place_block();
                    }
                    break;
                default:
                    break;
            }
        }

        u64 current_time = SDL_GetTicks();
        f32 elapsed_time = (current_time - start_time) / 1000.0f;
        f32 timestep = elapsed_time - last_time;
        last_time = elapsed_time;

        update_keys(&window);

        update_camera(&player.camera, &window);
        update_player(timestep, window.keys);

        update_world();

        mat4s view = player.camera.view;
        mat4s proj = player.camera.proj;

        for(i32 i = 0; i < world->chunk_count; i++) {
            Chunk *chunk = world->chunks[i];
            if(chunk) {
                draw_triangles(
                    chunk->mesh.vertex_count / 3,
                    chunk->mesh.vertices,
                    &texture,
                    proj,
                    view,
                    glms_translate(
                        glms_mat4_identity(),
                        (vec3s) {chunk->pos.x * 16, 0, chunk->pos.y * 16}));
            }
        }
        frames++;

        u64 now = ns_now();
        if(now - last_second > NS_PER_SECOND) {
            printf("FPS: %u\n", frames);
            frames = 0;
            last_second = now;
        }
        
        present();
    }

    destroy_world();
    cleanup_rendering();
    destroy_window(&window);
    destroy_texture(&texture);

    return 0;
}