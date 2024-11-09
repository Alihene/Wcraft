#include "player.h"

#include <SDL3/SDL.h>
#include "world.h"

#define HITBOX_WIDTH 0.6f
#define HITBOX_HEIGHT 1.8f
#define HITBOX_WIDTH_OFFSET ((1.0f - HITBOX_WIDTH) / 2.0f)

#define ACCELERATION 20.0f

Player player;

void init_player() {
    player.camera = (Camera) {0};
    player.pos = (vec3s) {0.0f, 20.0f, 0.0f};
    player.hotbar_slot = 1;
    player.y_velocity = 0.0f;
}

// Move and handle collisions on the xz plane
static void move_xz(vec3s original_pos, vec3s delta, f32 speed) {
    i32 block_pos_x = floorf(original_pos.x);
    i32 block_pos_y = floorf(original_pos.y);
    i32 block_pos_z = floorf(original_pos.z);

    vec3s scaled_delta = glms_vec3_scale(delta, speed);
    player.pos.x += scaled_delta.x;

    for(i32 x = block_pos_x - 2; x <= block_pos_x + 2; x++) {
        for(i32 y = block_pos_y - 2; y <= block_pos_y + 3; y++) {
            for(i32 z = block_pos_z - 2; z <= block_pos_z + 2; z++) {
                Block *block = world_get(x, y, z);
                if(block && block->solid) {
                    AABB player_aabb = (AABB) {
                        .pos = (vec3s) {
                            player.pos.x + HITBOX_WIDTH_OFFSET,
                            player.pos.y,
                            player.pos.z + HITBOX_WIDTH_OFFSET
                        },
                        .size = (vec3s) {
                            HITBOX_WIDTH,
                            HITBOX_HEIGHT,
                            HITBOX_WIDTH}
                    };
                    AABB block_aabb = (AABB) {
                        .pos = (vec3s) {x, y, z},
                        .size = (vec3s) {1.0f, 1.0f, 1.0f}
                    };
                    if(aabb_colliding(player_aabb, block_aabb)) {
                        if(delta.x >= 0.0f) {
                            player.pos.x = floorf(player.pos.x) + HITBOX_WIDTH_OFFSET - 0.01f;
                        } else {
                            player.pos.x = ceilf(player.pos.x) - HITBOX_WIDTH_OFFSET + 0.01f;
                        }
                    }
                }
            }
        }
    }

    player.pos.z += scaled_delta.z;
    for(i32 x = block_pos_x - 2; x <= block_pos_x + 2; x++) {
        for(i32 y = block_pos_y - 2; y <= block_pos_y + 3; y++) {
            for(i32 z = block_pos_z - 2; z <= block_pos_z + 2; z++) {
                Block *block = world_get(x, y, z);
                if(block && block->solid) {
                    AABB player_aabb = (AABB) {
                        .pos = (vec3s) {
                            player.pos.x + HITBOX_WIDTH_OFFSET,
                            player.pos.y,
                            player.pos.z + HITBOX_WIDTH_OFFSET
                        },
                        .size = (vec3s) {
                            HITBOX_WIDTH,
                            HITBOX_HEIGHT,
                            HITBOX_WIDTH}
                    };
                    AABB block_aabb = (AABB) {
                        .pos = (vec3s) {x, y, z},
                        .size = (vec3s) {1.0f, 1.0f, 1.0f}
                    };
                    if(aabb_colliding(player_aabb, block_aabb)) {
                        if(delta.z >= 0.0f) {
                            player.pos.z = floorf(player.pos.z) + HITBOX_WIDTH_OFFSET - 0.01f;
                        } else {
                            player.pos.z = ceilf(player.pos.z) - HITBOX_WIDTH_OFFSET + 0.01f;
                        }
                    }
                }
            }
        }
    }
}

// Move and handle collisions on the y plane
static void move_y(vec3s original_pos, f32 delta_y, f32 speed) {
    i32 block_pos_x = floorf(original_pos.x);
    i32 block_pos_y = floorf(original_pos.y);
    i32 block_pos_z = floorf(original_pos.z);

    player.pos.y += delta_y * speed;

    for(i32 x = block_pos_x - 2; x <= block_pos_x + 2; x++) {
        for(i32 y = block_pos_y - 2; y <= block_pos_y + 3; y++) {
            for(i32 z = block_pos_z - 2; z <= block_pos_z + 2; z++) {
                Block *block = world_get(x, y, z);
                if(block && block->solid) {
                    AABB player_aabb = (AABB) {
                        .pos = (vec3s) {
                            player.pos.x + HITBOX_WIDTH_OFFSET,
                            player.pos.y,
                            player.pos.z + HITBOX_WIDTH_OFFSET
                        },
                        .size = (vec3s) {HITBOX_WIDTH, HITBOX_HEIGHT, HITBOX_WIDTH}
                    };
                    AABB block_aabb = (AABB) {
                        .pos = (vec3s) {x, y, z},
                        .size = (vec3s) {1.0f, 1.0f, 1.0f}
                    };
                    if(aabb_colliding(player_aabb, block_aabb)) {
                        if(delta_y >= 0.0f) {
                            player.pos.y = floorf(player.pos.y) - 1.0f;
                        } else {
                            player.pos.y = ceilf(player.pos.y);
                        }
                    }
                }
            }
        }
    }
}

void update_player(f32 timestep, const u8 *keys) {
    i32 block_pos_x = floorf(player.pos.x);
    i32 block_pos_y = floorf(player.pos.y);
    i32 block_pos_z = floorf(player.pos.z);
    vec3s original_pos = player.pos;

    Block *block_below = world_get(block_pos_x, block_pos_y - 1, block_pos_z);
    bool is_on_block = block_below && block_below->solid && player.pos.y - block_pos_y < 0.02f;
    if(!is_on_block) {
        player.y_velocity -= ACCELERATION * timestep;
    } else {
        player.y_velocity = 0;
    }

    f32 speed = 4.0f * timestep;
    if(keys[SDL_GetScancodeFromKey(SDLK_w)]) {
        vec3s delta = (vec3s) {
            cosf(glm_rad(player.camera.yaw)),
            0.0f,
            sinf(glm_rad(player.camera.yaw))};
        move_xz(original_pos, delta, speed);
    }
    if(keys[SDL_GetScancodeFromKey(SDLK_s)]) {
        vec3s delta = (vec3s) {
            cosf(glm_rad(player.camera.yaw)),
            0.0f,
            sinf(glm_rad(player.camera.yaw))};
        delta = glms_vec3_negate(delta);
        move_xz(original_pos, delta, speed);
    }
    if(keys[SDL_GetScancodeFromKey(SDLK_a)]) {
        vec3s delta = glms_normalize(glms_cross(player.camera.front, player.camera.up));
        delta = glms_vec3_negate(delta);
        move_xz(original_pos, delta, speed);
    }
    if(keys[SDL_GetScancodeFromKey(SDLK_d)]) {
        vec3s delta = glms_normalize(glms_cross(player.camera.front, player.camera.up));
        move_xz(original_pos, delta, speed);
    }
    if(keys[SDL_GetScancodeFromKey(SDLK_SPACE)]) {
        if(is_on_block) {
            player.y_velocity = 7.5f;
        }
    }
    move_y(original_pos, player.y_velocity, timestep);

    player.camera.pos = player.pos;
    player.camera.pos.x += 0.5f;
    player.camera.pos.y += 1.5f; // Camera is at eye level
    player.camera.pos.z += 0.5f;
}

static Block *raycast(vec3s dir, f32 distance, ivec3s *out_pos, ivec3s *last_pos) {
    f32 delta_magnitude = 0.01f;
    vec3s delta = glms_vec3_scale(dir, delta_magnitude);

    vec3s current_pos = player.camera.pos;

    for(f32 distance_travelled = 0.0f; distance_travelled < distance; distance_travelled += delta_magnitude) {
        // Fast floor
        i32 block_pos_x = floorf(current_pos.x);
        i32 block_pos_y = floorf(current_pos.y);
        i32 block_pos_z = floorf(current_pos.z);

        Block *block = world_get(block_pos_x, block_pos_y, block_pos_z);
        if(block && block->type != BLOCK_AIR) {
            if(out_pos) {
                *out_pos = (ivec3s) {
                    block_pos_x,
                    block_pos_y,
                    block_pos_z
                };
            }
            return block;
        }

        if(last_pos) {
            *last_pos = (ivec3s) {
                block_pos_x,
                block_pos_y,
                block_pos_z
            };
        }

        current_pos = glms_vec3_add(current_pos, delta);
    }

    return NULL;
}

void try_break_block() {
    ivec3s block_pos;
    Block *block = raycast(player.camera.front, 5, &block_pos, NULL);

    if(!block) {
        return;
    }

    world_set_and_mesh(
        &blocks[BLOCK_AIR],
        block_pos.x,
        block_pos.y,
        block_pos.z);
}

void try_place_block() {
    ivec3s place_pos;
    if(raycast(player.camera.front, 5, NULL, &place_pos)) {
        world_set_and_mesh(
            &blocks[player.hotbar_slot],
            place_pos.x,
            place_pos.y,
            place_pos.z);
    }
}