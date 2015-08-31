/**
 * @file include/GFraMe/core/gfmTimer_bkend.h
 * 
 * Timer module. Should signal whenever a new frame is to be issued
 */
#ifndef __GFRAMETIMER_STRUCT__
#define __GFRAMETIMER_STRUCT__

/** "Export" the timer structure's type */
typedef struct stGFMTimer gfmTimer;

#endif /* __GFRAMETIMER_STRUCT__ */

#ifndef __GFRAMETIMER_BKEND_H_
#define __GFRAMETIMER_BKEND_H_

#include <GFraMe/gfmError.h>

/** 'Exportable' size of gfmTimer */
extern const int sizeofGFMTimer;

/**
 * Get how long each frame must take for the timer function;
 * The result is rounded down to the nearest multiple of ten
 * 
 * @param  fps How many frames should run per second
 * @return     How long each frame must take, in milliseconds
 */
int gfmTimer_getMs(int fps);

/**
 * Get how long each frame must take for the timer function
 * 
 * @param  fps How many frames should run per second
 * @return     How long each frame must take, in milliseconds
 */
int gfmTimer_getMsRaw(int fps);

/**
 * Initialize (i.e., alloc) a new timer
 * 
 * @param  ppTimer The timer
 * @param  pCtx    The game's context
 * @return         GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_ALLOC_FAILED
 */
gfmRV gfmTimer_getNew(gfmTimer **ppTimer, gfmCtx *pCtx);

/**
 * Releases a timer's resources; This function MUST stop the timer if it's
 * still running
 * 
 * @param  ppCtx The timer
 * @return       GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmTimer_free(gfmTimer **ppCtx);

/**
 * Initiate a timer
 * The fps's interval is rounded down to the nearest multiple of ten
 * 
 * @param  pCtx The timer
 * @param  fps  The desired FPS
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_FPS_TOO_HIGH,
 *              GFMRV_INTERNAL_ERROR, GFMRV_TIMER_ALREADY_INITIALIZED
 */
gfmRV gfmTimer_init(gfmTimer *pCtx, int fps);

/**
 * Initiate a timer
 * 
 * @param  pCtx The timer
 * @param  fps  The desired FPS
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_FPS_TOO_HIGH,
 *              GFMRV_INTERNAL_ERROR, GFMRV_TIMER_ALREADY_INITIALIZED
 */
gfmRV gfmTimer_initRaw(gfmTimer *pCtx, int fps);

/**
 * Stops a timer
 * 
 * @param  pCtx The timer
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_TIMER_NOT_INITIALIZED,
 *              GFMRV_FAILED_TO_STOP_TIMER
 */
gfmRV gfmTimer_stop(gfmTimer *pCtx);

/**
 * Changes a timer's FPS
 * The result is rounded down to the nearest multiple of ten
 * 
 * @param  pCtx The timer
 * @param  fps  The desired FPS
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_TIMER_NOT_INITIALIZED,
 *              GFMRV_FPS_TOO_HIGH
 */
gfmRV gfmTimer_setFPS(gfmTimer *pCtx, int fps);

/**
 * Changes a timer's FPS
 * 
 * @param  pCtx The timer
 * @param  fps  The desired FPS
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_TIMER_NOT_INITIALIZED,
 *              GFMRV_FPS_TOO_HIGH
 */
gfmRV gfmTimer_setFPSRaw(gfmTimer *pCtx, int fps);

/**
 * Issue a new frame; Should only be used on singled threaded environments
 *
 * @param  pCtx The timer
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_TIMER_NOT_INITIALIZED
 */
gfmRV gfmTimer_issue(gfmTimer *pCtx);

/**
 * Wait for a new frame; Should only be used on singled threaded environments
 *
 * @param  pCtx The timer
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_TIMER_NOT_INITIALIZED
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
gfmRV gfmTimer_getCurTimeMs(int *pTime);

#endif /* __GFRAMETIMER_BKEND_H_ */

