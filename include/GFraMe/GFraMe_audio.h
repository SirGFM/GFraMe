/**
 * @include/GFraMe/GFraMe_audio.h
 */
#ifndef __GFRAME_AUDIO_H_
#define __GFRAME_AUDIO_H_

#include <GFraMe/GFraMe_error.h>

/**
 * Container for a loaded audio.
 */
struct stGFraMe_audio {
	/**
	 * Buffered audio
	 */
	char *buf;
	/**
	 * How many samples there are in the audio
	 */
	int len;
	/**
	 * To which sample it should go back, when looping
	 */
	int loop_pos;
	/**
	 * Whether it's a looping audio or not
	 */
	int loop;
};
typedef struct stGFraMe_audio GFraMe_audio;

/**
 * Loads an audio from a WAVE file into an GFraMe_audio
 * @param	*aud	Struct where the audio will be stored
 * @param	*filenam	Filename of the audio source
 * @param	loop	Whether the audio should loop or not
 * @param	loop_pos	Sample that should be jumped to on loop
 * @return	GFraMe_ok if the audio loaded correctly
 */
GFraMe_ret GFraMe_audio_init(GFraMe_audio *aud, char *filename, int loop, int loop_pos);
/**
 * Clear up memory allocated by the audio
 * @param	*aud	Struct which should be cleared
 */
void GFraMe_audio_clear(GFraMe_audio *aud);
void GFraMe_audio_play(GFraMe_audio *aud, double volume);

#endif

