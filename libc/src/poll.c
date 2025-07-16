#include <poll.h>
#include <unistd.h>
#include <errno.h>

#include <stdio.h>
#define UNIMP()			printf("[libc] Warning: %s is not implemented.\n" ,__FUNCTION__);

int poll(struct pollfd fds[], nfds_t nfds, int timeout) {
	//UNIMP();
	//printf("%d FDs, timeout=%d\n", (int)nfds, timeout);
	for (nfds_t i = 0; i < nfds; ++i) {
		char buf[1];
		const ssize_t result = read(fds[i].fd, buf, 0);
		fds[i].revents = (result == -1 && errno == EAGAIN) ? 0 : POLLIN;
	}
	return nfds;
}
