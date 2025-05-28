#include "io.h"
#include "proc.h"
#include "commands.h"
#include "heap.h"
#include "fs.h"
#include "exception.h"
#include <string.h>

#define MAX_COMMAND_LENGTH 63

void print_prompt(void) {
	char cwd[64];
	proc_getcwd(cwd, 64);
	print(cwd);
	print(" $ ");
}

extern u8 _TEXT_START_;
extern u8 _TEXT_END_;
extern u8 _DATA_START_;
extern u8 _DATA_END_;
extern u8 _BSS_START_;
extern u8 _BSS_END_;
extern u8 _HEAP_START_;

void kmain(void) {
	print("[kernel] Program Linking Information:\n");
	printf("_TEXT_START_: %\n", (u64)&_TEXT_START_);
	printf("_TEXT_END_: %\n", (u64)&_TEXT_END_);
	printf("_DATA_START_: %\n", (u64)&_DATA_START_);
	printf("_DATA_END_: %\n", (u64)&_DATA_END_);
	printf("_BSS_START_: %\n", (u64)&_BSS_START_);
	printf("_BSS_END_: %\n", (u64)&_BSS_END_);
	printf("_HEAP_START_: %\n", (u64)&_HEAP_START_);
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

	char command[MAX_COMMAND_LENGTH + 1] = {};
	u8 command_length = 0;
	print_prompt();

	while (1) {
		char c = tty_getchar();
		switch (c) {
		case 0x0:
			break;
		case 0x7f:
		case '\b':
			if (command_length > 0) {
				print("\b \b");
				--command_length;
			}
			break;
		case '\r':
		case '\n':
			putchar('\n');
			if (command_length > 0) {
				command[command_length] = '\0';
				syscall_command(command, command_length);
				command_length = 0;
			}
			print_prompt();
			break;
		default:
			if (c < ' ' || c > '~') {
				break;
			}
			putchar(c);
			if (command_length < MAX_COMMAND_LENGTH) {
				command[command_length++] = c;
			}
			break;
		}
	}
	__builtin_unreachable();
}
