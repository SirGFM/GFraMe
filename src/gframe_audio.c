/**
 * @src/gframe_audio.c
 */
#include <GFraMe/GFraMe_assets.h>
#include <GFraMe/GFraMe_audio_player.h>
#include <GFraMe/GFraMe_audio.h>
#include <GFraMe/GFraMe_error.h>
#include <SDL2/SDL_audio.h>

/**
 * Loads an audio from a WAVE file into an GFraMe_audio
 * @param	*aud	Struct where the audio will be stored
 * @param	*filenam	Filename of the audio source
 * @param	loop	Whether the audio should loop or not
 * @param	loop_pos	Sample that should be jumped to on loop
 * @return	GFraMe_ok if the audio loaded correctly
 */
GFraMe_ret GFraMe_audio_init(GFraMe_audio *aud, char *filename, int loop, int loop_pos) {
	SDL_AudioSpec *spec;
	GFraMe_ret rv = GFraMe_ret_ok;
	// Check if every parameter is OK
	GFraMe_assertRV(aud, "Invalid GFraMe_audio passed",
					rv = GFraMe_ret_bad_param, _ret);
	GFraMe_assertRV(filename, "Invalid filename passed",
					rv = GFraMe_ret_bad_param, _ret);
	rv = GFraMe_assets_check_file(filename);
	GFraMe_assertRet(rv == GFraMe_ret_ok,
					"File not found", _ret);
	// Load the file into the buffer
	aud->len = 0;
	spec = SDL_LoadWAV(filename, GFraMe_audio_player_get_spec(),
					   (Uint8**)&aud->buf, (Uint32*)&aud->len);
	GFraMe_SDLassertRV(spec != NULL, "Failed to open file", rv = GFraMe_ret_failed, _ret);
	aud->loop = loop;
	aud->loop_pos = loop_pos;
_ret:
	return rv;
}

/**
 * Clear up memory allocated by the audio
 * @param	*aud	Struct which should be cleared
 */
void GFraMe_audio_clear(GFraMe_audio *aud) {
	SDL_FreeWAV((Uint8*)aud->buf);
	aud->buf = 0;
}

void GFraMe_audio_play(GFraMe_audio *aud, double volume) {
	GFraMe_audio_player_push(aud, volume);
}

