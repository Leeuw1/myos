#!/bin/bash
mount -o loop,offset=512 qemu_sd_image.img mnt
cp vim/src/vim mnt/bin/vim
cp vim/src/xxd/xxd mnt/bin/xxd
cp lua/lua mnt/bin/lua
cp tinycc/arm64-tcc mnt/bin/tcc
cp tinycc/arm64-libtcc1.a mnt/lib
cp -r libc/include mnt/usr
cp libc/lib/libmyos_c2.a mnt/lib/libc.a
cp libc/lib/libmyos_m.a mnt/lib/libm.a
cp libc/bin/crt*.o mnt/lib
umount mnt
