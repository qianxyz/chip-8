#include <SDL2/SDL.h>

#include "audio.h"

#define SAMPLE_FREQ 48000

static SDL_AudioSpec want, have;

static void audio_callback(void *unused, Uint8 *buffer, int bytes);

int initialize_audio()
{
	if(SDL_Init(SDL_INIT_AUDIO) != 0) {
		printf("[ERROR] cannot initialize SDL: %s\n", SDL_GetError());
		return 1;
	}

	want.freq = SAMPLE_FREQ;
	want.format = AUDIO_F32;
	want.channels = 1;
	want.samples = 8192;
	want.callback = audio_callback;
	want.userdata = NULL;

	if(SDL_OpenAudio(&want, &have) != 0) {
		printf("[ERROR] cannot open audio: %s\n", SDL_GetError());
		return 1;
	}
	if(want.format != have.format) {
		printf("[ERROR] cannot get the desired AudioSpec\n");
		return 1;
	}

	return 0;
}

void open_audio()
{
	SDL_PauseAudio(0);
}

void close_audio()
{
	SDL_PauseAudio(1);
}

void terminate_audio()
{
	SDL_CloseAudio();
}

static void audio_callback(void *unused, Uint8 *buffer, int bytes)
{
	(void)unused;
	for(int i = 0; i < bytes; i++)
	{
		buffer[i] = i;
	}
}
