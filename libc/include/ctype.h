#ifndef _CTYPE_H
#define _CTYPE_H

int isalnum(int);
//int isalnum_l(int, locale_t);
#define isalnum_l(c, l)	isalnum(c)
int isalpha(int);
//int isalpha_l(int, locale_t);
#define isalpha_l(c, l)	isalpha(c)
int isblank(int);
//int isblank_l(int, locale_t);
#define isblank_l(c, l)	isblank(c)
int iscntrl(int);
//int iscntrl_l(int, locale_t);
#define iscntrl_l(c, l)	iscntrl(c)
int isdigit(int);
//int isdigit_l(int, locale_t);
#define isdigit_l(c, l)	isdigit(c)
int isgraph(int);
//int isgraph_l(int, locale_t);
#define isgraph_l(c, l)	isgraph(c)
int islower(int);
//int islower_l(int, locale_t);
#define islower_l(c, l)	islower(c)
int isprint(int);
//int isprint_l(int, locale_t);
#define isprint_(c, l)	isprint(c)
int ispunct(int);
//int ispunct_l(int, locale_t);
#define ispunct_l(c, l)	ispunct(c)
int isspace(int);
//int isspace_l(int, locale_t);
#define isspace_l(c, l)	isspace(c)
int isupper(int);
//int isupper_l(int, locale_t);
#define isupper_l(c, l)	isupper(c)
int isxdigit(int);
//int isxdigit_l(int, locale_t);
#define isxdigit_l(c, l)	isxdigit(c)
int tolower(int);
//int tolower_l(int, locale_t);
#define tolower_l(c, l)	tolower(c)
int toupper(int);
//int toupper_l(int, locale_t);
#define toupper_l(c, l)	toupper(c)

#endif //_CTYPE_H
