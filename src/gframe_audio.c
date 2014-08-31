/**
 * @src/gframe_audio.c
 */
#include <GFraMe/GFraMe_assets.h>
#include <GFraMe/GFraMe_audio.h>
#include <GFraMe/GFraMe_error.h>
#include <stdio.h>
#include <stdlib.h>

/**
 * Loads an audio from a WAVE file into an GFraMe_audio
 * @param	*aud	Struct where the audio will be stored
 * @param	*filenam	Filename of the audio source
 * @return	GFraMe_ok if the audio loaded correctly
 */
GFraMe_ret GFraMe_audio_init(GFraMe_audio *aud, char *filename) {
	char *clean_fn;
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
_ret:
	return rv;
}

/**
 * Clear up memory allocated by the audio
 * @param	*aud	Struct which should be cleared
 */
void GFraMe_audio_clear(GFraMe_audio *aud) {
	
}

