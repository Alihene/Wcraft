#ifndef _UTIL_H
#define _UTIL_H

#include <stdint.h>
#include <stdbool.h>
#include <cglm/struct.h>

typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef float f32;
typedef double f64;

#define SQ(x) ((x) * (x))
#define LERP(x, y, s) ((x) + (s) * ((y) - (x)))
#define LERP_SMOOTH(x, y, s) LERP((x), (y), (s) * (s) * (3 - 2 * (s)))

#define MOD(x, y) (((x) % (y) + (y)) % (y))

typedef struct {
    vec3s pos;
    vec3s size;
} AABB;

bool aabb_colliding(AABB a, AABB b);

#define NS_PER_SECOND 1000000000
u64 ns_now();

void *memset32(void *s, u32 c, u64 n);

// https://stackoverflow.com/questions/6996764/fastest-way-to-do-horizontal-sse-vector-sum-or-other-reduction
inline f32 hsum_ps_sse3(__m128 v) {
    __m128 shuf = _mm_movehdup_ps(v);
    __m128 sums = _mm_add_ps(v, shuf);
    shuf = _mm_movehl_ps(shuf, sums);
    sums = _mm_add_ss(sums, shuf);
    return _mm_cvtss_f32(sums);
}

#endif