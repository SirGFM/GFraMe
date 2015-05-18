/**
 * @file tst/gframe_loop_tst.c
 * 
 * Simple test that runs a loop for a few frames
 */
#include <GFraMe/gframe.h>
#include <GFraMe/gfmAssert.h>
#include <GFraMe/gfmError.h>
#include <GFraMe/gfmTilemap.h>
#include <GFraMe/gfmSpriteset.h>

// Set the game's FPS
#define FPS       60

/** Create the tilemap in a lazy way */
int pTmData[] = {
#include "../assets/map.csv"
};

/** Create the animations */
int pTmAnims[] = {
/* num|fps|loop|frames... */
    // Electric thing loop
    4 , 16,  1 , 8,10,12,14,
    4 , 16,  1 , 9,11,13,15,
    // Monitor transition from rest to flicker (long)
    2 , 4 ,  0 ,16,18,
    2 , 4 ,  0 ,17,19,
    2 , 4 ,  0 ,24,26,
    2 , 4 ,  0 ,25,27,
    // Monitor flickering and going to rest (short and quick)
    3 , 12,  0 ,18,20,16,
    3 , 12,  0 ,19,21,17,
    3 , 12,  0 ,26,28,24,
    3 , 12,  0 ,27,29,25,
};

int main(int arg, char *argv[]) {
    gfmCtx *pCtx;
    gfmRV rv;
    gfmTilemap *pTMap;
    gfmSpriteset *pSset;
    int iTex;
    
    // Initialize every variable
    pCtx = 0;
    pTMap = 0;
    pSset = 0;
    
    // Try to get a new context
    rv = gfm_getNew(&pCtx);
    ASSERT_NR(rv == GFMRV_OK);
    
    // Try to set a title
    rv = gfm_setTitleStatic(pCtx, "com.gfmgamecorner", "gframe_test_loop");
    ASSERT_NR(rv == GFMRV_OK);
    
    // Initialize the window
    rv = gfm_initGameWindow(pCtx, 160, 120, 640, 480, 0);
    ASSERT_NR(rv == GFMRV_OK);
    
    // Load the texture
    rv = gfm_loadTextureStatic(&iTex, pCtx, "tm_atlas_font.bmp", 0xff00ff);
    ASSERT_NR(rv == GFMRV_OK);
    // Set it as the default
    rv = gfm_setDefaultTexture(pCtx, iTex);
    ASSERT_NR(rv == GFMRV_OK);
    
    // Create a spriteset
    rv = gfmSpriteset_getNew(&pSset);
    ASSERT_NR(rv == GFMRV_OK);
    rv = gfmSpriteset_initCached(pSset, pCtx, iTex, 8/*tw*/, 8/*th*/);
    ASSERT_NR(rv == GFMRV_OK);
    
    // Initalize the FPS counter
    rv = gfm_initFPSCounter(pCtx, pSset, 64/*firstTile*/);
    
    // Create and load the tilemap
    rv = gfmTilemap_getNew(&pTMap);
    ASSERT_NR(rv == GFMRV_OK);
    rv = gfmTilemap_init(pTMap, pSset, 20/*mapWidth*/, 15/*mapHeight*/,
            0/*defTile*/);
    ASSERT_NR(rv == GFMRV_OK);
    rv = gfmTilemap_loadStatic(pTMap, pTmData, 20/*mapWidth*/, 15/*mapHeight*/);
    ASSERT_NR(rv == GFMRV_OK);
    // Add the tilemap's animations
    rv = gfmTilemap_addAnimationsStatic(pTMap, pTmAnims);
    ASSERT_NR(rv == GFMRV_OK);
    // Recache the animations
    rv = gfmTilemap_recacheAnimations(pTMap);
    ASSERT_NR(rv == GFMRV_OK);
    
    // Set the main loop framerate
    rv = gfm_setStateFrameRate(pCtx, FPS, FPS);
    ASSERT_NR(rv == GFMRV_OK);
    // Initialize the timer
    rv = gfm_setFPS(pCtx, FPS);
    ASSERT_NR(rv == GFMRV_OK);
    
    // Run until the window is closed
    while (gfm_didGetQuitFlag(pCtx) == GFMRV_FALSE) {
        int frames, ms;
        
        rv = gfm_handleEvents(pCtx);
        ASSERT_NR(rv == GFMRV_OK);
        
        // Update stuff
        rv = gfm_getUpdates(&frames, pCtx);
        ASSERT_NR(rv == GFMRV_OK);
        rv = gfm_getElapsedTime(&ms, pCtx);
        while (frames > 0) {
            rv = gfmTilemap_update(pTMap, ms);
            ASSERT_NR(rv == GFMRV_OK);
            
            rv = gfm_updateFPSCounter(pCtx);
            ASSERT_NR(rv == GFMRV_OK);
            
            frames--;
        }
        
        // Draw stuff
        rv = gfm_getDraws(&frames, pCtx);
        ASSERT_NR(rv == GFMRV_OK);
        while (frames > 0) {
            rv = gfm_drawBegin(pCtx);
            ASSERT_NR(rv == GFMRV_OK);
            
            rv = gfmTilemap_draw(pTMap, pCtx);
            ASSERT_NR(rv == GFMRV_OK);
            
            rv = gfm_drawEnd(pCtx);
            ASSERT_NR(rv == GFMRV_OK);
            
            frames--;
        }
    }
    
    rv = GFMRV_OK;
__ret:
    gfmTilemap_free(&pTMap);
    gfmSpriteset_free(&pSset);
    gfm_free(&pCtx);
    
    return rv;
}


