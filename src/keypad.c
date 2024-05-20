#include <stdint.h>

#include <SDL2/SDL.h>

#include "keypad.h"
#include "args.h"

static const Uint8 *keystate;

/* 1 2 3 C    1 2 3 4
 * 4 5 6 D    Q W E R
 * 7 8 9 E -> A S D F
 * A 0 B F    Z X C V
 */
static const SDL_Scancode keymap[] = {
	SDL_SCANCODE_X,
	SDL_SCANCODE_1,
	SDL_SCANCODE_2,
	SDL_SCANCODE_3,
	SDL_SCANCODE_Q,
	SDL_SCANCODE_W,
	SDL_SCANCODE_E,
	SDL_SCANCODE_A,
	SDL_SCANCODE_S,
	SDL_SCANCODE_D,
	SDL_SCANCODE_Z,
	SDL_SCANCODE_C,
	SDL_SCANCODE_4,
	SDL_SCANCODE_R,
	SDL_SCANCODE_F,
	SDL_SCANCODE_V,
};

void initialize_keypad()
{
	keystate = SDL_GetKeyboardState(NULL);
}

int is_quitting()
{
	static SDL_Event event;

	for (;;) {
		if (!SDL_PollEvent(&event) && freq > 0)
			break;
		switch (event.type) {
		case SDL_QUIT:  // click close button
			return 1;
		case SDL_KEYDOWN:
			switch (event.key.keysym.scancode) {
			case SDL_SCANCODE_ESCAPE:
				return 1;
			case SDL_SCANCODE_RETURN:
				if (freq == 0)
					return 0;
				break;
			default:
				break;
			}
		default:
			break;
		}
	}

	return 0;
}

int is_keydown(uint8_t keycode)
{
	SDL_PumpEvents();
	return keystate[keymap[keycode]];
}

int get_key()
{
	SDL_PumpEvents();

	int i;
	for (i = 0; i < 16; i++)
		if (keystate[keymap[i]])
			return i;
	return -1;
}
