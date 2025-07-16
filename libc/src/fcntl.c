#include <fcntl.h>
#define MYOS_NO_BOOL
#include "../../src/syscall.h"

#include <stdio.h>
#define UNIMP()			printf("[libc] Warning: %s is not implemented.\n" ,__FUNCTION__);

int open(const char* path, int flags, ...) {
	_syscall_2arg(SYSCALL_OPEN, int, path, flags);
}

int fcntl(int fd, int cmd, ...) {
	UNIMP();
	return 0;
}

int ioctl(int, int, ...) {
	UNIMP();
	return -1;
}
