#include <semaphore.h>

int sem_init(sem_t* sem, int pshared, unsigned value) {
	(void)pshared;
	*sem = value;
	return 0;
}

int sem_post(sem_t* sem) {
	++*sem;
	return 0;
}

int sem_wait(sem_t* sem) {
	sem_t old_value;
	while (1) {
		asm volatile (
			"ldar	%w1, [%2]"
			: "=m" (*sem), "=r" (old_value)
			: "r" (sem)
		);
		if (old_value != 0) {
			break;
		}
	}
	asm volatile (
		"stlr	wzr, [%1]"
		: "=m" (*sem)
		: "r" (sem)
	);
	return 0;
}
