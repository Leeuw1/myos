#ifndef _POLL_H
#define _POLL_H

#define POLLIN	0x1

struct pollfd {
	int		fd;			// The following descriptor being polled.
	short	events;		// The input event flags (see below).
	short	revents;	// The output event flags (see below).
};

typedef long	nfds_t;

int poll(struct pollfd [], nfds_t, int);

#endif //_POLL_H
