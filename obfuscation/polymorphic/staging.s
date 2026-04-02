BITS 64

section .text
global _start
_start:
	mov al, 4 ; stat
	mov rdi, [rsp + 8] ; get argv[0]
	lea rsi, [rel stat]
	syscall
	; save st_ino stat to rdx
	mov dl, [rel st_ino]

	; xor stuff
	mov rax, "iiiiiiii"
	xor rcx, rcx
	lea rbx, [rel text]
.Ldecoding:
	xor [rbx + rcx], dl
	cmp rcx, rax
	jge .Lcont
	inc rcx
	jmp .Ldecoding
.Lcont:
	jmp text

	; struct stat
stat:
	dq 0
st_ino:
	dq 0
	times 144-$+stat db 0

text:
