/**
 * @file include/GFraMe/core/gfmTimer_bkend.h
 * 
 * Timer module. Should signal whenever a new frame is to be issued
 */
#ifndef __GFMTIMER_STRUCT__
#define __GFMTIMER_STRUCT__

typedef struct stGFMTimer gfmTimer;

#endif /* __GFMTIMER_STRUCT__ */

#ifndef __GFRAMETIMER_BKEND_H_
#define __GFRAMETIMER_BKEND_H_

#include <GFraMe/gfmError.h>

/**
 * Initialize a timer for the desired fps
 * 
 * @param  [out]ppCtx The alloc'ed timer
 * @param  [ in]fps   The desired FPS
 * @return            GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmTimer_init(gfmTimer **ppCtx, int fps);

/**
 * Dealloc the timer
 * 
 * @param  [out]ppCtx The alloc'ed timer
 * @return            GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmTimer_free(gfmTimer **ppCtx);

/**
 * Sleep for a while, waiting for the next frame
 * 
 * If the next frame is to be issued, it won't sleep at all
 * 
 * @param  [ in]pCtx The timer context
 * @return           GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmTimer_wait(gfmTimer *pCtx);

/**
 * Get the current time, in milliseconds, since the game started;
 * 
 * This function must be implemented for gfmFPSCounter to work!
 * 
 * @param  pTime The returned time
 * @return       GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmTimer_getCurTimeMs(unsigned int *pTime);

#endif /* __GFRAMETIMER_BKEND_H_ */

