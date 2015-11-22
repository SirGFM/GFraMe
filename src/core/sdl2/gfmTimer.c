/**
 * @file src/core/sdl2/gframe_timer.c
 * 
 * Timer module. Should signal whenever a new frame is to be issued.
 * This is a SDL2 implementation.
 */
#include <GFraMe/gfmAssert.h>
#include <GFraMe/core/gfmTimer_bkend.h>

#include <SDL2/SDL_timer.h>

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

    /* Sanitize arguments */
    ASSERT(pTime, GFMRV_ARGUMENTS_BAD);

    *pTime = (int)SDL_GetTicks();

    rv = GFMRV_OK;
__ret:
    return rv;
}

