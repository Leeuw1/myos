#ifndef _REGS_H
#define _REGS_H

#include "core.h"

#define AUX			((volatile struct AuxRegs*)			0xffffffff3f215000)
#define AUX_MU		((volatile struct AuxMURegs*)		0xffffffff3f215040)
#define	SYS_TIMER	((volatile struct SysTimerRegs*)	0xffffffff3f003000)
#define	GPIO		((volatile struct GPIORegs*)		0xffffffff3f200000)
#define PCM			((volatile struct PCMRegs*)			0xffffffff3f203000)
#define UART		((volatile struct UARTRegs*)		0xffffffff3f201000)
#define INTERRUPTS	((volatile struct InterruptRegs*)	0xffffffff3f00b200)
#define EMMC		((volatile struct EMMCRegs*)		0xffffffff3f300000)

struct InterruptRegs {
	u32	irq_basic_pending;
	u32	irq_pending_1;
	u32	irq_pending_2;
	u32	fiq_control;
	u32	enable_irqs_1;
	u32	enable_irqs_2;
	u32	enable_basic_irqs;
	u32	disable_irqs_1;
	u32	disable_irqs_2;
	u32	disable_basic_irqs;
} __attribute__((packed));

struct AuxRegs {
	u32	irq;
	u32	enables;
} __attribute__((packed));

struct AuxMURegs {
	u32	io_reg;
	u32	ier_reg;
	u32	iir_reg;
	u32	lcr_reg;
	u32	mcr_reg;
	u32	lsr_reg;
	u32	msr_reg;
	u32	scratch;
	u32	cntl_reg;
	const u32	stat_reg;
	u32	baud_reg;
} __attribute__((packed));

struct SysTimerRegs {
	u32	cs;
	u32	clo;
	u32	chi;
	u32	c0;
	u32	c1;
	u32	c2;
	u32	c3;
} __attribute__((packed));

struct GPIORegs {
	u32		fsel0;
	u32		fsel1;
	u32		fsel2;
	u32		fsel3;
	u32		fsel4;
	u32		fsel5;
	u32	_pad0;
	u32		set0;
	u32		set1;
	u32	_pad1;
	u32		clr0;
	u32		clr1;
	u32	_pad2;
	const u32	lev0;
	const u32	lev1;
	u32	_pad3;
	u32		eds0;
	u32		eds1;
	u32	_pad4;
	u32		ren0;
	u32		ren1;
	u32	_pad5;
	u32		fen0;
	u32		fen1;
	u32	_pad6;
	u32		hen0;
	u32		hen1;
	u32	_pad7;
	u32		len0;
	u32		len1;
	u32	_pad8;
	u32		aren0;
	u32		aren1;
	u32	_pad9;
	u32		afen0;
	u32		afen1;
	u32	_pad10;
	u32	pud;
	u32	pudclk0;
	u32	pudclk1;
} __attribute__((packed));

struct PCMRegs {
	volatile struct {
		u32			en:1;
		u32			rxon:1;
		u32			txon:1;
		u32			txclr:1;
		u32			rxclr:1;
		u32			txthr:2;
		u32			rxthr:2;
		u32			dmaen:1;
		u32			:3;
		u32			txsync:1;
		u32			rxsync:1;
		u32			txerr:1;
		u32			rxerr:1;
		u32			txw:1;
		u32			rxr:1;
		u32			txd:1;
		u32			rxd:1;
		u32			txe:1;
		u32			rxf:1;
		u32			rxsex:1;
		u32			sync:1;
		u32			stby:1;
		u32			:6;
	} cs_a;
	volatile u32	fifo_a;
	volatile struct {
		u32			fslen:10;
		u32			flen:10;
		u32			fsi:1;
		u32			fsm:1;
		u32			clki:1;
		u32			clkm:1;
		u32			ftxp:1;
		u32			frxp:1;
		u32			pdme:1;
		u32			pdmn:1;
		u32			clk_dis:1;
		u32			:3;
	} mode_a;
	volatile struct {
		u32			ch2wid:4;
		u32			ch2pos:10;
		u32			ch2en:1;
		u32			ch2wex:1;
		u32			ch1wid:4;
		u32			ch1pos:10;
		u32			ch1en:1;
		u32			ch1wex:1;
	} rxc_a, txc_a;
	volatile u32	dreq_a;
	volatile u32	inten_a;
	volatile u32	intstc_a;
	volatile u32	gray;
} __attribute__((packed));

struct UARTRegs {
	/*
	volatile struct {
		u32			data:8;
		u32			fe:1;
		u32			pe:1;
		u32			be:1;
		u32			oe:1;
		u32			:20;
	} dr;
	*/
	volatile u32	dr;
	volatile u32	rsrecr;
	u8	_pad0[0x10 - 0x8];
	volatile u32	fr;
	u8	_pad1[0x24 - 0x1c];
	// TODO: double check these paddings
	volatile u32	irbd;
	volatile u32	frbd;
	volatile u32	lrch;
	volatile struct {
		u32			uarten:1;
		u32			siren:1;
		u32			sirlp:1;
		u32			:4;
		u32			lbe:1;
		u32			txe:1;
		u32			rxe:1;
		u32			:1;
		u32			rts:1;
		u32			out1:1;
		u32			rtsen:1;
		u32			ctsen:1;
		u32			:16;
	} cr;
	volatile u32	ifls;
	volatile u32	imsc;
	volatile u32	ris;
	volatile u32	mis;
	volatile u32	icr;
	volatile u32	dmacr;
	volatile u32	itcr;
	volatile u32	itip;
	volatile u32	itop;
	volatile u32	tdr;
} __attribute__((packed));

struct EMMCRegs {
	u32	arg2;
	u32	blk_size_cnt;
	u32	arg1;
	u32	cmd_tm;
	u32	resp0;
	u32	resp1;
	u32	resp2;
	u32	resp3;
	u32	data;
	u32	status;
	u32	control0;
	u32	control1;
	u32	interrupt;
	u32	irpt_mask;
	u32	irpt_en;
	u32	control2;
	u8	_pad[0x10];
	u32	force_irpt;
	// ...
} __attribute__((packed));

#endif // _REGS_H
