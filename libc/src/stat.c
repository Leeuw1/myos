#include <sys/stat.h>
#include <unistd.h>

#define MYOS_NO_BOOL
#include "../../src/syscall.h"

int chmod(const char *, mode_t) {
	printf("[libc] Warning, chmod is not implemented.\n");
	return 0;
}

int __attribute__((noinline)) fstat(int fd, struct stat* statp) {
	_syscall_2arg(SYSCALL_FSTAT, int, fd, statp);
}

int mkdir(const char* path, mode_t mode) {
	_syscall_2arg(SYSCALL_MKDIR, int, path, mode);
}

// TODO: could also just make this a system call
int stat(const char* restrict path, struct stat* restrict statp) {
	const int fd = open(path, O_RDONLY);
	if (fd == -1) {
		return -1;
	}
	const int result = fstat(fd, statp);
	close(fd);
	return result;
}

mode_t umask(mode_t) {
	printf("[libc] Warning, umask is not implemented.\n");
	return 0;
}
