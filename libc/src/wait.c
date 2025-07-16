#include <sys/wait.h>
#define MYOS_NO_BOOL
#include "../../src/syscall.h"

//pid_t wait(int* wstatus);
//int waitid(idtype_t idtype, id_t id, siginfo_t* infop, int options);

int _syscall_waitpid(pid_t pid) {
	_syscall_1arg(SYSCALL_WAITPID, int, pid);
}

pid_t waitpid(pid_t pid, int* wstatus, int options) {
	(void)options;
	*wstatus = _syscall_waitpid(pid);
	return pid;
}
