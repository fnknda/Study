#include <stdio.h>

int main(int argc, char **argv)
{
	if (argc != 2) {
		printf("Usage: %s <bin_filename>\n", argv[0]);
		return 1;
	}

	char *bin_filename = argv[1];
	FILE* bin = fopen(bin_filename, "r");
	if (!bin) {
		return 1;
	}

	fclose(bin);
}
