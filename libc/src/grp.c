#include <grp.h>

struct group* getgrgid(gid_t gid) {
#if 0
	static struct group g = {
		.gr_gid = gid,
	};
	return &g;
#endif
	return (void*)0;
}
