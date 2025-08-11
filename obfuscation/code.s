global _start
section .text
_start:
	jmp greet
cont:

	mov rax, 60
	mov rdi, 0
	syscall

greet:
	mov rax, 1
	mov rdi, 1
	lea rsi, [rel msg]
	mov rdx, 14
	syscall

	jmp cont

msg:
	db "Hello, world!", 10

