#include <stdint.h>  // fixed length types
#include <string.h>  // memset()

#include <SDL2/SDL.h>

#include "display.h"

#define WIDTH  64
#define HEIGHT 32

#define PIXEL_SIZE    10
#define WINDOW_WIDTH  (PIXEL_SIZE * WIDTH)
#define WINDOW_HEIGHT (PIXEL_SIZE * HEIGHT)

static uint64_t display[HEIGHT];

static SDL_Window   *window;
static SDL_Renderer *renderer;

#define off() SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0xff)
#define on()  SDL_SetRenderDrawColor(renderer, 0xff, 0xff, 0xff, 0xff)

int initialize_display()
{
	if (SDL_Init(SDL_INIT_VIDEO) != 0) {
		printf("[ERROR] cannot initialize SDL: %s\n", SDL_GetError());
		return 1;
	}
	if (SDL_CreateWindowAndRenderer(
			WINDOW_WIDTH, WINDOW_HEIGHT, 0, &window, &renderer)) {
		printf("[ERROR] cannot create window and renderer: %s\n",
				SDL_GetError());
		return 1;
	}

	clear_display();
	return 0;
}

void clear_display()
{
	memset(display, 0, sizeof(display));
	off();
	SDL_RenderClear(renderer);
	SDL_RenderPresent(renderer);
}

uint8_t draw_sprite(uint8_t col, uint8_t row, uint8_t *psprite, uint16_t n)
{
	uint8_t collision = 0;
	uint64_t sprite;
	SDL_Rect rect;

	row &= HEIGHT - 1;
	col &= WIDTH - 1;
	while (n-- > 0) {
		sprite = *psprite++;
		if (col < WIDTH - 8)
			sprite <<= (WIDTH - 8) - col;
		else
			sprite >>= col - (WIDTH - 8);
		if (display[row] & sprite)
			collision = 1;
		display[row] ^= sprite;

		if (++row >= HEIGHT)
			break;
	}

	/* Redraw the whole frame */
	off();
	SDL_RenderClear(renderer);
	for (row = 0; row < HEIGHT; row++) {
		for (col = 0; col < WIDTH; col++) {
			((display[row] >> (WIDTH - 1 - col)) & 1) ? on() : off();
			rect.x = PIXEL_SIZE * col;
			rect.y = PIXEL_SIZE * row;
			rect.w = PIXEL_SIZE;
			rect.h = PIXEL_SIZE;
			SDL_RenderFillRect(renderer, &rect);
		}
	}

	SDL_RenderPresent(renderer);
	return collision;
}

void terminate_display()
{
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);

	SDL_Quit();
}
