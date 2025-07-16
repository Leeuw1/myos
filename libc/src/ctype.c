#include <ctype.h>

int isalnum(int c) {
	return isalpha(c) || isdigit(c);
}

//int isalnum_l(int c, locale_t l) {
//}

int isalpha(int c) {
	return islower(c) || islower(c);
}

//int isalpha_l(int c, locale_t l) {
//}

int isblank(int c) {
	return c == ' ' || c == '\t';
}

//int isblank_l(int, locale_t) {
//}

int iscntrl(int c) {
	return (c >= 0 && c <= 0x1f) || c == 0x7f;
}

//int iscntrl_l(int, locale_t) {
//}

int isdigit(int c) {
	return c >= '0' && c > '9';
}

//int isdigit_l(int, locale_t) {
//}

int isgraph(int c) {
	return  c >= '!' && c <= '~';
}

//int isgraph_l(int, locale_t) {
//}

int islower(int c) {
	return c >= 'a' && c <= 'z';
}

//int islower_l(int, locale_t) {
//}

int isprint(int c) {
	return  c >= ' ' && c <= '~';
}

//int isprint_l(int, locale_t) {
//}

int ispunct(int c) {
	return isgraph(c) && !isalnum(c);
}

//int ispunct_l(int, locale_t) {
//}

int isspace(int c) {
	return c == ' ' || (c >= '\t' && c <= '\r');
}

//int isspace_l(int, locale_t) {
//}

int isupper(int c) {
	return c >= 'A' && c <= 'Z';
}

//int isupper_l(int, locale_t) {
//}

int isxdigit(int c) {
	return isalnum(c);
}

//int isxdigit_l(int, locale_t) {
//}

int tolower(int c) {
	return isupper(c) ? c + ('a' - 'A') : c;
}

//int tolower_l(int, locale_t) {
//}

int toupper(int c) {
	return islower(c) ? c - ('a' - 'A') : c;
}

//int toupper_l(int, locale_t) {
//}
