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

#define COS_40_DEG 0.766

struct {
    RenderState *render_state;
    World *world;
    Texture *atlas;
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
        update_player(timestep, window.keys);

        update_world();
        
        mat4s view = player.camera.view;
        mat4s proj = player.camera.proj;

        for(u32 i = 0; i < world->chunk_count; i++) {
            Chunk *chunk = world->chunks[i];
            if(chunk) {
                // Something resembling frustum culling
                vec2s chunk_pos_xz = (vec2s) {chunk->pos.x * 16 - player.pos.x, chunk->pos.y * 16 - player.pos.z};
                vec2s camera_front_xz = (vec2s) {player.camera.front.x, player.camera.front.z};

                f32 dp1 = glms_vec2_dot(camera_front_xz, glms_vec2_normalize(chunk_pos_xz));
                chunk_pos_xz.x += CHUNK_WIDTH;
                f32 dp2 = glms_vec2_dot(camera_front_xz, glms_vec2_normalize(chunk_pos_xz));
                chunk_pos_xz.y += CHUNK_DEPTH;
                f32 dp3 = glms_vec2_dot(camera_front_xz, glms_vec2_normalize(chunk_pos_xz));
                chunk_pos_xz.x -= CHUNK_WIDTH;
                f32 dp4 = glms_vec2_dot(camera_front_xz, glms_vec2_normalize(chunk_pos_xz));

                f32 horizontal_angle = cosf(CAMERA_FOV / 2) / ((f32) SCREEN_WIDTH / (f32) SCREEN_HEIGHT);

                if(dp1 >= horizontal_angle
                    || dp2 >= horizontal_angle
                    || dp3 >= horizontal_angle
                    || dp4 >= horizontal_angle) {
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
        }
        draw_screen();
        
        frames++;
        u64 now = ns_now();
        if(now - last_second > NS_PER_SECOND) {
            printf("FPS: %u\n", frames);
            frames = 0;
            last_second = now;
        }
        
        render_wait();
        present();
    }

    destroy_world();
    cleanup_rendering();
    destroy_window(&window);
    destroy_texture(&texture);

    return 0;
}