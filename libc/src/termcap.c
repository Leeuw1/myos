#include <termcap.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>

#define UNIMP()			printf("[libc] Warning: %s is not implemented.\n", __FUNCTION__);

#define HASH(c1, c2)		(((uint16_t)c1 << 8) | (uint16_t)c2)
#define HASH_STR(str)		HASH(str[0], str[1])

char PC;
short ospeed;
char* BC;
char* UP;

int tgetent(char* buffer, const char* termtype) {
	//UNIMP();
	return 1;
}

int tgetflag(char* name) {
	switch (HASH_STR(name)) {
	case HASH('a', 'm'):
	case HASH('x', 'n'):
	case HASH('h', 's'):
	case HASH('m', 'i'):
	case HASH('m', 's'):
	case HASH('o', 's'):
	case HASH('N', 'P'):
	case HASH('c', 'c'):
	case HASH('u', 't'):
		return 1;
	default:
		return 0;
	}
}

int tgetnum(char* name) {
	switch (HASH_STR(name)) {
	case HASH('l', 'i'):
		return 40;
	case HASH('c', 'o'):
		return 180;
	case HASH('C', 'o'):
		return 256;
	case HASH('i', 't'):
		return 8;
	case HASH('p', 'a'):
		return 32767;
	default:
		return -1;
	}
}

char* tgetstr(char* name, char** area) {
	(void)area;
	switch (HASH_STR(name)) {
	default:
		return NULL;
	}
}

char* tgoto(char* cstring, int hpos, int vpos) {
	static char buf[32];
	size_t buf_count = 0;
	int argc = 0;
	for (const char* it = cstring; *it != '\0'; ++it) {
		if (*it == '%') {
			++argc;
		}
	}
	for (const char* it = cstring; *it != '\0'; ++it) {
		if (*it != '%') {
			buf[buf_count++] = *it;
			continue;
		}
		++it;
		if (*it == 'i') {
			++hpos;
			++vpos;
			--argc;
			continue;
		}
		if (*it != 'd') {
			printf("termcap: Error, unsupported %% sequence.\n");
			return NULL;
		}
		int arg = argc == 2 ? vpos : hpos;
		buf[buf_count++] = '0' + arg / 100;
		arg %= 100;
		buf[buf_count++] = '0' + arg / 10;
		arg %= 10;
		buf[buf_count++] = '0' + arg;
		--argc;
	}
	buf[buf_count] = '\0';
	return buf;
}

int tputs(char* string, int nlines, int (*outfun)(int)) {
	(void)nlines; (void)outfun;
	for (size_t i = 0; string[i] != '\0'; ++i) {
		outfun(string[i]);
	}
	return 0;
}

#if 0
#define HASH(c1, c2)		(((uint16_t)c1 << 8) | (uint16_t)c2)
#define HASH_STR(str)		HASH(str[0], str[1])
#define CASE(c1, c2, str)	case HASH(c1, c2): return str

char* tgetstr(char* name, char** area) {
	(void)area;
	switch (HASH_STR(name)) {
	default:
		printf("tgetstr('%s')\n", name);
		return NULL;
	CASE('c', 'e', "ce");
	CASE('a', 'l', "al");
	CASE('A', 'L', "AL");
	CASE('d', 'l', "dl");
	CASE('D', 'L', "DL");
	CASE('c', 'l', "cl");
	CASE('m', 'e', "me");
	CASE('m', 'r', "mr");
	CASE('m', 's', "ms");
	CASE('u', 't', "ut");
	CASE('l', 'e', "le");
	CASE('c', 'm', "cm");
	CASE('R', 'I', "RI");
	}
}

#undef CASE

char* tgoto(char* cstring, int hpos, int vpos) {
	switch (HASH_STR(cstring)) {
	case HASH('c', 'm'):
		printf("\033[%d;%dH", vpos + 1, hpos + 1);
		return cstring;
	case HASH('R', 'I'):
		printf("\033[%dC", hpos);
		return cstring;
	case HASH('A', 'L'):
		printf("\033[%dL", hpos);
		return cstring;
	case HASH('D', 'L'):
		printf("\033[%dM", hpos);
		return cstring;
	default:
		printf("tgoto(%s, %d, %d)", cstring, hpos, vpos);
		return NULL;
	}
}

/*
	// From vim source code
	static tcap_entry_T builtin_ansi[] = {
		{(int)KS_CE,	"\033[K"},
		{(int)KS_AL,	"\033[L"},
		{(int)KS_CAL,	"\033[%dL"},
		{(int)KS_DL,	"\033[M"},
		{(int)KS_CDL,	"\033[%dM"},
		{(int)KS_CL,	"\033[H\033[2J"},
		{(int)KS_ME,	"\033[0m"},
		{(int)KS_MR,	"\033[7m"},
		{(int)KS_MS,	"y"},
		{(int)KS_UT,	"y"},		// guessed
		{(int)KS_LE,	"\b"},
		{(int)KS_CM,	"\033[%i%d;%dH"},
		{(int)KS_CRI,	"\033[%dC"},
		{(int)KS_NAME,	NULL}  // end marker
	};
*/

int tputs(char* string, int nlines, int (*outfun)(int)) {
	(void)outfun;
	switch (HASH_STR(string)) {
	default:
		printf("tputs(%s, %d)", string, nlines);
		return -1;
	case HASH('c', 'e'):
		printf("\033[K");
		return 0;
	case HASH('a', 'l'):
		printf("\033[L");
		return 0;
	case HASH('d', 'l'):
		printf("\033[M");
		return 0;
	case HASH('c', 'l'):
		printf("\033[H\033[2J");
		return 0;
	case HASH('m', 'e'):
		printf("\033[0m");
		return 0;
	case HASH('m', 'r'):
		printf("\033[7m");
		return 0;
	case HASH('m', 's'):
		putchar('y');
		return 0;
	case HASH('u', 't'):
		putchar('y');
		return 0;
	case HASH('l', 'e'):
		//putchar('\b');
		printf("\033[D");
		return 0;
	case HASH('c', 'm'):
		//printf("\e[H");
		return -1;
	case HASH('R', 'I'):
		printf("\e[C");
		return 0;
	case HASH('A', 'L'):
		printf("\e[L");
		return 0;
	case HASH('D', 'L'):
		printf("\e[M");
		return 0;
	}
}
#endif
