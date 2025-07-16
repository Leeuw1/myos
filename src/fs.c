#include "fs.h"
#include "heap.h"
#include "io.h"
#include "fat.h"
#include <string.h>
#include <errno.h>

// TODO: Once syncing is implemented, we can probably decrease MAX_OPEN_NODES
#define MAX_OPEN_NODES		128

static struct FSNode _open_nodes[MAX_OPEN_NODES];
static usize _open_node_count;

extern u8 _shell_binary;

static isize _fs_read_reg(struct FSNode* node, void* buf, usize size, isize* offset);
static isize _fs_write_reg(struct FSNode* node, const void* buf, usize size, isize* offset);
static isize _fs_read_null(struct FSNode* node, void* buf, usize size, isize* offset);
static isize _fs_write_null(struct FSNode* node, const void* buf, usize size, isize* offset);
static isize _fs_read_tty(struct FSNode* node, void* buf, usize size, isize* offset);
static isize _fs_write_tty(struct FSNode* node, const void* buf, usize size, isize* offset);
static isize _fs_read_console(struct FSNode* node, void* buf, usize size, isize* offset);
static isize _fs_write_console(struct FSNode* node, const void* buf, usize size, isize* offset);

static u32 _fs_new_id(void) {
	static u32 id = 0xff000000;
	return id++;
}

static i32 _fs_file_create(struct FSNode** dst, struct FSNode* parent, const char* name, FSNodeType type, FSReadFunc read_func, FSWriteFunc write_func) {
	if (strlen(name) > MAX_FILENAME) {
		PRINT_ERROR("File name too long.");
		return ENAMETOOLONG;
	}
	if (_open_node_count == MAX_OPEN_NODES) {
		PRINT_ERROR("Max nodes open.");
		return ENFILE;
	}
	struct FSNode* node = NULL;
	for (usize i = 0; i < MAX_OPEN_NODES; ++i) {
		if (_open_nodes[i].id == NODE_ID_NONE) {
			node = &_open_nodes[i];
			break;
		}
	}
	memset(node, 0, sizeof *node);
	node->id = _fs_new_id();
	node->type = type;
	node->file.read_func = read_func;
	node->file.write_func = write_func;
	const struct timespec now = time_current();
	node->access_time = now;
	node->modify_time = now;
	node->status_change_time = now;
	node->should_sync = true;
	node->ref_count = 1;
	if (parent->dir.entry_count == MAX_DIR_ENTRIES) {
		PRINT_ERROR("Parent directory is full.");
		print("name='");
		print(name);
		print("'\n");
		return ENOMEM;
	}
	parent->dir.entries[parent->dir.entry_count].id = node->id;
	strcpy(parent->dir.entries[parent->dir.entry_count].name, name);
	++parent->dir.entry_count;
	parent->modify_time = time_current();
	parent->status_change_time = parent->modify_time;
	parent->should_sync = true;
	*dst = node;
	return 0;
}

static i32 _fs_dir_create(struct FSNode** dst, struct FSNode* parent, const char* name) {
	if (strlen(name) > MAX_FILENAME) {
		PRINT_ERROR("Directory name too long.");
		return ENAMETOOLONG;
	}
	if (_open_node_count == MAX_OPEN_NODES) {
		PRINT_ERROR("Max nodes open.");
		return ENFILE;
	}
	struct FSNode* node = NULL;
	for (usize i = 0; i < MAX_OPEN_NODES; ++i) {
		if (_open_nodes[i].id == NODE_ID_NONE) {
			node = &_open_nodes[i];
			break;
		}
	}
	memset(node, 0, sizeof *node);
	node->id = _fs_new_id();
	node->type = FS_NODE_TYPE_DIR;
	const struct timespec now = time_current();
	node->access_time = now;
	node->modify_time = now;
	node->status_change_time = now;
	node->should_sync = true;
	node->ref_count = 1;
	node->dir.entry_count = 2;
	strcpy(node->dir.entries[0].name, ".");
	node->dir.entries[0].id = node->id;
	strcpy(node->dir.entries[1].name, "..");
	if (__builtin_expect(parent == NULL, false)) {
		node->dir.entries[1].id = node->id;
		*dst = node;
		return 0;
	}
	node->dir.entries[1].id = parent->id;
	if (parent->dir.entry_count == MAX_DIR_ENTRIES) {
		PRINT_ERROR("Parent directory is full.");
		return ENOMEM;
	}
	parent->dir.entries[parent->dir.entry_count].id = node->id;
	strcpy(parent->dir.entries[parent->dir.entry_count].name, name);
	++parent->dir.entry_count;
	parent->modify_time = time_current();
	parent->status_change_time = parent->modify_time;
	parent->should_sync = true;
	*dst = node;
	return 0;
}

static void _fs_init_node(struct FSNode* node, u32 id) {
	fat_create_fs_node(node, id);
	node->ref_count = 1;
	if (node->type != FS_NODE_TYPE_REG) {
		return;
	}
	node->file.read_func = _fs_read_reg;
	node->file.write_func = _fs_write_reg;
}

struct FSNode* fs_open(u32 id) {
	if (id == NODE_ID_NONE) {
		PRINT_ERROR("id == NODE_ID_NONE");
		return NULL;
	}
	if (_open_node_count == MAX_OPEN_NODES) {
		PRINT_ERROR("Max nodes open.");
		return NULL;
	}
	usize dst_index = 0;
	for (usize i = 0; i < MAX_OPEN_NODES; ++i) {
		if (_open_nodes[i].id == id) {
			++_open_nodes[i].ref_count;
			return &_open_nodes[i];
		}
		if (_open_nodes[i].id == NODE_ID_NONE) {
			dst_index = i;
		}
	}
	struct FSNode* node = &_open_nodes[dst_index];
	_fs_init_node(node, id);
	++_open_node_count;
	return node;
}

void fs_close(struct FSNode* node) {
	if (node == NULL) {
		PRINT_ERROR("node == NULL");
		return;
	}
	if (node->ref_count == 0) {
		PRINT_ERROR("node->ref_count == 0");
		return;
	}
	--node->ref_count;
	if (node->ref_count != 0) {
		return;
	}
	if (node->should_sync) {
		// TODO: sync
	}
	// NOTE: before syncing is implemented, we should prevent modified nodes from being closed
	if (node->id >= 0xff000000 && node->should_sync) {
		node->ref_count = 1;
		return;
	}
	node->id = NODE_ID_NONE;
	if (node->type == FS_NODE_TYPE_REG) {
		kfree(node->file.data);
	}
}

void fs_init(void) {
	_open_node_count = 0;
	memset(_open_nodes, 0, sizeof(_open_nodes));
	struct FSNode* root = fs_open(NODE_ID_ROOT);
	struct FSNode* dev;
	i32 b = _fs_dir_create(&dev, root, "dev");
	struct FSNode* dst;
	b |= _fs_file_create(&dst, dev, "null", FS_NODE_TYPE_CHR, _fs_read_null, _fs_write_null);
	b |= _fs_file_create(&dst, dev, "tty", FS_NODE_TYPE_CHR, _fs_read_tty, _fs_write_tty);
	b |= _fs_file_create(&dst, dev, "console", FS_NODE_TYPE_CHR, _fs_read_console, _fs_write_console);
	u32 bin_id;
	const i32 result = fs_find(&bin_id, "/bin");
	struct FSNode* bin;
	if (result == 0) {
		bin = fs_open(bin_id);
	}
	else {
		b |= _fs_dir_create(&bin, root, "bin");
	}
	struct FSNode* sh;
	b |= _fs_file_create(&sh, bin, "sh", FS_NODE_TYPE_REG, NULL, NULL);
	sh->file.data = (void*)&_shell_binary;
	if (b != 0) {
		PRINT_ERROR("Failed.");
	}
}

static i32 _fs_find_child_index(usize* dst, struct FSNode* node, const char* name, usize length) {
	if (node->type != FS_NODE_TYPE_DIR) {
		return ENOTDIR;
	}
	for (u32 i = 0; i < node->dir.entry_count; ++i) {
		if (strlen(node->dir.entries[i].name) != length) {
			continue;
		}
		if (memcmp(node->dir.entries[i].name, name, length) == 0) {
			*dst = i;
			return 0;
		}
	}
	return ENOENT;
}

static i32 _fs_find_child(u32* dst, u32 id, const char* name, usize length) {
	struct FSNode* node = fs_open(id);
	usize i;
	const i32 result = _fs_find_child_index(&i, node, name, length);
	if (result == 0) {
		*dst = node->dir.entries[i].id;
	}
	fs_close(node);
	return result;
}

i32 fs_find_from(u32* dst, const char* path, u32 start_id) {
	*dst = NODE_ID_NONE;
	u32 id = start_id;
	const char* name_start = path;
	if (*name_start == '/') {
		id = NODE_ID_ROOT;
		while (*name_start == '/') {
			++name_start;
		}
	}
	const char* path_end = name_start + strlen(name_start);
	const char* name_end;
	while ((name_end = memchr(name_start, '/', path_end - name_start)) != NULL) {
		const i32 result = _fs_find_child(&id, id, name_start, name_end - name_start);
		if (result != 0) {
			return result;
		}
		// Skip the '/' (could be more than one)
		while (*name_end == '/') {
			++name_end;
		}
		name_start = name_end;
	}
	if (name_start == path_end) {
		*dst = id;
		return 0;
	}
	return _fs_find_child(dst, id, name_start, path_end - name_start);
}

#if 0
struct FSNode* fs_find_from(const char* path, struct FSNode* start) {
	if (path == NULL || start == NULL) {
		return NULL;
	}
	struct FSNode* node = start;
	const char* name_start = path;
	if (*name_start == '/') {
		node = root;
		while (*name_start == '/') {
			++name_start;
		}
	}
	const char* path_end = name_start + strlen(name_start);
	const char* name_end;
	while ((name_end = memchr(name_start, '/', path_end - name_start)) != NULL) {
		node = _fs_next(node, name_start, name_end - name_start);
		if (node == NULL) {
			return NULL;
		}
		// Skip the '/' (could be more than one)
		while (*name_end == '/') {
			++name_end;
		}
		name_start = name_end;
	}
	if (name_start == path_end) {
		return node;
	}
	return _fs_next(node, name_start, path_end - name_start);
}
#endif

i32 fs_find(u32* dst, const char* path) {
	return fs_find_from(dst, path, NODE_ID_ROOT);
}

i32 fs_rename(u32 old_parent_id, const char* old_name, u32 new_parent_id, const char* new_name) {
	if (strlen(new_name) > MAX_FILENAME) {
		return ENAMETOOLONG;
	}
	struct FSNode* old_parent = fs_open(old_parent_id);
	struct FSNode* new_parent = fs_open(new_parent_id);
	i32 result = 0;
	if (new_parent->dir.entry_count == MAX_DIR_ENTRIES) {
		result = ENOMEM;
		goto cleanup;
	}
	usize index;
	result = _fs_find_child_index(&index, old_parent, old_name, strlen(old_name));
	if (result != 0) {
		goto cleanup;
	}
	const u32 node_id = old_parent->dir.entries[index].id;
	--old_parent->dir.entry_count;
	for (usize i = (usize)index; i < old_parent->dir.entry_count; ++i) {
		old_parent->dir.entries[i].id = old_parent->dir.entries[i + 1].id;
		strcpy(old_parent->dir.entries[i].name, old_parent->dir.entries[i + 1].name);
	}
	new_parent->dir.entries[new_parent->dir.entry_count].id = node_id;
	strcpy(new_parent->dir.entries[new_parent->dir.entry_count].name, new_name);
	++new_parent->dir.entry_count;
	old_parent->modify_time = time_current();
	old_parent->status_change_time = old_parent->modify_time;
	old_parent->should_sync = true;
	new_parent->modify_time = old_parent->modify_time;
	new_parent->status_change_time = new_parent->modify_time;
	new_parent->should_sync = true;
cleanup:
	fs_close(old_parent);
	fs_close(new_parent);
	return 0;
}

// Since FAT32 does not support links, unlink just deletes a file
i32 fs_unlink(u32 parent_id, const char* name) {
	struct FSNode* parent = fs_open(parent_id);
	usize index;
	const i32 result = _fs_find_child_index(&index, parent, name, strlen(name));
	if (result != 0) {
		fs_close(parent);
		return result;
	}
	struct FSNode* node = fs_open(parent->dir.entries[index].id);
	// TODO: we need to mark node for deletion
	// fs_destroy(node) -> this will kfree(node->file.data) for files, and mark node to be deleted once it is closed
	--parent->dir.entry_count;
	for (usize i = (usize)index; i < parent->dir.entry_count; ++i) {
		parent->dir.entries[i].id = parent->dir.entries[i + 1].id;
		strcpy(parent->dir.entries[i].name, parent->dir.entries[i + 1].name);
	}
	parent->modify_time = time_current();
	parent->status_change_time = parent->modify_time;
	parent->should_sync = true;
	fs_close(parent);
	fs_close(node);
	return 0;
}

i32 fs_mkdir(u32 parent_id, const char* name) {
	struct FSNode* parent = fs_open(parent_id);
	struct FSNode* node;
	const i32 result = _fs_dir_create(&node, parent, name);
	if (result == 0) {
		parent->modify_time = time_current();
		parent->status_change_time = parent->modify_time;
		parent->should_sync = true;
	}
	fs_close(parent);
	return result;
}

static i32 _fs_find_child_index2(usize* dst, const struct FSNode* node, const struct FSNode* child) {
	if (node->type != FS_NODE_TYPE_DIR) {
		PRINT_ERROR("Node is not a directory.");
		return ENOTDIR;
	}
	for (u32 i = 2; i < node->dir.entry_count; ++i) {
		if (node->dir.entries[i].id == child->id) {
			*dst = i;
			return 0;
		}
	}
	return ENOENT;
}

i32 fs_rmdir(u32 node_id) {
	struct FSNode* node = fs_open(node_id);
	struct FSNode* parent = fs_open(node->dir.entries[1].id);
	i32 result;
	if (node->type != FS_NODE_TYPE_DIR) {
		result = ENOTDIR;
		goto cleanup;
	}
	if (node->dir.entry_count != 2) {
		result = ENOTEMPTY;
		goto cleanup;
	}
	usize index;
	result = _fs_find_child_index2(&index, parent, node);
	if (result != 0) {
		goto cleanup;
	}
	--parent->dir.entry_count;
	for (usize i = index; i < parent->dir.entry_count; ++i) {
		parent->dir.entries[i] = parent->dir.entries[i + 1];
	}
	parent->modify_time = time_current();
	parent->status_change_time = parent->modify_time;
	parent->should_sync = true;
cleanup:
	fs_close(node);
	fs_close(parent);
	return result;
}

struct FSNode* fs_creat(u32 parent_id, const char* name) {
	struct FSNode* parent = fs_open(parent_id);
	if (parent == NULL) {
		return NULL;
	}
	struct FSNode* node;
	const i32 result = _fs_file_create(&node, parent, name, FS_NODE_TYPE_REG, _fs_read_reg, _fs_write_reg);
	fs_close(parent);
	return result == 0 ? node : NULL;
}

static void _fs_update_stack(char stack[8][MAX_FILENAME], usize* stack_count, const char* name, usize length) {
	if (length == 1 && name[0] == '.') {
		return;
	}
	if (length == 2 && memcmp(name, "..", 2) == 0) {
		if (*stack_count != 0) {
			--*stack_count;
		}
	}
	else {
		memcpy(stack[*stack_count], name, length);
		stack[*stack_count][length] = '\0';
#if 0
		print("NAME: '");
		print(stack[*stack_count]);
		print("'\n");
#endif
		++*stack_count;
	}
}

i32 fs_canonicalize(char* dst, const char* path) {
#if 0
	print("fs_canonicalize() path='");
	print(path);
	print("'\n");
#endif
	char stack[8][MAX_FILENAME];
	usize stack_count = 0;
	u32 id = NODE_ID_ROOT;
	const char* name_start = path;
	while (*name_start == '/') {
		++name_start;
	}
	const char* path_end = name_start + strlen(name_start);
	const char* name_end;
	while ((name_end = memchr(name_start, '/', path_end - name_start)) != NULL) {
		const i32 result = _fs_find_child(&id, id, name_start, name_end - name_start);
		if (result != 0) {
#if 0
			PRINT_ERROR("Bad path.");
			print("path was: '");
			print(path);
			print("'\n");
#endif
			return result;
		}
		_fs_update_stack(stack, &stack_count, name_start, name_end - name_start);
		// Skip the '/' (could be more than one)
		while (*name_end == '/') {
			++name_end;
		}
		name_start = name_end;
	}
	if (name_start != path_end) {
		const i32 result = _fs_find_child(&id, id, name_start, path_end - name_start);
		if (result != 0) {
#if 0
			PRINT_ERROR("Bad path.");
			print("path was: '");
			print(path);
			print("'\n");
#endif
			return result;
		}
		_fs_update_stack(stack, &stack_count, name_start, path_end - name_start);
	}

	//if (name_start == path_end) {
	//	return node;
	//}
	//return _fs_find_child(node, name_start, path_end - name_start);

	if (stack_count == 0) {
		dst[0] = '/';
		dst[1] = '\0';
		return 0;
	}
	dst[0] = '\0';
	for (usize i = 0; i < stack_count; ++i) {
		strcat(dst, "/");
		strcat(dst, stack[i]);
	}
	return 0;
}

#if 0
void fs_canonicalize(char* dst, const struct FSNode* node) {
	const usize max_name_count = 8;
	const char* names[max_name_count];
	usize name_count = 0;
	usize length = 0;
	for (const struct FSNode* it = node; it != root; it = it->parent) {
		names[name_count++] = it->name;
		length += strlen(it->name) + 1;
	}
	if (length == 0) {
		dst[0] = '/';
		dst[1] = '\0';
		return;
	}
	dst[0] = '\0';
	for (usize i = 0; i < name_count; ++i) {
		strcat(dst, "/");
		strcat(dst, names[name_count - 1 - i]);
	}
}
#endif

#if 0
// Write filename to dst
static void _fs_extract_name(const char* path, const char** start, const char** end) {
	*end = path + strlen(path) - 1;
	// Ignore trailing slashes
	while (**end == '/') {
		--*end;
	}
	*start = *end;
	++*end;
	while (**start != '/') {
		--*start;
	}
	++*start;
}
#endif

// TODO: might want to check whether the node is a directory, in which case read_func and write_func are NULL
isize fs_read(struct FSNode* node, void* buf, usize size, isize* offset) {
	if (node == NULL) {
		return -1;
	}
	if (node->file.read_func == NULL) {
		PRINT_ERROR("read_func == NULL");
		return -1;
	}
	return node->file.read_func(node, buf, size, offset);
}

isize fs_write(struct FSNode* node, const void* buf, usize size, isize* offset) {
	if (node == NULL) {
		return -1;
	}
	if (node->file.write_func == NULL) {
		PRINT_ERROR("write_func == NULL");
		return -1;
	}
	const isize result = node->file.write_func(node, buf, size, offset);
	if (result > 0) {
		node->modify_time = time_current();
		node->status_change_time = node->modify_time;
		node->should_sync = true;
	}
	return result;
}

static void _fs_ensure_min_size(struct FSNode* node, usize size) {
	if ((usize)node->file.size >= size) {
		return;
	}
	node->file.data = krealloc(node->file.data, size);
	node->file.size = size;
}

static isize _fs_read_reg(struct FSNode* node, void* buf, usize size, isize* offset) {
	if (*offset > (isize)node->file.size) {
		return 0;
	}
	const usize max_size = node->file.size - *offset;
	const usize s = size > max_size ? max_size : size;
	memcpy(buf, node->file.data + *offset, s);
	*offset += s;
	return (isize)s;
}

static isize _fs_write_reg(struct FSNode* node, const void* buf, usize size, isize* offset) {
	_fs_ensure_min_size(node, *offset + size);
	memcpy(node->file.data + *offset, buf, size);
	*offset += size;
	return (isize)size;
}

static isize _fs_read_null(struct FSNode* node, void* buf, usize size, isize* offset) {
	(void)node; (void)buf; (void)size; (void)offset;
	return 0;
}

static isize _fs_write_null(struct FSNode* node, const void* buf, usize size, isize* offset) {
	(void)node; (void)buf; (void)size; (void)offset;
	return 0;
}

static isize _fs_read_tty(struct FSNode* node, void* buf, usize size, isize* offset) {
	(void)node; (void)offset;
#if 0
	if (tty_canonical_mode()) {
		return -2;
	}
	char* b = buf;
	usize i;
	for (i = 0; i < size; ++i) {
		const char c = tty_getchar();
		if (c == '\0') {
			break;
		}
		b[i] = c;
	}
	return (isize)i;
#endif
	return -2;
}

static isize _fs_write_tty(struct FSNode* node, const void* buf, usize size, isize* offset) {
	(void)node; (void)offset;
	const char* b = buf;
	for (usize i = 0; i < size; ++i) {
		putchar(b[i]);
	}
	return (isize)size;
}

static isize _fs_read_console(struct FSNode* node, void* buf, usize size, isize* offset) {
	return _fs_read_null(node, buf, size, offset);
}

static isize _fs_write_console(struct FSNode* node, const void* buf, usize size, isize* offset) {
	return _fs_write_null(node, buf, size, offset);
}
