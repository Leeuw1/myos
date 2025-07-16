.section .init
.globl _init
_init:
	mrs		x0, CurrentEL
	cmp		x0, #0b1000
	beq		el2_begin
	cmp		x0, #0b0100
	beq		el1_begin

	// Change from EL3 to EL2
	mov		x0, #0x5b1
	msr		scr_el3, x0
    mov     x0, #0x3c9
    msr     spsr_el3, x0
    adr     x0, el2_begin
    msr     elr_el3, x0
	eret

el2_begin:
	// Change from EL2 to EL1

	// enable CNTP for EL1
    mrs     x0, cnthctl_el2
    orr     x0, x0, #3
    msr     cnthctl_el2, x0
    msr     cnthp_ctl_el2, xzr
    // initialize virtual MPIDR
    mrs     x0, midr_el1
    mrs     x2, mpidr_el1
    msr     vpidr_el2, x0
    msr     vmpidr_el2, x2
    // disable coprocessor traps
    mov     x0, #0x33FF
    msr     cptr_el2, x0
    msr     hstr_el2, xzr
    mov     x0, #(3 << 20)
    msr     cpacr_el1, x0
    // enable AArch64 in EL1
    mov     x0, #(1 << 31)      // AArch64
    orr     x0, x0, #(1 << 1)   // SWIO hardwired on Pi3
    msr     hcr_el2, x0
    mrs     x0, hcr_el2
    // Setup SCTLR access
    //mov     x0, #0x0800
    ////movk    x0, #0x30d0, lsl #16
    //movk    x0, #0x80, lsl #16
	mov		x0, #(1 << 22)
	orr		x0, x0, #(1 << 11)
    msr     sctlr_el1, x0
    // change exception level to EL1
    //mov     x0, #0x3c4
    mov     x0, #0x3c5
    msr     spsr_el2, x0
    adr     x0, el1_begin
    msr     elr_el2, x0
	eret

el1_begin:
	ldr		x0, =vectors_el1
	msr		vbar_el1, x0

	// Isolate Core 0
	mrs		x0, mpidr_el1
	tst		x0, #0xff
	beq		core0
halt:
	wfe
	b		halt

core0:
	msr		daifset, #0xf

	mov		sp, #0xffffffff00000000
	add		sp, sp, #0x80000

	// Set up MMU

	// Granule size is 4KB
	mov		x0, #(0b10 << 30)
	// T0SZ
	mov		x1, #32
	orr		x0, x0, x1
	// T1SZ
	mov		x1, #(32 << 16)
	orr		x0, x0, x1
	// Use 16 bit ASID
	orr		x0, x0, #(1 << 36)
	// Disable hierarchical permissions
	//orr		x0, x0, #(0b11 << 41)
	msr		tcr_el1, x0

	// attr0 is Normal Memory
	// attr1 is Device Memory -> use attr index 1 for peripheral mapping
	// For now, we will set normal memory to non-cacheable
	mov		x0, #0x44
	msr		mair_el1, x0

	// Descriptors
	adr		x1, _l2_table
	orr		x1, x1, #0b11
	mov		x2, #0b01
	// Use attr1 (device)
	orr		x2, x2, #(1 << 2)
	// For now, we will set Access Flag preemptively to avoid faults
	orr		x2, x2, #(1 << 10)

	// Translation tables
	adr		x0, _l2_table
	str		x2, [x0]
	adr		x0, _l1_table
	str		x1, [x0]
	msr		ttbr1_el1, x0
	msr		ttbr0_el1, x0 
	isb

	mrs		x0, sctlr_el1
	orr		x0, x0, #1
	msr		sctlr_el1, x0
	isb

	b		kmain
