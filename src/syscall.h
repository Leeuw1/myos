#ifndef _SYSCALL_H
#define _SYSCALL_H

#include "core.h"
#include <signal.h>

#define str(x)			#x

#define _syscall_0arg_noreturn(num, ...)\
	asm volatile (\
		"svc " str(num) "\n"\
		: __VA_ARGS__\
	);
#define _syscall_1arg_noreturn(num, arg1, ...)\
	asm volatile (\
		"svc " str(num) "\n"\
		: __VA_ARGS__\
		: "r" (arg1)\
	);

#define _syscall_0arg(num, type)\
	register type _retval asm ("x0");\
	_syscall_0arg_noreturn(num, "=r" (_retval))\
	return _retval
#define _syscall_1arg(num, type, arg1)\
	register type _retval asm ("x0");\
	_syscall_1arg_noreturn(num, arg1, "=r" (_retval))\
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
#define _syscall_4arg(num, type, arg1, arg2, arg3, arg4)\
	register type _retval asm ("x0");\
	asm volatile (\
		 "svc " str(num) "\n"\
		 : "=r" (_retval)\
		 : "r" (arg1), "r" (arg2), "r" (arg3), "r" (arg4)\
	);\
	return _retval
#define _syscall_5arg(num, type, arg1, arg2, arg3, arg4, arg5)\
	register type _retval asm ("x0");\
	asm volatile (\
		 "svc " str(num) "\n"\
		 : "=r" (_retval)\
		 : "r" (arg1), "r" (arg2), "r" (arg3), "r" (arg4), "r" (arg5)\
	);\
	return _retval

#define SYSCALL_READ			2
#define SYSCALL_WRITE			3
#define SYSCALL_GETCWD			4
#define SYSCALL_CHDIR			5
#define SYSCALL_FORK			6
#define SYSCALL_EXECVE			7
#define SYSCALL_GETPID			8
#define SYSCALL_EXIT			9
#define SYSCALL_OPEN			10
#define SYSCALL_WAITPID			11
#define SYSCALL_UNLINK			12
#define SYSCALL_RMDIR			13
#define SYSCALL_TIME			14
#define SYSCALL_POSIX_GETDENTS	15
#define SYSCALL_CLOSE			16
#define SYSCALL_RENAME			17
#define SYSCALL_MKDIR			18
#define SYSCALL_FSTAT			19
#define SYSCALL_TCGETATTR		20
#define SYSCALL_TCSETATTR		21
#define SYSCALL_NANOSLEEP		22
#define SYSCALL_LSEEK			23
#define SYSCALL_SIGACTION		24
#define SYSCALL_SIGRETURN		25
#define SYSCALL_GROW_HEAP		26
#define SYSCALL_CANONICALIZE	27

union SyscallArgs {
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
			int			retval;
			const char*	path;
		} ALIGN(8);
	} chdir, unlink, rmdir;
	struct {
		i16	ALIGN(8)	retval;
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
		i16	ALIGN(8)	retval;
	} getpid;
	struct {
		i32	ALIGN(8)	status;
	} exit;
	struct {
		union {
			i32			retval;
			const char*	path;
		} ALIGN(8);
		i32 ALIGN(8)	flags;
	} open;
	struct {
		union {
			i32	retval;
			i16	pid;
		} ALIGN(8);
	} waitpid;
	struct {
		i64 ALIGN(8)	retval;
	} time;
	struct {
		union {
			isize	retval;
			i32		fd;
		} ALIGN(8);
		void*	ALIGN(8)	buf;
		usize	ALIGN(8)	size;
		i32		ALIGN(8)	flags;
	} posix_getdents;
	struct {
		union {
			i32	retval;
			i32	fd;
		} ALIGN(8);
	} close;
	struct {
		union {
			i32			retval;
			const char*	oldpath;
		} ALIGN(8);
		const char*	ALIGN(8)	newpath;
	} rename;
	struct {
		union {
			i32			retval;
			const char*	path;
		} ALIGN(8);
		u16	ALIGN(8)	mode;
	} mkdir;
	struct {
		union {
			i32	retval;
			i32	fd;
		} ALIGN(8);
		struct stat*	ALIGN(8)	buf;
	} fstat;
	struct {
		union {
			i32	retval;
			i32	fd;
		} ALIGN(8);
		struct termios*	ALIGN(8)	buf;
	} tcgetattr;
	struct {
		union {
			i32	retval;
			i32	fd;
		} ALIGN(8);
		int						ALIGN(8)	opt;
		const struct termios*	ALIGN(8)	buf;
	} tcsetattr;
	struct {
		union {
			i32						retval;
			const struct timespec*	rqtp;
		} ALIGN(8);
		struct timespec*	ALIGN(8)	rmtp;
	} nanosleep;
	struct {
		union {
			i32	retval;
			i32	fd;
		} ALIGN(8);
		isize	ALIGN(8)	offset;
		i32		ALIGN(8)	whence;
	} lseek;
	struct {
		union {
			i32	retval;
			i32	sig;
		} ALIGN(8);
		sigset_t	ALIGN(8)	mask;
		i32			ALIGN(8)	flags;
		void*		ALIGN(8)	handler;
		void*		ALIGN(8)	wrapper;
	} sigaction;
	struct {
		usize	ALIGN(8)	size;
	} grow_heap;
	struct {
		union {
			i32			retval;
			const char*	path;
		} ALIGN(8);
		char*	ALIGN(8)	dst;
	} canonicalize;
};

u64 syscall_time(void);

#endif //_SYSCALL_H
