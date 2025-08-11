#include "code.h"

#include <elf.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#define PHDR_NUM 4
#define SHDR_NUM 6
#define DYNAMIC_NUM 8
#define DYNSYM_NUM 3
#define RELA_NUM 1

int main()
{
	FILE *f = fopen("reloc.out", "w+");
	if (!f) {
		exit(1);
	}
	fchmod(fileno(f), 0755);

	char *interp = "/lib64/ld-linux-x86-64.so.2";

	int dynstr_len = 7;
	char *dynstr[] = {
		"",
		".dynstr",
		".dynamic",
		".dynsym",
		".rela.dyn",
		".text",
		"_start",
	};
	size_t dynstr_size = 0;
	for (int d = 0; d < dynstr_len; d++) {
		dynstr_size += strlen(dynstr[d]) + 1;
	}

	size_t ehdr_size = sizeof(Elf64_Ehdr);

	size_t phdr_size = PHDR_NUM * sizeof(Elf64_Phdr);
	size_t phdr_offset = ehdr_size;
	size_t interp_offset = phdr_offset + phdr_size;
	size_t interp_size = strlen(interp) + 1;
	size_t dynamic_offset = interp_offset + interp_size;
	size_t dynamic_size = DYNAMIC_NUM * sizeof(Elf64_Dyn);
	size_t code_offset = dynamic_offset + dynamic_size;

	size_t shdr_offset = code_offset + code_len;
	size_t shdr_size = SHDR_NUM * sizeof(Elf64_Shdr);
	size_t dynstr_offset = shdr_offset + shdr_size;
	size_t dynsym_offset = dynstr_offset + dynstr_size;
	size_t dynsym_size = DYNSYM_NUM * sizeof(Elf64_Sym);
	size_t rela_offset = dynsym_offset + dynsym_size;
	size_t rela_size = RELA_NUM * sizeof(Elf64_Rela);

	size_t file_size = rela_offset + rela_size;

	Elf64_Ehdr ehdr = {0};
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
	ehdr.e_entry = code_offset;
	ehdr.e_phoff = phdr_offset;
	ehdr.e_shoff = shdr_offset;
	ehdr.e_flags = 0;
	ehdr.e_ehsize = sizeof(Elf64_Ehdr);
	ehdr.e_phentsize = sizeof(Elf64_Phdr);
	ehdr.e_phnum = PHDR_NUM;
	ehdr.e_shentsize = sizeof(Elf64_Shdr);
	ehdr.e_shnum = SHDR_NUM;
	ehdr.e_shstrndx = 1;

	Elf64_Phdr phphdr = {0};
	phphdr.p_type = PT_PHDR;
	phphdr.p_flags = PF_R;
	phphdr.p_offset = phdr_offset;
	phphdr.p_vaddr = phdr_offset;
	phphdr.p_paddr = phdr_offset;
	phphdr.p_filesz = phdr_size;
	phphdr.p_memsz = phdr_size;
	phphdr.p_align = 0x1000;

	Elf64_Phdr phinterp = {0};
	phinterp.p_type = PT_INTERP;
	phinterp.p_flags = PF_R;
	phinterp.p_offset = interp_offset;
	phinterp.p_vaddr = interp_offset;
	phinterp.p_paddr = interp_offset;
	phinterp.p_filesz = interp_size;
	phinterp.p_memsz = interp_size;
	phinterp.p_align = 1;

	Elf64_Phdr phdynamic = {0};
	phdynamic.p_type = PT_DYNAMIC;
	phdynamic.p_flags = PF_R | PF_W;
	phdynamic.p_offset = dynamic_offset;
	phdynamic.p_vaddr = dynamic_offset;
	phdynamic.p_paddr = dynamic_offset;
	phdynamic.p_filesz = dynamic_size;
	phdynamic.p_memsz = dynamic_size;
	phdynamic.p_align = 1;

	// .text
	Elf64_Phdr phcode = {0};
	phcode.p_type = PT_LOAD;
	phcode.p_flags = PF_R | PF_W | PF_X;
	phcode.p_offset = 0;
	phcode.p_vaddr = 0;
	phcode.p_paddr = 0;
	phcode.p_filesz = file_size;
	phcode.p_memsz = file_size;
	phcode.p_align = 0x1000;

	Elf64_Dyn dynamic[DYNAMIC_NUM] = {
		{ .d_tag = DT_STRTAB, .d_un.d_val = dynstr_offset, },
		{ .d_tag = DT_STRSZ, .d_un.d_val = dynstr_size, },
		{ .d_tag = DT_SYMTAB, .d_un.d_val = dynsym_offset, },
		{ .d_tag = DT_SYMENT, .d_un.d_val = sizeof(Elf64_Sym), },
		{ .d_tag = DT_RELA, .d_un.d_val = rela_offset, },
		{ .d_tag = DT_RELASZ, .d_un.d_val = rela_size, },
		{ .d_tag = DT_RELAENT, .d_un.d_val = sizeof(Elf64_Rela), },
		{ .d_tag = DT_NULL, .d_un.d_val = 0, },
	};

	Elf64_Shdr shnull = {0};

	Elf64_Shdr shdynstr = {0};
	shdynstr.sh_name = 1;
	shdynstr.sh_type = SHT_STRTAB;
	shdynstr.sh_flags = SHF_ALLOC;
	shdynstr.sh_addr = dynstr_offset;
	shdynstr.sh_offset = dynstr_offset;
	shdynstr.sh_size = dynstr_size;
	shdynstr.sh_addralign = 8;

	Elf64_Shdr shdynamic = {0};
	shdynamic.sh_name = 9;
	shdynamic.sh_type = SHT_DYNAMIC;
	shdynamic.sh_flags = SHF_ALLOC;
	shdynamic.sh_addr = dynamic_offset;
	shdynamic.sh_offset = dynamic_offset;
	shdynamic.sh_size = dynamic_size;
	shdynamic.sh_link = 1;
	shdynamic.sh_addralign = 8;

	Elf64_Shdr shdynsym = {0};
	shdynsym.sh_name = 18;
	shdynsym.sh_type = SHT_DYNSYM;
	shdynsym.sh_flags = SHF_ALLOC;
	shdynsym.sh_addr = dynsym_offset;
	shdynsym.sh_offset = dynsym_offset;
	shdynsym.sh_size = dynsym_size;
	shdynsym.sh_link = 1;
	shdynsym.sh_info = 3;
	shdynsym.sh_addralign = 8;
	shdynsym.sh_entsize = sizeof(Elf64_Sym);

	Elf64_Shdr shrela = {0};
	shrela.sh_name = 26;
	shrela.sh_type = SHT_RELA;
	shrela.sh_flags = SHF_ALLOC;
	shrela.sh_addr = rela_offset;
	shrela.sh_offset = rela_offset;
	shrela.sh_size = rela_size;
	shrela.sh_link = 3;
	shrela.sh_addralign = 8;
	shrela.sh_entsize = sizeof(Elf64_Rela);

	Elf64_Shdr shtext = {0};
	shtext.sh_name = 36;
	shtext.sh_type = SHT_PROGBITS;
	shtext.sh_flags = SHF_ALLOC;
	shtext.sh_addr = code_offset;
	shtext.sh_offset = code_offset;
	shtext.sh_size = code_len;
	shtext.sh_link = 1;
	shtext.sh_addralign = 8;

	Elf64_Sym dynsym[DYNSYM_NUM] = {
		{0},
		{ .st_name = 0, .st_info = ELF64_ST_INFO(STB_LOCAL, STT_NOTYPE), .st_value = 0, .st_shndx = SHN_ABS, },
		{ .st_name = 42, .st_info = ELF64_ST_INFO(STB_LOCAL, STT_FUNC), .st_value = code_offset, .st_size = code_len, .st_shndx = 5, },
	};

	// Overwrite first 2 bytes of the code segment
	Elf64_Rela rela[RELA_NUM] = {
		{ .r_offset = code_offset-6, .r_info = ELF64_R_INFO(1, R_X86_64_64), .r_addend = 0x9090909090909090 }
	};

	fwrite(&ehdr, 1, sizeof(ehdr), f);

	fwrite(&phphdr, 1, sizeof(phphdr), f);
	fwrite(&phinterp, 1, sizeof(phinterp), f);
	fwrite(&phdynamic, 1, sizeof(phdynamic), f);
	fwrite(&phcode, 1, sizeof(phcode), f);
	fwrite(interp, 1, strlen(interp) + 1, f);
	fwrite(dynamic, DYNAMIC_NUM, sizeof(Elf64_Dyn), f);
	fwrite(code, 1, code_len, f);

	fwrite(&shnull, 1, sizeof(shnull), f);
	fwrite(&shdynstr, 1, sizeof(shdynstr), f);
	fwrite(&shdynamic, 1, sizeof(shdynamic), f);
	fwrite(&shdynsym, 1, sizeof(shdynsym), f);
	fwrite(&shrela, 1, sizeof(shrela), f);
	fwrite(&shtext, 1, sizeof(shtext), f);

	for (int d = 0; d < dynstr_len; d++) {
		fwrite(dynstr[d], 1, strlen(dynstr[d]) + 1, f);
	}
	fwrite(dynsym, DYNSYM_NUM, sizeof(Elf64_Sym), f);
	fwrite(rela, RELA_NUM, sizeof(Elf64_Rela), f);

	fclose(f);
}
