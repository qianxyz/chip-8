#include <SDL2/SDL.h>

#include "keypad.h"

int quit = 0;

static const Uint8 *key_state;

int initialize_keypad()
{
	key_state = SDL_GetKeyboardState(NULL);

	return 0;
}

void update_keypad()
{
	SDL_PumpEvents();
	if (key_state[SDL_SCANCODE_ESCAPE])
		quit = 1;
}
