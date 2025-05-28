#include "fs.h"
#include "heap.h"
#include "io.h"
#include <string.h>

#include "shell_generated.h"

static struct FSNode* root;

static isize _fs_read_reg(struct FSNode* node, void* buf, usize size);
static isize _fs_write_reg(struct FSNode* node, const void* buf, usize size);
static isize _fs_read_null(struct FSNode* node, void* buf, usize size);
static isize _fs_write_null(struct FSNode* node, const void* buf, usize size);
static isize _fs_read_tty(struct FSNode* node, void* buf, usize size);
static isize _fs_write_tty(struct FSNode* node, const void* buf, usize size);
static isize _fs_read_console(struct FSNode* node, void* buf, usize size);
static isize _fs_write_console(struct FSNode* node, const void* buf, usize size);

// TODO: initialize stat
// TODO: maybe combine _fs_node_create and _fs_node_add_child into one function
static struct FSNode* _fs_node_create(const char* name, mode_t mode, FSReadFunc read_func, FSWriteFunc write_func) {
	struct FSNode* node = kmalloc(sizeof *node);
	memset(node, 0, sizeof *node);
	strncpy(node->name, name, FILENAME_MAX);
	node->stat.st_mode = mode;
	node->read_func = read_func;
	node->write_func = write_func;
	return node;
}

static void _fs_node_add_child(struct FSNode* node, struct FSNode* child) {
	child->next = node->children;
	node->children = child;
	child->parent = node;
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
	root = _fs_node_create("", S_IFDIR, NULL, NULL);
	_fs_node_add_child(root, _fs_node_create("tmp", S_IFDIR, NULL, NULL));
	struct FSNode* dev_dir = _fs_node_create("dev", S_IFDIR, NULL, NULL);
	_fs_node_add_child(root, dev_dir);
	_fs_node_add_child(dev_dir, _fs_node_create("null", S_IFCHR, _fs_read_null, _fs_write_null));
	_fs_node_add_child(dev_dir, _fs_node_create("tty", S_IFCHR, _fs_read_tty, _fs_write_tty));
	_fs_node_add_child(dev_dir, _fs_node_create("console", S_IFCHR, _fs_read_console, _fs_write_console));
	//struct FSNode* bin_dir = _fs_node_create("bin", S_IFDIR, NULL, NULL);
	//_fs_node_add_child(root, bin_dir);
	struct FSNode* sh = _fs_node_create("sh", S_IFREG, NULL, NULL);
	sh->data = (void*)_shell_binary;
	//_fs_node_add_child(bin_dir, sh);
	_fs_node_add_child(root, sh);
}

// NOTE: Operations like chdir(), open(), close(), etc. that are process-specific are not implemented here (see proc_ functions instead)

// TODO: we need to check permissions when performing FS operations

static struct FSNode* _fs_find_child(const struct FSNode* node, const char* name, size_t length) {
	if ((node->stat.st_mode & S_IFMT) != S_IFDIR) {
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
	struct FSNode* node = fs_find(path);
	const usize max_name_count = 8;
	const char* names[max_name_count];
	usize name_count = 0;
	usize length = 0;
	for (struct FSNode* it = node; it != root; it = it->parent) {
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
void fs_add(const char* path, mode_t mode) {
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
	if ((parent->stat.st_mode & S_IFMT) != S_IFDIR) {
		print("fs_add: Parent node is not a directory.\n");
		return;
	}
	if (_fs_find_child(parent, name_start, name_end - name_start)) {
		print("fs_add: Child node already exists.\n");
		return;
	}
	FSReadFunc read_func = NULL;
	FSWriteFunc write_func = NULL;
	switch (mode & S_IFMT) {
	case S_IFREG:
		read_func = _fs_read_reg;
		write_func = _fs_write_reg;
		break;
	}
	char name[FILENAME_MAX];
	strncpy(name, name_start, name_end - name_start);
	_fs_node_add_child(parent,
			_fs_node_create(name, mode, read_func, write_func));
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
	if ((usize)node->stat.st_size >= size) {
		return;
	}
	node->data = krealloc(node->data, size);
	node->stat.st_size = size;
}

static isize _fs_read_reg(struct FSNode* node, void* buf, usize size) {
	if (node->offset > node->stat.st_size) {
		//return EOF;
		return 0;
	}
	usize max_size = node->stat.st_size - node->offset;
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
