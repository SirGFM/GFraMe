/**
 * @src/gframe_audio_player.c
 */
#include <GFraMe/GFraMe_audio.h>
#include <GFraMe/GFraMe_audio_player.h>
#include <GFraMe/GFraMe_error.h>
#include <SDL2/SDL.h>
#include <stdio.h>

struct stGFraMe_audio_ll {
	struct stGFraMe_audio_ll *next;
	GFraMe_audio *audio;
	int pos;
	double volume;
};
typedef struct stGFraMe_audio_ll GFraMe_audio_ll;

static int did_audio_init = 0;
static SDL_AudioDeviceID dev = 0;
static SDL_AudioSpec spec;
int count;
static GFraMe_audio_ll *cur;
static GFraMe_audio_ll *recycle;
static GFraMe_audio_ll bgm;

static void GFraMe_audio_player_callback(void *arg, Uint8 *stream, int len);
static GFraMe_audio_ll* GFraMe_audio_player_remove(GFraMe_audio_ll *prev,
											GFraMe_audio_ll *node);
static GFraMe_audio_ll* GFraMe_audio_player_get_new_audio_ll(GFraMe_audio *aud, double volume);
static int GFraMe_audio_player_mix(GFraMe_audio_ll *node, Uint8 *dst, int len);

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
	GFraMe_log("=============================");
	GFraMe_log("| Audio info:");
	GFraMe_log("-----------------------------");
	GFraMe_log("|   Frequency: %i", spec.freq);
	if (spec.format == AUDIO_S16LSB)
		GFraMe_log("|   Format: signed 16 bits little endian");
	GFraMe_log("|   Channels: %i", spec.channels);
	GFraMe_log("|   Samples: %i", spec.samples);
	// GFraMe_log("|   Bytes per sample: %i\n", bytesPerSample);
	GFraMe_log("=============================\n");
	
	cur = NULL;
	recycle = NULL;
	bgm.next = NULL;
	bgm.audio = NULL;
	count = 0;
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
	while (cur) {
		GFraMe_audio_ll *tmp = cur;
		cur = cur->next;
		free(tmp);
	}
	while (recycle) {
		GFraMe_audio_ll *tmp = recycle;
		recycle = recycle->next;
		free(tmp);
	}
}

SDL_AudioSpec* GFraMe_audio_player_get_spec() {
	return &spec;
}

void GFraMe_audio_player_play_bgm(GFraMe_audio *aud, double volume) {
	if (aud != bgm.audio) {
		// TODO lock_bgm
		if (aud && !bgm.audio)
			count++;
		else if (!aud && bgm.audio)
			count--;
		bgm.audio = aud;
		bgm.pos = 0;
		// TODO unlock_bgm
	}
	bgm.volume = volume;
}

void GFraMe_audio_player_push(GFraMe_audio *aud, double volume) {
	GFraMe_audio_ll *node;
	node = GFraMe_audio_player_get_new_audio_ll(aud, volume);
	// TODO lock_cur
	node->next = cur;
	cur = node;
	// TODO unlock_cur
	count++;
	SDL_PauseAudioDevice(dev, 0);
}

static GFraMe_audio_ll* GFraMe_audio_player_remove(GFraMe_audio_ll *prev,
											GFraMe_audio_ll *node) {
	GFraMe_audio_ll *ret;
	if (prev) {
		prev->next = node->next;
		ret = prev->next;
	}
	else {
		// TODO lock_cur
		cur = node->next;
		ret = cur;
		// TODO lock_recycle
	}
	// TODO unlock_cur
	node->next = recycle;
	recycle = node;
	// TODO unlock_recycle
	return ret;
}

static GFraMe_audio_ll* GFraMe_audio_player_get_new_audio_ll(GFraMe_audio *aud,
															 double volume) {
	GFraMe_audio_ll *node;
	// TODO lock_recycle
	if (!recycle)
		node = (GFraMe_audio_ll*)malloc(sizeof(GFraMe_audio_ll));
	else {
		node = recycle;
		recycle = recycle->next;
	}
	// TODO unlock_recycle
	node->audio = aud;
	node->pos = 0;
	node->volume = volume;
	return node;
}

static void GFraMe_audio_player_callback(void *arg, Uint8 *stream, int len) {
	GFraMe_audio_ll *node;
	GFraMe_audio_ll *prev = NULL;
	// TODO lock_cur
	node = cur;
	// TODO unlock_cur
	while (node) {
		int remove;
		remove = GFraMe_audio_player_mix(node, stream, len);
		if (remove)
			node = GFraMe_audio_player_remove(prev, node);
		else {
			prev = node;
			node = node->next;
		}
	}
	// TODO lock_bgm
	if (bgm.audio) {
		GFraMe_audio_player_mix(&bgm, stream, len);
	}
	// TODO unlock_bgm
	if (!cur && !bgm.audio)
		SDL_PauseAudioDevice(dev, 1);
}

static int GFraMe_audio_player_mix(GFraMe_audio_ll *node, Uint8 *dst, int len) {
	int i = 0;
	while (i < len) {
		// Get the data to be put into both channels
		Sint16 chan1 = node->audio->buf[i + node->pos]
					 | ((node->audio->buf[i + node->pos + 1] << 8) & 0xff00);
		Sint16 chan2 = node->audio->buf[i + node->pos + 3]
					 | ((node->audio->buf[i + node->pos + 4] << 8) & 0xff00);
		// Modify its volume
		chan1 = (Sint16)(chan1*node->volume);
		chan2 = (Sint16)(chan2*node->volume);
		// Add it to the channel
		dst[i] += (Uint8)(chan1&0xff);
		dst[i+1] += (Uint8)((chan1>>8)&0xff);
		dst[i+2] += (Uint8)(chan2&0xff);
		dst[i+3] += (Uint8)((chan2>>8)&0xff);
		
		i += 4;
		// If the sample is over
		if (i + node->pos > node->audio->len) {
			// Loop it
			if (node->audio->loop) {
				len -= i;
				i = 0;
				node->pos = node->audio->loop_pos;
			}
			else
				// Issue it to be recycled
				return 1;
		}
	}
	// Update the sample position
	node->pos += i;
	return 0;
}

