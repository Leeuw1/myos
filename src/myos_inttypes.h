#ifndef _MYOS_INTTYPES_H
#define _MYOS_INTTYPES_H

typedef unsigned long long	u64;
typedef unsigned int		u32;
typedef unsigned short		u16;
typedef unsigned char		u8;
typedef signed long long	i64;
typedef signed int			i32;
typedef signed short		i16;
typedef signed char			i8;

typedef	u32					usize;
typedef i32					isize;

_Static_assert(sizeof(u64) == 8, "sizeof(u64) must be 8");
_Static_assert(sizeof(u32) == 4, "sizeof(u32) must be 4");
_Static_assert(sizeof(u16) == 2, "sizeof(u16) must be 2");
_Static_assert(sizeof(u8) == 1, "sizeof(u8) must be 1");
_Static_assert(sizeof(i64) == 8, "sizeof(i64) must be 8");
_Static_assert(sizeof(i32) == 4, "sizeof(i32) must be 4");
_Static_assert(sizeof(i16) == 2, "sizeof(i16) must be 2");
_Static_assert(sizeof(i8) == 1, "sizeof(i8) must be 1");

#endif //_MYOS_INTTYPES_H
