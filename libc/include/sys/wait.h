#ifndef _SYS_WAIT_H
#define _SYS_WAIT_H

#include <sys/types.h>
#include <signal.h>

// TODO
#define WEXITSTATUS(x)	(x & 0xf)
#define WIFEXITED(x)	1
#define WIFSIGNALED(x)	1
#define WTERMSIG(x)		727

#define WEXITED		1
#define WNOWAIT		2
#define WSTOPPED	3
#define WNOHANG		4

typedef enum idtype_t	idtype_t;
enum idtype_t {
	P_ALL,
	P_PGID,
	P_PID,
};

pid_t wait(int* wstatus);
int waitid(idtype_t idtype, id_t id, siginfo_t* infop, int options);
pid_t waitpid(pid_t pid, int* wstatus, int options);

#endif //_SYS_WAIT_H
