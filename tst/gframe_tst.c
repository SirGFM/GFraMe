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

