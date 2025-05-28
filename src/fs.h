#ifndef _FS_H
#define _FS_H

#include "types.h"
#include <sys/stat.h>

#define FILENAME_MAX	127

struct FSNode;

typedef isize (* FSReadFunc)(struct FSNode* node, void* buf, usize size);
typedef isize (* FSWriteFunc)(struct FSNode* node, const void* buf, usize size);

// NOTE: we could use a union
// TODO: order fields to minimize padding
// TODO: the struct stat data should maybe only be recorded inside file descriptions (per-process)
struct FSNode {
	char			name[FILENAME_MAX + 1];
	struct stat		stat;
	// Parent directory
	struct FSNode*	parent;
	// Pointer to next sibling node
	struct FSNode*	next;
	// Head of a linked list of children
	struct FSNode*	children;

	FSReadFunc		read_func;
	FSWriteFunc		write_func;

	off_t			offset;
	void*			data;
};

// NOTE: We must call fs_init() before doing any file operations
void fs_init(void);
void fs_add(const char* path, mode_t mode);
void fs_remove(struct FSNode* node);
isize fs_read(struct FSNode* node, void* buf, usize size);
isize fs_write(struct FSNode* node, const void* buf, usize size);

struct FSNode* fs_find(const char* path);
void fs_canonicalize(const char* path, char* canonical_path);

#endif //_FS_H
