#include "payload.h"
#include "staging.h"

#define VADDR_BASE 0x420000

#include <elf.h>
#include <memory.h>
#include <stddef.h>
#include <stdio.h>
#include <sys/fcntl.h>
#include <sys/stat.h>
#include <sys/unistd.h>

int main(void)
{
	struct __attribute__((packed)) File {
		Elf64_Ehdr ehdr;
		Elf64_Phdr phdr;
		unsigned char code[staging_len + payload_len];
	} file = {0};

	file.ehdr.e_ident[EI_MAG0] = ELFMAG0;
	file.ehdr.e_ident[EI_MAG1] = ELFMAG1;
	file.ehdr.e_ident[EI_MAG2] = ELFMAG2;
	file.ehdr.e_ident[EI_MAG3] = ELFMAG3;
	file.ehdr.e_ident[EI_CLASS] = ELFCLASS64;
	file.ehdr.e_ident[EI_DATA] = ELFDATA2LSB;
	file.ehdr.e_ident[EI_VERSION] = EV_CURRENT;
	file.ehdr.e_ident[EI_OSABI] = ELFOSABI_LINUX;
	file.ehdr.e_ident[EI_ABIVERSION] = 0;
	file.ehdr.e_type = ET_EXEC;
	file.ehdr.e_machine = EM_X86_64;
	file.ehdr.e_version = EV_CURRENT;
	file.ehdr.e_entry = VADDR_BASE + offsetof(struct File, code);
	file.ehdr.e_phoff = offsetof(struct File, phdr);
	file.ehdr.e_shoff = 0;
	file.ehdr.e_flags = 0; // EF_...
	file.ehdr.e_ehsize = sizeof(Elf64_Ehdr);
	file.ehdr.e_phentsize = sizeof(Elf64_Phdr);
	file.ehdr.e_phnum = 1;
	file.ehdr.e_shentsize = sizeof(Elf64_Shdr);
	file.ehdr.e_shnum = 0;
	file.ehdr.e_shstrndx = 0;

	file.phdr.p_type = PT_LOAD;
	file.phdr.p_flags = PF_R | PF_W | PF_X;
	file.phdr.p_offset = offsetof(struct File, code);
	file.phdr.p_vaddr = VADDR_BASE + offsetof(struct File, code);
	file.phdr.p_paddr = VADDR_BASE + offsetof(struct File, code);
	file.phdr.p_filesz = sizeof(file.code);
	file.phdr.p_memsz = sizeof(file.code);
	file.phdr.p_align = 0x1000;

	int out = open("poly", O_RDWR | O_CREAT, S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH);
	struct stat s;
	fstat(out, &s);

	for (unsigned long i = 0; i < payload_len; i++) {
		payload[i] ^= (char)s.st_ino;
	}

	*(unsigned long*)(memmem(staging, staging_len, "iiiiiiii", 8)) = payload_len;

	memcpy(file.code, staging, staging_len);
	memcpy(file.code + staging_len, payload, payload_len);

	write(out, &file, sizeof(file));

	close(out);
}
