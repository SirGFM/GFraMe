/**
 * @src/gframe_accumulator.c
 */
#include <GFraMe/GFraMe_accumulator.h>

/**
 * Start a new accumulator; this can only fail if fps is zero,
 * which is totally the user own fault!
 * @param	*acc	Accumulator to be (re)started
 * @param	fps	How many frames per second should be issued
 * @param	max_frames	At most, how many frames can be accumulated
 */
void GFraMe_accumulator_init_fps(GFraMe_accumulator *acc, int fps,
								int max_frames) {
	// Set everything as specified
	GFraMe_accumulator_set_fps(acc, fps, max_frames);
	// (Re)set elapsed to zero
	acc->elapsed = 0;
}

/**
 * Start a new accumulator
 * @param	*acc	Accumulator to be (re)started
 * @param	time	How long (in milliseconds) until a frame is issued
 * @param	max_time	At most, how long (in ms) can be accumulated
 */ 
void GFraMe_accumulator_init_time(GFraMe_accumulator *acc, int time,
								  int max_time) {
	// Set everything as specified and set elapsed to zero
	GFraMe_accumulator_init_set(acc, time, max_time);
	acc->elapsed = 0;
}

/**
 * Change the fps an accumulator is run at, without clearing the
 * already accumulated time; this can only fail if fps is zero,
 * which is totally the user own fault!
 * @param	*acc	Accumulator to be set
 * @param	fps	How many frames per second should be issued
 * @param	max_frames	At most, how many frames can be accumulated
 */
void GFraMe_accumulator_set_fps(GFraMe_accumulator *acc, int fps,
								int max_frames) {
	// Calculate timeout and set elapsed to zero
	acc->timeout = 1000 / fps;
	// Set cap as 10% before the next frame
	acc->cap = max_frames * acc->timeout + (int)(1000 / fps * 0.9f);
}

/**
 * Change an accumulator without clearing its accumulated time
 * which is totally the user own fault!
 * @param	*acc	Accumulator to be (re)started
 * @param	time	How long (in milliseconds) until a frame is issued
 * @param	max_time	At most, how long (in ms) can be accumulated
 */ 
void GFraMe_accumulator_init_set(GFraMe_accumulator *acc, int time,
								  int max_time) {
	// Set everything as specified
	acc->timeout = time;
	acc->cap = max_time;
}

/**
 * Update an accumulator
 * @param	*acc	Accumulator to be updated
 * @param	dt	How long, in milliseconds, has passed since its last update
 */
void GFraMe_accumulator_update(GFraMe_accumulator *acc, int dt) {
	// Update how long has elapsed
	acc->elapsed += dt;
	// Check if the accumulated time has overflown (lol)
	if (acc->elapsed > acc->cap)
		acc->elapsed = acc->cap;
}

/**
 * Check if the desired time was accumulated and decreases it; call this
 * in a loop for the update loop (running accumulated frames, if any)
 * @param	*acc	Accumulator to be checked
 * @return	GFraMe_new_acc_frame if a new frame was issued, GFraMe_ret_ok otherwise (makes looping easier)
 */
GFraMe_ret GFraMe_accumulator_loop(GFraMe_accumulator *acc) {
	// Check if any frame should be issued
	if (acc->elapsed >= acc->timeout) {
		// Decrease the elapsed time and issue a frame
		acc->elapsed -= acc->timeout;
		return GFraMe_ret_new_acc_frame;
	}
	// Do nothing
	return GFraMe_ret_ok;
}

