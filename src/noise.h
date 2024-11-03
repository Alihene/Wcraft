#ifndef _NOISE_H
#define _NOISE_H

#include "util.h"

void set_seed(i32 s);
f32 noise2d(f32 x, f32 y);

f32 perlin(f32 x, f32 y, f32 frequency, i32 depth);

#endif