/**
 * @include/GFraMe/GFraMe_texture.h
 */
#ifndef __GFRAME_TEXTURE_H_
#define __GFRAME_TEXTURE_H_

#include <GFraMe/GFraMe_error.h>
#include <SDL2/SDL.h>

struct stGFraMe_texture {
	SDL_Texture *texture;
	int w;
	int h;
	int is_target;
};

typedef struct stGFraMe_texture GFraMe_texture;

/**
 * Initialize a texture (so every field is actually 'nil')
 */
void GFraMe_texture_init(GFraMe_texture *tex);

/**
 * Clear up a texture
 */
void GFraMe_texture_clear(GFraMe_texture *tex);

/**
 * Creates a texture that can be blitted into
 * @param	*out	GFraMe_texture created (allocated by caller!)
 * @param	width	Texture's width
 * @param	height	Texture's height
 * @return	GFraMe_ret_ok - Success; Anything else - Failure
 */
GFraMe_ret GFraMe_texture_create_blank(GFraMe_texture *out,
								int width, int height);

/**
 * Loads a texture's data into a renderable texture
 * @param	*out	GFraMe_texture created (allocated by caller!)
 * @param	width	Texture's width
 * @param	height	Texture's height
 * @param	*data	Input data (must actually be formatted RGBA)
 * @return	GFraMe_ret_ok - Success; Anything else - Failure
 */
GFraMe_ret GFraMe_texture_load(GFraMe_texture *out, int width, int height,
						unsigned char *data);

/**
 * Set some internal state to use l_copy
 * @param *tex	Texture that will be drawn into
 * @return	GFraMe_ret_ok - Success; Anything else - Failure
 */
GFraMe_ret GFraMe_texture_lock(GFraMe_texture *tex);

/**
 * Return state so everything renders correctly
 */
void GFraMe_texture_unlock();

/**
 * Copy into the texture defined at lock (no check is made!!)
 * @param	sx	Source upper-left horizontal position
 * @param	sy	Source upper-left vertical position
 * @param	sw	Source's rect width
 * @param	sh	Source's rect height
 * @param	dx	Destination upper-left horizontal position
 * @param	dy	Destionation upper-left vertical position
 * @param	dw	Destination's rect width
 * @param	dh	Destionation's rect height
 * @param	*tex	Source texture for copying
 */
GFraMe_ret GFraMe_texture_l_copy(int sx, int sy, int sw, int sh,
						  int dx, int dy, int dw, int dh,
						  GFraMe_texture *tex);
/**
 * Copy into the texture defined at lock (no check is made!!),
 * flipping source horizontally
 * @param	sx	Source upper-left horizontal position
 * @param	sy	Source upper-left vertical position
 * @param	sw	Source's rect width
 * @param	sh	Source's rect height
 * @param	dx	Destination upper-left horizontal position
 * @param	dy	Destionation upper-left vertical position
 * @param	dw	Destination's rect width
 * @param	dh	Destionation's rect height
 * @param	*tex	Source texture for copying
 */
GFraMe_ret GFraMe_texture_l_copy_flipped(int sx, int sy, int sw, int sh,
						  int dx, int dy, int dw, int dh,
						  GFraMe_texture *tex);

/**
 * Copies portion of a texture into another texture
 * @param	sx	Source upper-left horizontal position
 * @param	sy	Source upper-left vertical position
 * @param	sw	Source's rect width
 * @param	sh	Source's rect height
 * @param	dx	Destination upper-left horizontal position
 * @param	dy	Destionation upper-left vertical position
 * @param	dw	Destination's rect width
 * @param	dh	Destionation's rect height
 * @param	*src	Source texture for copying
 * @param	*dst	Target texture
 */
GFraMe_ret GFraMe_texture_copy(int sx, int sy, int sw, int sh,
						int dx, int dy, int dw, int dh,
						GFraMe_texture *src, GFraMe_texture *dst);

#endif

