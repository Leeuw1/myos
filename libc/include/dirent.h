#ifndef _DIRENT_H
#define _DIRENT_H

#include <stddef.h>

struct dirent {
	ino_t	d_ino;		// File serial number
	char	d_name[32];	// Filename string of entry
};

// TODO: in the future we could have d_name[] (variable length) and use d_reclen to get offset of next entry
struct posix_dent {
	ino_t			d_ino;		// File serial number.
	reclen_t		d_reclen;	// Length of this entry, including trailing
								// padding if necessary. See posix_getdents().
	unsigned char	d_type;		// File type or unknown-file-type indication.
	char			d_name[32];	// Filename string of this entry.
};

typedef struct _DIR	DIR;

int closedir(DIR* dir);
DIR* opendir(const char* path);
ssize_t posix_getdents(int fd, void* buf, size_t size, int flags);
struct dirent* readdir(DIR* dir);

#endif //_DIRENT_H
