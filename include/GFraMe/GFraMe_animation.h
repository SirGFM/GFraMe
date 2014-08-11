/**
 * @include/GFraMe/GFraMe_animation.h
 */
#ifndef __GFRAME_ANIMATION_H_
#define __GFRAME_ANIMATION_H_

#include <GFraMe/GFraMe_error.h>
#include <GFraMe/GFraMe_spriteset.h>

/**
 * Struct that defines a animation
 */
struct stGFraMe_animation {
	/**
	 * Frames indexes in the spriteset 
	 */
	int const *frames;
	/**
	 * How many frames there are
	 */
	int num_frames;
	/**
	 * Whether the animation should loop
	 */
	int do_loop;
	/**
	 * Current frame in the frames array
	 */
	int index;
	/**
	 * How long, in milliseconds, a frame should last
	 */
	int frame_duration;
	/**
	 * Decreasing accumulator, if this makes any sense;
	 * when it's less or equal to zero, issue a new frame
	 */
	int acc;
	/**
	 * How many times the animations has finished, i.e. it looped
	 */
	int num_finished;
	/**
	 * Current tile being displayed
	 */
	int tile;
};
typedef struct stGFraMe_animation GFraMe_animation;
 
/**
 * Initialize an animation	
 * @param	*anim	Animation to be initialized
 * @param	fps	At how many frames per second it should run
 * @param	*frames	Array of frames (int)
 * @param	num_frames	How many frames there are in the animation
 * @param	do_loop	Whether it should loop (1) or not (0)
 */
void GFraMe_animation_init(
	GFraMe_animation *anim,
	int fps,
	int const *frames,
	int num_frames,
	int do_loop
);

/**
 * Updates a animation, changing the frame, if needed
 * @param	*anim	Current animation
 * @param	ms	Time elapsed, in milliseconds
 * @return	GFraMe_ret_ok - if nothing happened; GFraMe_ret_anim_new_frame - if a new frame was issued; GFraMe_ret_anim_finished - if the animation finished running
 */
GFraMe_ret GFraMe_animation_update(GFraMe_animation *anim, int ms);
 
#endif

