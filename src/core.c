#include <stdio.h>
#include <stdint.h>  // fixed length types
#include <string.h>  // memset(), memcpy()
#include <unistd.h>  // usleep()
#include <stdlib.h>  // rand(), srand()
#include <time.h>    // provide time for random seed

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

#include "audio.h"
#include "core.h"
#include "display.h"
#include "keypad.h"
#include "args.h"

#define RAM_SIZE  0x1000  // 4096 Bytes
#define PRG_START 0x200
#define FNT_START 0x050   // font in 0x050 - 0x09f

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
static int load_rom();
static int execute_opcode();
static void update_timers();

static void main_loop()
{
	if (is_quitting()) {
		/* cleanup */
		terminate_audio();
		terminate_display();
#ifdef __EMSCRIPTEN__
		emscripten_cancel_main_loop();
#else
		exit(0);
#endif /* ifdef __EMSCRIPTEN__ */
	}

	opcode = ((uint16_t)memory[pc] << 8) | memory[pc + 1];
	if (verbose) {
		// TODO: print more system info
		printf("[INFO] pc: %.3x, opcode: %.4x\n", pc, opcode);
	}
	pc += 2;  // NOTE: pc incremented here

	if (execute_opcode()) {
		exit(1);
	};

	update_timers();
}

void run_emulator()
{
	/* initialization */
	initialize_core();
#ifdef __EMSCRIPTEN__
	/* Path is not specified, write opcode 0x1200 to memory 0x200
	 * so that the emulator loops indefinitely. */
	memory[PRG_START] = 0x12;
	memory[PRG_START + 1] = 0x00;
#else
	if (load_rom()) {
		printf("[ERROR] failed to load rom\n");
		exit(1);
	}
#endif /* ifdef __EMSCRIPTEN__ */
	if (initialize_display()) {
		printf("[ERROR] failed to initialize display\n");
		exit(1);
	}
	if (initialize_audio()) {
		printf("[ERROR] failed to initialize audio\n");
		exit(1);
	}
	initialize_keypad();

#ifdef __EMSCRIPTEN__
	emscripten_set_main_loop(main_loop, freq, 1);
#else
	while (1) {
		main_loop();

		if (freq > 0)
			usleep(1000 * 1000 / freq);
	}
#endif /* ifdef __EMSCRIPTEN__ */
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

#ifdef __EMSCRIPTEN__
int reload_rom(char *path)
{
	rom_path = path;
	initialize_core(); // reset VM state
	clear_display();
	return load_rom();
}
#endif /* ifdef __EMSCRIPTEN__ */

int load_rom()
{
	FILE *fp = fopen(rom_path, "rb");
	if (!fp) {
		printf("[ERROR] cannot open file %s\n", rom_path);
		return 1;
	}
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
			printf("[WARNING] unknown opcode: %.4x\n", opcode);
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
			printf("[WARNING] unknown opcode: %.4x\n", opcode);
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
			if (original)
				V[x] = V[y];
			V[0xf] = V[x] & 1;
			V[x] >>= 1;
			break;
		case 0x7:
			V[0xf] = (V[y] > V[x]) ? 1 : 0;
			V[x] = V[y] - V[x];
			break;
		case 0xe:
			/* WARN: ambiguous instruction */
			if (original)
				V[x] = V[y];
			V[0xf] = (V[x] >> 7) & 1;
			V[x] <<= 1;
			break;
		default:
			printf("[WARNING] unknown opcode: %.4x\n", opcode);
			break;
		}
		break;
	case 0x9:
		if (n == 0) {
			if (V[x] != V[y])
				pc += 2;
		} else {
			printf("[WARNING] unknown opcode: %.4x\n", opcode);
		}
		break;
	case 0xa:
		I = nnn;
		break;
	case 0xb:
		/* WARN: ambiguous instruction */
		if (original)
			pc = nnn + V[0x0];
		else
			pc = nnn + V[x];
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
			printf("[WARNING] unknown opcode: %.4x\n", opcode);
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
			open_audio();
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
			if (original)
				I += x + 1;
			break;
		case 0x65:
			/* WARN: ambiguous instruction */
			memcpy(V, memory + I, x + 1);
			if (original)
				I += x + 1;
			break;
		default:
			printf("[WARNING] unknown opcode: %.4x\n", opcode);
			break;
		}
		break;
	default:
		printf("[WARNING] unknown opcode: %.4x\n", opcode);
		break;
	}

	return 0;
}

void update_timers()
{
	/* timers do NOT work at step mode (freq = 0) */
	if (freq == 0) {
		delay_timer = 0;
		sound_timer = 0;
	}

	if (delay_timer > 0)
		delay_timer -= 60.0f / freq;
	if (delay_timer <= 0)
		delay_timer = 0;

	if (sound_timer > 0)
		/* The sound timer is supposed to decrease by 60 every second
		 * but sometimes the interval is too narrow for SDL to react,
		 * resulting in no sound. Here we essentially double the length
		 * of beeps to make beeps more noticeable. */
		sound_timer -= 30.0f / freq;
	if (sound_timer <= 0) {
		close_audio();
		sound_timer = 0;
	}
}
