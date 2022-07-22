#include <stdint.h>  // fixed length types
#include <string.h>  // memset()

#include <SDL2/SDL.h>

#include "display.h"

#define PIXEL_SIZE    10
#define WINDOW_WIDTH  (PIXEL_SIZE * WIDTH)
#define WINDOW_HEIGHT (PIXEL_SIZE * HEIGHT)

uint64_t display[HEIGHT];

static SDL_Window   *window;
static SDL_Renderer *renderer;

int initialize_display()
{
	// TODO: serious error handling
	SDL_Init(SDL_INIT_VIDEO);
	SDL_CreateWindowAndRenderer(
			WINDOW_WIDTH, WINDOW_HEIGHT, 0, &window, &renderer);
	return 0;
}

void clear_display()
{
	memset(display, 0, sizeof(display));
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
	SDL_RenderClear(renderer);
	SDL_RenderPresent(renderer);
}

void refresh_display()
{
	SDL_Rect rect;
	for (int row = 0; row < HEIGHT; row++) {
		for (int col = 0; col < WIDTH; col++) {
			if ((display[row] >> (WIDTH - 1 - col)) & 0x1)
				SDL_SetRenderDrawColor(
						renderer, 255, 255, 255, 255);
			else
				SDL_SetRenderDrawColor(
						renderer, 0, 0, 0, 255);
			rect.x = PIXEL_SIZE * col;
			rect.y = PIXEL_SIZE * row;
			rect.w = PIXEL_SIZE;
			rect.h = PIXEL_SIZE;
			SDL_RenderFillRect(renderer, &rect);
		}
	}
	SDL_RenderPresent(renderer);
}
