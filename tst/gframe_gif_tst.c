/**
 * @file tst/gframe_gif_tst.c
 * 
 * Simple test to check that gif exporting is working
 */
#include <GFraMe/gframe.h>
#include <GFraMe/gfmAssert.h>
#include <GFraMe/gfmError.h>
#include <GFraMe/gfmSprite.h>
#include <GFraMe/gfmSpriteset.h>
#include <GFraMe/gfmString.h>
#include <GFraMe/core/gfmGifExporter_bkend.h>

#if 0
// Set the game's FPS
#define FPS       60

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
int pTmAnims[] = {
/* num|fps|loop|frames... */
    // Electric thing loop
    4 , 16,  1 , 8,10,12,14,
    4 , 16,  1 , 9,11,13,15,
    // Monitor transition from rest to flicker (long)
    2 , 4 ,  0 ,16,18,
    2 , 4 ,  0 ,17,19,
    2 , 4 ,  0 ,24,26,
    2 , 4 ,  0 ,25,27,
    // Monitor flickering and going to rest (short and quick)
    3 , 12,  0 ,18,20,16,
    3 , 12,  0 ,19,21,17,
    3 , 12,  0 ,26,28,24,
    3 , 12,  0 ,27,29,25,
};
#endif

int main(int arg, char *argv[]) {
    gfmCtx *pCtx;
    gfmGifExporter *pGif;
    gfmRV rv;
    gfmSprite *pSpr;
    //gfmSpriteset *pSset8, *pSset16;
    gfmString *pPath;
    //int iTex, anim;
    
    // Icon from a previous game
            /* RR , GG , BB */
    #define x 0x00,0x00,0x00,
    #define w 0x3f,0x3f,0x74,
    #define m 0x5b,0x6e,0xe1,
    #define n 0x63,0x9b,0xff,
    #define o 0x22,0x20,0x34,
    #define e 0x5f,0xcd,0xe5,
    #define s 0xcb,0xdb,0xfc,
    #define i 0xff,0xff,0xff,
    #define z 0x30,0x60,0x82,
	unsigned char pData[16*16*3] = 
{
x w w w w w w w w w w w w w w x
w n n n n n n n n n n n n n n w
w m o o o o o o o o o o o o n w
w m o o o o o e e e e o o o n w
w m o o o o e m n m m e o o n w
w m o e e e m e m n m n e o n w
w m e m n e n m s n n m e o n w
w m e w m e m s w s w n e o n w
w m o e s w s i z i z s m e n w
w m o e w i w i i i i w e o n w
w m o e z i z w w w w e o o n w
w m o e i i i i i w e o o o n w
w m o e w w w w w e o o o o n w
w m o o e e e e e o o o o o n w
w m m m m m m m m m m m m m m w
x w w w w w w w w w w w w w w x
};
    
    // Initialize every variable
    pCtx = 0;
    pGif = 0;
    pSpr = 0;
    pPath = 0;
    
    // Create the image's name
    rv = gfmString_getNew(&pPath);
    ASSERT_NR(rv == GFMRV_OK);
    rv = gfmString_initStatic(pPath, "image.gif", 0);
    ASSERT_NR(rv == GFMRV_OK);
    
    // Try to get a new context
    rv = gfm_getNew(&pCtx);
    ASSERT_NR(rv == GFMRV_OK);
    rv = gfm_initStatic(pCtx, "com.gfmgamecorner", "gframe_gif");
    ASSERT_NR(rv == GFMRV_OK);
    
    // Initialize the gif exporter
    rv = gfmGif_getNew(&pGif);
    ASSERT_NR(rv == GFMRV_OK);
    rv = gfmGif_init(pGif, pCtx, 16/*width*/, 16/*height*/);
    ASSERT_NR(rv == GFMRV_OK);
    
    // Store some frames
    rv = gfmGif_storeFrame(pGif, (unsigned char*)pData, sizeof(pData));
    ASSERT_NR(rv == GFMRV_OK);
    
    // Export the stored frame to a GIF image
    rv = gfmGif_exportImage(pGif, pPath);
    ASSERT_NR(rv == GFMRV_OK);
    
#if 0
    // Initialize the window
    rv = gfm_initGameWindow(pCtx, 160, 120, 640, 480, 0);
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
    rv = gfm_createSpritesetCached(&pSset16, pCtx, iTex, 16/*tw*/, 16/*th*/);
    ASSERT_NR(rv == GFMRV_OK);
    
    // Initalize the FPS counter
    rv = gfm_initFPSCounter(pCtx, pSset8, 64/*firstTile*/);
    
    // Create the sprite
    rv = gfmSprite_getNew(&pSpr);
    ASSERT_NR(rv == GFMRV_OK);
    rv = gfmSprite_init(pSpr, 16/*x*/, 16/*y*/, 6/*width*/, 12/*height*/,
            pSset16, -4/*offX*/, -4/*offY*/, 0/*pChild*/, 0/*type*/);
    ASSERT_NR(rv == GFMRV_OK);
    
    // Add the animations
    rv = gfmSprite_addAnimationsStatic(pSpr, pSprAnims);
    ASSERT_NR(rv == GFMRV_OK);
    
    // Play an animation
    anim = ANIM_STAND;
    rv = gfmSprite_playAnimation(pSpr, anim);
    ASSERT_NR(rv == GFMRV_OK);
    
    // Set the main loop framerate
    rv = gfm_setStateFrameRate(pCtx, FPS, FPS);
    ASSERT_NR(rv == GFMRV_OK);
    // Initialize the timer
    rv = gfm_setFPS(pCtx, FPS);
    ASSERT_NR(rv == GFMRV_OK);
    
    // Run until the window is closed
    while (gfm_didGetQuitFlag(pCtx) == GFMRV_FALSE) {
        int frames;
        
        rv = gfm_handleEvents(pCtx);
        ASSERT_NR(rv == GFMRV_OK);
        
        // Update stuff
        rv = gfm_getUpdates(&frames, pCtx);
        ASSERT_NR(rv == GFMRV_OK);
        while (frames > 0) {
            rv = gfm_fpsCounterUpdateBegin(pCtx);
            ASSERT_NR(rv == GFMRV_OK);
            
            rv = gfmSprite_update(pSpr, pCtx);
            ASSERT_NR(rv == GFMRV_OK);
            
            rv = gfmSprite_didAnimationJustLoop(pSpr);
            if (rv == GFMRV_TRUE) {
                anim++;
                
                // Loop the animations
                if (anim >= ANIM_MAX) {
                    anim = 0;
                }
                
                // Play the next animation
                rv = gfmSprite_playAnimation(pSpr, anim);
                ASSERT_NR(rv == GFMRV_OK);
            }
            
            rv = gfm_fpsCounterUpdateEnd(pCtx);
            ASSERT_NR(rv == GFMRV_OK);
            
            frames--;
        }
        
        // Draw stuff
        rv = gfm_getDraws(&frames, pCtx);
        ASSERT_NR(rv == GFMRV_OK);
        while (frames > 0) {
            //rv = gfm_snapshotStatic(pCtx, "bla.gif", 0);
            //ASSERT_NR(rv == GFMRV_OK);
            
            rv = gfm_drawBegin(pCtx);
            ASSERT_NR(rv == GFMRV_OK);
            
            rv = gfmSprite_draw(pSpr, pCtx);
            ASSERT_NR(rv == GFMRV_OK);
            
            rv = gfm_drawEnd(pCtx);
            ASSERT_NR(rv == GFMRV_OK);
            
            frames--;
        }
    }
#endif
    
    rv = GFMRV_OK;
__ret:
    gfmString_free(&pPath);
    gfmGif_free(&pGif);
    gfmSprite_free(&pSpr);
    gfm_free(&pCtx);
    
    return rv;
}


