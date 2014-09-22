/**
 * @include/GFraMe/GFraMe_assets.h
 */
#ifndef __GFRAME_ASSETS_H_
#define __GFRAME_ASSETS_H_

#include <GFraMe/GFraMe_error.h>

/**
 * Check whether a file exists
 * @param	*filename	File to be checked
 * GFraMe_ret_ok - Exists; GFraMe_ret_file_not_found - Doesn't exists
 */
GFraMe_ret GFraMe_assets_check_file(char *fileName);

/**
 * Loads a image into a buffer
 * @param	*filename	Image's filename
 * @param	width	Image's width
 * @param	height	Image's height
 * @param	**buf	Allocated buffer (caller freed!!)
 * @return	GFraMe_ret_ok - Success; Anything else - Failure
 */
GFraMe_ret GFraMe_assets_buffer_image(char *filename, int width, int height,
	char **buf);

GFraMe_ret GFraMe_assets_buffer_audio(char *filename, char **buf, int *len);

/**
 * Someday I'll properly comment this, but it reads a 24 bits R8 G8 B8
 * bitmap and convert it into a data file.
 * @param	*inFile	Filename of the input bitmap file
 * @param	keycolor	AARRGGBB color to be considered translucent
 * @param	*outFile	Filename for the generated file
 * @return 0 - Success; Anything else - Failure
 */
int GFraMe_assets_bmp2dat(char *inFile, int keycolor, char *outFile);

#endif

