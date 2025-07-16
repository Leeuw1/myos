#include <time.h>
#include <string.h>
#include <stdio.h>
#define MYOS_NO_BOOL
#include "../../src/syscall.h"

#define UNIMP()			printf("[libc] Warning: %s is not implemented.\n" ,__FUNCTION__);

#define SECS_PER_YEAR	31536000
#define SECS_PER_DAY	86400

static const char* _months[12] = {
	"Jan", "Feb", "Mar", "Apr", "May", "Jun",
	"Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
};

clock_t clock(void) {
	//_syscall_0arg(SYSCALL_CLOCK, clock_t);
	UNIMP();
	return 0;
}

static time_t _time(void) {
	_syscall_0arg(SYSCALL_TIME, time_t);
}

time_t time(time_t* tloc) {
	const time_t t = _time();
	if (tloc != NULL) {
		*tloc = t;
	}
	return t;
}

// TODO: can also do this years (leap years give a different number)
#if 0
// TODO
static time_t _seconds_per_month(int month) {
	switch () {

	}
}
#endif

// NOTE: Not a correct implementation, just a rough estimate
struct tm* gmtime_r(const time_t* restrict timep, struct tm* restrict result) {
	time_t t = *timep;
	memset(result, 0, sizeof *result);
	const int year = t / SECS_PER_YEAR;
	t %= SECS_PER_YEAR;
	result->tm_year = year + 70;
	result->tm_yday = t / SECS_PER_DAY;
	result->tm_mon = t / (30 * SECS_PER_DAY);
	t %= 30 * SECS_PER_DAY;
	result->tm_mday = t / SECS_PER_DAY + 1;
	t %= SECS_PER_DAY;
	result->tm_hour = t / 3600;
	t %= 3600;
	result->tm_min = t / 60;
	t %= 60;
	result->tm_sec = t;
	result->tm_zone = "UTC";
	return result;
}

struct tm* localtime_r(const time_t* restrict timep, struct tm* restrict result) {
	return gmtime_r(timep, result);
}

time_t mktime(struct tm *) {
	UNIMP();
	return 0;
}

int nanosleep(const struct timespec* rqtp, struct timespec* rmtp) {
	_syscall_2arg(SYSCALL_NANOSLEEP, int, rqtp, rmtp);
}

// TODO: use fmt string
size_t strftime(char* restrict s, size_t max, const char* restrict fmt, const struct tm* restrict tm) {
	(void)fmt;
	const char* month = (tm->tm_mon >= 0 && tm->tm_mon <= 11) ? _months[tm->tm_mon] : "BAD_MONTH";
	return snprintf(s, max, "%s %d %s %d", month, tm->tm_mday, tm->tm_zone, 1900 + tm->tm_year);
}

double difftime(time_t, time_t) {
	UNIMP();
	return 0;
}

int timespec_get(struct timespec* buf, int base) {
	UNIMP();
	return 0;
}
