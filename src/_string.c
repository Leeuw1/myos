#include <string.h>

void* memchr(const void* s, int c, size_t n) {
	for (size_t i = 0; i < n; ++i) {
		if (((char*)s)[i] == (char)c) {
			return (void*)(s + i);
		}
	}
	return NULL;
}

int memcmp(const void* s1, const void* s2, size_t n) {
	for (size_t i = 0; i < n; ++i) {
		int diff = ((char*)s2)[i] - ((char*)s1)[i];
		if (diff != 0) {
			return diff;
		}
	}
	return 0;
}

void* memcpy(void* restrict dst, const void *restrict src, size_t n) {
	for (size_t i = 0; i < n; ++i) {
		((char*)dst)[i] = ((char*)src)[i];
	}
	return dst;
}

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
	stpcpy(dst + strlen(dst), src);
	return dst;
}

int strcmp(const char* s1, const char* s2) {
	for (size_t i = 0;; ++i) {
		int diff = ((char*)s2)[i] - ((char*)s1)[i];
		if (diff != 0 || s1[i] * s2[i] == 0) {
			return diff;
		}
	}
	return 0;
}

char* strcpy(char* restrict dst, const char* restrict src) {
	stpcpy(dst, src);
	return dst;
}

size_t strlen(const char* s) {
	for (size_t i = 0;; ++i) {
		if (s[i] == '\0') {
			return i;
		}
	}
}

int strncmp(const char* s1, const char* s2, size_t n) {
	for (size_t i = 0; i < n; ++i) {
		int diff = s2[i] - s1[i];
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

size_t strnlen(const char* s, size_t n) {
	for (size_t i = 0; i < n; ++i) {
		if (s[i] == '\0') {
			return i;
		}
	}
	return n;
}
