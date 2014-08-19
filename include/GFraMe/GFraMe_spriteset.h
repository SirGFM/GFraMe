/**
 * @include/GFraMe/GFraMe_include.h
 */
#ifndef __GFRAME_SPRITESET_H_
#define __GFRAME_SPRITESET_H_

#include <GFraMe/GFraMe_error.h>
#include <GFraMe/GFraMe_texture.h>

/**
 * Helper struct to enable rendering from an index (the tile)
 */
struct stGFraMe_spriteset {
	/**
	 * Texture used when rendering
	 */
	GFraMe_texture *tex;
	/**
	 * Texture's width (for easier fetching, if necessary)
	 */
	int w;
	/**
	 * Texture's height (for easier fetching, if necessary)
	 */
	int h;
	/**
	 * Width of one tile
	 */
	int tw;
	/**
	 * Height of one tile
	 */
	int th;
	/**
	 * How many rows there are in this spriteset
	 */
	int rows;
	/**
	 * How many columns there are in this spriteset
	 */
	int columns;
	/**
	 * How many tiles there are on the spriteset
	 */
	int max;
	//                   offset-x, offset-y, sprite-width, sprite-height
	// TODO enable adding a border to each sprite? (like ox, oy, sw, sh)
};
typedef struct stGFraMe_spriteset GFraMe_spriteset;

/**
 * Initialize a new spriteset
 * @param	*sset	Spriteset to be initialized
 * @param	*tex	Texture to be used by the spriteset
 * @param	tile_w	Each tiles' width
 * @param	tile_h	Each tiles' height
 */
void GFraMe_spriteset_init(GFraMe_spriteset *sset, GFraMe_texture *tex,
						   int tile_w, int tile_h);

/**
 * Render a frame from the spriteset to the screen
 * @param	*sset	Spriteset used to render
 * @param	tile	Index from the spriteset to be used
 * @param	x	Horizontal position, on the screen
 * @param	y	Vertical position, on the screen
 * @param	flipped	Whether the tile should be drawn flipped or not
 * @return	0 - Success; Anything else - Failure
 */
GFraMe_ret GFraMe_spriteset_draw(GFraMe_spriteset *sset, int tile, int x,
								 int y, int flipped);

#endif

