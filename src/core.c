#include <stdio.h>
#include <stdint.h>  // fixed length types
#include <string.h>  // memset()
#include <unistd.h>  // usleep

#include "core.h"
#include "display.h"

#define RAM_SIZE  0x1000  // 4096 Bytes
#define PRG_START 0x200

#define CPU_FREQ  10  // Hz

// TODO: implement stack, timers and keypad
static uint8_t  memory[RAM_SIZE];
static uint16_t pc;     // program counter, an index of memory
static uint16_t I;      // index register, an index of memory
static uint8_t  V[16];  // variable registers

static uint16_t opcode;
/* Handy but dangerous macros. Use with care! */
#define op  (opcode >> 12)
#define x   ((opcode >> 8) & 0xf)
#define y   ((opcode >> 4) & 0xf)
#define n   (opcode & 0xf)
#define nn  (opcode & 0xff)
#define nnn (opcode & 0xfff)

static int initialize_core();
static int load_rom(char *rom_path);
static void fetch_opcode();
static void execute_opcode();

static void draw_sprite();

int run_emulator(char *rom_path)
{
	int ret;
	ret = initialize_core();
	ret = initialize_display();
	ret = load_rom(rom_path);
	// TODO: error handling from ret

	for (;;) {
		fetch_opcode();
		execute_opcode();

		if (0)  // TODO: some quit flag
			break;

		usleep(1000 * 1000 / CPU_FREQ);
	}

	return 0;
}

int initialize_core()
{
	pc = PRG_START;
	I = 0;
	memset(V, 0, sizeof(V));

	return 0;
}

int load_rom(char *rom_path)
{
	// TODO: error handling
	FILE *fp = fopen(rom_path, "rb");
	fread(memory + PRG_START, 1, sizeof(memory) - PRG_START, fp);
	fclose(fp);

	return 0;
}

void fetch_opcode()
{
	opcode = ((uint16_t)memory[pc] << 8) | memory[pc + 1];
	pc += 2;  // NOTE: pc incremented here
}

void execute_opcode()
{
	switch (op) {
	case 0x0:
		switch (nnn) {
		case 0x0e0:
			clear_display();
			break;
		default:
			printf("Not implemented: %.4x\n", opcode);
			break;
		}
		break;
	case 0x1:
		pc = nnn;
		break;
	case 0x6:
		V[x] = nn;
		break;
	case 0x7:
		V[x] += nn;
		break;
	case 0xa:
		I = nnn;
		break;
	case 0xd:
		draw_sprite();
		break;
	default:
		printf("Not implemented: %.4x\n", opcode);
		break;
	}
}

void draw_sprite()
{
	uint8_t col = V[x] & (WIDTH - 1);
	uint8_t row = V[y] & (HEIGHT - 1);
	V[0xf] = 0;
	for (int i = 0; i < n; i++){
		uint64_t sprite = (uint64_t)memory[I + i];
		if (col < WIDTH - 8)
			sprite <<= (WIDTH - 8) - col;
		else
			sprite >>= col - (WIDTH - 8);
		if (display[row] & sprite)
			V[0xf] = 1;
		display[row] ^= sprite;
		row++;
		if (row > HEIGHT - 1)
			break;
	}
	refresh_display();
}
