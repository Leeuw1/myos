#!/bin/sh
mount /dev/sdc1 /mnt &&
ls /mnt/kernel8.img &&
cp ./bin/kernel.img /mnt/kernel8.img &&
umount /mnt &&
echo "kernel.img written to /dev/sdc1"
