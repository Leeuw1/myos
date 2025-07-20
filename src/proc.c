#include "proc.h"
#include "heap.h"
#include "elf.h"
#include "fs.h"
#include "io.h"
#include "regs.h"
#include "exception.h"
#include "commands.h"
#include "myos_time.h"
#include <string.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <termios.h>
#include <limits.h>
#define STDIO_NO_FUNCTIONS
#include <stdio.h>
#include <unistd.h>

#define MAX_PROCS		8
#define PATH_MAX_LENGTH	127
// Max file descriptors that a process can have open at the same time
#define MAX_FD_ENTRIES	8

#define L1_TABLE_ENTRY_COUNT	4
#define L2_TABLE_ENTRY_COUNT	512
#define L3_TABLE_ENTRY_COUNT	512

#define L3_TABLE_COUNT			11

#define DESC_FLAGS_TABLE		0b11
#define DESC_FLAGS_BLOCK		0b01
#define DESC_FLAGS_PAGE			0b11
#define DESC_FLAGS_ATTR0		0
#define DESC_FLAGS_ATTR1		BIT(2)
#define DESC_FLAGS_AP_PRW		(0b00 << 6)
#define DESC_FLAGS_AP_RW		(0b01 << 6)
#define DESC_FLAGS_AP_PRO		(0b10 << 6)
#define DESC_FLAGS_AP_RO		(0b11 << 6)
#define DESC_FLAGS_TABLE_AP_PRW	(0b00ull << 61)
#define DESC_FLAGS_TABLE_AP_RW	(0b01ull << 61)
#define DESC_FLAGS_TABLE_AP_PRO	(0b10ull << 61)
#define DESC_FLAGS_TABLE_AP_RO	(0b11ull << 61)
#define DESC_FLAGS_AF			BIT(10)
#define DESC_FLAGS_NG			BIT(11)
#define DESC_FLAGS_PXN			BIT(53)
#define DESC_FLAGS_UXN			BIT(54)

//#define PROC_TERM_LINE_BUF_SIZE	128

enum {
	PROC_STATE_RUN = 0,
	PROC_STATE_WAIT_READ,
	PROC_STATE_WAIT_PID,
	PROC_STATE_SLEEP,
};

struct FDEntry {
	struct FSNode*	node;
	isize			offset;
};

struct L1Table {
	u64	entries[L1_TABLE_ENTRY_COUNT];
} ALIGN(32);
struct L2Table {
	u64	entries[L2_TABLE_ENTRY_COUNT];
} ALIGN(PAGE_SIZE);
struct L3Table {
	u64	entries[L3_TABLE_ENTRY_COUNT];
} ALIGN(PAGE_SIZE);

#define HEAP_BLOCK_SIZE	0x400000

struct HeapBlock {
	u8					data[HEAP_BLOCK_SIZE];
	struct HeapBlock*	next;
};

usize _l2_indices[L3_TABLE_COUNT] = {
	0,
	1,
	2,
	0x80,
	0x81,
	0x82,
	0x83,
	0x84,
	0x85,
	0x86,
	0x1fe
};

struct Proc {
	struct L3Table	l3_tables[L3_TABLE_COUNT];
	struct L2Table	l2_table;
	struct L1Table	l1_table;
	struct Regs		regs;
	struct FDEntry	fd_table[MAX_FD_ENTRIES];
	char			cwd[PATH_MAX_LENGTH + 1];
	//char			term_line[PROC_TERM_LINE_BUF_SIZE];
	void*			image;
	struct {
		void*		handler;
		sigset_t	mask;
		i32			flags;
	} sigactions[SIGNAL_COUNT];
	void*			sigaction_wrapper;
	siginfo_t		signal_info;
	//usize			term_line_count;
	//usize			term_line_cursor;
	usize			size;
	usize			heap_size;
	struct HeapBlock*	heap_blocks;
	//u64			clock;
	union {
		struct {
			struct FSNode*	node;
			void*			dst;
			usize			size;
		} wait_read;
		i16					wait_pid;
		struct timespec		wait_time;
	};
	i32				signal;
	i32				prev_signal;
	i16				id;
	u8				state;
};

static struct Proc* _procs[MAX_PROCS];
static usize _proc_count;
static usize _current;

static _Noreturn void _proc_load_regs_and_run(void);
static i32 _proc_program(struct Proc* proc, const char* path, const char* argv[]);

static i32 _proc_new_id(void) {
	static i16 id = 1;
	return id++;
}

// TODO: make sure cwd is valid
static void _proc_init(struct Proc* proc, i16 id, const char* cwd) {
	memset(proc, 0, sizeof *proc);
	proc->id = id;
	strncpy(proc->cwd, cwd, PATH_MAX_LENGTH);

	proc->l1_table.entries[0] = ((u64)&proc->l2_table & 0xffffffff)
		| DESC_FLAGS_TABLE;
	for (usize i = 0; i < L3_TABLE_COUNT; ++i) {
		proc->l2_table.entries[_l2_indices[i]] = ((u64)&proc->l3_tables[i] & 0xffffffff)
			| DESC_FLAGS_TABLE | DESC_FLAGS_AF;
	}

	u32 tty_id;
	const i32 result = fs_find(&tty_id, "/dev/tty");
	if (result != 0) {
		PRINT_ERROR("Could not find node for /dev/tty.");
	}
	struct FSNode* tty = fs_open(tty_id);
	proc->fd_table[0].node = tty;
	proc->fd_table[1].node = tty;
	proc->fd_table[2].node = tty;
}

static struct Proc* _proc_create(const char* cwd) {
	if (_proc_count == MAX_PROCS) {
		PRINT_ERROR("_proc_count == MAX_PROCS");
		return NULL;
	}
	struct Proc* proc = kmalloc_page_align(sizeof *proc);
	_proc_init(proc, _proc_new_id(), cwd);
#if 1
	printf("New process: % (pid=%)\n", (u64)proc, (u64)proc->id);
#endif
	_procs[_proc_count++] = proc;
	return proc;
}

// Entry point to process-handling code
void proc_main(void) {
	_proc_count = 0;
	_current = 0;
	const char* argv[] = { "/bin/sh", NULL };
	const i32 result = _proc_program(_proc_create("/"), "/bin/sh", argv);
	if (result != 0) {
		printf("[proc] _proc_program() failed (result=%).\n", (u64)result);
		shutdown(0, NULL);
	}
	print("[proc] Starting system shell...\n");
	AUX_MU->ier_reg = 0b1;
	_proc_load_regs_and_run();
}

#define SAVED_GENERAL_REGS	(const void*)(0xffffffff0007ff00)

static void _proc_store_regs(struct Proc* proc) {
	memcpy(proc->regs.general, SAVED_GENERAL_REGS, sizeof proc->regs.general);
	u64	sp;
	u64	spsr;
	u64	elr;
	u64	tpidrro;
	asm (
		"mrs	%0, sp_el0\n"
		"mrs	%1, spsr_el1\n"
		"mrs	%2, elr_el1\n"
		"mrs	%3, tpidrro_el0"
		: "=r" (sp), "=r" (spsr), "=r" (elr), "=r" (tpidrro)
	);
	proc->regs.sp = sp;
	proc->regs.spsr = spsr;
	proc->regs.elr = elr;
	proc->regs.tpidrro = tpidrro;
}

static void _proc_schedule_next(void) {
	// Provide an opportunity for UART IRQ
	INTERRUPTS->disable_irqs_1 = 1;
	INTERRUPTS->enable_irqs_1 = 1 << 29;
	irq_enable();
	do {
		_current = (_current + 1) % _proc_count;
		if (_procs[_current]->state == PROC_STATE_SLEEP) {
			const struct timespec now = time_current();
			if (now.tv_sec > _procs[_current]->wait_time.tv_sec) {
				_procs[_current]->state = PROC_STATE_RUN;
				return;
			}
			if (now.tv_sec == _procs[_current]->wait_time.tv_sec
				&& now.tv_nsec >= _procs[_current]->wait_time.tv_nsec) {
				_procs[_current]->state = PROC_STATE_RUN;
				return;
			}
		}
	}
	while (_procs[_current]->state != PROC_STATE_RUN);
}

static void _proc_use_ttbr(const struct Proc* proc) {
	const u64 ttbr = ((u64)proc->id << 48) | ((u64)&proc->l1_table & 0xffffffff);
	asm volatile (
		"msr	ttbr0_el1, %0\n"
		"isb"
		:
		: "r" (ttbr)
	);
}

// Round up to the nearest multiple of 16 bytes
#define SIGINFO_SIZE	(((sizeof(siginfo_t) + 15) / 16) * 16)

// NOTE: We are assuming that _proc_use_ttbr() has been called
static void _proc_prepare_signal_handler(struct Proc* proc) {
	proc->regs.sp -= 256;
	u64* regs = (u64*)proc->regs.sp;
	memcpy(regs, proc->regs.general, sizeof proc->regs.general);
	regs[31] = proc->regs.elr;
	proc->regs.sp -= SIGINFO_SIZE;
	siginfo_t* info = (siginfo_t*)proc->regs.sp;
	memcpy(info, &proc->signal_info, sizeof *info);
	memset(proc->regs.general, 0, sizeof proc->regs.general);
	// Args: int sig, siginfo_t* info, void* context, void* handler
	proc->regs.general[0] = proc->signal;
	const usize index = SIGNAL_INDEX(proc->signal);
	if (proc->sigactions[index].flags & SA_SIGINFO) {
		proc->regs.general[1] = (u64)info;
	}
	proc->regs.general[3] = (u64)proc->sigactions[index].handler;
	// Wrapper function should call handler and then do a sigreturn syscall
	proc->regs.elr = (u64)proc->sigaction_wrapper;
}

// TODO
static void _proc_sig_dfl(struct Proc* proc) {
	PRINT_ERROR("Not yet implemented.");
	printf("SIGNAL: %\n", (u64)proc->signal);
	proc_exit(1);
}

static bool _proc_fatal_signal(i32 sig) {
	switch (sig) {
	case SIGSEGV:
	case SIGILL:
		return true;
	default:
		return false;
	}
}

static _Noreturn void _proc_load_regs_and_run(void) {
	struct Proc* proc = _procs[_current];
	_proc_use_ttbr(proc);
	if (proc->signal != 0) {
		// If we catch a fatal signal twice, kill process using _proc_sig_dfl
		const void* handler = proc->sigactions[SIGNAL_INDEX(proc->signal)].handler;
		if (handler == SIG_DFL
			|| (_proc_fatal_signal(proc->signal) && proc->signal == proc->prev_signal)) {
			_proc_sig_dfl(proc);
		}
		else if (handler != SIG_IGN) {
			_proc_prepare_signal_handler(proc);
		}
		proc->prev_signal = proc->signal;
		proc->signal = 0;
	}
	// Set timer for time slice
	SYS_TIMER->cs = 1;
	SYS_TIMER->c0 = SYS_TIMER->clo + 0xffff;
	INTERRUPTS->disable_irqs_1 = 1 << 29;
	INTERRUPTS->enable_irqs_1 = 1;
	irq_enable();
	const u64 sp = proc->regs.sp;
	const u64 spsr = proc->regs.spsr;
	const u64 elr = proc->regs.elr;
	const u64 tpidrro = proc->regs.tpidrro;
#if 0
	print("Switching process\n");
	printf("pid:\t%\n", (u64)proc->id);
	printf("SP:\t%\n", sp);
	printf("SPSR:\t%\n", spsr);
	printf("ELR:\t%\n", elr);
#endif
	asm volatile (
		"msr	sp_el0, %0\n"
		"msr	spsr_el1, %1\n"
		"msr	elr_el1, %2\n"
		"msr	tpidrro_el0, %3\n"
		"isb"
		:
		: "r" (sp), "r" (spsr), "r" (elr), "r" (tpidrro)
	);
	const void* general_regs = proc->regs.general;
	asm volatile (
		"mov	sp, %0\n"
		"ldp	x0, x1, [sp, #16 * 0]\n"
		"ldp	x2, x3, [sp, #16 * 1]\n"
		"ldp	x4, x5, [sp, #16 * 2]\n"
		"ldp	x6, x7, [sp, #16 * 3]\n"
		"ldp	x8, x9, [sp, #16 * 4]\n"
		"ldp	x10, x11, [sp, #16 * 5]\n"
		"ldp	x12, x13, [sp, #16 * 6]\n"
		"ldp	x14, x15, [sp, #16 * 7]\n"
		"ldp	x16, x17, [sp, #16 * 8]\n"
		"ldp	x18, x19, [sp, #16 * 9]\n"
		"ldp	x20, x21, [sp, #16 * 10]\n"
		"ldp	x22, x23, [sp, #16 * 11]\n"
		"ldp	x24, x25, [sp, #16 * 12]\n"
		"ldp	x26, x27, [sp, #16 * 13]\n"
		"ldp	x28, x29, [sp, #16 * 14]\n"
		"ldr	x30, [sp, #16 * 15]\n"
		"eret"
		:
		: "r" (general_regs)
	);
	__builtin_unreachable();
}

_Noreturn void proc_run_next(void) {
	_proc_store_regs(_procs[_current]);
	_proc_schedule_next();
	_proc_load_regs_and_run();
}

static isize _proc_table_index(usize index) {
	for (usize i = 0; i < L3_TABLE_COUNT; ++i) {
		if (_l2_indices[i] == index) {
			return i;
		}
	}
	return -1;
}

void _proc_map_pages(struct Proc* proc, u64 virt_addr, u64 kernel_addr, usize size, u32 flags) {
	const usize page_count = (size + (virt_addr & 0xfff) + PAGE_SIZE - 1) >> 12;
#if 0
	printf("Mapping % pages.\n", (u64)page_count);
	printf("Kernel address: %.\n", kernel_addr);
	printf("Virtual address: %.\n", virt_addr);
#endif
	const usize start_index = (usize)virt_addr >> 12;
	kernel_addr &= ~0xfff;
	kernel_addr &= 0xffffffff;
	for (usize i = 0; i < page_count; ++i) {
		u64 attributes = DESC_FLAGS_AF | DESC_FLAGS_NG | DESC_FLAGS_PXN;
		if (!(flags & PF_X)) {
			attributes |= DESC_FLAGS_UXN;
		}
		if ((flags & PF_R) && !(flags & PF_W)) {
			attributes |= DESC_FLAGS_AP_RO;
		}
		else if (flags & PF_W) {
			attributes |= DESC_FLAGS_AP_RW;
		}
#if 0
		if (start_index + i >= L3_TABLE_COUNT * L3_TABLE_ENTRY_COUNT) {
			PRINT_ERROR("Pages out of table 3 range.");
			printf("Index: %\n", (u64)(start_index + i));
			shutdown(0, NULL);
		}
#endif
		const usize l2_index = (start_index + i) / L3_TABLE_ENTRY_COUNT;
		const isize which_l3_table = _proc_table_index(l2_index);
		if (which_l3_table == -1) {
			PRINT_ERROR("No suitable table found.");
			printf("l2_index was %\n", (u64)l2_index);
			shutdown(0, NULL);
		}
		u64* dst = &proc->l3_tables[which_l3_table].entries[(start_index + i) % L3_TABLE_ENTRY_COUNT];
		if (*dst != 0) {
			PRINT_ERROR("Page already mapped.");
			shutdown(0, NULL);
		}
		*dst = (u64)(kernel_addr + (i << 12))
			| DESC_FLAGS_PAGE
			| attributes;
	}
}

#define STACK_SIZE		0x40000
#define VIRT_HEAP_BASE	(0x80ull << 21)
#define VIRT_STACK_BASE	(0x1ffull << 21)

i32 _proc_image(struct Proc* proc, const void* elf, i32 argc, const char* argv[]) {
	const struct ELFHeader* elf_header = elf;
	if (elf_header->ident.magic != ELF_HEADER_MAGIC) {
		return ENOEXEC;
	}
	const struct ProgramHeader64* prog_headers = elf + elf_header->info64.ph_off;
	const usize prog_header_count = (usize)elf_header->info64.ph_num;
	u64 highest_addr = 0;
	for (usize i = 0; i < prog_header_count; ++i) {
		const struct ProgramHeader64* header = &prog_headers[i];
		const u64 addr = header->v_addr + header->mem_sz;
		if (addr > highest_addr) {
			highest_addr = addr;
		}
	}
	proc->size = PAGE_SIZE * ((highest_addr + PAGE_SIZE - 1) / PAGE_SIZE);
	//proc->heap_offset = proc->size;
	const usize heap_offset = proc->size;
	proc->heap_size = HEAP_BLOCK_SIZE;
	proc->size += proc->heap_size + STACK_SIZE;
	// NOTE: image should be aligned to the size of the largest translation table (T3)
	proc->image = kmalloc_page_align(proc->size);
	memset(proc->image, 0, proc->size);
	for (usize i = 0; i < prog_header_count; ++i) {
		const struct ProgramHeader64* header = &prog_headers[i];
		if (header->type != PT_LOAD) {
			continue;
		}
		void* kernel_addr = proc->image + header->v_addr;
		memcpy(kernel_addr, elf + header->offset, header->file_sz);
		_proc_map_pages(proc, header->v_addr, (u64)kernel_addr, header->mem_sz, header->flags);
	}
	_proc_map_pages(proc, VIRT_HEAP_BASE, (u64)proc->image + heap_offset, proc->heap_size, PF_R | PF_W);
	_proc_map_pages(proc, VIRT_STACK_BASE - STACK_SIZE, (u64)proc->image + heap_offset + proc->heap_size, STACK_SIZE, PF_R | PF_W);
	void* stack = proc->image + proc->size;
	usize argv_size = 0;
	for (i32 i = 0; i < argc; ++i) {
		const usize length = strlen(argv[i]) + 1;
		argv_size += length;
		memcpy(stack - argv_size, argv[i], length);
	}
	argv_size += 0x10 - (argv_size & 0xf);
	argv_size += (argc + 1) * sizeof(u64);
	u64* pointers = stack - argv_size;
	u64 pointer = VIRT_STACK_BASE;
	for (i32 i = 0; i < argc; ++i) {
		pointer -= strlen(argv[i]) + 1;
		pointers[i] = pointer;
	}
	// Terminate with a NULL pointer
	pointers[argc] = 0;
	memset(proc->regs.general, 0, sizeof proc->regs.general);
	proc->regs.general[0] = argc;
	proc->regs.general[1] = VIRT_STACK_BASE - argv_size;
	proc->regs.general[2] = VIRT_HEAP_BASE;
	proc->regs.general[3] = HEAP_BLOCK_SIZE;
	proc->regs.sp = VIRT_STACK_BASE - argv_size - sizeof(i32);
	proc->regs.tpidrro = proc->regs.sp;
	//printf("STACK: %\n", proc->regs.sp);
	proc->regs.elr = (u64)elf_header->info64.entry;
	//printf("ELR: %\n", proc->regs.elr);
	proc->regs.spsr = 0;
	return 0;
}

// Create process image from program stored at path
i32 _proc_program(struct Proc* proc, const char* path, const char* argv[]) {
	u32 node_id;
	i32 result = fs_find(&node_id, path);
	if (result != 0) {
		return result;
	}
	struct FSNode* node = fs_open(node_id);
	if (node == NULL) {
		fs_close(node);
		return ENOENT;
	}
	if (node->type == FS_NODE_TYPE_DIR) {
		fs_close(node);
		return EISDIR;
	}
	if (node->type != FS_NODE_TYPE_REG) {
		fs_close(node);
		return EPERM;
	}
	i32 argc = 0;
	for (; argv[argc] != NULL; ++argc) {
	}
	result = _proc_image(proc, node->file.data, argc, argv);
	fs_close(node);
	return result;
}

// TODO: size should be nonzero
static bool _proc_validate_read(const void* addr, usize size) {
	u64 result1;
	asm volatile (
		"at		s1e0r, %1\n"
		"mrs %0, par_el1"
		: "=r" (result1)
		: "r" (addr)
	);
	addr += size - 1;
	u64 result2;
	asm volatile (
		"at		s1e0r, %1\n"
		"mrs %0, par_el1"
		: "=r" (result2)
		: "r" (addr)
	);
	return !((result1 | result2) & 0x1);
}

static bool _proc_validate_write(const void* addr, usize size) {
	u64 result1;
	asm volatile (
		"at		s1e0w, %1\n"
		"mrs %0, par_el1"
		: "=r" (result1)
		: "r" (addr)
	);
	addr += size - 1;
	u64 result2;
	asm volatile (
		"at		s1e0w, %1\n"
		"mrs %0, par_el1"
		: "=r" (result2)
		: "r" (addr)
	);
	return !((result1 | result2) & 0x1);
}

static bool _proc_validate_str_read(const char* string) {
	//return _proc_validate_read(string, strlen(string) + 1);
	while (true) {
		if (!_proc_validate_read(string, 1)) {
			return false;
		}
		if (*string == '\0') {
			return true;
		}
		++string;
	}
}

static i32 _proc_find(u32* dst, const char* path) {
	u32 cwd_id;
	const u32 result = fs_find(&cwd_id, _procs[_current]->cwd);
	if (result != 0) {
		PRINT_ERROR("Bad cwd.");
		print(_procs[_current]->cwd);
		shutdown(0, NULL);
	}
	return fs_find_from(dst, path, cwd_id);
}

static i32 _proc_decompose(u32* dst, const char** name, const char* path) {
	char parent_path[PATH_MAX_LENGTH + 1];
	strcpy(parent_path, path);
	char* where = strrchr(parent_path, '/');
	if (where != NULL) {
		*where = '\0';
		*name = path + (where - parent_path) + 1;
	}
	else {
		parent_path[0] = '.';
		parent_path[1] = '\0';
		*name = path;
	}
	return _proc_find(dst, parent_path);
}

static void _proc_set_errno(i32 err) {
	*(int*)(_procs[_current]->regs.tpidrro) = err;
}

i32 proc_open(const char* path, i32 flags) {
	if (!_proc_validate_str_read(path)) {
		_proc_set_errno(EFAULT);
		return -1;
	}
	struct Proc* proc = _procs[_current];
	u32 node_id;
	i32 result = _proc_find(&node_id, path);
	if (result == ENOENT) {
		if (!(flags & O_CREAT)) {
			_proc_set_errno(ENOENT);
			return -1;
		}
		for (i32 i = 0; i < MAX_FD_ENTRIES; ++i) {
			if (proc->fd_table[i].node != NULL) {
				continue;
			}
			const char* name;
			u32 parent_id;
			result = _proc_decompose(&parent_id, &name, path);
			if (result != 0) {
#if 1
		PRINT_ERROR("Could not create file. Reason: _proc_decompose() failed.");
		print("path='");
		print(path);
		print("'\n");
#endif
				_proc_set_errno(result);
				return -1;
			}
			struct FSNode* node = fs_creat(parent_id, name);
			if (node == NULL) {
#if 1
		PRINT_ERROR("Could not create file. Reason: fs_creat() failed.");
		print("path='");
		print(path);
		print("'\n");
#endif
				return -1;
			}
			proc->fd_table[i].node = node;
			proc->fd_table[i].offset = 0;
			return i;
		}
		PRINT_ERROR("Max file descriptors in use.");
		_proc_set_errno(ENFILE);
		return -1;
	}
	if (result != 0) {
		_proc_set_errno(result);
		return -1;
	}
	for (i32 i = 0; i < MAX_FD_ENTRIES; ++i) {
		if (proc->fd_table[i].node != NULL) {
			continue;
		}
		proc->fd_table[i].node = fs_open(node_id);
		proc->fd_table[i].offset = 0;
		return i;
	}
	return -1;
}

i32 proc_close(i32 fd) {
	if (fd < 0 || fd >= MAX_FD_ENTRIES) {
		_proc_set_errno(EBADF);
		return -1;
	}
	struct Proc* proc = _procs[_current];
	if (proc->fd_table[fd].node == NULL) {
		_proc_set_errno(EBADF);
		return -1;
	}
	fs_close(proc->fd_table[fd].node);
	proc->fd_table[fd].node = NULL;
	return 0;
}

char* proc_getcwd(char* buf, usize size) {
	if (!_proc_validate_write(buf, size)) {
		_proc_set_errno(EFAULT);
		return NULL;
	}
	const usize length = strlen(_procs[_current]->cwd) + 1;
	if (length > size) {
		return NULL;
	}
	return memcpy(buf, _procs[_current]->cwd, length);
}

static i32 _proc_fs_result(i32 result) {
	if (result == 0) {
		return 0;
	}
	_proc_set_errno(result);
	return -1;
}

i32 proc_chdir(const char* path) {
	if (!_proc_validate_str_read(path)) {
		_proc_set_errno(EFAULT);
		return -1;
	}
	u32 node_id;
	i32 result = _proc_find(&node_id, path);
	if (result != 0) {
		_proc_set_errno(result);
		return -1;
	}
	struct FSNode* node = fs_open(node_id);
	if (node->type != FS_NODE_TYPE_DIR) {
		_proc_set_errno(ENOTDIR);
		fs_close(node);
		return -1;
	}
	char abs_path[PATH_MAX_LENGTH + 1];
	strcpy(abs_path, _procs[_current]->cwd);
	strcat(abs_path, "/");
	strcat(abs_path, path);
	char can_path[PATH_MAX_LENGTH + 1];
	result = fs_canonicalize(can_path, abs_path);
	if (result == 0) {
		strcpy(_procs[_current]->cwd, can_path);
	}
	fs_close(node);
	return _proc_fs_result(result);
}

void _proc_copy_table(i64 image_offset, u64* dst, const u64* src, usize count) {
	for (usize i = 0; i < count; ++i) {
		if (!(src[i] & 0b11)) {
			continue;
		}
		const u64 addr_mask = 0xfffff000;
		const u64 new_addr = (i64)(src[i] & addr_mask) + image_offset;
		dst[i] = new_addr | (src[i] & ~addr_mask);
	}
}

static void _proc_destroy(struct Proc* proc) {
#if 0
	printf("[_proc_destroy] Deleting process (pid=%)\n", (u64)proc->id);
#endif
	// Invalidate all TLB entries with ASID of this process
	const u64 asid = (u64)proc->id << 48;
	asm volatile (
		"tlbi	aside1, %0\n"
		"isb"
		:
		: "r" (asid)
	);
	for (struct HeapBlock* it = proc->heap_blocks; it != NULL;) {
		struct HeapBlock* temp = it->next;
		kfree(it);
		it = temp;
	}
	kfree(proc->image);
	kfree(proc);
}

static void _proc_copy_heap(struct Proc* dst, const struct Proc* src) {
	dst->heap_size = src->heap_size;
	struct HeapBlock** dst_it = &dst->heap_blocks;
	for (struct HeapBlock* it = src->heap_blocks; it != NULL; it = it->next) {
		struct HeapBlock* copy = kmalloc_page_align(sizeof *copy);
		memcpy(copy, it->data, sizeof *copy);
		copy->next = NULL;
		*dst_it = copy;
		dst_it = &copy->next;
	}
}

i16 proc_fork(void) {
	struct Proc* parent = _procs[_current];
	// We need the most up to date registers
	_proc_store_regs(parent);
	struct Proc* child = _proc_create(parent->cwd);
	if (child == NULL) {
		_proc_set_errno(EAGAIN);
		return -1;
	}
	child->size = parent->size;
	child->image = kmalloc_page_align(child->size);
	_proc_copy_heap(child, parent);
	if (child->image == NULL) {
		_proc_destroy(child);
		--_proc_count;
		_proc_set_errno(ENOMEM);
		return -1;
	}
	memcpy(child->fd_table, parent->fd_table, sizeof child->fd_table);
	memcpy(child->image, parent->image, child->size);
	const i64 image_offset = (i64)child->image - (i64)parent->image;
	for (usize i = 0; i < L3_TABLE_COUNT; ++i) {
		_proc_copy_table(image_offset, child->l3_tables[i].entries, parent->l3_tables[i].entries, L3_TABLE_ENTRY_COUNT);
	}
	memcpy(child->regs.general, parent->regs.general, sizeof child->regs.general);
	child->regs.sp = parent->regs.sp;
	child->regs.spsr = parent->regs.spsr;
	child->regs.elr = parent->regs.elr;
	child->regs.tpidrro = parent->regs.tpidrro;
	child->regs.general[0] = 0;
#if 0
	const u64 address = child->regs.elr;
	asm volatile (
		"tlbi	vaae1, %0\n"
		"isb"
		:
		: "r"(address)
	);
#endif
	return child->id;
}

static bool _proc_validate_str_list_read(const char* strings[]) {
	// TODO: there should be a max number of strings
	usize i;
	for (i = 0; strings[i] != NULL; ++i) {
		if (!_proc_validate_str_read(strings[i])) {
			return false;
		}
	}
	return _proc_validate_read(strings, (i + 1) * sizeof *strings);
}

i32 proc_execve(const char* path, const char* argv[], const char* envp[]) {
	if (!_proc_validate_str_read(path)
			|| !_proc_validate_str_list_read(argv)
			|| !_proc_validate_str_list_read(envp)) {
		_proc_set_errno(EFAULT);
		return -1;
	}
	// TODO: environment
	(void)envp;
	struct Proc* new_proc = kmalloc_page_align(sizeof *new_proc);
#if 0
	printf("[proc_execve] new_proc=%\n", (u64)new_proc);
#endif
	_proc_init(new_proc, _procs[_current]->id, _procs[_current]->cwd);
	const i32 err = _proc_program(new_proc, path, argv);
	if (err != 0) {
		_proc_destroy(new_proc);
		_proc_set_errno(err);
		return -1;
	}
	_proc_destroy(_procs[_current]);
	_procs[_current] = new_proc;
	_proc_load_regs_and_run();
	__builtin_unreachable();
}

// TODO: we need to close all file descriptions
_Noreturn void proc_exit(i32 status) {
	printf("[proc] Process % exiting with status %\n", (u64)_procs[_current]->id, (u64)status);
	const i16 pid = _procs[_current]->id;
	if (pid == 1) {
		// TODO: re-initialize shell
		PRINT_ERROR("Fatal error occurred in shell.");
		shutdown(0, NULL);
	}
	_proc_destroy(_procs[_current]);
	--_proc_count;
	for (usize i = _current; i < _proc_count; ++i) {
		_procs[i] = _procs[i + 1];
	}
	for (usize i = 0; i < _proc_count; ++i) {
		if (_procs[i]->state != PROC_STATE_WAIT_PID || _procs[i]->wait_pid != pid) {
			continue;
		}
		_procs[i]->regs.general[0] = status;
		_procs[i]->state = PROC_STATE_RUN;
	}
	_proc_schedule_next();
	_proc_load_regs_and_run();
}

i16 proc_getpid(void) {
	return _procs[_current]->id;
}

i32 proc_waitpid(i16 pid) {
	// TODO: maintain a list of child processes and keep track of which ones have exited
	bool found_pid = false;
	for (usize i = 0; i < _proc_count; ++i) {
		if (_procs[i]->id == pid) {
			found_pid = true;
			break;
		}
	}
	if (!found_pid) {
		return 0;
	}
	_proc_store_regs(_procs[_current]);
	_procs[_current]->wait_pid = pid;
	_procs[_current]->state = PROC_STATE_WAIT_PID;
	_proc_schedule_next();
	_proc_load_regs_and_run();
}

_Noreturn void proc_queue_read(struct FSNode* node, void* dst, usize size) {
	_proc_store_regs(_procs[_current]);
	_procs[_current]->wait_read.node = node;
	_procs[_current]->wait_read.dst = dst;
	_procs[_current]->wait_read.size = size;
	_procs[_current]->state = PROC_STATE_WAIT_READ;
	_proc_schedule_next();
	_proc_load_regs_and_run();
}

_Noreturn void proc_update_pending_io(void) {
	const char c = tty_getchar();
	u64 saved_ttbr;
	asm volatile (
		"mrs	%0, ttbr0_el1\n"
		: "=r" (saved_ttbr)
	);
	u32 tty_id;
	fs_find(&tty_id, "/dev/tty");
	struct FSNode* node = fs_open(tty_id);
	for (usize i = 0; i < _proc_count; ++i) {
		if (_procs[i]->state != PROC_STATE_WAIT_READ || _procs[i]->wait_read.node != node) {
			continue;
		}
		const bool should_flush = tty_line_add_char(c);
		if (!should_flush) {
			continue;
		}
		_proc_use_ttbr(_procs[i]);
		const usize read_size = tty_flush_line(_procs[i]->wait_read.dst, _procs[i]->wait_read.size);
		_procs[i]->regs.general[0] = read_size;
		_procs[i]->state = PROC_STATE_RUN;
	}
	fs_close(node);
	asm volatile (
		"msr	ttbr0_el1, %0\n"
		"isb"
		:
		: "r" (saved_ttbr)
	);
	_proc_schedule_next();
	_proc_load_regs_and_run();
}

i32 proc_rename(const char* old_path, const char* new_path) {
	if (!_proc_validate_str_read(old_path) || !_proc_validate_str_read(new_path)) {
		_proc_set_errno(EFAULT);
		return -1;
	}
	const char* old_name;
	u32 old_parent_id;
	i32 result = _proc_decompose(&old_parent_id, &old_name, old_path);
	if (result != 0) {
		_proc_set_errno(result);
		return -1;
	}
	const char* new_name;
	u32 new_parent_id;
	result = _proc_decompose(&new_parent_id, &new_name, new_path);
	if (result != 0) {
		_proc_set_errno(result);
		return -1;
	}
	// Don't let . and .. entries be changed
	if (strcmp(old_name, ".") == 0 || strcmp(old_name, "..") == 0
		|| strcmp(new_name, ".") == 0 || strcmp(new_name, "..") == 0) {
		_proc_set_errno(EINVAL);
		return -1;
	}
	return _proc_fs_result(fs_rename(old_parent_id, old_name, new_parent_id, new_name));
}

i32 proc_unlink(const char* path) {
	if (!_proc_validate_str_read(path)) {
		_proc_set_errno(EFAULT);
		return -1;
	}
	const char* name;
	u32 parent_id;
	const i32 result = _proc_decompose(&parent_id, &name, path);
	if (result != 0) {
		_proc_set_errno(result);
		return -1;
	}
	return _proc_fs_result(fs_unlink(parent_id, name));
}

// TODO: use mode
i32 proc_mkdir(const char* path, u16 mode) {
	if (!_proc_validate_str_read(path)) {
		_proc_set_errno(EFAULT);
		return -1;
	}
	(void)mode;
	const char* name;
	u32 parent_id;
	const i32 result = _proc_decompose(&parent_id, &name, path);
	if (result != 0) {
		_proc_set_errno(result);
		return -1;
	}
	return _proc_fs_result(fs_mkdir(parent_id, name));
}

i32 proc_rmdir(const char* path) {
	if (!_proc_validate_str_read(path)) {
		_proc_set_errno(EFAULT);
		return -1;
	}
	u32 node_id;
	const i32 result = _proc_find(&node_id, path);
	if (result != 0) {
		_proc_set_errno(result);;
		return -1;
	}
	return _proc_fs_result(fs_rmdir(node_id));
}

// TODO: apparently it is valid to read when offset is past the end of the file
// in this case, the read would return 'count' bytes set to zero
isize proc_read(i32 fd, void* buf, usize count) {
	if (!_proc_validate_write(buf, count)) {
		_proc_set_errno(EFAULT);
		return -1;
	}
	if (fd < 0 || fd >= MAX_FD_ENTRIES) {
		_proc_set_errno(EBADF);
		return -1;
	}
	if (_procs[_current]->fd_table[fd].node == NULL) {
		_proc_set_errno(EBADF);
		return -1;
	}
	struct FSNode* node = _procs[_current]->fd_table[fd].node;
	if (node->type == FS_NODE_TYPE_DIR) {
		_proc_set_errno(EISDIR);
		return -1;
	}
	// NOTE: Doing a read of size 0 is a trick to check if data is ready
	// POSIX says that read with size 0 may cause errors to be detected
	// (although technically it should only work like this if O_NONBLOCK flag is set)
	if (fd == STDIN_FILENO && count == 0) {
		if (!tty_read_would_block()) {
			return 0;
		}
		_proc_set_errno(EAGAIN);
		return -1;
	}
	const isize result = fs_read(node, buf, count, &_procs[_current]->fd_table[fd].offset);
	if (result == -2) {
		proc_queue_read(node, buf, count);
	}
	return result;
}

isize proc_write(i32 fd, const void* buf, usize count) {
	if (!_proc_validate_read(buf, count)) {
		_proc_set_errno(EFAULT);
		return -1;
	}
	if (fd < 0 || fd >= MAX_FD_ENTRIES) {
		_proc_set_errno(EBADF);
		return -1;
	}
	if (_procs[_current]->fd_table[fd].node == NULL) {
		_proc_set_errno(EBADF);
		return -1;
	}
	struct FSNode* node = _procs[_current]->fd_table[fd].node;
	return fs_write(node, buf, count, &_procs[_current]->fd_table[fd].offset);
}

// TODO: flags
isize proc_getdents(i32 fd, void* buf, usize size, i32 flags) {
	(void)flags;
	if (!_proc_validate_write(buf, size)) {
		_proc_set_errno(EFAULT);
		return -1;
	}
	if (fd < 0 || fd >= MAX_FD_ENTRIES) {
		_proc_set_errno(EBADF);
		return -1;
	}
	struct FDEntry* fd_entry = &_procs[_current]->fd_table[fd];
	struct FSNode* node = fd_entry->node;
	if (node == NULL) {
		_proc_set_errno(EBADF);
		return -1;
	}
	if (node->type != FS_NODE_TYPE_DIR) {
		_proc_set_errno(ENOTDIR);
		return -1;
	}
	if (fd_entry->offset < 0) {
		// TODO: maybe more specfic error
		_proc_set_errno(EIO);
		return -1;
	}
	const usize entries_remaining = node->dir.entry_count - (fd_entry->offset / sizeof *node->dir.entries);
	if (entries_remaining == 0) {
		return 0;
	}
	const usize count = MIN(size / sizeof(struct posix_dent), entries_remaining);
	usize size_written = 0;
	struct posix_dent* dents = buf;
	struct DirEntry* dir_entries = (void*)node->dir.entries + fd_entry->offset;
	for (usize i = 0; i < count; ++i) {
		dents[i].d_ino = 0;
		dents[i].d_reclen = sizeof(dents[i]);
		dents[i].d_type = 0;
		strcpy(dents[i].d_name, dir_entries[i].name);
		size_written += sizeof(dents[i]);
	}
	fd_entry->offset += count * sizeof *node->dir.entries;
	return size_written;
}

i32 proc_fstat(i32 fd, struct stat* buf) {
	if (!_proc_validate_write(buf, sizeof *buf)) {
#if 1
		printf("buf=%\n", (u64)buf);
#endif
		_proc_set_errno(EFAULT);
		return -1;
	}
	if (fd < 0 || fd >= MAX_FD_ENTRIES) {
		_proc_set_errno(EBADF);
		return -1;
	}
	if (_procs[_current]->fd_table[fd].node == NULL) {
		_proc_set_errno(EBADF);
		return -1;
	}
	struct FSNode* node = _procs[_current]->fd_table[fd].node;
	memset(buf, 0, sizeof *buf);
	buf->st_atim = node->access_time;
	buf->st_mtim = node->modify_time;
	buf->st_ctim = node->status_change_time;
	switch (node->type) {
	case FS_NODE_TYPE_REG:
		buf->st_mode = S_IFREG;
		break;
	case FS_NODE_TYPE_DIR:
		buf->st_mode = S_IFDIR;
		break;
	case FS_NODE_TYPE_CHR:
		buf->st_mode = S_IFCHR;
		break;
	default:
		PRINT_ERROR("Invalid node type.");
		shutdown(0, NULL);
		break;
	}
	buf->st_mode |= 0777;
	return 0;
}

i32 proc_tcgetattr(i32 fd, struct termios* termios_p) {
	if (!_proc_validate_write(termios_p, sizeof *termios_p)) {
		_proc_set_errno(EFAULT);
		return -1;
	}
	if (fd < 0 || fd >= MAX_FD_ENTRIES) {
		_proc_set_errno(EBADF);
		return -1;
	}
	if (_procs[_current]->fd_table[fd].node == NULL) {
		_proc_set_errno(EBADF);
		return -1;
	}
	tty_tcgetattr(termios_p);
	return 0;
}

i32 proc_tcsetattr(i32 fd, i32 optional_actions, const struct termios* termios_p) {
	if (!_proc_validate_read(termios_p, sizeof *termios_p)) {
		_proc_set_errno(EFAULT);
		return -1;
	}
	if (fd < 0 || fd >= MAX_FD_ENTRIES) {
		_proc_set_errno(EBADF);
		return -1;
	}
	if (_procs[_current]->fd_table[fd].node == NULL) {
		_proc_set_errno(EBADF);
		return -1;
	}
	tty_tcsetattr(optional_actions, termios_p);
	return 0;
}

void proc_send_signal(i32 sig, const siginfo_t* info) {
	_procs[_current]->signal = sig;
	if (info != NULL) {
		memcpy(&_procs[_current]->signal_info, info, sizeof *info);
	}
}

i32 proc_nanosleep(const struct timespec* rqtp, struct timespec* rmtp) {
	if (!_proc_validate_read(rqtp, sizeof *rqtp) || (rmtp != NULL && !_proc_validate_write(rmtp, sizeof *rmtp))) {
		_proc_set_errno(EFAULT);
		return -1;
	}
	if (rqtp->tv_nsec < 0 || rqtp->tv_nsec >= BILLION) {
		_proc_set_errno(EINVAL);
		return -1;
	}
	_proc_store_regs(_procs[_current]);
	struct timespec wait_time = time_current();
	wait_time.tv_nsec += rqtp->tv_nsec;
	wait_time.tv_sec += rqtp->tv_sec + (wait_time.tv_nsec / BILLION);
	wait_time.tv_nsec %= BILLION;
	_procs[_current]->wait_time = wait_time;
	_procs[_current]->state = PROC_STATE_SLEEP;
	_procs[_current]->regs.general[0] = 0;
	_proc_schedule_next();
	_proc_load_regs_and_run();
}

isize proc_lseek(i32 fd, isize offset, i32 whence) {
	if (fd < 0 || fd >= MAX_FD_ENTRIES) {
		_proc_set_errno(EBADF);
		return -1;
	}
	if (_procs[_current]->fd_table[fd].node == NULL) {
		_proc_set_errno(EBADF);
		return -1;
	}
	isize new_offset = offset;
	switch (whence) {
	case SEEK_SET:
		break;
	case SEEK_CUR:
		new_offset += _procs[_current]->fd_table[fd].offset;
		break;
	case SEEK_END:
		{
			struct FSNode* node = _procs[_current]->fd_table[fd].node;
			new_offset += (isize)node->file.size;
		}
		break;
	default:
		_proc_set_errno(EINVAL);
		return -1;
	}
	if (new_offset < 0) {
		_proc_set_errno(EINVAL);
		return -1;
	}
	_procs[_current]->fd_table[fd].offset = new_offset;
	return new_offset;
}

i32 proc_sigaction(i32 sig, sigset_t mask, i32 flags, void* handler, void* wrapper) {
	if (!VALID_SIGNAL(sig)) {
		_proc_set_errno(EINVAL);
		return -1;
	}
	// TODO: validate and make use of mask
	// TODO: validate handler (cannot set SIG_IGN for signals like SIGSEGV)
	const usize index = SIGNAL_INDEX(sig);
	struct Proc* proc = _procs[_current];
	proc->sigactions[index].handler = handler;
	proc->sigactions[index].mask = mask;
	proc->sigactions[index].flags = flags;
	proc->sigaction_wrapper = wrapper;
	return 0;
}

_Noreturn void proc_sigreturn(void) {
	struct Proc* proc = _procs[_current];
	proc->regs.sp += SIGINFO_SIZE;
	u64* regs = (u64*)proc->regs.sp;
	memcpy(proc->regs.general, regs, sizeof proc->regs.general);
	proc->regs.elr = regs[31];
	proc->regs.sp += 256;
	_proc_schedule_next();
	_proc_load_regs_and_run();
}

void proc_grow_heap(usize size) {
	struct Proc* proc = _procs[_current];
	//const usize block_count = (size + HEAP_BLOCK_SIZE - 1) / HEAP_BLOCK_SIZE;
	const usize block_count = size / HEAP_BLOCK_SIZE;
	for (usize i = 0; i < block_count; ++i) {
		struct HeapBlock* block = kmalloc_page_align(sizeof *block);
		memset(block, 0, sizeof* block);
		block->next = proc->heap_blocks;
		proc->heap_blocks = block;
		_proc_map_pages(proc, VIRT_HEAP_BASE + proc->heap_size, (u64)block->data, HEAP_BLOCK_SIZE, PF_R | PF_W);
		proc->heap_size += HEAP_BLOCK_SIZE;
	}
}

i32 proc_canonicalize(const char* restrict path, char* restrict dst) {
	if (!_proc_validate_str_read(path) || !_proc_validate_write(dst, PATH_MAX)) {
		_proc_set_errno(EFAULT);
		return -1;
	}
	return _proc_fs_result(fs_canonicalize(dst, path));
}
