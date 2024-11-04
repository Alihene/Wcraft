#ifndef _UTIL_H
#define _UTIL_H

#include <stdint.h>

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

#define NS_PER_SECOND 1000000000
u64 ns_now();

void *memset32(void *s, u32 c, u64 n);

#endif