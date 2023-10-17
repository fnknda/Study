.intel_syntax noprefix

.text
.global _start
_start:
	push rdx

	mov rax, 0x0a21616e616e6162
	mov QWORD PTR [rsp - 8], rax

	mov eax, 1
	mov edi, 1
	lea rsi, [rsp - 8]
	mov edx, 8
	syscall

	pop rdx
	jmp 0x0000
