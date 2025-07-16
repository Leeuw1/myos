#ifndef _SYS_TYPES_H
#define _SYS_TYPES_H

#include <stdint.h>

typedef uint64_t	size_t;
typedef int64_t		ssize_t;

// TODO
typedef signed short		blkcnt_t;
typedef ssize_t				blksize_t;
typedef uint64_t			clock_t;
typedef void				clockid_t;
typedef unsigned long		dev_t;
typedef unsigned short		fsblkcnt_t;
typedef unsigned short		fsfilcnt_t;
typedef unsigned long		gid_t;
typedef unsigned long		id_t;
typedef uint32_t			ino_t;
typedef void				key_t;
typedef uint16_t			mode_t;
typedef unsigned short		nlink_t;
typedef ssize_t				off_t;
typedef int16_t				pid_t;
typedef void				pthread_attr_t;
typedef void				pthread_barrier_t;
typedef void				pthread_barrierattr_t;
typedef void				pthread_cond_t;
typedef void				pthread_condattr_t;
typedef void				pthread_key_t;
typedef void				pthread_mutex_t;
typedef void				pthread_mutexattr_t;
typedef void				pthread_once_t;
typedef void				pthread_rwlock_t;
typedef void				pthread_rwlockattr_t;
typedef void				pthread_spinlock_t;
typedef void				pthread_t;
typedef uint8_t				reclen_t;
typedef uint64_t			suseconds_t;
typedef int64_t				time_t;
typedef void				timer_t;
typedef unsigned long		uid_t;

#endif //_SYS_TYPES_H
