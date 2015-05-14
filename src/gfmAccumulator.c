/**
 * @file include/GFraMe/gfmAccumulator.h
 * 
 * Module for accumulating time; Mostly used for timers
 */
#include <GFraMe/gfmAccumulator.h>
#include <GFraMe/gfmAssert.h>
#include <GFraMe/gfmError.h>

#include <stdlib.h>
#include <string.h>

/** The gfmAccumulator structure */
struct stGFMAccumulator {
    /** Elapsed time on the current frame */
    int elapsed;
    /** How long until a 'frame' is issued */
    int delay;
    /** How many frames have been accumulated */
    int accFrames;
    /** At most, how many frames can be accumulated */
    int maxFrames;
};

/** Size of gfmAccumulator */
const int sizeofGFMAccumulator = (int)sizeof(gfmAccumulator);

/**
 * Alloc a new accumulator
 * 
 * @param  ppCtx The accumulator
 * @return       GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_ALLOC_FAILED
 */
gfmRV gfmAccumulator_getNew(gfmAccumulator **ppCtx) {
    gfmRV rv;
    
    // Sanitize argments
    ASSERT(ppCtx, GFMRV_ARGUMENTS_BAD);
    ASSERT(!(*ppCtx), GFMRV_ARGUMENTS_BAD);
    
    // Alloc and clean the accumulator
    *ppCtx = (gfmAccumulator*)malloc(sizeof(gfmAccumulator));
    ASSERT(*ppCtx, GFMRV_ALLOC_FAILED);
    
    memset(*ppCtx, 0x0, sizeof(gfmAccumulator));
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Free a previously allocated accumulator
 * 
 * @param  ppCtx The accumulator
 * @return       GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmAccumulator_free(gfmAccumulator **ppCtx) {
    gfmRV rv;
    
    // Sanitize argments
    ASSERT(ppCtx, GFMRV_ARGUMENTS_BAD);
    ASSERT(*ppCtx, GFMRV_ARGUMENTS_BAD);
    
    // Free the 'object'
    free(*ppCtx);
    *ppCtx = 0;
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Set how long a frame takes, based on a desired FPS rate
 * 
 * @param  pCtx      The accumulator
 * @param  fps       The desired frames per second
 * @param  maxFrames How many frames can be accumulated, at most
 * @return           GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_ACC_FPS_TOO_HIGH
 */
gfmRV gfmAccumulator_setFPS(gfmAccumulator *pCtx, int fps, int maxFrames) {
    gfmRV rv;
    int delay;
    
    // Sanitize argments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    ASSERT(fps > 0, GFMRV_ARGUMENTS_BAD);
    ASSERT(maxFrames > 0, GFMRV_ARGUMENTS_BAD);
    // Check that the FPS is valid
    ASSERT(fps < 1000, GFMRV_ACC_FPS_TOO_HIGH);
    
    // Get the expected delay, in milliseconds
    delay = 1000 / fps;
    // Set the delay
    rv = gfmAccumulator_setTime(pCtx, delay, maxFrames);
    ASSERT_NR(rv == GFMRV_OK);
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Manually set how long each frame takes
 * 
 * @param  pCtx      The accumulator
 * @param  delay     How long the frame should take
 * @param  maxFrames How many frames can be accumulated, at most
 * @return           GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmAccumulator_setTime(gfmAccumulator *pCtx, int delay, int maxFrames) {
    gfmRV rv;
    
    // Sanitize argments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    ASSERT(delay > 0, GFMRV_ARGUMENTS_BAD);
    ASSERT(maxFrames > 0, GFMRV_ARGUMENTS_BAD);
    
    // Set the delay (and clean the previous state)
    pCtx->elapsed = 0;
    pCtx->delay = delay;
    pCtx->accFrames = 0;
    pCtx->maxFrames = maxFrames;
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Check how many frames have been accumulated, but don't clean it up
 * 
 * @param  pFrames How many frames have passed, if any
 * @param  pCtx    The accumulator
 * @return         GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_ACC_NOT_INITIALIZED
 */
gfmRV gfmAccumulator_checkFrames(int *pFrames, gfmAccumulator *pCtx) {
    gfmRV rv;
    
    // Sanitize argments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    ASSERT(*pFrames, GFMRV_ARGUMENTS_BAD);
    // Check that it was initialized
    ASSERT(pCtx->delay > 0, GFMRV_ACC_NOT_INITIALIZED);
    
    // Simply return the accumulated frames
    *pFrames = pCtx->accFrames;
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Get how many frames have passed, capping on the max number of frames
 * 
 * @param  pFrames How many frames have passed, if any
 * @param  pCtx    The accumulator
 * @return         GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_ACC_NOT_INITIALIZED
 */
gfmRV gfmAccumulator_getFrames(int *pFrames, gfmAccumulator *pCtx) {
    gfmRV rv;
    
    // Sanitize argments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    ASSERT(*pFrames, GFMRV_ARGUMENTS_BAD);
    // Check that it was initialized
    ASSERT(pCtx->delay > 0, GFMRV_ACC_NOT_INITIALIZED);
    
    // Return the accumulated frames or the maximum number of frames
    if (pCtx->accFrames < pCtx->maxFrames)
        *pFrames = pCtx->accFrames;
    else
        *pFrames = pCtx->maxFrames;
    // Clean up the frames
    pCtx->accFrames = 0;
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Clear both the accumulated value and the remainder of the time; Note that the
 * current delay isn't modified
 * 
 * @param  pCtx The accumulator
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmAccumulator_reset(gfmAccumulator *pCtx) {
    gfmRV rv;
    
    // Sanitize argments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    
    // Clean up the frames
    pCtx->elapsed = 0;
    pCtx->accFrames = 0;
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Update the accumulated time
 * @param  pCtx The accumulator
 * @param  ms   Time elapsed, in milliseconds
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_ACC_NOT_INITIALIZED
 */
gfmRV gfmAccumulator_update(gfmAccumulator *pCtx, int ms) {
    gfmRV rv;
    
    // Sanitize argments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    ASSERT(ms > 0, GFMRV_ARGUMENTS_BAD);
    // Check that it was initialized
    ASSERT(pCtx->delay > 0, GFMRV_ACC_NOT_INITIALIZED);
    
    // Update the elapsed time
    pCtx->elapsed += ms;
    // Check if new frames should be issued
    pCtx->accFrames += pCtx->elapsed / pCtx->delay;
    pCtx->elapsed = pCtx->elapsed % pCtx->delay;
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

