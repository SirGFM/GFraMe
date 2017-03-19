/**
 * @file src/util/gfmFPSCounter.c
 * 
 * Module that calculates and displays the current FPS
 * This module requires gfmTimer_getCurTimeMs to be implemented!
 */
#include <GFraMe/gframe.h>
#include <GFraMe/gfmAssert.h>
#include <GFraMe/gfmDebug.h>
#include <GFraMe/gfmError.h>
#include <GFraMe/gfmSpriteset.h>
#include <GFraMe/core/gfmTimer_bkend.h>
#include <GFraMe_int/gfmFPSCounter.h>

#include <stdlib.h>
#include <string.h>

/** The gfmFPSCounter structure */
struct stGFMFPSCounter {
    /** FPS counter's horizontal position, on the screen */
    int x;
    /** FPS counter's vertical position, on the screen */
    int y;
    /** Time, in milliseconds, that the drawing process was initialized */
    unsigned int drawInit;
    /** Count how many draws were made last second */
    int drawCount;
    /** Accumulate draws */
    int drawAcc;
    /** Time, in milliseconds, that the update process was initialized */
    unsigned int updateInit;
    /** How long the update took */
    unsigned int updateTime;
    /** Count how many updates were made last second */
    int updateCount;
    /** Accumulate updates */
    int updateAcc;
    /** Last time the fps counter was updated */
    unsigned int lastTime;
};

/** Size of gfmFPSCounter */
const int sizeofGFMFPSCounter = (int)sizeof(gfmFPSCounter);

/**
 * Alloc a new FPS counter structure
 * 
 * @param  ppCtx The FPS counter
 * @return       GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_ALLOC_FAILED
 */
gfmRV gfmFPSCounter_getNew(gfmFPSCounter **ppCtx) {
    gfmRV rv;
    unsigned int time;
    
    // Sanitize arguments
    ASSERT(ppCtx, GFMRV_ARGUMENTS_BAD);
    ASSERT(!(*ppCtx), GFMRV_ARGUMENTS_BAD);
    // Check that the counter is supported
    ASSERT(gfmTimer_getCurTimeMs(&time) == GFMRV_OK,
            GFMRV_FUNCTION_NOT_SUPPORTED);
    
    // Alloc the struct
    *ppCtx = (gfmFPSCounter*)malloc(sizeof(gfmFPSCounter));
    ASSERT(*ppCtx, GFMRV_ALLOC_FAILED);
    
    // Initialize everything
    memset(*ppCtx, 0x0, sizeof(gfmFPSCounter));
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Release a previously alloc'ed FPS counter
 * 
 * @param  ppCtx The FPS counter
 * @return       GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmFPSCounter_free(gfmFPSCounter **ppCtx) {
    gfmRV rv;
    
    // Sanitize arguments
    ASSERT(ppCtx, GFMRV_ARGUMENTS_BAD);
    ASSERT(*ppCtx, GFMRV_ARGUMENTS_BAD);
    
    // Free the struct
    free(*ppCtx);
    *ppCtx = 0;
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Initialize the FPS counter and set its spriteset; If this function isn't
 * called the default texture will be used, and it'll expect the bitmap font to
 * be at the first tile;
 * 
 * NOTE: The bitmap font should follow the ASCII table, with the first character
 * being '!'
 * 
 * @param  pCtx      The FPS counter
 * @param  pSset     The spriteset
 * @param  firstTile The first ASCII character's tile ('!') on the spriteset
 * @return           GFMRV_OK, GFMRV_ARGUMENTS_BAD, ...
 */
gfmRV gfmFPSCounter_init(gfmFPSCounter *pCtx, gfmSpriteset *pSset,
        int firstTile) {
    gfmRV rv;
    
    // Sanitize arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Set the position where the FPS counter is to be rendered
 *
 * @param  [ in]pCtx The FPS counter
 * @param  [ in]x    The horizontal position
 * @param  [ in]y    The vertical position
 * @return           GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmFPSCounter_setPosition(gfmFPSCounter *pCtx, int x, int y) {
    gfmRV rv;

    /* Sanitize arguments */
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);

    /* Set the position */
    pCtx->x = x;
    pCtx->y = y;

    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Signal when an update started, to calculate how long it took
 * 
 * @param  pCtx      The FPS counter
 * @return           GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmFPSCounter_updateBegin(gfmFPSCounter *pCtx) {
    gfmRV rv;
    
    // Sanitize arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    
    // Store the moment an update was initialized
    rv = gfmTimer_getCurTimeMs(&(pCtx->updateInit));
    ASSERT_NR(rv == GFMRV_OK);
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Signal that the update ended, and actually calculate how long it took
 * 
 * @param  pCtx      The FPS counter
 * @return           GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmFPSCounter_updateEnd(gfmFPSCounter *pCtx) {
    gfmRV rv;
    unsigned int curTime;
    
    // Sanitize arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    
    // Store the moment an update was initialized
    rv = gfmTimer_getCurTimeMs(&curTime);
    ASSERT_NR(rv == GFMRV_OK);
    
    // Update the update operation time
    pCtx->updateTime += curTime - pCtx->updateInit;
    
    // Update the number of updates
    pCtx->updateAcc++;
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Called on gfm_drawBegin to calculate how long it takes to render a frame
 * 
 * @param  pCtx      The FPS counter
 * @return           GFMRV_OK, GFMRV_ARGUMENTS_BAD, ...
 */
gfmRV gfmFPSCounter_initDraw(gfmFPSCounter *pCtx) {
    gfmRV rv;
    
    // Sanitize arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    
    // Store the moment a draw was initialized
    rv = gfmTimer_getCurTimeMs(&(pCtx->drawInit));
    ASSERT_NR(rv == GFMRV_OK);
    
    // Increase the draw count
    pCtx->drawAcc++;
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Draw the FPS counter on the screen; It's position should be defined before
 * hand
 * 
 * @param  pCounter The FPS counter
 * @param  pCtx     The game's context
 * @return          GFMRV_OK, GFMRV_ARGUMENTS_BAD,
 *                  GFMRV_FPSCOUNTER_NOT_INITIALIZED
 */
gfmRV gfmFPSCounter_draw(gfmFPSCounter *pCounter, gfmCtx *pCtx) {
    gfmRV rv;
    int dps, ups;
    unsigned int curTime;

    /* Sanitize arguments */
    ASSERT(pCounter, GFMRV_ARGUMENTS_BAD);
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);

    /* Calculate how long it took to draw */
    rv = gfmTimer_getCurTimeMs(&curTime);
    ASSERT_NR(rv == GFMRV_OK);

    /* Check if the fps should be updated */
    if (curTime - pCounter->lastTime >= 1000) {
        pCounter->updateCount = pCounter->updateAcc;
        pCounter->updateAcc = 0;
        pCounter->drawCount = pCounter->drawAcc;
        pCounter->drawAcc = 0;

        pCounter->lastTime = curTime;
    }

    /* Get the expected FPS */
    rv = gfm_getStateFrameRate(&ups, &dps, pCtx);
    ASSERT_NR(rv == GFMRV_OK);

    /* Draw the information */
    gfmDebug_printf(pCtx, pCounter->x, pCounter->y,
            " UPD %02i/%02i - %04iMS\n"
            "DRAW %02i/%02i - %04iMS\n",
            pCounter->updateCount, ups, pCounter->updateTime,
            pCounter->drawCount, dps, curTime - pCounter->drawInit);
    pCounter->updateTime = 0;

    rv = GFMRV_OK;
__ret:
    return rv;
}

