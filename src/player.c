#include "player.h"

#include <SDL3/SDL.h>

Player player;

void init_player() {
    player.camera = (Camera) {0};
    player.pos = (vec3s) {0.0f, 0.0f, 0.0f};
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
    player.camera.pos.y += 1.5f; // Camera is at eye level
}