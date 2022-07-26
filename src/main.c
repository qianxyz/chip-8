#include <stdio.h>

#include "core.h"

/* TODO: config support
 * 1. path/to/rom
 * 2. cpu frequency
 * 3. modern (CHIP-48/SCHIP) or original (COSMAC VIP)
 * 4. verbosity
 */
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
