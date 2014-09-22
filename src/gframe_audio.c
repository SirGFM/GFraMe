/**
 * @src/gframe_audio.c
 */
#include <GFraMe/GFraMe_assets.h>
#include <GFraMe/GFraMe_audio_player.h>
#include <GFraMe/GFraMe_audio.h>
#include <GFraMe/GFraMe_error.h>
#include <SDL2/SDL_audio.h>
#include <SDL2/SDL_rwops.h>
#include <stdio.h>
#include <stdlib.h>

/**
 * Loads an audio from a WAVE file into an GFraMe_audio
 * @param	*aud	Struct where the audio will be stored
 * @param	*filename	Filename of the audio source
 * @param	loop	Whether the audio should loop or not
 * @param	loop_pos	Sample that should be jumped to on loop
 * @return	GFraMe_ok if the audio loaded correctly
 */
GFraMe_ret GFraMe_audio_init(GFraMe_audio *aud, char *filename, int loop,
	int loop_pos, int stereo) {
	SDL_RWops *file = NULL;
	GFraMe_ret rv;
	
	rv = GFraMe_assets_buffer_audio(filename, &(aud->buf), &(aud->len));
	GFraMe_assertRV(rv == GFraMe_ret_ok, "Failed to buffer audio file",
		rv = GFraMe_ret_failed, _ret);
	
	aud->loop = loop;
	aud->loop_pos = loop_pos;
	aud->stereo = stereo;
_ret:
	if (file)
		SDL_RWclose(file);
	return rv;
}

/**
 * Clear up memory allocated by the audio
 * @param	*aud	Struct which should be cleared
 */
void GFraMe_audio_clear(GFraMe_audio *aud) {
	free(aud->buf);
	aud->buf = NULL;
}

void GFraMe_audio_play(GFraMe_audio *aud, double volume) {
	GFraMe_audio_player_push(aud, volume);
}

