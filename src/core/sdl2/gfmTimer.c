/**
 * @file src/core/sdl2/gframe_timer.c
 * 
 * Timer module. Should signal whenever a new frame is to be issued.
 * This is a SDL2 implementation.
 */
#include <GFraMe/gfmAssert.h>
#include <GFraMe/core/gfmTimer_bkend.h>

#include <SDL2/SDL_timer.h>

struct stGFMTimer {
    unsigned int desiredDelay;
    unsigned int lastTime;
    int accTime;
};

/**
 * Initialize a timer for the desired fps
 * 
 * @param  [out]ppCtx The alloc'ed timer
 * @param  [ in]fps   The desired FPS
 * @return            GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmTimer_init(gfmTimer **ppCtx, int fps) {
    gfmRV rv;

    /* Sanitize arguments */
    ASSERT(ppCtx, GFMRV_ARGUMENTS_BAD);
    ASSERT(fps > 0, GFMRV_ARGUMENTS_BAD);

    /* Check if the timer had already been alloc'ed */
    if (!(*ppCtx)) {
        *ppCtx = (gfmTimer*)malloc(sizeof(gfmTimer));
        ASSERT(*ppCtx, GFMRV_ALLOC_FAILED);

        memset(*ppCtx, 0x0, sizeof(gfmTimer));
    }

    /* Set the desired delay */
    (*ppCtx)->desiredDelay = 1000 / fps;

    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Dealloc the timer
 * 
 * @param  [out]ppCtx The alloc'ed timer
 * @return            GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmTimer_free(gfmTimer **ppCtx) {
    gfmRV rv;

    /* Sanitize arguments */
    ASSERT(ppCtx, GFMRV_ARGUMENTS_BAD);

    if (*ppCtx) {
        free(*ppCtx);
    }

    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Sleep for a while, waiting for the next frame
 * 
 * If the next frame is to be issued, it won't sleep at all
 * 
 * @param  [ in]pCtx The timer context
 * @return           GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmTimer_wait(gfmTimer *pCtx) {
    gfmRV rv;
    unsigned int curTime;

    /* Sanitize arguments */
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);

    /* Retrieve the current time */
    rv = gfmTimer_getCurTimeMs(&curTime);
    ASSERT(rv == GFMRV_OK, rv);

    /* Update the elapsed time since the last frame */
    pCtx->accTime += curTime - pCtx->lastTime;

    /* Check if sleeping would miss a frame */
    if (pCtx->accTime + 1 < pCtx->desiredDelay) {
        SDL_Delay(1);
    }
    else {
        /* Otherwise, reset the accumulated time */
        pCtx->accTime -= pCtx->desiredDelay;
    }

    /* Update the last time a sleep may have happened */
    pCtx->lastTime = curTime;

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
gfmRV gfmTimer_getCurTimeMs(unsigned int *pTime) {
    gfmRV rv;

    /* Sanitize arguments */
    ASSERT(pTime, GFMRV_ARGUMENTS_BAD);

    *pTime = SDL_GetTicks();

    rv = GFMRV_OK;
__ret:
    return rv;
}

