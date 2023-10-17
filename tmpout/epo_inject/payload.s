.intel_syntax noprefix

.text
.global _start
_start:
	lea rax, [rip + .Lcode]
	jmp rax
	.Lstring:
	.ascii "Pvvn3d!\n"
	.Lcode:
	mov eax, 1
	mov edi, 1
	lea rsi, [rip + .Lstring]
	mov edx, 8
	syscall

	jmp 0x0000
