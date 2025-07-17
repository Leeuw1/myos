#include <dlfcn.h>
#include <stdio.h>
#define UNIMP()			printf("[libc] Warning: %s is not implemented.\n", __FUNCTION__);

int dlclose(void* handle) {
	(void)handle;
	UNIMP();
	return -1;
}

void* dlopen(const char* file, int mode) {
	(void)file; (void)mode;
	UNIMP();
	return NULL;
}

void* dlsym(void* restrict handle, const char* restrict name) {
	(void)handle; (void)name;
	UNIMP();
	return NULL;
}
