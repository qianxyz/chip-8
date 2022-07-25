#include <stdio.h>
#include <stdint.h>  // fixed length types
#include <string.h>  // memset()
#include <unistd.h>  // usleep()
#include <stdlib.h>  // rand(), srand()
#include <time.h>    // provide time for random seed

#include "core.h"
#include "display.h"
#include "keypad.h"

#define RAM_SIZE  0x1000  // 4096 Bytes
#define PRG_START 0x200

#define CPU_FREQ  10  // Hz

// TODO: implement timers and keypad
uint8_t  memory[RAM_SIZE];
uint16_t pc;     // program counter, an index of memory
uint16_t I;      // index register, an index of memory
uint8_t  V[16];  // variable registers
uint16_t stack[16];
uint8_t  sp;     // stack pointer

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
static int execute_opcode();

int run_emulator(char *rom_path)
{
	int ret;
	ret = initialize_core();
	ret = initialize_display();
	ret = initialize_keypad();
	ret = load_rom(rom_path);
	// TODO: error handling from ret

	while (!is_quitting()) {
		opcode = ((uint16_t)memory[pc] << 8) | memory[pc + 1];
		pc += 2;  // NOTE: pc incremented here

		if (execute_opcode())  // may fail due to stack flow
			break;

		usleep(1000 * 1000 / CPU_FREQ);
	}

	terminate_display();

	return 0;
}

int initialize_core()
{
	memset(memory, 0, sizeof(memory));
	pc = PRG_START;
	I = 0;
	memset(V, 0, sizeof(V));
	sp = 0;

	srand(time(NULL));

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

int execute_opcode()
{
	switch (op) {
	case 0x0:
		switch (nnn) {
		case 0x0e0:
			clear_display();
			break;
		case 0x0ee:
			if (sp == 0) {
				// TODO: print error, empty stack
				return 1;
			}
			pc = stack[--sp];
			break;
		default:
			printf("Not implemented: %.4x\n", opcode);
			break;
		}
		break;
	case 0x1:
		pc = nnn;
		break;
	case 0x2:
		if (sp > 0xf) {
			// TODO: print error, stack overflow
			return 1;
		}
		stack[sp++] = pc;
		pc = nnn;
		break;
	case 0x3:
		if (V[x] == nn)
			pc += 2;
		break;
	case 0x4:
		if (V[x] != nn)
			pc += 2;
		break;
	case 0x5:
		if (n == 0) {
			if (V[x] == V[y])
				pc += 2;
		} else {
			printf("Not implemented: %.4x\n", opcode);
		}
		break;
	case 0x6:
		V[x] = nn;
		break;
	case 0x7:
		V[x] += nn;
		break;
	case 0x8:
		switch (n) {
		case 0x0:
			V[x] = V[y];
			break;
		case 0x1:
			V[x] |= V[y];
			break;
		case 0x2:
			V[x] &= V[y];
			break;
		case 0x3:
			V[x] ^= V[y];
			break;
		case 0x4:
			V[x] += V[y];
			V[0xf] = (V[x] < V[y]) ? 1 : 0;
			break;
		case 0x5:
			V[0xf] = (V[x] > V[y]) ? 1 : 0;
			V[x] = V[x] - V[y];
			break;
		case 0x6:
			/* WARN: ambiguous instruction */
			V[0xf] = V[x] & 1;
			V[x] >>= 1;
			break;
		case 0x7:
			V[0xf] = (V[y] > V[x]) ? 1 : 0;
			V[x] = V[y] - V[x];
			break;
		case 0xe:
			/* WARN: ambiguous instruction */
			V[0xf] = (V[x] >> 7) & 1;
			V[x] <<= 1;
			break;
		default:
			printf("Not implemented: %.4x\n", opcode);
			break;
		}
		break;
	case 0x9:
		if (n == 0) {
			if (V[x] != V[y])
				pc += 2;
		} else {
			printf("Not implemented: %.4x\n", opcode);
		}
		break;
	case 0xa:
		I = nnn;
		break;
	case 0xb:
		/* WARN: ambiguous instruction */
		pc = nnn + V[0x0];
		break;
	case 0xc:
		V[x] = nn & (uint8_t)rand();
		break;
	case 0xd:
		V[0xf] = draw_sprite(V[x], V[y], memory + I, n);
		break;
	case 0xe:
	case 0xf:
	default:
		printf("Not implemented: %.4x\n", opcode);
		break;
	}

	return 0;
}
