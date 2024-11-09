#include "util.h"

#include <time.h>
#include <cglm/struct.h>

bool aabb_colliding(AABB a, AABB b) {
	vec3s min = a.pos;
	vec3s max = glms_vec3_add(a.pos, a.size);
	vec3s other_min = b.pos;
	vec3s other_max = glms_vec3_add(b.pos, b.size);

	return
		min.x < other_max.x &&
		max.x > other_min.x &&
		min.y < other_max.y &&
		max.y > other_min.y &&
		min.z < other_max.z &&
		max.z > other_min.z;
}

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