#include "commands.h"
#include "mailbox.h"
#include "io.h"
#include "regs.h"
#include "exception.h"
#include "proc.h"
#include "fs.h"
#include "sd.h"
#include "mbr.h"
#include "fat.h"
#include <string.h>

#define EXIT_SUCCESS	0
#define	EXIT_FAILURE	1

static u8 ascii_digit_value(char c) {
	if (c >= 'a') {
		return c - 'a' + 10;
	}
	if (c >= 'A') {
		return c - 'A' + 10;
	}
	return c - '0';
}

static u64 str_to_u64(const char* str) {
	u64 base = 10;
	u64 value = 0;
	for (const char* it = str; *it != '\0'; ++it) {
		// TODO: maybe just check at the beginning
		if (it != str && *it == 'x') {
			if (*(it - 1) == '0') {
				base = 16;
				continue;
			}
		}
		
		value *= base;
		value += ascii_digit_value(*it);
	}
	return value;
}

/*
static void pcm_wait_2_clocks() {
	PCM->cs_a.sync = 1;
	while (!PCM->cs_a.sync) {
	}
	PCM->cs_a.sync = 0;
}
*/

/*
// NOTE: use this when FIFOs are disabled
i32 pcm_clock_test(u64 argc, const char* argv[]) {
	(void)argc;
	(void)argv;
	PCM->cs_a.en = 1;
	u32 time = SYS_TIMER->clo;
	pcm_wait_2_clocks();
	time = (SYS_TIMER->clo - time) / 2;
	PCM->cs_a.en = 0;
	printf("1 PCM clock = % SYS_TIMER clocks\n", (u64)time);
	printf("pcm_clocks_per_sec = %\n", 0x100000 / (u64)time);
	return EXIT_SUCCESS;
}
*/

// NOTE: these are like assembly labels, so we want their ADDRESS rather than their VALUE
extern u8 _TEXT_START_;
extern u8 _TEXT_END_;
extern u8 _DATA_START_;
extern u8 _DATA_END_;
extern u8 _BSS_START_;
extern u8 _BSS_END_;
extern u8 _HEAP_START_;

i32 gpu_test(u8 argc, const char* argv[]) {
	(void)argc;
	(void)argv;
	volatile u32* v3d_regs = (volatile u32*)0x3fc00000;
	u32 ident0 = v3d_regs[0];
	u32 ident1 = v3d_regs[1];
	u32 ident2 = v3d_regs[2];
	printf("ident0: %\n", (u64)ident0);
	printf("ident1: %\n", (u64)ident1);
	printf("ident2: %\n", (u64)ident2);
	return EXIT_SUCCESS;
}

static bool power_info(const char* str, u32* device_id, bool* domain) {
	if (strcmp(str, "v3d") == 0) {
		*device_id = 11;
		*domain = true;
		return true;
	}
	if (strcmp(str, "sd") == 0) {
		*device_id = 0;
		*domain = false;
		return true;
	}
	return false;
}

i32 power(u8 argc, const char* argv[]) {
	if (argc < 2) {
		for (u32 i = 1; i < 24; ++i) {
			printf("%: ", (u64)i);
			if (mailbox_get_power(i, true) != 0) {
				break;
			}
		}
		return EXIT_SUCCESS;
	}
	//u32 device_id = (u32)str_to_u64(argv[1]);
	u32 device_id;
	bool domain;
	if (!power_info(argv[1], &device_id, &domain)) {
		print("Unknown device/domain\n");
		return EXIT_FAILURE;
	}
	if (argc < 3) {
		mailbox_get_power(device_id, domain);
		return EXIT_SUCCESS;
	}
	u8 on;
	if (strcmp(argv[2], "on") == 0) {
		on = 1;
	}
	else if (strcmp(argv[2], "off") == 0) {
		on = 0;
	}
	else {
		print("Invalid power state\n");
		return EXIT_FAILURE;
	}
	mailbox_set_power(device_id, on, domain);
	return EXIT_SUCCESS;
}

i32 clear_screen(u8 argc, const char* argv[]) {
	(void)argv;
	if (argc < 2) {
		print("Not enough arguments\n");
		return EXIT_FAILURE;
	}
	//u64 value = str_to_u64(argv[1]);
	//screen_clear(value >> 16, (value >> 8) & 0xff, value & 0xff);
	return EXIT_SUCCESS;
}

i32 blank_screen(u8 argc, const char* argv[]) {
	if (argc < 2) {
		print("Not enough arguments\n");
		return EXIT_FAILURE;
	}
	mailbox_blank_screen((u32)str_to_u64(argv[1]));
	return EXIT_SUCCESS;
}

void make_abs(const char* path, char* abs_path) {
	// NOTE: assuming that the length of the resulting path will not exceed PAGE_SIZE
	abs_path[0] = '\0';
	if (path[0] == '\0') {
		return;
	}
	const char* rest = path;
	// Expand beginning
	if (path[0] != '/') {
		proc_getcwd(abs_path, 128);
		strcat(abs_path, "/");
		if (path[0] == '.' && path[1] == '/') {
			rest += 2;
		}
	}
	strcat(abs_path, rest);
}

i32 pwd(u8 argc, const char* argv[]) {
	(void)argc; (void)argv;
	char cwd[64];
	char* result = proc_getcwd(cwd, 64);
	if (result == NULL) {
		print("pwd: Buffer too small (cwd will not fit).\n");
		return EXIT_FAILURE;
	}
	print(cwd);
	print("\n");
	return EXIT_SUCCESS;
}

struct MBR mbr = {};

u32 read32_unaligned(const u32* data) {
	const u8* bytes = (u8*)data;
	return (u32)bytes[0] | ((u32)bytes[1] << 8) | ((u32)bytes[2] << 16) | ((u32)bytes[3] << 24);
}

u16 read16_unaligned(const u16* data) {
	const u8* bytes = (u8*)data;
	return (u16)bytes[0] | ((u16)bytes[1] << 8);
}

i32 fat_test(u8 argc, const char* argv[]) {
	(void)argc;
	(void)argv;

	// TODO: once this is working we will probably only call this function once
	if (!sd_init()) {
		print("sd_init() was unsuccessful\n");
		return EXIT_FAILURE;
	}

	print("sd_init() was successful\n");

	if (!sd_read_sectors(&mbr, 0, 1)) {
		print("sd_read_sectors() was unsuccessful\n");
		return EXIT_FAILURE;
	}
	print("sd_read_sectors() was successful\n");

	if (read16_unaligned(&mbr.magic) != 0xaa55) {
		print("magic bytes not found at end of MBR\n");
		return EXIT_FAILURE;
	}
	print("MBR magic OK\n");
#if 0
	for (usize i = 0; i < 4; ++i) {
		const u32 start_sector = read32_unaligned(&mbr.partitions[i].start_sector);
		const u32 sector_count = read32_unaligned(&mbr.partitions[i].sector_count);
		printf("Partition %:\n", (u64)i);
		printf("start_sector=%, sector_count=%, type=%, attr=%\n",
				(u64)start_sector, (u64)sector_count,
				(u64)mbr.partitions[i].type, (u64)mbr.partitions[i].attr);
	}
#endif
	const u32 start_sector = read32_unaligned(&mbr.partitions[0].start_sector);
	const u32 sector_count = read32_unaligned(&mbr.partitions[0].sector_count);
	printf("Partition: start_sector=%, sector_count=%\n",
			(u64)start_sector, (u64)sector_count);
	fat_mount_partition(start_sector, sector_count);
	return EXIT_SUCCESS;
}

struct CommandEntry {
	const char*	name;
	i32 (*func)(u8 argc, const char* argv[]);
};

#define ARRAY_SIZE(x)		(sizeof(x) / sizeof(x[0]))
#define COMMAND_ENTRY(x)	{ .name = #x, .func = x }

i32 help(u8 argc, const char* argv[]);
i32 shutdown(u8 argc, const char* argv[]);

const struct CommandEntry command_table[] = {
	COMMAND_ENTRY(help),
	COMMAND_ENTRY(shutdown),
	//COMMAND_ENTRY(pcm_clock_test),
	COMMAND_ENTRY(gpu_test),
	COMMAND_ENTRY(power),
	COMMAND_ENTRY(clear_screen),
	COMMAND_ENTRY(blank_screen),
	//COMMAND_ENTRY(snake),
	COMMAND_ENTRY(fat_test),
	COMMAND_ENTRY(pwd),
};

i32 shutdown(u8 argc, const char* argv[]) {
	(void)argc; (void)argv;
	print("[kernel] Shutting down...\n");
	asm ("msr daifset, #0b1111");
	while (true) {
	}
	mailbox_blank_screen(1);
	for (u32 i = 1; i < 23; ++i) {
		mailbox_set_power(i, 0, true);
	}
	asm (
		"halt:\n"
		"wfe\n"
		"b halt"
	);
	__builtin_unreachable();
}

i32 help(u8 argc, const char* argv[]) {
	(void)argc;
	(void)argv;
	print("Commands:\n");
	for (u64 i = 0; i < ARRAY_SIZE(command_table); ++i) {
		print("- ");
		print(command_table[i].name);
		putchar('\n');
	}
	return EXIT_SUCCESS;
}

i32 syscall_command(u8 argc, const char* argv[]) {
	for (u64 i = 0; i < ARRAY_SIZE(command_table); ++i) {
		if (strcmp(argv[0], command_table[i].name) == 0) {
			return command_table[i].func(argc, argv);
		}
	}
	return EXIT_FAILURE;
}
