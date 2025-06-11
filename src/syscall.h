#ifndef _SYSCALL_H
#define _SYSCALL_H

#include "core.h"

#define str(x)			#x
#define _syscall_0arg(num, type)\
	register type _retval asm ("x0");\
	asm volatile (\
		"svc " str(num) "\n"\
		: "=r" (_retval)\
	);\
	return _retval
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

#define SYSCALL_COMMAND	1
#define SYSCALL_READ	2
#define SYSCALL_WRITE	3
#define SYSCALL_GETCWD	4
#define SYSCALL_CHDIR	5
#define SYSCALL_FORK	6
#define SYSCALL_EXECVE	7
#define SYSCALL_GETPID	8
#define SYSCALL_EXIT	9

union SyscallArgs {
	struct {
		union {
			i32		retval;
			u8		argc;
		} ALIGN(8);
		const char**	ALIGN(8) argv;
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
	struct {
		union {
			i16		retval;
		} ALIGN(8);
	} fork;
	struct {
		union {
			i32			retval;
			const char*	path;
		} ALIGN(8);
		const char**	ALIGN(8)	argv;
		const char**	ALIGN(8)	envp;
	} execve;
	struct {
		union {
			i16		retval;
		} ALIGN(8);
	} getpid;
};

i32 syscall_command(u8 argc, const char* argv[]);
isize syscall_read(i32 fd, void* buf, usize count);
isize syscall_write(i32 fd, const void* buf, usize count);

#endif //_SYSCALL_H
