BITS 64

section .text
global _start
_start:
	mov rax, 1
	mov rdi, 1
	lea rsi, [rel msg]
	mov rdx, 14
	syscall

	mov rax, 60
	mov rdi, 0
	syscall

msg:
	db "Hello, world!", 10

