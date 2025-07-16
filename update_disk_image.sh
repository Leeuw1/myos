#!/bin/bash
mount -o loop,offset=512 qemu_sd_image.img mnt
cp vim/src/vim mnt/bin/vim
cp lua/lua mnt/bin/lua
umount mnt
