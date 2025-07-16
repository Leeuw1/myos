.equ SYSCALL_EXIT, 9

.section .text
.globl _start
_start:
	stp		x0, x1, [sp, #-16]!
	mov		x0,	x2
	mov		x1,	x3
	bl		_libc_init_stdlib
	bl		_libc_init_signal
	bl		_libc_init_stdio
	bl		_libc_init_locale
	ldp		x0, x1, [sp], #16
	bl		main
	svc		#SYSCALL_EXIT
