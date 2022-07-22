#include <stdio.h>

#include "core.h"  // run_emulator

int main(int argc, char *argv[])
{
	if (argc != 2) {
		printf("Usage: ./chip8 <path/to/rom>\n");
		return 1;
	}
	char *rom_path = argv[1];

	run_emulator(rom_path);

	return 0;
}
