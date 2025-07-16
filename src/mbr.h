#ifndef _MBR_H
#define _MBR_H

#include "core.h"

struct MBRPartitionEntry {
	u8	attr;
	u8	start_chs[3];
	u8	type;
	u8	end_chs[3];
	u32	start_sector;
	u32	sector_count;
} __attribute__((packed));

struct MBR {
	u8							_pad[446];
	struct MBRPartitionEntry	partitions[4];
	u16							magic;
} __attribute__((packed));

#endif //_MBR_H
