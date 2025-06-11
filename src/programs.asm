.section .programs
.globl _shell_binary
_shell_binary:
	.incbin "/home/david/myos/shell/shell"
.globl _count_binary
_count_binary:
	.incbin "/home/david/myos/shell/count"
