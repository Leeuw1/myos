#ifndef _TIME_H
#define _TIME_H

#include <sys/types.h>
#include <sys/select.h>
#include <locale.h>
#include <signal.h>
#include <stddef.h>

// TODO: macro definitions, functions

// TODO
#define CLOCKS_PER_SEC	0x100000
#define TIME_UTC		1

struct tm {
	int         tm_sec;    // Seconds [0,60].
	int         tm_min;    // Minutes [0,59].
	int         tm_hour;   // Hour [0,23].
	int         tm_mday;   // Day of month [1,31].
	int         tm_mon;    // Month of year [0,11].
	int         tm_year;   // Years since 1900.
	int         tm_wday;   // Day of week [0,6] (Sunday =0).
	int         tm_yday;   // Day of year [0,365].
	int         tm_isdst;  // Daylight Saving flag.
	long        tm_gmtoff; // Seconds east of UTC.
	const char *tm_zone;   // Timezone abbreviation.
};

struct timespec {
	time_t  tv_sec;    // Whole seconds.
	long    tv_nsec;   // Nanoseconds [0, 999999999].
};

struct itimerspec {
	struct timespec  it_interval;  // Timer period.
	struct timespec  it_value;     // Timer expiration.
};

extern int    daylight;
extern long   timezone;
extern char  *tzname[];

clock_t clock(void);
time_t time(time_t* tloc);
int gettimeofday(struct timeval* restrict tp, void* restrict tzp); // Deprecated
struct tm* gmtime_r(const time_t* restrict, struct tm* restrict);
struct tm *localtime(const time_t*);
struct tm *localtime_r(const time_t *restrict, struct tm *restrict);
time_t mktime(struct tm* );
int nanosleep(const struct timespec* rqtp, struct timespec* rmtp);
size_t strftime(char *restrict, size_t, const char *restrict, const struct tm *restrict);
double difftime(time_t, time_t);
int timespec_get(struct timespec* buf, int base);

#if 0
char      *asctime(const struct tm *);
clock_t    clock(void);
int        clock_getcpuclockid(pid_t, clockid_t *);
int        clock_getres(clockid_t, struct timespec *);
int        clock_gettime(clockid_t, struct timespec *);
int        clock_nanosleep(clockid_t, int, const struct timespec *,
               struct timespec *);
int        clock_settime(clockid_t, const struct timespec *);
char      *ctime(const time_t *);
double     difftime(time_t, time_t);
struct tm *getdate(const char *);
struct tm *gmtime(const time_t *);
struct tm *gmtime_r(const time_t *restrict, struct tm *restrict);
struct tm *localtime(const time_t *);
struct tm *localtime_r(const time_t *restrict, struct tm *restrict);
time_t     mktime(struct tm *);
int        nanosleep(const struct timespec *, struct timespec *);
size_t     strftime(char *restrict, size_t, const char *restrict,
           const struct tm *restrict);
size_t     strftime_l(char *restrict, size_t, const char *restrict,
               const struct tm *restrict, locale_t);
char      *strptime(const char *restrict, const char *restrict,
               struct tm *restrict);
time_t     time(time_t *);
int        timer_create(clockid_t, struct sigevent *restrict,
               timer_t *restrict);
int        timer_delete(timer_t);
int        timer_getoverrun(timer_t);
int        timer_gettime(timer_t, struct itimerspec *);
int        timer_settime(timer_t, int, const struct itimerspec *restrict,
               struct itimerspec *restrict);
int        timespec_get(struct timespec *, int);
void       tzset(void);
#endif

#endif //_TIME_H
