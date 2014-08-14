/**
 * @src/gframe_tilemap.c
 */
#include <GFraMe/GFraMe_error.h>
#include <GFraMe/GFraMe_object.h>
#include <GFraMe/GFraMe_spriteset.h>
#include <GFraMe/GFraMe_tilemap.h>
#include <stdio.h>
#include <stdlib.h>

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
							   int col_len) {
	int i;
	int max = width_in_tiles * height_in_tiles;
	GFraMe_ret rv = GFraMe_ret_ok;
	// Init every alloc'ed pointer with NULL
	tmap->data = NULL;
	tmap->boxes = NULL;
	// Copy tilemap's limits
	tmap->width_in_tiles = width_in_tiles;
	tmap->height_in_tiles = height_in_tiles;
	// Alloc tilemap's buffer
	GFraMe_assertRV(data, "Invalid tilemap data", rv = GFraMe_ret_bad_param, _ret);
	tmap->data = (char*)malloc(max);
	GFraMe_assertRV(tmap->data, "Failed to alloc tilemap data",
					rv = GFraMe_ret_memory_error, _ret);
	// Copy it
	i = 0;
	while (i < max) {
		tmap->data[i] = data[i];
		i++;
	}
	// TODO check collideable array and create the minimum amount
	// of objects to cover the complete area
	// Copy the spriteset
	tmap->sset = sset;
_ret:
	if (rv != GFraMe_ret_ok)
		GFraMe_tilemap_clear(tmap);
	return rv;
}

void GFraMe_tilemap_clear(GFraMe_tilemap *tmap) {
	// Set it to (0,0)
	tmap->x = 0;
	tmap->y = 0;
	// Clear it's dimensions
	tmap->width_in_tiles = 0;
	tmap->height_in_tiles = 0;
	// Check if there was any data and free it
	if (tmap->data)
		free(tmap->data);
	tmap->data = NULL;
	// Check if there was any data and free it
	if (tmap->boxes)
		free(tmap->boxes);
	tmap->boxes = NULL;
}

GFraMe_ret GFraMe_tilemap_draw(GFraMe_tilemap *tmap) {
	int i;
	GFraMe_ret rv = GFraMe_ret_ok;
	// TODO check which rect is inside the screen and draw only that!
/*
	// stupid way to use only one loop and no if *inside* the loop
	i = 0;
	int x = 0;
	int y = 0;
	while (i < tmap->num_tiles) {
		rv = GFraMe_spriteset_draw(tmap->sset, tmap->data[i],
								   tmap->x + x*tmap->sset->tw,
								   tmap->y + y);
		GFraMe_assertRet(rv == GFraMe_ret_ok, "Failed to draw tilemap",
						 _ret);
		x++;
		y += tmap->sset->th * (x >= width_in_tiles);
		x -= width_in_tiles * (x >= width_in_tiles);
		i++;
	}
*/
	// Loop each row
	i = 0;
	while (i < tmap->width_in_tiles) {
		int j;
		// Loop each column
		j = 0;
		while (j < tmap->height_in_tiles) {
			rv = GFraMe_spriteset_draw(tmap->sset,
								  tmap->data[i + j*tmap->width_in_tiles],
								  tmap->x + i*tmap->sset->tw,
								  tmap->y + j*tmap->sset->th);
			GFraMe_assertRet(rv == GFraMe_ret_ok, "Failed to draw tilemap",
							 _ret);
			j++;
		}
		i++;
	}
_ret:
	return rv;
}

GFraMe_ret GFraMe_tilemap_overlap(GFraMe_tilemap *tmap,GFraMe_object *obj){
	GFraMe_ret rv = GFraMe_ret_no_overlap;
	return rv;
}

