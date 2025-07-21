#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stddef.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <fcntl.h>
#include <math.h>
#include <assert.h>
#include <errno.h>
#define MYOS_NO_BOOL
#include "../../src/syscall.h"

#define UNIMP()			printf("[libc] Warning: %s is not implemented.\n" ,__FUNCTION__);

#define STREAM_FLAGS_EOF	0x1
#define STREAM_FLAGS_ERROR	0x2

struct _FILE {
	FILE*			next;
	int				fd;
	unsigned char	flags;
};

static FILE* _tail;
FILE* stdin;
FILE* stdout;
FILE* stderr;

void _libc_init_stdio(void) {
	stdin = fdopen(STDIN_FILENO, "r");
	stdout = fdopen(STDOUT_FILENO, "w");
	stderr = fdopen(STDERR_FILENO, "w");
	stdin->next = stdout;
	stdout->next = stderr;
	stderr->next = NULL;
	_tail = stderr;
}

void clearerr(FILE* stream) {
	stream->flags &= ~STREAM_FLAGS_ERROR;
}

char* ctermid(char* s) {
	char* path = "/dev/tty";
	if (s == NULL)  {
		return path;
	}
	return strcpy(s, path);
}

int dprintf(int fd, const char* restrict fmt, ...) {
	va_list args;
	va_start(args, fmt);
	int length = vdprintf(fd, fmt, args);
	va_end(args);
	return length;
}

int fclose(FILE* stream) {
	close(stream->fd);
	free(stream);
	return 0;
}

// TODO
FILE* fdopen(int fd, const char* mode) {
	// TODO: parse mode
	FILE* stream = malloc(sizeof *stream);
	memset(stream, 0, sizeof *stream);
	// TODO: set mode
	stream->fd = fd;
	return stream;
}

int feof(FILE* stream) {
	return stream->flags & STREAM_FLAGS_EOF;
}

int ferror(FILE* stream) {
	return stream->flags & STREAM_FLAGS_ERROR;
}

// TODO
static int _fflush(FILE* stream) {
	return 0;
}

// TODO
int fflush(FILE* stream) {
#if 0
	if (stream != NULL) {
		return _fflush(stream);
	}
	for (FILE* it = stdin; it != NULL; it = it->next) {
		_fflush(it);
	}
#endif
	return 0;
}

int fgetc(FILE* stream) {
	return getc(stream);
}

// TODO
int fgetpos(FILE* restrict stream, fpos_t* restrict) {
	UNIMP();
	return 0;
}

// TODO: make everything consistent so that it doesn't matter whether 'stream' refers to stdin
char* fgets(char* restrict s, int size, FILE* restrict stream) {
#if 1
	if (stream->fd == STDIN_FILENO) {
		const size_t actual_size = fread(s, 1, size - 1, stream);
		if (ferror(stream)) {
			return NULL;
		}
		s[actual_size] = '\0';
		return s;
	}
#endif
	int i;
	for (i = 0; i < size - 1; ++i) {
		const int c = getc(stream);
		if (c == EOF) {
			if (ferror(stream) || i == 0) {
				return NULL;
			}
			break;
		}
		s[i] = (char)c;
		if (c == '\n') {
			++i;
			break;
		}
	}
	s[i] = '\0';
	return s;
}

int fileno(FILE* stream) {
	return stream->fd;
}

// TODO
void flockfile(FILE* stream) {
}

// TODO
FILE* fmemopen(void* restrict, size_t, const char* restrict) {
	UNIMP();
	return NULL;
}

// TODO
FILE* fopen(const char* restrict path, const char* restrict mode) {
	// TODO: parse mode
	FILE* stream = malloc(sizeof *stream);
	memset(stream, 0, sizeof *stream);
	// TODO: initialize
	stream->fd = open(path, O_CREAT);
	//_tail->next = stream;
	//_tail = stream;
	return stream;
}

int fprintf(FILE* restrict stream, const char* restrict fmt, ...) {
	va_list args;
	va_start(args, fmt);
	int length = vfprintf(stream, fmt, args);
	va_end(args);
	return length;
}

int fputc(int c, FILE* stream) {
	return putc(c, stream);
}

int _fputs_unlocked(const char* restrict s, FILE* restrict stream) {
	return (int)fwrite(s, 1, strlen(s), stream);
}

int fputs(const char* restrict s, FILE* restrict stream) {
	flockfile(stream);
	size_t length = _fputs_unlocked(s, stream);
	funlockfile(stream);
	return length;
}

size_t fread(void* restrict data, size_t size, size_t n, FILE* restrict stream) {
	const ssize_t result = read(stream->fd, data, size * n);
	if (result == -1) {
		stream->flags |= STREAM_FLAGS_ERROR;
		return 0;
	}
	if (result == 0 || (size_t)result < size * n) {
		stream->flags |= STREAM_FLAGS_EOF;
	}
	return result / size;
}

// TODO
FILE* freopen(const char* restrict, const char* restrict, FILE* restrict) {
	UNIMP();
	return NULL;
}

int fscanf(FILE* restrict stream, const char* restrict fmt, ...) {
	va_list args;
	va_start(args, fmt);
	int result = vfscanf(stream, fmt, args);
	va_end(args);
	return result;
}

// TODO
int fseek(FILE* stream, long offset, int whence) {
	UNIMP();
	return 0;
}

// TODO
int fseeko(FILE* stream, off_t offset, int whence) {
	UNIMP();
	return 0;
}

// TODO
int fsetpos(FILE* stream, const fpos_t* pos) {
	UNIMP();
	return 0;
}

// TODO
long ftell(FILE* stream) {
	UNIMP();
	return 0;
}

// TODO
off_t ftello(FILE* stream) {
	UNIMP();
	return 0;
}

// TODO
int ftrylockfile(FILE* stream) {
	return 0;
}

// TODO
void funlockfile(FILE* stream) {
}

size_t fwrite(const void* restrict data, size_t size, size_t n, FILE* restrict stream) {
	write(stream->fd, data, size * n);
	return n;
}

int getc(FILE* stream) {
	flockfile(stream);
	int c = getc_unlocked(stream);
	funlockfile(stream);
	return c;
}

int getchar(void) {
	return getc(stdin);
}

int getc_unlocked(FILE* stream) {
	unsigned char c = 0;
	const ssize_t result = read(stream->fd, &c, 1);
	if (result == 0) {
		stream->flags |= STREAM_FLAGS_EOF;
		return EOF;
	}
	if (result == -1) {
		stream->flags |= STREAM_FLAGS_ERROR;
		return EOF;
	}
	return (int)c;
}

int getchar_unlocked(void) {
	return getc_unlocked(stdin);
}

// TODO
ssize_t getdelim(char** restrict lineptr, size_t* restrict n, int delim, FILE* restrict stream) {
	UNIMP();
	return 0;
}

// TODO
ssize_t getline(char** restrict lineptr, size_t* restrict n, FILE* restrict stream) {
	UNIMP();
	return 0;
}

// TODO
FILE* open_memstream(char**, size_t* ) {
	UNIMP();
	return NULL;
}

// TODO
int pclose(FILE* stream) {
	UNIMP();
	return 0;
}

void perror(const char* s) {
	fprintf(stderr, "%s: %s\n", s, strerror(errno));
}

// TODO
FILE* popen(const char* , const char* ) {
	UNIMP();
	return NULL;
}

int printf(const char* restrict fmt, ...) {
	va_list args;
	va_start(args, fmt);
	int length = vfprintf(stdout, fmt, args);
	va_end(args);
	return length;
}

int putc(int c, FILE* stream) {
	flockfile(stream);
	int result = putc_unlocked(c, stream);
	funlockfile(stream);
	return result;
}

int putc_unlocked(int c, FILE* stream) {
	const unsigned char byte = (unsigned char)c;
	fwrite(&byte, 1, 1, stream);
	return (int)byte;
}

int putchar(int c) {
	return putc(c, stdout);
}

int putchar_unlocked(int c) {
	return putc_unlocked(c, stdout);
}

int puts(const char* s) {
	const int length = fputs(s, stdout);
	putchar('\n');
	return length + 1;
}

int remove(const char* path) {
	if (unlink(path) == 0) {
		return 0;
	}
	return rmdir(path);
}

int rename(const char* oldpath, const char* newpath) {
	_syscall_2arg(SYSCALL_RENAME, int, oldpath, newpath);
}

int renameat(int, const char* , int, const char* ) {
	UNIMP();
	return 0;
}

void rewind(FILE* stream) {
	fseek(stream, 0, SEEK_SET);
	stream->flags &= ~STREAM_FLAGS_ERROR;
}

int scanf(const char* restrict fmt, ...) {
	va_list args;
	va_start(args, fmt);
	int result = vfscanf(stdout, fmt, args);
	va_end(args);
	return result;
}

void setbuf(FILE* restrict, char* restrict);

// TODO
int setvbuf(FILE* restrict stream, char* restrict buf, int mode, size_t size) {
	UNIMP();
	return 0;
}

int snprintf(char* restrict s, size_t size, const char* restrict fmt, ...) {
	va_list args;
	va_start(args, fmt);
	int result = vsnprintf(s, size, fmt, args);
	va_end(args);
	return result;
}

int sprintf(char* restrict s, const char* restrict fmt, ...) {
	va_list args;
	va_start(args, fmt);
	int result = vsprintf(s, fmt, args);
	va_end(args);
	return result;
}

int sscanf(const char* restrict s, const char* restrict fmt, ...) {
	va_list args;
	va_start(args, fmt);
	int result = vsscanf(s, fmt, args);
	va_end(args);
	return result;
}

// TODO
FILE* tmpfile(void) {
	UNIMP();
	return NULL;
}

// TODO
char* tmpnam(char* s) {
	UNIMP();
	return NULL;
}

// TODO
int ungetc(int, FILE* ) {
	UNIMP();
	return 0;
}

// TODO
int vdprintf(int fd, const char* restrict fmt, va_list args) {
	UNIMP();
	return 0;
}

#define CONVERSION_FLAGS_ALT_FORM	0x1
#define CONVERSION_FLAGS_ZERO_PAD	0x2
#define CONVERSION_FLAGS_LEFT_ADJ	0x4
#define CONVERSION_FLAGS_SPACE		0x8
#define CONVERSION_FLAGS_SIGN		0x10

// NOTE: - overrides 0, + overrides ' '
static unsigned int _parse_conversion_flags(const char* restrict* fmt) {
	unsigned int flags = 0;
	for (;; ++*fmt) {
		switch (**fmt) {
		case '#':
			flags |= CONVERSION_FLAGS_ALT_FORM;
			break;
		case '0':
			if (!(flags & CONVERSION_FLAGS_LEFT_ADJ)) {
				flags |= CONVERSION_FLAGS_ZERO_PAD;
			}
			break;
		case '-':
			flags |= CONVERSION_FLAGS_LEFT_ADJ;
			flags &= ~CONVERSION_FLAGS_ZERO_PAD;
			break;
		case ' ':
			if (!(flags & CONVERSION_FLAGS_SIGN)) {
				flags |= CONVERSION_FLAGS_SPACE;
			}
			break;
		case '+':
			flags |= CONVERSION_FLAGS_SIGN;
			flags &= ~CONVERSION_FLAGS_SPACE;
			break;
		default:
			return flags;
		}
	}
}

// NOTE: This function is for parsing numbers in format strings
static unsigned int _parse_decimal(const char* restrict* fmt) {
	if (**fmt < '0' || **fmt > '9') {
		// TODO: throw error
		return 0;
	}
	unsigned int value = 0;
	for (;; ++*fmt) {
		if (**fmt < '0' || **fmt > '9') {
			return value;
		}
		unsigned int digit = **fmt - '0';
		value *= 10;
		value += digit;
	}
}

static size_t _parse_conversion_length_modifier(const char* restrict* fmt) {
	char c = **fmt;
	++*fmt;
	switch (c) {
	case 'h':
		if (**fmt == 'h') {
			++*fmt;
			return sizeof(char);
		}
		return sizeof(short);
	case 'l':
		if (**fmt == 'l') {
			++*fmt;
			return sizeof(long long);
		}
		return sizeof(long);
	case 'j':
		return sizeof(intmax_t);
	case 'z':
		return sizeof(size_t);
	case 't':
		return sizeof(ptrdiff_t);
	default:
		--*fmt;
		return 0;
	}
}

static uint64_t _abs(int64_t x) {
	return x >= 0 ? x : -x;
}

static signed char _sgn(int64_t x) {
	return x / (int64_t)_abs(x);
}

static uint64_t _max_magnitude(unsigned int base) {
	switch (base) {
		case 8:
			return 01000000000000000000000ull;
		case 10:
			return 10000000000000000000ull;
		case 16:
			return 0x1000000000000000ull;
	}
	__builtin_unreachable();
}

static char _digit_to_ascii(unsigned int x, bool upper) {
	if (x <= 9) {
		return x + '0';
	}
	return x - 10 + (upper ? 'A' : 'a');
}

struct _GenericInt {
	uint64_t	value;
	int8_t		sign;
};

// Our lowest possible base is octal, which requires:
// - At most 1 sign character
// - At most 22 digit characters
#define MAX_INTEGER_CHARS	23

static int _print_integer(void (*char_func)(char, void*), void* data, struct _GenericInt integer, uint32_t base, bool upper, uint32_t flags, uint32_t width, int precision) {
	char int_string[MAX_INTEGER_CHARS];
	int length = 0;
	if (integer.sign == -1) {
		int_string[length++] = '-';
	}
	else if (flags & CONVERSION_FLAGS_SIGN) {
		int_string[length++] = '+';
	}
	else if (flags & CONVERSION_FLAGS_SPACE) {
		int_string[length++] = ' ';
	}
	if ((flags & CONVERSION_FLAGS_ALT_FORM) && base != 10
		&& (base == 8 || integer.value != 0)) {
		int_string[length++] = '0';
		if (base == 16) {
			int_string[length++] = upper ? 'X' : 'x';
		}
	}
	bool show_zeros = false;
	uint64_t magnitude = _max_magnitude(base);
	for (; magnitude > 0; magnitude /= base) {
		uint8_t digit = integer.value / magnitude;
		integer.value %= magnitude;
		if (digit != 0) {
			show_zeros = true;
		}
		if (digit != 0 || show_zeros) {
			int_string[length++] = _digit_to_ascii(digit, upper);
		}
	}
	if (!show_zeros && precision != 0) {
		int_string[length++] = '0';
	}
	const int zero_pad = precision > length ? precision - length : 0;
	const int space_pad = (int)width > length + zero_pad ? (int)width - length - zero_pad : 0;
	const char pad_char = (flags & CONVERSION_FLAGS_ZERO_PAD) ? '0' : ' ';
	if (space_pad != 0 && !(flags & CONVERSION_FLAGS_LEFT_ADJ)) {
		for (int i = 0; i < space_pad; ++i) {
			char_func(pad_char, data);
		}
	}
	for (int i = 0; i < zero_pad; ++i) {
		char_func('0', data);
	}
	for (int i = 0; i < length; ++i) {
		char_func(int_string[i], data);
	}
	if (space_pad != 0 && (flags & CONVERSION_FLAGS_LEFT_ADJ)) {
		for (int i = 0; i < space_pad; ++i) {
			char_func(pad_char, data);
		}
	}
	return length + zero_pad + space_pad;
}

static int _print_string(void (*char_func)(char, void*), void* data, const char* restrict s, int precision) {
	size_t length = strlen(s);
	if (precision != -1 && (size_t)precision < length) {
		length = (size_t)precision;
	}
	for (size_t i = 0; i < length; ++i) {
		char_func(s[i], data);
	}
	return length;
}

#define NUMBER	10000000000

static int _print_double(void (*char_func)(char, void*), void* data, double value) {
	int length = 2;
	if (value < 0.0) {
		char_func('-', data);
		++length;
	}
	const int exp = 1 + (int)floor(log10(value));
	double m = 1.0;
	if (exp >= 0) {
		for (int i = 0; i < exp; ++i) {
			m *= 10.0;
		}
	}
	else {
		for (int i = 0; i < -exp; ++i) {
			m *= 0.1;
		}
	}
	double digits = (value / m) * (double)NUMBER;
	uint64_t digits_int = (uint64_t)digits;
	char_func('0' + digits_int / NUMBER, data);
	digits_int %= NUMBER;
	char_func('.', data);
	for (uint64_t magnitude = NUMBER / 10; magnitude > 0; magnitude /= 10) {
		uint8_t digit = digits_int / magnitude;
		digits_int %= magnitude;
		char_func('0' + digit, data);
		++length;
	}
	if (exp != 0) {
		char_func('e', data);
		++length;
		struct _GenericInt integer = {
			.value = _abs(exp),
			.sign = _sgn(exp),
		};
		length += _print_integer(char_func, data, integer, 10, false, 0, 0, 0);
	}
	return length;
}

#define SIGNED		(1ull << 32)
#define UNSIGNED	0

static struct _GenericInt _generic_int(va_list* args, size_t size, size_t default_size, size_t is_signed) {
	struct _GenericInt integer = {};
	if (size == 0) {
		size = default_size;
	}
	switch (size | is_signed) {
	case 4 | SIGNED: {
		int32_t value = va_arg(*args, int32_t);
		integer.value = _abs(value);
		integer.sign = _sgn(value);
		break;
	 }
	case 4 | UNSIGNED: {
		integer.value = va_arg(*args, uint32_t);
		integer.sign = 1;
		break;
	 }
	case 8 | SIGNED: {
		int64_t value = va_arg(*args, int64_t);
		integer.value = _abs(value);
		integer.sign = _sgn(value);
		break;
	}
	case 8 | UNSIGNED: {
		integer.value = va_arg(*args, uint64_t);
		integer.sign = 1;
		break;
	}
	}
	return integer;
}

// This function is called whenever we hit a % in the format string
// *fmt points to the first character after the %
static int _print_arg(void (*char_func)(char, void*), void* data, const char* restrict* fmt, va_list* args) {
	unsigned int flags = _parse_conversion_flags(fmt);
	unsigned int width = _parse_decimal(fmt);
	int precision = -1;
	if (**fmt == '.') {
		++*fmt;
		precision = _parse_decimal(fmt);
		// TODO: only ignore 0 flag for d, i, o, u, x, X conversion specifiers (when precision is specified)
		flags &= ~CONVERSION_FLAGS_ZERO_PAD;
	}
	size_t size = _parse_conversion_length_modifier(fmt);
	switch (**fmt) {
	case 'd':
	case 'i':
		return _print_integer(char_func, data, _generic_int(args, size, 4, SIGNED), 10, false, flags, width, precision);
	case 'o':
		return _print_integer(char_func, data, _generic_int(args, size, 4, SIGNED), 8, false, flags, width, precision);
	case 'u':
		return _print_integer(char_func, data, _generic_int(args, size, 4, UNSIGNED), 10, false, flags, width, precision);
	case 'x':
		return _print_integer(char_func, data, _generic_int(args, size, 4, UNSIGNED), 16, false, flags, width, precision);
	case 'X':
		return _print_integer(char_func, data, _generic_int(args, size, 4, UNSIGNED), 16, true, flags, width, precision);
	case 'e':
	case 'E':
	case 'f':
	case 'F':
	case 'g':
	case 'G':
	case 'a':
		return _print_double(char_func, data, va_arg(*args, double) /* TODO: flags, width, precision */);
	case 'c':
		char_func((char)va_arg(*args, int), data);
		return 1;
	case 's':
		return _print_string(char_func, data, va_arg(*args, const char*), precision);
	case 'p':
		return _print_integer(char_func, data, _generic_int(args, size, 8, UNSIGNED), 16, false, flags, width, precision);
	case '%':
		char_func('%', data);
		return 1;
	default:
		// TODO: throw an error
		printf("ERROR: Unknown conversion specifier '%c'.\n", **fmt);
		while (**fmt != '%') {
			--*fmt;
		}
		printf("Format string was: \"%s\"\n", *fmt);
		abort();
	}
}

// Conversion specification format: %[argument$][flags][width][.precision][length modifier]conversion
// NOTE: currently we are ignoring the [argument$] part
static int _generic_printf(void (*char_func)(char, void*), void* data, const char* restrict fmt, va_list args) {
	int length = 0;
	for (; *fmt != '\0'; ++fmt) {
		if (*fmt != '%') {
			char_func(*fmt, data);
			++length;
		}
		else {
			++fmt;
			length += _print_arg(char_func, data, &fmt, &args);
		}
	}
	char_func('\0', data);
	return length;
}

static void _fprint_char_func(char c, void* data) {
	putc_unlocked(c, data);
}

int vfprintf(FILE* restrict stream, const char* restrict fmt, va_list args) {
	return _generic_printf(_fprint_char_func, stream, fmt, args);
}

// TODO
int vfscanf(FILE* restrict, const char* restrict, va_list) {
	UNIMP();
	return 0;
}

int vprintf(const char* restrict fmt, va_list args) {
	return vfprintf(stdout, fmt, args);
}

int vscanf(const char* restrict fmt, va_list args) {
	return vfscanf(stdin, fmt, args);
}

struct _SNPrintData {
	char*				it;
	const char* const	end;
};

static void _snprint_char_func(char c, void* data) {
	struct _SNPrintData* snprint_data = data;
	if (snprint_data->it == snprint_data->end) {
		return;
	}
	*snprint_data->it = c;
	++snprint_data->it;
}

int vsnprintf(char* restrict s, size_t size, const char* restrict fmt, va_list args) {
	if (size != 0) {
		s[size - 1] = '\0';
	}
	const size_t end_offset = size == 0 ? 0 : size - 1;
	struct _SNPrintData snprint_data = {
		.it = s,
		.end = s + end_offset,
	};
	return _generic_printf(_snprint_char_func, &snprint_data, fmt, args);
}

static void _sprint_char_func(char c, void* data) {
	char** it = data;
	**it = c;
	++*it;
}

int vsprintf(char* restrict s, const char* restrict fmt, va_list args) {
	char* it = s;
	return _generic_printf(_sprint_char_func, &it, fmt, args);
}

// TODO
int vsscanf(const char* restrict, const char* restrict, va_list) {
	UNIMP();
	return 0;
}
