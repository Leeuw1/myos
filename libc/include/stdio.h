#ifndef _STDIO_H
#define _STDIO_H

#include <sys/types.h>
#include <stddef.h>
#include <stdarg.h>

#define	_IOFBF	0
#define	_IOLBF	1
#define	_IONBF	2

#define BUFSIZ	8192

#define EOF	(-1)

#define	SEEK_SET	0
#define	SEEK_CUR	1
#define	SEEK_END	2

#define stdin	stdin
#define stdout	stdout
#define stderr	stderr

typedef struct _FILE		FILE;
typedef unsigned long long	fpos_t;

extern FILE* stdin;
extern FILE* stdout;
extern FILE* stderr;

#ifndef STDIO_NO_FUNCTIONS
void clearerr(FILE* );
char* ctermid(char* );
int dprintf(int, const char* restrict, ...);
int fclose(FILE* );
FILE* fdopen(int, const char* );
int feof(FILE* );
int ferror(FILE* );
int fflush(FILE* );
int fgetc(FILE* );
int fgetpos(FILE* restrict, fpos_t* restrict);
char* fgets(char* restrict, int, FILE* restrict);
int fileno(FILE* );
void flockfile(FILE* );
FILE* fmemopen(void* restrict, size_t, const char* restrict);
FILE* fopen(const char* restrict, const char* restrict);
int fprintf(FILE* restrict stream, const char* restrict fmt, ...);
int fputc(int c, FILE* stream);
int fputs(const char* restrict s, FILE* restrict stream);
size_t fread(void* restrict, size_t, size_t, FILE* restrict);
FILE* freopen(const char* restrict, const char* restrict,
             FILE* restrict);
int fscanf(FILE* restrict, const char* restrict, ...);
int fseek(FILE* , long, int);
int fseeko(FILE* , off_t, int);
int fsetpos(FILE* , const fpos_t* );
long ftell(FILE* );
off_t ftello(FILE* );
int ftrylockfile(FILE* );
void funlockfile(FILE* );
size_t fwrite(const void* restrict, size_t, size_t, FILE* restrict);
int getc(FILE* );
int getchar(void);
int getc_unlocked(FILE* );
int getchar_unlocked(void);
ssize_t getdelim(char** restrict, size_t* restrict, int,
             FILE* restrict);
ssize_t getline(char** restrict, size_t* restrict, FILE* restrict);
FILE* open_memstream(char** , size_t* );
int pclose(FILE* );
void perror(const char* s);
FILE* popen(const char* , const char* );
int printf(const char* restrict fmt, ...);
//#define printf(fmt, ...)	fprintf(stdout, fmt, __VA_ARGS__)
int putc(int c, FILE* stream);
//#define putc(c, stream)		fputc(c, stream)
int putchar(int c);
//#define putchar(c)			fputc(c, stdout)
int putc_unlocked(int c, FILE* stream);
int putchar_unlocked(int c);
//#define putchar_unlocked(c)			putc_unlocked(c, stdout)
int puts(const char* s);
//#define puts(s)						fputs(s, stdout)
int remove(const char* );
int rename(const char* , const char* );
int renameat(int, const char* , int, const char* );
void rewind(FILE* );
int scanf(const char* restrict, ...);
void setbuf(FILE* restrict, char* restrict);
int setvbuf(FILE* restrict, char* restrict, int, size_t);
int snprintf(char* restrict, size_t, const char* restrict, ...);
int sprintf(char* restrict, const char* restrict, ...);
int sscanf(const char* restrict, const char* restrict, ...);
FILE* tmpfile(void);
char* tmpnam(char* );
int ungetc(int, FILE* );
int vdprintf(int, const char* restrict, va_list);
int vfprintf(FILE* restrict stream, const char* restrict fmt, va_list args);
int vfscanf(FILE* restrict, const char* restrict, va_list);
int vprintf(const char* restrict fmt, va_list args);
//#define vprintf(fmt, args)	vfprintf(stdout, fmt, args)
int vscanf(const char* restrict, va_list);
int vsnprintf(char* restrict, size_t, const char* restrict,
             va_list);
int vsprintf(char* restrict, const char* restrict, va_list);
int vsscanf(const char* restrict, const char* restrict, va_list);
#endif //STDIO_NO_FUNCTIONS

#endif //_STDIO_H
