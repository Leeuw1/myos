CC := bear -- aarch64-linux-gnu-gcc
LD := aarch64-linux-gnu-ld
CFLAGS := -pie -nostdinc -nostdlib -Wno-builtin-declaration-mismatch -Wall -Wextra -O1 -mcpu=cortex-a53 -I../libc/include -I..

all: shell count

shell: main.o
	$(CC) $(CFLAGS) main.o -o shell -L../libc/lib -L../readline -lreadline -lmyos_c

count: count.o
	$(CC) $(CFLAGS) count.o -o count -L../libc/lib -lmyos_c

count.o: count.c
	$(CC) $(CFLAGS) -c count.c

main.o: main.c
	$(CC) $(CFLAGS) -c main.c

clean:
	rm -f *.o shell
