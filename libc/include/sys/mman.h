#ifndef _SYS_MMAN_H
#define _SYS_MMAN_H

#include <stddef.h>

#define PROT_EXEC	0x1
#define PROT_NONE	0x2
#define PROT_READ	0x4
#define PROT_WRITE	0x8

int mprotect(void* addr, size_t len, int prot);

#endif //_SYS_MMAN_H
