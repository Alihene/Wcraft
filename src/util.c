#include "util.h"

#include <time.h>

#define NS_PER_SECOND 1000000000

u64 ns_now() {
	struct timespec ts;
	timespec_get(&ts, TIME_UTC);
	return ((ts.tv_sec * NS_PER_SECOND) + ts.tv_nsec);
}

void *memset32(void *s, u32 c, u64 n) {
	u32 *_s = (u32*) s;
	for(u64 i = 0; i < n / sizeof(u32); i++) {
		_s[i] = c;
	}
	return s;
}