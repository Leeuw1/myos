#ifndef _SYS_MMAN_H
#define _SYS_MMAN_H

#include <stddef.h>

#define PROT_EXEC	0x1
#define PROT_NONE	0x2
#define PROT_READ	0x4
#define PROT_WRITE	0x8

#define MAP_ANONYMOUS	0x1
#define MAP_ANON		MAP_ANONYMOUS
#define MAP_FIXED		0x2
#define MAP_PRIVATE		0x4
#define MAP_SHARED		0x8

#define MAP_FAILED		(void*)0

void* mmap(void*, size_t, int, int, int, off_t);
int mprotect(void* addr, size_t len, int prot);
int munmap(void*, size_t);

#endif //_SYS_MMAN_H
