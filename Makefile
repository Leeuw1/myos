CC := bear -- aarch64-linux-gnu-gcc
LD := aarch64-linux-gnu-ld
AS := aarch64-linux-gnu-as
OBJCOPY := aarch64-linux-gnu-objcopy
DATE := $(shell date)
CFLAGS := -ffreestanding -fno-unwind-tables -fno-asynchronous-unwind-tables -nostdlib -nostdinc -Wall -Wextra -O1 -D MYOS_BUILD_INFO='"$(DATE)"' -mcpu=cortex-a53 -I./libc/include -I./libc/src
ASFLAGS := -mcpu=cortex-a53

c_objects := ./bin/main.o ./bin/tty.o ./bin/mailbox.o ./bin/print.o ./bin/exception.o ./bin/syscall.o ./bin/fs.o ./bin/heap.o ./bin/proc.o ./bin/_string.o ./bin/commands.o
objects := ./bin/init.o ./bin/vectors.o $(c_objects)

./bin/kernel.img: ./bin/kernel.elf
	$(OBJCOPY) -O binary ./bin/kernel.elf ./bin/kernel.img
	truncate -s %512 ./bin/kernel.img

./bin/kernel.elf: $(objects) kernel.ld
	$(LD) -T kernel.ld $(objects) -o ./bin/kernel.elf

./bin/init.o: ./src/init.asm
	$(AS) $(ASFLAGS) ./src/init.asm -o ./bin/init.o

./bin/vectors.o: ./src/vectors.asm
	$(AS) $(ASFLAGS) ./src/vectors.asm -o ./bin/vectors.o

$(c_objects): ./bin/%.o: ./src/%.c
	$(CC) $(CFLAGS) -c $^ -o $@

clean:
	rm -f ./bin/*
