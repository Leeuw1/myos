#ifndef _PROC_H
#define _PROC_H

#include "core.h"
#include <sys/stat.h>

struct FSNode;
struct termios;

struct Regs {
	u64	general[31];
	u64	sp;
	u64	spsr;
	u64	elr;
	u64	tpidrro;
};

_Noreturn void proc_main(void);
_Noreturn void proc_run_next(void);
const char* proc_fd_path(i32 fd);

i32 proc_open(const char* path, i32 flags);
i32 proc_close(i32 fd);
char* proc_getcwd(char* buf, usize size);
i32 proc_chdir(const char* path);
i16 proc_fork(void);
i32 proc_execve(const char* path, const char* argv[], const char* envp[]);
_Noreturn void proc_exit(i32 status);
i16 proc_getpid(void);
i32 proc_waitpid(i16 pid);
_Noreturn void proc_queue_read(struct FSNode* node, void* dst, usize size);
_Noreturn void proc_update_pending_io(void);
i32 proc_rename(const char* old_path, const char* new_path);
i32 proc_unlink(const char* path);
i32 proc_mkdir(const char* path, u16 mode);
i32 proc_rmdir(const char* path);
isize proc_read(i32 fd, void* buf, usize count);
isize proc_write(i32 fd, const void* buf, usize count);
isize proc_getdents(i32 fd, void* buf, usize size, i32 flags);
i32 proc_fstat(i32 fd, struct stat* buf);
i32 proc_tcgetattr(i32 fd, struct termios* termios_p);
i32 proc_tcsetattr(i32 fd, i32 optional_actions, const struct termios* termios_p);
void proc_send_signal(i32 sig, const siginfo_t* info);
i32 proc_nanosleep(const struct timespec* rqtp, struct timespec* rmtp);
isize proc_lseek(i32 fd, isize offset, i32 whence);
i32 proc_sigaction(i32 sig, sigset_t mask, i32 flags, void* handler, void* wrapper);
_Noreturn void proc_sigreturn(void);
void proc_grow_heap(usize size);
i32 proc_canonicalize(const char* restrict path, char* restrict dst);

#endif //_PROC_H
