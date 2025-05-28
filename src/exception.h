#ifndef _EXCEPTION_H
#define _EXCEPTION_H

#include "types.h"

void enter_el0(void);
void irq_enable(void);
void irq_disable(void);
u8 get_current_el(void);
bool mmu_enabled(void);
u64 try_translate(void* addr);

#endif //_EXCEPTION_H
