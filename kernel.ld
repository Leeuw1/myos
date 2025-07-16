ENTRY(_init)

MEMORY {
	stub	(rwx)	: org = 0x80000, len = 0x8000
	/*
	rom		(rx)	: org = 0x88000, len = 0x14000
	ram		(rw)	: org = 0x9c000, len = 0x10000
	v_rom	(rx)	: org = 0xffffffff00088000, len = 0x14000
	v_ram	(rw)	: org = 0xffffffff0009c000, len = 0x10000
	*/
}

SECTIONS {
	.init : {
		*(.init)
		_l2_table = ALIGN(32);
		. += 32;
		_l1_table = ALIGN(8);
		. += 8;
	} > stub

	.rom 0xffffffff00088000 : AT(0x88000) {
		_TEXT_START_ = .;
		*(.got)
		*(.got.plt)
		*(.text)
		_TEXT_END_ = .;
	}

	.ram 0xffffffff0009c000 : AT(0x9c000) {
		_DATA_START_ = .;
		*(.programs)
		*(.rodata)
		*(.rodata.str1.8)
		*(.data.rel.ro.local)
		*(.data)
		_DATA_END_ = .;
		. = ALIGN(8);
		_BSS_START_ = .;
		*(.bss)
		_BSS_END_ = .;
		_HEAP_START_ = ALIGN(4096);
	}
}
