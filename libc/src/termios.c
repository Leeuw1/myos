#include <termios.h>

#define MYOS_NO_BOOL
#include "../../src/syscall.h"

#include <stdio.h>
#define UNIMP()			printf("[libc] Warning: %s is not implemented.\n" ,__FUNCTION__);

int tcgetattr(int fd, struct termios* termios_p) {
	_syscall_2arg(SYSCALL_TCGETATTR, int, fd, termios_p);
}

int tcsetattr(int fd, int optional_actions, struct termios* termios_p) {
	_syscall_3arg(SYSCALL_TCSETATTR, int, fd, optional_actions, termios_p);
}

int tcflow(int, int) {
	UNIMP();
	return 0;
}
