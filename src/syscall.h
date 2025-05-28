#ifndef _SYSCALL_H
#define _SYSCALL_H

#include "types.h"

#define str(x)			#x
#define _syscall_2arg(num, type, arg1, arg2)\
	register type _retval asm ("x0");\
	asm volatile (\
			"svc " str(num) "\n"\
			: "=r" (_retval)\
			: "r" (arg1), "r" (arg2)\
			);\
	return _retval
#define _syscall_3arg(num, type, arg1, arg2, arg3)\
	register type _retval asm ("x0");\
	asm volatile (\
		 "svc " str(num) "\n"\
		 : "=r" (_retval)\
		 : "r" (arg1), "r" (arg2), "r" (arg3)\
	);\
	return _retval

#define SYSCALL_COMMAND	0
#define SYSCALL_READ	1
#define SYSCALL_WRITE	2
#define SYSCALL_GETCWD	3
#define SYSCALL_CHDIR	4

union SyscallArgs {
	struct {
		union {
			i32		retval;
			char*	cmd;
		} ALIGN(8);
		u8	ALIGN(8) len;
	} command;
	struct {
		union {
			isize	retval;
			i32		fd;
		} ALIGN(8);
		void*	ALIGN(8) buf;
		usize	ALIGN(8) count;
	} read, write;
	struct {
		union {
			char*	retval;
			char*	buf;
		} ALIGN(8);
		usize	ALIGN(8)	size;
	} getcwd;
	struct {
		union {
			int		retval;
			char*	path;
		} ALIGN(8);
	} chdir;
};

// NOTE: Some syscalls are not listed here because they are implemented entirely by some other module e.g. proc_getcwd()
// TODO: we should actually put all syscalls in syscall.c because any pointer arguments need to checked using address translation
i32 syscall_command(char* cmd, u8 len);
isize syscall_read(i32 fd, void* buf, usize count);
isize syscall_write(i32 fd, const void* buf, usize count);

#endif //_SYSCALL_H
