/**
 * @file tst/gframe_tilemap_load_tst.c
 * 
 * Simple test to check the framework's tilemap loading
 */
#include <GFraMe/gframe.h>
#include <GFraMe/gfmAssert.h>
#include <GFraMe/gfmError.h>
#include <GFraMe/gfmTilemap.h>
#include <GFraMe/gfmTypes.h>
#include <GFraMe/gfmQuadtree.h>
#include <GFraMe/gfmSpriteset.h>

// Set the game's FPS
#define FPS       60
#define WNDW     160
#define WNDH     120

/** Dictionaries for the tilemap */
char *pDictStr[] = {
    "floor",
    "spike"
};
int pDictTypes[] = {
    gfmType_reserved_2,
    gfmType_reserved_3
};

int main(int arg, char *argv[]) {
    gfmCtx *pCtx;
    gfmRV rv;
    gfmQuadtreeRoot *pQTRoot;
    gfmSpriteset *pSset;
    gfmTilemap *pTMap;
    int iTex;
    
    // Initialize every variable
    pCtx = 0;
    pTMap = 0;
    pSset = 0;
    pQTRoot = 0;
    
    // Try to get a new context
    rv = gfm_getNew(&pCtx);
    ASSERT_NR(rv == GFMRV_OK);
    rv = gfm_initStatic(pCtx, "com.gfmgamecorner", "gframe_test_tilemap");
    ASSERT_NR(rv == GFMRV_OK);
    
    // Initialize the window
    rv = gfm_initGameWindow(pCtx, 160, 120, 640, 480, 0, 0);
    ASSERT_NR(rv == GFMRV_OK);
    
    // Load the texture
    rv = gfm_loadTextureStatic(&iTex, pCtx, "ld32-atlas.bmp", 0xff00ff);
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
    rv = gfmTilemap_init(pTMap, pSset, 1/*mapWidth*/, 1/*mapHeight*/,
            0/*defTile*/);
    ASSERT_NR(rv == GFMRV_OK);
    rv = gfmTilemap_loadf(pTMap, pCtx, "map.gfm", 7/*strLen*/, pDictStr,
            pDictTypes, 2);
    ASSERT_NR(rv == GFMRV_OK);
    
    // Alloc the quadtree
    rv = gfmQuadtree_getNew(&pQTRoot);
    ASSERT_NR(rv == GFMRV_OK);
    
    // Set the main loop framerate
    rv = gfm_setStateFrameRate(pCtx, FPS, FPS);
    ASSERT_NR(rv == GFMRV_OK);
    // Initialize the timer
    rv = gfm_setFPS(pCtx, FPS);
    ASSERT_NR(rv == GFMRV_OK);
    
    // Run until the window is closed
    while (gfm_didGetQuitFlag(pCtx) == GFMRV_FALSE) {
        rv = gfm_handleEvents(pCtx);
        ASSERT_NR(rv == GFMRV_OK);
        
        // Update stuff
        while (gfm_isUpdating(pCtx) == GFMRV_TRUE) {
            rv = gfm_fpsCounterUpdateBegin(pCtx);
            ASSERT_NR(rv == GFMRV_OK);
            
            // These parameters forces the qt to be subdivided even with 2 objs
            // Also, it's a little smaller than the screen to be visible
            rv = gfmQuadtree_initRoot(pQTRoot, 2, 2, WNDW-4, WNDH-4, 2, 1);
            ASSERT_NR(rv == GFMRV_OK);
            // Populate the quadtree with the object
            rv = gfmQuadtree_populateTilemap(pQTRoot, pTMap);
            ASSERT_NR(rv == GFMRV_OK);
            
            rv = gfm_fpsCounterUpdateEnd(pCtx);
            ASSERT_NR(rv == GFMRV_OK);
        }
        
        // Draw stuff
        while (gfm_isDrawing(pCtx) == GFMRV_TRUE) {
            rv = gfm_drawBegin(pCtx);
            ASSERT_NR(rv == GFMRV_OK);
            
            rv = gfmTilemap_draw(pTMap, pCtx);
            ASSERT_NR(rv == GFMRV_OK);
            // Draw the quadtree's bounds
            rv = gfmQuadtree_drawBounds(pQTRoot, pCtx, 0);
            ASSERT_NR(rv == GFMRV_OK);
            
            rv = gfm_drawEnd(pCtx);
            ASSERT_NR(rv == GFMRV_OK);
        }
    }
            
    rv = GFMRV_OK;
__ret:
    gfmQuadtree_free(&pQTRoot);
    gfmTilemap_free(&pTMap);
    gfmSpriteset_free(&pSset);
    gfm_free(&pCtx);
    
    return rv;
}


