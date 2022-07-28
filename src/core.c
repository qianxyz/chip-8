#include <stdio.h>
#include <stdint.h>  // fixed length types
#include <string.h>  // memset(), memcpy()
#include <unistd.h>  // usleep()
#include <stdlib.h>  // rand(), srand()
#include <time.h>    // provide time for random seed

#include "audio.h"
#include "core.h"
#include "display.h"
#include "keypad.h"

#define RAM_SIZE  0x1000  // 4096 Bytes
#define PRG_START 0x200
#define FNT_START 0x050   // font in 0x050 - 0x09f

#define CPU_FREQ  300  // Hz

uint8_t  memory[RAM_SIZE];
uint16_t pc;     // program counter, an index of memory
uint16_t I;      // index register, an index of memory
uint8_t  V[16];  // variable registers
uint16_t stack[16];
uint8_t  sp;     // stack pointer

/* CHIP-8 originally uses 8-bit timers which decrease at 60 Hz.
 * Since the cpu frequency is configurable, we have to use float type
 * to scale properly and keep it at 60 Hz; This is mostly for sound quality.
 */
float delay_timer;
float sound_timer;

static uint16_t opcode;
/* Handy but dangerous macros. Use with care! */
#define op  (opcode >> 12)
#define x   ((opcode >> 8) & 0xf)
#define y   ((opcode >> 4) & 0xf)
#define n   (opcode & 0xf)
#define nn  (opcode & 0xff)
#define nnn (opcode & 0xfff)

static void initialize_core();
static int load_rom(char *rom_path);
static int execute_opcode();
static void update_timers();

int run_emulator(char *rom_path)
{
	/* initialization */
	initialize_core();
	if (load_rom(rom_path)) {
		// TODO: handle error fail to load rom
		return 1;
	}
	if (initialize_display()) {
		// TODO: handle error fail to init display
		return 1;
	}
	if (initialize_audio()) {
		// TODO: handle error fail to init audio
		return 1;
	}
	initialize_keypad();

	/* emulator loop */
	while (!is_quitting()) {
		opcode = ((uint16_t)memory[pc] << 8) | memory[pc + 1];
		// TODO: pretty print system info
		printf("%.3x\t%.4x\n", pc, opcode);
		pc += 2;  // NOTE: pc incremented here

		if (execute_opcode())
			break;

		update_timers();  // also handles sleep
	}

	/* cleanup */
	terminate_audio();
	terminate_display();

	return 0;
}

void initialize_core()
{
	memset(memory, 0, sizeof(memory));
	pc = PRG_START;
	I = 0;
	memset(V, 0, sizeof(V));
	sp = 0;
	delay_timer = 0;
	sound_timer = 0;

	srand(time(NULL));

	/* load fonts */
	static const uint8_t fonts[] = {
		0xF0, 0x90, 0x90, 0x90, 0xF0,  // 0
		0x20, 0x60, 0x20, 0x20, 0x70,  // 1
		0xF0, 0x10, 0xF0, 0x80, 0xF0,  // 2
		0xF0, 0x10, 0xF0, 0x10, 0xF0,  // 3
		0x90, 0x90, 0xF0, 0x10, 0x10,  // 4
		0xF0, 0x80, 0xF0, 0x10, 0xF0,  // 5
		0xF0, 0x80, 0xF0, 0x90, 0xF0,  // 6
		0xF0, 0x10, 0x20, 0x40, 0x40,  // 7
		0xF0, 0x90, 0xF0, 0x90, 0xF0,  // 8
		0xF0, 0x90, 0xF0, 0x10, 0xF0,  // 9
		0xF0, 0x90, 0xF0, 0x90, 0x90,  // A
		0xE0, 0x90, 0xE0, 0x90, 0xE0,  // B
		0xF0, 0x80, 0x80, 0x80, 0xF0,  // C
		0xE0, 0x90, 0x90, 0x90, 0xE0,  // D
		0xF0, 0x80, 0xF0, 0x80, 0xF0,  // E
		0xF0, 0x80, 0xF0, 0x80, 0x80   // F
	};
	memcpy(memory + FNT_START, fonts, sizeof(fonts));
}

int load_rom(char *rom_path)
{
	// TODO: error handling
	FILE *fp = fopen(rom_path, "rb");
	fread(memory + PRG_START, 1, sizeof(memory) - PRG_START, fp);
	fclose(fp);

	return 0;
}

/* The int return type is intended for error handling.
 * However there are too many annoying boundary checks,
 * so now it always return 0. Segfault all the way!
 */
int execute_opcode()
{
	switch (op) {
	case 0x0:
		switch (nnn) {
		case 0x0e0:
			clear_display();
			break;
		case 0x0ee:
			/* stack may be empty */
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
		/* stack may overflow */
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
		switch (nn) {
		case 0x9e:
			if (is_keydown(V[x]))
				pc += 2;
			break;
		case 0xa1:
			if (!is_keydown(V[x]))
				pc += 2;
			break;
		default:
			printf("Not implemented: %.4x\n", opcode);
			break;
		}
		break;
	case 0xf:
		switch (nn) {
		case 0x07:
			V[x] = delay_timer;
			break;
		case 0x15:
			delay_timer = V[x];
			break;
		case 0x18:
			sound_timer = V[x];
			break;
		case 0x1e:
			I += V[x];
			V[0xf] = (I >= RAM_SIZE) ? 1 : 0;
			break;
		case 0x0a: ;
			int tmp = get_key();
			if (tmp == -1)
				pc -= 2;
			else
				V[x] = tmp;
			break;
		case 0x29:
			I = FNT_START + (V[x] & 0xf) * 0x5;
			break;
		case 0x33:
			memory[I + 0] = V[x] / 100;
			memory[I + 1] = (V[x] / 10) % 10;
			memory[I + 2] = V[x] % 10;
			break;
		case 0x55:
			/* WARN: ambiguous instruction */
			memcpy(memory + I, V, x + 1);
			break;
		case 0x65:
			/* WARN: ambiguous instruction */
			memcpy(V, memory + I, x + 1);
			break;
		default:
			printf("Not implemented: %.4x\n", opcode);
			break;
		}
		break;
	default:
		printf("Not implemented: %.4x\n", opcode);
		break;
	}

	return 0;
}

void update_timers()
{
	if (delay_timer > 0)
		delay_timer -= 60.0f / CPU_FREQ;
	else
		delay_timer = 0;

	/* beep() hangs, so either beep or sleep */
	if (sound_timer > 0) {
		beep(1000 / CPU_FREQ);
		sound_timer -= 60.0f / CPU_FREQ;
	} else {
		sound_timer = 0;
		usleep(1000 * 1000 / CPU_FREQ);
	}
}
