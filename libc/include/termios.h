#ifndef _TERMIOS_H
#define _TERMIOS_H

#include <stdint.h>

#define VEOF	0
#define VEOL	1
#define VERASE	2
#define VINTR	3
#define VKILL	4
#define VMIN	5
#define VQUIT	6
#define VSTART	7
#define VSTOP	8
#define VSUSP	9
#define VTIME	10
#define NCCS	11

#define BRKINT	0x001
#define ICRNL	0x002
#define IGNBRK	0x004
#define IGNCR	0x008
#define IGNPAR	0x010
#define INLCR	0x020
#define INPCK	0x040
#define ISTRIP	0x080
#define IXANY	0x100
#define IXOFF	0x200
#define IXON	0x400
#define PARMRK	0x800

#define OPOST	0x001
#define ONLCR	0x002
#define OCRNL	0x004
#define ONOCR	0x008
#define ONLRET	0x010
#define OFDEL	0x020
#define OFILL	0x040
// TODO
// ...

#define CSIZE	0x00f
#define CS5		0x001
#define CS6		0x002
#define CS7		0x004
#define CS8		0x008
#define CSTOPB	0x010
#define CREAD	0x020
#define	PARENB	0x040
#define PARODD	0x080
#define CLOCAL	0x100

#define ECHO	0x001
#define ECHOE	0x002
#define ECHOK	0x004
#define ECHONL	0x008
#define	ICANON	0x010
#define IEXTEN	0x020
#define ISIG	0x040
#define NOFLSH	0x080
#define TOSTOP	0x100

#define TCSANOW		0
#define TCSADRAIN	1
#define TCSAFLUSH	2

#define TCIOFF	0
#define TCION	1
#define TCOOFF	2
#define TCOON	3

typedef uint8_t		cc_t;
typedef uint16_t	speed_t;
typedef uint16_t	tcflag_t;

struct termios {
	tcflag_t	c_iflag;	// Input modes.
	tcflag_t	c_oflag;	// Output modes.
	tcflag_t	c_cflag;	// Control modes.
	tcflag_t	c_lflag;	// Local modes.
	cc_t		c_cc[NCCS];	// Control characters.
};

struct winsize {
	unsigned short	ws_row;
	unsigned short	ws_col;
	unsigned short	ws_xpixel;
	unsigned short	ws_ypixel;
};

int tcgetattr(int fd, struct termios* termios_p);
int tcsetattr(int fd, int optional_actions, struct termios* termios_p);
int tcflow(int, int);

#endif //_TERMIOS_H
