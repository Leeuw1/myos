#ifndef _IO_H
#define _IO_H

#include "core.h"

struct termios;

struct Pixel {
	u8	r;
	u8	g;
	u8	b;
} __attribute__((packed));

struct Framebuffer {
	u32 width;
	u32 height;
	u32 physical_width;
	u32 physical_height;
	void* address;
	u32 size;
	u32 pitch;
};

extern struct Framebuffer framebuffer;
extern u16 row;
extern u16 column;

void screen_putchar(char c);
void screen_clear(u8 r, u8 g, u8 b);

void tty_init(void);
bool tty_canonical_mode(void);
char tty_getchar(void);
void tty_putchar(char c);
bool tty_line_add_char(char c);
usize tty_flush_line(void* dst, usize size);
void tty_tcgetattr(struct termios* termios_p);
void tty_tcsetattr(i32 optional_actions, const struct termios* termios_p);
bool tty_read_would_block(void);

char putchar(char c);
void print(const char* str);
void print_hex(u64 value);
void printf(const char* format, ...);

#endif // _IO_H
