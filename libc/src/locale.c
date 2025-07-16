#include <locale.h>
#include <stddef.h>

#include <stdio.h>
#define UNIMP()			printf("[libc] Warning: %s is not implemented.\n" ,__FUNCTION__);

static struct lconv _lconv;

void _libc_init_locale(void) {
	_lconv.decimal_point = ".";
	// TODO ...
}

struct lconv* localeconv(void) {
	return &_lconv;
}

char* setlocale(int category, const char* locale) {
	(void)category; (void)locale;
	//UNIMP();
	return NULL;
}
