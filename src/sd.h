#ifndef _SD_H
#define _SD_H

#include "core.h"

bool sd_init(void);
//bool sd_read_sector(void* dest, u32 src);
bool sd_read_sectors(void* dest, u32 sector_index, u16 sector_count);

#endif //_SD_H
