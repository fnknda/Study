#define _GNU_SOURCE

#include <dlfcn.h>
#include <fcntl.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

char *get_leaf(char *path)
{
	return strrchr(path, '/');
}

int main(int argc, char *argv[], char *env[])
{
	void *lib = dlopen("./libculexec.so", RTLD_NOW); //FIXME: Remove hard coded path
	if (!lib) {
		fprintf(stderr, "dlopen(): %s\n", dlerror());
		exit(1);
	}

	void(*lib_ulexec)(char*, char**, char**) = (void(*)(char*, char**, char**))dlsym(lib, "ulexec");
	if (!lib_ulexec) {
		fprintf(stderr, "dlsym(): %s\n", dlerror());
		exit(1);
	}

	lib_ulexec(get_leaf(argv[0]), &argv[1], env);
}
