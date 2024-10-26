#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <math.h>
#include <time.h>
#include <limits.h>
#include <cglm/struct/cam.h>

#include "rendering.h"

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

    state.render_state = init_rendering();
    if(!state.render_state) {
        return -1;
    }

    set_clear_color(0, 0, 0, 0xFF);

    Texture texture = load_texture("resources/texture.png", FORMAT_RGBA);

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

        vec3s pos = {0.0f, 0.0f, 3.0f};
        mat4s model = glms_mat4_identity();
        mat4s view = glms_mat4_identity();
        mat4s proj = glms_mat4_identity();
        view = glms_lookat(
            pos,
            glms_vec3_add(pos, (vec3s) {0.0f, 0.0f, -1.0f}),
            (vec3s) {0.0f, 1.0f, 0.0f});
        proj = glms_perspective(glm_rad(90.0f), (f32) SCREEN_WIDTH / (f32) SCREEN_HEIGHT, 0.1f, 100.0f);
        model = glms_rotate(model, glm_rad((f32) counter), (vec3s) {1.0f, 1.0f, 0.0f});

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
    destroy_texture(&texture);

    return 0;
}