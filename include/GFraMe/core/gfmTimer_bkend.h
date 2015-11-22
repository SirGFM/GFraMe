/**
 * @file include/GFraMe/core/gfmTimer_bkend.h
 * 
 * Timer module. Should signal whenever a new frame is to be issued
 */
#ifndef __GFRAMETIMER_BKEND_H_
#define __GFRAMETIMER_BKEND_H_

#include <GFraMe/gfmError.h>

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

