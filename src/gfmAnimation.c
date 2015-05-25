/**
 * @file src/gfmAnimation.c
 * 
 * Sprite's animation
 */
#include <GFraMe/gframe.h>
#include <GFraMe/gfmAnimation.h>
#include <GFraMe/gfmAssert.h>
#include <GFraMe/gfmError.h>

#include <stdlib.h>
#include <string.h>

/** The gfmAnimation structure */
struct stGFMAnimation {
    /** Array of frames on the animation */
    int *pData;
    /** How many frames there are in the animation */
    int numFrames;
    /** Whether the animation should loop or not */
    int doLoop;
    /** Current fps (for ease of use) */
    int fps;
    /** How long until a frame ends */
    int delay;
    /** How much time have accumulated from the previous frame */
    int accTime;
    /** Current index in the animation */
    int index;
    /** How many times the animation have looped */
    int loopCount;
    /** Whether the frame just changed */
    gfmRV justChangedFrame;
    /** Whether the animation just looped */
    gfmRV justLooped;
};

/** 'Exportable' size of gfmAnimation */
const int sizeofGFMAnimation = (int)sizeof(gfmAnimation);

/**
 * Alloc a new gfmAnimation
 * 
 * @param  ppCtx The gfmAnimation
 * @return       GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_ALLOC_FAILED
 */
gfmRV gfmAnimation_getNew(gfmAnimation **ppCtx) {
    gfmRV rv;
    
    // Sanitize arguments
    ASSERT(ppCtx, GFMRV_ARGUMENTS_BAD);
    ASSERT(!(*ppCtx), GFMRV_ARGUMENTS_BAD);
    
    // Alloc the object
    *ppCtx = (gfmAnimation*)malloc(sizeof(gfmAnimation));
    ASSERT(*ppCtx, GFMRV_ALLOC_FAILED);
    
    // Clean up the object
    memset(*ppCtx, 0x0, sizeof(gfmAnimation));
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Free (and clean) a gfmAnimation
 * 
 * @param  ppCtx The gfmAnimation
 * @return       GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmAnimation_free(gfmAnimation **ppCtx) {
    gfmRV rv;
    
    // Sanitize arguments
    ASSERT(ppCtx, GFMRV_ARGUMENTS_BAD);
    ASSERT(*ppCtx, GFMRV_ARGUMENTS_BAD);
    
    // Clean the animation
    rv = gfmAnimation_clean(*ppCtx);
    ASSERT_NR(rv == GFMRV_OK);
    
    // Free the object
    free(*ppCtx);
    *ppCtx = 0;
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Initialize the animation
 * 
 * @param  pCtx      The animation
 * @param  pData     The animation's frames, in order; Must be kept by the
 *                   caller, as no copy is made!
 * @param  numFrames How many frames there are in pData (not how many bytes!!)
 * @param  fps       The animation's frame rate
 * @param  doLoop    Whether the animation should loop (1) or  not (0)
 * @return           GFMRV_OK, GFMRV_ARGUMENTS_BAD,
 *                   GFMRV_ANIMATION_ALREADY_INITIALIZED
 */
gfmRV gfmAnimation_init(gfmAnimation *pCtx, int *pData, int numFrames, int fps,
        int doLoop) {
    gfmRV rv;
    
    // Sanitize arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    ASSERT(pData, GFMRV_ARGUMENTS_BAD);
    ASSERT(numFrames > 0, GFMRV_ARGUMENTS_BAD);
    // FPS can only be 0 if there's a single frame
    ASSERT(fps > 0 || numFrames == 1, GFMRV_ARGUMENTS_BAD);
    // Check that it still wasn't initialized
    ASSERT(!(pCtx->pData), GFMRV_ANIMATION_ALREADY_INITIALIZED);
    
    // Initialize everything
    pCtx->pData = pData;
    pCtx->numFrames = numFrames;
    pCtx->doLoop = doLoop;
    if (fps > 0)
        pCtx->delay = 1000 / fps;
    pCtx->fps = fps;
    
    // Reset the animation
    rv = gfmAnimation_reset(pCtx);
    ASSERT_NR(rv == GFMRV_OK);
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Free any memory alloc'ed on init (will probably do nothing, though...)
 * 
 * @param  pCtx The gfmAnimation
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmAnimation_clean(gfmAnimation *pCtx) {
    gfmRV rv;
    
    // Sanitize arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    
    // Clean up the object
    memset(pCtx, 0x0, sizeof(gfmAnimation));
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Update the animation, changing frames, looping, etc.
 * 
 * @param  pCtx  The game's context
 * @param  pAnim The gfmAnimation
 * @return       GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_ANIMATION_NOT_INITIALIZED,
 *               GFMRV_ANIMATION_ALREADY_FINISHED
 */
gfmRV gfmAnimation_update(gfmAnimation *pAnim, gfmCtx *pCtx) {
    gfmRV rv;
    int ms;
    
    // Sanitize arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    ASSERT(pAnim, GFMRV_ARGUMENTS_BAD);
    // Check that it was initialized
    ASSERT(pAnim->pData, GFMRV_ANIMATION_NOT_INITIALIZED);
    // Check that the animation hasn't finished
    ASSERT(pAnim->doLoop || pAnim->loopCount == 0,
            GFMRV_ANIMATION_ALREADY_FINISHED);
    
    // Get the elapsed time from the previous frame
    rv = gfm_getElapsedTime(&ms, pCtx);
    ASSERT_NR(rv == GFMRV_OK);
    
    // Reset these every frame
    pAnim->justLooped = GFMRV_FALSE;
    pAnim->justChangedFrame = GFMRV_FALSE;
    // Update the timer
    pAnim->accTime += ms;
    // Update as many frames as necessary
    while (pAnim->accTime > pAnim->delay) {
        // Update the current index
        pAnim->index++;
        pAnim->justChangedFrame = GFMRV_TRUE;
        
        // Check if the animation finished or looped
        if (pAnim->index >= pAnim->numFrames) {
            // Increase how many times it finished
            pAnim->loopCount++;
            pAnim->justLooped = GFMRV_TRUE;
            // Go back to the first frame, if it's looped
            if (pAnim->doLoop) {
                pAnim->index = 0;
            }
            else {
                // Keep the last frame, if it's a non-looping animation
                pAnim->index--;
            }
        }
        
        // Break if it's a single-frame animation
        if (pAnim->delay == 0)
            break;
        // Remove this frame's accumulated time
        pAnim->accTime -= pAnim->delay;
    }
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Returns an animation to its initial status
 * 
 * @param  pCtx The gfmAnimation
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_ANIMATION_NOT_INITIALIZED
 */
gfmRV gfmAnimation_reset(gfmAnimation *pCtx) {
    gfmRV rv;
    
    // Sanitize arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    // Check that it was initialized
    ASSERT(pCtx->pData, GFMRV_ANIMATION_NOT_INITIALIZED);
    
    // Reset everything
    pCtx->accTime = 0;
    pCtx->index = 0;
    pCtx->loopCount = 0;
    pCtx->justChangedFrame = 0;
    pCtx->justLooped = 0;
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Returns the current frame
 * 
 * @param  pFrame The frame
 * @param  pCtx   The gfmAnimation
 * @return        GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_INVALID_INDEX,
 *                GFMRV_ANIMATION_NOT_INITIALIZED
 */
gfmRV gfmAnimation_getFrame(int *pFrame, gfmAnimation *pCtx) {
    gfmRV rv;
    
    // Sanitize arguments
    ASSERT(pFrame, GFMRV_ARGUMENTS_BAD);
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    // Check that it was initialized
    ASSERT(pCtx->pData, GFMRV_ANIMATION_NOT_INITIALIZED);
    // Check that it's a valid index
    ASSERT(pCtx->index < pCtx->numFrames, GFMRV_INVALID_INDEX);
    
    // Get the frame
    *pFrame = pCtx->pData[pCtx->index];
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Returns whether the animation have already looped
 * 
 * @param  pCtx The gfmAnimation
 * @return      GFMRV_TRUE, GFMRV_FALSE, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmAnimation_didLoop(gfmAnimation *pCtx) {
    gfmRV rv;
    
    // Sanitize argument
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    
    if (pCtx->loopCount > 0)
        rv = GFMRV_TRUE;
    else
        rv = GFMRV_FALSE;
__ret:
    return rv;
}

/**
 * Returns whether the animation just looped
 * 
 * @param  pCtx The gfmAnimation
 * @return      GFMRV_TRUE, GFMRV_FALSE, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmAnimation_didJustLoop(gfmAnimation *pCtx) {
    gfmRV rv;
    
    // Sanitize argument
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    
    rv = pCtx->justLooped;
__ret:
    return rv;
}

/**
 * Returns whether the animation just change the frame
 * 
 * @param  pCtx The gfmAnimation
 * @return      GFMRV_TRUE, GFMRV_FALSE, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmAnimation_didJustChangeFrame(gfmAnimation *pCtx) {
    gfmRV rv;
    
    // Sanitize argument
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    
    rv = pCtx->justChangedFrame;
__ret:
    return rv;
}

/**
 * Returns whether the animation finished running
 * 
 * @param  pCtx The gfmAnimation
 * @return      GFMRV_TRUE, GFMRV_FALSE, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmAnimation_didFinish(gfmAnimation *pCtx) {
    gfmRV rv;
    
    // Sanitize argument
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    // Only non-looping animations loop!
    ASSERT(pCtx->doLoop == 0, GFMRV_ARGUMENTS_BAD);
    
    if (pCtx->loopCount > 0)
        rv = GFMRV_TRUE;
    else
        rv = GFMRV_FALSE;
__ret:
    return rv;
}

