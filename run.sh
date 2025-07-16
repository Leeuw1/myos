if [ "$1" == "d" ]; then
	extra="-s -S"
fi

qemu-system-aarch64 $extra \
	-machine raspi3b \
	-d guest_errors,unimp \
	-D qemu_logs.txt \
	-serial none \
	-serial stdio \
	-vga none \
	-kernel ./bin/kernel.elf \
	-drive if=sd,file=./qemu_sd_image.img,format=raw
