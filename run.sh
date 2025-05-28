qemu-system-aarch64 \
	-machine raspi3b \
	-d guest_errors,unimp,int \
	-D qemu_logs.txt \
	-serial none \
	-serial stdio \
	-vga none \
	-device loader,file=./qemu/qemu.bin,addr=0x0 \
	-device loader,file=./bin/kernel.img,addr=0x80000
	#-d cpu_reset,int,guest_errors,unimp \
