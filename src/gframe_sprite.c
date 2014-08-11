/**
 * @src/gframe_sprite.c
 */
#include <GFraMe/GFraMe_animation.h>
#include <GFraMe/GFraMe_error.h>
#include <GFraMe/GFraMe_object.h>
#include <GFraMe/GFraMe_sprite.h>
#include <GFraMe/GFraMe_spriteset.h>
#include <GFraMe/GFraMe_texture.h>

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
						GFraMe_spriteset *sset, int ox, int oy) {
	// First, clear the object, set its position...
	GFraMe_object_clear(&(spr->obj));
	GFraMe_object_set_pos(&(spr->obj), x, y);
	// And modify its hitbox
	GFraMe_object_set_hitbox(&(spr->obj), GFraMe_set_hitbox_upper_left,
							 0, 0, w, h);
	// Now, set the sprite graphics
	spr->sset = sset;
	spr->cur_tile = 0;
	// Make sure no animation is running
	spr->anim = NULL;
	// Set the graphic's offset from the physical position
	spr->offset_x = ox;
	spr->offset_y = oy;
	// Set id to zero because... whatever
	spr->id = 0;
}

/**
 * Update the sprite (its position, animation and tile);
 * note that a non-looping animation will be removed on finish!
 * @param	*spr	Sprite to be updated
 * @param	ms	Elapsed time from previous frame (in milliseconds)
 */
void GFraMe_sprite_update(GFraMe_sprite *spr, int ms) {
	// Physically update the object
	GFraMe_object_update(&(spr->obj), ms);
	if (spr->anim) {
		GFraMe_ret ret;
		// Update the animation and check what happened
		ret = GFraMe_animation_update(spr->anim, ms);
		// If a new frame was issued, update it
		if (ret == GFraMe_ret_anim_new_frame)
			spr->cur_tile = spr->anim->tile;
		// Else, if animation is finished, remove it
		else if (ret == GFraMe_ret_anim_finished)
			spr->anim = NULL;
	}
}

/**
 * Draw a sprite at its current position
 * @param	*spr	Sprite to be drawn
 */
void GFraMe_sprite_draw(GFraMe_sprite *spr) {
	// Simply draw the current frame at the current position
	GFraMe_spriteset_draw(spr->sset, spr->cur_tile,
			spr->obj.x + spr->offset_x, spr->obj.y + spr->offset_y);
}

/**
 * Change the sprite's animation
 * @param	*spr	Sprite to have it's animation changed
 * @param	*anim	Animation to be set as current
 */
void GFraMe_sprite_set_animation(GFraMe_sprite *spr,
								 GFraMe_animation *anim) {
	// Simply change the current animation and tile
	spr->anim = anim;
	spr->cur_tile = anim->tile;
}

