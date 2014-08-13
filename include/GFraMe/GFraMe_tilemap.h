/**
 * @include/GFraMe/GFraMe_tilemap.h
 */
#ifndef __GFRAME_TILEMAP_H
#define __GFRAME_TILEMAP_H

#include <GFraMe/GFraMe_object.h>
#include <GFraMe/GFraMe_spriteset.h>

struct stGFraMe_tilemap {
	int x;
	int y;
	char *data;
	int width_in_tiles;
	int height_in_tiles;
	GFraMe_object *boxes;
	GFraMe_spriteset *sset;
};
typedef struct stGFraMe_tilemap GFraMe_tilemap;

/**
 * 
 * @param	*tmap	Tilemap to be initialized
 * @param	width_in_tiles	How many tiles there are horizontally
 * @param	height_in_tiles	How many tiles there are vertically
 * @param	*data	Array of bytes with the tiles
 * @param	*sset	Spriteset used to render the tilemap
 * @param	*collideable	Array with which tile types are solid
 * @param	col_len	Length of the collideable array
 */
GFraMe_ret GFraMe_tilemap_init(GFraMe_tilemap *tmap, int width_in_tiles,
							   int height_in_tiles, char *data,
							   GFraMe_spriteset *sset, char *collideable, 
							   int col_len);

void GFraMe_tilemap_clear(GFraMe_tilemap *tmap);

GFraMe_ret GFraMe_tilemap_draw(GFraMe_tilemap *tmap);

GFraMe_ret GFraMe_tilemap_overlap(GFraMe_tilemap *tmap,GFraMe_object *obj);

#endif

