#include "syscall.h"
#include "proc.h"
#include "fs.h"
#include "io.h"
#include "exception.h"

isize syscall_read(i32 fd, void* buf, usize count) {
	if (try_translate(buf) & 1) {
		PRINT_ERROR("Invalid address");
		printf("size %, buf=%\n", (u64)count, (u64)buf);
		proc_kill();
	}
#if 0
	struct FSNode* node = fs_find(proc_fd_path(fd));
	isize size = fs_read(node, buf, count);
	return size;
#else
	proc_queue_read(fd, buf, count);
	__builtin_unreachable();
#endif
}

isize syscall_write(i32 fd, const void* buf, usize count) {
	if (try_translate(buf) & 1) {
		PRINT_ERROR("Invalid address.");
		printf("size %, buf=%\n", (u64)count, (u64)buf);
		proc_kill();
	}
	struct FSNode* node = fs_find(proc_fd_path(fd));
	return fs_write(node, buf, count);
}
