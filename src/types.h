#ifndef _TYPES_H
#define _TYPES_H

// Register types (Not currently being used)
#define reg32_r		const volatile u32
#define reg32_w		volatile u32
#define reg32_rw	volatile u32

// Basic Integer types
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
typedef u8					bool;

// Misc.
#define NULL				((void*)0)
#define true				1
#define false				0
#define PAGE_SIZE			4096
#define ALIGN(x)			__attribute__((aligned(x)))
#define BIT(x)				(1ull << (x))
#define PRINT_ERROR(msg)	print(__FUNCTION__);\
							print(": ");\
							print(msg);\
							print("\n")

#endif
