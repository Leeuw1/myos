CC := bear -- aarch64-linux-gnu-gcc
AS := aarch64-linux-gnu-as
AR := aarch64-linux-gnu-ar
ARFLAGS := rc
CFLAGS := -nostdlib -nostdinc -Wall -Wextra -Wno-builtin-declaration-mismatch -O1 -mcpu=cortex-a53 -I./include
objects := ./bin/string.o ./bin/stdio.o ./bin/stdlib.o ./bin/fcntl.o ./bin/unistd.o ./bin/ctype.o ./bin/errno.o ./bin/wait.o ./bin/locale.o ./bin/signal.o ./bin/time.o ./bin/math.o ./bin/dirent.o ./bin/stat.o ./bin/termios.o ./bin/termcap.o ./bin/poll.o ./bin/grp.o ./bin/pwd.o ./bin/semaphore.o ./bin/dlfcn.o ./bin/mman.o

#all: ./lib/libc.so.6 ./lib/libm.so.6
all: ./lib/libmyos_c.a ./lib/libmyos_c2.a ./lib/libmyos_m.a

#./lib/libc.so.6: $(objects)
#	$(CC) $(CFLAGS) -shared $(objects) -o ./lib/libc.so.6

#./lib/libm.so.6: ./bin/math.o
#	$(CC) $(CFLAGS) -shared ./bin/math.o -o ./lib/libm.so.6

./lib/libmyos_c.a: $(objects) ./bin/crt1.o ./bin/setjmp.o
	$(AR) $(ARFLAGS) ./lib/libmyos_c.a $(objects) ./bin/crt1.o ./bin/setjmp.o

./lib/libmyos_c2.a: $(objects) ./bin/crt1.o ./bin/crti.o ./bin/crtn.o
	$(AR) $(ARFLAGS) ./lib/libmyos_c2.a $(objects)

./lib/libmyos_m.a: ./bin/math.o
	$(AR) $(ARFLAGS) ./lib/libmyos_m.a ./bin/math.o

$(objects): ./bin/%.o: ./src/%.c
	$(CC) $(CFLAGS) -c $^ -o $@

./bin/math.o: ./src/math.c
	$(CC) $(CFLAGS) -c ./src/math.c -o ./bin/math.o

./bin/crt1.o: ./src/crt1.asm
	$(AS) ./src/crt1.asm -o ./bin/crt1.o

./bin/crti.o: ./src/crti.asm
	$(AS) ./src/crti.asm -o ./bin/crti.o

./bin/crtn.o: ./src/crtn.asm
	$(AS) ./src/crtn.asm -o ./bin/crtn.o

./bin/setjmp.o: ./src/setjmp.asm
	$(AS) ./src/setjmp.asm -o ./bin/setjmp.o

clean:
	rm -f ./bin/* ./lib/*
