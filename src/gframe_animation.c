/**
 * @src/gframe_animation.c
 */
#include <GFraMe/GFraMe_animation.h>
#include <GFraMe/GFraMe_error.h>

/**
 * Initialize an animation	
 * @param	*anim	Animation to be initialized
 * @param	fps	At how many frames per second it should run
 * @param	*frames	Array of frames (int)
 * @param	num_frames	How many frames there are in the animation
 * @param	do_loop	Whether it should loop (1) or not (0)
 */
void GFrame_animation_init(
	GFraMe_animation *anim,
	int fps,
	int const *frames,
	int num_frames,
	int do_loop
) {
	// Set everything as was passed
	anim->frames = frames;
	anim->num_frames = num_frames;
	anim->do_loop = do_loop;
	// Calculate how long each frame should take, in milliseconds
	anim->frame_duration = 1000 / fps;
	// Issue a frame change after the normal time is elapsed
	anim->acc = anim->frame_duration;
	// Set it as never being finished
	anim->num_finished = 0;
	// Set it at the first position on the array
	anim->index = 0;
	// Set the current tile
	anim->tile = frames[0];
}

/**
 * Updates a animation, changing the frame, if needed
 * @param	*anim	Current animation
 * @param	ms	Time elapsed, in milliseconds
 * @return	GFraMe_ret_ok - if nothing happened; GFraMe_ret_anim_new_frame - if a new frame was issued; GFraMe_ret_anim_finished - if the animation finished running
 */
GFraMe_ret GFraMe_animation_update(GFraMe_animation *anim, int ms) {
	// Check if the animation already finished
	if (anim->acc <= 0)
		return GFraMe_ret_anim_finished;
	// Otherwise, decrease the timer
	anim->acc -= ms;
	// If a new frame should be issued
	if (anim->acc <= 0) {
		// Increase the current index
		anim->index++;
		// Check if it's within the frames array
		if (anim->index < anim->num_frames)
			// and set the time for the next frame
			anim->acc += anim->frame_duration;
		else {
			// Otherwise, increment how many times it has finished
			anim->num_finished++;
			// and loop, if enabled
			if (anim->do_loop) {
				anim->acc += anim->frame_duration;
				anim->index = 0;
			}
		}
		// Also, return that a new frame was issued
		return GFraMe_ret_anim_new_frame;
	}
	// Return that the function didn't break
	return GFraMe_ret_ok;
}

