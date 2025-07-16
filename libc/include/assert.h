#ifndef _ASSERT_H
#define _ASSERT_H

#include <stdio.h>
#include <stdlib.h>

#ifdef NDEBUG
#define assert(ignore)	((void)0)
#else
#define assert(expr) \
	if (!(expr)) {\
		fprintf(stderr, "[libs] Assertion '%s' failed in %s, line %d, inside function '%s'.\n",\
			#expr,\
			__FILE__,\
			(int)__LINE__,\
			__FUNCTION__\
		);\
		abort();\
	}
#endif

#endif //_ASSERT_H
