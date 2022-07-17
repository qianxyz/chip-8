#include <stdio.h>
#include <stdint.h>
#include <unistd.h>

#define PRG_START 0x200
#define RAM_END   0xFFF

uint8_t  memory[RAM_END + 1];
uint16_t pc;         // program counter, an index of memory
uint16_t reg_I;      // index register, an index of memory
uint8_t  reg_V[16];  // variable registers

// TODO: implement the following
//uint16_t stack[16];
//uint8_t  sp;  // stack pointer
//uint8_t  display[64 * 32];
//uint8_t  delay_timer;
//uint8_t  sound_timer;
//uint8_t  key[16];

static uint16_t opcode;
#define op  (opcode >> 12)
#define x   ((opcode >> 8) & 0xF)
#define y   ((opcode >> 4) & 0xF)
#define n   (opcode & 0xF)
#define nn  (opcode & 0xFF)
#define nnn (opcode & 0xFFF)

void initialize();
void load_rom(char *);
void fetch_opcode();
void execute();

int main(int argc, char *argv[])
{
	initialize();

	// TODO: actual arg parsing
	load_rom(argv[1]);

	for (;;) {
		fetch_opcode();

		printf("%.3x\t%.4x\n", pc, opcode);
		printf("\t%x\t%x\t%x\t%x\t%.2x\t%.3x\n",
		       op, x, y, n, nn, nnn);

		execute();

		sleep(1);
	}
}

void load_rom(char *rom)
{
	// TODO: error handling
	FILE *fp = fopen(rom, "rb");
	fread(memory + PRG_START, 1, RAM_END - PRG_START, fp);
	fclose(fp);
}

void initialize()
{
	// TODO: load font
	pc = 0x200;
}

void fetch_opcode()
{
	opcode = ((uint16_t)memory[pc] << 8) | memory[pc + 1];
	pc += 2;
}

void execute()
{
	switch (op) {
	case 0x0:
		if (nnn == 0x0e0)
			printf("\tClear display\n");
		else
			printf("\tNot implemented\n");
		break;
	case 0x1:
		pc = nnn;
		printf("\tJump to %.3x\n", nnn);
		break;
	case 0x6:
		reg_V[x] = nn;
		printf("\tSet V%x to %.2x\n", x, nn);
		break;
	case 0x7:
		reg_V[x] += nn;
		printf("\tAdd %.2x to V%x\n", nn, x);
		break;
	case 0xa:
		reg_I = nnn;
		printf("\tSet I to %.3x\n", nnn);
		break;
	case 0xd:
		printf("\tDisplay\n");
		break;
	default:
		printf("\tNot implemented\n");
		break;
	}
}
