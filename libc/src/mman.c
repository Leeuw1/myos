#include <sys/mman.h>
#include <stdio.h>
#define UNIMP()			printf("[libc] Warning: %s is not implemented.\n" ,__FUNCTION__);

int mprotect(void* addr, size_t len, int prot) {
	UNIMP();
	return -1;
}
