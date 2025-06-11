#include "io.h"
#include "proc.h"
#include "heap.h"
#include "fs.h"
#include "exception.h"
#include <string.h>

extern u8 _TEXT_START_;
extern u8 _TEXT_END_;
extern u8 _DATA_START_;
extern u8 _DATA_END_;
extern u8 _BSS_START_;
extern u8 _BSS_END_;
extern u8 _HEAP_START_;

_Noreturn void kmain(void) {
	// Unmask error related exceptions
	asm ("msr daifclr, #0b1100");

	print("[kernel] Program Linking Information:\n");
	printf("_TEXT_START_:\t%\n", (u64)&_TEXT_START_);
	printf("_TEXT_END_:\t%\n", (u64)&_TEXT_END_);
	printf("_DATA_START_:\t%\n", (u64)&_DATA_START_);
	printf("_DATA_END_:\t%\n", (u64)&_DATA_END_);
	printf("_BSS_START_:\t%\n", (u64)&_BSS_START_);
	printf("_BSS_END_:\t%\n", (u64)&_BSS_END_);
	printf("_HEAP_START_:\t%\n", (u64)&_HEAP_START_);
	memset(&_BSS_START_, 0, &_BSS_END_ - &_BSS_START_);

	print("[kernel] Initializing heap...\n");
	heap_init();
	print("[kernel] Initializing filesystem...\n");
	fs_init();

	//mailbox_create_framebuffer(&framebuffer);

	if (mmu_enabled()) {
		print("[kernel] Using MMU\n");
	}

	print("[kernel] Welcome to my OS\n");
	print("[kernel] Build info: ");
	print(MYOS_BUILD_INFO);
	putchar('\n');

	// Start shell
	proc_main();
}
