/**
 * @src/global.c
 */
#include <GFraMe/GFraMe_assets.h>
#include <GFraMe/GFraMe_audio.h>
#include <GFraMe/GFraMe_error.h>
#include <GFraMe/GFraMe_spriteset.h>
#include <GFraMe/GFraMe_texture.h>
#include <stdio.h>
#include <stdlib.h>
#include "global.h"

/**
 * Whether the main loop is running
 */
int gl_running = 0;
/**
 * Reference to the atlas with 8x16 sprites
 */
GFraMe_spriteset gl_sset8x16;
/**
 * Reference to the atlas with 8x8 sprites
 */
GFraMe_spriteset gl_sset8;
/**
 * Reference to the atlas with 16x16 sprites
 */
GFraMe_spriteset gl_sset16;
/**
 * Reference to the atlas with 32x32 sprites
 */
GFraMe_spriteset gl_sset32;
/**
 * Reference to the atlas with 32x64 sprites
 */
GFraMe_spriteset gl_sset32x64;
/**
 * Reference to the atlas with 64x64 sprites
 */
GFraMe_spriteset gl_sset64;

GFraMe_audio gl_jump;
GFraMe_audio gl_death;
GFraMe_audio gl_hit;
GFraMe_audio gl_start;
GFraMe_audio gl_charge;
GFraMe_audio gl_floor;
GFraMe_audio gl_song;

/**
 * Atlas with every in-game sprite
 */
static GFraMe_texture gl_atlas;

/**
 * Initialize global stuff (like graphics)
 */
GFraMe_ret global_init() {
	GFraMe_ret rv;
	unsigned char *pixels = NULL;
	const int atlas_w = 256;
	const int atlas_h = 256;
	
	// Init the texture, so nothing happens on error
	GFraMe_texture_init(&gl_atlas);
	// Load the pixels of the texture
	rv = GFraMe_assets_buffer_image("new-atlas-2", atlas_w, atlas_h,
		(char**)(&pixels));
	GFraMe_assertRet(rv == GFraMe_ret_ok, "Failed to load pixels", _ret);
	// Load the texture itself
	rv = GFraMe_texture_load(&gl_atlas, atlas_w, atlas_h, pixels);
	GFraMe_assertRet(rv == GFraMe_ret_ok,"Failed to create texture", _ret);
	// Create the spritesets (i.e., make some basic calculations)
	GFraMe_spriteset_init(&gl_sset8x16, &gl_atlas, 8, 16);
	GFraMe_spriteset_init(&gl_sset8, &gl_atlas, 8, 8);
	GFraMe_spriteset_init(&gl_sset16, &gl_atlas, 16, 16);
	GFraMe_spriteset_init(&gl_sset32, &gl_atlas, 32, 32);
	GFraMe_spriteset_init(&gl_sset32x64, &gl_atlas, 32, 64);
	GFraMe_spriteset_init(&gl_sset64, &gl_atlas, 64, 64);
	
	rv = GFraMe_audio_init(&gl_jump, "jump", 0, 0, 1);
	GFraMe_assertRet(rv == GFraMe_ret_ok,"Failed to open sfx", _ret);
	
	rv = GFraMe_audio_init(&gl_death, "death", 0, 0, 1);
	GFraMe_assertRet(rv == GFraMe_ret_ok,"Failed to open sfx", _ret);
	
	rv = GFraMe_audio_init(&gl_hit, "hit", 0, 0, 1);
	GFraMe_assertRet(rv == GFraMe_ret_ok,"Failed to open sfx", _ret);
	
	rv = GFraMe_audio_init(&gl_start, "start", 0, 0, 1);
	GFraMe_assertRet(rv == GFraMe_ret_ok,"Failed to open sfx", _ret);
	
	rv = GFraMe_audio_init(&gl_charge, "charge", 0, 0, 1);
	GFraMe_assertRet(rv == GFraMe_ret_ok,"Failed to open sfx", _ret);
	
	rv = GFraMe_audio_init(&gl_floor, "floor", 0, 0, 1);
	GFraMe_assertRet(rv == GFraMe_ret_ok,"Failed to open sfx", _ret);
	
	rv = GFraMe_audio_init(&gl_song, "song01", 1, 0, 1);
	GFraMe_assertRet(rv == GFraMe_ret_ok,"Failed to open music", _ret);
	gl_song.loop_pos = gl_song.len / 2;
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
	
	GFraMe_audio_clear(&gl_jump);
	GFraMe_audio_clear(&gl_death);
	GFraMe_audio_clear(&gl_hit);
	GFraMe_audio_clear(&gl_start);
	GFraMe_audio_clear(&gl_charge);
	GFraMe_audio_clear(&gl_floor);
	GFraMe_audio_clear(&gl_song);
}
