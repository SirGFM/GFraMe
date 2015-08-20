/**
 * @file src/include/GFraMe_int/gfmFPSCounter.h
 * 
 * Module that calculates and displays the current FPS
 * This module requires gfmTimer_getCurTimeMs to be implemented!
 */
#ifndef __GFMFPSCOUNTER_STRUCT_H__
#define __GFMFPSCOUNTER_STRUCT_H__

/** 'Exports' the gfmFPSCounter structure */
typedef struct stGFMFPSCounter gfmFPSCounter;

#endif /* __GFMFPSCOUNTER_STRUCT_H__ */

#ifndef __GFMFPSCOUNTER_H__
#define __GFMFPSCOUNTER_H__

#include <GFraMe/gframe.h>
#include <GFraMe/gfmError.h>
#include <GFraMe/gfmSpriteset.h>

/** 'Exportable' size of gfmFPSCounter */
extern const int sizeofGFMFPSCounter;

/**
 * Alloc a new FPS counter structure
 * 
 * @param  ppCtx The FPS counter
 * @return       GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_ALLOC_FAILED
 */
gfmRV gfmFPSCounter_getNew(gfmFPSCounter **ppCtx);

/**
 * Release a previously alloc'ed FPS counter
 * 
 * @param  ppCtx The FPS counter
 * @return       GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmFPSCounter_free(gfmFPSCounter **ppCtx);

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
        int firstTile);

/**
 * Signal when an update started, to calculate how long it took
 * 
 * @param  pCtx      The FPS counter
 * @return           GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmFPSCounter_updateBegin(gfmFPSCounter *pCtx);

/**
 * Signal that the update ended, and actually calculate how long it took
 * 
 * @param  pCtx      The FPS counter
 * @return           GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmFPSCounter_updateEnd(gfmFPSCounter *pCtx);

/**
 * Called on gfm_drawBegin to calculate how long it takes to render a frame
 * 
 * @param  pCtx      The FPS counter
 * @return           GFMRV_OK, GFMRV_ARGUMENTS_BAD, ...
 */
gfmRV gfmFPSCounter_initDraw(gfmFPSCounter *pCtx);

/**
 * Draw the FPS counter on the screen; It's position should be defined before
 * hand
 * 
 * @param  pCounter The FPS counter
 * @param  pCtx     The game's context
 * @return          GFMRV_OK, GFMRV_ARGUMENTS_BAD,
 *                  GFMRV_FPSCOUNTER_NOT_INITIALIZED
 */
gfmRV gfmFPSCounter_draw(gfmFPSCounter *pCounter, gfmCtx *pCtx);

#endif /* __GFMFPSCOUNTER_H__ */

