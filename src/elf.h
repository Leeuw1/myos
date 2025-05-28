#ifndef _ELF_H
#define _ELF_H

#include "types.h"

#define ELF_HEADER_MAGIC	0x464c457f
#define PT_LOAD				0x1
#define PF_X				0x1
#define PF_W				0x2
#define PF_R				0x4

struct ELFHeader {
	struct {
		u32	magic;
		u8	class;
		u8	data;
		u8	version;
		u8	os_abi;
		u8	abi_version;
		u8	_pad[7];
	} ident;
	union {
		struct {
			u16	type;
			u16	machine;
			u32	version;
			u32	entry;
			u32	ph_off;
			u32	sh_off;
			u32	flags;
			u16	eh_size;
			u16	ph_ent_size;
			u16	ph_num;
			u16	sh_ent_size;
			u16	sh_num;
			u16	sh_str_ndx;
		} info32;
		struct {
			u16	type;
			u16	machine;
			u32	version;
			u64	entry;
			u64	ph_off;
			u64	sh_off;
			u32	flags;
			u16	eh_size;
			u16	ph_ent_size;
			u16	ph_num;
			u16	sh_ent_size;
			u16	sh_num;
			u16	sh_str_ndx;
		} info64;
	};
} __attribute__((packed));

struct ProgramHeader64 {
	u32	type;
	u32	flags;
	u64	offset;
	u64	v_addr;
	u64	p_addr;
	u64	file_sz;
	u64	mem_sz;
	u64	align;
} __attribute__((packed));

/*
struct SectionHeader64 {
	u32	name;
	u32	type;
	u64	flags;
	u64	addr;
	u64	offset;
	u64	size;
	u32	link;
	u32	info;
	u64	addr_align;
	u64	ent_size;
} __attribute__((packed));
*/

#endif //_ELF_H
