#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <math.h>
#include <time.h>
#include <limits.h>
#include <cglm/struct/cam.h>

#include "rendering.h"
#include "camera.h"

#define NS_PER_SECOND 1000000000

static long ns_now() {
	struct timespec ts;
	timespec_get(&ts, TIME_UTC);
	return ((ts.tv_sec * NS_PER_SECOND) + ts.tv_nsec);
}

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

    Texture texture = load_texture("resources/texture.png", FORMAT_RGBA);

    Camera camera = {0};
    camera.pitch = 0.0f;
    camera.yaw = 90.0f;
    camera.pos = (vec3s) {0.0f, 0.0f, 3.0f};

    u32 counter = 0;

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

        long start = ns_now();

        vec3s pos = {0.0f, 0.0f, 3.0f};
        mat4s model = glms_mat4_identity();
        mat4s view = glms_mat4_identity();
        mat4s proj = glms_mat4_identity();
        view = camera.view;
        proj = camera.proj;

        Vertex vertices[36];
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
            (vec2s) {1.0f / 8.0f, 0.0f}
        };
        vertices[7] = (Vertex) {
            (vec3s) {-1.0f, 1.0f, 1.0f},
            1.0f,
            (vec2s) {1.0f / 8.0f, 1.0f / 8.0f}
        };
        vertices[8] = (Vertex) {
            (vec3s) {-1.0f, 1.0f, -1.0f},
            1.0f,
            (vec2s) {0.0f, 1.0f / 8.0f}
        };
        vertices[9] = (Vertex) {
            (vec3s) {-1.0f, 1.0f, -1.0f},
            1.0f,
            (vec2s) {0.0f, 1.0f / 8.0f}
        };
        vertices[10] = (Vertex) {
            (vec3s) {-1.0f, -1.0f, -1.0f},
            1.0f,
            (vec2s) {0.0f, 0.0f}
        };
        vertices[11] = (Vertex) {
            (vec3s) {-1.0f, -1.0f, 1.0f},
            1.0f,
            (vec2s) {1.0f / 8.0f, 0.0f}
        };
        vertices[12] = (Vertex) {
            (vec3s) {-1.0f, 1.0f, 1.0f},
            1.0f,
            (vec2s) {0.0f, 0.0f}
        };
        vertices[13] = (Vertex) {
            (vec3s) {1.0f, 1.0f, 1.0f},
            1.0f,
            (vec2s) {1.0f / 8.0f, 0.0f}
        };
        vertices[14] = (Vertex) {
            (vec3s) {1.0f, 1.0f, -1.0f},
            1.0f,
            (vec2s) {1.0f / 8.0f, 1.0f / 8.0f}
        };
        vertices[15] = (Vertex) {
            (vec3s) {1.0f, 1.0f, -1.0f},
            1.0f,
            (vec2s) {1.0f / 8.0f, 1.0f / 8.0f}
        };
        vertices[16] = (Vertex) {
            (vec3s) {-1.0f, 1.0f, -1.0f},
            1.0f,
            (vec2s) {0.0f, 1.0f / 8.0f}
        };
        vertices[17] = (Vertex) {
            (vec3s) {-1.0f, 1.0f, 1.0f},
            1.0f,
            (vec2s) {0.0f, 0.0f}
        };
        vertices[18] = (Vertex) {
            (vec3s) {-1.0f, 1.0f, -1.0f},
            1.0f,
            (vec2s) {0.0f, 1.0f / 8.0f}
        };
        vertices[19] = (Vertex) {
            (vec3s) {1.0f, 1.0f, -1.0f},
            1.0f,
            (vec2s) {1.0f / 8.0f, 1.0f / 8.0f}
        };
        vertices[20] = (Vertex) {
            (vec3s){1.0f, -1.0f, -1.0f},
            1.0f,
            (vec2s) {1.0f / 8.0f, 0.0f}
        };
        vertices[21] = (Vertex) {
            (vec3s){1.0f, -1.0f, -1.0f},
            1.0f,
            (vec2s) {1.0f / 8.0f, 0.0f}
        };
        vertices[22] = (Vertex) {
            (vec3s){-1.0f, -1.0f, -1.0f},
            1.0f,
            (vec2s) {0.0f, 0.0f}
        };
        vertices[23] = (Vertex) {
            (vec3s) {-1.0f, 1.0f, -1.0f},
            1.0f,
            (vec2s) {0.0f, 1.0f / 8.0f}
        };
        vertices[24] = (Vertex) {
            (vec3s) {1.0f, -1.0f, 1.0f},
            1.0f,
            (vec2s) {1.0f / 8.0f, 0.0f}
        };
        vertices[25] = (Vertex) {
            (vec3s) {1.0f, 1.0f, 1.0f},
            1.0f,
            (vec2s) {1.0f / 8.0f, 1.0f / 8.0f}
        };
        vertices[26] = (Vertex) {
            (vec3s) {1.0f, 1.0f, -1.0f},
            1.0f,
            (vec2s) {0.0f, 1.0f / 8.0f}
        };
        vertices[27] = (Vertex) {
            (vec3s) {1.0f, 1.0f, -1.0f},
            1.0f,
            (vec2s) {0.0f, 1.0f / 8.0f}
        };
        vertices[28] = (Vertex) {
            (vec3s) {1.0f, -1.0f, -1.0f},
            1.0f,
            (vec2s) {0.0f, 0.0f}
        };
        vertices[29] = (Vertex) {
            (vec3s) {1.0f, -1.0f, 1.0f},
            1.0f,
            (vec2s) {1.0f / 8.0f, 0.0f}
        };
        vertices[30] = (Vertex) {
            (vec3s) {-1.0f, -1.0f, 1.0f},
            1.0f,
            (vec2s) {0.0f, 0.0f}
        };
        vertices[31] = (Vertex) {
            (vec3s) {1.0f, -1.0f, 1.0f},
            1.0f,
            (vec2s) {1.0f / 8.0f, 0.0f}
        };
        vertices[32] = (Vertex) {
            (vec3s) {1.0f, -1.0f, -1.0f},
            1.0f,
            (vec2s) {1.0f / 8.0f, 1.0f / 8.0f}
        };
        vertices[33] = (Vertex) {
            (vec3s) {1.0f, -1.0f, -1.0f},
            1.0f,
            (vec2s) {1.0f / 8.0f, 1.0f / 8.0f}
        };
        vertices[34] = (Vertex) {
            (vec3s) {-1.0f, -1.0f, -1.0f},
            1.0f,
            (vec2s) {0.0f, 1.0f / 8.0f}
        };
        vertices[35] = (Vertex) {
            (vec3s) {-1.0f, -1.0f, 1.0f},
            1.0f,
            (vec2s) {0.0f, 0.0f}
        };

        draw_triangles(12, vertices, &texture, proj, view, model);

        long end = ns_now();
        
        present();

        counter++;
        printf("Took %ld ns to render\n", end-start);
    }

    cleanup_rendering();
    destroy_window(&window);
    destroy_texture(&texture);

    return 0;
}