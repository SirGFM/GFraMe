/**
 * @file tst/gframe_mml_tst.c
 * 
 * Simple test with audio
 */
#include <GFraMe/gframe.h>
#include <GFraMe/gfmAssert.h>
#include <GFraMe/gfmError.h>
#include <GFraMe/core/gfmAudio_bkend.h>

// Use a macro to easily make the program sleep in windows and linux
#if defined(WIN32)
#  include <windows.h>
#  define USLEEP(n) ;//Sleep(n)
#else
#  include <unistd.h>
#  define USLEEP(n) usleep(n*100)
#endif

int main(int arg, char *argv[]) {
    double volume;
    gfmCtx *pCtx;
    gfmRV rv;
    int handle;
    
    // Initialize every variable
    pCtx = 0;
    
    // Try to get a new context
    rv = gfm_getNew(&pCtx);
    ASSERT_NR(rv == GFMRV_OK);
    rv = gfm_initStatic(pCtx, "com.gfmgamecorner", "gfmMMLTest");
    ASSERT_NR(rv == GFMRV_OK);
    
    // Initialize the audio sub-system
    //rv = gfm_initAudio(pCtx, gfmAudio_lowQuality);
    //rv = gfm_initAudio(pCtx, gfmAudio_medQuality);
    rv = gfm_initAudio(pCtx, gfmAudio_defQuality);
    //rv = gfm_initAudio(pCtx, gfmAudio_highQuality);
    ASSERT_NR(rv == GFMRV_OK);
    
    // Load an audio
    rv = gfm_loadAudio(&handle, pCtx, "jjat-boss.mml",
            sizeof("jjat-boss.mml") - 1);
    ASSERT_NR(rv == GFMRV_OK);
    
    // Play the audio
    volume = 0.8;
    rv = gfm_playAudio(0/* ignore the instance */, pCtx, handle, volume);
    ASSERT_NR(rv == GFMRV_OK);
    
    // Wait for some time while the song should be playing...
#if defined(WIN32)
    Sleep(30);
#else
    sleep(30);
#endif
    
    rv = GFMRV_OK;
__ret:
    gfm_free(&pCtx);
    
    return rv;
}


