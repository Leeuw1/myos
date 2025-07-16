.section .data
longjmp_msg:
.asciz "setjmp()\n"
setjmp_msg:
.asciz "setjmp()\n"

.section .text

.globl longjmp
longjmp:
	mov		x3, x0
	adr		x0, longjmp_msg
	bl		puts
	mov		x0, x3

	ldr		x2, [x0, #8]
	mov		sp, x2
	ldr		x2, [x0, #16]
	mov		x0, x1
	tst		x0, x0
	cset	x0, eq
	br		x2

.globl setjmp
setjmp:
	mov		x3, x0
	adr		x0, setjmp_msg
	bl		puts
	mov		x0, x3

	mov		x1, sp
	str		x1, [x0, #8]
	str		lr, [x0, #16]
	mov		x0, xzr
	ret
