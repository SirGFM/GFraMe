/**
 * @file tst/gframe_continuous_collision_tst.c
 */
#include <GFraMe/gframe.h>
#include <GFraMe/gfmAssert.h>
#include <GFraMe/gfmError.h>
#include <GFraMe/gfmGroup.h>
#include <GFraMe/gfmSprite.h>
#include <GFraMe/gfmSpriteset.h>
#include <GFraMe/gfmText.h>

#define WNDW        320
#define WNDH        240
#define SPRA_INIX   (-4)
#define SPRA_SPEED  600
#define SPRB_INIX   (304)
#define SPRB_SPEED  -600
#define FPS         60


int main(int argc, char *argv[]) {
    gfmCtx *pCtx;
    gfmRV rv;
    gfmSprite *pSprA, *pSprB, *pSprC;
    gfmSpriteset *pSset8, *pSset16;
    int iTex;

    /* Initialize every variable */
    pCtx = 0;
    pSprA = 0;
    pSprB = 0;
    pSprC = 0;

    /* Initialize the test */
    rv = gfm_getNew(&pCtx);
    ASSERT_NR(rv == GFMRV_OK);
    rv = gfm_initStatic(pCtx, "com.gfmgamecorner", "continuous_collision");
    ASSERT_NR(rv == GFMRV_OK);
    rv = gfm_initGameWindow(pCtx, WNDW, WNDH, 640, 480, /*resize*/1
            , 0/*vsync*/);
    ASSERT_NR(rv == GFMRV_OK);

    /* Load the texture */
    rv = gfm_loadTextureStatic(&iTex, pCtx, "big_atlas.bmp", 0xff00ff);
    ASSERT_NR(rv == GFMRV_OK);
    rv = gfm_setDefaultTexture(pCtx, iTex);
    ASSERT_NR(rv == GFMRV_OK);
    rv = gfm_createSpritesetCached(&pSset8, pCtx, iTex, 8/*tw*/, 8/*th*/);
    ASSERT_NR(rv == GFMRV_OK);
    rv = gfm_createSpritesetCached(&pSset16, pCtx, iTex, 16/*tw*/, 16/*th*/);
    ASSERT_NR(rv == GFMRV_OK);

    /* Create some sprites */
    rv = gfmSprite_getNew(&pSprA);
    ASSERT_NR(rv == GFMRV_OK);
    rv = gfmSprite_init(pSprA, SPRA_INIX, WNDH / 2 - 8/*y*/, 4/*w*/, 4/*h*/
            , pSset16, 0/*offX*/, 0/*offY*/, 0/*child*/, 0/*type*/);
    ASSERT_NR(rv == GFMRV_OK);
    rv = gfmSprite_setHorizontalVelocity(pSprA, SPRA_SPEED);
    ASSERT_NR(rv == GFMRV_OK);

    rv = gfmSprite_getNew(&pSprB);
    ASSERT_NR(rv == GFMRV_OK);
    rv = gfmSprite_init(pSprB, SPRB_INIX, WNDH / 2 - 8/*y*/, 4/*w*/
            , 4/*h*/, pSset16, 0/*offX*/, 0/*offY*/, 0/*child*/, 0/*type*/);
    ASSERT_NR(rv == GFMRV_OK);
    rv = gfmSprite_setDirection(pSprB, 1/*flip*/);
    ASSERT_NR(rv == GFMRV_OK);
    rv = gfmSprite_setHorizontalVelocity(pSprB, SPRB_SPEED);
    ASSERT_NR(rv == GFMRV_OK);

    /* Start running it */
    rv = gfm_setStateFrameRate(pCtx, FPS, FPS);
    ASSERT_NR(rv == GFMRV_OK);
    rv = gfm_setFPS(pCtx, FPS);
    ASSERT_NR(rv == GFMRV_OK);
    while (gfm_didGetQuitFlag(pCtx) == GFMRV_FALSE) {
        rv = gfm_handleEvents(pCtx);
        ASSERT_NR(rv == GFMRV_OK);

        while (gfm_isUpdating(pCtx) == GFMRV_TRUE) {
            gfmObject *pObjA, *pObjB;

            rv = gfm_fpsCounterUpdateBegin(pCtx);
            ASSERT_NR(rv == GFMRV_OK);

            rv = gfmSprite_update(pSprA, pCtx);
            ASSERT_NR(rv == GFMRV_OK);
            rv = gfmSprite_update(pSprB, pCtx);
            ASSERT_NR(rv == GFMRV_OK);

            rv =  gfmSprite_getObject(&pObjA, pSprA);
            ASSERT_NR(rv == GFMRV_OK);
            rv =  gfmSprite_getObject(&pObjB, pSprB);
            ASSERT_NR(rv == GFMRV_OK);
            rv = gfmObject_sweepJustOverlaped(pObjA, pObjB);
            if (rv == GFMRV_TRUE) {
                gfmObject_sweepCollision(pObjA, pObjB);
            }

            rv = gfm_fpsCounterUpdateEnd(pCtx);
            ASSERT_NR(rv == GFMRV_OK);
        }

        while (gfm_isDrawing(pCtx) == GFMRV_TRUE) {
            rv = gfm_drawBegin(pCtx);
            ASSERT_NR(rv == GFMRV_OK);

            rv = gfmSprite_draw(pSprA, pCtx);
            ASSERT_NR(rv == GFMRV_OK);
            rv = gfmSprite_draw(pSprB, pCtx);
            ASSERT_NR(rv == GFMRV_OK);

            rv = gfm_drawEnd(pCtx);
            ASSERT_NR(rv == GFMRV_OK);
        }
    }

__ret:
    gfmSprite_free(&pSprC);
    gfmSprite_free(&pSprB);
    gfmSprite_free(&pSprA);
    gfm_free(&pCtx);
    
    return rv;
}

