/**
 * @file tst/gframe_tilemap_tst.c
 * 
 * Simple test to check the framework's tilemap
 */
#include <GFraMe/gframe.h>
#include <GFraMe/gfmAssert.h>
#include <GFraMe/gfmError.h>
#include <GFraMe/gfmTilemap.h>
#include <GFraMe/gfmSpriteset.h>

#include <unistd.h>

/** Create the tilemap in a lazy way */
int pTmData[] = {
#include "../assets/map.csv"
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
    rv = gfm_setTitleStatic(pCtx, "com.gfmgamecorner", "gframe_test_tilemap");
    ASSERT_NR(rv == GFMRV_OK);
    
    // Initialize the window
    rv = gfm_initGameWindow(pCtx, 160, 120, 640, 480, 0);
    ASSERT_NR(rv == GFMRV_OK);
    
    // Load the texture
    rv = gfm_loadTextureStatic(&iTex, pCtx, "tm_atlas.bmp", 0xff00ff);
    ASSERT_NR(rv == GFMRV_OK);
    // Set it as the default
    rv = gfm_setDefaultTexture(pCtx, iTex);
    ASSERT_NR(rv == GFMRV_OK);
    
    // Create a spriteset
    rv = gfmSpriteset_getNew(&pSset);
    ASSERT_NR(rv == GFMRV_OK);
    rv = gfmSpriteset_initCached(pSset, pCtx, iTex, 8/*tw*/, 8/*th*/);
    ASSERT_NR(rv == GFMRV_OK);
    
    // Create and load the tilemap
    rv = gfmTilemap_getNew(&pTMap);
    ASSERT_NR(rv == GFMRV_OK);
    rv = gfmTilemap_init(pTMap, pSset, 20/*mapWidth*/, 15/*mapHeight*/,
            0/*defTile*/);
    ASSERT_NR(rv == GFMRV_OK);
    rv = gfmTilemap_loadStatic(pTMap, pTmData, 20/*mapWidth*/, 15/*mapHeight*/);
    ASSERT_NR(rv == GFMRV_OK);
    
    // Draw something
    rv = gfm_drawBegin(pCtx);
    ASSERT_NR(rv == GFMRV_OK);
    rv = gfmTilemap_draw(pTMap, pCtx);
    ASSERT_NR(rv == GFMRV_OK);
    rv = gfm_drawEnd(pCtx);
    ASSERT_NR(rv == GFMRV_OK);
    
    sleep(2);
    
    rv = GFMRV_OK;
__ret:
    gfmTilemap_free(&pTMap);
    gfmSpriteset_free(&pSset);
    gfm_free(&pCtx);
    
    return rv;
}


