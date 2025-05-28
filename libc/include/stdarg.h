#ifndef _STDARG_H
#define _STDARG_H

#define va_start(list, argn)	__builtin_va_start(list, argn)
#define va_copy(dst, src)		__builtin_va_copy(dst, src)
#define va_arg(list, type)		__builtin_va_arg(list, type)
#define va_end(list)			__builtin_va_end(list)

typedef __builtin_va_list va_list;

#endif //_STDARG_H
