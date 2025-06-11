#include "proc.h"
#include "heap.h"
#include "elf.h"
#include "fs.h"
#include "io.h"
#include "regs.h"
#include "exception.h"
#include "commands.h"
#include <string.h>

#define MAX_PROCS		4
#define PATH_MAX_LENGTH	127
// Max file descriptors that a process can have open at the same time
#define MAX_FD_ENTRIES	4

#define L1_TABLE_ENTRY_COUNT	4
#define L2_TABLE_ENTRY_COUNT	512
#define L3_TABLE_ENTRY_COUNT	512

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

enum {
	PROC_STATE_RUN = 0,
	PROC_STATE_SLEEP,
};

struct FDEntry {
	struct stat	stat;
	char		path[PATH_MAX_LENGTH + 1];
	bool		in_use;
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

struct Proc {
	struct L3Table	l3_table;
	struct L2Table	l2_table;
	struct L1Table	l1_table;
	struct Regs		regs;
	struct FDEntry	fd_table[MAX_FD_ENTRIES];
	char			cwd[PATH_MAX_LENGTH + 1];
	void*			image;
	struct {
		void*	dst;
		usize	size;
		i32		fd;
	} pending_read;
	usize			size;
	usize			heap_offset;
	i16				id;
	u8				state;
};

static struct Proc* _procs[MAX_PROCS];
static usize _proc_count;
static usize _current;

static _Noreturn void _proc_load_regs_and_run(const struct Proc* proc);
static i32 _proc_program(struct Proc* proc, const char* path, const char* argv[]);

static i32 _proc_new_id(void) {
	static i16 id = 1;
	return id++;
}

static void _proc_init(struct Proc* proc, i16 id, const char* cwd) {
	memset(proc, 0, sizeof *proc);
	proc->id = id;
	strncpy(proc->cwd, cwd, PATH_MAX_LENGTH);

	proc->regs.ttbr = ((u64)proc->id << 48) | ((u64)&proc->l1_table & 0xffffffff);
	proc->l1_table.entries[0] = ((u64)&proc->l2_table & 0xffffffff)
		| DESC_FLAGS_TABLE;
	proc->l2_table.entries[0] = ((u64)&proc->l3_table & 0xffffffff)
		| DESC_FLAGS_TABLE | DESC_FLAGS_AF;

	const char* path = "/dev/tty";
	strcpy(proc->fd_table[0].path, path);
	proc->fd_table[0].in_use = true;
	strcpy(proc->fd_table[1].path, path);
	proc->fd_table[1].in_use = true;
	strcpy(proc->fd_table[2].path, path);
	proc->fd_table[2].in_use = true;
}

static struct Proc* _proc_create(const char* cwd) {
	if (_proc_count == MAX_PROCS) {
		return NULL;
	}
	struct Proc* proc = kmalloc_page_align(sizeof *proc);
	_proc_init(proc, _proc_new_id(), cwd);
	_procs[_proc_count++] = proc;
	return proc;
}

// Entry point to process-handling code
void proc_main(void) {
	_proc_count = 0;
	_current = 0;
	const char* argv[] = { "/bin/sh", NULL };
	if (_proc_program(_proc_create("/"), "/bin/sh", argv) == -1) {
		printf("[proc] _proc_program() failed.\n");
		shutdown(0, NULL);
	}
	print("[proc] Starting system shell...\n");
	AUX_MU->ier_reg = 0b1;
	_proc_load_regs_and_run(_procs[0]);
}

#define SAVED_GENERAL_REGS	(const void*)(0xffffffff0007ff00)

static void _proc_store_regs(struct Proc* proc) {
	memcpy(proc->regs.general, SAVED_GENERAL_REGS, sizeof proc->regs.general);
	u64	sp;
	u64	spsr;
	u64	elr;
	u64	ttbr;
	asm (
		"mrs	%0, sp_el0\n"
		"mrs	%1, spsr_el1\n"
		"mrs	%2, elr_el1\n"
		"mrs	%3, ttbr0_el1"
		: "=r" (sp), "=r" (spsr), "=r" (elr), "=r" (ttbr)
	);
	proc->regs.sp = sp;
	proc->regs.spsr = spsr;
	proc->regs.elr = elr;
	proc->regs.ttbr = ttbr;
}

static _Noreturn void _proc_load_regs_and_run(const struct Proc* proc) {
	// Set timer for time slice
	SYS_TIMER->cs = 1;
	SYS_TIMER->c0 = SYS_TIMER->clo + 0xffff;
	INTERRUPTS->disable_irqs_1 = 1 << 29;
	INTERRUPTS->enable_irqs_1 = 1;
	irq_enable();
	const u64 sp = proc->regs.sp;
	const u64 spsr = proc->regs.spsr;
	const u64 elr = proc->regs.elr;
	const u64 ttbr = proc->regs.ttbr;
#if 0
	print("Switching process\n");
	printf("pid:\t%\n", (u64)proc->id);
	printf("SP:\t%\n", sp);
	printf("SPSR:\t%\n", spsr);
	printf("ELR:\t%\n", elr);
	printf("TTBR:\t%\n", ttbr);
#endif
	asm volatile (
		"msr	sp_el0, %0\n"
		"msr	spsr_el1, %1\n"
		"msr	elr_el1, %2\n"
		"msr	ttbr0_el1, %3\n"
		"isb"
		:
		: "r" (sp), "r" (spsr), "r" (elr), "r" (ttbr)
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

void _proc_schedule_next(void) {
	// Provide an opportunity for UART IRQ
	INTERRUPTS->disable_irqs_1 = 1;
	INTERRUPTS->enable_irqs_1 = 1 << 29;
	irq_enable();
	do {
		_current = (_current + 1) % _proc_count;
	}
	while (_procs[_current]->state != PROC_STATE_RUN);
}

_Noreturn void proc_run_next(void) {
	_proc_store_regs(_procs[_current]);
	_proc_schedule_next();
	_proc_load_regs_and_run(_procs[_current]);
}

i32 proc_open_fd(const char* path) {
	if (strlen(path) > PATH_MAX_LENGTH) {
		return -1;
	}
	struct Proc* proc = _procs[_current];
	for (i32 i = 0; i < MAX_FD_ENTRIES; ++i) {
		if (proc->fd_table[i].in_use) {
			continue;
		}
		if (fs_find(path) == NULL) {
			return -1;
		}
		// NOTE: currently assuming that path is not too long to fit
		strcpy(proc->fd_table[i].path, path);
		proc->fd_table[i].in_use = true;
		return i;
	}
	return -1;
}

const char* proc_fd_path(i32 fd) {
	if (fd < 0 || fd >= MAX_FD_ENTRIES) {
		return NULL;
	}
	return _procs[_current]->fd_table[fd].path;
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
		if (start_index + i >= L3_TABLE_ENTRY_COUNT) {
			PRINT_ERROR("Pages out of table 3 range.");
			printf("Index: %\n", (u64)(start_index + i));
			return;
		}
		proc->l3_table.entries[start_index + i] = (u64)(kernel_addr + (i << 12))
			| DESC_FLAGS_PAGE
			| attributes;
	}
}

#define HEAP_PLUS_STACK_SIZE	0x8000

i32 _proc_image(struct Proc* proc, const void* elf, u8 argc, const char* argv[]) {
	const struct ELFHeader* elf_header = elf;
	if (elf_header->ident.magic != ELF_HEADER_MAGIC) {
		PRINT_ERROR("ELF has wrong magic bytes.\n");
		return -1;
	}
	const struct ProgramHeader64* prog_headers = elf + elf_header->info64.ph_off;
	const usize prog_header_count = (usize)elf_header->info64.ph_num;
	u64 highest_addr = 0;
	for (usize i = 0; i < prog_header_count; ++i) {
		const struct ProgramHeader64* header = &prog_headers[i];
		//if (header->align != PAGE_SIZE) {
		//	return -1;
		//}
		const u64 addr = header->v_addr + header->mem_sz;
		if (addr > highest_addr) {
			highest_addr = addr;
		}
	}
	proc->size = PAGE_SIZE * ((highest_addr + PAGE_SIZE - 1) / PAGE_SIZE);
	proc->heap_offset = proc->size;
	proc->size += HEAP_PLUS_STACK_SIZE;
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
	_proc_map_pages(proc, (u64)proc->heap_offset, (u64)proc->image + proc->heap_offset, HEAP_PLUS_STACK_SIZE, PF_R | PF_W);
	// TODO: we should write args to the stack, and then initialize x0 and x1 to argc and argv
	(void)argv;
	memset(proc->regs.general, 0, sizeof proc->regs.general);
	proc->regs.general[0] = argc;
	proc->regs.sp = proc->size;
	proc->regs.elr = (u64)elf_header->info64.entry;
	proc->regs.spsr = 0;
	return 0;
}

// Create process image from program stored at path
i32 _proc_program(struct Proc* proc, const char* path, const char* argv[]) {
	struct FSNode* node = fs_find(path);
	if (node == NULL) {
		PRINT_ERROR("Node not found.");
		print("_proc_program: path was '");
		print(path);
		print("'.\n");
		return -1;
	}
	if (node->type != FS_NODE_TYPE_REG) {
		PRINT_ERROR("Node is not a regular file.");
		return -1;
	}
	u8 argc = 0;
	for (; argv[argc] != NULL; ++argc) {
	}
	if (_proc_image(proc, node->data, argc, argv) == -1) {
		return -1;
	}
	return 0;
}

char* proc_getcwd(char* buf, usize size) {
	usize length = strlen(_procs[_current]->cwd) + 1;
	if (length > size) {
		return NULL;
	}
	return memcpy(buf, _procs[_current]->cwd, length);
}

int proc_chdir(const char* path) {
	usize length = strnlen(path, PATH_MAX_LENGTH + 1);
	if (length == PATH_MAX_LENGTH + 1) {
		return -1;
	}
	memcpy(_procs[_current]->cwd, path, length + 1);
	return 0;
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

i16 proc_fork(void) {
	struct Proc* parent = _procs[_current];
	// We need the most up to date registers
	_proc_store_regs(parent);
	struct Proc* child = _proc_create(parent->cwd);
	child->size = parent->size;
	child->heap_offset = parent->heap_offset;
	child->image = kmalloc_page_align(child->size);
	memcpy(child->fd_table, parent->fd_table, sizeof child->fd_table);
	memcpy(child->image, parent->image, child->size);
	const i64 image_offset = (i64)child->image - (i64)parent->image;
	_proc_copy_table(image_offset, child->l1_table.entries, parent->l1_table.entries, L1_TABLE_ENTRY_COUNT);
	_proc_copy_table(image_offset, child->l2_table.entries, parent->l2_table.entries, L2_TABLE_ENTRY_COUNT);
	_proc_copy_table(image_offset, child->l3_table.entries, parent->l3_table.entries, L3_TABLE_ENTRY_COUNT);
	memcpy(child->regs.general, &parent->regs.general, sizeof child->regs.general);
	child->regs.sp = parent->regs.sp;
	child->regs.spsr = parent->regs.spsr;
	child->regs.elr = parent->regs.elr;
	child->regs.general[0] = 0;
	return child->id;
}

i32 proc_execve(const char* path, const char* argv[], const char* envp[]) {
	char path_copy[PATH_MAX_LENGTH + 1];
	strncpy(path_copy, path, PATH_MAX_LENGTH);
	path_copy[PATH_MAX_LENGTH] = '\0';
	// TODO: environment
	(void)argv;
	(void)envp;
	struct Proc* proc = _procs[_current];
	kfree(proc->image);
	const i16 pid = proc->id;
	_proc_init(proc, pid, "/");
	const char* my_argv[] = { path_copy, NULL };
	if (_proc_program(proc, path_copy, my_argv) == -1) {
		printf("[proc] _proc_program() failed.\n");
		return -1;
	}
	// Invalidate all TLB entries with ASID of this process
	const u64 asid = pid;
	asm volatile (
		"tlbi	aside1, %0\n"
		"isb"
		:
		: "r" (asid)
	);
	_proc_load_regs_and_run(proc);
	__builtin_unreachable();
}

_Noreturn void proc_kill(void) {
	printf("[proc] Killing process (pid=%).\n", (u64)_procs[_current]->id);
	kfree(_procs[_current]->image);
	kfree(_procs[_current]);
	--_proc_count;
	for (usize i = _current; i < _proc_count; ++i) {
		_procs[i] = _procs[i + 1];
	}
	_proc_schedule_next();
	_proc_load_regs_and_run(_procs[_current]);
}

i16 proc_getpid(void) {
	return _procs[_current]->id;
}

_Noreturn void proc_queue_read(i32 fd, void* dst, usize size) {
	_proc_store_regs(_procs[_current]);
	_procs[_current]->pending_read.fd = fd;
	_procs[_current]->pending_read.dst = dst;
	_procs[_current]->pending_read.size = size;
	_procs[_current]->state = PROC_STATE_SLEEP;
	_proc_schedule_next();
	_proc_load_regs_and_run(_procs[_current]);
}

_Noreturn void proc_update_pending_io(i32 fd) {
	u64 saved_ttbr;
	asm volatile (
		"mrs	%0, ttbr0_el1\n"
		: "=r" (saved_ttbr)
	);
	for (usize i = 0; i < _proc_count; ++i) {
		if (_procs[i]->state != PROC_STATE_SLEEP || _procs[i]->pending_read.fd != fd) {
			continue;
		}
		if (_procs[i]->pending_read.size == 0) {
			_procs[i]->state = PROC_STATE_RUN;
			continue;
		}
		const u64 ttbr = _procs[i]->regs.ttbr;
		asm volatile (
			"msr	ttbr0_el1, %0\n"
			"isb"
			:
			: "r" (ttbr)
		);
		*(char*)(_procs[i]->pending_read.dst++) = (char)AUX_MU->io_reg;
		--_procs[i]->pending_read.size;
		if (_procs[i]->pending_read.size == 0) {
			_procs[i]->state = PROC_STATE_RUN;
		}
	}
	asm volatile (
		"msr	ttbr0_el1, %0\n"
		"isb"
		:
		: "r" (saved_ttbr)
	);
	_proc_schedule_next();
	_proc_load_regs_and_run(_procs[_current]);
}
