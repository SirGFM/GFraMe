/**
 * @file tst/gframe_group_tst.c
 * 
 * Simple test with group
 */
#include <GFraMe/gframe.h>
#include <GFraMe/gfmAssert.h>
#include <GFraMe/gfmError.h>
#include <GFraMe/gfmGroup.h>
#include <GFraMe/gfmQuadtree.h>
#include <GFraMe/gfmSprite.h>
#include <GFraMe/gfmSpriteset.h>
#include <GFraMe/gfmText.h>
#include <GFraMe/gfmTypes.h>

// Set the game's FPS
#define FPS       60
#define WNDW     320
#define WNDH     240
#define PARTICLE gfmType_reserved_2

int main(int arg, char *argv[]) {
    gfmCtx *pCtx;
    gfmGroup *pGrp;
    gfmRV rv;
    gfmQuadtreeRoot *pQt;
    gfmSpriteset *pSset4, *pSset8;
    int click, gif, frame, iTex;
    
    // Initialize every variable
    pCtx = 0;
    pGrp = 0;
    pQt = 0;
    
    // Try to get a new context
    rv = gfm_getNew(&pCtx);
    ASSERT_NR(rv == GFMRV_OK);
    rv = gfm_initStatic(pCtx, "com.gfmgamecorner", "gframe_particle_collision");
    ASSERT_NR(rv == GFMRV_OK);
    
    // Initialize the window
    rv = gfm_initGameWindow(pCtx, WNDW, WNDH, 640, 480, 0);
    ASSERT_NR(rv == GFMRV_OK);
    
    // Create the inputs
    rv = gfm_addVirtualKey(&click, pCtx);
    ASSERT_NR(rv == GFMRV_OK);
    rv = gfm_bindInput(pCtx, click, gfmPointer_button);
    ASSERT_NR(rv == GFMRV_OK);
    rv = gfm_addVirtualKey(&gif, pCtx);
    ASSERT_NR(rv == GFMRV_OK);
    rv = gfm_bindInput(pCtx, gif, gfmKey_space);
    ASSERT_NR(rv == GFMRV_OK);
    
    // Load the texture
    rv = gfm_loadTextureStatic(&iTex, pCtx, "rainbow_atlas.bmp", 0xff00ff);
    ASSERT_NR(rv == GFMRV_OK);
    // Set it as the default
    rv = gfm_setDefaultTexture(pCtx, iTex);
    ASSERT_NR(rv == GFMRV_OK);
    
    // Create the spritesets
    rv = gfm_createSpritesetCached(&pSset8, pCtx, iTex, 8/*tw*/, 8/*th*/);
    ASSERT_NR(rv == GFMRV_OK);
    rv = gfm_createSpritesetCached(&pSset4, pCtx, iTex, 4/*tw*/, 4/*th*/);
    ASSERT_NR(rv == GFMRV_OK);
    
    // Initalize the FPS counter
    rv = gfm_initFPSCounter(pCtx, pSset8, 0/*firstTile*/);
    ASSERT_NR(rv == GFMRV_OK);
    
    // Create the group
    rv = gfmGroup_getNew(&pGrp);
    ASSERT_NR(rv == GFMRV_OK);
    // Set the group attributes
    rv = gfmGroup_setDefSpriteset(pGrp, pSset4);
    ASSERT_NR(rv == GFMRV_OK);
    rv = gfmGroup_setDefDimensions(pGrp, 4/*width*/, 4/*height*/, -2/*offX*/,
        -2/*offY*/);
    ASSERT_NR(rv == GFMRV_OK);
    // Create the group's sprite
    rv = gfmGroup_preCache(pGrp, 0/*initLen*/, 8192/*maxLen*/);
    ASSERT_NR(rv == GFMRV_OK);
    // Those can be set after caching everything, since they are global
    rv = gfmGroup_setDeathOnLeave(pGrp, 1);
    ASSERT_NR(rv == GFMRV_OK);
    // Set the draw order (FUN!!!)
    rv = gfmGroup_setDrawOrder(pGrp, gfmDrawOrder_oldestFirst);
    ASSERT_NR(rv == GFMRV_OK);
    rv = gfmGroup_setDefType(pGrp, PARTICLE);
    ASSERT_NR(rv == GFMRV_OK);
    // Set collision (EVEN MORE FUN!!!)
    rv = gfmGroup_setCollisionQuality(pGrp,
            gfmCollisionQuality_collideEverything);
    ASSERT_NR(rv == GFMRV_OK);
    
    rv = gfmQuadtree_getNew(&pQt);
    ASSERT_NR(rv == GFMRV_OK);
    
    // Set the main loop framerate
    rv = gfm_setStateFrameRate(pCtx, FPS, FPS);
    ASSERT_NR(rv == GFMRV_OK);
    // Initialize the timer
    rv = gfm_setFPS(pCtx, FPS);
    ASSERT_NR(rv == GFMRV_OK);
    
    // Run until the window is closed
    frame = 0;
    while (gfm_didGetQuitFlag(pCtx) == GFMRV_FALSE) {
        rv = gfm_handleEvents(pCtx);
        ASSERT_NR(rv == GFMRV_OK);
        
        // Update stuff
        while (gfm_isUpdating(pCtx) == GFMRV_TRUE) {
            gfmInput *pInput;
            gfmInputState kclick, kgif;
            int nclick, ngif;
            
            rv = gfm_fpsCounterUpdateBegin(pCtx);
            ASSERT_NR(rv == GFMRV_OK);
            
            rv = gfm_getInput(&pInput, pCtx);
            ASSERT_NR(rv == GFMRV_OK);
            
            rv = gfm_getKeyState(&kclick, &nclick, pCtx, click);
            ASSERT_NR(rv == GFMRV_OK);
            rv = gfm_getKeyState(&kgif, &ngif, pCtx, gif);
            ASSERT_NR(rv == GFMRV_OK);
            
            if ((kgif & gfmInput_justPressed) == gfmInput_justPressed) {
                gfm_didExportGif(pCtx);
                rv = gfm_recordGif(pCtx, 10000/*ms*/, "anim.gif", 8, 0);
                ASSERT_NR(rv == GFMRV_OK);
            }
            
            // Spawn particles!
            if ((kclick & gfmInput_pressed) == gfmInput_pressed && nclick == 2) {
                int i, x, y;
                
                rv = gfmInput_getPointerPosition(&x, &y, pInput);
                ASSERT_NR(rv == GFMRV_OK);
                
                i = 0;
                while (i < 8) {
                    gfmSprite *pSpr;
                    
                    rv = gfmGroup_recycle(&pSpr, pGrp);
                    ASSERT_NR(rv == GFMRV_OK || rv == GFMRV_GROUP_MAX_SPRITES);
                    
                    if (rv == GFMRV_OK) {
                        int sprX, sprY;
                        float fvx, fvy;
                        
                        if (i < 4) {
                            int vx, vy;
                            
                            // Set X = 1 on even and Y = 1 on odd
                            vx = (i % 2);
                            vy = 1 - vx;
                            // Set to positive on 0,1 and negative on 2,3
                            fvx = vx * (1 - i / 2) - vx * (i / 2);
                            fvy = vy * (1 - i / 2) - vy * (i / 2);
                        }
                        else if (i >= 4) {
                            int j;
                            
                            // Set to positive on 0,1 and negative on 2,3
                            j = i % 4;
                            fvx = (1 - j / 2) - (j / 2);
                            // Set to positive on 1,2 and negative on 0,3
                            j = (i - 1) % 4;
                            fvy = (1 - j / 2) - (j / 2);
                            
                            fvx *= 0.7071;
                            fvy *= 0.7071;
                        }
                        
                        sprX = x + 16 * fvx;
                        sprY = y + 16 * fvy;
                        fvx *= 100;
                        fvy *= 100;
                        
                        rv = gfmGroup_setPosition(pGrp, sprX, sprY);
                        ASSERT_NR(rv == GFMRV_OK);
                        rv = gfmGroup_setFrame(pGrp, frame);
                        ASSERT_NR(rv == GFMRV_OK);
                        rv = gfmGroup_setVelocity(pGrp, fvx, fvy);
                        ASSERT_NR(rv == GFMRV_OK);
                        
                        frame = (frame + 1) % 7;
                    }
                    i++;
                }
            }
            
            // Update group
            rv = gfmGroup_update(pGrp, pCtx);
            ASSERT_NR(rv == GFMRV_OK);
            
            rv = gfmQuadtree_initRoot(pQt, 0, 0, WNDW, WNDH, 6, 10);
            ASSERT_NR(rv == GFMRV_OK);
            
            // Collide everything
            rv = gfmQuadtree_collideGroup(pQt, pGrp);
            ASSERT(rv == GFMRV_QUADTREE_OVERLAPED || rv == GFMRV_QUADTREE_DONE,
                    rv);
            while (rv != GFMRV_QUADTREE_DONE) {
                gfmObject *pObj1, *pObj2;
                gfmSprite *pSpr1, *pSpr2;
                gfmGroupNode *pNode1, *pNode2;
                int type1, type2;
                
                // Retrieve the colliding objects
                rv = gfmQuadtree_getOverlaping(&pObj1, &pObj2, pQt);
                ASSERT(rv == GFMRV_OK, rv);
                
                // Retrieve its child and the child's type
                rv = gfmObject_getChild((void**)&pSpr1, &type1, pObj1);
                ASSERT(rv == GFMRV_OK, rv);
                rv = gfmObject_getChild((void**)&pSpr2, &type2, pObj2);
                ASSERT(rv == GFMRV_OK, rv);
                
                // Retrieve the child's child and type
                if (type1 == gfmType_sprite) {
                    rv = gfmSprite_getChild((void**)&pNode1, &type1, pSpr1);
                    ASSERT(rv == GFMRV_OK, rv);
                }
                if (type2 == gfmType_sprite) {
                    rv = gfmSprite_getChild((void**)&pNode2, &type2, pSpr2);
                    ASSERT(rv == GFMRV_OK, rv);
                }
                
                // If two particles are colliding, do fun stuff
                if (type1 == PARTICLE && type2 == PARTICLE) {
                    rv = gfmObject_collide(pObj1, pObj2);
                    ASSERT(rv == GFMRV_TRUE || rv == GFMRV_FALSE, rv);
                    
                    if (rv == GFMRV_TRUE) {
                        double vx, vy;
                        
                        rv = gfmObject_getVelocity(&vx, &vy, pObj1);
                        ASSERT(rv == GFMRV_OK, rv);
                        rv = gfmObject_setVelocity(pObj1, vy * 1.01, vx * 1.01);
                        ASSERT(rv == GFMRV_OK, rv);
                        rv = gfmObject_getVelocity(&vx, &vy, pObj2);
                        ASSERT(rv == GFMRV_OK, rv);
                        rv = gfmObject_setVelocity(pObj2, vy * 1.01, vx * 1.01);
                        ASSERT(rv == GFMRV_OK, rv);
                    }
                }
                
                rv = gfmQuadtree_continue(pQt);
                ASSERT(rv == GFMRV_QUADTREE_OVERLAPED ||
                        rv == GFMRV_QUADTREE_DONE, rv);
            }
            
            rv = gfm_fpsCounterUpdateEnd(pCtx);
            ASSERT_NR(rv == GFMRV_OK);
        }
        
        // Draw stuff
        while (gfm_isDrawing(pCtx) == GFMRV_TRUE) {
            rv = gfm_drawBegin(pCtx);
            ASSERT_NR(rv == GFMRV_OK);
            
            // Draw group
            rv = gfmGroup_draw(pGrp, pCtx);
            ASSERT_NR(rv == GFMRV_OK);
            
            rv = gfm_drawEnd(pCtx);
            ASSERT_NR(rv == GFMRV_OK);
        }
    }
    
    rv = GFMRV_OK;
__ret:
    gfmGroup_free(&pGrp);
    gfmQuadtree_free(&pQt);
    gfm_free(&pCtx);
    
    return rv;
}


