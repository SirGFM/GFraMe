/**
 * @src/gframe_spriteset.c
 */
#include <GFraMe/GFraMe_error.h>
#include <GFraMe/GFraMe_screen.h>
#include <GFraMe/GFraMe_spriteset.h>
#include <GFraMe/GFraMe_texture.h>

/**
 * Initialize a new spriteset
 * @param	*sset	Spriteset to be initialized
 * @param	*tex	Texture to be used by the spriteset
 * @param	tile_w	Each tiles' width
 * @param	tile_h	Each tiles' height
 */
void GFraMe_spriteset_init(GFraMe_spriteset *sset, GFraMe_texture *tex,
						   int tile_w, int tile_h) {
	// Store the texture
	sset->tex = tex;
	// Simply copy the dimensions (to make it easier to fetch)
	sset->w = tex->w;
	sset->h = tex->h;
	// Set the tile's width and height
	sset->tw = tile_w;
	sset->th = tile_h;
	// Calculate how many rows there are
	sset->rows = sset->h / tile_h;
	// Calculate how many columns there are
	sset->columns = sset->w / tile_w;
	// Calculate the maximun tile
	sset->max = sset->rows * sset->columns;
}

/**
 * Render a frame from the spriteset to the screen
 * @param	*sset	Spriteset used to render
 * @param	tile	Index from the spriteset to be used
 * @param	x	Horizontal position, on the screen
 * @param	y	Vertical position, on the screen
 * @param	flipped	Whether the tile should be drawn flipped or not
 * @return	GFraMe_ret_ok - Success; Anything else - Failure
 */
GFraMe_ret GFraMe_spriteset_draw(GFraMe_spriteset *sset, int tile, int x,
								 int y, int flipped){
	GFraMe_ret rv = GFraMe_ret_ok;
	// Source position, on the texture
	int sx, sy;
	// Check that the index isn't out of bounds
	GFraMe_assertRV(tile < sset->max, "Invalid tile!",
					rv = 1, _ret);
	// Calculate the tile position
	sx = (tile % sset->columns) * sset->tw;
	sy = (tile / sset->columns) * sset->th;
	// If no lock was performed (and rendering was initiated),
	// GFraMe_texture_l_copy will copy to the screen
	if (!flipped)
		rv = GFraMe_texture_l_copy(sx, sy, sset->tw, sset->th,
								    x,  y, sset->tw, sset->th, sset->tex);
	else
		rv = GFraMe_texture_l_copy_flipped(sx, sy, sset->tw, sset->th,
								    x,  y, sset->tw, sset->th, sset->tex);
	GFraMe_assertRet(rv == 0, "Failed to render tile!", _ret);
_ret:
	return rv;
}

