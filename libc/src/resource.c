#include <sys/resource.h>

#include <stdio.h>
#define UNIMP()			printf("[libc] Warning: %s is not implemented.\n" ,__FUNCTION__);

int getrlimit(int, struct rlimit*) {
	UNIMP();
	return -1;
}

int setrlimit(int, const struct rlimit*) {
	UNIMP();
	return -1;
}
