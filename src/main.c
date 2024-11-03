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

        mat4s view = player.camera.view;
        mat4s proj = player.camera.proj;

        u32 triangle_count = 0;
        long start = ns_now();
        for(i32 i = 0; i < SQ(LOAD_WIDTH); i++) {
            Chunk *chunk = &world->chunks[i];
            draw_triangles(
                chunk->mesh.vertex_count / 3,
                chunk->mesh.vertices,
                &texture,
                proj,
                view,
                glms_translate(
                    glms_mat4_identity(),
                    (vec3s) {chunk->pos.x * 16, 0, chunk->pos.y * 16}));
            triangle_count += chunk->mesh.vertex_count;
        }
        long end = ns_now();
        
        present();
        triangle_count /= 3;
        if(player.camera.pos.y >= 0.0f) {
            triangle_count -= SQ(LOAD_WIDTH) * CHUNK_WIDTH * CHUNK_DEPTH * 2;
        }
        printf("Took %ld ns to render %u triangles\n", end-start, triangle_count);
    }

    cleanup_rendering();
    destroy_window(&window);
    destroy_texture(&texture);

    return 0;
}