#include "io.h"
#include "proc.h"
#include "heap.h"
#include "fs.h"
#include "commands.h"
#include <string.h>

extern u8 _TEXT_START_;
extern u8 _TEXT_END_;
extern u8 _DATA_START_;
extern u8 _DATA_END_;
extern u8 _BSS_START_;
extern u8 _BSS_END_;
extern u8 _HEAP_START_;

_Noreturn void kmain(void) {
	// Invalidate the identity mapping, we are using kernel addresses now
	asm (
		"tlbi	vaae1, xzr\n"
		"isb"
	);
	// Unmask error related exceptions
	asm ("msr daifclr, #0b1100");

	memset(&_BSS_START_, 0, &_BSS_END_ - &_BSS_START_);

	tty_init();

	print("[kmain] Initializing heap...\n");
	heap_init();

	fat_test(0, NULL);

#if 0
	print("[kmain] Program Linking Information:\n");
	printf("_TEXT_START_:\t%\n", (u64)&_TEXT_START_);
	printf("_TEXT_END_:\t%\n", (u64)&_TEXT_END_);
	printf("_DATA_START_:\t%\n", (u64)&_DATA_START_);
	printf("_DATA_END_:\t%\n", (u64)&_DATA_END_);
	printf("_BSS_START_:\t%\n", (u64)&_BSS_START_);
	printf("_BSS_END_:\t%\n", (u64)&_BSS_END_);
	printf("_HEAP_START_:\t%\n", (u64)&_HEAP_START_);
#endif

	print("[kmain] Initializing filesystem...\n");
	fs_init();

	//mailbox_create_framebuffer(&framebuffer);

	print("[kmain] Build info: ");
	print(MYOS_BUILD_INFO);
	putchar('\n');
	print("[kmain] Welcome to my OS\n");

	// Start shell
	proc_main();
}
