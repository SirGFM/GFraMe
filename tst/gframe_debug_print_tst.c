/**
 * @file tst/gframe_debug_print_tst.c
 * 
 * Simple test with sprites
 */
#include <GFraMe/gframe.h>
#include <GFraMe/gfmAssert.h>
#include <GFraMe/gfmDebug.h>
#include <GFraMe/gfmError.h>
#include <GFraMe/gfmTilemap.h>
#include <GFraMe/gfmSpriteset.h>

#include <GFraMe_int/gfmCtx_struct.h>

// Set the game's FPS
#define FPS       60
/** Create a tilemap in a lazy way */
int pTmData[] = {
#include "assets/map.csv"
};

int main(int arg, char *argv[]) {
    gfmCtx *pCtx;
    gfmTilemap *pTMap;
    gfmSpriteset *pSset;
    int tex, x, y;
    gfmRV rv;

    // Initialize every variable
    pCtx = 0;
    pTMap = 0;

    // Try to get a new context
    rv = gfm_getNew(&pCtx);
    ASSERT_NR(rv == GFMRV_OK);
    rv = gfm_initStatic(pCtx, "com.gfmgamecorner", "gframe_debug_print_tst");
    ASSERT_NR(rv == GFMRV_OK);

    // Initialize the window
    rv = gfm_initGameWindow(pCtx, 160, 120, 640, 480, 1, 0);
    ASSERT_NR(rv == GFMRV_OK);

    rv = gfm_loadTextureStatic(&tex, pCtx, "tm_atlas.bmp", 0xff00ff);
    ASSERT_NR(rv == GFMRV_OK);
    rv = gfm_createSpritesetCached(&pSset, pCtx, tex, 8/*x*/, 8/*h*/);
    ASSERT_NR(rv == GFMRV_OK);

    // Create and load the tilemap
    rv = gfmTilemap_getNew(&pTMap);
    ASSERT_NR(rv == GFMRV_OK);
    rv = gfmTilemap_init(pTMap, pSset, 20/*mapWidth*/, 15/*mapHeight*/,
            0/*defTile*/);
    ASSERT_NR(rv == GFMRV_OK);
    rv = gfmTilemap_loadStatic(pTMap, pTmData, 20/*mapWidth*/, 15/*mapHeight*/);
    ASSERT_NR(rv == GFMRV_OK);

    // Initalize the FPS counter
    //rv = gfm_initFPSCounter(pCtx, pSset8, 64/*firstTile*/);
    //ASSERT_NR(rv == GFMRV_OK);

    // Set the main loop framerate
    rv = gfm_setStateFrameRate(pCtx, FPS, FPS);
    ASSERT_NR(rv == GFMRV_OK);
    // Initialize the timer
    rv = gfm_setFPS(pCtx, FPS);
    ASSERT_NR(rv == GFMRV_OK);

    // Run until the window is closed
    while (gfm_didGetQuitFlag(pCtx) == GFMRV_FALSE) {
        rv = gfm_handleEvents(pCtx);
        ASSERT_NR(rv == GFMRV_OK);

        // Update stuff
        while (gfm_isUpdating(pCtx) == GFMRV_TRUE) {
            rv = gfm_fpsCounterUpdateBegin(pCtx);
            ASSERT_NR(rv == GFMRV_OK);

            rv = gfm_fpsCounterUpdateEnd(pCtx);
            ASSERT_NR(rv == GFMRV_OK);
        }

        // Draw stuff
        while (gfm_isDrawing(pCtx) == GFMRV_TRUE) {
            char *pos = "POS";
            rv = gfm_drawBegin(pCtx);
            ASSERT_NR(rv == GFMRV_OK);

            rv = gfmInput_getPointerPosition(&x, &y, pCtx->pInput);
            ASSERT_NR(rv == GFMRV_OK);

            rv = gfmTilemap_draw(pTMap, pCtx);
            ASSERT_NR(rv == GFMRV_OK);

            gfmDebug_printf(pCtx, 4/*x*/, 4/*y*/, "MOVE YOUR MOUSE:\n%s(%03X, %03X)", pos, x, y);
            gfmDebug_printf(pCtx, x, y - 12, "POS: (%i, %i)", x, y);

            rv = gfm_drawEnd(pCtx);
            ASSERT_NR(rv == GFMRV_OK);
        }
    }

    rv = GFMRV_OK;
__ret:
    gfmTilemap_free(&pTMap);
    gfm_free(&pCtx);

    return rv;
}


