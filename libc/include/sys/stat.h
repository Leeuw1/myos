#ifndef _SYS_STAT_H
#define _SYS_STAT_H

#include <time.h>

// NOTE: assuming this is a mask for the filetype bits
#define	S_IFMT		070000
// TODO: use higher bits
#define	S_IFBLK		010000
#define	S_IFCHR		020000
#define	S_IFIFO		030000
#define	S_IFREG		040000
#define	S_IFDIR		050000
#define	S_IFLNK		060000
#define	S_IFSOCK	070000

#define S_ISBLK(m)	(((m) & S_IFMT) == S_IFBLK)
#define S_ISCHR(m)	(((m) & S_IFMT) == S_IFCHR)
#define S_ISFIFO(m)	(((m) & S_IFMT) == S_IFIFO)
#define S_ISREG(m)	(((m) & S_IFMT) == S_IFREG)
#define S_ISDIR(m)	(((m) & S_IFMT) == S_IFDIR)
#define S_ISLNK(m)	(((m) & S_IFMT) == S_IFLNK)
#define S_ISSOC(m)	(((m) & S_IFMT) == S_IFSOC)

#define S_IRWXU		0700
#define S_IRUSR		0400
#define S_IWUSR		0200
#define S_IXUSR		0100
#define S_IRWXG		070
#define S_IRGRP		040
#define S_IWGRP		020
#define S_IXGRP		010
#define S_IRWXO		07
#define S_IROTH		04
#define S_IWOTH		02
#define S_IXOTH		01
#define S_ISUID		04000
#define S_ISGID		02000
#define S_ISVTX		01000

struct stat {
	dev_t st_dev;        // Device ID of device containing file.
	ino_t st_ino;        // File serial number.
	mode_t st_mode;      // Mode of file (see below).
	nlink_t st_nlink;    // Number of hard links to the file.
	uid_t st_uid;        // User ID of file.
	gid_t st_gid;        // Group ID of file.
	dev_t st_rdev;       // Device ID (if file is character or block special).
	off_t st_size;       // For regular files, the file size in bytes.
						 // For symbolic links, the length in bytes of the
						 // pathname contained in the symbolic link.
						 // For a shared memory object, the length in bytes.
						 // For a typed memory object, the length in bytes.
						 // For other file types, the use of this field is
						 // unspecified.
	struct timespec st_atim; // Last data access timestamp.
	struct timespec st_mtim; // Last data modification timestamp.
	struct timespec st_ctim; // Last file status change timestamp.
	blksize_t st_blksize;    // A file system-specific preferred I/O block size
							 // for this object. In some file system types, this
							 // may vary from file to file.
	blkcnt_t st_blocks;      // Number of blocks allocated for this object.
};

// TODO: implement
#if 0
int    chmod(const char *, mode_t);
int    fchmod(int, mode_t);
int    fchmodat(int, const char *, mode_t, int);
int    fstat(int, struct stat *);
int    fstatat(int, const char *restrict, struct stat *restrict, int);
int    futimens(int, const struct timespec [2]);
int    lstat(const char *restrict, struct stat *restrict);
int    mkdir(const char *, mode_t);
int    mkdirat(int, const char *, mode_t);
int    mkfifo(const char *, mode_t);
int    mkfifoat(int, const char *, mode_t);
int    mknod(const char *, mode_t, dev_t);
int    mknodat(int, const char *, mode_t, dev_t);
int    stat(const char *restrict, struct stat *restrict);
mode_t umask(mode_t);
int    utimensat(int, const char *, const struct timespec [2], int);
#endif

#endif //_SYS_STAT_H
