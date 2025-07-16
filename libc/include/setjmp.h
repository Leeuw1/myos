#ifndef _SETJMP_H
#define _SETJMP_H

#include <stdint.h>

#define _longjmp(...)		longjmp(__VA_ARGS__)
#define _siglongjmp(...)	siglongjmp(__VA_ARGS__)
#define _setjmp(...)		setjmp(__VA_ARGS__)
#define _sigsetjmp(...)		sigsetjmp(__VA_ARGS__)

typedef uint64_t	jmp_buf[5];
typedef uint64_t	sigjmp_buf[5];

_Noreturn void longjmp(jmp_buf env, int val);
_Noreturn void siglongjmp(sigjmp_buf env, int val);
int setjmp(jmp_buf env);
int sigsetjmp(sigjmp_buf env, int savesigs);

#define longjmp(e, v)	__builtin_longjmp(e, v)
#define setjmp(e)		__builtin_setjmp(e)

#endif //_SETJMP_H
