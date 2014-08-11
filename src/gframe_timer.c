/**
 * @src/gframe_timer.c
 */
#include <GFraMe/GFraMe_timer.h>
#include <SDL2/SDL.h>

/**
 * Callback to be called by SDL each time a time event is issued.
 */
static Uint32 simple_callback(Uint32 interval, void *param);

/**
 * Get how long each frame must take for the timer function
 * @param	fps	How many frames should run per second
 * @return	How long each frame must take, in milliseconds
 */
int GFraMe_timer_get_ms(int fps) {
	// Apparently, SDL requires this granularity where interval must be
	// a multiple of 10...
	return 1000 / fps / 10 * 10;
}

/**
 * Initiate a timer
 * @param	ms	How long between each call to the callback
 * @param	GFraMe_timer	Where the timer will be stored
 * @return	GFraMe_ret_ok - Success; Anything else - Failure
 */
GFraMe_ret GFraMe_timer_init(int ms, GFraMe_timer *timer) {
	GFraMe_ret rv = GFraMe_ret_ok;
	// GFraMe_timer and SDL_TimerID are the same, so...
	SDL_TimerID _timer;
	
	// Try to add a timer
	_timer = SDL_AddTimer(ms, simple_callback, NULL);
	GFraMe_SDLassertRV(_timer, "Couldn't create timer", rv = GFraMe_ret_timer_creation_failed, _ret);
	// Set return (remember, they mean both the same thing)
	*timer = _timer;
_ret:
	return rv;
}

/**
 * Stops a timer
 * @param	timer	Which timer to stop
 * @return	GFraMe_ret_ok - Success; Anything else - Failure
 */
GFraMe_ret GFraMe_timer_stop(GFraMe_timer timer) {
	GFraMe_ret rv = GFraMe_ret_ok;
	SDL_bool ret;
	// Try to remove the timer
	ret = SDL_RemoveTimer(timer);
	// Don't ask me what to do if this fails!
	GFraMe_assertRV(ret == SDL_TRUE, "Couldn't stop requested timer",
					rv = GFraMe_ret_stop_timer_failed, _ret);
_ret:
	return rv;
}

/**
 * Callback to be called by SDL each time a time event is issued.
 */
static Uint32 simple_callback(Uint32 interval, void *param) {
    SDL_Event event;
    SDL_UserEvent userevent;
	// Set the event data as an user event
	// TODO change it to something more specific
    userevent.type = SDL_USEREVENT;
    userevent.code = 0;
    userevent.data1 = NULL;
    userevent.data2 = NULL;
	// Set the event type as an user event
    event.type = SDL_USEREVENT;
    event.user = userevent;
	// Add it to the event queue (so the main thread can see it)
    SDL_PushEvent(&event);
	// Return how long till the next callback (usually, the same amount)
    return interval;
}

