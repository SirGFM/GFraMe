/**
 * @file include/GFraMe/gfmAnimation.h
 * 
 * Sprite's animation
 */
#ifndef __GFMANIMATION_STRUCT__
#define __GFMANIMATION_STRUCT__

/** 'Exports' the gfmAnimation structure */
typedef struct stGFMAnimation gfmAnimation;

#endif /* __GFMANIMATION_STRUCT__ */

#ifndef __GFMANIMATION_H__
#define __GFMANIMATION_H__

/** 'Exportable' size of gfmAnimation */
extern const int sizeofGFMAnimation;

#include <GFraMe/gframe.h>
#include <GFraMe/gfmError.h>

/**
 * Alloc a new gfmAnimation
 * 
 * @param  ppCtx The gfmAnimation
 * @return       GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_ALLOC_FAILED
 */
gfmRV gfmAnimation_getNew(gfmAnimation **ppCtx);

/**
 * Free (and clean) a gfmAnimation
 * 
 * @param  ppCtx The gfmAnimation
 * @return       GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmAnimation_free(gfmAnimation **ppCtx);

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
        int doLoop);

/**
 * Free any memory alloc'ed on init (will probably do nothing, though...)
 * 
 * @param  pCtx The gfmAnimation
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmAnimation_clean(gfmAnimation *pCtx);

/**
 * Update the animation, changing frames, looping, etc.
 * 
 * @param  pCtx  The game's context
 * @param  pAnim The gfmAnimation
 * @return       GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_ANIMATION_NOT_INITIALIZED,
 *               GFMRV_ANIMATION_ALREADY_FINISHED
 */
gfmRV gfmAnimation_update(gfmAnimation *pAnim, gfmCtx *pCtx);

/**
 * Returns an animation to its initial status
 * 
 * @param  pCtx The gfmAnimation
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_ANIMATION_NOT_INITIALIZED
 */
gfmRV gfmAnimation_reset(gfmAnimation *pCtx);

/**
 * Returns the current frame
 * 
 * @param  pFrame The frame
 * @param  pCtx   The gfmAnimation
 * @return        GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_INVALID_INDEX,
 *                GFMRV_ANIMATION_NOT_INITIALIZED
 */
gfmRV gfmAnimation_getFrame(int *pFrame, gfmAnimation *pCtx);

/**
 * Returns whether the animation have already looped
 * 
 * @param  pCtx The gfmAnimation
 * @return      GFMRV_TRUE, GFMRV_FALSE, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmAnimation_didLoop(gfmAnimation *pCtx);

/**
 * Returns whether the animation just looped
 * 
 * @param  pCtx The gfmAnimation
 * @return      GFMRV_TRUE, GFMRV_FALSE, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmAnimation_didJustLoop(gfmAnimation *pCtx);

/**
 * Returns whether the animation just change the frame
 * 
 * @param  pCtx The gfmAnimation
 * @return      GFMRV_TRUE, GFMRV_FALSE, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmAnimation_didJustChangeFrame(gfmAnimation *pCtx);

/**
 * Returns whether the animation finished running
 * 
 * @param  pCtx The gfmAnimation
 * @return      GFMRV_TRUE, GFMRV_FALSE, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmAnimation_didFinish(gfmAnimation *pCtx);

#endif /* __GFMANIMATION_H__ */

