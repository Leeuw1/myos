#ifndef _MYOS_TIME_H
#define _MYOS_TIME_H

#include <time.h>

// Nanoseconds per second
#define BILLION			1000000000
#define CLOCKS_PER_SEC	0x100000

struct timespec time_current(void);

#endif //_MYOS_TIME_H
