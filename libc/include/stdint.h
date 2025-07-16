#ifndef _STDINT_H
#define _STDINT_H

#define UINT8_MAX	__UINT8_MAX__
#define UINT16_MAX	__UINT16_MAX__
#define UINT32_MAX	__UINT32_MAX__
#define UINT64_MAX	__UINT64_MAX__
#define SIZE_MAX	__SIZE_MAX__

typedef __UINT8_TYPE__	uint8_t;
typedef __UINT16_TYPE__	uint16_t;
typedef __UINT32_TYPE__	uint32_t;
typedef __UINT64_TYPE__	uint64_t;
typedef __INT8_TYPE__	int8_t;
typedef __INT16_TYPE__	int16_t;
typedef __INT32_TYPE__	int32_t;
typedef __INT64_TYPE__	int64_t;

typedef __UINTPTR_TYPE__	uintptr_t;

typedef __UINT64_TYPE__	uintmax_t;
typedef __INT64_TYPE__	intmax_t;

#endif
