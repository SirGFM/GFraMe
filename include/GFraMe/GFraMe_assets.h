/**
 * @include/GFraMe/GFraMe_assets.h
 */
#ifndef __GFRAME_ASSETS_H_
#define __GFRAME_ASSETS_H_

#include <GFraMe/GFraMe_error.h>

/**
 * SDL already considers assets on a mobile device to be on a 'assets/'
 * folder, so this function removes it (if it was compiled for mobile)
 * @param	*filename	Original filename, that may begin with 'assets/'
 * @retrun	Filename with 'assets/' removed.
 */
char* GFraMe_assets_clean_filename(char *filename);

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
GFraMe_ret GFraMe_assets_buffer_image(char *filename, int width, int height, char **buf);

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

