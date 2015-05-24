/**
 * @file tst/gframe_sprite_tst.c
 * 
 * Simple test with sprites
 */
#include <GFraMe/gframe.h>
#include <GFraMe/gfmAssert.h>
#include <GFraMe/gfmError.h>
#include <GFraMe/gfmSprite.h>
#include <GFraMe/gfmSpriteset.h>

// Set the game's FPS
#define FPS       60

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
    gfmSprite *pSpr;
    gfmSpriteset *pSset8, *pSset16;
    int iTex;
    
    // Initialize every variable
    pCtx = 0;
    pSpr = 0;
    
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
    rv = gfm_loadTextureStatic(&iTex, pCtx, "big_atlas.bmp", 0xff00ff);
    ASSERT_NR(rv == GFMRV_OK);
    // Set it as the default
    rv = gfm_setDefaultTexture(pCtx, iTex);
    ASSERT_NR(rv == GFMRV_OK);
    
    // Create the spritesets
    rv = gfm_createSpritesetCached(&pSset8, pCtx, iTex, 8/*tw*/, 8/*th*/);
    ASSERT_NR(rv == GFMRV_OK);
    rv = gfm_createSpritesetCached(&pSset16, pCtx, iTex, 16/*tw*/, 16/*th*/);
    ASSERT_NR(rv == GFMRV_OK);
    
    // Initalize the FPS counter
    rv = gfm_initFPSCounter(pCtx, pSset8, 64/*firstTile*/);
    
    // Create the sprite
    rv = gfmSprite_getNew(&pSpr);
    ASSERT_NR(rv == GFMRV_OK);
    rv = gfmSprite_init(pSpr, 16/*x*/, 16/*y*/, 6/*width*/, 12/*height*/,
            pSset16, -4/*offX*/, -4/*offY*/, 0/*pChild*/, 0/*type*/);
    ASSERT_NR(rv == GFMRV_OK);
    rv = gfmSprite_setFrame(pSpr, 32);
    ASSERT_NR(rv == GFMRV_OK);
    
    // Set the main loop framerate
    rv = gfm_setStateFrameRate(pCtx, FPS, FPS);
    ASSERT_NR(rv == GFMRV_OK);
    // Initialize the timer
    rv = gfm_setFPS(pCtx, FPS);
    ASSERT_NR(rv == GFMRV_OK);
    
    // Run until the window is closed
    while (gfm_didGetQuitFlag(pCtx) == GFMRV_FALSE) {
        int frames;
        
        rv = gfm_handleEvents(pCtx);
        ASSERT_NR(rv == GFMRV_OK);
        
        // Update stuff
        rv = gfm_getUpdates(&frames, pCtx);
        ASSERT_NR(rv == GFMRV_OK);
        while (frames > 0) {
            rv = gfm_fpsCounterUpdateBegin(pCtx);
            ASSERT_NR(rv == GFMRV_OK);
            
            rv = gfmSprite_update(pSpr, pCtx);
            ASSERT_NR(rv == GFMRV_OK);
            
            rv = gfm_fpsCounterUpdateEnd(pCtx);
            ASSERT_NR(rv == GFMRV_OK);
            
            frames--;
        }
        
        // Draw stuff
        rv = gfm_getDraws(&frames, pCtx);
        ASSERT_NR(rv == GFMRV_OK);
        while (frames > 0) {
            rv = gfm_drawBegin(pCtx);
            ASSERT_NR(rv == GFMRV_OK);
            
            rv = gfmSprite_draw(pSpr, pCtx);
            ASSERT_NR(rv == GFMRV_OK);
            
            rv = gfm_drawEnd(pCtx);
            ASSERT_NR(rv == GFMRV_OK);
            
            frames--;
        }
    }
    
    rv = GFMRV_OK;
__ret:
    gfmSprite_free(&pSpr);
    gfm_free(&pCtx);
    
    return rv;
}


