/**
 * @src/gframe_assets.c
 */
#include <GFraMe/GFraMe_assets.h>
#include <GFraMe/GFraMe_error.h>
#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdlib.h>

/**
 * SDL already considers assets on a mobile device to be on a 'assets/'
 * folder, so this function removes it (if it was compiled for mobile)
 * @param	*filename	Original filename, that may begin with 'assets/'
 * @retrun	Filename with 'assets/' removed.
 */
char* GFraMe_assets_clean_filename(char *filename) {
#ifdef MOBILE
	// String to be removed from the begin of a filename
	char *match = "assets/";
#else
	// Which should be empty, for non-mobile
	char *match = "";
#endif
	// Copy it, so we can recover if it doesn't start with 'match'
	char *tmp = filename;
	// Move filename until it's after the match, or not matching anymore
	while (*tmp == *match) {
		tmp++;
		match++;
	}
	// If the end of match was reached, return the new string
	if (*match == '\0')
		filename = tmp;
	return filename;
}


/**
 * Check whether a file exists
 * @param	*filename	File to be checked
 * GFraMe_ret_ok - Exists; GFraMe_ret_file_not_found - Doesn't exists
 */
GFraMe_ret GFraMe_assets_check_file(char *fileName) {
	GFraMe_ret rv = GFraMe_ret_file_not_found;
	SDL_RWops *fp = NULL;
	// Try to open the file
	fp = SDL_RWFromFile(fileName, "rb");
	if (fp) {
		// If it was opened, it exists; so close it!
		SDL_RWclose(fp);
		rv = GFraMe_ret_ok;
	}
	return rv;
}

/**
 * Loads a image into a buffer
 * @param	*filename	Image's filename
 * @param	width	Image's width
 * @param	height	Image's height
 * @param	**buf	Allocated buffer (caller freed!!)
 * @return	GFraMe_ret_ok - Success; Anything else - Failure
 */
GFraMe_ret GFraMe_assets_buffer_image(char *filename, int width, int height, char **buf) {
	GFraMe_ret rv = GFraMe_ret_ok;
	int size = 0;
	char *pixels = NULL;
	SDL_RWops *fp = NULL;
	// Get file lenght in bytes
	size = width*height*4;
	// Alloc return buffer
	pixels = (char*)malloc(size);
	GFraMe_assertRV(pixels, "Couldn't alloc memory", rv = GFraMe_ret_memory_error, _ret);
    // Open image file
    fp = SDL_RWFromFile(filename, "rb");
	GFraMe_assertRV(fp, "Couldn't find file", rv = GFraMe_ret_file_not_found, _ret);
	// Load image
    rv = SDL_RWread(fp, pixels, size, 1);
	GFraMe_assertRV(rv == 1, "Failed to read file", rv = GFraMe_ret_read_file_failed, _ret);
	rv = 0;
	*buf = pixels;
_ret:
    if (fp)
		SDL_RWclose(fp);
	return rv;
}

/**
 * Gets a buffer into a uint (actually a int, but whatever)
 * @param	*buffer	Buffer to be read
 * @param	pos	Position where to start reading on the buffer
 * @return	The calculated uint
 */
static int GFraMe_read_UINT(char *buffer, int pos) {
	// Move the byte on buffer position 'n' to bit starting at '8*n'
	return (0xff&buffer[pos]) +
		   ((buffer[pos+1]<<8)&0xff00) +
		   ((buffer[pos+2]<<16)&0xff0000) +
		   ((buffer[pos+3]<<24)&0xff000000);
}

/**
 * TODO fix this shit!
 * Someday I'll properly comment this, but it reads a 24 bits R8 G8 B8
 * bitmap and convert it into a data file.
 * @param	*inFile	Filename of the input bitmap file
 * @param	keycolor	AARRGGBB color to be considered translucent
 * @param	*outFile	Filename for the generated file
 * @return 0 - Success; Anything else - Failure
 */
int GFraMe_assets_bmp2dat(char *inFile, int keycolor, char *outFile) {
	char buffer[4];
	int i;
	int offset;
	int width;
	int height;
	int bwidth;
	int padding;
	int total;
	int rv = 0;
	char *datab = NULL;
	FILE *in = NULL;
	FILE *out = NULL;
	
#ifdef MOBILE
	GFraMe_assertRV(0, "This shouldn't be run on a mobile dev!", 1, _err);
#endif
	in = fopen(inFile, "rb");
	GFraMe_assertRV(in, "File not found", rv = 3, _err);
	
	fseek(in, 0x0a, SEEK_SET);
	fread(buffer, 4, 1, in);
	offset = GFraMe_read_UINT(buffer, 0);
	// seek and read width from file
	fseek(in, 0x12-0x0a-4, SEEK_CUR);
	fread(buffer, 4, 1, in);
	width = GFraMe_read_UINT(buffer, 0);
	fread(buffer, 4, 1, in);
	height = GFraMe_read_UINT(buffer, 0);
	// get how many bytes are needed and how many are needed to align it
	bwidth = width * 3;//width / 8 + (width%8?1:0);
	padding = bwidth % 4;
	bwidth += padding;
	datab = (char*)malloc(sizeof(char)*width*height*4);
	GFraMe_assertRV(datab, "Failed to alloc memory", rv = 3, _err);
	
	fseek(in, offset, SEEK_SET);
	i = width * (height - 1);
	buffer[4] = 0;
	total = 0;
	while (1) {
		int n = fread(buffer, 3, 1, in);
		if (n == 0)
			break;
		int color = GFraMe_read_UINT(buffer, 0);
		int blue = (color >> 16)&0xff;
		int green = (color >> 8)&0xff;
		int red = color&0xff;
		int pos = i * 4;
		if (i < 0)
			break;
		if (color == keycolor) {
			datab[pos] = 0;
			datab[pos+1] = 0;
			datab[pos+2] = 0;
			datab[pos+3] = 0;
		}
		else {
			datab[pos] = (char)red & 0xfe;
			datab[pos+1] = (char)green & 0xfe;
			datab[pos+2] = (char)blue & 0xfe;
			datab[pos+3] = 0xff & 0xfe;
		}
		i++;
		if (i % width == 0) {
			i -= width * 2;
			fseek(in, padding, SEEK_CUR);
		}
		total += n;
	}
	
	out = fopen(outFile, "wb");
	
	fwrite(datab, sizeof(char)*width*height*4, 1, out);
	
	rv = 0;
_err:
	if (in)
		fclose(in);
	if (out)
		fclose(out);
	if (datab)
		free(datab);
	
	return rv;
}

