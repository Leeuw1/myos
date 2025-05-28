#include "syscall.h"
#include "proc.h"
#include "fs.h"

isize syscall_read(i32 fd, void* buf, usize count) {
	struct FSNode* node = fs_find(proc_fd_path(fd));
	return fs_read(node, buf, count);
}

isize syscall_write(i32 fd, const void* buf, usize count) {
	struct FSNode* node = fs_find(proc_fd_path(fd));
	return fs_write(node, buf, count);
}
