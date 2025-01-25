.intel_syntax noprefix

.data
string1:
	.ascii "Hello\n"
string2:
	.ascii "Good-bye\n"

.text
.global _start
_start:
	lea rsi, [rip + string1]
	mov rdx, 6
	call print

	lea rsi, [rip + string2]
	mov rdx, 9
	call print

	mov rax, 60
	xor rdi, rdi
	syscall

print:
	mov rax, 1
	mov rdi, 1
	syscall
	ret
