/**
 * @include/GFraMe/GFraMe_object.h
 */
#ifndef __GFRAME_OBJECT_H_
#define __GFRAME_OBJECT_H_

#include <GFraMe/GFraMe_error.h>

#define GAME_HORIZONTAL_COLLISION	1
#define GAME_VERTICAL_COLLISION		2
#define GAME_COLLISION_OK			(GAME_HORIZONTAL_COLLISION | GAME_VERTICAL_COLLISION)

/**
 * Enumeration that defines the directions
 */
enum enGFraMe_direction {
	GFraMe_direction_none       = 0x00,
	GFraMe_direction_left       = 0x01,
	GFraMe_direction_right      = 0x02,
	GFraMe_direction_up         = 0x04,
	GFraMe_direction_down       = 0x08,
	GFraMe_direction_now        = 0x0f,
	GFraMe_direction_last_left  = 0x10,
	GFraMe_direction_last_right = 0x20,
	GFraMe_direction_last_up    = 0x40,
	GFraMe_direction_last_down  = 0x80,
	GFraMe_direction_last       = 0xf0,
};
typedef enum enGFraMe_direction GFraMe_direction;
#define GFM_LAST_BITS 8

/**
 * Enumeration that defines the collision types
 */
enum enGFraMe_collision_type {
	/**
	 * Don't collide at all (only set flags)
	 */
	GFraMe_dont_collide = 0,
	/**
	 * Don't move the first object (parameter)
	 */
	GFraMe_first_fixed,
	/**
	 * Don't move the second object (parameter)
	 */
	GFraMe_second_fixed,
	/**
	 * Adjust both objects position
	 */
	GFraMe_collision_full
};
typedef enum enGFraMe_collision_type GFraMe_collision_type;

/**
 * Enumeration that defines what the position passed to 'set_hitbox' means
 */
enum stGFraMe_set_hitbox_anchor {
	/**
	 * Set the position as the center
	 */
	GFraMe_set_hitbox_center = 0,
	/**
	 * Set the position as the upper left corner
	 */
	GFraMe_set_hitbox_upper_left,
	/**
	 * Set the position as the upper right corner
	 */
	GFraMe_set_hitbox_upper_right,
	/**
	 * Set the position as the lower right corner
	 */
	GFraMe_set_hitbox_lower_right,
	/**
	 * Set the position as the lower left corner
	 */
	GFraMe_set_hitbox_lower_left
};
typedef enum stGFraMe_set_hitbox_anchor GFraMe_set_hitbox_anchor;

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

struct stGFraMe_object {
	/**
	 * Current horizontal position (read only!);
	 * used to render (should be used for position checks)
	 */
	int x;
	/**
	 * Current vertical position (read only!);
	 * used to render (should be used for position checks)
	 */
	int y;
	/**
	 * Actual horizontal position (read only!);
	 * should only be used internally
	 */
	double dx;
	/**
	 * Actual vertical position (read only!);
	 * should only be used internally
	 */
	double dy;
	/**
	 * Last horizontal possition (read only!);
	 * used to decide previous position on collision
	 */
	double ldx;
	/**
	 * Last vertical possition (read only!);
	 * used to decide previous position on collision
	 */
	double ldy;
	/**
	 * Current horizontal velocity
	 */
	double vx;
	/**
	 * Current vertical velocity
	 */
	double vy;
	/**
	 * Current horizontal acceleration
	 */
	double ax;
	/**
	 * Current vertical acceleration
	 */
	double ay;
	/**
	 * Just/last collided direction(s)
	 */
	GFraMe_direction hit;
	/**
	 * The object's hitbox
	 */
	GFraMe_hitbox hitbox;
};
typedef struct stGFraMe_object GFraMe_object;

/**
 * Clear every one of the object's attribute
 * @param	*obj	The object
 */
void GFraMe_object_clear(GFraMe_object *obj);

/**
 * Sets an object horizontal position
 * @param	*obj	The object
 * @param	X	New horizontal position
 */
void GFraMe_object_set_x(GFraMe_object *obj, int X);

/**
 * Sets an object vertical position
 * @param	*obj	The object
 * @param	Y	New vertical position
 */
void GFraMe_object_set_y(GFraMe_object *obj, int Y);

/**
 * Sets an object position
 * @param	*obj	The object
 * @param	X	New horizontal position
 * @param	Y	New vertical position
 */
void GFraMe_object_set_pos(GFraMe_object *obj, int X, int Y);

/**
 * Sets the object's hitbox as specified by the anchor;
 * notice, however, that it'll be stored as the center position
 * @param	*obj	The object
 */
void GFraMe_object_set_hitbox(GFraMe_object *obj,
							  GFraMe_set_hitbox_anchor anchor,
							  int x, int y, int w, int h);

/**
 * Updates an object's position, velocity and collision state
 * @param	*obj	Object to be update
 * @param	ms	How long this frame took
 */
void GFraMe_object_update(GFraMe_object *obj, int ms);

/**
 * Overlaps two objects, according to the mode passed
 * @param	*o1	One of the objects to be overlaped
 * @param	*o2	The other object to be overlaped
 * @param	mode	How collision should be handle
 * @return	Whether an overlap occured (GFraMe_ret_ok) or not (GFraMe_ret_no_overlap)
 */
GFraMe_ret GFraMe_object_overlap(GFraMe_object *o1, GFraMe_object *o2,
						   GFraMe_collision_type mode);

#endif

