#ifndef _SYS_RESOURCE_H
#define _SYS_RESOURCE_H

#define RLIMIT_CORE		0	// Limit on size of core image.
#define RLIMIT_CPU		1	// Limit on CPU time per process.
#define RLIMIT_DATA		2	// Limit on data segment size.
#define RLIMIT_FSIZE	3	// Limit on file size.
#define RLIMIT_NOFILE	4	// Limit on number of open files.
#define RLIMIT_STACK	5	// Limit on stack size.
#define RLIMIT_AS		6	// Limit on address space size.

typedef unsigned int	rlim_t;

struct rlimit {
	rlim_t	rlim_cur;	// The current (soft) limit.
	rlim_t	rlim_max;	// The hard limit.
};

int getrlimit(int, struct rlimit*);
int setrlimit(int, const struct rlimit*);

#endif //_SYS_RESOURCE_H
