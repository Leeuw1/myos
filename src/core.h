#ifndef _CORE_H
#define _CORE_H

typedef unsigned long long	u64;
typedef unsigned int		u32;
typedef unsigned short		u16;
typedef unsigned char		u8;
typedef signed long long	i64;
typedef signed int			i32;
typedef signed short		i16;
typedef signed char			i8;

typedef	u64					usize;
typedef i64					isize;

_Static_assert(sizeof(u64) == 8, "sizeof(u64) must be 8");
_Static_assert(sizeof(u32) == 4, "sizeof(u32) must be 4");
_Static_assert(sizeof(u16) == 2, "sizeof(u16) must be 2");
_Static_assert(sizeof(u8) == 1, "sizeof(u8) must be 1");
_Static_assert(sizeof(i64) == 8, "sizeof(i64) must be 8");
_Static_assert(sizeof(i32) == 4, "sizeof(i32) must be 4");
_Static_assert(sizeof(i16) == 2, "sizeof(i16) must be 2");
_Static_assert(sizeof(i8) == 1, "sizeof(i8) must be 1");

#ifndef MYOS_NO_BOOL
#define bool	u8
#endif

// Misc.
#define NULL				((void*)0)
#define true				1
#define false				0
#define PAGE_SIZE			4096
#define ALIGN(x)			__attribute__((aligned(x)))
#define BIT(x)				(1ull << (x))
#define MAX(x, y)			(x > y ? x : y)
#define MIN(x, y)			(x < y ? x : y)
#define PRINT_ERROR(msg)	print(__FUNCTION__);\
							print(": ");\
							print(msg);\
							print("\n")

#endif //_CORE_H
