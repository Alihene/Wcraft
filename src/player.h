#ifndef _PLAYER_H
#define _PLAYER_H

#include <cglm/struct.h>

#include "util.h"
#include "camera.h"

typedef struct {
    vec3s pos;
    Camera camera;
    u32 hotbar_slot;

    f32 y_velocity;
} Player;

extern Player player;

void init_player();

void update_player(f32 timestep, const u8 *keys);

void try_break_block();
void try_place_block();

#endif