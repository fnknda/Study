#include <stdio.h>

int main(int argc, char* arg[], char* env[])
{
	printf("argc: %d\n", argc);
	printf("argn:\n");
	for (int argn = 0; arg[argn] != NULL; argn++)
		printf("  %s\n", arg[argn]);
	printf("env:\n");
	for (int envn = 0; env[envn] != NULL; envn++)
		printf("  %s\n", env[envn]);
}
