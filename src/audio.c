#include <SDL2/SDL.h>

#include "audio.h"

#define SAMPLE_RATE 44100
#define AMPLITUDE 0.5
#define FREQUENCY 440

static SDL_AudioSpec want, have;

static void audio_callback(void *unused, Uint8 *buffer, int bytes);

int initialize_audio()
{
	if (SDL_Init(SDL_INIT_AUDIO) != 0) {
		printf("[ERROR] cannot initialize SDL: %s\n", SDL_GetError());
		return 1;
	}

	SDL_memset(&want, 0, sizeof(want));
	want.freq = SAMPLE_RATE;
	want.format = AUDIO_F32;
	want.channels = 1;
	want.samples = 4096;
	want.callback = audio_callback;

	if (SDL_OpenAudio(&want, &have) != 0) {
		printf("[ERROR] cannot open audio: %s\n", SDL_GetError());
		return 1;
	}
	if (want.format != have.format) {
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

static void audio_callback(void *unused, Uint8 *stream, int len)
{
	(void)unused;
	static double phase = 0.0;
	double phase_increment = 2.0 * M_PI * FREQUENCY / SAMPLE_RATE;
	float *buffer = (float *)stream;
	int length = len / sizeof(float);

	for (int i = 0; i < length; i++) {
		buffer[i] = (float)(AMPLITUDE * sin(phase));
		phase += phase_increment;
		if (phase >= 2.0 * M_PI) {
			phase -= 2.0 * M_PI;
		}
	}
}
