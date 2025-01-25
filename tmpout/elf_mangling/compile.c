#include <elf.h>
#include <fcntl.h>
#include <memory.h>
#include <sys/stat.h>
#include <unistd.h>

#include "code.c"

//#define CODE_OFFSET EI_PAD
#define CODE_OFFSET (sizeof(Elf64_Ehdr) - 8 + sizeof(Elf64_Phdr))
#define CODE_VADDR (0x10000 + CODE_OFFSET)

char *target = "code";

int main(void)
{
	int fd = open(target, O_WRONLY | O_CREAT | O_TRUNC);
	fchmod(fd, 0755);

	Elf64_Ehdr ehdr;
	memset(&ehdr, 0, sizeof(ehdr));

	ehdr.e_ident[EI_MAG0] = ELFMAG0;
	ehdr.e_ident[EI_MAG1] = ELFMAG1;
	ehdr.e_ident[EI_MAG2] = ELFMAG2;
	ehdr.e_ident[EI_MAG3] = ELFMAG3;
	ehdr.e_ident[EI_CLASS] = 0x58;
	ehdr.e_ident[EI_DATA] = 0x58;
	ehdr.e_ident[EI_VERSION] = 0x58;
	ehdr.e_ident[EI_OSABI] = 0x58;
	ehdr.e_ident[EI_ABIVERSION] = 0x58;

	//memcpy(&ehdr.e_ident[EI_PAD], code, code_len);

	ehdr.e_type = ET_EXEC;
	ehdr.e_machine = EM_X86_64;
	ehdr.e_version = 0x58585858;
	ehdr.e_entry = CODE_VADDR;
	ehdr.e_phoff = sizeof(Elf64_Ehdr) - 8;
	ehdr.e_shoff = 0x5858585858585858;
	ehdr.e_flags = 0x58585858;
	ehdr.e_ehsize = sizeof(Elf64_Ehdr);
	ehdr.e_phentsize = sizeof(Elf64_Phdr);
	ehdr.e_phnum = 1;
	ehdr.e_shentsize = 0x5858;
	ehdr.e_shnum = 0x5858;
	ehdr.e_shstrndx = 0x5858;


	Elf64_Phdr phdr;
	memset(&phdr, 0, sizeof(phdr));

	phdr.p_type = PT_LOAD;
	phdr.p_offset = CODE_OFFSET;
	phdr.p_vaddr = CODE_VADDR;
	phdr.p_paddr = 0x5858585858585858;
	phdr.p_filesz = code_len;
	phdr.p_memsz = code_len;
	phdr.p_flags = PF_R | PF_X;
	phdr.p_align = 0x5858585858585858;


	write(fd, &ehdr, sizeof(ehdr) - 8);
	write(fd, &phdr, sizeof(phdr));
	write(fd, code, code_len);

	close(fd);
}
