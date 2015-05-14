/**
 * @file include/GFraMe/gfmAccumulator.h
 * 
 * Module for accumulating frames; After some time has passed, a new frame shall
 * be issued and stored; Mostly used for timers
 */
#ifndef __GFMACCUMULATOR_STRUCT__
#define __GFMACCUMULATOR_STRUCT__

/** 'Exports' the gfmAccumulator structure */
typedef struct stGFMAccumulator gfmAccumulator;

#endif /* __GFMACCUMULATOR_STRUCT__ */

#ifndef __GFMACCUMULATOR_H__
#define __GFMACCUMULATOR_H__

#include <GFraMe/gfmError.h>

/** 'Exportable' size of gfmAccumulator */
extern const int sizeofGFMAccumulator;

/**
 * Alloc a new accumulator
 * 
 * @param  ppCtx The accumulator
 * @return       GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_ALLOC_FAILED
 */
gfmRV gfmAccumulator_getNew(gfmAccumulator **ppCtx);

/**
 * Free a previously allocated accumulator
 * 
 * @param  ppCtx The accumulator
 * @return       GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmAccumulator_free(gfmAccumulator **ppCtx);

/**
 * Set how long a frame takes, based on a desired FPS rate
 * 
 * @param  pCtx      The accumulator
 * @param  fps       The desired frames per second
 * @param  maxFrames How many frames can be accumulated, at most
 * @return           GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_ACC_FPS_TOO_HIGH
 */
gfmRV gfmAccumulator_setFPS(gfmAccumulator *pCtx, int fps, int maxFrames);

/**
 * Manually set how long each frame takes
 * 
 * @param  pCtx      The accumulator
 * @param  delay     How long the frame should take
 * @param  maxFrames How many frames can be accumulated, at most
 * @return           GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmAccumulator_setTime(gfmAccumulator *pCtx, int delay, int maxFrames);

/**
 * Check how many frames have been accumulated, but don't clean it up
 * 
 * @param  pFrames How many frames have passed, if any
 * @param  pCtx    The accumulator
 * @return         GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_ACC_NOT_INITIALIZED
 */
gfmRV gfmAccumulator_checkFrames(int *pFrames, gfmAccumulator *pCtx);

/**
 * Get how many frames have passed, capping on the max number of frames
 * 
 * @param  pFrames How many frames have passed, if any
 * @param  pCtx    The accumulator
 * @return         GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_ACC_NOT_INITIALIZED
 */
gfmRV gfmAccumulator_getFrames(int *pFrames, gfmAccumulator *pCtx);

/**
 * Clear both the accumulated value and the remainder of the time; Note that the
 * current delay isn't modified
 * 
 * @param  pCtx The accumulator
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmAccumulator_reset(gfmAccumulator *pCtx);

/**
 * Update the accumulated time
 * @param  pCtx The accumulator
 * @param  ms   Time elapsed, in milliseconds
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_ACC_NOT_INITIALIZED
 */
gfmRV gfmAccumulator_update(gfmAccumulator *pCtx, int ms);

#endif /* __GFMACCUMULATOR_H__ */

