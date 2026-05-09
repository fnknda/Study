#include "code.h"
#include "evil.h"

#include <elf.h>
#include <memory.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

int makeRela(size_t code_off, bool use_evil, Elf64_Rela **out, size_t *out_num);

int main(int argc, char *argv[])
{
	if (argc != 2) {
		printf("Usage: %s <output>\n", argv[0]);
		exit(1);
	}
	FILE *f = fopen(argv[1], "w+");
	if (!f) {
		exit(1);
	}
	fchmod(fileno(f), 0755);

	Elf64_Ehdr ehdr = {0};
	Elf64_Phdr phdrs[4];
	char *interpreter = "/lib64/ld-linux-x86-64.so.2";

	size_t ehdr_size = sizeof(Elf64_Ehdr);

	size_t phdr_off = ehdr_size;
	size_t phdr_size = sizeof(phdrs);
	size_t code_off = phdr_off + phdr_size;
	size_t interp_off = code_off + code_len;
	size_t interp_size = strlen(interpreter) + 1;

	Elf64_Shdr shdrs[3];
	Elf64_Dyn dynamic[6];
	Elf64_Sym sym[1];
	Elf64_Rela *rela;
	char *strtab[] = {
	    "",
	};

	size_t shdr_off = interp_off + interp_size;
	size_t shdr_size = sizeof(shdrs);
	size_t rela_num;
	makeRela(code_off, true, &rela, &rela_num);
	size_t rela_off = shdr_off + shdr_size;
	size_t rela_size = rela_num * sizeof(Elf64_Rela);
	size_t dynamic_off = rela_off + rela_size;
	size_t dynamic_size = sizeof(dynamic);
	size_t strtab_off = dynamic_off + dynamic_size;
	size_t strtab_size = 0;
	for (int i = 0; i < sizeof(strtab) / sizeof(char *); ++i) {
		strtab_size += strlen(strtab[i]) + 1;
	}
	size_t sym_off = strtab_off + strtab_size;
	size_t sym_size = sizeof(sym);

	size_t file_size = sym_off + sym_size;

	ehdr.e_ident[EI_MAG0] = ELFMAG0;
	ehdr.e_ident[EI_MAG1] = ELFMAG1;
	ehdr.e_ident[EI_MAG2] = ELFMAG2;
	ehdr.e_ident[EI_MAG3] = ELFMAG3;
	ehdr.e_ident[EI_CLASS] = ELFCLASS64;
	ehdr.e_ident[EI_DATA] = ELFDATA2LSB;
	ehdr.e_ident[EI_VERSION] = EV_CURRENT;
	ehdr.e_ident[EI_OSABI] = ELFOSABI_LINUX;
	ehdr.e_ident[EI_ABIVERSION] = EV_CURRENT;
	ehdr.e_type = ET_DYN;
	ehdr.e_machine = EM_X86_64;
	ehdr.e_version = EV_CURRENT;
	ehdr.e_entry = code_off;
	ehdr.e_phoff = phdr_off;
	ehdr.e_shoff = shdr_off;
	ehdr.e_flags = 0;
	ehdr.e_ehsize = sizeof(Elf64_Ehdr);
	ehdr.e_phentsize = sizeof(Elf64_Phdr);
	ehdr.e_phnum = sizeof(phdrs) / sizeof(Elf64_Phdr);
	ehdr.e_shentsize = sizeof(Elf64_Shdr);
	ehdr.e_shnum = sizeof(shdrs) / sizeof(Elf64_Shdr);
	ehdr.e_shstrndx = 3;

	phdrs[0].p_type = PT_PHDR;
	phdrs[0].p_flags = PF_R;
	phdrs[0].p_offset = phdr_off;
	phdrs[0].p_vaddr = phdr_off;
	phdrs[0].p_paddr = phdr_off;
	phdrs[0].p_filesz = phdr_size;
	phdrs[0].p_memsz = phdr_size;
	phdrs[0].p_align = 0x8;

	phdrs[1].p_type = PT_INTERP;
	phdrs[1].p_flags = PF_R;
	phdrs[1].p_offset = interp_off;
	phdrs[1].p_vaddr = interp_off;
	phdrs[1].p_paddr = interp_off;
	phdrs[1].p_filesz = interp_size;
	phdrs[1].p_memsz = interp_size;
	phdrs[1].p_align = 0x1;

	phdrs[2].p_type = PT_LOAD;
	phdrs[2].p_flags = PF_R | PF_W | PF_X;
	phdrs[2].p_offset = 0;
	phdrs[2].p_vaddr = 0;
	phdrs[2].p_paddr = 0;
	phdrs[2].p_filesz = file_size;
	phdrs[2].p_memsz = file_size;
	phdrs[2].p_align = 0x1000;

	phdrs[3].p_type = PT_DYNAMIC;
	phdrs[3].p_flags = PF_R;
	phdrs[3].p_offset = dynamic_off;
	phdrs[3].p_vaddr = dynamic_off;
	phdrs[3].p_paddr = dynamic_off;
	phdrs[3].p_filesz = dynamic_size;
	phdrs[3].p_memsz = dynamic_size;
	phdrs[3].p_align = 0x8;

	memset(&shdrs[0], 0, sizeof(Elf64_Shdr));
	shdrs[0].sh_name = 0;
	shdrs[0].sh_type = SHT_RELA;
	shdrs[0].sh_addr = rela_off;
	shdrs[0].sh_offset = rela_off;
	shdrs[0].sh_size = rela_size;
	shdrs[0].sh_addralign = 8;
	shdrs[0].sh_entsize = sizeof(Elf64_Rela);

	memset(&shdrs[1], 0, sizeof(Elf64_Shdr));
	shdrs[1].sh_name = 0;
	shdrs[1].sh_type = SHT_STRTAB;
	shdrs[1].sh_flags = SHF_ALLOC;
	shdrs[1].sh_addr = strtab_off;
	shdrs[1].sh_offset = strtab_off;
	shdrs[1].sh_size = strtab_size;
	shdrs[1].sh_addralign = 8;

	memset(&shdrs[2], 0, sizeof(Elf64_Shdr));
	shdrs[2].sh_name = 0;
	shdrs[2].sh_type = SHT_DYNSYM;
	shdrs[2].sh_flags = SHF_ALLOC;
	shdrs[2].sh_addr = sym_off;
	shdrs[2].sh_offset = sym_off;
	shdrs[2].sh_size = sym_size;
	shdrs[2].sh_info = 1;
	shdrs[2].sh_addralign = 8;
	shdrs[2].sh_entsize = sizeof(Elf64_Sym);

	dynamic[0].d_tag = DT_RELA;
	dynamic[0].d_un.d_ptr = rela_off;

	dynamic[1].d_tag = DT_RELASZ;
	dynamic[1].d_un.d_val = rela_size;

	dynamic[2].d_tag = DT_RELAENT;
	dynamic[2].d_un.d_val = sizeof(Elf64_Rela);

	dynamic[3].d_tag = DT_STRTAB;
	dynamic[3].d_un.d_ptr = strtab_off;

	dynamic[4].d_tag = DT_SYMTAB;
	dynamic[4].d_un.d_ptr = sym_off;

	dynamic[5].d_tag = DT_NULL;
	dynamic[5].d_un.d_val = 0;

	memset(&sym[0], 0, sizeof(Elf64_Sym));

	fwrite(&ehdr, 1, ehdr_size, f);

	fwrite(phdrs, 1, phdr_size, f);
	fwrite(code, 1, code_len, f);
	fwrite(interpreter, 1, interp_size, f);

	fwrite(shdrs, 1, shdr_size, f);
	fwrite(rela, 1, rela_size, f);
	fwrite(dynamic, 1, dynamic_size, f);
	for (int i = 0; i < sizeof(strtab) / sizeof(char *); ++i) {
		fwrite(strtab[i], 1, strlen(strtab[i]) + 1, f);
	}
	fwrite(sym, 1, sym_size, f);

	free(rela);
	fclose(f);
}

int makeRela(size_t code_off, bool use_evil, Elf64_Rela **out, size_t *out_num)
{
	Elf64_Rela *rela;
	int rela_num;

	if (use_evil) {
		rela_num = evil_len / 8;
		if (evil_len % 8 > 0) {
			rela_num++;
		}

		rela = calloc(rela_num, sizeof(Elf64_Rela));

		size_t bytes_left = evil_len;
		int rela_i = 0;
		for (; bytes_left >= 8; rela_i++) {
			rela[rela_i].r_offset = code_off + rela_i * 8;
			rela[rela_i].r_info = ELF64_R_INFO(0, R_X86_64_SIZE64);
			rela[rela_i].r_addend = *(uint64_t *) (&evil[rela_i * 8]);
			bytes_left -= 8;
		}

		if (bytes_left > 0) {
			rela[rela_i].r_offset = code_off + rela_i * 8 - 8 + bytes_left;
			rela[rela_i].r_info = ELF64_R_INFO(0, R_X86_64_SIZE64);
			rela[rela_i].r_addend = *(uint64_t *) (&evil[rela_i * 8 - 8 + bytes_left]);
		}
	}
	else {
		rela = calloc(1, sizeof(Elf64_Rela));
		rela_num = 1;

		union {
			uint64_t i64;
			uint8_t a8[8];
		} value;
		value.i64 = *(uint64_t *) code;
		value.a8[0] = 0xcc;

		rela[0].r_offset = code_off;
		rela[0].r_info = ELF64_R_INFO(0, R_X86_64_64);
		rela[0].r_addend = value.i64;
	}
	*out = rela;
	*out_num = rela_num;
}
