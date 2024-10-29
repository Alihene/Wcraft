#include "util.h"

#include <time.h>

#define NS_PER_SECOND 1000000000

u64 ns_now() {
	struct timespec ts;
	timespec_get(&ts, TIME_UTC);
	return ((ts.tv_sec * NS_PER_SECOND) + ts.tv_nsec);
}