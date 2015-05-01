/**
 * @file tst/gframe_tstWindowBasic.c
 * 
 * Simple test to check the framework's main module functions; So far, it goes
 * through creating and resizing a window
 */
#include <GFraMe/gframe.h>
#include <GFraMe/gfmAssert.h>
#include <GFraMe/gfmError.h>
#include <GFraMe/core/gfmTexture_bkend.h>

int main(int arg, char *argv[]) {
    gfmCtx *pCtx;
    gfmRV rv;
    gfmTexture *pTex;
    
    // Initialize every variable
    pCtx = 0;
    pTex = 0;
    
    // Try to get a new context
    rv = gfm_getNew(&pCtx);
    ASSERT_NR(rv == GFMRV_OK);
    
    // Try to set a title
    rv = gfm_setTitleStatic(pCtx, "com.gfmgamecorner", "gframe_test_texture");
    ASSERT_NR(rv == GFMRV_OK);
    
    // Initialize the window
    rv = gfm_initGameWindow(pCtx, 320, 240, 640, 480, 0);
    ASSERT_NR(rv == GFMRV_OK);
    
    // Get a new texture
    rv = gfmTexture_getNew(&pTex);
    ASSERT_NR(rv == GFMRV_OK);
    // Initialize it
    rv = gfmTexture_init(pTex, pCtx, 32, 32);
    ASSERT_NR(rv == GFMRV_OK);
    // Load the data
    rv = gfmTexture_loadStatic(pTex, pCtx, "atlas.bmp", 0xff00ff);
    ASSERT_NR(rv == GFMRV_OK);
    
    rv = GFMRV_OK;
__ret:
    gfmTexture_free(&pTex);
    // In case some error happened
    gfm_free(&pCtx);
    
    return rv;
}

