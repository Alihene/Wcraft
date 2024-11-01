#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <math.h>
#include <limits.h>
#include <cglm/struct/cam.h>

#include "rendering.h"
#include "camera.h"
#include "world.h"

struct {
    RenderState *render_state;
    bool quit;
} state;

int main() {
    state.quit = false;

    Window window = init_window("Wcraft", (ivec2s) {854, 480});

    state.render_state = init_rendering(&window);
    if(!state.render_state) {
        return -1;
    }

    set_clear_color(0, 0, 0, 0xFF);

    Texture texture = load_texture("resources/texture.png");

    Camera camera = {0};
    camera.pitch = 0.0f;
    camera.yaw = 90.0f;
    camera.pos = (vec3s) {0.0f, 0.0f, 3.0f};

    init_blocks();

    Chunk chunk = init_chunk();
    mesh_chunk(&chunk);

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
                case SDL_EVENT_KEY_DOWN:
                    i32 key_down = event.key.keysym.sym;
                    if (key_down >= 0 && key_down < 512) {
                        window.keyboard[key_down] = true;
                    }
                    break;
                case SDL_EVENT_KEY_UP:
                    i32 key_up = event.key.keysym.sym;
                    if (key_up >= 0 && key_up < 512) {
                        window.keyboard[key_up] = false;
                    }
                    break;
                default:
                    break;
            }
        }

        const u8 *key_states = SDL_GetKeyboardState(NULL);

        if(key_states[SDL_GetScancodeFromKey(SDLK_w)]) {
            camera.pos = glms_vec3_add(camera.pos, glms_vec3_scale((vec3s) {
                cosf(glm_rad(camera.yaw)),
                0.0f,
                sinf(glm_rad(camera.yaw))}, 0.1f));
        }
        if(key_states[SDL_GetScancodeFromKey(SDLK_s)]) {
            camera.pos = glms_vec3_sub(camera.pos, glms_vec3_scale((vec3s) {
                cosf(glm_rad(camera.yaw)),
                0.0f,
                sinf(glm_rad(camera.yaw))}, 0.1f));
        }
        if(key_states[SDL_GetScancodeFromKey(SDLK_a)]) {
            camera.pos =
                glms_vec3_sub(
                    camera.pos,
                    glms_vec3_scale(glms_normalize(glms_cross(camera.front, camera.up)), 0.1f));
        }
        if(key_states[SDL_GetScancodeFromKey(SDLK_d)]) {
            camera.pos =
                glms_vec3_add(
                    camera.pos,
                    glms_vec3_scale(glms_normalize(glms_cross(camera.front, camera.up)), 0.1f));
        }
        if(key_states[SDL_GetScancodeFromKey(SDLK_SPACE)]) {
            camera.pos.y += 0.1f;
        }
        if(key_states[SDL_GetScancodeFromKey(SDLK_LSHIFT)]) {
            camera.pos.y -= 0.1f;
        }

        update_camera(&camera, &window);

        vec3s pos = {0.0f, 0.0f, 3.0f};
        mat4s model = glms_mat4_identity();
        mat4s view = glms_mat4_identity();
        mat4s proj = glms_mat4_identity();
        view = camera.view;
        proj = camera.proj;

        long start = ns_now();

        draw_triangles(chunk.mesh.vertex_count, chunk.mesh.vertices, &texture, proj, view, model);

        long end = ns_now();
        
        present();
        printf("Took %ld ns to render\n", end-start);
    }

    destroy_chunk(&chunk);
    cleanup_rendering();
    destroy_window(&window);
    destroy_texture(&texture);

    return 0;
}