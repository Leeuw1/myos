#ifndef _FS_H
#define _FS_H

#include "core.h"
#include <sys/stat.h>

#define FILENAME_MAX	127

enum {
	FS_NODE_TYPE_REG,
	FS_NODE_TYPE_DIR,
	FS_NODE_TYPE_CHR,
};
typedef u8	FSNodeType;

struct FSNode;

typedef isize (* FSReadFunc)(struct FSNode* node, void* buf, usize size);
typedef isize (* FSWriteFunc)(struct FSNode* node, const void* buf, usize size);

struct FSNode {
	char			name[FILENAME_MAX + 1];
	// Parent directory
	struct FSNode*	parent;
	// Pointer to next sibling node
	struct FSNode*	next;
	// Head of a linked list of children
	struct FSNode*	children;

	FSReadFunc		read_func;
	FSWriteFunc		write_func;

	void*			data;
	usize			size;
	isize			offset;
	FSNodeType		type;
};

// NOTE: We must call fs_init() before doing any file operations
void fs_init(void);
void fs_add(const char* path, FSNodeType mode);
void fs_remove(struct FSNode* node);
isize fs_read(struct FSNode* node, void* buf, usize size);
isize fs_write(struct FSNode* node, const void* buf, usize size);

struct FSNode* fs_find(const char* path);
void fs_canonicalize(const char* path, char* canonical_path);

#endif //_FS_H
