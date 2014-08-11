/**
 * @include/GFraMe/GFraMe_timer.h
 */
#ifndef __GFRAME_TIMER_H_
#define __GFRAME_TIMER_H_

#include <GFraMe/GFraMe_error.h>
#include <SDL2/SDL.h>

/**
 * A simple typedef so (explicitly) including SDL2 isn't necessary
 */
typedef SDL_TimerID GFraMe_timer;

/**
 * Get how long each frame must take for the timer function
 * @param	fps	How many frames should run per second
 * @return	How long each frame must take, in milliseconds
 */
int GFraMe_timer_get_ms(int fps);

/**
 * Initiate a timer
 * @param	ms	How long between each call to the callback
 * @param	GFraMe_timer	Where the timer will be stored
 * @return	GFraMe_ret_ok - Success; Anything else - Failure
 */
GFraMe_ret GFraMe_timer_init(int ms, GFraMe_timer *timer);

/**
 * Stops a timer
 * @param	timer	Which timer to stop
 * @return	GFraMe_ret_ok - Success; Anything else - Failure
 */
GFraMe_ret GFraMe_timer_stop(GFraMe_timer timer);

#endif

