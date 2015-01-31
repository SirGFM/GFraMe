/**
 * @src/gframe_object.c
 */
#include <GFraMe/GFraMe_error.h>
#include <GFraMe/GFraMe_hitbox.h>
#include <GFraMe/GFraMe_object.h>
#include <GFraMe/GFraMe_tween.h>
#include <GFraMe/GFraMe_util.h>

/**
 * Clear every one of the object's attribute
 * @param	*obj	The object
 */
void GFraMe_object_clear(GFraMe_object *obj) {
	// Reset its position
	GFraMe_object_set_x(obj, 0);
	GFraMe_object_set_y(obj, 0);
	// Reset its velocity
	obj->vx = 0.0;
	obj->vx = 0.0;
	// Reset its acceleration
	obj->ax = 0.0;
	obj->ay = 0.0;
	// Reset collision
	obj->hit = GFraMe_direction_none;
	// Reset the hitbox
	GFraMe_hitbox_set(GFraMe_object_get_hitbox(obj), GFraMe_hitbox_center, 
					  0, 0, 0, 0);
	// Clear any tween
	GFraMe_tween_clear(GFraMe_object_get_tween(obj));
}

/**
 * Sets an object horizontal position
 * @param	*obj	The object
 * @param	X	New horizontal position
 */
void GFraMe_object_set_x(GFraMe_object *obj, int X) {
	// Set the horizontal position
	obj->x = X;
	obj->dx = (double)X;
	// Setting this avoids glitches on collision
	obj->ldx = (double)X;
}

/**
 * Sets an object vertical position
 * @param	*obj	The object
 * @param	Y	New vertical position
 */
void GFraMe_object_set_y(GFraMe_object *obj, int Y) {
	// Set the vertical position
	obj->y = Y;
	obj->dy = (double)Y;
	// Setting this avoids glitches on collision
	obj->ldy = (double)Y;
}

/**
 * Sets an object position
 * @param	*obj	The object
 * @param	X	New horizontal position
 * @param	Y	New vertical position
 */
void GFraMe_object_set_pos(GFraMe_object *obj, int X, int Y) {
	// Set the horizontal position
	GFraMe_object_set_x(obj, X);
	// Set the vertical position
	GFraMe_object_set_y(obj, Y);
}

/**
 * Updates an object's position, velocity and collision state
 * @param	*obj	Object to be update
 * @param	ms	How long this frame took
 */
void GFraMe_object_update(GFraMe_object *obj, int ms) {
	GFraMe_tween *tw;
	// Get the time in a nicer way to the integration
	double time = ((double)ms) / 1000.0;
	// Update last position (really important to collision)
	obj->ldx = obj->dx;
	obj->ldy = obj->dy;
	// Try to update the position by tweening
	tw = GFraMe_object_get_tween(obj);
	if (GFraMe_tween_update(tw, time) == GFraMe_tween_ret_ok) {
		GFraMe_tween_set_obj(tw, obj);
	}
	else {
		// Integrate the speed horizontally
		if (obj->ax != 0.0)
			obj->vx += GFraMe_util_integrate(obj->ax, time);
		// Integrate the position horizontally
		if (obj->vx != 0.0)
			obj->dx += GFraMe_util_integrate(obj->vx, time);
		// Integrate the velocity vertically
		if (obj->ay != 0.0)
			obj->vy += GFraMe_util_integrate(obj->ay, time);
		// Integrate the position vertically
		if (obj->vy != 0.0)
			obj->dy += GFraMe_util_integrate(obj->vy, time);
		// Set the actual display position
		obj->x = (int)obj->dx;
		obj->y = (int)obj->dy;
	}
	// Update the direction hit/that was hit
	obj->hit = (obj->hit << GFM_LAST_BITS) & GFraMe_direction_last;
}

/**
 * Overlaps two objects, according to the mode passed
 * @param	*o1	One of the objects to be overlaped
 * @param	*o2	The other object to be overlaped
 * @param	mode	How collision should be handle
 * @return	Whether an overlap occured (GFraMe_ret_ok) or not (GFraMe_ret_no_overlap)
 * NOTE if one of the objects is static and the other is manually
 * repositioned, overlap is detected but not handled
 */
GFraMe_ret GFraMe_object_overlap(GFraMe_object *o1, GFraMe_object *o2,
						   GFraMe_collision_type mode) {
	double hdist;
	double hmax;
	double vdist;
	double vmax;
	GFraMe_ret rv = GFraMe_ret_no_overlap;
	
	// Compute horizontal distance
	hdist = o2->dx + o2->hitbox.cx - o1->dx - o1->hitbox.cx;
	// Compute vertical distance
	vdist = o2->dy + o2->hitbox.cy - o1->dy - o1->hitbox.cy;
	// Compute maximum horizontal distance for overlap
	hmax = o2->hitbox.hw + o1->hitbox.hw;
	// Compute maximum vertical distance for overlap
	vmax = o2->hitbox.hh + o1->hitbox.hh;
	// Check if the centers are closer than the maximum distances
	if (GFraMe_util_absd(hdist) < hmax && GFraMe_util_absd(vdist) < vmax) {
		// Set overlap as having happened this frame only
		int vcol = 1;
		int hcol = 1;
		//==============================================================//
		// Note about my overlap implementation                         //
		//==============================================================//
		// If last frame they were already overlaping, then they aren't //
		// actually overlapping on that "direction" (two objects may    //
		// intersect vertically but not horizontally, e.g.:             //
		//==============================================================//
		//            time 0           -->           time 1             //
		//==============================|===============================//
		//             ----             |                               //
		//             |  |             |                               //
		//             ----             |             ----              //
		//           --------           |           --|--|--            //
		//           |      |           |           | ---- |            //
		//           --------           |           --------            //
		//==============================================================//
		
		// Check if it was already overlaping horizontally and clear it
		if (GFraMe_util_absd(o2->ldx + o2->hitbox.cx - 
							  o1->ldx - o1->hitbox.cx) < hmax) {
			hcol = 0;
		}
		// Check if it was already overlaping vertically and clear it
		if (GFraMe_util_absd(o2->ldy + o2->hitbox.cy -
							 o1->ldy - o1->hitbox.cy) < vmax) {
			vcol = 0;
		}
		
		// Don't use the set function because it only works for integers!!
		
		// If both objects should be moved to avoid visual collision
		if (mode == GFraMe_collision_full) {
			// If they weren't overlapping horizontally, separate it
			if (hcol) {
				// Move them by the same amount
				o1->dx -= hdist * 0.5;
				o2->dx += hdist * 0.5;
				// Update the actual position!!
				o1->x = (int)o1->dx;
				o2->x = (int)o2->dx;
			}
			// If they weren't overlapping vertically, separate it
			if (vcol) {
				// Move them by the same amount
				o1->dy -= vdist * 0.5;
				o2->dy += vdist * 0.5;
				// Update the actual position!!
				o1->y = (int)o1->dy;
				o2->y = (int)o2->dy;
			}
		}
		// Otherwise, if should actually collide (instead of only flag it)
		else if (mode != GFraMe_dont_collide) {
			GFraMe_object *cur, *other;
			// Decide which object should be moved (cur) and
			// which is the static one (other)
			switch (mode) {
				case GFraMe_first_fixed:  cur = o2; other = o1; break;
				case GFraMe_second_fixed: cur = o1; other = o2; break;
				default: goto _aftercol; // Avoids accessing null objects
			}
			// If they weren't overlapping horizontally, separate it
			if (hcol) {
				// Position it exactly grazing horizontally
				cur->dx = other->dx + other->hitbox.cx - cur->hitbox.cx;
				if (cur->ldx + cur->hitbox.cx > other->ldx + other->hitbox.cx) {
                    // GFraMe_log("right");
					cur->dx += hmax;
                }
				else {
                    // GFraMe_log("left");
					cur->dx -= hmax;
                }
				// Update the actual position!!
				cur->x = (int)cur->dx;
			}
			// If they weren't overlapping vertically, separate it
			if (vcol) {
				// Position it exactly grazing vertically
				cur->dy = other->dy + other->hitbox.cy - cur->hitbox.cy;
				if (cur->ldy + cur->hitbox.cy > other->ldy + other->hitbox.cy) {
                    // GFraMe_log("bellow");
					cur->dy += vmax;
                }
				else {
                    // GFraMe_log("above");
					cur->dy -= vmax;
                }
				// Update the actual position!!
				cur->y = (int)cur->dy;
			}
		}
_aftercol:
		// TODO this will probably clear overlap after one frame...
		// is it a problem?
		
		// Now, set all the flags
		// If they are overlapping horizontally
		if (hcol) {
			// Check which was farther to the right/left
			if (hdist > 0) {
				o1->hit |= GFraMe_direction_right;
				o2->hit |= GFraMe_direction_left;
			}
			else {
				o1->hit |= GFraMe_direction_left;
				o2->hit |= GFraMe_direction_right;
			}
		}
		// If they are overlapping vertically
		if (vcol) {
			// Check which was higher/lower
			if (vdist > 0) {
				o1->hit |= GFraMe_direction_down;
				o2->hit |= GFraMe_direction_up;
			}
			else {
				o1->hit |= GFraMe_direction_up;
				o2->hit |= GFraMe_direction_down;
			}
		}
		// Signal as overlap happening
		rv = GFraMe_ret_ok;
	}
	// Return result
	return rv;
}

GFraMe_hitbox *GFraMe_object_get_hitbox(GFraMe_object *obj) {
	return &obj->hitbox;
}

GFraMe_tween *GFraMe_object_get_tween(GFraMe_object *obj) {
	return &obj->tween;
}

