#include <string.h>
#include <stdlib.h>
#include <errno.h>

#include <stdio.h>
#define UNIMP()			printf("[libc] Warning: %s is not implemented.\n" ,__FUNCTION__);

void* memccpy(void* restrict dst, const void* restrict src, int c, size_t n) {
	for (size_t i = 0; i < n; ++i) {
		((char*)dst)[i] = ((char*)src)[i];
		if (((char*)src)[i] == (char)c) {
			return (void*)(src + i + 1);
		}
	}
	return NULL;
}

void* memchr(const void* s, int c, size_t n) {
	for (size_t i = 0; i < n; ++i) {
		if (((char*)s)[i] == (char)c) {
			return (void*)(s + i);
		}
	}
	return NULL;
}

// TODO: upgrade
int memcmp(const void* s1, const void* s2, size_t n) {
	const char* chars1 = s1;
	const char* chars2 = s2;
	for (size_t i = 0; i < n; ++i) {
		const int diff = (int)chars1[i] - (int)chars2[i];
		if (diff != 0) {
			return diff;
		}
	}
	return 0;
}

// TODO: upgrade
void* memcpy(void* restrict dst, const void *restrict src, size_t n) {
	for (size_t i = 0; i < n; ++i) {
		((char*)dst)[i] = ((char*)src)[i];
	}
	return dst;
}

// TODO: maybe upgrade
void* memmove(void* dst, const void* src, size_t n) {
	if (dst < src) {
		for (size_t i = 0; i < n; ++i) {
			((char*)dst)[i] = ((char*)src)[i];
		}
	}
	else {
		for (size_t i = 0; i < n; ++i) {
			((char*)dst)[n - 1 - i] = ((char*)src)[n - 1 - i];
		}
	}
	return dst;
}

// TODO: upgrade
void* memset(void* s, int c, size_t n) {
	for (size_t i = 0; i < n; ++i) {
		((char*)s)[i] = c;
	}
	return s;
}

char* stpcpy(char* restrict dst, const char* restrict src) {
	size_t length = strlen(src);
	memcpy(dst, src, length);
	dst[length] = '\0';
	return dst + length;
}

char* stpncpy(char* restrict dst, const char *restrict src, size_t n) {
	size_t length = strnlen(src, n);
	memcpy(dst, src, length);
	return memset(dst + length, 0, n - length);
}

char* strcat(char* restrict dst, const char* restrict src) {
	strcpy(dst + strlen(dst), src);
	return dst;
}

char* strchr(const char* s, int c) {
	size_t i;
	for (i = 0; s[i] != '\0'; ++i) {
		if (s[i] == (char)c) {
			return (char*)&s[i];
		}
	}
	return c == '\0' ? (char*)&s[i] : NULL;
}

int strcmp(const char* s1, const char* s2) {
	for (size_t i = 0;; ++i) {
		const int diff = (int)s1[i] - (int)s2[i];
		if (diff != 0 || s1[i] * s2[i] == 0) {
			return diff;
		}
	}
	return 0;
}

// NOTE: locales are not implemented
int strcoll(const char* s1, const char* s2) {
	return strcmp(s1, s2);
}

// NOTE: locales are not implemented
int strcoll_l(const char* s1, const char* s2, locale_t locale) {
	(void)locale;
	return strcmp(s1, s2);
}

char* strcpy(char* restrict dst, const char* restrict src) {
	size_t length = strlen(src);
	memcpy(dst, src, length);
	dst[length] = '\0';
	return dst;
}

size_t strcspn(const char* s, const char* reject) {
	size_t length = strlen(reject);
	size_t i;
	for (i = 0; s[i] != '\0'; ++i) {
		if (memchr(reject, s[i], length) != NULL) {
			return i;
		}
	}
	return i;
}

char* strdup(const char* s) {
	size_t length = strlen(s);
	char* dup = malloc(length + 1);
	memcpy(dup, s, length);
	dup[length] = '\0';
	return dup;
}

char* strerror(int errnum) {
	switch (errnum) {
	default:
		return "Unknown error.";
	case EBADF:
		return "Bad file descriptor.";
	case EEXIST:
		return "File exists.";
	case EFAULT:
		return "Bad address.";
	case EINVAL:
		return "Invalid argument.";
	case EIO:
		return "I/O error.";
	case EISDIR:
		return "Is a directory.";
	case ENFILE:
		return "Too many files open in system.";
	case ENOENT:
		return "No such file or directory.";
	case ENOEXEC:
		return "Executable file format error.";
	case ENOTDIR:
		return "Not a directory or a symbolic link to a directory.";
	case ENOTEMPTY:
		return "Directory not empty.";
	case EPERM:
		return "Operation not permitted.";
	case ERANGE:
		return "Result too large.";
	case ESRCH:
		return "No such process.";
	}
}

char* strerror_l(int errnum, locale_t locale) {
	(void)locale;
	return strerror(errnum);
}

// TODO
int strerror_r(int errnum, char* buf, size_t size) {
	UNIMP();
	return 0;
}

size_t strlen(const char* s) {
	for (size_t i = 0;; ++i) {
		if (s[i] == '\0') {
			return i;
		}
	}
}

char* strncat(char* restrict dst, const char* restrict src, size_t n) {
	size_t length = strnlen(src, n);
	char* end = memcpy(dst + strlen(dst), src, length) + length;
	*end = '\0';
	return dst;
}

int strncmp(const char* s1, const char* s2, size_t n) {
	for (size_t i = 0; i < n; ++i) {
		const int diff = (int)s1[i] - (int)s2[i];
		if (diff != 0 || s1[i] * s2[i] == 0) {
			return diff;
		}
	}
	return 0;
}

char* strncpy(char* restrict dst, const char* restrict src, size_t n) {
	stpncpy(dst, src, n);
	return dst;
}

char* strndup(const char* s, size_t n) {
	size_t length = strlen(s);
	size_t l = length > n ? n : length;
	char* dup = malloc(l + 1);
	memcpy(dup, s, l);
	dup[l] = '\0';
	return dup;
}

size_t strnlen(const char* s, size_t n) {
	for (size_t i = 0; i < n; ++i) {
		if (s[i] == '\0') {
			return i;
		}
	}
	return n;
}

char* strpbrk(const char* s, const char* accept) {
	size_t length = strlen(accept);
	size_t i;
	for (i = 0; s[i] != '\0'; ++i) {
		if (memchr(accept, s[i], length) != NULL) {
			return (char*)&s[i];
		}
	}
	return NULL;
}

char* strrchr(const char* s, int c) {
	size_t length = strlen(s) + 1;
	size_t i;
	for (i = 0; i < length; ++i) {
		const char* it = s + length - 1 - i;
		if (*it == (char)c) {
			return (char*)it;
		}
	}
	return NULL;
}

// TODO
char* strsignal(int sig) {
	UNIMP();
	return NULL;
}

size_t strspn(const char* s, const char* accept) {
	size_t length = strlen(accept);
	size_t i;
	for (i = 0; s[i] != '\0'; ++i) {
		if (memchr(accept, s[i], length) == NULL) {
			return i;
		}
	}
	return i;
}

char* strstr(const char* haystack, const char* needle) {
	const size_t n_length = strlen(needle);
	if (n_length == 0) {
		return (char*)haystack;
	}
	const size_t h_length = strlen(haystack);
	if (n_length > h_length) {
		return NULL;
	}
	for (size_t i = 0; i <= h_length - n_length; ++i) {
		if (memcmp(haystack + i, needle, n_length) == 0) {
			return (char*)haystack + i;
		}
	}
	return NULL;
}

// TODO
char* strtok(char* restrict s, const char* restrict delim) {
	UNIMP();
	return NULL;
}

char* strtok_r(char* restrict s, const char* restrict delim, char** restrict saveptr) {
	UNIMP();
	return NULL;
}
size_t strxfrm(char* restrict dst, const char* restrict src, size_t n) {
	UNIMP();
	return 0;
}

size_t strxfrm_l(char* restrict dst, const char* restrict src, size_t n, locale_t locale) {
	UNIMP();
	return 0;
}
