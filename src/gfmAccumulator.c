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
    /** At how many frames per seconds this is running */
    int fps;
    /** Accumulate the elapsed time, in ms, since last frame */
    int accElapsed;
    /** How many frames were issued since the decimal part was zeroed */
    int accDelays;
    /** How long most frames takes, in ms */
    int delay;
    /** How long the frame that rounds the decimal takes */
    int delayOff;
    /** Number of 'delay' frames for each 'delayOff' */
    int delayProportion;
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

    /* Sanitize argments */
    ASSERT(ppCtx, GFMRV_ARGUMENTS_BAD);
    ASSERT(!(*ppCtx), GFMRV_ARGUMENTS_BAD);

    /* Alloc and clean the accumulator */
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

    /* Sanitize argments */
    ASSERT(ppCtx, GFMRV_ARGUMENTS_BAD);
    ASSERT(*ppCtx, GFMRV_ARGUMENTS_BAD);

    /* Free the 'object' */
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
    int delay, prop;

    /* Sanitize argments */
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    ASSERT(fps > 0, GFMRV_ARGUMENTS_BAD);
    ASSERT(maxFrames > 0, GFMRV_ARGUMENTS_BAD);
    /* Check that the FPS is valid */
    ASSERT(fps < 1000, GFMRV_ACC_FPS_TOO_HIGH);

    /* Calculate how long a frame should take (rounded down) and how many of
     * those there are in a second */
    delay = 1000 / fps;
    prop = fps * (delay + 1) - 1000;
    ASSERT(prop <= fps, GFMRV_INTERNAL_ERROR);
    ASSERT(prop > 0, GFMRV_INTERNAL_ERROR);

    /* Adjust the time each frame takes */
    if (prop == fps) {
        /* All frames should take the same time */
        pCtx->delayProportion = 1;

        pCtx->delay = delay;
        pCtx->delayOff = delay;
    }
    else if (prop < fps - prop) {
        /* There are more frame that takes delay + 1 ms */
        pCtx->delayProportion = fps / prop;

        pCtx->delay = delay + 1;
        pCtx->delayOff = delay;
    }
    else {
        /* There are more frames that takes the exact delay */
        pCtx->delayProportion = fps / (fps - prop);

        pCtx->delay = delay;
        pCtx->delayOff = delay + 1;
    }

    /* Store the current fps */
    pCtx->fps = fps;
    /* Store the maximum number of accumulated frames */
    pCtx->maxFrames = maxFrames;

    /* Initialize the elapsed time */
    pCtx->accElapsed = -pCtx->delay;

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

    /* Sanitize argments */
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    ASSERT(delay > 0, GFMRV_ARGUMENTS_BAD);
    ASSERT(maxFrames > 0, GFMRV_ARGUMENTS_BAD);

    /* Set the delay (and clean the previous state) */
    pCtx->delayProportion = 1;
    pCtx->delay = delay;
    pCtx->delayOff = delay;
    pCtx->accFrames = 0;
    pCtx->maxFrames = maxFrames;
    pCtx->fps = 1000 / delay;

    /* Initialize the elapsed time */
    pCtx->accElapsed = -pCtx->delay;

    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Get the fps set
 * 
 * @param  pFps The current fps
 * @param  pCtx The accumulator
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_ACC_NOT_INITIALIZED
 */
gfmRV gfmAccumulator_getFPS(int *pFps, gfmAccumulator *pCtx) {
    gfmRV rv;

    /* Sanitize argments */
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    ASSERT(pFps, GFMRV_ARGUMENTS_BAD);
    /* Check that it was initialized */
    ASSERT(pCtx->delay > 0, GFMRV_ACC_NOT_INITIALIZED);

    /* Simply return the FPS */
    *pFps = pCtx->fps;

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

    /* Sanitize argments */
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    ASSERT(*pFrames, GFMRV_ARGUMENTS_BAD);
    /* Check that it was initialized */
    ASSERT(pCtx->delay > 0, GFMRV_ACC_NOT_INITIALIZED);

    /* Simply return the accumulated frames */
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

    /* Sanitize argments */
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    ASSERT(pFrames, GFMRV_ARGUMENTS_BAD);
    /* Check that it was initialized */
    ASSERT(pCtx->delay > 0, GFMRV_ACC_NOT_INITIALIZED);

    /* Return the accumulated frames or the maximum number of frames */
    if (pCtx->accFrames < pCtx->maxFrames)
        *pFrames = pCtx->accFrames;
    else
        *pFrames = pCtx->maxFrames;
    /* Clean up the frames */
    pCtx->accFrames = 0;

    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Get the delay between frames
 * 
 * @param  pDelay The delay
 * @param  pCtx   The accumulator
 * @return         GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_ACC_NOT_INITIALIZED
 */
gfmRV gfmAccumulator_getDelay(int *pDelay, gfmAccumulator *pCtx) {
    gfmRV rv;

    /* Sanitize argments */
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    ASSERT(pDelay, GFMRV_ARGUMENTS_BAD);
    /* Check that it was initialized */
    ASSERT(pCtx->delay > 0, GFMRV_ACC_NOT_INITIALIZED);

    /* Simply return the accumulated frames */
    if (pCtx->accDelays == 0) {
        *pDelay = pCtx->delayOff;
    }
    else {
        *pDelay = pCtx->delay;
    }

    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Clear both the accumulated value and the remainder of the time
 * 
 * @param  pCtx The accumulator
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmAccumulator_reset(gfmAccumulator *pCtx) {
    gfmRV rv;

    /* Sanitize argments */
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);

    /* Clean up the frames */
    pCtx->accFrames = 0;
    pCtx->accElapsed = -pCtx->delay;
    pCtx->accDelays = 0;

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

    /* Sanitize argments */
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    ASSERT(ms > 0, GFMRV_ARGUMENTS_BAD);
    /* Check that it was initialized */
    ASSERT(pCtx->delay > 0, GFMRV_ACC_NOT_INITIALIZED);

    /* Update the elapsed time */
    pCtx->accElapsed += ms;

    /* Check if a new frame should be issued */
    if (pCtx->accElapsed > 0) {
        /* Update how many frames were issued since the last time it rounded */
        pCtx->accDelays = (pCtx->accDelays + 1) % pCtx->delayProportion;

        /* Update how long the next frame should take */
        if (pCtx->accDelays == 0) {
            pCtx->accElapsed -= pCtx->delayOff;
        }
        else {
            pCtx->accElapsed -= pCtx->delay;
        }

        /* Update the number of issued frames */
        pCtx->accFrames++;
    }

    rv = GFMRV_OK;
__ret:
    return rv;
}

