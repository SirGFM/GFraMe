/**
 * @include/GFraMe/GFraMe_sprite.h
 */
#ifndef __GFRAME_SPRITE_H_
#define __GFRAME_SPRITE_H_

#include <GFraMe/GFraMe_animation.h>
#include <GFraMe/GFraMe_object.h>
#include <GFraMe/GFraMe_spriteset.h>
#include <GFraMe/GFraMe_texture.h>

struct stGFraMe_sprite {
	/**
	 * Handles the physics/positioning (yep, the actual structure!)
	 */
	GFraMe_object obj;
	/**
	 * Spriteset used by this sprite
	 */
	GFraMe_spriteset *sset;
	/**
	 * Currently displaying animation or NULL
	 */
	GFraMe_animation *anim;
	/**
	 * Current displaying tile
	 */
	int cur_tile;
	/**
	 * Graphic's horizontal offset (from physical position)
	 */
	int offset_x;
	/**
	 * Graphic's vertical offset (from physical position)
	 */
	int offset_y;
	/**
	 * Unused; a possible utillity is for setting "types"
	 */
	int id;
};
typedef struct stGFraMe_sprite GFraMe_sprite;

/**
 * Initilialize a sprite with its most basic features;
 * note that the hitbox will be centered on the object
 * @param	*spr	Sprite to be initialized
 * @param	x	Horizontal position
 * @param	y	Vertical position
 * @param	w	Object's width
 * @param	h	Object's height
 * @param	*sset	Spriteset used by the player (will default to tile 0)
 * @param	ox	Sprite's (image) horizontal offset from the "physical" pos.
 * @param	oy	Sprite's (image) vertical offset from the "physical" pos.
 */
void GFraMe_sprite_init(GFraMe_sprite *spr, int x, int y, int w, int h,
						GFraMe_spriteset *sset, int ox, int oy);

/**
 * Update the sprite (its position, animation and tile);
 * note that a non-looping animation will be removed on finish!
 * @param	*spr	Sprite to be updated
 * @param	ms	Elapsed time from previous frame (in milliseconds)
 */
void GFraMe_sprite_update(GFraMe_sprite *spr, int ms);

/**
 * Draw a sprite at its current position
 * @param	*spr	Sprite to be drawn
 */
void GFraMe_sprite_draw(GFraMe_sprite *spr);

/**
 * Change the sprite's animation
 * @param	*spr	Sprite to have it's animation changed
 * @param	*anim	Animation to be set as current
 */
void GFraMe_sprite_set_animation(GFraMe_sprite *spr,
								 GFraMe_animation *anim);

#endif

