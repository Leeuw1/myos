#ifndef _CORE_H
#define _CORE_H

#include "myos_inttypes.h"

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

#endif //_CORE_H
