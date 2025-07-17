#ifndef _SEMAPHORE_H
#define _SEMAPHORE_H

#include <stdint.h>

typedef uint32_t	sem_t;

int sem_init(sem_t* sem, int pshared, unsigned value);
int sem_post(sem_t* sem);
int sem_wait(sem_t* sem);

#endif //_SEMAPHORE_H
