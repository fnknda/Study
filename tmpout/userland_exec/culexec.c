#include <alloca.h>
#include <elf.h>
#include <fcntl.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

static void(*entrypoint)(void) = NULL;

void unmap(char *org_path) {
	printf("Unmapping\n");

	FILE *maps = fopen("/proc/self/maps", "r");
	if (maps == NULL)
		exit(1);

	char *line = NULL;
	size_t linesz = 0;
	while (getline(&line, &linesz, maps) != -1) {
		uint64_t start, end, offset;
		char path[PATH_MAX];
		char flags[4];
		char *dev = NULL, *inode = NULL;
		int fields_num = sscanf(line, "%llx-%llx %4s %llx %ms %ms %s", &start, &end, flags, &offset, &dev, &inode, path);
		free(dev);
		free(inode);

		if (fields_num == 7 && strstr(path, org_path) != NULL) {
			printf("Unmapping %llx-%llx %s\n", start, end, path);
			munmap((void *)start, end - start);
		}

		free(line);
		line = NULL;
		linesz = 0;
	}
	free(line);
	line = NULL;

	printf("Unmapped\n");
}

void load(char *args[]) {
	printf("Loading %s\n", args[0]);

	int fd = open(args[0], O_RDONLY);
	if (fd == -1)
		exit(1);

	Elf64_Ehdr ehdr;
	read(fd, &ehdr, sizeof(ehdr));
	entrypoint = (void(*)(void))ehdr.e_entry;
	lseek(fd, ehdr.e_phoff, SEEK_SET);

	for (int pidx = 0; pidx < ehdr.e_phnum; pidx++) {
		Elf64_Phdr phdr;
		read(fd, &phdr, sizeof(phdr));
		if (phdr.p_type == PT_LOAD) {
			int prot = (phdr.p_flags & PF_R ? PROT_READ : 0) | (phdr.p_flags & PF_W ? PROT_WRITE : 0) | (phdr.p_flags & PF_X ? PROT_EXEC : 0);
			void *addr = (void *)(phdr.p_vaddr & (~0 ^ 0xff));
			size_t delta = phdr.p_vaddr - (uint64_t)addr;
			printf("  Loading %llx from %p at %p\n", phdr.p_memsz + delta, phdr.p_offset - delta, addr);
			void *map = mmap(addr, phdr.p_memsz + delta, prot, MAP_PRIVATE, fd, phdr.p_offset - delta);
			if (map == MAP_FAILED || map != addr) {
				printf("  Failed to map %p -> %p\n", phdr.p_vaddr, map);
			}
		}
	}

	close(fd);
	printf("Loaded\n");
}

void set_env_and_exec(char *args[], char *env[]) {
	printf("Setting environment\n");

	void *stack = alloca(1024*1024); // 1M
	int *auxv_argc = stack;
	void *aux = stack + sizeof(int);

	char **auxv_args = aux;
	int argn;
	for (argn = 0; args[argn] != NULL; argn++) {
		aux += sizeof(long);
	}
	aux += sizeof(long);

	*auxv_argc = argn;

	char **auxv_env = aux;
	for (int envn = 0; env[envn] != NULL; envn++) {
		aux += sizeof(long);
	}
	aux += sizeof(long);

	Elf64_auxv_t *auxv = aux;
	auxv[0].a_type = AT_ENTRY;
	auxv[0].a_un.a_ptr = entrypoint;
	auxv[1].a_type = AT_NULL;
	auxv[1].a_un.a_val = 0;
	aux += sizeof(Elf64_auxv_t) * 2;

	for (int argn = 0; args[argn] != NULL; argn++) {
		aux += sizeof(long);
	}
	aux += sizeof(long);

	*auxv_argc = argn;

	char **auxv_env = aux;
	for (int envn = 0; env[envn] != NULL; envn++) {
		aux += sizeof(long);
	}

	printf("Environment set, executing\n");
	entrypoint();
}

[[noreturn]] void ulexec(char *org_path, char *args[], char *env[]) {
	unmap(org_path);
	load(args);
	set_env_and_exec(args, env);

	//exit(0);
	asm("mov $60, %rax\n"
	    "xor %rdi, %rdi\n"
	    "syscall");
}
