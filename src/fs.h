#ifndef _FS_H
#define _FS_H

#include "core.h"
#include "myos_time.h"

#define MAX_FILENAME	31

#define NODE_ID_NONE		0
#define NODE_ID_ROOT		1

enum {
	FS_NODE_TYPE_REG,
	FS_NODE_TYPE_DIR,
	FS_NODE_TYPE_CHR,
	FS_NODE_TYPE_BLK,
};
typedef u8	FSNodeType;

struct FSNode;

typedef isize (* FSReadFunc)(struct FSNode* node, void* buf, usize size, isize* offset);
typedef isize (* FSWriteFunc)(struct FSNode* node, const void* buf, usize size, isize* offset);

#define MAX_DIR_ENTRIES	64

struct DirEntry {
	u32		id;
	char	name[MAX_FILENAME + 1];
};

struct FSNode {
	union {
		struct {
			FSReadFunc	read_func;
			FSWriteFunc	write_func;
			void*		data;
			usize		size;
		} file;
		struct {
			struct DirEntry	entries[MAX_DIR_ENTRIES];
			u32				entry_count;
		} dir;
	};
	struct timespec	access_time;
	struct timespec	modify_time;
	struct timespec	status_change_time;
	u32				id;
	u16				ref_count;
	FSNodeType		type;
	bool			should_sync;
};

// NOTE: We must call fs_init() before doing any file operations
void fs_init(void);
isize fs_read(struct FSNode* node, void* buf, usize size, isize* offset);
isize fs_write(struct FSNode* node, const void* buf, usize size, isize* offset);

i32 fs_find(u32* dst, const char* path);
i32 fs_find_from(u32* dst, const char* path, u32 start_id);

struct FSNode* fs_open(u32 id);
void fs_close(struct FSNode* node);
i32 fs_rename(u32 old_parent_id, const char* old_name, u32 new_parent_id, const char* new_name);
i32 fs_unlink(u32 parent_id, const char* name);
i32 fs_mkdir(u32 parent_id, const char* name);
i32 fs_rmdir(u32 node_id);
struct FSNode* fs_creat(u32 parent_id, const char* name);

i32 fs_canonicalize(char* dst, const char* path);

#endif //_FS_H
