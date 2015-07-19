/**
 * @file tst/gframe_group_tst.c
 * 
 * Simple test with group
 */
#include <GFraMe/gframe.h>
#include <GFraMe/gfmAssert.h>
#include <GFraMe/gfmError.h>
#include <GFraMe/gfmSpriteset.h>
#include <GFraMe/gfmText.h>
#include <GFraMe/gfmTilemap.h>

// Set the game's FPS
#define FPS       60
#define WNDW     160
#define WNDH     120

static int tmapBuf[WNDW * WNDH / 8 / 8];
static char text1[] = "HEY! THIS IS QUITE A LONG TEXT THAT SHOULD BE "
        "AUTOMATICALLY BROKEN INTO LOTS OF LINES. AS SOON AS IT FINISHES, IT "
        "WILL BE POSSIBLE TO NAVIGATE THIS USING THE UP AND DOWN ARROW KEYS. "
        "THIS SHOULD BE QUITE USEFUL FOR RPGS AND THE LIKES... THOUGH I'VE "
        "NEVER SEEN IT DONE BEFORE, SO THERE'S THAT... WELL, ENOUGH OF MY "
        "BABLING. THIS SHOULD BE LONG ENOUGH FOR A SIMPLE TEST.\n"
        "\n"
        "\n"
        "\n"
        ":)";

int main(int arg, char *argv[]) {
    gfmCtx *pCtx;
    gfmRV rv;
    gfmSpriteset *pSset8;
    gfmText *pText;
    gfmTilemap *pTmap;
    int i, iTex;
    int up, down, space;
    
    // Initialize every variable
    pCtx = 0;
    pText = 0;
    pTmap = 0;
    
    // Try to get a new context
    rv = gfm_getNew(&pCtx);
    ASSERT_NR(rv == GFMRV_OK);
    rv = gfm_init(pCtx);
    ASSERT_NR(rv == GFMRV_OK);
    
    // Try to set a title
    rv = gfm_setTitleStatic(pCtx, "com.gfmgamecorner", "gframe_textbox");
    ASSERT_NR(rv == GFMRV_OK);
    
    // Initialize the window
    rv = gfm_initGameWindow(pCtx, WNDW, WNDH, 640, 480, 0);
    ASSERT_NR(rv == GFMRV_OK);
    
    // Create the inputs
    rv = gfm_addVirtualKey(&up, pCtx);
    ASSERT_NR(rv == GFMRV_OK);
    rv = gfm_addVirtualKey(&down, pCtx);
    ASSERT_NR(rv == GFMRV_OK);
    rv = gfm_addVirtualKey(&space, pCtx);
    ASSERT_NR(rv == GFMRV_OK);
    // Bind 'em to keys
    rv = gfm_bindInput(pCtx, up, gfmKey_up);
    ASSERT_NR(rv == GFMRV_OK);
    rv = gfm_bindInput(pCtx, down, gfmKey_down);
    ASSERT_NR(rv == GFMRV_OK);
    rv = gfm_bindInput(pCtx, space, gfmKey_space);
    ASSERT_NR(rv == GFMRV_OK);
    
    // Load the texture
    rv = gfm_loadTextureStatic(&iTex, pCtx, "big_atlas.bmp", 0xff00ff);
    ASSERT_NR(rv == GFMRV_OK);
    // Set it as the default
    rv = gfm_setDefaultTexture(pCtx, iTex);
    ASSERT_NR(rv == GFMRV_OK);
    
    // Create the spritesets
    rv = gfm_createSpritesetCached(&pSset8, pCtx, iTex, 8/*tw*/, 8/*th*/);
    ASSERT_NR(rv == GFMRV_OK);
    
    // Initalize the FPS counter
    rv = gfm_initFPSCounter(pCtx, pSset8, 64/*firstTile*/);
    ASSERT_NR(rv == GFMRV_OK);
    
    // Initialize the text
    rv = gfmText_getNew(&pText);
    ASSERT_NR(rv == GFMRV_OK);
    rv = gfmText_init(pText, 8/*x*/, 32/*y*/, WNDW / 8 - 2/*maxWidth*/,
            3/*maxLines*/, 60/*delay*/, 0/*bindToWorld*/, pSset8,
            64/*firstTile*/);
    ASSERT_NR(rv == GFMRV_OK);
    // Also set the first text
    rv = gfmText_setTextStatic(pText, text1, 0/*doCopy*/);
    ASSERT_NR(rv == GFMRV_OK);
    
    // Initialize the tilemap
    rv = gfmTilemap_getNew(&pTmap);
    ASSERT_NR(rv == GFMRV_OK);
    rv = gfmTilemap_init(pTmap, pSset8, WNDW / 8/*width*/, 5/*height*/,
            0/*defTile*/);
    ASSERT_NR(rv == GFMRV_OK);
    // Create the tilemap window
    i = 0;
    while (i < 5 * WNDW / 8) {
        int tile, x, y;
        
        // Get the current tile position
        x = i % (WNDW / 8);
        y = i / (WNDW / 8);
        
        /* Top left corner     */      if (x == 0        && y == 0  ) tile = 32;
        /* Bottom left corner  */ else if (x == 0        && y == 5-1) tile = 48;
        /* Left wall           */ else if (x == 0                   ) tile = 40;
        /* Top right corner    */ else if (x == WNDW/8-1 && y == 0  ) tile = 34;
        /* Bottom right corner */ else if (x == WNDW/8-1 && y == 5-1) tile = 50;
        /* Right wall          */ else if (x == WNDW/8-1            ) tile = 42;
        /* Ceiling             */ else if (                 y == 0  ) tile = 33;
        /* Floor               */ else if (                 y == 5-1) tile = 49;
        /* Center              */ else                                tile = 41;
        
        // Set the tile
        tmapBuf[x + (y + 3) * WNDW / 8] = tile;
        
        i++;
    }
    rv = gfmTilemap_loadStatic(pTmap, tmapBuf, WNDW / 8 /*width*/,
            WNDH / 8/*height*/);
    ASSERT_NR(rv == GFMRV_OK);
    
    // Set the main loop framerate
    rv = gfm_setStateFrameRate(pCtx, FPS, FPS);
    ASSERT_NR(rv == GFMRV_OK);
    // Initialize the timer
    rv = gfm_setFPS(pCtx, FPS);
    ASSERT_NR(rv == GFMRV_OK);
    
    // start GIF
    //rv = gfm_recordGif(pCtx, 30000/*ms*/, "anim.gif", 8, 0);
    //ASSERT_NR(rv == GFMRV_OK);
    
    // Run until the window is closed
    while (gfm_didGetQuitFlag(pCtx) == GFMRV_FALSE) {
        rv = gfm_handleEvents(pCtx);
        ASSERT_NR(rv == GFMRV_OK);
        
        // Update stuff
        while (gfm_isUpdating(pCtx) == GFMRV_TRUE) {
            gfmInputState kup, kdown, kspace;
            int nup, ndown, nspace;
            rv = gfm_fpsCounterUpdateBegin(pCtx);
            ASSERT_NR(rv == GFMRV_OK);
            
            // Retrieve every key state (or set keys, if not yet set)
            rv = gfm_getKeyState(&kup, &nup, pCtx, up);
            ASSERT_NR(rv == GFMRV_OK);
            rv = gfm_getKeyState(&kdown, &ndown, pCtx, down);
            ASSERT_NR(rv == GFMRV_OK);
            rv = gfm_getKeyState(&kspace, &nspace, pCtx, space);
            ASSERT_NR(rv == GFMRV_OK);
            
            // Finish the animation if space was pressed
            if ((kspace & gfmInput_justPressed) == gfmInput_justPressed) {
                rv = gfmText_forceFinish(pText);
                ASSERT_NR(rv == GFMRV_OK);
            }
            
            // If the text finished, allow scrolling
            rv = gfmText_didFinish(pText);
            if (rv == GFMRV_TRUE) {
                if ((kup & gfmInput_justReleased) == gfmInput_justReleased) {
                    // Try to move one line up
                    rv = gfmText_moveLineUp(pText);
                    ASSERT_NR(rv == GFMRV_OK || rv == GFMRV_TEXT_NO_MORE_LINES);
                }
                if ((kdown & gfmInput_justReleased) == gfmInput_justReleased) {
                    // Try to move one line down
                    rv = gfmText_moveLineDown(pText);
                    ASSERT_NR(rv == GFMRV_OK || rv == GFMRV_TEXT_NO_MORE_LINES);
                }
            }
            
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
            
            // Draw the tilemap
            rv = gfmTilemap_draw(pTmap, pCtx);
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
    gfmTilemap_free(&pTmap);
    gfmText_free(&pText);
    gfm_free(&pCtx);
    
    return rv;
}


