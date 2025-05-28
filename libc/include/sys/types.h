#ifndef _SYS_TYPES_H
#define _SYS_TYPES_H

typedef unsigned long		size_t;
#define _SIZE_T_DEFINED
typedef signed long			ssize_t;

// TODO
typedef signed short		blkcnt_t;
typedef ssize_t				blksize_t;
typedef unsigned long		clock_t;
typedef void				clockid_t;
typedef unsigned long		dev_t;
typedef unsigned short		fsblkcnt_t;
typedef unsigned short		fsfilcnt_t;
typedef unsigned long		gid_t;
typedef unsigned long		id_t;
typedef unsigned long		ino_t;
typedef void				key_t;
typedef unsigned short		mode_t;
typedef unsigned short		nlink_t;
typedef ssize_t				off_t;
typedef signed long			pid_t;
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
typedef void				reclen_t;
typedef unsigned long long	suseconds_t;
typedef unsigned long long	time_t;
typedef void				timer_t;
typedef unsigned long		uid_t;

#endif //_SYS_TYPES_H
