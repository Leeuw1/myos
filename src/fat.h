#ifndef _FAT_H
#define _FAT_H

#include "core.h"

struct FSNode;

void fat_mount_partition(u32 start_sector, u32 sector_count);
bool fat_create_fs_node(struct FSNode* dst, u32 id);

#endif //_FAT_H
