#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <math.h>
#include <limits.h>
#include <cglm/struct/cam.h>
#include <pthread.h>

#include "rendering.h"
#include "camera.h"
#include "world.h"
#include "player.h"
#include "thread_pool.h"

struct {
    RenderState *render_state;
    World *world;
    Texture *atlas;
    bool quit;
} state;

static void *render_chunks(void *arg) {
    World *world = state.world;

    while(true) {
        pthread_mutex_lock(&world->chunks_mutex);
        for(i32 i = 0; i < world->chunk_count; i++) {
            Chunk *chunk = world->chunks[i];
            if(chunk && !chunk->mesh.being_rendered) {
                chunk->mesh.being_rendered = true;
                draw_triangles(
                    chunk->mesh.vertex_count / 3,
                    chunk->mesh.vertices,
                    state.atlas,
                    player.camera.proj,
                    player.camera.view,
                    glms_translate(
                        glms_mat4_identity(),
                        (vec3s) {chunk->pos.x * 16, 0, chunk->pos.y * 16}));
                chunk->mesh.being_rendered = false;
            }
        }
        pthread_mutex_unlock(&world->chunks_mutex);
    }
    return NULL;
}

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
    state.atlas = &texture;

    init_player();

    init_blocks();

    World *world = init_world();
    state.world = world;
    world_set(&blocks[BLOCK_COBBLESTONE], 100, 31, 100);

    u64 start_time = SDL_GetTicks();
    f32 last_time = 0.0f;

    u64 last_second = ns_now();
    u32 frames = 0;

    // pthread_t render_threads[1];
    // for(i32 i = 0; i < 1; i++) {
    //     if(pthread_create(&render_threads[i], NULL, render_chunks, NULL) != 0) {
    //         printf("Failed\n");
    //         return -1;
    //     }
    // }

    ThreadPool thread_pool;
    init_thread_pool(&thread_pool, 2);

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
                case SDL_EVENT_KEY_DOWN:
                    if(event.key.keysym.scancode <= SDL_GetScancodeFromKey(SDLK_9) && event.key.keysym.scancode >= SDL_GetScancodeFromKey(SDLK_1)) {
                        player.hotbar_slot = event.key.keysym.scancode - 29;
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
        pthread_mutex_lock(&world->chunks_mutex);
        update_player(timestep, window.keys);
        pthread_mutex_unlock(&world->chunks_mutex);

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

    destroy_thread_pool(&thread_pool);

    destroy_world();
    cleanup_rendering();
    destroy_window(&window);
    destroy_texture(&texture);

    return 0;
}