#ifndef _STRING_H
#define _STRING_H

#include <stddef.h>

// TODO
typedef int	locale_t;

void    *memccpy(void *restrict, const void *restrict, int, size_t);
void    *memchr(const void *, int, size_t);
int      memcmp(const void *, const void *, size_t);
void    *memcpy(void *restrict, const void *restrict, size_t);
void    *memmove(void *, const void *, size_t);
void    *memset(void *, int, size_t);
char    *stpcpy(char *restrict, const char *restrict);
char    *stpncpy(char *restrict, const char *restrict, size_t);
char    *strcat(char *restrict, const char *restrict);
char    *strchr(const char *, int);
int      strcmp(const char *, const char *);
int      strcoll(const char *, const char *);
int      strcoll_l(const char *, const char *, locale_t);
char    *strcpy(char *restrict, const char *restrict);
size_t   strcspn(const char *, const char *);
char    *strdup(const char *);
char    *strerror(int);
char    *strerror_l(int, locale_t);
int      strerror_r(int, char *, size_t);
size_t   strlen(const char *);
char    *strncat(char *restrict, const char *restrict, size_t);
int      strncmp(const char *, const char *, size_t);
char    *strncpy(char *restrict, const char *restrict, size_t);
char    *strndup(const char *, size_t);
size_t   strnlen(const char *, size_t);
char    *strpbrk(const char *, const char *);
char    *strrchr(const char *, int);
char    *strsignal(int);
size_t   strspn(const char *, const char *);
char    *strstr(const char *, const char *);
char    *strtok(char *restrict, const char *restrict);
char    *strtok_r(char *restrict, const char *restrict, char **restrict);
size_t   strxfrm(char *restrict, const char *restrict, size_t);
size_t   strxfrm_l(char *restrict, const char *restrict, size_t, locale_t);

#endif //_STRING_H
