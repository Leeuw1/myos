#ifndef _STDINT_H
#define _STDINT_H

#define UINT8_MAX	__UINT8_MAX__
#define UINT16_MAX	__UINT16_MAX__
#define UINT32_MAX	__UINT32_MAX__
#define UINT64_MAX	__UINT64_MAX__
#define SIZE_MAX	__SIZE_MAX__

typedef unsigned char	uint8_t;
typedef unsigned short	uint16_t;
typedef unsigned int	uint32_t;
typedef unsigned long	uint64_t;
typedef signed char		int8_t;
typedef signed short	int16_t;
typedef signed int		int32_t;
typedef signed long		int64_t;

typedef __UINTPTR_TYPE__	uintptr_t;

typedef uint64_t	uintmax_t;
typedef int64_t		intmax_t;

_Static_assert(sizeof(uint8_t) == 1, "sizeof(uint8_t) must be 1");
_Static_assert(sizeof(uint16_t) == 2, "sizeof(uint16_t) must be 2");
_Static_assert(sizeof(uint32_t) == 4, "sizeof(uint32_t) must be 4");
_Static_assert(sizeof(uint64_t) == 8, "sizeof(uint64_t) must be 8");

_Static_assert(sizeof(int8_t) == 1, "sizeof(int8_t) must be 1");
_Static_assert(sizeof(int16_t) == 2, "sizeof(int16_t) must be 2");
_Static_assert(sizeof(int32_t) == 4, "sizeof(int32_t) must be 4");
_Static_assert(sizeof(int64_t) == 8, "sizeof(int64_t) must be 8");

#endif
