.intel_syntax noprefix

.text
	mov al, 1
	push rax
	pop rdi
	lea rsi, [rip + .Ltext]
	mov dl, 14
	syscall

	mov al, 60
	xor edi, edi
	syscall

.Ltext:
	.ascii "Hello, world!\n"
