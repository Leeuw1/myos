#include "fat.h"
#include "sd.h"
#include "io.h"
#include "fs.h"
#include "heap.h"
#include <string.h>

#define NODE_ATTR_READ_ONLY		0x01
#define NODE_ATTR_HIDDEN		0x02
#define NODE_ATTR_SYSTEM		0x04
#define NODE_ATTR_VOLUME_ID		0x08
#define NODE_ATTR_DIRECTORY		0x10
#define NODE_ATTR_ARCHIVE		0x20
#define NODE_ATTR_LONG_NAME		0x0f

#define LAST_LONG_NAME_ENTRY	0x40

#define DIRECTORY_ENTRY_UNUSED	0xe5
#define DIRECTORY_ENTRY_END		0x00

#define FAT_CLUSTER_FREE		0x0
#define FAT_CLUSTER_DEFECTIVE	0xfffffff7
#define FAT_CLUSTER_EOF			0xffffffff

struct FATInfo {
	usize	sector_size;
	usize	sectors_per_cluster;
	u32		root_cluster;
	u32		data_start_sector;
} _info;

struct FATPartitionData {
	u32		start_sector;
	u32		sector_count;
} _partition;

struct BPB {
	u8		jmp_boot[3];
	char	oem_name[8];
	u16		bytes_per_sector;
	u8		sectors_per_cluster;
	u16		reserved_sector_count;
	u8		fat_count;
	u16		_reserved0;
	u16		_reserved1;
	u8		media;
	u16		_reserved2;
	u16		sectors_per_track;
	u16		head_count;
	u32		hidden_sector_count;
	u32		total_sector_count;
	u32		sectors_per_fat;
	u16		flags;
	u16		_reserved3;
	u32		root_cluster;
	u16		fs_info_cluster;
	u16		backup_boot_sector;
	u8		_reserved4[12];
	u8		drive_number;
	u8		_reserved5;
	u8		boot_signature;
	u32		volume_id;
	char	volume_label[11];
	char	fs_type[8];
	u8		_reserved6[420];
	u16		magic;
} __attribute__((packed));

// NOTE: fields are aligned
struct FATNode {
	char	name[8];
	char	extension[3];
	u8		attr;
	u8		_reserved;
	u8		creation_time_cs;
	u16		creation_time;
	u16		creation_date;
	u16		last_access_date;
	u16		first_cluster_hi;
	u16		last_write_time;
	u16		last_write_date;
	u16		first_cluster_lo;
	u32		file_size;
} __attribute__((packed));

struct FATLongName {
	u8		ord;
	u16		name1[5];
	u8		attr;
	u8		_reserved0;
	u8		checksum;
	u16		name2[6];
	u16		_reserved1;
	u16		name3[2];
} __attribute__((packed));

_Static_assert(sizeof(struct FATNode) == sizeof(struct FATLongName), "struct FATNode and struct FATLongName must have same size.");

union FATEntry {
	struct FATNode		node;
	struct FATLongName	long_name;
	u8					first_byte;
} __attribute__((packed));

static u32* _fat;

#if 0
static u32 read32_unaligned(const u32* data) {
	const u8* bytes = (u8*)data;
	return (u32)bytes[0] | ((u32)bytes[1] << 8) | ((u32)bytes[2] << 16) | ((u32)bytes[3] << 24);
}
#endif

static u16 read16_unaligned(const u16* data) {
	const u8* bytes = (u8*)data;
	return (u16)bytes[0] | ((u16)bytes[1] << 8);
}

static bool _fat_read_sectors(void* dst, u32 start_sector, u16 sector_count) {
	if (sector_count == 0) {
		return true;
	}
	const u32 start_index = _partition.start_sector + start_sector;
	const u32 end_index = start_index + (u32)sector_count;
	if (end_index > _partition.start_sector + _partition.sector_count) {
		PRINT_ERROR("Requested sectors are not within the mounted partition.");
		printf("start_sector=%, sector_count=%\n", (u64)start_sector, (u64)sector_count);
		return false;;
	}
	const u32 tries = 10;
	for (u32 i = 0; i < tries; ++i) {
		if (sd_read_sectors(dst, start_index, sector_count)) {
			return true;
		}
	}
	PRINT_ERROR("Failed.");
	return false;
}

void fat_mount_partition(u32 start_sector, u32 sector_count) {
	_partition.start_sector = start_sector;
	_partition.sector_count = sector_count;
	struct BPB bpb;
	memset(&bpb, 0, sizeof bpb);
	_fat_read_sectors(&bpb, 0, 1);
	_partition.sector_count = bpb.total_sector_count;
	print("================ BPB ==================\n");
	printf("bytes_per_sector: %\n", (u64)read16_unaligned((void*)&bpb.bytes_per_sector));
	printf("sectors_per_cluster: %\n", (u64)bpb.sectors_per_cluster);
	printf("total_sector_count: %\n", (u64)bpb.total_sector_count);
	printf("reserved_sector_count: %\n", (u64)bpb.reserved_sector_count);
	printf("fat_count: %\n", (u64)bpb.fat_count);
	printf("root_cluster: %\n", (u64)bpb.root_cluster);
	print("fs type: '");
	for (usize i = 0; i < 8; ++i) {
		putchar(bpb.fs_type[i]);
	}
	print("'\n");
	print("oem_name: '");
	for (usize i = 0; i < 8; ++i) {
		putchar(bpb.oem_name[i]);
	}
	print("'\n");
	printf("MAGIC: %\n", bpb.magic);
	print("=======================================\n");
	_info.root_cluster = bpb.root_cluster;
	_info.sector_size = (usize)read16_unaligned((void*)&bpb.bytes_per_sector);
	_info.sectors_per_cluster = (usize)bpb.sectors_per_cluster;

	const u32 fat_start_sector = (u32)bpb.reserved_sector_count;
	_fat = kmalloc(bpb.sectors_per_fat * _info.sector_size);
	_fat_read_sectors(_fat, fat_start_sector, bpb.sectors_per_fat);

	_info.data_start_sector = fat_start_sector + (u32)bpb.fat_count * bpb.sectors_per_fat;
}

static u32 _fat_cluster_to_sector(u32 cluster) {
	return _info.data_start_sector + (cluster - 2) * _info.sectors_per_cluster;
}

static usize _fat_long_name_portion(char dst[13], const struct FATLongName* long_name) {
	usize length = 0;
	for (usize i = 0; i < 5; ++i) {
		if (read16_unaligned(&long_name->name1[i]) == 0) {
			return length;
		}
		dst[length++] = (char)read16_unaligned(&long_name->name1[i]);
	}
	for (usize i = 0; i < 6; ++i) {
		if (long_name->name2[i] == 0) {
			return length;
		}
		dst[length++] = (char)long_name->name2[i];
	}
	for (usize i = 0; i < 2; ++i) {
		if (long_name->name3[i] == 0) {
			return length;
		}
		dst[length++] = (char)long_name->name3[i];
	}
	return length;
}

// TODO: error handling
static u32 _fat_next_cluster(u32 cluster) {
	if (_fat[cluster] == FAT_CLUSTER_EOF || _fat[cluster] == FAT_CLUSTER_EOF) {
		printf("_fat_next_cluster(%) (result=%)\n", cluster, _fat[cluster]);
	}
	return _fat[cluster];
}

// Get next entry, reading the next sector into buffer if necessary
// NOTE: size of buffer should be 1 sector
static const union FATEntry* _fat_next_entry(union FATEntry* buffer, u32* cluster, u32* sector, u32* entry) {
	if (*sector == 0) {
		if (!_fat_read_sectors(buffer, _fat_cluster_to_sector(*cluster) + *sector, 1)) {
			return NULL;
		}
	}
	const union FATEntry* fat_entry = &buffer[*entry];
	++*entry;
	if (*entry < _info.sector_size / sizeof(union FATEntry)) {
		return fat_entry;
	}
	*entry = 0;
	++*sector;
	if (*sector < _info.sectors_per_cluster) {
		return fat_entry;
	}
	*sector = 0;
	*cluster = _fat_next_cluster(*cluster);
	return fat_entry;
}

static u32 _fat_node_first_cluster(const struct FATNode* node) {
	return ((u32)node->first_cluster_hi << 16) | (u32)node->first_cluster_lo;
}

// Returns the node id (sector where corresponding FATNode is located), or 0 if there are no more directory entries
static u32 _fat_read_name(char dst[MAX_FILENAME + 1], union FATEntry* buffer, u32* cluster, u32* sector, u32* entry) {
	usize name_it;
	u32 node_cluster;
	u32 node_sector;
	u32 node_entry;
	bool long_name = false;
	const union FATEntry* fat_entry;
	while (true) {
		node_cluster = *cluster;
		node_sector = *sector;
		node_entry = *entry;
		fat_entry = _fat_next_entry(buffer, cluster, sector, entry);
		if (fat_entry == NULL) {
			PRINT_ERROR("_fat_next_entry() returned NULL.");
			return NODE_ID_NONE;
		}
		if (fat_entry->first_byte == DIRECTORY_ENTRY_UNUSED) {
#if 0
			PRINT_ERROR("Skipping unused entry...");
#endif
			continue;
		}
#if 0
		printf("ATTR=%\n", (u64)fat_entry->node.attr);
#endif
		if (!(fat_entry->node.attr & NODE_ATTR_LONG_NAME)) {
			break;
		}
		long_name = true;
		char name_portion[13];
		const usize name_portion_length = _fat_long_name_portion(name_portion, &fat_entry->long_name);
		const u8 portion = (fat_entry->long_name.ord & ~LAST_LONG_NAME_ENTRY) - 1;
		if (fat_entry->long_name.ord & LAST_LONG_NAME_ENTRY) {
			const usize length = 13 * (usize)portion + name_portion_length;
			if (length >= MAX_FILENAME) {
				PRINT_ERROR("Name too long!");
				// TODO: return an error
				return NODE_ID_NONE;
			}
			dst[length] = '\0';
		}
		char* where = dst + 13 * portion;
		memcpy(where, name_portion, name_portion_length);
	}
	if (fat_entry->first_byte == DIRECTORY_ENTRY_END) {
#if 0
		PRINT_ERROR("Reached end of directory.");
#endif
		return NODE_ID_NONE;
	}
	// This is mainly for . and .. entries
	if (!long_name) {
		usize length = 0;
		for (usize i = 0; i < 8; ++i) {
			if (fat_entry->node.name[i] == ' ') {
				break;
			}
			dst[length++] = fat_entry->node.name[i];
		}
		if (fat_entry->node.extension[0] != ' ') {
			dst[length++] = '.';
		}
		for (usize i = 0; i < 3; ++i) {
			if (fat_entry->node.extension[i] == ' ') {
				break;
			}
			dst[length++] = fat_entry->node.extension[i];
		}
		dst[length] = '\0';
	}
	return (_fat_cluster_to_sector(node_cluster) + node_sector) * 16 + node_entry;
}

static bool _fat_read_file(void* dst, u32 start_cluster, u32 sector_count) {
	if (sector_count == 0) {
		PRINT_ERROR("sector_count == 0");
		return true;
	}
	const usize cluster_count = (sector_count + _info.sectors_per_cluster - 1) / _info.sectors_per_cluster;
	u32 cluster = start_cluster;
	for (usize i = 0; i < cluster_count - 1; ++i) {
		if (!_fat_read_sectors(dst, _fat_cluster_to_sector(cluster), _info.sectors_per_cluster)) {
			return false;
		}
		dst += _info.sectors_per_cluster * _info.sector_size;
		cluster = _fat_next_cluster(cluster);
	}
	const u32 sectors_remaining = _info.sectors_per_cluster == 1 ? 1 : sector_count % _info.sectors_per_cluster;
	return _fat_read_sectors(dst, _fat_cluster_to_sector(cluster), sectors_remaining);
}

// NOTE: node_id corresponds to the node address
bool fat_create_fs_node(struct FSNode* node, u32 node_id) {
	u32 first_cluster;
	union FATEntry* buf = kmalloc(_info.sector_size);
	if (__builtin_expect(node_id == NODE_ID_ROOT, false)) {
		node->id = NODE_ID_ROOT;
		node->type = FS_NODE_TYPE_DIR;
		node->dir.entry_count = 2;
		node->dir.entries[0].id = NODE_ID_ROOT;
		strcpy(node->dir.entries[0].name, ".");
		node->dir.entries[1].id = NODE_ID_ROOT;
		strcpy(node->dir.entries[1].name, "..");
		first_cluster = _info.root_cluster;
	}
	else {
		if (!_fat_read_sectors(buf, node_id / 16, 1)) {
			return false;
		}
		const struct FATNode* fat_node = &buf[node_id % 16].node;
		first_cluster = _fat_node_first_cluster(fat_node);
		memset(node, 0, sizeof *node);
		node->id = node_id;
		node->type = (fat_node->attr & NODE_ATTR_DIRECTORY) ? FS_NODE_TYPE_DIR : FS_NODE_TYPE_REG;
	}
	if (node->type == FS_NODE_TYPE_REG) {
		const struct FATNode* fat_node = &buf[node_id % 16].node;
		node->file.size = (usize)fat_node->file_size;
		const usize file_sector_count = (node->file.size + _info.sector_size - 1) / _info.sector_size;
		node->file.data = kmalloc(file_sector_count * _info.sector_size);
		memset(node->file.data, 0, node->file.size);
		_fat_read_file(node->file.data, first_cluster, file_sector_count);
		kfree(buf);
		return true;
	}
	u32 cluster = first_cluster;
	u32 sector = 0;
	u32 entry = 0;
#if 0
		PRINT_ERROR("Reading directory entries from disk...");
#endif
	while (true) {
		const u32 id = _fat_read_name(node->dir.entries[node->dir.entry_count].name,
				buf, &cluster, &sector, &entry);
		if (id == NODE_ID_NONE) {
			break;
		}
#if 0
		printf("DIR ENTRY (id=%): '", (u64)id);
		print(node->dir.entries[node->dir.entry_count].name);
		print("'\n");
#endif
		node->dir.entries[node->dir.entry_count].id = id;
		++node->dir.entry_count;
	}
	kfree(buf);
	return true;
}
