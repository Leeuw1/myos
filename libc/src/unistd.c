#include <unistd.h>
#include <sys/stat.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#define MYOS_NO_BOOL
#include "../../src/syscall.h"

#define UNIMP()			printf("[libc] Warning: %s is not implemented.\n" ,__FUNCTION__);

char** environ = { NULL };
char* optarg;
int opterr, optind, optopt;

int access(const char *, int) {
	UNIMP();
	return 0;
}

unsigned     alarm(unsigned);

int chdir(const char* path) {
	_syscall_1arg(SYSCALL_CHDIR, int, path);
}

int          chown(const char *, uid_t, gid_t);

int close(int fd) {
	_syscall_1arg(SYSCALL_CLOSE, int, fd);
}

size_t       confstr(int, char *, size_t);
char        *crypt(const char *, const char *);

int dup(int) {
	UNIMP();
	return -1;
}

int          dup2(int, int);
int          dup3(int, int, int);

_Noreturn void _exit(int status) {
	exit(status);
}

void         encrypt(char [64], int);
int          execl(const char *, const char *, ...);
int          execle(const char *, const char *, ...);
int          execlp(const char *, const char *, ...);

int execv(const char* path, char* const argv[]) {
	char* const envp[] = { NULL };
	return execve(path, argv, envp);
}

int execve(const char* path, char* const argv[], char* const envp[]) {
	_syscall_3arg(SYSCALL_EXECVE, int, path, argv, envp);
}

int execvp(const char* file, char *const argv[]) {
	return execv(file, argv);
}

int          faccessat(int, const char *, int, int);
int          fchdir(int);
int          fchown(int, uid_t, gid_t);
int          fchownat(int, const char *, uid_t, gid_t, int);
int          fdatasync(int);
int          fexecve(int, char *const [], char *const []);
pid_t        _Fork(void);

pid_t fork(void) {
	_syscall_0arg(SYSCALL_FORK, pid_t);
}

long         fpathconf(int, int);

int fsync(int fd) {
	(void)fd;
	return 0;
}

int          ftruncate(int, off_t);

char* getcwd(char* buf, size_t size) {
	_syscall_2arg(SYSCALL_GETCWD, char*, buf, size);
}

gid_t        getegid(void);
int          getentropy(void *, size_t);
uid_t        geteuid(void);

gid_t getgid(void) {
	UNIMP();
	return 0;
}

int          getgroups(int, gid_t []);
long         gethostid(void);

int gethostname(char* buf, size_t size) {
	if (size < 5) {
		return -1;
	}
	strcpy(buf, "myos");
	return 0;
}

char        *getlogin(void);
int          getlogin_r(char *, size_t);

int getopt(int, char * const [], const char *) {
	UNIMP();
	return -1;
}

pid_t        getpgid(pid_t);
pid_t        getpgrp(void);

pid_t getpid(void) {
	_syscall_0arg(SYSCALL_GETPID, pid_t);
}

pid_t        getppid(void);
int          getresgid(gid_t *restrict, gid_t *restrict,
                 gid_t *restrict);
int          getresuid(uid_t *restrict, uid_t *restrict,
                 uid_t *restrict);
pid_t        getsid(pid_t);

// TODO
uid_t getuid(void) {
	return 0;
}

int isatty(int fd) {
	struct stat status;
	if (fstat(fd, &status) == -1) {
		return 0;
	}
	return S_ISCHR(status.st_mode);
}

int          lchown(const char *, uid_t, gid_t);

int link(const char* oldpath, const char* newpath) {
	(void)oldpath; (void)newpath;
	errno = ENOTSUP;
	return -1;
}

int          linkat(int, const char *, int, const char *, int);
int          lockf(int, int, off_t);

off_t lseek(int fd, off_t offset, int whence) {
	_syscall_3arg(SYSCALL_LSEEK, off_t, fd, offset, whence);
}

int          nice(int);
long         pathconf(const char *, int);
int          pause(void);

int pipe(int [2]) {
	UNIMP();
	return -1;
}

int          pipe2(int [2], int);
int          posix_close(int, int);
ssize_t      pread(int, void *, size_t, off_t);
ssize_t      pwrite(int, const void *, size_t, off_t);

ssize_t read(int fd, void* buf, size_t count) {
	_syscall_3arg(SYSCALL_READ, ssize_t, fd, buf, count);
}

ssize_t readlink(const char* restrict path, char* restrict buf, size_t size) {
	(void)path; (void)buf; (void)size;
	errno = ENOTSUP;
	return -1;
}

ssize_t      readlinkat(int, const char *restrict, char *restrict,
                 size_t);

int rmdir(const char* path) {
	_syscall_1arg(SYSCALL_RMDIR, int, path);
}

int          setegid(gid_t);
int          seteuid(uid_t);
int          setgid(gid_t);


int          setpgid(pid_t, pid_t);
int          setregid(gid_t, gid_t);
int          setresgid(gid_t, gid_t, gid_t);
int          setresuid(uid_t, uid_t, uid_t);
int          setreuid(uid_t, uid_t);
pid_t        setsid(void);
int          setuid(uid_t);
unsigned     sleep(unsigned);
void         swab(const void *restrict, void *restrict, ssize_t);

int symlink(const char* path1, const char* path2) {
	(void)path1; (void)path2;
	errno = ENOTSUP;
	return -1;
}

int          symlinkat(const char *, int, const char *);
void         sync(void);
long         sysconf(int);
pid_t        tcgetpgrp(int);
int          tcsetpgrp(int, pid_t);
int          truncate(const char *, off_t);
char        *ttyname(int);
int          ttyname_r(int, char *, size_t);

int unlink(const char* path) {
	_syscall_1arg(SYSCALL_UNLINK, int, path);
}

int          unlinkat(int, const char *, int);

ssize_t write(int fd, const void* buf, size_t count) {
	_syscall_3arg(SYSCALL_WRITE, ssize_t, fd, buf, count);
}
