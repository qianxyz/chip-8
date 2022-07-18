#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>

#include <SDL2/SDL.h>

#define PRG_START 0x200
#define RAM_END   0xFFF

#define WINDOW_WIDTH  640
#define WINDOW_HEIGHT 320

#define CPU_CLOCK 100  // ms

uint8_t  memory[RAM_END + 1];
uint16_t pc;	     // program counter, an index of memory
uint16_t reg_I;      // index register, an index of memory
uint8_t  reg_V[16];  // variable registers

uint64_t     display[32];
SDL_Window   *window;
SDL_Renderer *renderer;

// TODO: implement the following
//uint16_t stack[16];
//uint8_t  sp;  // stack pointer
//uint8_t  delay_timer;
//uint8_t  sound_timer;
//uint8_t  key[16];

uint16_t opcode;
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
	fread(memory + PRG_START, 1, sizeof(memory) - PRG_START, fp);
	fclose(fp);
}

void initialize()
{
	// TODO: load font
	pc = 0x200;
	reg_I = 0;
	memset(reg_V, 0, sizeof(reg_V));

	SDL_Init(SDL_INIT_VIDEO);
	SDL_CreateWindowAndRenderer(WINDOW_WIDTH, WINDOW_HEIGHT,
			0, &window, &renderer);
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
	SDL_RenderClear(renderer);
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
	SDL_Rect rect;
	for (int row = 0; row < 32; row++) {
		for (int col = 0; col < 64; col++) {
			int rgb = ((display[row] >> (63 - col)) & 0x1) ?
				255 : 0;
			SDL_SetRenderDrawColor(renderer, rgb, rgb, rgb, 255);
			rect = (SDL_Rect){10 * col, 10 * row, 10, 10};
			SDL_RenderFillRect(renderer, &rect);
		}
	}
	SDL_RenderPresent(renderer);
}
