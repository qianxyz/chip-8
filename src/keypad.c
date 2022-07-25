#include <SDL2/SDL.h>

#include "keypad.h"

static const Uint8 *key_state;

int initialize_keypad()
{
	key_state = SDL_GetKeyboardState(NULL);

	return 0;
}

int is_quitting()
{
	static SDL_Event event;

	while (SDL_PollEvent(&event)) {
		switch (event.type) {
		case SDL_QUIT:  // click close button
			return 1;
		case SDL_KEYDOWN:  // press ESC key
			if (event.key.keysym.scancode == SDL_SCANCODE_ESCAPE)
				return 1;
		default:
			break;
		}
	}

	return 0;
}
