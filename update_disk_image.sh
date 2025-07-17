#!/bin/bash
mount -o loop,offset=512 qemu_sd_image.img mnt
cp vim/src/vim mnt/bin/vim
cp lua/lua mnt/bin/lua
cp tinycc/arm64-tcc mnt/bin/tcc
umount mnt
