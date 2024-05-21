#include <stdio.h>
#include <stdlib.h>  // atoi
#include <unistd.h>  // getopt

#include "core.h"
#include "args.h"

int freq = 300;  // Hz
int original = 0;  // use modern behavior
int verbose = 0;
char *rom_path = NULL;

static const char *help =
	"Usage: chip8 [options] <path/to/rom>\n"
	" -f <freq>  Set cpu frequency in Hz (0 for step mode)\n"
	" -h         Display this help\n"
	" -o         Use original COSMAC VIP behavior\n"
	" -v         Show more debug messages\n";

int main(int argc, char *argv[])
{
#ifndef __EMSCRIPTEN__
	int opt;
	while ((opt = getopt(argc, argv, "f:ovh")) != -1) {
		switch (opt) {
		case 'f':
			freq = atoi(optarg);
			if (freq < 0) {
				fprintf(stderr, "%s: invalid frequency %d\n",
						argv[0], freq);
				fprintf(stderr, "%s", help);
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
			fprintf(stdout, "%s", help);
			return 0;
		default:
			fprintf(stderr, "%s", help);
			return 1;
		}
	}

	if (optind != argc - 1) {
		fprintf(stderr, (optind > argc - 1) ?
				"%s: expect argument after options\n" :
				"%s: too many arguments\n",
				argv[0]);
		fprintf(stderr, "%s", help);
		return 1;
	}

	rom_path = argv[optind];
#endif /* ifndef __EMSCRIPTEN__ */

	run_emulator();

	return 0;
}
