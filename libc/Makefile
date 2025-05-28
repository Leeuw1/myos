CC := bear -- aarch64-linux-gnu-gcc
AR := aarch64-linux-gnu-ar
CFLAGS := -nostdlib -nostdinc -Wall -Wextra -O1 -mcpu=cortex-a53 -I./include -I./src
objects := ./bin/string.o

./bin/libc.a: $(objects)
	$(AR) rcs ./bin/libc.a $(objects)

$(objects): ./bin/%.o: ./src/%.c
	$(CC) $(CFLAGS) -c $^ -o $@

clean:
	rm -f ./bin/*
