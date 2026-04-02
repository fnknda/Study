BITS 64

section .text
global _start
_start:
	mov rax, 1
	push rax
	pop rdi
	lea rsi, [rel text]
	mov rdx, 14
	syscall

	mov rax, 60
	xor rdi, rdi
	syscall

text:
	db "Hello, world!", 10

