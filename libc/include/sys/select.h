#ifndef _SYS_SELECT_H
#define _SYS_SELECT_H

#include <sys/types.h>

struct timeval {
	time_t		tv_sec;		// Seconds.
	suseconds_t	tv_usec;	// Microseconds.
};

#endif //_SYS_SELECT_H
