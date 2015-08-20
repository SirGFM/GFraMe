/**
 * @file tst/gframe_quadtree_tst.c
 * 
 * Quadtree test with a tilemap and moving actors(?)
 */
#include <GFraMe/gframe.h>
#include <GFraMe/gfmAssert.h>
#include <GFraMe/gfmError.h>
#include <GFraMe/gfmQuadtree.h>
#include <GFraMe/gfmSprite.h>
#include <GFraMe/gfmSpriteset.h>
#include <GFraMe/gfmTilemap.h>
#include <GFraMe/gfmTypes.h>

// Set the game's FPS
#define FPS       60
#define WNDW     160
#define WNDH     120

// Static buffer with the tilemap data
static int pBuf[] = {
    #include "../assets/map2.csv"
};

// Define two types for the tiles
#define collideable gfmType_reserved_2
#define spikes      gfmType_reserved_3

// Static buffer with the type data
static int pTypes[] = {
    64 , collideable,
    66 , collideable,
    65 , collideable,
    81 , collideable,
    80 , collideable,
    82 , collideable,
    83 , collideable,
    84 , collideable,
    114, collideable,
    115, collideable,
    116, spikes
};

// Define the animations
enum {
    ANIM_STAND = 0,
    ANIM_WALK,
    ANIM_HURT,
    ANIM_JUMP,
    ANIM_FALL,
    ANIM_MAX,
};

/** Create the animations */
int pSprAnims[] = {
/* num|fps|loop|frames... */
    // Stand animation
    8 ,  8,  1 , 32,32,43,32,32,44,32,45,
    // Walk animation
    8 , 14,  1 , 33,34,35,36,37,38,39,40,
    // Hurt animation
    8 , 12,  0 , 41,42,41,42,41,42,41,42,
    // Jump animation
    1 ,  0,  0 , 46,
    // Fall animation
    1 ,  0,  0 , 47
};

int main(int arg, char *argv[]) {
    gfmCtx *pCtx;
    gfmQuadtreeRoot *pQTRoot;
    gfmRV rv;
    gfmSprite *pSpr;
    gfmSpriteset *pSset8, *pSset16;
    gfmTilemap *pTMap;
    int iTex;
    
    // Initialize every variable
    pCtx = 0;
    pQTRoot = 0;
    pSpr = 0;
    pTMap = 0;
    
    // Try to get a new context
    rv = gfm_getNew(&pCtx);
    ASSERT_NR(rv == GFMRV_OK);
    rv = gfm_initStatic(pCtx, "com.gfmgamecorner", "gframe_quadtree_basic");
    ASSERT_NR(rv == GFMRV_OK);
    
    // Initialize the window
    rv = gfm_initGameWindow(pCtx, WNDW, WNDH, 640, 480, 0);
    ASSERT_NR(rv == GFMRV_OK);
    
    // Load the texture
    rv = gfm_loadTextureStatic(&iTex, pCtx, "ld32-atlas.bmp", 0xff00ff);
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
    rv = gfm_initFPSCounter(pCtx, pSset8, 0/*firstTile*/);
    ASSERT_NR(rv == GFMRV_OK);
    
    // Create a tilemap
    rv = gfmTilemap_getNew(&pTMap);
    ASSERT_NR(rv == GFMRV_OK);
    // Initialize it
    rv = gfmTilemap_init(pTMap, pSset8, WNDW / 8, WNDH / 8, 0);
    ASSERT_NR(rv == GFMRV_OK);
    // Load it from memory
    rv = gfmTilemap_loadStatic(pTMap, pBuf, WNDW / 8, WNDH / 8);
    ASSERT_NR(rv == GFMRV_OK);
    // Initialize the per-tile types
    rv = gfmTilemap_addTileTypesStatic(pTMap, pTypes);
    ASSERT_NR(rv == GFMRV_OK);
    rv = gfmTilemap_recalculateAreas(pTMap);
    ASSERT_NR(rv == GFMRV_OK);
    
    // Create a sprite
    rv = gfmSprite_getNew(&pSpr);
    ASSERT_NR(rv == GFMRV_OK);
    // Initialize it
    rv = gfmSprite_init(pSpr, 16/*x*/, 16/*y*/, 6/*width*/, 12/*height*/,
            pSset16, -4/*offX*/, -4/*offY*/, 0/*pChild*/, 0/*type*/);
    ASSERT_NR(rv == GFMRV_OK);
    // Add the animations
    rv = gfmSprite_addAnimationsStatic(pSpr, pSprAnims);
    ASSERT_NR(rv == GFMRV_OK);
    // Set the sprite's gravity
    rv = gfmSprite_setVerticalAcceleration(pSpr, 500.0);
    ASSERT_NR(rv == GFMRV_OK);
    
    // Play an animation
    rv = gfmSprite_playAnimation(pSpr, ANIM_FALL);
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
            
            // Update the tilemap
            ASSERT_NR(rv == GFMRV_OK);
            
            // Update the sprite's physics
            rv = gfmSprite_update(pSpr, pCtx);
            ASSERT_NR(rv == GFMRV_OK);
            
            // Initialize the root of the quadtree
            //rv = gfmQuadtree_initRoot(pQTRoot, -16, -16, WNDW+32, WNDH+32, 2, 4);
            //ASSERT_NR(rv == GFMRV_OK);
            
            // These parameters forces the qt to be subdivided even with 2 objs
            // Also, it's a little smaller than the screen to be visible
            rv = gfmQuadtree_initRoot(pQTRoot, 2, 2, WNDW-4, WNDH-4, 2, 1);
            ASSERT_NR(rv == GFMRV_OK);
            // Populate the quadtree with the object
            rv = gfmQuadtree_populateTilemap(pQTRoot, pTMap);
            ASSERT_NR(rv == GFMRV_OK);
            
            // Add the sprite, colliding with everything
            rv = gfmQuadtree_collideSprite(pQTRoot, pSpr);
            ASSERT_NR(rv == GFMRV_QUADTREE_DONE ||
                    rv == GFMRV_QUADTREE_OVERLAPED);
            while (rv != GFMRV_QUADTREE_DONE) {
                gfmObject *pObj1, *pObj2;
                gfmSprite *pSpr1, *pSpr2;
                int pType1, pType2;
                
                // Get the objects that collided
                rv = gfmQuadtree_getOverlaping(&pObj1, &pObj2, pQTRoot);
                ASSERT_NR(rv == GFMRV_OK);
                
                // Separate both objects
                rv = gfmObject_separateHorizontal(pObj1, pObj2);
                rv = gfmObject_separateVertical(pObj1, pObj2);
                
                // Try to get both sprites children
                rv = gfmObject_getChild((void**)&pSpr1, &pType1, pObj1);
                ASSERT_NR(rv == GFMRV_OK);
                rv = gfmObject_getChild((void**)&pSpr2, &pType2, pObj2);
                ASSERT_NR(rv == GFMRV_OK);
                
                // Change the sprite animation and cap its velocity
                if (pType1 == gfmType_sprite) {
                    rv = gfmSprite_playAnimation(pSpr1, ANIM_STAND);
                    ASSERT_NR(rv == GFMRV_OK);
                    rv = gfmSprite_setVerticalVelocity(pSpr1, 0.0);
                    ASSERT_NR(rv == GFMRV_OK);
                    rv = gfmSprite_setVerticalAcceleration(pSpr1, 0.0);
                    ASSERT_NR(rv == GFMRV_OK);
                }
                if (pType2 == gfmType_sprite) {
                    rv = gfmSprite_playAnimation(pSpr2, ANIM_STAND);
                    ASSERT_NR(rv == GFMRV_OK);
                    rv = gfmSprite_setVerticalVelocity(pSpr2, 0.0);
                    ASSERT_NR(rv == GFMRV_OK);
                    rv = gfmSprite_setVerticalAcceleration(pSpr2, 0.0);
                    ASSERT_NR(rv == GFMRV_OK);
                }
                
                // Continue colliding
                rv = gfmQuadtree_continue(pQTRoot);
                ASSERT_NR(rv == GFMRV_QUADTREE_DONE ||
                        rv == GFMRV_QUADTREE_OVERLAPED);
            }
            
            rv = gfm_fpsCounterUpdateEnd(pCtx);
            ASSERT_NR(rv == GFMRV_OK);
        }
        
        // Draw stuff
        while (gfm_isDrawing(pCtx) == GFMRV_TRUE) {
            rv = gfm_drawBegin(pCtx);
            ASSERT_NR(rv == GFMRV_OK);
            
            // Draw the tilemap
            rv = gfmTilemap_update(pTMap, pCtx);
            rv = gfmTilemap_draw(pTMap, pCtx);
            ASSERT_NR(rv == GFMRV_OK);
            
            // Draw the sprite
            rv = gfmSprite_draw(pSpr, pCtx);
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
    gfmSprite_free(&pSpr);
    gfm_free(&pCtx);
    
    return rv;
}

