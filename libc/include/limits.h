#ifndef _LIMITS_H
#define _LIMITS_H

#define CHAR_BIT	__CHAR_BIT__

#define ULLONG_MAX	__UINT64_MAX__
#define LLONG_MAX	__INT64_MAX__
#define LLONG_MIN	(-LLONG_MAX - 1ll)
#define ULONG_MAX	__UINT64_MAX__
#define LONG_MAX	__INT64_MAX__
#define LONG_MIN	(-LONG_MAX - 1l)
#define UINT_MAX	__UINT32_MAX__
#define INT_MAX		__INT32_MAX__
#define INT_MIN		(-INT_MAX - 1)
#define USHRT_MAX	__UINT16_MAX__
#define SHRT_MAX	__UINT16_MAX__
#define UCHAR_MAX	__UINT8_MAX__
#define CHAR_MAX	__INT8_MAX__

//#define OPEN_MAX TODO

#define PAGE_SIZE	4096
#define PAGESIZE	PAGE_SIZE

#define MAX_INPUT	255
#define NAME_MAX	31
#define PATH_MAX	256

#endif //_LIMITS_H
