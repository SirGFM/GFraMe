/**
 * @file tst/gframe_tst.c
 * 
 * Simple test to check the framework's main module functions
 */
#include <GFraMe/gframe.h>
#include <GFraMe/gfmAssert.h>
#include <GFraMe/gfmError.h>

int main(int arg, char *argv[]) {
    gfmCtx *pCtx;
    gfmRV rv;
    
    // Initialize every variable
    pCtx = 0;
    
    // Try to get a new context
    rv = gfm_getNew(&pCtx);
    ASSERT_NR(rv == GFMRV_OK);
    // Check that it won't be initialized twice
    rv = gfm_getNew(&pCtx);
    ASSERT_NR(rv == GFMRV_ARGUMENTS_BAD);
    
    // Try to set a title
    rv = gfm_setTitleStatic(pCtx, "com.gfmgamecorner", "gframe_test");
    ASSERT_NR(rv == GFMRV_OK);
    // Try to set the title again
    rv = gfm_setTitleStatic(pCtx, "com.gfmgamecorner", "gframe_test");
    ASSERT_NR(rv != GFMRV_OK);
    
    // Initialize with wrong arguments
    rv = gfm_initGameWindow(0, 320, 240, 640, 480, 0);
    ASSERT_NR(rv == GFMRV_ARGUMENTS_BAD);
    rv = gfm_initGameWindow(pCtx, 320, 240, 0, 480, 0);
    ASSERT_NR(rv == GFMRV_INVALID_WIDTH);
    rv = gfm_initGameWindow(pCtx, 320, 240, 10000, 480, 0);
    ASSERT_NR(rv == GFMRV_INVALID_WIDTH);
    rv = gfm_initGameWindow(pCtx, 320, 240, 640, 0, 0);
    ASSERT_NR(rv == GFMRV_INVALID_HEIGHT);
    rv = gfm_initGameWindow(pCtx, 320, 240, 640, 10000, 0);
    ASSERT_NR(rv == GFMRV_INVALID_HEIGHT);
    // Initialize the window
    rv = gfm_initGameWindow(pCtx, 320, 240, 640, 480, 0);
    ASSERT_NR(rv == GFMRV_OK);
    // Try to initialize the window
    rv = gfm_initGameWindow(pCtx, 320, 240, 640, 480, 0);
    ASSERT_NR(rv == GFMRV_WINDOW_ALREADY_INITIALIZED);
    
    // And free it
    rv = gfm_free(&pCtx);
    ASSERT_NR(rv == GFMRV_OK);
    // Check that it won't be freed twice
    rv = gfm_free(&pCtx);
    ASSERT_NR(rv == GFMRV_ARGUMENTS_BAD);
    
    rv = GFMRV_OK;
__ret:
    // In case some error happened
    gfm_free(&pCtx);
    
    return rv;
}

