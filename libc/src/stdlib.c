#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>
#include <limits.h>
#include <assert.h>
#define MYOS_NO_BOOL
#include "../../src/syscall.h"

#define UNIMP()			printf("[libc] Warning: %s is not implemented.\n", __FUNCTION__);

#define MIN_BIT_INDEX		4
#define MAX_BIT_INDEX		20
#define FREE_LIST_COUNT		(MAX_BIT_INDEX - MIN_BIT_INDEX + 1)
#define FREE_LIST_MAX_INDEX	(FREE_LIST_COUNT - 1)
#define MAX_NODE_SIZE		((1ull << MAX_BIT_INDEX) - 16)

struct FreeNode {
	size_t				size;
	struct FreeNode*	next;
	uint8_t				data[];
};

static void* _heap_base;
static size_t _heap_size;
static size_t _heap_block_size;
// _free_lists[i] should correspond to blocks of size 2^(i+5) - 16
struct FreeNode* _free_lists[FREE_LIST_COUNT];

#if 0
struct HeapNode {
	void*	address;
	size_t	size;
	bool	in_use;
};
#endif

#if 0
static struct HeapNode _heap[MAX_HEAP_COUNT];
static size_t _heap_count;
#endif
static uint64_t _malloc_counter;
static uint64_t _free_counter;
// TODO: maybe use 'environ' instead
#define MAX_ENV_COUNT	16
#define ENTRY_SIZE		128
static char _env[MAX_ENV_COUNT][ENTRY_SIZE];
static size_t _env_count;

static unsigned _rand_seed = 0;

// Take a block of memory and fill it with max size free nodes
// Add these nodes to the max level free list
static void _init_heap_block(void* addr, usize size) {
	struct FreeNode* node = addr;
	const usize node_count = size / MAX_NODE_SIZE;
	for (usize i = 0; i < node_count - 1; ++i) {
		node->size = 0xdeadbeef;
		node->next = (void*)node->data + MAX_NODE_SIZE; 
		node = node->next;
	}
	node->size = 0xdeadbeef;
	node->next = NULL;
	_free_lists[FREE_LIST_MAX_INDEX] = addr;
}

void _libc_init_stdlib(void* base, size_t block_size) {
	_heap_base = base;
	_heap_block_size = block_size;
	_heap_size = _heap_block_size;
	memset(_free_lists, 0, sizeof _free_lists);
	_init_heap_block(_heap_base, _heap_size);
#if 0
	_heap_count = 1;
	_heap[0].address = base;
	_heap[0].size = HEAP_SIZE;
	_heap[0].in_use = false;
#endif
	_env_count = 0;
	setenv("LINES", "40", 0);
	setenv("COLUMNS", "180", 0);
	setenv("TERM", "xterm-256color", 0);
	setenv("HOME", "/", 0);
	setenv("LANG", "en_US.UTF-8", 0);
	setenv("SHELL", "/bin/sh", 0);
	setenv("VIM", "/vim", 0);
	setenv("VIMRUNTIME", "/vim/vim91", 0);
}

#if 0
static usize _heap_lookup(const void* address) {
	for (size_t i = 0; i < _heap_count; ++i) {
		if (_heap[i].address == address) {
			return i;
		}
	}
	printf("_heap_lookup(): Error, no node with address %p.\n", address);
	abort();
}
#endif

_Noreturn void abort(void) {
	exit(EXIT_FAILURE);
}

int abs(int x) {
	return x >= 0 ? x : -x;
}

// TODO
int atoi(const char* nptr) {
	return (int)atol(nptr);
}

// TODO
long atol(const char* nptr) {
	return strtol(nptr, NULL, 10);
}

// TODO: use binary search
void* bsearch(const void* key, const void* base, size_t n, size_t size, int (*compare)(const void*, const void*)) {
	for (size_t i = 0; i < n; ++i) {
		if (compare(base + i * size, key) == 0) {
			return (void*)(base + i * size);
		}
	}
	return NULL;
}

void* calloc(size_t n, size_t size) {
	void* buf = malloc(n * size);
	memset(buf, 0, n * size);
	return buf;
}

_Noreturn void exit(int status) {
	_syscall_1arg_noreturn(SYSCALL_EXIT, status);
	__builtin_unreachable();
}

#if 0
void _try_merge(usize i) {
	if (i >= _heap_count - 1) {
		return;
	}
	if (_heap[i].in_use || _heap[i + 1].in_use) {
		return;
	}
	_heap[i].size += _heap[i + 1].size;
	--_heap_count;
	for (usize j = i + 1; j < _heap_count; ++j) {
		_heap[j] = _heap[j + 1];
	}
}
#endif

void free(void* ptr) {
	++_free_counter;
	if (ptr == NULL) {
		return;
	}
	struct FreeNode* node = ptr - offsetof(struct FreeNode, data);
	assert((void*)node >= _heap_base);
	assert((void*)node < _heap_base + _heap_size);
	if (node->size == 0xdeadbeef) {
		printf("[libc] Double free() detected.\n");
		abort();
	}
	const size_t size = node->size + sizeof(struct FreeNode);
	node->size = 0xdeadbeef;
	const unsigned int first_leading_one = __builtin_stdc_first_leading_one(size);
	const size_t index = 64 - first_leading_one - MIN_BIT_INDEX;
	node->next = _free_lists[index];
	_free_lists[index] = node;
#if 0
	struct FreeNode* it = _free_lists[index];
	if (it == NULL) {
		node->next = NULL;
		_free_lists[index] = node;
		return;
	}
	if (it->next == NULL) {
		node->next = _free_lists[index]->next;
		_free_lists[index] = node;
		return;
	}
	while (it->next != NULL) {

	}
#endif
}

char* getenv(const char* name) {
	for (size_t i = 0; i < _env_count; ++i) {
		char* equals = strchr(_env[i], '=');
		if (equals == NULL) {
			return NULL;
		}
		const size_t name_length = equals - _env[i];
		if (strlen(name) != name_length) {
			continue;
		}
		if (memcmp(_env[i], name, name_length) != 0) {
			continue;
		}
		return equals + 1;
	}
	return NULL;
}

long labs(long x) {
	return x >= 0 ? x : -x;
}

#if 0
// When we run out of memory, check how much free space we lost due to fragmentation
void _free_size(size_t* restrict size, size_t* restrict regions) {
	*size = 0;
	*regions = 0;
	for (size_t i = 0; i < _heap_count; ++i) {
		if (!_heap[i].in_use) {
			*size += _heap[i].size;
			++*regions;
		}
	}
}
#endif

static size_t _free_list_index(size_t size) {
	size += sizeof(struct FreeNode);
	const unsigned int first_leading_one = __builtin_stdc_first_leading_one(size);
	size_t bit_index = 64 - first_leading_one;
	if (bit_index < MIN_BIT_INDEX) {
		bit_index = MIN_BIT_INDEX;
	}
	else if (bit_index > MAX_BIT_INDEX) {
		printf("[libc] malloc(): Size %d is too big.\n", (int)size);
		abort();
	}
	else if (size & (~0ull >> first_leading_one)) {
		++bit_index;
	}
	return bit_index - MIN_BIT_INDEX;
}

static void __attribute__((noinline)) _grow_heap(size_t size) {
	_syscall_1arg_noreturn(SYSCALL_GROW_HEAP, size);
}

#if 0
static void* _malloc(size_t size) {
	struct FreeNode* prev = NULL;
	struct FreeNode* it = _free_lists[FREE_LIST_MAX_INDEX];
	while (it != NULL) {
		if (it->size >= size) {
			break;
		}
		prev = it;
		it = it->next;
	}
	if (it == NULL) {
		const size_t increase = ((size + _heap_block_size - 1) / _heap_block_size) * _heap_block_size;
		printf("Growing heap by %d bytes...\n", (int)increase);
		_grow_heap(increase);
		prev = NULL;
		it = _heap_base + _heap_size;
		it->size = increase - sizeof(struct FreeNode);
		it->next = _free_lists[FREE_LIST_MAX_INDEX];
		_free_lists[FREE_LIST_MAX_INDEX] = it;
		_heap_size += increase;
	}
	const size_t diff = it->size - size;
	if (diff >= sizeof(struct FreeNode) + MIN_NODE_SIZE) {
		struct FreeNode* next = it->next;
		it->size = size;
		it->next = (struct FreeNode*)(it->data + it->size);
		it->next->size = diff - sizeof(struct FreeNode);
		it->next->next = next;
	}
	if (prev == NULL) {
		_free_lists[FREE_LIST_MAX_INDEX] = it->next;
	}
	else {
		prev->next = it->next;
	}
	return it->data;
}
#endif

// Ensure a node is available. If _free_lists[index] is empty there are 2 options:
// 1. Split a node from level index + 1 in half
// 2. Map more memory
static void _ensure_node(size_t index) {
	if (_free_lists[index] != NULL) {
		return;
	}
	if (index == FREE_LIST_MAX_INDEX) {
		const usize increase = _heap_block_size;
		_grow_heap(increase);
		_init_heap_block(_heap_base + _heap_size, increase);
		_heap_size += increase;
		assert((void*)_free_lists[index] >= _heap_base);
		assert((void*)_free_lists[index] < _heap_base + _heap_size);
		return;
	}
	_ensure_node(index + 1);
	_free_lists[index] = _free_lists[index + 1];
	_free_lists[index + 1] = _free_lists[index + 1]->next;
	const size_t size = (1ull << (index + MIN_BIT_INDEX)) - 16;
	_free_lists[index]->size = 0xdeadbeef;
	_free_lists[index]->next = (void*)_free_lists[index]->data + size;
	_free_lists[index]->next->size = 0xdeadbeef;
	_free_lists[index]->next->next = NULL;
	assert((void*)_free_lists[index] >= _heap_base);
	assert((void*)_free_lists[index] < _heap_base + _heap_size);
	assert((void*)_free_lists[index]->next >= _heap_base);
	assert((void*)_free_lists[index]->next < _heap_base + _heap_size);
}

void* malloc(size_t size) {
	++_malloc_counter;
	const size_t index = _free_list_index(size);
	_ensure_node(index);
	void* addr = _free_lists[index]->data;
	if (addr <= _heap_base || addr >= _heap_base + _heap_size) {
		printf("[libc] malloc() error (heap base: %p, heap size: %x, addr: %p)\n",
				_heap_base, (unsigned int)_heap_size, addr);
		abort();
	}
	size = (1ull << (index + MIN_BIT_INDEX)) - 16;
	_free_lists[index]->size = size;
	_free_lists[index] = _free_lists[index]->next;
	//printf("malloc() success, returning %p\n", addr);
	return addr;
#if 0
#if 1
	if (size >= 0x8000) {
		printf("malloc(%d)\n", (int)size);
	}
#endif
	size = size == 0 ? 8 : ((size + 7) / 8) * 8;
	for (size_t i = 0; i < _heap_count; ++i) {
		if (_heap[i].in_use || _heap[i].size < size) {
			continue;
		}
		_heap[i].in_use = true;
		const size_t diff = _heap[i].size - size;
		if (diff == 0 || _heap_count == MAX_HEAP_COUNT) {
			//printf("malloc() success, returning %p\n", _heap[i].address);
			return _heap[i].address;
		}
		_heap[i].size = size;
		for (size_t j = _heap_count; j > i + 1; --j) {
			_heap[j] = _heap[j - 1];
		}
		++_heap_count;
		_heap[i + 1].address = _heap[i].address + _heap[i].size;
		_heap[i + 1].size = diff;
		_heap[i + 1].in_use = false;
		//printf("malloc() success, returning %p\n", _heap[i].address);
		return _heap[i].address;
	}
	size_t free_size;
	size_t free_regions;
	_free_size(&free_size, &free_regions);
	printf("malloc(): Error, out of memory (%d/%d regions used, %d free regions, %d free bytes, %d mallocs, %d frees).\n",
			(int)_heap_count, (int)MAX_HEAP_COUNT, (int)free_regions, (int)free_size, (int)_malloc_counter, (int)_free_counter);
	abort();
#endif
}

int mkstemp(char* temp) {
	UNIMP();
	return 0;
}

static void _swap(void* restrict a, void* restrict b, void* restrict temp, size_t size) {
	memcpy(temp, a, size);
	memcpy(a, b, size);
	memcpy(b, temp, size);
}

static void _qsort(void* base, size_t n, size_t size, int (* compare)(const void*, const void*), void* temp) {
	if (n <= 1) {
		return;
	}
	// TODO: should use random pivot
	size_t pivot = n / 2;
	size_t i = 0;
	size_t j = n - 1;
	while (i < pivot && j > pivot) {
		const bool left_swap = compare(base + i * size, base + pivot * size) >= 0;
		const bool right_swap = compare(base + j * size, base + pivot * size) <= 0;
		if (left_swap && right_swap) {
			_swap(base + i * size, base + j * size, temp, size);
			++i;
			--j;
			continue;
		}
		if (!left_swap) {
			++i;
		}
		if (!right_swap) {
			--j;
		}
	}
	if (j == pivot) {
		for (; i < pivot; ++i) {
			if (compare(base + i * size, base + pivot * size) <= 0) {
				continue;
			}
			if (i == pivot - 1) {
				_swap(base + i * size, base + pivot * size, temp, size);
			}
			else {
				_swap(base + i * size, base + (pivot - 1) * size, temp, size);
				_swap(base + (pivot - 1) * size, base + pivot * size, temp, size);
			}
			--pivot;
		}
	}
	else if (i == pivot) {
		for (; j > pivot; --j) {
			if (compare(base + j * size, base + pivot * size) >= 0) {
				continue;
			}
			if (j == pivot + 1) {
				_swap(base + j * size, base + pivot * size, temp, size);
			}
			else {
				_swap(base + j * size, base + (pivot + 1) * size, temp, size);
				_swap(base + (pivot + 1) * size, base + pivot * size, temp, size);
			}
			++pivot;
		}
	}
	_qsort(base, pivot, size, compare, temp);
	_qsort(base + (size * (pivot + 1)), n - pivot - 1, size, compare, temp);
}

void qsort(void* base, size_t n, size_t size, int (* compare)(const void*, const void*)) {
	void* temp = malloc(size);
	_qsort(base, n, size, compare, temp);
	free(temp);
}

// Linear Congruential Generator
int rand(void) {
	_rand_seed = 214013 * _rand_seed + 2531011;
	return (_rand_seed >> 16) & RAND_MAX;
}

void* realloc(void* ptr, size_t size) {
	//printf("realloc(%d)...\n", (int)size);
	if (ptr == NULL) {
		return malloc(size);
	}
	const struct FreeNode* node = ptr - offsetof(struct FreeNode, data);
	assert(node->size != 0xdeadbeef);
	const size_t min = size < node->size ? size : node->size;
	void* new = malloc(size);
	memcpy(new, ptr, min);
	free(ptr);
	return new;
}

static int __attribute__((noinline)) canonicalize(const char* restrict path, char* restrict dst) {
	_syscall_2arg(SYSCALL_CANONICALIZE, int, path, dst);
}

char* realpath(const char* restrict file_name, char* restrict resolved_name) {
	char* buf = resolved_name == NULL ? malloc(PATH_MAX) : resolved_name;
	if (canonicalize(file_name, buf) == 0) {
		return buf;
	}
	if (resolved_name == NULL) {
		free(buf);
	}
	return NULL;
}

int setenv(const char* name, const char* value, int overwrite) {
	if (name == NULL) {
		errno = EINVAL;
		return -1;
	}
	if (name[0] == '\0' || strchr(name, '=') != NULL) {
		errno = EINVAL;
		return -1;
	}
	if (strlen(name) + strlen(value) + 2 > ENTRY_SIZE) {
		errno = ENOMEM;
		return -1;
	}
	for (size_t i = 0; i < _env_count; ++i) {
		const size_t name_length = strchr(_env[i], '=') - _env[i];
		if (strlen(name) != name_length) {
			continue;
		}
		if (memcmp(_env[i], name, name_length) != 0) {
			continue;
		}
		if (overwrite) {
			strcpy(_env[i] + name_length + 1, value);
#if 1
	printf("changed env entry: [%s]\n", _env[_env_count - 1]);
#endif
		}
		return 0;
	}
	if (_env_count == MAX_ENV_COUNT) {
		errno = ENOMEM;
		return -1;
	}
	snprintf(_env[_env_count++], ENTRY_SIZE, "%s=%s", name, value);
	return 0;
}

void srand(unsigned seed) {
	_rand_seed = seed;
}

// TODO
double strtod(const char* restrict nptr, char** restrict endptr) {
	UNIMP();
	while (isspace(*nptr)) {
		++nptr;
	}
	int sign = 1;
	if (*nptr == '-') {
		sign = -1;
		++nptr;
	}
	else if (*nptr == '+') {
		++nptr;
	}
	double value = 0.0;
	while (isdigit(*nptr)) {
		value *= 10.0;
		value += *nptr - '0';
		++nptr;
	}
	double divisor = 1.0;
	if (*nptr == '.') {
		++nptr;
		while (isdigit(*nptr)) {
			value *= 10.0;
			value += *nptr - '0';
			divisor *= 10.0;
			++nptr;
		}
	}
	if (endptr != NULL) {
		*endptr = (char*)nptr;
	}
	const double abs = value / divisor;
	return sign == 1 ? abs : -abs;
}

// TODO
float strtof(const char* restrict nptr, char** restrict endptr) {
	UNIMP();
	while (isspace(*nptr)) {
		++nptr;
	}
	while (isalnum(*nptr) || *nptr == '.') {
		++nptr;
	}
	if (endptr != NULL) {
		*endptr = (char*)nptr;
	}
	return 0.0f;
}

// Returns -1 if base is invalid or parsed base does not match
static int _determine_base(int base, const char* restrict* nptr) {
	if (base != 0 && (base < 2 || base > 36)) {
		return -1;
	}
	if (**nptr != '0') {
		return base == 0 ? 10 : base;
	}
	++*nptr;
	int parsed_base = 8;
	switch (**nptr) {
	case 'b':
	case 'B':
		parsed_base = 2;
		break;
	case 'x':
	case 'X':
		parsed_base = 16;
		break;
	}
	if (**nptr != '\0') {
		++*nptr;
	}
	return (parsed_base != base && base != 0) ? -1 : parsed_base;
}

// base is in range [2, 36]
static bool _read_digit(long* digit, char c, int base) {
	if (isdigit(c)) {
		*digit = c - '0';
	}
	else if (islower(c)) {
		*digit = 10 + c - 'a';
	}
	else if (isupper(c)) {
		*digit = 10 + c - 'A';
	}
	else {
		return false;
	}
	return *digit < base;
}

long strtol(const char* restrict nptr, char** restrict endptr, int base) {
	long sign = 1;
	if (*nptr == '-') {
		sign = -1;
		++nptr;
	}
	else if (*nptr == '+') {
		++nptr;
	}
	base = _determine_base(base, &nptr);
	if (base == -1) {
		errno = EINVAL;
		return 0;
	}
	long value = 0;
	long digit;
	for (; _read_digit(&digit, *nptr, base); ++nptr) {
		value *= (long)base;
		value += digit;
	}
	if (endptr != NULL) {
		*endptr = (char*)nptr;
	}
	return value * sign;
}

long double strtold(const char* restrict nptr, char** restrict endptr) {
	UNIMP();
	return 0;
}

long long strtoll(const char* restrict nptr, char** restrict endptr, int base) {
	UNIMP();
	return 0;
}

unsigned long strtoul(const char* restrict nptr, char** restrict endptr, int base) {
	UNIMP();
	return 0;
}

unsigned long long strtoull(const char* restrict nptr, char** restrict endptr, int base) {
	UNIMP();
	return 0;
}

int system(const char* command) {
	if (command == NULL) {
		return 1;
	}
	const pid_t pid = fork();
	if (pid == -1) {
		return -1;
	}
	if (pid == 0) {
		char* const argv[] = { "/bin/sh", "-c", (char*)command, NULL };
		execv("/bin/sh", argv);
		exit(127);
	}
	int status;
	waitpid(pid, &status, 0);
	return status;
}
