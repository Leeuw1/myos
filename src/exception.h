#ifndef _EXCEPTION_H
#define _EXCEPTION_H

#include "core.h"

void irq_enable(void);
void irq_disable(void);
u8 get_current_el(void);
bool mmu_enabled(void);
u64 try_translate(const void* addr);

#endif //_EXCEPTION_H
