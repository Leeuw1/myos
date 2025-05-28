#include "io.h"
#include "syscall.h"
#include <stdarg.h>

/*
char syscall_putchar(char c) {
	tty_putchar(c);
	screen_putchar(c);
	return c;
}
*/

char putchar(char c) {
	tty_putchar(c);
	// TEMPc
	//screen_putchar(c);
	return c;
}

void print(const char* str) {
	while (*str != 0) {
		putchar(*str);
		++str;
	}
}

void print_hex(u64 value) {
	char str[17];
	for (u64 i = 0; i < 16; ++i) {
		char c = (char)(value & 0xf);
		c += (c < 10) ? '0' : 'a' - 10;
		str[15 - i] = c;
		value >>= 4;
	}
	str[16] = '\0';
	print(str);
}

// NOTE: for now just using % (instead of %d or %x etc.)
// NOTE: for now all args should be u64
// TODO: % can be escaped using %%
void printf(const char* format, ...) {
	va_list args;
	va_start(args, format);

	for (; *format != '\0'; ++format) {
		if (*format != '%') {
			putchar(*format);
		}
		else {
			print_hex(va_arg(args, u64));
		}
	}
	va_end(args);
}
