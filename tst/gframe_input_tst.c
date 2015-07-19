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
#include <GFraMe/gfmText.h>

// Set the game's FPS
#define FPS       60
#define WNDW     160
#define WNDH     120

static char pressLeft[]  = "PRESS THE LEFT KEY";
static char pressRight[] = "PRESS THE RIGHT KEY";
static char pressUp[]    = "PRESS THE UP KEY";
static char pressDown[]  = "PRESS THE DOWN KEY";
static char pressReset[] = "PRESS 'R' TO REBIND";

/**
 * Reset all key bindings
 * 
 * @param  pCtx  The game's context
 * @param  space The space v-key handle
 * @param  reset The reset v-key handle
 * @return       GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV resetBinding(gfmCtx *pCtx, int space, int reset) {
    gfmRV rv;
    
    // Sanitize arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    
    // Reset all bindings
    rv = gfm_resetInput(pCtx);
    ASSERT_NR(rv == GFMRV_OK);
    // Bind both 'space' and 'reset'
    rv = gfm_bindInput(pCtx, space, gfmKey_space);
    ASSERT_NR(rv == GFMRV_OK);
    rv = gfm_bindInput(pCtx, reset, gfmKey_r);
    ASSERT_NR(rv == GFMRV_OK);
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

int main(int arg, char *argv[]) {
    gfmCtx *pCtx;
    gfmGroup *pGrp;
    gfmRV rv;
    gfmSprite *pSpr, *pPlayer;
    gfmSpriteset *pSset4, *pSset8;
    gfmText *pText;
    int iTex;
    int left, right, up, down, space, reset;
    int keysSet;
    
    // Initialize every variable
    pCtx = 0;
    pGrp = 0;
    pPlayer = 0;
    pText = 0;
    
    // Try to get a new context
    rv = gfm_getNew(&pCtx);
    ASSERT_NR(rv == GFMRV_OK);
    rv = gfm_init(pCtx);
    ASSERT_NR(rv == GFMRV_OK);
    
    // Try to set a title
    rv = gfm_setTitleStatic(pCtx, "com.gfmgamecorner", "gframe_input");
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
    rv = gfm_addVirtualKey(&space, pCtx);
    ASSERT_NR(rv == GFMRV_OK);
    rv = gfm_addVirtualKey(&reset, pCtx);
    ASSERT_NR(rv == GFMRV_OK);
    
    // Bind only the 'genGif' and 'reset' buttons
    rv = resetBinding(pCtx, space, reset);
    ASSERT_NR(rv == GFMRV_OK);
    // Force all keys to be rebound
    keysSet = 0;
    
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
    
    // Initialize the text
    rv = gfmText_getNew(&pText);
    ASSERT_NR(rv == GFMRV_OK);
    rv = gfmText_init(pText, 0/*x*/, WNDH - 8/*y*/, WNDW / 8/*maxWidth*/,
            1/*maxLines*/, 60/*delay*/, 0/*bindToWorld*/, pSset8,
            0/*firstTile*/);
    ASSERT_NR(rv == GFMRV_OK);
    // Also set the first text
    rv = gfmText_setTextStatic(pText, pressLeft, 0/*doCopy*/);
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
    rv = gfmGroup_preCache(pGrp, 0/*initLen*/, 8192/*maxLen*/);
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
    
    // Run until the window is closed
    while (gfm_didGetQuitFlag(pCtx) == GFMRV_FALSE) {
        rv = gfm_handleEvents(pCtx);
        ASSERT_NR(rv == GFMRV_OK);
        
        // Update stuff
        while (gfm_isUpdating(pCtx) == GFMRV_TRUE) {
            gfmInputState kleft, kright, kup, kdown, kspace, kreset;
            int i, nleft, nright, nup, ndown, nspace, nreset, x, y;
            
            rv = gfm_fpsCounterUpdateBegin(pCtx);
            ASSERT_NR(rv == GFMRV_OK);
            
            // Retrieve every key state (or set keys, if not yet set)
            if (keysSet < 4) {
                gfmInputIface iface;
                
                kleft = gfmInput_released;
                kright = gfmInput_released;
                kup = gfmInput_released;
                kdown = gfmInput_released;
                
                // Get the last pressed key
                rv = gfm_getLastPressed(&iface, pCtx);
                ASSERT_NR(rv == GFMRV_OK || rv == GFMRV_WAITING);
                
                if (rv == GFMRV_OK) {
                    int handle;
                    
                    switch (keysSet) {
                        case 0: {
                            handle = left;
                            rv = gfmText_setTextStatic(pText, pressRight,
                                    0/*doCopy*/);
                            ASSERT_NR(rv == GFMRV_OK);
                        } break;
                        case 1: {
                            handle = right;
                            rv = gfmText_setTextStatic(pText, pressUp,
                                    0/*doCopy*/);
                            ASSERT_NR(rv == GFMRV_OK);
                        } break;
                        case 2: {
                            handle = up;
                            rv = gfmText_setTextStatic(pText, pressDown,
                                    0/*doCopy*/);
                            ASSERT_NR(rv == GFMRV_OK);
                        } break;
                        case 3: {
                            handle = down;
                            rv = gfmText_setTextStatic(pText, pressReset,
                                    0/*doCopy*/);
                            ASSERT_NR(rv == GFMRV_OK);
                        } break;
                        default: handle = 0;
                    }
                    
                    rv = gfm_bindInput(pCtx, handle, iface);
                    ASSERT_NR(rv == GFMRV_OK);
                    
                    keysSet++;
                }
            }
            else {
                rv = gfm_getKeyState(&kleft, &nleft, pCtx, left);
                ASSERT_NR(rv == GFMRV_OK);
                rv = gfm_getKeyState(&kright, &nright, pCtx, right);
                ASSERT_NR(rv == GFMRV_OK);
                rv = gfm_getKeyState(&kup, &nup, pCtx, up);
                ASSERT_NR(rv == GFMRV_OK);
                rv = gfm_getKeyState(&kdown, &ndown, pCtx, down);
                ASSERT_NR(rv == GFMRV_OK);
            }
            
            // Set horizontal speed
            if ((kleft & gfmInput_pressed) == gfmInput_pressed) {
                rv = gfmSprite_setHorizontalVelocity(pPlayer, -100);
                ASSERT_NR(rv == GFMRV_OK);
            }
            else if ((kright & gfmInput_pressed) == gfmInput_pressed) {
                rv = gfmSprite_setHorizontalVelocity(pPlayer, 100);
                ASSERT_NR(rv == GFMRV_OK);
            }
            else {
                rv = gfmSprite_setHorizontalVelocity(pPlayer, 0);
                ASSERT_NR(rv == GFMRV_OK);
            }
            // Set vertical speed
            if ((kup & gfmInput_pressed) == gfmInput_pressed) {
                rv = gfmSprite_setVerticalVelocity(pPlayer, -100);
                ASSERT_NR(rv == GFMRV_OK);
            }
            else if ((kdown & gfmInput_pressed) == gfmInput_pressed) {
                rv = gfmSprite_setVerticalVelocity(pPlayer, 100);
                ASSERT_NR(rv == GFMRV_OK);
            }
            else {
                rv = gfmSprite_setVerticalVelocity(pPlayer, 0);
                ASSERT_NR(rv == GFMRV_OK);
            }
            
            rv = gfm_getKeyState(&kspace, &nspace, pCtx, space);
            ASSERT_NR(rv == GFMRV_OK);
            rv = gfm_getKeyState(&kreset, &nreset, pCtx, reset);
            ASSERT_NR(rv == GFMRV_OK);
            
            if ((kspace & gfmInput_justPressed) == gfmInput_justPressed) {
                int ms;
                
                ms = 5000;
                // start GIF
                rv = gfm_recordGif(pCtx, ms, "anim.gif", 8, 0);
                ASSERT_NR(rv == GFMRV_OK);
            }
            // Reset all bindings
            if ((kreset & gfmInput_justPressed) == gfmInput_justPressed) {
                rv = resetBinding(pCtx, space, reset);
                ASSERT_NR(rv == GFMRV_OK);
                // Force all keys to be rebound
                keysSet = 0;
                rv = gfmText_setTextStatic(pText, pressLeft,
                        0/*doCopy*/);
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
            // Update the text
            rv = gfmText_update(pText, pCtx);
            ASSERT_NR(rv == GFMRV_OK);
            
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
            // Draw the 'player'
            rv = gfmSprite_draw(pPlayer, pCtx);
            ASSERT_NR(rv == GFMRV_OK);
            // Draw the text
            rv = gfmText_draw(pText, pCtx);
            ASSERT_NR(rv == GFMRV_OK);
            
            rv = gfm_drawEnd(pCtx);
            ASSERT_NR(rv == GFMRV_OK);
        }
    }
    
    rv = GFMRV_OK;
__ret:
    gfmText_free(&pText);
    gfmSprite_free(&pPlayer);
    gfmGroup_free(&pGrp);
    gfm_free(&pCtx);
    
    return rv;
}


