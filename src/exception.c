#include "exception.h"
#include "io.h"
#include "syscall.h"
#include "proc.h"
#include "commands.h"
#include "regs.h"
#include <signal.h>

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
		proc_update_pending_io();
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

static void* _fault_address(void) {
	register void* far asm ("x0");
	asm (
		"mrs x0, far_el1"
		: "=r" (far)
	);
	return far;
}

static void* _elr_address(void) {
	register void* elr asm ("x0");
	asm (
		"mrs x0, elr_el1"
		: "=r" (elr)
	);
	return elr;
}

static u64 _sp_el0(void) {
	register u64 sp asm ("x0");
	asm (
		"mrs x0, sp_el0"
		: "=r" (sp)
	);
	return sp;
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
#if 0
	printf("Synchronous error occurred at instruction address: %\n", _elr_address());
	if (!(try_translate((void*)_elr_address()) & 1)) {
		printf("Instruction: %\n", (u64)*(u32*)(_elr_address()));
	}
	printf("Current EL: %\n", (u64)get_current_el());
	print("Exception Class: ");
#endif
	const u8 ec = get_exception_class();
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
		{
			const siginfo_t info = {
				.si_value.sival_ptr = _elr_address(),
			};
			proc_send_signal(SIGILL, &info);
			proc_run_next();
		}
	case CLASS_INST_ABORT_SAME_EL_EX:
		print("Instruction abort from same exception level (EL1)\n");
		printf("Instruction address: %\n", _elr_address());
		_print_data_abort_info();
		break;
	case CLASS_DATA_ABORT_LOWER_EL_EX:
		{
			print("Segmentation Violation from EL0\n");
			printf("Instruction address: %\n", _elr_address());
			_print_data_abort_info();
			const siginfo_t info = {
				.si_addr = _fault_address(),
				.si_value.sival_ptr = _elr_address(),
			};
			proc_send_signal(SIGSEGV, &info);
			proc_run_next();
		}
	case CLASS_DATA_ABORT_SAME_EL_EX:
		print("Data abort from same exception level (EL1)\n");
		printf("Instruction address: %\n", _elr_address());
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
	default:
		printf("Unknown syscall!\n");
		return;
	case SYSCALL_READ:
		args->read.retval = proc_read(args->read.fd, args->read.buf, args->read.count);
		return;
	case SYSCALL_WRITE:
		args->write.retval = proc_write(args->write.fd, args->write.buf, args->write.count);
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
		proc_exit(args->exit.status);
	case SYSCALL_OPEN:
		args->open.retval = proc_open(args->open.path, args->open.flags);
		return;
	case SYSCALL_WAITPID:
		/*args->waitpid.retval = */proc_waitpid(args->waitpid.pid);
		return;
	case SYSCALL_UNLINK:
		args->unlink.retval = proc_unlink(args->unlink.path);
		return;
	case SYSCALL_RMDIR:
		args->rmdir.retval = proc_rmdir(args->rmdir.path);
		return;
	case SYSCALL_TIME:
		args->time.retval = syscall_time();
		return;
	case SYSCALL_POSIX_GETDENTS:
		args->posix_getdents.retval = proc_getdents(
			args->posix_getdents.fd,
			args->posix_getdents.buf,
			args->posix_getdents.size,
			args->posix_getdents.flags
		);
		return;
	case SYSCALL_CLOSE:
		args->close.retval = proc_close(args->close.fd);
		return;
	case SYSCALL_RENAME:
		args->rename.retval = proc_rename(args->rename.oldpath, args->rename.newpath);
		return;
	case SYSCALL_MKDIR:
		args->mkdir.retval = proc_mkdir(args->mkdir.path, args->mkdir.mode);
		return;
	case SYSCALL_FSTAT:
		args->fstat.retval = proc_fstat(args->fstat.fd, args->fstat.buf);
		return;
	case SYSCALL_TCGETATTR:
		args->tcgetattr.retval = proc_tcgetattr(args->tcgetattr.fd, args->tcgetattr.buf);
		return;
	case SYSCALL_TCSETATTR:
		args->tcsetattr.retval = proc_tcsetattr(args->tcsetattr.fd, args->tcsetattr.opt, args->tcsetattr.buf);
		return;
	case SYSCALL_NANOSLEEP:
		args->nanosleep.retval = proc_nanosleep(args->nanosleep.rqtp, args->nanosleep.rmtp);
		return;
	case SYSCALL_LSEEK:
		args->lseek.retval = proc_lseek(args->lseek.fd, args->lseek.offset, args->lseek.whence);
		return;
	case SYSCALL_SIGACTION:
		args->sigaction.retval = proc_sigaction(
			args->sigaction.sig,
			args->sigaction.mask,
			args->sigaction.flags,
			args->sigaction.handler,
			args->sigaction.wrapper
		);
		return;
	case SYSCALL_SIGRETURN:
		proc_sigreturn();
	case SYSCALL_GROW_HEAP:
		proc_grow_heap(args->grow_heap.size);
		return;
	case SYSCALL_CANONICALIZE:
		args->canonicalize.retval = proc_canonicalize(args->canonicalize.path, args->canonicalize.dst);
		return;
	}
}

void serror_handler(void) {
	print("[EXCEPTION] SError Exception Occurred.\n");
}
