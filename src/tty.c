#include "io.h"
#include "regs.h"
#include <string.h>
#include <termios.h>

#define MAX_LINE_LENGTH		256

static struct termios _term;
static usize _line_length;
static char _line[MAX_LINE_LENGTH];

void tty_init(void) {
	memset(&_term, 0, sizeof _term);
	_term.c_iflag = ICRNL;
	_term.c_oflag = ONLCR;
	_term.c_cflag = CS8;
	_term.c_lflag = ECHO | ICANON;
	//_term.c_cc[VEOF] = 0x4;
	//_term.c_cc[VEOL] = '\n';
	//_term.c_cc[VERASE] = '\b';
	_line_length = 0;
}

bool tty_canonical_mode(void) {
	return _term.c_lflag & ICANON;
}

static char _tty_translate(char c) {
	if (c == '\r' && (_term.c_iflag & ICRNL)) {
		return '\n';
	}
	if (c == 0x7f) {
		return '\b';
	}
	return c;
}

char tty_getchar(void) {
	if (!(AUX_MU->stat_reg & 1)) {
		return 0;
	}
	const char c = _tty_translate((char)AUX_MU->io_reg);
	if (c == '\n' && (_term.c_lflag & ECHONL)) {
		tty_putchar(c);
	}
	else if (c == '\e' && (_term.c_lflag & ECHO)) {
		tty_putchar('^');
		tty_putchar('[');
	}
	else if (c != '\b' && (_term.c_lflag & ECHO)) {
		tty_putchar(c);
	}
	return c;
}

static void _tty_putchar(char c) {
	while (!(AUX_MU->stat_reg & 2)) {
	}
	AUX_MU->io_reg = (u32)c;
}

void tty_putchar(char c) {
	if (c == '\n' && (_term.c_oflag & ONLCR)) {
		_tty_putchar('\r');
		_tty_putchar('\n');
		return;
	}
	if (c == '\r' && (_term.c_oflag & OCRNL)) {
		_tty_putchar('\n');
		return;
	}
	_tty_putchar(c);
}

// Return true if tty_flush_line() should be called
bool tty_line_add_char(char c) {
	if (_line_length == MAX_LINE_LENGTH) {
		return true;
	}
	if (c == '\b' && (_term.c_lflag & ICANON)) {
		if (_line_length == 0) {
			return false;
		}
		--_line_length;
		_tty_putchar('\b');
		_tty_putchar(' ');
		_tty_putchar('\b');
		return false;
	}
	_line[_line_length++]= c;
	return c == '\n' || !(_term.c_lflag & ICANON);
}

usize tty_flush_line(void* dst, usize size) {
	const usize read_size = MIN(size, _line_length);
	memcpy(dst, _line, read_size);
	_line_length = 0;
	return read_size;
}

void tty_tcgetattr(struct termios* termios_p) {
	memcpy(termios_p, &_term, sizeof _term);
}

void tty_tcsetattr(i32 optional_actions, const struct termios* termios_p) {
	(void)optional_actions;
	memcpy(&_term, termios_p, sizeof _term);
}

bool tty_read_would_block(void) {
	return !(AUX_MU->stat_reg & 1);
}
