/**
 * Simple test with save files
 *
 * @file tst/gframe_save_tst.c
 */
#include <GFraMe/gframe.h>
#include <GFraMe/gfmAssert.h>
#include <GFraMe/gfmError.h>
#include <GFraMe/gfmSave.h>

int main(int argc, char *argv[]) {
    gfmCtx *pCtx;
    gfmRV rv;
    gfmSave *pSave;

    /* Initialize those to 0 */
    pCtx = 0;
    pSave = 0;

    rv = gfm_getNew(&pCtx);
    ASSERT(rv == GFMRV_OK, rv);

    rv = gfm_initStatic(pCtx, "com.gfmgamecorner", "gframe_save_tst");
    ASSERT(rv == GFMRV_OK, rv);

    rv = gfmSave_getNew(&pSave);
    ASSERT(rv == GFMRV_OK, rv);

    rv = gfmSave_bindStatic(pSave, pCtx, "test.sav");
    ASSERT(rv == GFMRV_OK, rv);

    rv = GFMRV_OK;
__ret:
    if (pSave) {
        gfmSave_erase(pSave);
    }
    gfmSave_free(&pSave);
    gfm_free(&pCtx);

    return rv;
}

