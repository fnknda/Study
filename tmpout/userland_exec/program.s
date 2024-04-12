.intel_syntax noprefix

.data
string:
	.ascii "Hello\n"

.text
.global _start
_start:
	mov rax, 1
	mov rdi, 1
	lea rsi, [rip + string]
	mov rdx, 6
	syscall

	mov rax, 60
	xor rdi, rdi
	syscall
