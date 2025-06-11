.macro save_regs
	mov		sp, #0xffffffff00000000
	add		sp, sp, #0x80000

	sub sp, sp, #256
	stp	x0, x1, [sp, #16 * 0]
	stp x2, x3, [sp, #16 * 1]
	stp	x4, x5, [sp, #16 * 2]
	stp	x6, x7, [sp, #16 * 3]
	stp	x8, x9, [sp, #16 * 4]
	stp	x10, x11, [sp, #16 * 5]
	stp	x12, x13, [sp, #16 * 6]
	stp	x14, x15, [sp, #16 * 7]
	stp	x16, x17, [sp, #16 * 8]
	stp	x18, x19, [sp, #16 * 9]
	stp	x20, x21, [sp, #16 * 10]
	stp	x22, x23, [sp, #16 * 11]
	stp	x24, x25, [sp, #16 * 12]
	stp	x26, x27, [sp, #16 * 13]
	stp	x28, x29, [sp, #16 * 14]
	str	x30, [sp, #16 * 15]

	mov x0, sp
.endm

.macro restore_regs
	ldp	x0, x1, [sp, #16 * 0]
	ldp	x2, x3, [sp, #16 * 1]
	ldp	x4, x5, [sp, #16 * 2]
	ldp	x6, x7, [sp, #16 * 3]
	ldp	x8, x9, [sp, #16 * 4]
	ldp	x10, x11, [sp, #16 * 5]
	ldp	x12, x13, [sp, #16 * 6]
	ldp	x14, x15, [sp, #16 * 7]
	ldp	x16, x17, [sp, #16 * 8]
	ldp	x18, x19, [sp, #16 * 9]
	ldp	x20, x21, [sp, #16 * 10]
	ldp	x22, x23, [sp, #16 * 11]
	ldp	x24, x25, [sp, #16 * 12]
	ldp	x26, x27, [sp, #16 * 13]
	ldp	x28, x29, [sp, #16 * 14]
	ldr	x30, [sp, #16 * 15] 
	add	sp, sp, #256
.endm

.section .text
.globl vectors_el1
.balign 2048
vectors_el1:
	b sp_el0_synchronous
	.balign 0x80
	b sp_el0_irq
	.balign 0x80
	b sp_el0_fiq
	.balign 0x80
	b sp_el0_serror

	.balign 0x80
	b sp_elx_synchronous
	.balign 0x80
	b sp_elx_irq
	.balign 0x80
	b sp_elx_fiq
	.balign 0x80
	b sp_elx_serror

	.balign 0x80
	b aarch64_synchronous
	.balign 0x80
	b aarch64_irq
	.balign 0x80
	b aarch64_fiq
	.balign 0x80
	b aarch64_serror

	.balign 0x80
	b aarch32_synchronous
	.balign 0x80
	b aarch32_irq
	.balign 0x80
	b aarch32_fiq
	.balign 0x80
	b aarch32_serror

sp_el0_synchronous:
sp_elx_synchronous:
aarch64_synchronous:
aarch32_synchronous:
	save_regs
	bl synchronous_handler
	restore_regs
	eret

sp_el0_irq:
sp_elx_irq:
aarch64_irq:
aarch32_irq:
	save_regs
	bl irq_handler
	restore_regs
	eret

sp_el0_fiq:
sp_elx_fiq:
aarch64_fiq:
aarch32_fiq:
	save_regs
	bl fiq_handler
	restore_regs
	eret

sp_el0_serror:
sp_elx_serror:
aarch64_serror:
aarch32_serror:
	save_regs
	bl serror_handler
	restore_regs
	eret
