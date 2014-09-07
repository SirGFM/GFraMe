/**
 * @include/GFraMe/GFraMe_hitbox.h
 */
#ifndef __GFRAME_HITBOX_H
#define __GFRAME_HITBOX_H

/**
 * Enumeration that defines what the position passed to 'set_hitbox' means
 */
enum stGFraMe_hitbox_anchor {
	/**
	 * Set the position as the center
	 */
	GFraMe_hitbox_center = 0,
	/**
	 * Set the position as the upper left corner
	 */
	GFraMe_hitbox_upper_left,
	/**
	 * Set the position as the upper right corner
	 */
	GFraMe_hitbox_upper_right,
	/**
	 * Set the position as the lower right corner
	 */
	GFraMe_hitbox_lower_right,
	/**
	 * Set the position as the lower left corner
	 */
	GFraMe_hitbox_lower_left
};
typedef enum stGFraMe_hitbox_anchor GFraMe_hitbox_anchor;

/**
 * Strucure that defines an object hitbox
 */
struct stGFraMe_hitbox {
	/**
	 * Offset to horizontal center
	 */
	double cx;
	/**
	 * Offset to vertical center
	 */
	double cy;
	/**
	 * Half width
	 */
	double hw;
	/**
	 * Half height
	 */
	double hh;
};
typedef struct stGFraMe_hitbox GFraMe_hitbox;

/**
 * Sets the hitbox as specified by the anchor;
 * notice, however, that it'll be stored as the center position
 */
void GFraMe_hitbox_set(GFraMe_hitbox *hb, GFraMe_hitbox_anchor anchor,
					   int x, int y, int w, int h);

#endif

