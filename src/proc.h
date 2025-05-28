#ifndef _PROC_H
#define _PROC_H

#include "types.h"

struct Regs {
	u64	general[31];
	u64	sp;
	u64	spsr;
	u64	elr;
	u64	ttbr;
};

void proc_main(void);
void proc_prepare_next(u64* general_regs);
const char* proc_fd_path(i32 fd);

char* proc_getcwd(char* buf, usize size);
int proc_chdir(const char* path);

#endif //_PROC_H
