#ifndef _PROC_H
#define _PROC_H

#include "core.h"

struct Regs {
	u64	general[31];
	u64	sp;
	u64	spsr;
	u64	elr;
	u64	ttbr;
};

_Noreturn void proc_main(void);
_Noreturn void proc_run_next(void);
const char* proc_fd_path(i32 fd);

char* proc_getcwd(char* buf, usize size);
int proc_chdir(const char* path);
i16 proc_fork(void);
i32 proc_execve(const char* path, const char* argv[], const char* envp[]);
_Noreturn void proc_kill(void);
i16 proc_getpid(void);
_Noreturn void proc_queue_read(i32 fd, void* dst, usize size);
_Noreturn void proc_update_pending_io(i32 fd);

#endif //_PROC_H
