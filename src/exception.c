#include "exception.h"
#include "io.h"
#include "regs.h"
#include "syscall.h"
#include "proc.h"

void enter_el0(void) {
	asm (
		"mov	x0, #0b0000\n"
		"msr	spsr_el1, x0\n"
		"adr	x0, el0_begin\n"
		"msr	elr_el1, x0\n"
		"eret\n"
		"el0_begin:"
		:
		:
		: "x0"
	);
}

u8 get_current_el(void) {
	register u8 el asm ("x0");
	asm (
		"mrs x0, CurrentEL"
		: "=r" (el)
	);
	return el >> 2;
}

bool mmu_enabled(void) {
	register u8 status asm ("x0");
	asm (
		"mrs x0, sctlr_el1"
		: "=r" (status)
	);
	return status & 1;
}

u64 try_translate(void* addr) {
	u64 result = 0;
	asm (
		"at s1e1r, %1\n"
		"mrs %0, par_el1"
		: "=r" (result)
		: "r" (addr)
	);
	return result;
}

void irq_enable(void) {
	asm ("msr daifclr, #2");
}

void irq_disable(void) {
	asm ("msr daifset, #2");
}

void irq_handler(u64* general_regs) {
	SYS_TIMER->cs |= 1;
	SYS_TIMER->c0 = SYS_TIMER->clo + 0xfffff;
	static u32 counter = 0;
	++counter;
	printf("IRQ Exception. counter=%\n", (u64)counter);
	const bool switching_proc = true;
	if (switching_proc) {
		proc_prepare_next(general_regs);
	}
}

void fiq_handler(void) {
	print("[EXCEPTION] FIQ Exception Occurred.\n");
}

#define CLASS_SVC_AARCH64		0b010101
#define CLASS_PC_ALIGN_FAULT_EX	0b100010
#define CLASS_SP_ALIGN_FAULT_EX	0b100110
#define CLASS_MEMORY_OP_EX		0b100111

static u8 get_exception_class(void) {
	register u8 ec asm ("x0");
	asm (
		"mrs x0, esr_el1\n"
		"lsr x0, x0, #26\n"
		"and x0, x0, #0x3f\n"
		: "=r" (ec)
	);
	return ec;
}

static u16 get_svc_arg(void) {
	register u16 arg asm ("x0");
	asm (
		"mrs x0, esr_el1\n"
		"and x0, x0, #0xffff\n"
		: "=r" (arg)
	);
	return arg;
}

static void print_exception_class(void) {
	print("Exception Class: ");
	u8 ec = get_exception_class();
	switch (ec) {
	default:
		printf("Unknown/undocumented (ec=%)\n", (u64)ec);
		break;
	case CLASS_SVC_AARCH64:
		print("SVC call in aarch64\n");
		break;
	case CLASS_PC_ALIGN_FAULT_EX:
		print("PC alignment fault exception\n");
		break;
	case CLASS_SP_ALIGN_FAULT_EX:
		print("SP alignment fault exception\n");
		break;
	case CLASS_MEMORY_OP_EX:
		print("Memory operation exception\n");
		break;
	}
}

void synchronous_handler(union SyscallArgs* args) {
	static u32 counter = 0;
	++counter;
	if (counter < 10) {
		print("[EXCEPTION] Synchronous Exception Occurred.\n");
		print_exception_class();
	}
	if (get_exception_class() != CLASS_SVC_AARCH64) {
		return;
	}
	print("SYSCALL\n");
	volatile bool b = true;
	while (b) {
	}
	switch (get_svc_arg()) {
	case SYSCALL_COMMAND:
		args->command.retval = syscall_command(args->command.cmd, args->command.len);
		return;
	case SYSCALL_READ:
		args->read.retval = syscall_read(args->read.fd, args->read.buf, args->read.count);
		return;
	case SYSCALL_WRITE:
		args->write.retval = syscall_write(args->write.fd, args->write.buf, args->write.count);
		return;
	case SYSCALL_GETCWD:
		args->getcwd.retval = proc_getcwd(args->getcwd.buf, args->getcwd.size);
		return;
	case SYSCALL_CHDIR:
		args->chdir.retval = proc_chdir(args->chdir.path);
		return;
	default:
		printf("Unknown syscall!\n");
		return;
	}
}

void serror_handler(void) {
	static u32 counter = 0;
	++counter;
	if (counter < 10) {
		print("[EXCEPTION] SError Exception Occurred.\n");
		print_exception_class();
	}
}
