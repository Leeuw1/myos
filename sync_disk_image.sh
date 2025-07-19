#!/usr/bin/bash

# Mount
modprobe nbd max_part=8
qemu-nbd --connect=/dev/nbd0 ./qemu_disk_image.qcow2
fdisk /dev/nbd0 -l > /dev/null
mount /dev/nbd0p1 ./mnt

if [ "$1" == "--reset" ]; then
	rm -rf ./mnt/*
	mkdir ./mnt/{lib,usr,bin}
	mkdir ./mnt/usr/include
fi

rsync -a ./vfs/ ./mnt --no-owner --no-group
rsync ./libc/lib/libmyos_c.a ./mnt/lib/libc.a --no-owner --no-group
rsync ./libc/lib/libmyos_m.a ./mnt/lib/libm.a --no-owner --no-group
rsync ./libc/lib/crt*.o ./mnt/lib --no-owner --no-group
rsync -a ./libc/include/ ./mnt/usr/include --no-owner --no-group
rsync ./tinycc/arm64-libtcc1.a ./mnt/lib/arm64-libtcc1.a --no-owner --no-group
rsync ./tinycc/arm64-tcc ./mnt/bin/tcc --no-owner --no-group
rsync ./lua/lua ./mnt/bin/lua --no-owner --no-group
rsync ./vim/src/vim ./mnt/bin/vim --no-owner --no-group
rsync ./vim/src/xxd/xxd ./mnt/bin/xxd --no-owner --no-group

# Unmount
umount ./mnt
qemu-nbd --disconnect /dev/nbd0 > /dev/null
