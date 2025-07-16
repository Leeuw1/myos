#ifndef _STDDEF_H
#define _STDDEF_H

#include <sys/types.h>

#define NULL			((void*)0)
#define offsetof(t, m)	__builtin_offsetof(t, m)

typedef __PTRDIFF_TYPE__	ptrdiff_t;

typedef __WCHAR_TYPE__		wchar_t;


#endif //_STDDEF_H
