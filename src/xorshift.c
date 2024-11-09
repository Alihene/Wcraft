#include "xorshift.h"

struct {
    u32 a;
} xorshift_state;

void set_xorshift32_seed(u32 seed) {
    xorshift_state.a = seed;
}

u32 xorshift32() {
    u32 x = xorshift_state.a;
    x ^= x << 13;
	x ^= x >> 17;
	x ^= x << 5;
    return xorshift_state.a = x;
}