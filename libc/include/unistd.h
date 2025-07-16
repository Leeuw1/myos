#ifndef _UNISTD_H
#define _UNISTD_H

#include <stddef.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/types.h>
#include <stdint.h>

#define F_OK	0x1
#define R_OK	0x2
#define W_OK	0x4
#define X_OK	0x8

#define	SEEK_HOLE	3
#define	SEEK_DATA	4

#define	F_LOCK	1
#define	F_TEST	2
#define	F_TLOCK	3
#define	F_ULOCK	4

#define	STDIN_FILENO	0
#define	STDOUT_FILENO	1
#define	STDERR_FILENO	2

extern char  *optarg;
extern int    opterr, optind, optopt;

int          access(const char *, int);
unsigned     alarm(unsigned);
int          chdir(const char *);
int          chown(const char *, uid_t, gid_t);
int          close(int);
size_t       confstr(int, char *, size_t);
char        *crypt(const char *, const char *);
int          dup(int);


int          dup2(int, int);
int          dup3(int, int, int);
_Noreturn void _exit(int);
void         encrypt(char [64], int);
int          execl(const char *, const char *, ...);
int          execle(const char *, const char *, ...);
int          execlp(const char *, const char *, ...);
int          execv(const char *, char *const []);
int          execve(const char *, char *const [], char *const []);
int          execvp(const char *, char *const []);
int          faccessat(int, const char *, int, int);
int          fchdir(int);
int          fchown(int, uid_t, gid_t);
int          fchownat(int, const char *, uid_t, gid_t, int);
int          fdatasync(int);
int          fexecve(int, char *const [], char *const []);
pid_t        _Fork(void);
pid_t        fork(void);
long         fpathconf(int, int);
int          fsync(int);
int          ftruncate(int, off_t);
char        *getcwd(char *, size_t);
gid_t        getegid(void);
int          getentropy(void *, size_t);
uid_t        geteuid(void);
gid_t        getgid(void);
int          getgroups(int, gid_t []);
long         gethostid(void);
int gethostname(char* buf, size_t size);
char        *getlogin(void);
int          getlogin_r(char *, size_t);
int          getopt(int, char * const [], const char *);
pid_t        getpgid(pid_t);
pid_t        getpgrp(void);
pid_t        getpid(void);
pid_t        getppid(void);
int          getresgid(gid_t *restrict, gid_t *restrict,
                 gid_t *restrict);
int          getresuid(uid_t *restrict, uid_t *restrict,
                 uid_t *restrict);
pid_t        getsid(pid_t);
uid_t        getuid(void);
int          isatty(int);
int          lchown(const char *, uid_t, gid_t);
int          link(const char *, const char *);
int          linkat(int, const char *, int, const char *, int);
int          lockf(int, int, off_t);
off_t        lseek(int, off_t, int);
int          nice(int);
long         pathconf(const char *, int);
int          pause(void);
int          pipe(int [2]);
int          pipe2(int [2], int);
int          posix_close(int, int);
ssize_t      pread(int, void *, size_t, off_t);
ssize_t      pwrite(int, const void *, size_t, off_t);
ssize_t read(int fd, void* buf, size_t count);
ssize_t      readlink(const char *restrict, char *restrict, size_t);
ssize_t      readlinkat(int, const char *restrict, char *restrict,
                 size_t);
int rmdir(const char* path);
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
int          symlink(const char *, const char *);
int          symlinkat(const char *, int, const char *);
void         sync(void);
long         sysconf(int);
pid_t        tcgetpgrp(int);
int          tcsetpgrp(int, pid_t);
int          truncate(const char *, off_t);
char        *ttyname(int);
int          ttyname_r(int, char *, size_t);
int          unlink(const char *);
int          unlinkat(int, const char *, int);
ssize_t write(int fd, const void* buf, size_t count);

#endif //_UNISTD_H
