CC := bear -- aarch64-linux-gnu-gcc
LD := aarch64-linux-gnu-ld
AS := aarch64-linux-gnu-as
OBJCOPY := aarch64-linux-gnu-objcopy
DATE := $(shell date)
CFLAGS := -ffreestanding -fno-unwind-tables -fno-asynchronous-unwind-tables -nostdinc -nostdlib -Wall -Wextra -O1 -D MYOS_BUILD_INFO='"$(DATE)"' -mcpu=cortex-a53 -I./libc/include
ASFLAGS := -mcpu=cortex-a53

c_objects := ./bin/main.o ./bin/tty.o ./bin/mailbox.o ./bin/print.o ./bin/exception.o ./bin/syscall.o ./bin/fs.o ./bin/heap.o ./bin/proc.o ./bin/_string.o ./bin/commands.o ./bin/myos_time.o ./bin/sd.o ./bin/fat.o
asm_objects := ./bin/init.o ./bin/vectors.o
objects :=  $(asm_objects) ./bin/programs.o $(c_objects)

./bin/kernel.img: ./bin/kernel.elf
	$(OBJCOPY) -O binary ./bin/kernel.elf ./bin/kernel.img
	truncate -s %512 ./bin/kernel.img

./bin/kernel.elf: $(objects) kernel.ld
	$(LD) -T kernel.ld $(objects) -o ./bin/kernel.elf

$(c_objects): ./bin/%.o: ./src/%.c
	$(CC) $(CFLAGS) -c $^ -o $@

$(asm_objects): ./bin/%.o: ./src/%.asm
	$(AS) $(ASFLAGS) $^ -o $@

./bin/programs.o: ./src/programs.asm ./shell/shell ./lua/lua
	$(AS) $(ASFLAGS) ./src/programs.asm -o ./bin/programs.o

./lua/lua: ./libc/lib/libmyos_c.a ./libc/lib/libmyos_m.a
	$(MAKE) -C lua

./shell/shell ./shell/count: ./libc/lib/libmyos_c.a
	$(MAKE) -C shell clean
	$(MAKE) -C shell

clean:
	rm -f ./bin/*
