/**
 * @src/gframe_hitbox.c
 */
#include <GFraMe/GFraMe_hitbox.h>

/**
 * Sets the hitbox as specified by the anchor;
 * notice, however, that it'll be stored as the center position
 */
void GFraMe_hitbox_set(GFraMe_hitbox *hb, GFraMe_hitbox_anchor anchor,
					   int x, int y, int w, int h) {
	// Macro to set center and make code cleaner
	#define center(X, Y) \
		hb->cx = X; \
		hb->cy = Y
	// Sets the hitbox dimensions
	hb->hw = w * 0.5;
	hb->hh = h * 0.5;
	// Now, set it's central position, according to the anchor
	switch (anchor) {
		case GFraMe_hitbox_center:      center(x        , y        ); break;
		case GFraMe_hitbox_upper_left:  center(x + w*0.5, y + h*0.5); break;
		case GFraMe_hitbox_upper_right: center(x - w*0.5, y + h*0.5); break;
		case GFraMe_hitbox_lower_right: center(x - w*0.5, y - h*0.5); break;
		case GFraMe_hitbox_lower_left:  center(x + w*0.5, y - h*0.5); break;
		default: center(0.0, 0.0); break;
	}
	// Clear the macro so it doesn't mess any other code
	#undef center
}

