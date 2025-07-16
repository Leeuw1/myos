#ifndef _STDLIB_H
#define _STDLIB_H

#include <stddef.h>

#define EXIT_FAILURE	1
#define EXIT_SUCCESS	0

#define RAND_MAX		0x7fff

_Noreturn void abort(void);
int abs(int x);
int atoi(const char* nptr);
long atol(const char* nptr);
void* bsearch(const void*, const void*, size_t, size_t, int (*)(const void*, const void*));
void* calloc(size_t n, size_t size);
_Noreturn void exit(int status);
void free(void* ptr);
char* getenv(const char* name);
long labs(long x);
int mkstemp(char* temp);
void* malloc(size_t size);
size_t mbstowcs(wchar_t* restrict dst, const char* restrict src, size_t dsize);
void qsort(void* base, size_t n, size_t size, int (* compare)(const void*, const void*));
int rand(void);
void* realloc(void* ptr, size_t size);
int setenv(const char* name, const char* value, int overwrite);
void srand(unsigned seed);
double strtod(const char* restrict nptr, char** restrict endptr);
float strtof(const char* restrict nptr, char** restrict endptr);
long strtol(const char* restrict nptr, char** restrict endptr, int);
unsigned long strtoul(const char* restrict nptr, char** restrict endptr, int);
int system(const char* command);

#endif //_STDLIB_H
