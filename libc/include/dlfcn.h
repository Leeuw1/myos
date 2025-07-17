#ifndef _DLFCN_H
#define _DLFCN_H

#define RTLD_LAZY	0x1
#define RTLD_NOW	0x2
#define RTLD_GLOBAL	0x4
#define RTLD_LOCAL	0x8

#define RTLD_DEFAULT	(void*)0
#define RTLD_NEXT		(void*)1

int dlclose(void* handle);
void* dlopen(const char* file, int mode);
void* dlsym(void* restrict handle, const char* restrict name);

#endif //_DLFCN_H
