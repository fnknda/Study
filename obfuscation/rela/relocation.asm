bits 64
org 0x27000

ehdr:
	db 0x7f, "ELF"
	db 2 ; ELFCLASS64
	db 1 ; ELFDATA2LSB
	db 1 ; EV_CURRENT
	db 3 ; ELFOSABI_LINUX
	db 0
	times (16 + ehdr - $) db 0

	dw 3 ; e_type = ET_DYNAMIC
	dw 62 ; e_machine = EM_X86_64
	dd 1 ; e_version = EV_CURRENT
	dq text ; e_entry
	dq phdr - $$ ; e_phoff
	dq shdr - $$ ; e_shoff
	dd 0 ; e_flags
	dw 64 ; e_ehsize = sizeof(Elf64_Ehdr)
	dw 56 ; e_phentsize = sizeof(Elf64_Phdr)
	dw 4 ; e_phnum
	dw 64 ; e_shentsize = sizeof(Elf64_Shdr)
	dw 3 ; e_shnum
	dw 0 ; e_shstrndx

phdr:
phdr.phdr:
	dd 6 ; p_type PT_INTERP
	dd 0x4 ; p_flags = PF_R
	dq phdr - $$ ; p_offset
	dq phdr ; p_vaddr
	dq phdr ; p_paddr
	dq phdr.end - phdr ; p_filesz
	dq phdr.end - phdr ; p_memsz
	dq 0x1 ; p_align

phdr.interp:
	dd 3 ; p_type PT_INTERP
	dd 0x4 ; p_flags = PF_R
	dq interp - $$ ; p_offset
	dq interp ; p_vaddr
	dq interp ; p_paddr
	dq interp.end - interp ; p_filesz
	dq interp.end - interp ; p_memsz
	dq 0x1 ; p_align

phdr.dynamic:
	dd 2 ; p_type PT_DYNAMIC
	dd 0x4 ; p_flags = PF_R
	dq dynamic - $$ ; p_offset
	dq dynamic ; p_vaddr
	dq dynamic ; p_paddr
	dq dynamic.end - dynamic ; p_filesz
	dq dynamic.end - dynamic ; p_memsz
	dq 0x1 ; p_align

phdr.text:
	dd 1 ; p_type PT_LOAD
	dd 0x7 ; p_flags = PF_R | PF_W | PF_X
	dq phdr - $$ ; p_offset
	dq phdr ; p_vaddr
	dq phdr ; p_paddr
	dq text.end - phdr ; p_filesz
	dq text.end - phdr ; p_memsz
	dq 0x1000 ; p_align
phdr.end:

interp:
	db "/lib64/ld-linux-x86-64.so.2", 0
interp.end:

dynamic:
dynamic.strtab:
	dq 5 ; d_tag = DT_STRTAB
	dq strtab ; d_ptr

dynamic.symtab:
	dq 6 ; d_tag = DT_SYMTAB
	dq symtab ; d_ptr

dynamic.rela:
	dq 7 ; d_tag = DT_RELA
	dq rela ; d_ptr

dynamic.relaent:
	dq 9 ; d_tag = DT_RELAENT
	dq rela.zero.end - rela.zero ; d_val

dynamic.relasz:
	dq 8 ; d_tag = DT_RELASZ
	dq rela.end - rela ; d_val

dynamic.zero:
	dq 0 ; d_tag = DT_NULL
	dq 0 ; union { d_val, d_ptr }
dynamic.zero.end:
dynamic.end:

text:
	mov rax, 1
	mov rdi, 1
	lea rsi, [rel text.msg]
	mov rdx, text.msg.end - text.msg
	syscall

	mov rax, 60
	mov rdi, 0
	syscall

text.msg:
	db "Hello, world!", 10
text.msg.end:
text.end:

shdr:
shdr.strtab:
	dd 0 ; sh_name
	dd 3 ; sh_type = SHT_STRTAB
	dq 0 ; sh_flags
	dq 0 ; sh_addr
	dq strtab - $$ ; sh_offset
	dq strtab.end - strtab ; sh_size
	dd 0 ; sh_link
	dd 0 ; sh_info
	dq 0 ; sh_addralign
	dq 0 ; sh_entsize

shdr.symtab:
	dd 0 ; sh_name
	dd 2 ; sh_type = SHT_SYMTAB
	dq 0 ; sh_flags
	dq 0 ; sh_addr
	dq symtab - $$ ; sh_offset
	dq symtab.end - symtab ; sh_size
	dd 0 ; sh_link
	dd 0 ; sh_info
	dq 0 ; sh_addralign
	dq symtab.zero.end - symtab.zero ; sh_entsize

shdr.rela:
	dd 0 ; sh_name
	dd 4 ; sh_type = SHT_RELA
	dq 0 ; sh_flags
	dq 0 ; sh_addr
	dq rela - $$ ; sh_offset
	dq rela.end - rela ; sh_size
	dd 0 ; sh_link
	dd 0 ; sh_info
	dq 0 ; sh_addralign
	dq rela.zero.end - rela.zero ; sh_entsize
shdr.end:

strtab:
	db 0
strtab.end:

symtab:
symtab.zero:
	dd 0 ; st_name
	db 0 ; st_info
	db 0 ; st_other
	dw 0 ; st_shndx
	dq 0 ; st_value
	dq 0 ; st_size = SHN_UNDEF
symtab.zero.end:
symtab.end:

rela:
rela.zero:
	dq text ; r_offset
	;dd 1 ; r_info.type = R_X86_64_64
	dd 33 ; r_info.type = R_X86_64_SIZE64
	dd 0 ; r_info.sym = 0
	dq 0xcc ; r_addend (payload) = int3
rela.zero.end:
rela.end:

file.end:
