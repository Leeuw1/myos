#ifndef _FCNTL_H
#define _FCNTL_H

#include <sys/types.h>

// File creation flags
#define O_CLOEXEC	0x1
#define O_CLOFORK	0x2
#define O_CREAT		0x4
#define O_DIRECTORY	0x8
#define O_EXCL		0x10
#define O_NOCTTY	0x20
#define O_NOFOLLOW	0x40
#define O_TRUNC		0x80
#define O_TTY_INIT	0

// Status flags
#define O_APPEND	0x1
#define O_DSYNC		0x2
#define O_NONBLOCK	0x4
#define O_RSYNC		0x8
#define O_SYNC		0x10

// Mask
#define O_ACCMODE	0x7
// Access modes
#define O_EXEC		0x1
#define O_RDONLY	0x0
#define O_RDWR		0x2
#define O_SEARCH	0x3
#define O_WRONLY	0x4

#define F_SETFD		1
#define F_SETFL		2
#define F_GETFD		3
#define F_GETFL		4

#define F_SETLKW	5

#define F_RDLCK		0
#define F_UNLCK		1
#define F_WRLCK		2

struct flock {
	short	l_type;		// Type of lock; F_RDLCK, F_WRLCK, F_UNLCK.
	short	l_whence;	// Flag for starting offset.
	off_t	l_start;	// Relative offset in bytes.
	off_t	l_len;		// Size; if 0 then until EOF.
	pid_t	l_pid;		// For a process-owned file lock, ignored on input or the process ID of the
						// owning process on output; for an OFD-owned file lock, zero on input or
						// (pid_t)-1 on output.
};

int creat(const char* path, mode_t mode);
int fcntl(int fd, int cmd, ...);
int open(const char* path, int flags, ...);
int openat(int fd, const char* path, int oflag, ...);
int posix_fadvise(int fd, off_t offset, off_t length, int advice);
int posix_fallocate(int fd, off_t offset, off_t size);

int ioctl(int, int, ...);

#endif //_FCNTL_H
