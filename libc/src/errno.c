#include "errno.h"

int* _errno(void) {
	int* e;
	asm volatile (
		"mrs	%0, tpidrro_el0"
		: "=r" (e)
	);
	return e;
}
