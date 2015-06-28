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

// Use a macro to easily make the program sleep in windows and linux
#if defined(WIN32)
#  define USLEEP(n) ;//Sleep(n)
#else
#  include <unistd.h>
#  define USLEEP(n) usleep(n*100)
#endif

// Those values are broken, but it doesn't really matter for now...
#define NUMFRAMES 1000
#define FPS       60
#define DELAY     1000 / FPS

/** Create the tilemap in a lazy way */
int pTmData[] = {
#include "../assets/map.csv"
};

// Those values are broken, but it doesn't really matter for now...
/** Create the animations */
int pTmAnims[] = {
/* num|fps|loop|frames... */
    4 , 16,  1 , 8,10,12,14,
    4 , 16,  1 , 9,11,13,15,
    3 , 8 ,  1 ,16,18,20,
    3 , 8 ,  1 ,17,19,21,
    3 , 8 ,  1 ,24,26,28,
    3 , 8 ,  1 ,25,27,29
};

int main(int arg, char *argv[]) {
    gfmCtx *pCtx;
    gfmRV rv;
    gfmTilemap *pTMap;
    gfmSpriteset *pSset;
    int i, iTex;
    
    // Initialize every variable
    pCtx = 0;
    pTMap = 0;
    pSset = 0;
    
    // Try to get a new context
    rv = gfm_getNew(&pCtx);
    ASSERT_NR(rv == GFMRV_OK);
    rv = gfm_init(pCtx);
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
    // Add the tilemap's animations
    rv = gfmTilemap_addAnimationsStatic(pTMap, pTmAnims);
    ASSERT_NR(rv == GFMRV_OK);
    // Recache the animations
    rv = gfmTilemap_recacheAnimations(pTMap);
    ASSERT_NR(rv == GFMRV_OK);
    
    // Run some frames
    i = 0;
    while (i < NUMFRAMES) {
        // Update it to the next frame
        rv = gfmTilemap_update(pTMap, pCtx);
        
        // Draw something
        rv = gfm_drawBegin(pCtx);
        ASSERT_NR(rv == GFMRV_OK);
        rv = gfmTilemap_draw(pTMap, pCtx);
        ASSERT_NR(rv == GFMRV_OK);
        rv = gfm_drawEnd(pCtx);
        ASSERT_NR(rv == GFMRV_OK);
        
        // Sleep for sometime
        USLEEP(DELAY);
        i++;
    }
    
    rv = GFMRV_OK;
__ret:
    gfmTilemap_free(&pTMap);
    gfmSpriteset_free(&pSset);
    gfm_free(&pCtx);
    
    return rv;
}


