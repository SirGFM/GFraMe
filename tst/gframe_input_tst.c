/**
 * @file tst/gframe_group_tst.c
 * 
 * Simple test with group
 */
#include <GFraMe/gframe.h>
#include <GFraMe/gfmAssert.h>
#include <GFraMe/gfmError.h>
#include <GFraMe/gfmGroup.h>
#include <GFraMe/gfmSprite.h>
#include <GFraMe/gfmSpriteset.h>

// Set the game's FPS
#define FPS       60
#define WNDW     160
#define WNDH     120

int main(int arg, char *argv[]) {
    gfmCtx *pCtx;
    gfmGroup *pGrp;
    gfmRV rv;
    gfmSprite *pSpr, *pPlayer;
    gfmSpriteset *pSset4, *pSset8;
    int iTex;
    //int ms;
    int left, right, up, down;
    
    // Initialize every variable
    pCtx = 0;
    pGrp = 0;
    pPlayer = 0;
    
    // Try to get a new context
    rv = gfm_getNew(&pCtx);
    ASSERT_NR(rv == GFMRV_OK);
    rv = gfm_init(pCtx);
    ASSERT_NR(rv == GFMRV_OK);
    
    // Try to set a title
    rv = gfm_setTitleStatic(pCtx, "com.gfmgamecorner", "gframe_group_loop");
    ASSERT_NR(rv == GFMRV_OK);
    
    // Initialize the window
    rv = gfm_initGameWindow(pCtx, WNDW, WNDH, 640, 480, 0);
    ASSERT_NR(rv == GFMRV_OK);
    
    // Create the inputs
    rv = gfm_addVirtualKey(&left, pCtx);
    ASSERT_NR(rv == GFMRV_OK);
    rv = gfm_addVirtualKey(&right, pCtx);
    ASSERT_NR(rv == GFMRV_OK);
    rv = gfm_addVirtualKey(&up, pCtx);
    ASSERT_NR(rv == GFMRV_OK);
    rv = gfm_addVirtualKey(&down, pCtx);
    ASSERT_NR(rv == GFMRV_OK);
    
    // Bind a few keys to each input (this generates a perfectly balanced tree)
    rv = gfm_bindInput(pCtx, left, gfmKey_left);
    ASSERT_NR(rv == GFMRV_OK);
    rv = gfm_bindInput(pCtx, left, gfmKey_a);
    ASSERT_NR(rv == GFMRV_OK);
    rv = gfm_bindInput(pCtx, up, gfmKey_up);
    ASSERT_NR(rv == GFMRV_OK);
    rv = gfm_bindInput(pCtx, up, gfmKey_w);
    ASSERT_NR(rv == GFMRV_OK);
    rv = gfm_bindInput(pCtx, right, gfmKey_right);
    ASSERT_NR(rv == GFMRV_OK);
    rv = gfm_bindInput(pCtx, right, gfmKey_d);
    ASSERT_NR(rv == GFMRV_OK);
    rv = gfm_bindInput(pCtx, down, gfmKey_down);
    ASSERT_NR(rv == GFMRV_OK);
    rv = gfm_bindInput(pCtx, down, gfmKey_s);
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
    
    // Create a 'player'
    rv = gfmSprite_getNew(&pPlayer);
    ASSERT_NR(rv == GFMRV_OK);
    // Initialize it
    rv = gfmSprite_init(pPlayer, WNDW / 2, WNDH / 2, 8/*w*/, 8/*h*/, pSset8,
            -4/*offX*/, -4/*offY*/, 0, 0);
    ASSERT_NR(rv == GFMRV_OK);
    rv = gfmSprite_setFrame(pPlayer, '*' - '!');
    ASSERT_NR(rv == GFMRV_OK);
    
    // Create the group
    rv = gfmGroup_getNew(&pGrp);
    ASSERT_NR(rv == GFMRV_OK);
    // Set the group attributes
    rv = gfmGroup_setDefSpriteset(pGrp, pSset4);
    ASSERT_NR(rv == GFMRV_OK);
    rv = gfmGroup_setDefDimensions(pGrp, 4/*width*/, 4/*height*/, 0/*offX*/,
        0/*offY*/);
    ASSERT_NR(rv == GFMRV_OK);
    // Create the group's sprite
    rv = gfmGroup_preCache(pGrp, 0/*initLen*/, 1024/*maxLen*/);
    ASSERT_NR(rv == GFMRV_OK);
    // Those can be set after caching everything, since they are global
    rv = gfmGroup_setDeathOnTime(pGrp, 2000/*ttl*/);
    ASSERT_NR(rv == GFMRV_OK);
    // And those are set when the sprite is recycled, so...
    rv = gfmGroup_setDefVelocity(pGrp, 0/*vx*/, -175/*vy*/);
    ASSERT_NR(rv == GFMRV_OK);
    rv = gfmGroup_setDefAcceleration(pGrp, 0/*vx*/, 100/*vy*/);
    ASSERT_NR(rv == GFMRV_OK);
    // Set the draw order (FUN!!!)
    rv = gfmGroup_setDrawOrder(pGrp, gfmDrawOrder_oldestFirst);
    ASSERT_NR(rv == GFMRV_OK);
    
    // Set the main loop framerate
    rv = gfm_setStateFrameRate(pCtx, FPS, FPS);
    ASSERT_NR(rv == GFMRV_OK);
    // Initialize the timer
    rv = gfm_setFPS(pCtx, FPS);
    ASSERT_NR(rv == GFMRV_OK);
    
    // Request the recording of an animation
    //ms = 5000;
    //rv = gfm_recordGif(pCtx, ms, "anim.gif", 8, 0);
    //rv = gfm_snapshot(pCtx, "ss.gif", 6, 0);
    //ASSERT_NR(rv == GFMRV_OK);
    
    // Run until the window is closed
    while (gfm_didGetQuitFlag(pCtx) == GFMRV_FALSE) {
        int frames;
        
        rv = gfm_handleEvents(pCtx);
        ASSERT_NR(rv == GFMRV_OK);
        
        // Update stuff
        rv = gfm_getUpdates(&frames, pCtx);
        ASSERT_NR(rv == GFMRV_OK);
        while (frames > 0) {
            gfmInputState kleft, kright, kup, kdown;
            int i, nleft, nright, nup, ndown, x, y;
            rv = gfm_fpsCounterUpdateBegin(pCtx);
            ASSERT_NR(rv == GFMRV_OK);
            
            // Retrieve every key state
            rv = gfm_getKeyState(&kleft, &nleft, pCtx, left);
            ASSERT_NR(rv == GFMRV_OK);
            rv = gfm_getKeyState(&kright, &nright, pCtx, right);
            ASSERT_NR(rv == GFMRV_OK);
            rv = gfm_getKeyState(&kup, &nup, pCtx, up);
            ASSERT_NR(rv == GFMRV_OK);
            rv = gfm_getKeyState(&kdown, &ndown, pCtx, down);
            ASSERT_NR(rv == GFMRV_OK);
            
            if (kleft & gfmInput_pressed) {
                rv = gfmSprite_setHorizontalVelocity(pPlayer, -100);
                ASSERT_NR(rv == GFMRV_OK);
            }
            else if (kright & gfmInput_pressed) {
                rv = gfmSprite_setHorizontalVelocity(pPlayer, 100);
                ASSERT_NR(rv == GFMRV_OK);
            }
            else {
                rv = gfmSprite_setHorizontalVelocity(pPlayer, 0);
                ASSERT_NR(rv == GFMRV_OK);
            }
            if (kup & gfmInput_pressed) {
                rv = gfmSprite_setVerticalVelocity(pPlayer, -100);
                ASSERT_NR(rv == GFMRV_OK);
            }
            else if (kdown & gfmInput_pressed) {
                rv = gfmSprite_setVerticalVelocity(pPlayer, 100);
                ASSERT_NR(rv == GFMRV_OK);
            }
            else {
                rv = gfmSprite_setVerticalVelocity(pPlayer, 0);
                ASSERT_NR(rv == GFMRV_OK);
            }
            
            // Get the sprite's position
            rv = gfmSprite_getPosition(&x, &y, pPlayer);
            ASSERT_NR(rv == GFMRV_OK);
            
            // Recycle a few sprites
            i = 0;
            while (i < 7) {
                rv = gfmGroup_recycle(&pSpr, pGrp);
                ASSERT_NR(rv == GFMRV_OK || rv == GFMRV_GROUP_MAX_SPRITES);
                if (rv == GFMRV_OK) {
                    int vx, vy;
                    
                    rv = gfmGroup_setPosition(pGrp, x, y);
                    ASSERT_NR(rv == GFMRV_OK);
                    rv = gfmGroup_setFrame(pGrp, i);
                    ASSERT_NR(rv == GFMRV_OK);
                    vx = -15 + i * 5;
                    vy = -50;
                    rv = gfmGroup_setVelocity(pGrp, vx, vy);
                    ASSERT_NR(rv == GFMRV_OK);
                }
                i++;
            }
            
            // Update group
            rv = gfmGroup_update(pGrp, pCtx);
            ASSERT_NR(rv == GFMRV_OK);
            // Update the player's physics
            rv = gfmSprite_update(pPlayer, pCtx);
            ASSERT_NR(rv == GFMRV_OK);
            
            rv = gfm_fpsCounterUpdateEnd(pCtx);
            ASSERT_NR(rv == GFMRV_OK);
            
            frames--;
        }
        
        // Draw stuff
        rv = gfm_getDraws(&frames, pCtx);
        ASSERT_NR(rv == GFMRV_OK);
        while (frames > 0) {
            rv = gfm_drawBegin(pCtx);
            ASSERT_NR(rv == GFMRV_OK);
            
            // Draw group
            rv = gfmGroup_draw(pGrp, pCtx);
            ASSERT_NR(rv == GFMRV_OK);
            // Draw the 'player'
            rv = gfmSprite_draw(pPlayer, pCtx);
            ASSERT_NR(rv == GFMRV_OK);
            
            rv = gfm_drawEnd(pCtx);
            ASSERT_NR(rv == GFMRV_OK);
            
            frames--;
        }
    }
    
    rv = GFMRV_OK;
__ret:
    gfmSprite_free(&pPlayer);
    gfmGroup_free(&pGrp);
    gfm_free(&pCtx);
    
    return rv;
}


