#include <stdio.h>
#include <stdlib.h>

int main(int argc, char** argv) {
	printf("There are %d ARGS: ", argc);
	for (int i = 0; i < argc; ++i) {
		printf("%s ", argv[i]);
	}
	putchar('\n');
	int* buffer = malloc(1000);
	for (int i = 0; i < 10; ++i) {
		buffer[i] = i;
		printf("hello %d\n", i);
		for (size_t j = 0; j < 0x1ffffff; ++j) {
		}
	}
	free(buffer);
	return 0;
}
