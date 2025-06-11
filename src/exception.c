#include "exception.h"
#include "io.h"
#include "syscall.h"
#include "proc.h"
#include "commands.h"
#include "regs.h"

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

u64 try_translate(const void* addr) {
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

static void _aux_mu_irq_handler(void) {
	const u8 status = (AUX_MU->iir_reg & 0b110) >> 1;
	if (status == 0b10) {
		proc_update_pending_io(0);
	}
}

void irq_handler(void) {
	switch (INTERRUPTS->irq_pending_1) {
	case 1:
		proc_run_next();
	case 1 << 29:
		_aux_mu_irq_handler();
		return;
	default:
		print("Unknown IRQ\n");
		return;
	}
}

void fiq_handler(void) {
	print("[EXCEPTION] FIQ Exception Occurred.\n");
}

#define CLASS_SVC_AARCH64				0b010101
#define CLASS_PC_ALIGN_FAULT_EX			0b100010
#define CLASS_INST_ABORT_LOWER_EL_EX	0b100000
#define CLASS_INST_ABORT_SAME_EL_EX		0b100001
#define CLASS_DATA_ABORT_LOWER_EL_EX	0b100100
#define CLASS_DATA_ABORT_SAME_EL_EX		0b100101
#define CLASS_SP_ALIGN_FAULT_EX			0b100110
#define CLASS_MEMORY_OP_EX				0b100111

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

static u64 _fault_address(void) {
	register u64 far asm ("x0");
	asm (
		"mrs x0, far_el1"
		: "=r" (far)
	);
	return far;
}

static u64 _elr_address(void) {
	register u64 elr asm ("x0");
	asm (
		"mrs x0, elr_el1"
		: "=r" (elr)
	);
	return elr;
}

static void _print_data_abort_info(void) {
	register u32 iss asm ("x0");
	asm (
		"mrs x0, esr_el1\n"
		"and x0, x0, #0x1ffffff\n"
		: "=r" (iss)
	);
	printf("\tFault address: %\n", _fault_address());
	print("\tData fault status code: ");
	const u32 dfsc = iss & 0x2f;
	switch (dfsc) {
	default:
		printf("Unknown/undocumented (iss=%)\n");
		return;
	case 0b00:
	case 0b01:
	case 0b10:
	case 0b11:
		printf("Address size fault, level %\n", (u64)dfsc);
		return;
	case 0b100:
	case 0b101:
	case 0b110:
	case 0b111:
		printf("Translation fault, level %\n", (u64)(dfsc & 0b11));
		return;
	}
}

static void _synchronous_error(void) {
	printf("Synchronous error occurred at instruction address: %\n", _elr_address());
	print("Exception Class: ");
	u8 ec = get_exception_class();
	switch (ec) {
	default:
		printf("Unknown/undocumented (ec=%)\n", (u64)ec);
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
	case CLASS_INST_ABORT_LOWER_EL_EX:
		print("Instruction abort from lower exception level (EL0)\n");
		_print_data_abort_info();
		proc_kill();
	case CLASS_INST_ABORT_SAME_EL_EX:
		print("Instruction abort from same exception level (EL1)\n");
		_print_data_abort_info();
		break;
	case CLASS_DATA_ABORT_LOWER_EL_EX:
		print("Data abort from lower exception level (EL0)\n");
		_print_data_abort_info();
		proc_kill();
	case CLASS_DATA_ABORT_SAME_EL_EX:
		print("Data abort from same exception level (EL1)\n");
		_print_data_abort_info();
		break;
	}
	shutdown(0, NULL);
}

void synchronous_handler(union SyscallArgs* args) {
	if (get_exception_class() != CLASS_SVC_AARCH64) {
		print("[EXCEPTION] Synchronous Exception Occurred.\n");
		_synchronous_error();
		return;
	}
	switch (get_svc_arg()) {
	case SYSCALL_COMMAND:
		args->command.retval = syscall_command(args->command.argc, args->command.argv);
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
	case SYSCALL_FORK:
		args->fork.retval = proc_fork();
		return;
	case SYSCALL_EXECVE:
		args->execve.retval = proc_execve(args->execve.path, args->execve.argv, args->execve.envp);
		return;
	case SYSCALL_GETPID:
		args->getpid.retval = proc_getpid();
		return;
	case SYSCALL_EXIT:
		proc_kill();
	default:
		printf("Unknown syscall!\n");
		return;
	}
}

void serror_handler(void) {
	print("[EXCEPTION] SError Exception Occurred.\n");
}
