/**
 * @file tst/gframe_texture_tst.c
 * 
 * Simple test to check the framework's basic rendering functionalities; It
 * loads a texture, create a spriteset for it and renders it on the screen
 */
#include <GFraMe/gframe.h>
#include <GFraMe/gfmAssert.h>
#include <GFraMe/gfmError.h>
#include <GFraMe/gfmSpriteset.h>

int main(int arg, char *argv[]) {
    gfmCtx *pCtx;
    gfmRV rv;
    gfmSpriteset *pSset;
    int iTex;
    
    // Initialize every variable
    pCtx = 0;
    pSset = 0;
    
    // Try to get a new context
    rv = gfm_getNew(&pCtx);
    ASSERT_NR(rv == GFMRV_OK);
    rv = gfm_initStatic(pCtx, "com.gfmgamecorner", "gframe_test_texture");
    ASSERT_NR(rv == GFMRV_OK);
    
    // Initialize the window
    rv = gfm_initGameWindow(pCtx, 320, 240, 640, 480, 0);
    ASSERT_NR(rv == GFMRV_OK);
    
    // Load the texture
    rv = gfm_loadTextureStatic(&iTex, pCtx, "atlas.bmp", 0xff00ff);
    ASSERT_NR(rv == GFMRV_OK);
    // Set it as the default
    rv = gfm_setDefaultTexture(pCtx, iTex);
    ASSERT_NR(rv == GFMRV_OK);
    
    // Create a spriteset
    rv = gfmSpriteset_getNew(&pSset);
    ASSERT_NR(rv == GFMRV_OK);
    rv = gfmSpriteset_initCached(pSset, pCtx, iTex, 16/*tw*/, 16/*th*/);
    ASSERT_NR(rv == GFMRV_OK);
    
    // Draw something
    rv = gfm_drawBegin(pCtx);
    ASSERT_NR(rv == GFMRV_OK);
    rv = gfm_drawTile(pCtx, pSset, 0/*x*/, 0/*y*/, 0/*tile*/);
    ASSERT_NR(rv == GFMRV_OK);
    rv = gfm_drawTile(pCtx, pSset, 16/*x*/, 0/*y*/, 1/*tile*/);
    ASSERT_NR(rv == GFMRV_OK);
    rv = gfm_drawTile(pCtx, pSset, 0/*x*/, 16/*y*/, 2/*tile*/);
    ASSERT_NR(rv == GFMRV_OK);
    rv = gfm_drawTile(pCtx, pSset, 16/*x*/, 16/*y*/, 3/*tile*/);
    ASSERT_NR(rv == GFMRV_OK);
    rv = gfm_drawEnd(pCtx);
    ASSERT_NR(rv == GFMRV_OK);
    
    rv = GFMRV_OK;
__ret:
    gfmSpriteset_free(&pSset);
    gfm_free(&pCtx);
    
    return rv;
}

