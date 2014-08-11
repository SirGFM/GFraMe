/**
 * @src/global.c
 */
#ifndef __GLOBAL_H_
#define __GLOBAL_H_

#include <GFraMe/GFraMe_spriteset.h>

/**
 * Whether the main loop is running
 */
extern int gl_running;
/**
 * Reference to the atlas with 16x16 sprites
 */
extern GFraMe_spriteset gl_sset16;
/**
 * Reference to the atlas with 32x32 sprites
 */
extern GFraMe_spriteset gl_sset32;
/**
 * Reference to the atlas with 64x64 sprites
 */
extern GFraMe_spriteset gl_sset64;

/**
 * Initialize global stuff (like graphics)
 */
GFraMe_ret global_init();

/**
 * Clear every global reference
 */
void global_clear();

#endif

