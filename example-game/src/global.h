/**
 * @src/global.c
 */
#ifndef __GLOBAL_H_
#define __GLOBAL_H_

#include <GFraMe/GFraMe_audio.h>
#include <GFraMe/GFraMe_spriteset.h>

/**
 * Function that maps a character to its tile counterpart
 */
#define CHAR2TILE(x)	(x - '!')

/**
 * Whether the main loop is running
 */
extern int gl_running;
/**
 * Reference to the atlas with 8x16 sprites
 */
extern GFraMe_spriteset gl_sset8x16;
/**
 * Reference to the atlas with 8x8 sprites
 */
extern GFraMe_spriteset gl_sset8;
/**
 * Reference to the atlas with 16x16 sprites
 */
extern GFraMe_spriteset gl_sset16;
/**
 * Reference to the atlas with 32x32 sprites
 */
extern GFraMe_spriteset gl_sset32;
/**
 * Reference to the atlas with 32x64 sprites
 */
extern GFraMe_spriteset gl_sset32x64;
/**
 * Reference to the atlas with 64x64 sprites
 */
extern GFraMe_spriteset gl_sset64;

extern GFraMe_audio jump;

/**
 * Initialize global stuff (like graphics)
 */
GFraMe_ret global_init();

/**
 * Clear every global reference
 */
void global_clear();

#endif

