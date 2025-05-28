#ifndef _HEAP_H
#define _HEAP_H

#include "types.h"

// Kernel heap functions
void heap_init(void);
void* kmalloc(usize size);
void* kmalloc_page_align(usize size);
void kfree(void* addr);
void* krealloc(void* addr, usize size);

#endif //_HEAP_H
