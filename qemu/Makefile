qemu.bin: qemu.elf
	aarch64-linux-gnu-objcopy -O binary qemu.elf qemu.bin

qemu.elf: qemu.asm
	aarch64-linux-gnu-as qemu.asm -o qemu.elf
