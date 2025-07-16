#include "syscall.h"
#include "proc.h"
#include "fs.h"
#include "io.h"
#include "exception.h"
#include "regs.h"

u64 syscall_time(void) {
	return SYS_TIMER->clo / 0x100000;
}
