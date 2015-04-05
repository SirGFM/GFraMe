/**
 * @file include/GFraMe/core/gfmTimer_bkend.h
 * 
 * Timer module. Should signal whenever a new frame is to be issued
 */
#ifndef __GFRAME_TIMER_H_
#define __GFRAME_TIMER_H_

#include <GFraMe/gfmError.h>

/** "Export" the timer structure's type */
typedef struct stGFMTimer gfmTimer;

/**
 * Get how long each frame must take for the timer function;
 * The result is rounded down to the nearest multiple of ten
 * 
 * @param  fps  How many frames should run per second
 * @return      How long each frame must take, in milliseconds
 */
int gfmTimer_getMs(int fps);

/**
 * Get how long each frame must take for the timer function
 * 
 * @param  fps  How many frames should run per second
 * @return      How long each frame must take, in milliseconds
 */
int gfmTimer_getMsRaw(int fps);

/**
 * Initialize (i.e., alloc) a new timer
 * 
 * @param  ppSelf The timer
 * @return        GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_ALLOC_FAILED
 */
gfmRV gfmTimer_new(gfmTimer **ppSelf);

/**
 * Releases a timer's resources; This function MUST stop the timer if it's
 * still running
 * 
 * @param  ppSelf The timer
 * @return        GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmTimer_free(gfmTimer **ppSelf);

/**
 * Initiate a timer
 * The fps's interval is rounded down to the nearest multiple of ten
 * 
 * @param  pSelf The timer
 * @param  fps   The desired FPS
 * @return       GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_FPS_TOO_HIGH
 */
gfmRV gfmTimer_init(gfmTimer *pSelf, int fps);

/**
 * Initiate a timer
 * 
 * @param  pSelf The timer
 * @param  fps   The desired FPS
 * @return       GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_FPS_TOO_HIGH
 */
gfmRV gfmTimer_initRaw(gfmTimer *pSelf, int fps);

/**
 * Stops a timer
 * 
 * @param  pSelf The timer
 * @return       GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_TIMER_NOT_INITIALIZED,
 *               GFMRV_FAILED_TO_STOP_TIMER
 */
gfmRV gfmTimer_stop(gfmTimer *pSelf);

/**
 * Changes a timer's FPS
 * The result is rounded down to the nearest multiple of ten
 * 
 * @param  pSelf The timer
 * @param  fps   The desired FPS
 * @return       GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_TIMER_NOT_INITIALIZED,
 *               GFMRV_FPS_TOO_HIGH
 */
gfmRV gfmTimer_setFPS(gfmTimer *pSelf, int fps);

/**
 * Changes a timer's FPS
 * 
 * @param  pSelf The timer
 * @param  fps   The desired FPS
 * @return       GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_TIMER_NOT_INITIALIZED,
 *               GFMRV_FPS_TOO_HIGH
 */
gfmRV gfmTimer_setFPSRaw(gfmTimer *pSelf, int fps);

#endif

