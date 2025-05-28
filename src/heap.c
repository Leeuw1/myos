#include "heap.h"
#include "io.h"
#include <string.h>

extern u8 _HEAP_START_;

#define HEAP_BASE	((void*)&_HEAP_START_)
#define HEAP_SIZE	0x80000

#define FLAGS_IN_USE	0x1
#define FLAGS_END		0x2

struct HeapNode {
	u32	size;
	u8	flags;
	u64	data[];
};

void heap_init(void) {
	struct HeapNode* node = HEAP_BASE;
	node->size = HEAP_SIZE;
	node->flags = FLAGS_END;
}

static void _heap_print(void) {
	struct HeapNode* node = HEAP_BASE;
	while (true) {
		printf("[heap] addr=%, size=%, flags=%\n", (u64)node, (u64)node->size, (u64)node->flags);
		if (node->flags & FLAGS_END) {
			return;
		}
		node = (void*)node + node->size;
	}
}

void* kmalloc(usize size) {
	size = 8 * ((size + sizeof(struct HeapNode) + 7) / 8);
	struct HeapNode* node = HEAP_BASE;
	while (true) {
		if (!(node->flags & FLAGS_IN_USE) && node->size >= size) {
			node->flags |= FLAGS_IN_USE;
			const u32 diff = node->size - size;
			if (diff < sizeof(struct HeapNode) + 8) {
				return node->data;
			}
			node->size = size;
			struct HeapNode* new_node = (void*)node + size;
			new_node->size = diff;
			// Copy the FLAGS_END flag in case it is set
			new_node->flags = node->flags & ~FLAGS_IN_USE;
			node->flags &= ~FLAGS_END;
			return node->data;
		}
		if (node->flags & FLAGS_END) {
			PRINT_ERROR("Out of memory.");
			_heap_print();
			return NULL;
		}
		node = (void*)node + node->size;
	}
}

void* kmalloc_page_align(usize size) {
	size = 8 * ((size + sizeof(struct HeapNode) + 7) / 8);
	struct HeapNode* node = HEAP_BASE;
	struct HeapNode* prev = NULL;
	while (true) {
		usize pad = PAGE_SIZE - (usize)((u64)node & 0xfff) - 8;
		if (!(node->flags & FLAGS_IN_USE) && node->size >= size + pad) {
			if (pad != 0) {
				prev->size += pad;
				node = (void*)prev + prev->size;
			}
			node->flags |= FLAGS_IN_USE;
			const u32 diff = node->size - size;
			if (diff < sizeof(struct HeapNode) + 8) {
				return node->data;
			}
			node->size = size;
			struct HeapNode* new_node = (void*)node + size;
			new_node->size = diff;
			// Copy the FLAGS_END flag in case it is set
			new_node->flags = node->flags & ~FLAGS_IN_USE;
			node->flags &= ~FLAGS_END;
			return node->data;
		}
		if (node->flags & FLAGS_END) {
			PRINT_ERROR("Out of memory.");
			return NULL;
		}
		prev = node;
		node = (void*)node + node->size;
	}
}

void kfree(void* addr) {
	struct HeapNode* node = addr - sizeof(*node);
	node->flags &= ~FLAGS_IN_USE;
	if (!(node->flags & FLAGS_END)) {
		struct HeapNode* next = (void*)node + node->size;
		if (!(next->flags & FLAGS_IN_USE)) {
			node->size += next->size;
			node->flags = next->flags;
		}
	}
	struct HeapNode* prev = HEAP_BASE;
	while (true) {
		if (prev->flags & FLAGS_END) {
			return;
		}
		struct HeapNode* next = (void*)prev + prev->size;
		if (next == node) {
			break;
		}
		prev = next;
	}
	if (prev->flags & FLAGS_IN_USE) {
		return;
	}
	prev->size += node->size;
	prev->flags = node->flags;
}

void* krealloc(void* addr, usize size) {
	if (addr == NULL) {
		return kmalloc(size);
	}
	struct HeapNode* node = addr - sizeof(*node);
	if (node->size >= size) {
		return addr;
	}
	void* new_addr = kmalloc(size);
	memcpy(new_addr, addr, node->size - sizeof(*node));
	kfree(addr);
	return new_addr;
}
