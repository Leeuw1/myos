#ifndef _SIGNAL_H
#define _SIGNAL_H

#include <sys/types.h>

#define SIG_DFL	(void*)0
#define SIG_IGN	(void*)1
#define SIG_ERR	(void*)2

#define SIGABRT		1
#define SIGALRM		2
#define SIGBUS		3
#define SIGCHLD		4
#define SIGCONT		5
#define SIGFPE		6
#define SIGHUP		7
#define SIGILL		8
#define SIGINT		9
#define SIGKILL		10
#define SIGPIPE		11
#define SIGQUIT		12
#define SIGSEGV		13
#define SIGSTOP		14
#define SIGTERM		15
#define SIGTSTP		16
#define SIGTTIN		17
#define SIGTTOU		18
#define SIGUSR1		19
#define SIGUSR2		20
#define SIGWINCH	21
#define SIGSYS		22
#define SIGTRAP		23
#define SIGURG		24
#define SIGVTALRM	25
#define SIGXCPU		26
#define SIGXFSZ		27

#define SIG_BLOCK	0
#define SIG_UNBLOCK	1
#define SIG_SETMASK	2

#define SA_SIGINFO	1

#define FPE_INTDIV	0
#define FPE_FLTDIV	1

#define SIGNAL_INDEX(sig)	(sig - 1)
#define VALID_SIGNAL(sig)	(sig >= SIGABRT && sig <= SIGXFSZ)
#define SIGNAL_COUNT		SIGXFSZ

typedef int					sig_atomic_t;
typedef unsigned int		sigset_t;

typedef struct siginfo_t	siginfo_t;

union sigval {
	int		sival_int;	// Integer signal value.
	void*	sival_ptr;	// Pointer signal value.
};

struct sigaction {
	union {
		void	(*sa_handler)(int);	// Pointer to a signal-catching function
									// or one of the SIG_IGN or SIG_DFL.
		void	(*sa_sigaction)(int, siginfo_t*, void*); // Pointer to a signal-catching function.
	};
	sigset_t	sa_mask;			// Set of signals to be blocked during execution
									// of the signal handling function.
	int			sa_flags;			// Special flags.
};

typedef struct mcontext_t	mcontext_t;
struct mcontext_t {
	uint64_t	regs[30];
	uint64_t	pc;
};

typedef struct stack_t	stack_t;
struct stack_t {
	void*	ss_sp;		// Stack base or pointer.
	size_t	ss_size;	// Stack size.
	int		ss_flags;	// Flags.
};

typedef struct ucontext_t	ucontext_t;
struct ucontext_t {
	ucontext_t*	uc_link;		// Pointer to the context that is resumed
								// when this context returns.
	sigset_t	uc_sigmask;		// The set of signals that are blocked when this
								// context is active.
	stack_t		uc_stack;		// The stack used by this context.
	mcontext_t	uc_mcontext;	// A machine-specific representation of the saved
								// context.
};

struct siginfo_t {
	int				si_signo;	// Signal number.
	int				si_code;	// Signal code.
	int				si_errno;	// If non-zero, an errno value associated with
								// this signal, as described in <errno.h>.
	pid_t			si_pid;		// Sending process ID.
	uid_t			si_uid;		// Real user ID of sending process.
	void*			si_addr;	// Address that caused fault.
	int				si_status;	// Exit value or signal.
	union sigval	si_value;	// Signal value.
};

int kill(pid_t pid, int sig);
int raise(int sig);
int sigaction(int sig, const struct sigaction* restrict act, struct sigaction* restrict oact);
int sigaddset(sigset_t* set, int sig);
int sigemptyset(sigset_t* set);
int sigismember(const sigset_t* set, int sig);
void (*signal(int sig, void (*func)(int)))(int);
int sigprocmask(int how, const sigset_t* restrict set, sigset_t* restrict oset);

#endif //_SIGNAL_H
