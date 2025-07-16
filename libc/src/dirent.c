#include <dirent.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#define MYOS_NO_BOOL
#include "../../src/syscall.h"

struct _DIR {
	int				fd;
	struct dirent	entry;
};

int closedir(DIR* dir) {
	const int result = close(dir->fd);
	free(dir);
	return result;
}

DIR* opendir(const char* path) {
	const int fd = open(path, O_RDONLY);
	if (fd == -1) {
		return NULL;
	}
	DIR* dir = malloc(sizeof *dir);
	dir->fd = fd;
	return dir;
};

ssize_t __attribute__((noinline)) posix_getdents(int fd, void* buf, size_t size, int flags) {
	_syscall_4arg(SYSCALL_POSIX_GETDENTS, ssize_t, fd, buf, size, flags);
}

struct dirent* readdir(DIR* dir) {
	struct posix_dent dent = {};
	ssize_t size = posix_getdents(dir->fd, &dent, sizeof dent, 0);
	if (size <= 0) {
		return NULL;
	}
	dir->entry.d_ino = dent.d_ino;
	strcpy(dir->entry.d_name, dent.d_name);
	return &dir->entry;
}
