.intel_syntax noprefix

.text
.global _start
_start:
	mov rax, 0x0a2164656e767670
	mov QWORD PTR [rsp - 8], rax

	mov rax, 1
	mov rdi, 1
	lea rsi, [rsp - 8]
	mov rdx, 8
	syscall

	jmp 0x0000
