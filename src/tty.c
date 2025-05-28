#include "io.h"
#include "regs.h"
#include "syscall.h"

char syscall_tty_getchar(void) {
	while (!(AUX_MU->stat_reg & 1)) {
	}
	return (char)AUX_MU->io_reg;
}

char tty_getchar(void) {
	//_syscall(SYSCALL_TTY_GETCHAR);
	return syscall_tty_getchar();
}

char tty_peekchar(void) {
	if (AUX_MU->stat_reg & 1) {
		return (char)AUX_MU->io_reg;
	}
	return 0;
}

void _tty_putchar(char c) {
	while (!(AUX_MU->stat_reg & 2)) {
	}
	AUX_MU->io_reg = (u32)c;
}

// Convert LF to CR-LF when printing to tty
void tty_putchar(char c) {
	if (c == '\n') {
		_tty_putchar('\r');
	}
	_tty_putchar(c);
}
