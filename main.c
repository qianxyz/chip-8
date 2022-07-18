#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>

#define PRG_START 0x200
#define RAM_END   0xFFF

#define CPU_CLOCK 100  // ms

uint8_t  memory[RAM_END + 1];
uint16_t pc;         // program counter, an index of memory
uint16_t reg_I;      // index register, an index of memory
uint8_t  reg_V[16];  // variable registers

uint64_t display[32];

// TODO: implement the following
//uint16_t stack[16];
//uint8_t  sp;  // stack pointer
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

void draw_sprite();
void refresh();

int main(int argc, char *argv[])
{
	initialize();

	// TODO: actual arg parsing
	load_rom(argv[1]);

	for (;;) {
		fetch_opcode();

		execute();

		usleep(CPU_CLOCK * 1000);
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
	reg_I = 0;
	memset(reg_V, 0, sizeof(reg_V));
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
		switch (nnn) {
		case 0x0e0:
			memset(display, 0, sizeof(display));
			refresh();
			break;
		default:
			printf("%.4x not implemented\n", opcode);
			break;
		}
		break;
	case 0x1:
		pc = nnn;
		break;
	case 0x6:
		reg_V[x] = nn;
		break;
	case 0x7:
		reg_V[x] += nn;
		break;
	case 0xa:
		reg_I = nnn;
		break;
	case 0xd:
		draw_sprite();
		refresh();
		break;
	default:
		printf("%.4x not implemented\n", opcode);
		break;
	}
}

void draw_sprite()
{
	uint8_t col = reg_V[x] & 63;
	uint8_t row = reg_V[y] & 31;
	reg_V[0xf] = 0;
	for (int i = 0; i < n; i++){
		uint64_t sprite = (uint64_t)memory[reg_I + i];
		sprite = (col < 56) ? (sprite << (56 - col))
			            : (sprite >> (col - 56));
		if (display[row] & sprite)
			reg_V[0xf] = 1;
		display[row] ^= sprite;
		row++;
		if (row > 31)
			break;
	}
}

void refresh()
{
	static char *table = " _^C";
	for (int row = 0; row < 32; row += 2) {
		for (int col = 63; col >= 0; col--) {
			uint8_t t = (display[row] >> col) & 0x1;
			uint8_t b = (display[row + 1] >> col) & 0x1;
			putchar(table[(t << 1) | b]);
		}
		putchar('\n');
	}
}
