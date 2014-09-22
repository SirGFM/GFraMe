/**
 * @src/gframe_sprite.c
 */
#include <GFraMe/GFraMe_animation.h>
#include <GFraMe/GFraMe_error.h>
#include <GFraMe/GFraMe_hitbox.h>
#include <GFraMe/GFraMe_object.h>
#ifdef GFRAME_DEBUG
#include <GFraMe/GFraMe_screen.h>
#endif
#include <GFraMe/GFraMe_sprite.h>
#include <GFraMe/GFraMe_spriteset.h>
#include <GFraMe/GFraMe_texture.h>
#ifdef GFRAME_DEBUG
#include <SDL2/SDL_video.h>
#endif

#ifdef GFRAME_DEBUG
/**
 * Context where the bounding box shall be rendered; debug-mode only
 */
extern SDL_Renderer *GFraMe_renderer;
#endif
/**
 * Whether the bounding box should be drawn
 */
int GFraMe_draw_debug = 0;

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
	GFraMe_hitbox_set(GFraMe_sprite_get_hitbox(spr),
					  GFraMe_hitbox_upper_left, 0, 0, w, h);
	// Now, set the sprite graphics
	spr->sset = sset;
	spr->cur_tile = 0;
	// Make sure no animation is running
	spr->anim = NULL;
	// Set the graphic's offset from the physical position
	spr->offset_x = ox;
	spr->offset_y = oy;
	// Set stuff to zero/one because... whatever
	spr->id = 0;
	spr->hp = 1;
	spr->is_visible = 1;
	spr->is_active = 1;
	spr->flipped = 0;
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
	int x = spr->obj.x;
	// Simply draw the current frame at the current position
	if (!spr->flipped)
		x += spr->offset_x;
	else
		x += -(spr->sset->tw - ((int)spr->obj.hitbox.hw * 2.0))
			 - spr->offset_x;
	GFraMe_spriteset_draw(spr->sset, spr->cur_tile,
			x, spr->obj.y + spr->offset_y,
			spr->flipped);
#ifdef GFRAME_DEBUG
	// If should draw the bounding box
	if (GFraMe_draw_debug) {
		// Get the sprite's hitbox
		GFraMe_hitbox *hb = &spr->obj.hitbox;
		
		// Create a SDL_Rect at its position
		SDL_Rect dbg_rect;
		dbg_rect.x = spr->obj.x + hb->cx - hb->hw;
		dbg_rect.y = spr->obj.y + hb->cy - hb->hh;
		dbg_rect.w = hb->hw * 2;
		dbg_rect.h = hb->hh * 2;
		// Render it to the screen, in red
		SDL_SetRenderDrawColor(GFraMe_renderer, 0xff, 0x00, 0x00, 0xff);
		SDL_RenderDrawRect(GFraMe_renderer, &dbg_rect);
	}
#endif
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

GFraMe_hitbox* GFraMe_sprite_get_hitbox(GFraMe_sprite *spr) {
	return &spr->obj.hitbox;
}

GFraMe_object* GFraMe_sprite_get_object(GFraMe_sprite *spr) {
	return &spr->obj;
}
GFraMe_tween* GFraMe_sprite_get_tween(GFraMe_sprite *spr) {
	return &spr->obj.tween;
}

