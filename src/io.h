#ifndef _IO_H
#define _IO_H

#include "types.h"

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

char tty_getchar(void);
char tty_peekchar(void);
void tty_putchar(char c);

char putchar(char c);
void print(const char* str);
void print_hex(u64 value);
void printf(const char* format, ...);

#endif // _IO_H
