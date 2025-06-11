#include "fs.h"
#include "heap.h"
#include "io.h"
#include <string.h>

extern u8 _shell_binary;
extern u8 _count_binary;

static struct FSNode* root;

static isize _fs_read_reg(struct FSNode* node, void* buf, usize size);
static isize _fs_write_reg(struct FSNode* node, const void* buf, usize size);
static isize _fs_read_null(struct FSNode* node, void* buf, usize size);
static isize _fs_write_null(struct FSNode* node, const void* buf, usize size);
static isize _fs_read_tty(struct FSNode* node, void* buf, usize size);
static isize _fs_write_tty(struct FSNode* node, const void* buf, usize size);
static isize _fs_read_console(struct FSNode* node, void* buf, usize size);
static isize _fs_write_console(struct FSNode* node, const void* buf, usize size);

static struct FSNode* _fs_node_create(struct FSNode* parent, const char* name, FSNodeType type, FSReadFunc read_func, FSWriteFunc write_func) {
	struct FSNode* node = kmalloc(sizeof *node);
	memset(node, 0, sizeof *node);
	strncpy(node->name, name, FILENAME_MAX);
	node->type = type;
	node->read_func = read_func;
	node->write_func = write_func;
	if (__builtin_expect(parent == NULL, false)) {
		return node;
	}
	node->next = parent->children;
	parent->children = node;
	node->parent = parent;
	return node;
}

static void _fs_node_remove_child(struct FSNode* child) {
	struct FSNode* node = child->parent;
	if (node->children == NULL) {
		// TODO: indicate that an error has occurred
		return;
	}
	if (node->children == child) {
		node->children = child->next;
		kfree(child);
		return;
	}
	for (struct FSNode* it = node->children; it->next != NULL; it = it->next) {
		if (it->next == child) {
			it->next = child->next;
			kfree(child);
			return;
		}
	}
	// TODO: indicate that an error has occurred
}

void fs_init(void) {
	root = _fs_node_create(NULL, "", FS_NODE_TYPE_DIR, NULL, NULL);
	_fs_node_create(root, "tmp", FS_NODE_TYPE_DIR, NULL, NULL);
	struct FSNode* dev_dir = _fs_node_create(root, "dev", FS_NODE_TYPE_DIR, NULL, NULL);
	_fs_node_create(dev_dir, "null", FS_NODE_TYPE_CHR, _fs_read_null, _fs_write_null);
	_fs_node_create(dev_dir, "tty", FS_NODE_TYPE_CHR, _fs_read_tty, _fs_write_tty);
	_fs_node_create(dev_dir, "console", FS_NODE_TYPE_CHR, _fs_read_console, _fs_write_console);
	struct FSNode* bin_dir = _fs_node_create(root, "bin", FS_NODE_TYPE_DIR, NULL, NULL);
	struct FSNode* sh = _fs_node_create(bin_dir, "sh", FS_NODE_TYPE_REG, NULL, NULL);
	sh->data = (void*)&_shell_binary;
	struct FSNode* count = _fs_node_create(bin_dir, "count", FS_NODE_TYPE_REG, NULL, NULL);
	count->data = (void*)&_count_binary;
}

// NOTE: Operations like chdir(), open(), close(), etc. that are process-specific are not implemented here (see proc_ functions instead)

// TODO: we need to check permissions when performing FS operations

static struct FSNode* _fs_find_child(const struct FSNode* node, const char* name, size_t length) {
	if (node->type != FS_NODE_TYPE_DIR) {
		print("fs_find_child: Node is not a directory.\n");
		return NULL;
	}
	for (const struct FSNode* it = node->children; it != NULL; it = it->next) {
		if (memcmp(it->name, name, length) == 0) {
			return (struct FSNode*)it;
		}
	}
	return NULL;
}

// Used by fs_find
static struct FSNode* _fs_next(struct FSNode* node, const char* name, size_t length) {
	if (strncmp(name, ".", length) == 0) {
		return node;
	}
	else if (strncmp(name, "..", length) == 0) {
		return node->parent != NULL ? node->parent : node;
	}
	return _fs_find_child(node, name, length);
}

// Find node and its parent by parsing path (which must be an absolute path)
// NOTE: Path does not need to be canonicalized
struct FSNode* fs_find(const char* path) {
	if (path == NULL) {
		return NULL;
	}
	const char* name_start = path + 1;
	if (*name_start == '\0') {
		return root;
	}
	struct FSNode* node = root;
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

// NOTE: assuming that path is absolute
void fs_canonicalize(const char* path, char* canonical_path) {
	print("fs_canonicalize(), path='");
	print(path);
	print("'\n");
	struct FSNode* node = fs_find(path);
	if (node == NULL) {
		PRINT_ERROR("path does not refer to a valid node.");
		return;
	}
	const usize max_name_count = 8;
	const char* names[max_name_count];
	usize name_count = 0;
	usize length = 0;
	for (struct FSNode* it = node; it != root; it = it->parent) {
		print("iteration\n");
		names[name_count++] = it->name;
		length += strlen(it->name) + 1;
	}
	if (length == 0) {
		canonical_path[0] = '/';
		canonical_path[1] = '\0';
		return;
	}
	canonical_path[0] = '\0';
	for (usize i = 0; i < name_count; ++i) {
		strcat(canonical_path, "/");
		strcat(canonical_path, names[name_count - 1 - i]);
	}
}

// Write filename to dst
void _fs_extract_name(const char* path, const char** start, const char** end) {
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

// Used by syscalls like: open(), mkdir()
void fs_add(const char* path, FSNodeType type) {
	if (strlen(path) == 1) {
		print("fs_add: Cannot add directory '/'.\n");
		return;
	}
	const char* name_start;
	const char* name_end;
	_fs_extract_name(path, &name_start, &name_end);
	const size_t length = name_start - path;
	if (length > 256) {
		print("fs_add: length > 256.\n");
		return;
	}
	char* parent_path = kmalloc(length + 1);
	memcpy(parent_path, path, length);
	parent_path[length] = '\0';
	struct FSNode* parent = fs_find(parent_path);
	kfree(parent_path);
	if (parent == NULL) {
		print("fs_add: Parent node does not exist.\n");
		return;
	}
	if (parent->type != FS_NODE_TYPE_DIR) {
		print("fs_add: Parent node is not a directory.\n");
		return;
	}
	if (_fs_find_child(parent, name_start, name_end - name_start)) {
		print("fs_add: Child node already exists.\n");
		return;
	}
	FSReadFunc read_func = NULL;
	FSWriteFunc write_func = NULL;
	switch (type) {
	case FS_NODE_TYPE_REG:
		read_func = _fs_read_reg;
		write_func = _fs_write_reg;
		break;
	}
	char name[FILENAME_MAX];
	strncpy(name, name_start, name_end - name_start);
	_fs_node_create(parent, name, type, read_func, write_func);
}

void fs_remove(struct FSNode* node) {
	if (node == NULL) {
		PRINT_ERROR("node == NULL.");
		return;
	}
	if (node == root) {
		// TODO: throw error, we cannot delete root node
		PRINT_ERROR("Cannot delete '/'.");
		return;
	}
	if (node == NULL) {
		PRINT_ERROR("Node does not exist.");
		return;
	}
	if (node->children != NULL) {
		PRINT_ERROR("Node has children.");
		return;
	}
	_fs_node_remove_child(node);
}

// TODO: might want to check whether the node is a directory, in which case read_func and write_func are NULL
isize fs_read(struct FSNode* node, void* buf, usize size) {
	if (node == NULL) {
		return -1;
	}
	return node->read_func(node, buf, size);
}

isize fs_write(struct FSNode* node, const void* buf, usize size) {
	if (node == NULL) {
		return -1;
	}
	return node->write_func(node, buf, size);
}

// Resize node->data if node->stat.st_size < size
static void _fs_ensure_min_size(struct FSNode* node, usize size) {
	if ((usize)node->size >= size) {
		return;
	}
	node->data = krealloc(node->data, size);
	node->size = size;
}

static isize _fs_read_reg(struct FSNode* node, void* buf, usize size) {
	if (node->offset > (isize)node->size) {
		//return EOF;
		return 0;
	}
	usize max_size = node->size - node->offset;
	usize s = size > max_size ? max_size : size;
	memcpy(buf, node->data + node->offset, s);
	node->offset += s;
	return (isize)s;
}

static isize _fs_write_reg(struct FSNode* node, const void* buf, usize size) {
	_fs_ensure_min_size(node, node->offset + size);
	memcpy(node->data + node->offset, buf, size);
	node->offset += size;
	return (isize)size;
}

static isize _fs_read_null(struct FSNode* node, void* buf, usize size) {
	(void)node; (void)buf; (void)size;
	return 0;
}

static isize _fs_write_null(struct FSNode* node, const void* buf, usize size) {
	(void)node; (void)buf; (void)size;
	return 0;
}

// TODO: in the future we could use termios info to modify read/write operations
// 		for example, the CR to NL translation could be recorded in the termios settings
static isize _fs_read_tty(struct FSNode* node, void* buf, usize size) {
	(void)node;
	char* b = buf;
	for (usize i = 0; i < size; ++i) {
		b[i] = tty_getchar();
	}
	return (isize)size;
}

static isize _fs_write_tty(struct FSNode* node, const void* buf, usize size) {
	(void)node;
	const char* b = buf;
	for (usize i = 0; i < size; ++i) {
		putchar(b[i]);
	}
	return (isize)size;
}

static isize _fs_read_console(struct FSNode* node, void* buf, usize size) {
	return _fs_read_tty(node, buf, size);
}

static isize _fs_write_console(struct FSNode* node, const void* buf, usize size) {
	return _fs_write_tty(node, buf, size);
}
