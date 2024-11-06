#include "player.h"

#include <SDL3/SDL.h>
#include "world.h"

Player player;

void init_player() {
    player.camera = (Camera) {0};
    player.pos = (vec3s) {0.0f, 20.0f, 0.0f};
}

void update_player(f32 timestep, const u8 *keys) {
    f32 speed = 10.0f * timestep;
    if(keys[SDL_GetScancodeFromKey(SDLK_w)]) {
        player.pos = glms_vec3_add(player.pos, glms_vec3_scale((vec3s) {
            cosf(glm_rad(player.camera.yaw)),
            0.0f,
            sinf(glm_rad(player.camera.yaw))}, speed));
    }
    if(keys[SDL_GetScancodeFromKey(SDLK_s)]) {
        player.pos = glms_vec3_sub(player.pos, glms_vec3_scale((vec3s) {
            cosf(glm_rad(player.camera.yaw)),
            0.0f,
            sinf(glm_rad(player.camera.yaw))}, speed));
    }
    if(keys[SDL_GetScancodeFromKey(SDLK_a)]) {
        player.pos =
            glms_vec3_sub(
                player.pos,
                glms_vec3_scale(glms_normalize(glms_cross(player.camera.front, player.camera.up)), speed));
    }
    if(keys[SDL_GetScancodeFromKey(SDLK_d)]) {
        player.pos =
            glms_vec3_add(
                player.pos,
                glms_vec3_scale(glms_normalize(glms_cross(player.camera.front, player.camera.up)), speed));
    }
    if(keys[SDL_GetScancodeFromKey(SDLK_SPACE)]) {
        player.pos.y += speed;
    }
    if(keys[SDL_GetScancodeFromKey(SDLK_LSHIFT)]) {
        player.pos.y -= speed;
    }
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
            &blocks[BLOCK_GLASS],
            place_pos.x,
            place_pos.y,
            place_pos.z);
    }
}