#ifndef _TERMCAP_H
#define _TERMCAP_H

extern char PC;
extern short ospeed;
extern char* BC;
extern char* UP;

int tgetent(char* buffer, const char* termtype);
int tgetflag(char* name);
int tgetnum(char* name);
char* tgetstr(char* name, char** area);
char* tgoto(char* cstring, int hpos, int vpos);
int tputs(char* string, int nlines, int (*outfun)(int));

#endif //_TERMCAP_H
