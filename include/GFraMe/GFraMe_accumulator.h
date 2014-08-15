/**
 * @include/GFraMe/GFraMe_accumulator.h
 */
#ifndef __GFRAME_ACCUMULATOR_H
#define __GFRAME_ACCUMULATOR_H

#include <GFraMe/GFraMe_error.h>

struct stGFraMe_accumulator {
	/**
	 * How many milliseconds elapsed from the last frame/check
	 */
	int elapsed;
	/**
	 * When should a new frame be issued
	 */
	int timeout;
	/**
	 * For looping accumulators, when should it be stopped;
	 * (useful for avoiding spirals [too many loops accumulated ->
	 * update/render them -> even more loops accumulated -> rep.])
	 */
	int cap;
};
typedef struct stGFraMe_accumulator GFraMe_accumulator;

/**
 * Start a new accumulator; this can only fail if fps is zero,
 * which is totally the user own fault!
 * @param	*acc	Accumulator to be (re)started
 * @param	fps	How many frames per second should be issued
 * @param	max_frames	At most, how many frames can be accumulated
 */
void GFraMe_accumulator_init_fps(GFraMe_accumulator *acc, int fps,
								int max_frames);

/**
 * Start a new accumulator
 * @param	*acc	Accumulator to be (re)started
 * @param	time	How long (in milliseconds) until a frame is issued
 * @param	max_time	At most, how long (in ms) can be accumulated
 */ 
void GFraMe_accumulator_init_time(GFraMe_accumulator *acc, int time,
								  int max_time);

/**
 * Change the fps an accumulator is run at, without clearing the
 * already accumulated time; this can only fail if fps is zero,
 * which is totally the user own fault!
 * @param	*acc	Accumulator to be set
 * @param	fps	How many frames per second should be issued
 * @param	max_frames	At most, how many frames can be accumulated
 */
void GFraMe_accumulator_set_fps(GFraMe_accumulator *acc, int fps,
								int max_frames);

/**
 * Change an accumulator without clearing its accumulated time
 * which is totally the user own fault!
 * @param	*acc	Accumulator to be (re)started
 * @param	time	How long (in milliseconds) until a frame is issued
 * @param	max_time	At most, how long (in ms) can be accumulated
 */ 
void GFraMe_accumulator_init_set(GFraMe_accumulator *acc, int time,
								  int max_time);

/**
 * Update an accumulator
 * @param	*acc	Accumulator to be updated
 * @param	dt	How long, in milliseconds, has passed since its last update
 */
void GFraMe_accumulator_update(GFraMe_accumulator *acc, int dt);

/**
 * Check if the desired time was accumulated and decreases it; call this
 * in a loop for the update loop (running accumulated frames, if any)
 * @param	*acc	Accumulator to be checked
 * @return	GFraMe_ret_new_acc_frame if a new frame was issued, GFraMe_ret_ok otherwise (makes looping easier)
 */
GFraMe_ret GFraMe_accumulator_loop(GFraMe_accumulator *acc);

#endif

