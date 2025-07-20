#include "heap.h"
#include "io.h"
#include <string.h>

extern u8 _HEAP_START_;

#define HEAP_BASE	((void*)&_HEAP_START_)
#define HEAP_SIZE	((void*)0x30000000 - HEAP_BASE)

#define MAX_HEAP_COUNT	512

struct _HeapNode {
	usize	size;
	u32		offset;
	bool	in_use;
};

static struct _HeapNode	_heap[MAX_HEAP_COUNT];
static usize			_heap_count;

void heap_init(void) {
	_heap_count = 1;
	_heap[0].size = HEAP_SIZE;
	_heap[0].offset = 0;
	_heap[0].in_use = false;
}

static void _heap_print(void) {
	print("_heap_print:\n");
	for (usize i = 0; i < _heap_count; ++i) {
		if (_heap[i].in_use) {
			printf("address=%, size=%, status=allocated\n",
					HEAP_BASE + (u64)_heap[i].offset, (u64)_heap[i].size);
		}
		else {
			printf("address=%, size=%, status=free\n",
					HEAP_BASE + (u64)_heap[i].offset, (u64)_heap[i].size);
		}
	}
}

static bool _heap_try_split(usize index, usize size) {
	const usize diff = _heap[index].size - size;
	if (diff < 8) {
		return false;
	}
	if (_heap_count == MAX_HEAP_COUNT) {
		PRINT_ERROR("No room left to split.");
		return false;
	}
	for (usize i = _heap_count; i > index; --i) {
		_heap[i] = _heap[i - 1];
	}
	++_heap_count;
	_heap[index].size = size;
	_heap[index + 1].offset = _heap[index].offset + size;
	_heap[index + 1].size = diff;
	return true;
}

void* kmalloc(usize size) {
	size = 8 * ((size + 7) / 8);
	for (usize i = 0; i < _heap_count; ++i) {
		if (_heap[i].in_use || _heap[i].size < size) {
			continue;
		}
		_heap_try_split(i, size);
		_heap[i].in_use = true;
		return HEAP_BASE + _heap[i].offset;
	}
	PRINT_ERROR("Out of memory.");
	_heap_print();
	return NULL;
}

void* kmalloc_page_align(usize size) {
	size = 8 * ((size + 7) / 8);
	for (usize i = 0; i < _heap_count; ++i) {
		const usize pad = (0x1000 - (_heap[i].offset & 0xfff)) & 0xfff;
		if (_heap[i].in_use || _heap[i].size < pad + size) {
			continue;
		}
		if (pad == 0) {
			_heap_try_split(i, size);
			_heap[i].in_use = true;
			return HEAP_BASE + _heap[i].offset;
		}
		if (!_heap_try_split(i, pad)) {
			continue;
		}
		_heap_try_split(i + 1, size);
		_heap[i + 1].in_use = true;
		return HEAP_BASE + _heap[i + 1].offset;
	}
	PRINT_ERROR("Out of memory.");
	_heap_print();
	return NULL;
}

void kfree(void* addr) {
	if (addr == NULL) {
		return;
	}
	for (usize i = 0; i < _heap_count; ++i) {
		if (HEAP_BASE + _heap[i].offset != addr) {
			continue;
		}
		if (!_heap[i].in_use) {
			PRINT_ERROR("Double free detected.");
			return;
		}
		_heap[i].in_use = false;
		// TODO: merging
		return;
	}
	PRINT_ERROR("Address does not match any existing node.");
}

void* krealloc(void* addr, usize size) {
	if (addr == NULL) {
		return kmalloc(size);
	}
	for (usize i = 0; i < _heap_count; ++i) {
		if (HEAP_BASE + _heap[i].offset != addr) {
			continue;
		}
		if (!_heap[i].in_use) {
			PRINT_ERROR("Corresponding node is not allocated.");
			return NULL;
		}
		void* new_addr = kmalloc(size);
		memcpy(new_addr, addr, (usize)_heap[i].size);
		kfree(addr);
		return new_addr;
	}
	PRINT_ERROR("Address does not match any existing node.");
	return NULL;
}
