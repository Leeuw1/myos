#include "myos_time.h"
#include "regs.h"

struct timespec time_current(void) {
	const u64 clocks_per_sec = 0x100000;
	const u64 t = SYS_TIMER->clo;
	return (struct timespec){
		.tv_sec = t / clocks_per_sec,
		.tv_nsec = ((t % clocks_per_sec) * BILLION) / clocks_per_sec,
	};
}
