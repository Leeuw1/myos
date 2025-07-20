#include <signal.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#define MYOS_NO_BOOL
#include "../../src/syscall.h"

#include <stdio.h>
#define UNIMP()			printf("[libc] Warning: %s is not implemented.\n" ,__FUNCTION__);

typedef void (*_Handler1)(int);
typedef void (*_Handler2)(int, siginfo_t*, void*);

static struct sigaction _sigactions[SIGNAL_COUNT];

static _Noreturn void __attribute__((noinline)) _sigreturn(void) {
	_syscall_0arg_noreturn(SYSCALL_SIGRETURN);
	__builtin_unreachable();
}

static _Noreturn void _handler_wrapper(int sig, siginfo_t* info, void* context, void* handler) {
	(void)info; (void)context;
	if (info == NULL) {
		_Handler1 h = handler;
		h(sig);
	}
	else {
		_Handler2 h = handler;
		h(sig, info, context);
	}
	_sigreturn();
}

static void _sigsegv_handler(int sig, siginfo_t* info, void* context) {
	(void)sig; (void)context;
	fprintf(stderr, "[libc] Segmentation violation occurred (fault address: %p, instruction address: %p).\n",
			info->si_addr, info->si_value.sival_ptr);
	abort();
}

void _libc_init_signal(void) {
	memset(_sigactions, 0, sizeof _sigactions);
	// TODO: could also do SIGABRT
	struct sigaction action = {
		.sa_flags = SA_SIGINFO,
		.sa_sigaction = _sigsegv_handler,
	};
	sigaction(SIGSEGV, &action, NULL);
}

int kill(pid_t pid, int sig) {
	if (!VALID_SIGNAL(sig)) {
		errno = EINVAL;
		return -1;
	}
	UNIMP();
	return 0;
}

int raise(int sig) {
	if (!VALID_SIGNAL(sig)) {
		errno = EINVAL;
		return -1;
	}
	UNIMP();
	return 0;
}

static int __attribute__((noinline)) _sigaction(int sig, sigset_t mask, int flags, void* handler, void* wrapper) {
	_syscall_5arg(SYSCALL_SIGACTION, int, sig, mask, flags, handler, wrapper);
}

int sigaction(int sig, const struct sigaction* restrict act, struct sigaction* restrict oact) {
	if (!VALID_SIGNAL(sig)) {
		errno = EINVAL;
		return -1;
	}
	if (oact != NULL) {
		memcpy(oact, &_sigactions[SIGNAL_INDEX(sig)], sizeof *oact);
	}
	if (act == NULL) {
		return 0;
	}
	void* handler = (act->sa_flags & SA_SIGINFO) ? (void*)act->sa_sigaction : (void*)act->sa_handler;
	return _sigaction(sig, act->sa_mask, act->sa_flags, handler, _handler_wrapper);
}

int sigaddset(sigset_t* set, int sig) {
	if (!VALID_SIGNAL(sig)) {
		errno = EINVAL;
		return -1;
	}
	*set |= 1 << sig;
	return 0;
}

int sigemptyset(sigset_t* set) {
	*set = 0;
	return 0;
}

int sigismember(const sigset_t* set, int sig) {
	if (!VALID_SIGNAL(sig)) {
		errno = EINVAL;
		return -1;
	}
	return !!(*set & (1 << sig));
}

void (*signal(int sig, void (*func)(int)))(int) {
	if (!VALID_SIGNAL(sig)) {
		errno = EINVAL;
		return SIG_ERR;
	}
	return 0;
}

// TODO
int sigprocmask(int how, const sigset_t* restrict set, sigset_t* restrict oset) {
	//UNIMP();
	return 0;
}

int sigtimedwait(const sigset_t *restrict, siginfo_t *restrict, const struct timespec *restrict) {
	UNIMP();
	return -1;
}
