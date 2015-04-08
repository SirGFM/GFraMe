/**
 * @file src/core/sdl2/gframe_timer.c
 * 
 * Timer module. Should signal whenever a new frame is to be issued.
 * This is a SDL2 implementation.
 */
#include <GFraMe/gfmAssert.h>
#include <GFraMe/core/gfmTime_bkend.h>

#include <SDL2/SDL.h>

#include <stdlib.h>

struct stGFMTimer {
    /** At how many FPS this timer is set */
    int fps;
    /** How long (in milliseconds) between each "timer interrupt" */
    int interval;
    /** The SDL2 timer handle */
    SDL_TimerID timer;
    /** Event that will be pushed on every callback execution */
    SDL_Event event;
    /** Detailes of event that will be pushed on every callback execution */
    SDL_UserEvent userevent;
};

/** 'Exportable' size of gfmTimer */
const size_t sizeofGFMTimer = sizeof(struct stGFMTimer);

/**
 * Callback to be called by SDL each time a time event is issued.
 */
static Uint32 gfmTimer_callback(Uint32 interval, void *param);

/**
 * Get how long each frame must take for the timer function;
 * The result is rounded down to the nearest multiple of ten
 * 
 * @param  fps  How many frames should run per second
 * @return      How long each frame must take, in milliseconds
 */
int gfmTimer_getMs(int fps) {
    return gfmTimer_getMsRaw(fps) / 10 * 10;
}

/**
 * Get how long each frame must take for the timer function
 * 
 * @param  fps  How many frames should run per second
 * @return      How long each frame must take, in milliseconds
 */
int gfmTimer_getMsRaw(int fps) {
	return 1000 / fps;
}

/**
 * Initialize (i.e., alloc) a new timer; If the parameter doesn't point to
 * NULL, the function will return GFMRV_ARGUMENTS_BAD (as it could be pointing
 * to a valid value)
 * 
 * @param  ppSelf The timer
 * @return        GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_ALLOC_FAILED
 */
gfmRV gfmTimer_new(gfmTimer **ppSelf) {
    gfmRV rv;
    
    // Sanitize the arguments
    ASSERT(ppSelf, GFMRV_ARGUMENTS_BAD);
    ASSERT(!(*ppSelf), GFMRV_ARGUMENTS_BAD);
    
    // Alloc and initialize the structure
    *ppSelf = (gfmTimer*)malloc(sizeofGFMTimer);
    ASSERT(*ppSelf, GFMRV_ALLOC_FAILED);
    (*ppSelf)->fps = 0;
    (*ppSelf)->interval = 0;
    (*ppSelf)->timer = 0;
    
    // Set SDL specific stuff
	// Set the event data as an user event
	// TODO change it to something more specific
    (*ppSelf)->userevent.type = SDL_USEREVENT;
    (*ppSelf)->userevent.code = 0;
    (*ppSelf)->userevent.data1 = NULL;
    (*ppSelf)->userevent.data2 = NULL;
	// Set the event type as an user event
    (*ppSelf)->event.type = SDL_USEREVENT;
    (*ppSelf)->event.user = (*ppSelf)->userevent;
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Releases a timer's resources; This function MUST stop the timer if it's
 * still running
 * 
 * @param  ppSelf The timer
 * @return        GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmTimer_free(gfmTimer **ppSelf) {
    gfmRV rv;
    
    // Sanitize the arguments
    ASSERT(ppSelf, GFMRV_ARGUMENTS_BAD);
    ASSERT(*ppSelf, GFMRV_ARGUMENTS_BAD);
    
    // Stop the timer, if it's running
    if ((*ppSelf)->timer) {
        gfmTimer_stop(*ppSelf);
    }
    
    // Dealloc the resources
    free(*ppSelf);
    *ppSelf = 0;
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Initiate a timer
 * The fps's interval is rounded down to the nearest multiple of ten
 * 
 * @param  pSelf The timer
 * @param  fps   The desired FPS
 * @return       GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_FPS_TOO_HIGH,
 *               GFMRV_FAILED_TO_INIT_TIMER
 */
gfmRV gfmTimer_init(gfmTimer *pSelf, int fps) {
    gfmRV rv;
    
    // Sanitize the arguments
    ASSERT(pSelf, GFMRV_ARGUMENTS_BAD);
    ASSERT(fps > 0, GFMRV_ARGUMENTS_BAD);
    // Check that the FPS is valid
    pSelf->fps = fps;
    pSelf->interval = gfmTimer_getMs(pSelf->fps);
    ASSERT(pSelf->interval > 0, GFMRV_FPS_TOO_HIGH);
    // Try to create the timer
    pSelf->timer = SDL_AddTimer(pSelf->interval, gfmTimer_callback, pSelf);
    ASSERT(pSelf->timer, GFMRV_FAILED_TO_INIT_TIMER);
    
    rv = GFMRV_OK;
__ret:
    // Clean up on error
    if (rv != GFMRV_OK && rv != GFMRV_ARGUMENTS_BAD) {
        pSelf->fps = 0;
        pSelf->interval = 0;
    }
    
    return rv;
}

/**
 * Initiate a timer
 * 
 * @param  pSelf The timer
 * @param  fps   The desired FPS
 * @return       GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_FPS_TOO_HIGH
 */
gfmRV gfmTimer_initRaw(gfmTimer *pSelf, int fps) {
    gfmRV rv;
    
    // Sanitize the arguments
    ASSERT(pSelf, GFMRV_ARGUMENTS_BAD);
    ASSERT(fps > 0, GFMRV_ARGUMENTS_BAD);
    // Check that the FPS is valid
    pSelf->fps = fps;
    pSelf->interval = gfmTimer_getMsRaw(pSelf->fps);
    ASSERT(pSelf->interval > 0, GFMRV_FPS_TOO_HIGH);
    // Try to create the timer
    pSelf->timer = SDL_AddTimer(pSelf->interval, gfmTimer_callback, pSelf);
    ASSERT(pSelf->timer, GFMRV_FAILED_TO_INIT_TIMER);
    
    rv = GFMRV_OK;
__ret:
    // Clean up on error
    if (rv != GFMRV_OK && rv != GFMRV_ARGUMENTS_BAD) {
        pSelf->fps = 0;
        pSelf->interval = 0;
    }
    
    return rv;
}

/**
 * Stops a timer
 * 
 * @param  pSelf The timer
 * @return       GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_TIMER_NOT_INITIALIZED,
 *               GFMRV_FAILED_TO_STOP_TIMER
 */
gfmRV gfmTimer_stop(gfmTimer *pSelf) {
    gfmRV rv;
    SDL_bool ret;
    
    // Sanitize the argments
    ASSERT(pSelf, GFMRV_ARGUMENTS_BAD);
    ASSERT(pSelf->timer, GFMRV_TIMER_NOT_INITIALIZED);
    // Try to remove the timer
    ret = SDL_RemoveTimer(pSelf->timer);
    ASSERT(ret == SDL_TRUE, GFMRV_FAILED_TO_STOP_TIMER);
    // Clean up the context
    pSelf->timer = 0;
    pSelf->interval = 0;
    pSelf->fps = 0;
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Changes a timer's FPS
 * The result is rounded down to the nearest multiple of ten
 * 
 * @param  pSelf The timer
 * @param  fps   The desired FPS
 * @return       GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_TIMER_NOT_INITIALIZED,
 *               GFMRV_FPS_TOO_HIGH
 */
gfmRV gfmTimer_setFPS(gfmTimer *pSelf, int fps) {
    gfmRV rv;
    int interval;
    
    // Sanitize the arguments
    ASSERT(pSelf, GFMRV_ARGUMENTS_BAD);
    ASSERT(pSelf->timer, GFMRV_TIMER_NOT_INITIALIZED);
    // Check if it's a valid FPS
    interval = gfmTimer_getMs(fps);
    ASSERT(interval > 0, GFMRV_FPS_TOO_HIGH);
    // Set the new fps
    pSelf->interval = interval;
    pSelf->fps = fps;
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Changes a timer's FPS
 * 
 * @param  pSelf The timer
 * @param  fps   The desired FPS
 * @return       GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_TIMER_NOT_INITIALIZED,
 *               GFMRV_FPS_TOO_HIGH
 */
gfmRV gfmTimer_setFPSRaw(gfmTimer *pSelf, int fps) {
    gfmRV rv;
    int interval;
    
    // Sanitize the arguments
    ASSERT(pSelf, GFMRV_ARGUMENTS_BAD);
    ASSERT(pSelf->timer, GFMRV_TIMER_NOT_INITIALIZED);
    // Check if it's a valid FPS
    interval = gfmTimer_getMsRaw(fps);
    ASSERT(interval > 0, GFMRV_FPS_TOO_HIGH);
    // Set the new fps
    pSelf->interval = interval;
    pSelf->fps = fps;
    
    rv = GFMRV_OK;
__ret:
    return rv;
}


/**
 * Callback to be called by SDL each time a time event is issued
 * 
 * @param  interval Delay from the previous callback
 * @param  param    gfmTimer structure (to retrieve the next interval)
 * @return          Delay to the next callback
 */
static Uint32 gfmTimer_callback(Uint32 interval, void *param) {
	// Add it to the event queue (so the main thread can see it)
    SDL_PushEvent(&((gfmTimer*)param)->event);
    
	// Return how long till the next callback (usually, the same amount)
    return ((gfmTimer*)param)->interval;
}

