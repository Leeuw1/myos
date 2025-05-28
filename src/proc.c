#include "proc.h"
#include "heap.h"
#include "elf.h"
#include "fs.h"
#include "io.h"
#include "regs.h"
#include "exception.h"
#include <string.h>

#define MAX_PROCS		4
#define PATH_MAX_LENGTH	127
// Max file descriptors that a process can have open at the same time
#define MAX_FD_ENTRIES	4

#define L0_TABLE_ENTRY_COUNT	1
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
#define DESC_FLAGS_PXN			BIT(53)
#define DESC_FLAGS_UXN			BIT(54)

struct FDEntry {
	char	path[PATH_MAX_LENGTH + 1];
	bool	in_use;
};

struct L0Table {
	u64	entries[L0_TABLE_ENTRY_COUNT];
} ALIGN(8);
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
	struct L0Table	l0_table;
	char			cwd[PATH_MAX_LENGTH + 1];
	void*			image;
	usize			size;
	usize			heap_offset;
	i32				id;
};

static struct Proc* _procs[MAX_PROCS];
static usize _proc_count;
static usize _current;

static struct Proc* _proc_create(const char* cwd);
static i32 _proc_program(struct Proc* proc, const char* path, char* argv[]);

i32 _proc_new_id(void) {
	static i32 id = 1;
	return id++;
}

static struct Proc* _proc_create(const char* cwd) {
	if (_proc_count == MAX_PROCS) {
		return NULL;
	}
	struct Proc* proc = kmalloc_page_align(sizeof *proc);
	// NOTE: if this memset is removed, just make sure zero the relevant table descriptors
	memset(proc, 0, sizeof *proc);
	proc->id = _proc_new_id();
	strncpy(proc->cwd, cwd, PATH_MAX_LENGTH);

	proc->l0_table.entries[0] = ((u64)&proc->l1_table & 0xffffffff)
		| DESC_FLAGS_TABLE;
	proc->l1_table.entries[0] = ((u64)&proc->l2_table & 0xffffffff)
		| DESC_FLAGS_TABLE;
	proc->l2_table.entries[0] = ((u64)&proc->l3_table & 0xffffffff)
		| DESC_FLAGS_TABLE;

	const char* path = "/dev/tty";
	strcpy(proc->fd_table[0].path, path);
	proc->fd_table[0].in_use = true;
	strcpy(proc->fd_table[1].path, path);
	proc->fd_table[1].in_use = true;
	strcpy(proc->fd_table[2].path, path);
	proc->fd_table[2].in_use = true;

	_procs[_proc_count++] = proc;
	return proc;
}

// Entry point to process-handling code
void proc_main(void) {
	_proc_count = 0;
	_current = 0;
	// System shell, a process that cannot be deleted
	const char* argv[] = { "/sh", NULL };
	if (_proc_program(_proc_create("/"), "/sh", (char**)argv) == -1) {
		printf("[proc] _proc_program() failed.\n");
		return;
	}
#if 0
	SYS_TIMER->c0 = SYS_TIMER->clo + 0xfffff;
	INTERRUPTS->enable_irqs_1 = 1;
	irq_enable();
	asm ("wfi");
#endif
}

void proc_prepare_next(u64* general_regs) {
	print("Preparing next process...\n");
	// We should not read registers for the first context switch
	static bool context_switched_before = false;
	if (context_switched_before) {
		struct Proc* proc = _procs[_current];
		memcpy(proc->regs.general, general_regs, sizeof proc->regs.general);
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
	else {
		context_switched_before = true;
	}
	// Schedule next process
	_current = (_current + 1) % _proc_count;
	struct Proc* proc = _procs[_current];
	memcpy(general_regs, proc->regs.general, sizeof proc->regs.general);
	const u64 sp = proc->regs.sp;
	const u64 spsr = proc->regs.spsr;
	const u64 elr = proc->regs.elr;
	const u64 ttbr = proc->regs.ttbr;
	asm volatile (
		"msr	sp_el0, %0\n"
		"msr	spsr_el1, %1\n"
		"msr	elr_el1, %2\n"
		"msr	ttbr0_el1, %3\n"
		"isb"
		:
		: "r" (sp), "r" (spsr), "r" (elr), "r" (ttbr)
	);
	if (!context_switched_before) {
		asm (
			"tlbi	ALLE1\n"
			"isb"
		);
	}
	print("Switching process\n");
	printf("SP:\t%\n", sp);
	printf("SPSR:\t%\n", spsr);
	printf("ELR:\t%\n", elr);
	printf("TTBR:\t%\n", ttbr);
	printf("L0 Table: %\n", (u64)&proc->l0_table);
	printf("L1 Table: %\n", (u64)&proc->l1_table);
	printf("L2 Table: %\n", (u64)&proc->l2_table);
	printf("L3 Table: %\n", (u64)&proc->l3_table);
	printf("Translation result: %\n", try_translate((void*)0));
	printf("Translation result: %\n", try_translate((void*)elr));
	while ((volatile bool)true) {
	}
}

// TODO: at what point should we check the length of path?
i32 proc_open_fd(const char* path) {
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
	const usize page_count = (size + (virt_addr & 0xfff) + PAGE_SIZE - 1) / PAGE_SIZE;
	printf("Mapping % pages.\n", (u64)page_count);
	printf("Virtual address: %.\n", virt_addr);
	const usize start_index = (usize)virt_addr >> 12;
	printf("L3 Table Index: %.\n", (u64)start_index);
	kernel_addr &= ~0xfff;
	kernel_addr &= 0xffffffff;
	for (usize i = 0; i < page_count; ++i) {
		u64 attributes = DESC_FLAGS_AF | DESC_FLAGS_PXN;
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
		proc->l3_table.entries[start_index + i] = (u64)(kernel_addr + i * PAGE_SIZE)
			| DESC_FLAGS_PAGE
			| attributes;
	}
}

#define HEAP_PLUS_STACK_SIZE	0x8000

i32 _proc_image(struct Proc* proc, const void* elf, int argc, char* argv[]) {
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
	// NOTE: *image should be aligned to the size of the largest translation table (T3)
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
	proc->regs.general[0] = argc;
	//proc->regs.sp = (u64)(proc->image + proc->size - argv_size);
	//proc->regs.general[1] = proc->regs.sp;
	proc->regs.sp = proc->size;
	proc->regs.elr = (u64)elf_header->info64.entry;
	proc->regs.ttbr = (u64)&proc->l0_table & 0xffffffff;
	return 0;
}

// Create process image from program stored at path
i32 _proc_program(struct Proc* proc, const char* path, char* argv[]) {
	struct FSNode* node = fs_find(path);
	if (node == NULL) {
		PRINT_ERROR("Node not found.");
		print("_proc_program: path was '");
		print(path);
		print("'.\n");
		return -1;
	}
	if ((node->stat.st_mode & S_IFMT) != S_IFREG) {
		PRINT_ERROR("Node is not a regular file.");
		return -1;
	}
	int argc = 0;
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
