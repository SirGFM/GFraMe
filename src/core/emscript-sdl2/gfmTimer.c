/**
 * @file src/core/emscript-sdl2/gframe_timer.c
 * 
 * Timer module. Should signal whenever a new frame is to be issued.
 * This is a SDL2 implementation.
 */
#include <GFraMe/gfmAssert.h>
#include <GFraMe/core/gfmEvent_bkend.h>
#include <GFraMe/core/gfmTimer_bkend.h>

#include <SDL2/SDL_timer.h>

#include <stdlib.h>
#include <string.h>

struct stGFMTimer {
    /** Pointer to the event context */
    gfmEvent *pEvent;
    /** At how many FPS this timer is set */
    int fps;
    /** How long (in milliseconds) between each "timer interrupt" */
    int interval;
};

/** 'Exportable' size of gfmTimer */
const int sizeofGFMTimer = sizeof(struct stGFMTimer);

/**
 * Get how long each frame must take for the timer function;
 * The result is rounded down to the nearest multiple of ten
 * 
 * @param  fps How many frames should run per second
 * @return     How long each frame must take, in milliseconds
 */
int gfmTimer_getMs(int fps) {
    return gfmTimer_getMsRaw(fps) / 10 * 10;
}

/**
 * Get how long each frame must take for the timer function
 * 
 * @param  fps How many frames should run per second
 * @return     How long each frame must take, in milliseconds
 */
int gfmTimer_getMsRaw(int fps) {
	return 1000 / fps;
}

/**
 * Initialize (i.e., alloc) a new timer; If the parameter doesn't point to
 * NULL, the function will return GFMRV_ARGUMENTS_BAD (as it could be pointing
 * to a valid value)
 * 
 * @param  ppTimer The timer
 * @param  pCtx    The game's context
 * @return         GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_ALLOC_FAILED
 */
gfmRV gfmTimer_getNew(gfmTimer **ppTimer, gfmCtx *pCtx) {
    gfmRV rv;
    
    // Sanitize the arguments
    ASSERT(ppTimer, GFMRV_ARGUMENTS_BAD);
    ASSERT(!(*ppTimer), GFMRV_ARGUMENTS_BAD);
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    
    // Alloc and initialize the structure
    *ppTimer = (gfmTimer*)malloc(sizeofGFMTimer);
    ASSERT(*ppTimer, GFMRV_ALLOC_FAILED);
    memset(*ppTimer, 0x0, sizeof(gfmTimer));
    
    // Get the game's event context
    rv = gfm_getEventCtx(&((*ppTimer)->pEvent), pCtx);
    ASSERT_NR(rv == GFMRV_OK);
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Releases a timer's resources; This function MUST stop the timer if it's
 * still running
 * 
 * @param  ppCtx The timer
 * @return       GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmTimer_free(gfmTimer **ppCtx) {
    gfmRV rv;
    
    // Sanitize the arguments
    ASSERT(ppCtx, GFMRV_ARGUMENTS_BAD);
    ASSERT(*ppCtx, GFMRV_ARGUMENTS_BAD);
    
    // Dealloc the resources
    free(*ppCtx);
    *ppCtx = 0;
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Initiate a timer
 * The fps's interval is rounded down to the nearest multiple of ten
 * 
 * @param  pCtx The timer
 * @param  fps  The desired FPS
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_FPS_TOO_HIGH,
 *              GFMRV_INTERNAL_ERROR, GFMRV_TIMER_ALREADY_INITIALIZED
 */
gfmRV gfmTimer_init(gfmTimer *pCtx, int fps) {
    gfmRV rv;
    
    // Sanitize the arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    ASSERT(fps > 0, GFMRV_ARGUMENTS_BAD);
    // Check that the FPS is valid
    pCtx->fps = fps;
    pCtx->interval = gfmTimer_getMs(pCtx->fps);
    ASSERT(pCtx->interval > 0, GFMRV_FPS_TOO_HIGH);
    
    rv = GFMRV_OK;
__ret:
    // Clean up on error
    if (rv != GFMRV_OK && rv != GFMRV_ARGUMENTS_BAD) {
        pCtx->fps = 0;
        pCtx->interval = 0;
    }
    
    return rv;
}

/**
 * Initiate a timer
 * 
 * @param  pCtx The timer
 * @param  fps  The desired FPS
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_FPS_TOO_HIGH,
 *              GFMRV_INTERNAL_ERROR, GFMRV_TIMER_ALREADY_INITIALIZED
 */
gfmRV gfmTimer_initRaw(gfmTimer *pCtx, int fps) {
    gfmRV rv;
    
    // Sanitize the arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    ASSERT(fps > 0, GFMRV_ARGUMENTS_BAD);
    // Check that the FPS is valid
    pCtx->fps = fps;
    pCtx->interval = gfmTimer_getMsRaw(pCtx->fps);
    ASSERT(pCtx->interval > 0, GFMRV_FPS_TOO_HIGH);
    
    rv = GFMRV_OK;
__ret:
    // Clean up on error
    if (rv != GFMRV_OK && rv != GFMRV_ARGUMENTS_BAD) {
        pCtx->fps = 0;
        pCtx->interval = 0;
    }
    
    return rv;
}

/**
 * Stops a timer
 * 
 * @param  pCtx The timer
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_TIMER_NOT_INITIALIZED,
 *              GFMRV_FAILED_TO_STOP_TIMER
 */
gfmRV gfmTimer_stop(gfmTimer *pCtx) {
    gfmRV rv;
    
    // Sanitize the argments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    // Clean up the context
    pCtx->interval = 0;
    pCtx->fps = 0;
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Changes a timer's FPS
 * The result is rounded down to the nearest multiple of ten
 * 
 * @param  pCtx The timer
 * @param  fps  The desired FPS
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_TIMER_NOT_INITIALIZED,
 *              GFMRV_FPS_TOO_HIGH
 */
gfmRV gfmTimer_setFPS(gfmTimer *pCtx, int fps) {
    gfmRV rv;
    int interval;
    
    // Sanitize the arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    // Check if it's a valid FPS
    interval = gfmTimer_getMs(fps);
    ASSERT(interval > 0, GFMRV_FPS_TOO_HIGH);
    // Set the new fps
    pCtx->interval = interval;
    pCtx->fps = fps;
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Changes a timer's FPS
 * 
 * @param  pCtx The timer
 * @param  fps  The desired FPS
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_TIMER_NOT_INITIALIZED,
 *              GFMRV_FPS_TOO_HIGH
 */
gfmRV gfmTimer_setFPSRaw(gfmTimer *pCtx, int fps) {
    gfmRV rv;
    int interval;
    
    // Sanitize the arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    // Check if it's a valid FPS
    interval = gfmTimer_getMsRaw(fps);
    ASSERT(interval > 0, GFMRV_FPS_TOO_HIGH);
    // Set the new fps
    pCtx->interval = interval;
    pCtx->fps = fps;
    
    rv = GFMRV_OK;
__ret:
    return rv;
}


/**
 * Issue a new frame; Shouldn't usually be used...
 *
 * @param  pCtx The timer
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_TIMER_NOT_INITIALIZED,
 */
gfmRV gfmTimer_issue(gfmTimer *pCtx) {
    gfmRV rv;
    int interval;
    
    // Sanitize the arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    
    // Add it to the event queue (so the main thread can see it)
    gfmEvent_pushTimeEvent(pCtx->pEvent);
    
    // Delay for a frame
    SDL_Delay(pCtx->interval);
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Get the current time, in milliseconds, since the game started;
 * 
 * This function must be implemented for gfmFPSCounter to work!
 * 
 * @param  pTime The returned time
 * @return       GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmTimer_getCurTimeMs(int *pTime) {
    gfmRV rv;
    
    // Sanitize arguments
    ASSERT(pTime, GFMRV_ARGUMENTS_BAD);
    
    *pTime = (int)SDL_GetTicks();
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

