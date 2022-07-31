#include <stdio.h>
#include <stdlib.h>  // atoi
#include <unistd.h>  // getopt

#include "core.h"

static const char *help =
	"Usage: chip8 [options] <path/to/rom>\n"
	" -f <freq>  Set cpu frequency in Hz (0 for step mode)\n"
	" -h         Display this help\n"
	" -o         Use original COSMAC VIP behavior\n"
	" -v         Show more debug messages\n";

int main(int argc, char *argv[])
{
	int freq, original, verbose;
	char *rom_path;

	/* default */
	freq = 300;  // Hz
	original = 0;  // use modern behavior
	verbose = 0;

	int opt;
	while ((opt = getopt(argc, argv, "f:ovh")) != -1) {
		switch (opt) {
		case 'f':
			freq = atoi(optarg);
			if (freq < 0) {
				fprintf(stderr, "%s: invalid frequency %d\n",
						argv[0], freq);
				fprintf(stderr, help);
				return 1;
			}
			break;
		case 'o':
			original = 1;
			break;
		case 'v':
			verbose = 1;
			break;
		case 'h':
			fprintf(stdout, help);
			return 0;
		default:
			fprintf(stderr, help);
			return 1;
		}
	}

	if (optind != argc - 1) {
		fprintf(stderr, (optind > argc - 1) ?
				"%s: expect argument after options\n" :
				"%s: too many arguments\n",
				argv[0]);
		fprintf(stderr, help);
		return 1;
	}

	rom_path = argv[optind];

	run_emulator(rom_path, freq, original, verbose);

	return 0;
}
