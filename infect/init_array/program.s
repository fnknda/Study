.intel_syntax noprefix

.data
.global hello_world
hello_world:
	.ascii "Hello, world!\n"
.Lhello_world_len:
	.int $-hello_world

.text
.global main
main:
	# exit(0)
	mov rax, 60
	xor rdi, rdi
	syscall

.section .text
.global custom_init
custom_init:
	# write(FILENO_STDOUT, "Hello, world!\n", 14)
	mov rax, 1
	mov rdi, 1
	lea rsi, [rip + hello_world]
	mov edx, [rip + .Lhello_world_len]
	syscall

	ret

.section .init_array
	.quad custom_init
