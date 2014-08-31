/**
 * @src/gframe_audio_player.c
 */
#include <GFraMe/GFraMe_audio.h>
#include <GFraMe/GFraMe_audio_player.h>
#include <GFraMe/GFraMe_error.h>
#include <SDL2/SDL.h>
#include <stdio.h>

struct stGFraMe_audio_ll {
	struct stGFraMe_audioll *next;
	GFraMe_audio *audio;
	int pos;
};
typedef struct stGFraMe_audio_ll GFraMe_audio_ll;

static int did_audio_init = 0;
static SDL_AudioDeviceID dev = 0;
static SDL_AudioSpec spec;
static GFraMe_audio_ll *cur = NULL;
static GFraMe_audio_ll *recycle = NULL;

static void GFraMe_audio_player_callback(void *arg, Uint8 *stream, int len);

GFraMe_ret GFraMe_audio_player_init() {
	GFraMe_ret rv = GFraMe_ret_ok;
	SDL_AudioSpec wanted;
	
	rv = SDL_InitSubSystem(SDL_INIT_AUDIO);
	GFraMe_SDLassertRV(rv == 0, "Failed to init audio",
					   rv = GFraMe_ret_sdl_init_failed, _ret);
	did_audio_init = 1;
	
	wanted.freq = 44100;
	wanted.format = AUDIO_S16LSB;
	wanted.channels = 2;
	wanted.samples = 1024;
	wanted.callback = GFraMe_audio_player_callback;
	wanted.userdata = 0;
	
	dev = SDL_OpenAudioDevice(NULL, 0, &wanted, &spec, 0);
	GFraMe_SDLassertRV(dev > 0, "Failed to open audio device",
					 rv = GFraMe_ret_failed, _ret);
	GFraMe_log("-----------------------------\n");
	GFraMe_log("Audio info:\n");
	GFraMe_log("  Frequency: %i\n", spec.freq);
	GFraMe_log("  Format: ");
	if (spec.format == AUDIO_S16LSB)
		GFraMe_log("signed 16 bits little endian");
	GFraMe_log("\n  Channels: %i\n", spec.channels);
	GFraMe_log("  Samples: %i\n", spec.samples);
	// GFraMe_log("  Bytes per sample: %i\n", bytesPerSample);
	GFraMe_log("-----------------------------\n");
_ret:
	return rv;
}

void GFraMe_audio_player_clear() {
	if (dev != 0)
		SDL_CloseAudioDevice(dev);
	dev = 0;
	if (did_audio_init) {
		SDL_QuitSubSystem(SDL_INIT_AUDIO);
		did_audio_init = 0;
	}
}

static void GFraMe_audio_player_callback(void *arg, Uint8 *stream, int len) {
}

