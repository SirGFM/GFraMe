/**
 * @src/global.c
 */
#include <GFraMe/GFraMe_assets.h>
#include <GFraMe/GFraMe_error.h>
#include <GFraMe/GFraMe_spriteset.h>
#include <GFraMe/GFraMe_texture.h>
#include <stdio.h>
#include "global.h"

/**
 * Whether the main loop is running
 */
int gl_running = 0;
/**
 * Reference to the atlas with 16x16 sprites
 */
GFraMe_spriteset gl_sset16;
/**
 * Reference to the atlas with 32x32 sprites
 */
GFraMe_spriteset gl_sset32;
/**
 * Reference to the atlas with 64x64 sprites
 */
GFraMe_spriteset gl_sset64;

/**
 * Atlas with every in-game sprite
 */
static GFraMe_texture gl_atlas;

/**
 * Initialize global stuff (like graphics)
 */
GFraMe_ret global_init() {
	GFraMe_ret rv;
	char *filename;
	unsigned char *pixels = NULL;
	const int atlas_w = 64;
	const int atlas_h = 64;
	
	// Init the texture, so nothing happens on error
	GFraMe_texture_init(&gl_atlas);
	// Check for the 'compiled' texture
	filename = GFraMe_assets_clean_filename("assets/atlas.dat");
	if (GFraMe_assets_check_file(filename) != GFraMe_ret_ok) {
		char *bmpfn = GFraMe_assets_clean_filename("assets/atlas.bmp");
		GFraMe_log("Couldn't find atlas.dat... creating it...");
		// Create the usable texture
		rv = GFraMe_assets_bmp2dat(bmpfn, 0xff00ff, filename);
		GFraMe_assertRet(rv == 0, "Failed to create atlas.dat", _ret);
	}
	// Load the pixels of the texture
	rv = GFraMe_assets_buffer_image(filename, atlas_w, atlas_h, (char**)(&pixels));
	GFraMe_assertRet(rv == GFraMe_ret_ok, "Failed to load pixels", _ret);
	// Load the texture itself
	rv = GFraMe_texture_load(&gl_atlas, atlas_w, atlas_h, pixels);
	GFraMe_assertRet(rv == GFraMe_ret_ok,"Failed to create texture", _ret);
	// Create the spritesets (i.e., make some basic calculations)
	GFraMe_spriteset_init(&gl_sset16, &gl_atlas, 16, 16);
	//GFraMe_spriteset_init(&gl_sset32, &gl_atlas, 32, 32);
	//GFraMe_spriteset_init(&gl_sset64, &gl_atlas, 64, 64);
_ret:
	if (pixels)
		free(pixels);
	return rv;
}

/**
 * Clear every global reference
 */
void global_clear() {
	GFraMe_texture_clear(&gl_atlas);
}

