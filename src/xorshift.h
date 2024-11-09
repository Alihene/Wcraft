#ifndef _XORSHIFT_H
#define _XORSHIFT_H

#include "util.h"

void set_xorshift32_seed(u32 seed);

u32 xorshift32();

#endif