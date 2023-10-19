.intel_syntax noprefix

.text
#	mov eax, 1
#	mov edi, 1
#	lea rsi, [rip + .Ltext]
#	mov edx, 14
#	syscall

	mov al, 60
	xor edi, edi
	syscall

#.Ltext:
#	.ascii "Hello, world!\n"
