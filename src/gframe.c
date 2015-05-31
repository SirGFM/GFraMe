/**
 * @file src/gframe.c
 * 
 * The game's main context
 */
#include <GFraMe/gframe.h>
#include <GFraMe/gfmAccumulator.h>
#include <GFraMe/gfmAssert.h>
#include <GFraMe/gfmCamera.h>
#include <GFraMe/gfmError.h>
#include <GFraMe/gfmGenericArray.h>
#include <GFraMe/gfmSprite.h>
#include <GFraMe/gfmSpriteset.h>
#include <GFraMe/gfmString.h>
#include <GFraMe/core/gfmBackbuffer_bkend.h>
#include <GFraMe/core/gfmBackend_bkend.h>
#include <GFraMe/core/gfmEvent_bkend.h>
#include <GFraMe/core/gfmGifExporter_bkend.h>
#include <GFraMe/core/gfmTexture_bkend.h>
#include <GFraMe/core/gfmTimer_bkend.h>
#include <GFraMe/core/gfmPath_bkend.h>
#include <GFraMe/core/gfmWindow_bkend.h>
#include <GFraMe_int/gfmFPSCounter.h>

#include <stdlib.h>
#include <string.h>

/** Define a texture array type */
gfmGenArr_define(gfmTexture);
/** Define a spriteset array type */
gfmGenArr_define(gfmSpriteset);

struct stGFMCtx {
    // TODO specify backend(?)
    /** "Organization" name; It's used as part of paths. */
    gfmString *pGameOrg;
    /** Game's title; It's used as part of paths. */
    gfmString *pGameTitle;
#ifndef GFRAME_MOBILE
    /** Directory where the game binary is being run from */
    gfmString *pBinPath;
    /** Length until the current directory (i.e., position to append stuff) */
    int binPathLen;
#endif
    /** Buffer for storing a save file's filename */
    gfmString *pSaveFilename;
    /** Length until the end of the save file's directory (i.e., position to
      append stuff) */
    int saveFilenameLen;
    /** Whether the backend was initialized */
    int isBackendInit;
    /** The game's backbuffer */
    gfmBackbuffer *pBackbuffer;
    /** Game's window */
    gfmWindow *pWindow;
    /** Timer used to issue new frames */
    gfmTimer *pTimer;
    /** Default camera */
    gfmCamera *pCamera;
    /** Every cached texture */
    gfmGenArr_var(gfmTexture, pTextures);
    /** Every cached spriteset */
    gfmGenArr_var(gfmSpriteset, pSpritesets);
    /** Texture that should be loaded on every gfm_drawBegin */
    int defaultTexture;
    /** Accumulate when new update frames should be issued */
    gfmAccumulator *pUpdateAcc;
    /** Accumulate when new draw frames should be issued */
    gfmAccumulator *pDrawAcc;
    /** Event context */
    gfmEvent *pEvent;
    /** Whether a quit event was received */
    gfmRV doQuit;
    /** The GIF exporter */
    gfmGifExporter *pGif;
    /** Whether a snapshot should be taken */
    int takeSnapshot;
    /** Whether is recording an animation or a single snapshot */
    int isAnimation;
    /** For how long the animation should be recorded, in milliseconds */
    int animationTime;
    /** Path where the snapshot should be saved */
    gfmString *pSsPath;
    /** Stores the snapshot */
    unsigned char *pSsData;
    /** Number of bytes on the snapshot data */
    int ssDataLen;
#if defined(DEBUG) || defined(FORCE_FPS)
    /** Whether the FPS counter should be displayed */
    int showFPS;
    /** FPS Counter; only enabled on debug version */
    gfmFPSCounter *pCounter;
#endif
};

/** 'Exportable' size of gfmStruct */
const int sizeofGFMCtx = sizeof(struct stGFMCtx);

/**
 * Alloc a new gfmContext
 * 
 * @param  ppCtx The allocated context
 * @return       GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_ALLOC_FAILED
 */
gfmRV gfm_getNew(gfmCtx **ppCtx) {
    gfmRV rv;
    
    // Sanitize the arguments
    ASSERT(ppCtx, GFMRV_ARGUMENTS_BAD);
    ASSERT(!(*ppCtx), GFMRV_ARGUMENTS_BAD);
    
    // Alloc the context
    *ppCtx = (gfmCtx*)malloc(sizeofGFMCtx);
    ASSERT(*ppCtx, GFMRV_ALLOC_FAILED);
    
    // Zero the context's contents
    memset(*ppCtx, 0x00, sizeof(gfmCtx));
    (*ppCtx)->defaultTexture = -1;
    
#ifndef GFRAME_MOBILE
	// Get current directory
    rv = gfmPath_getRunningPath(&((*ppCtx)->pBinPath));
    ASSERT_NR(rv == GFMRV_OK);
    rv = gfmString_getLength(&((*ppCtx)->binPathLen), (*ppCtx)->pBinPath);
    ASSERT_NR(rv == GFMRV_OK);
    
    // TODO check if it gets the directory or if the executable name is appended
#endif
    
    // Init the current backend
    // TODO allow more than one backend?
    rv = gfmBackend_init();
    ASSERT_NR(rv == GFMRV_OK);
    // Set the backend as initialized
    (*ppCtx)->isBackendInit = 1;
    
    // Initialize the event's context
    rv = gfmEvent_getNew(&((*ppCtx)->pEvent));
    ASSERT_NR(rv == GFMRV_OK);
    
    // Initialize the fps counter, if debug
#if defined(DEBUG) || defined(FORCE_FPS)
    rv = gfmFPSCounter_getNew(&((*ppCtx)->pCounter));
    ASSERT_NR(rv == GFMRV_OK);
#endif
    
    // Set the game as running
    (*ppCtx)->doQuit = GFMRV_FALSE;
    
    rv = GFMRV_OK;
__ret:
    // Clean up the context, on error
    if (rv != GFMRV_OK && rv != GFMRV_ARGUMENTS_BAD) {
        free(ppCtx);
    }
    
    return rv;
}

/**
 * Dealloc and clean up a gfmContext
 * 
 * @param  ppCtx The allocated context
 * @return       GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfm_free(gfmCtx **ppCtx) {
    gfmRV rv;
    
    // Sanitize the arguments
    ASSERT(ppCtx, GFMRV_ARGUMENTS_BAD);
    ASSERT(*ppCtx, GFMRV_ARGUMENTS_BAD);
    
    // Clean up the context
    rv = gfm_clean(*ppCtx);
    ASSERT_NR(rv == GFMRV_OK);
    
    // Dealloc the context
    free(*ppCtx);
    *ppCtx = 0;
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Get the binary's running path
 * 
 * @param  ppStr The running path as a gfmString
 * @param  pCtx  The game's context
 * @return       GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfm_getBinaryPath(gfmString **ppStr, gfmCtx *pCtx) {
    gfmRV rv;
    
    // Check that this isn't running on a mobile device
#ifdef GFRAME_MOBILE
    ASSERT(0, GFMRV_FUNCTION_NOT_SUPPORTED);
#else
    // Sanitize arguments
    ASSERT(ppStr, GFMRV_ARGUMENTS_BAD);
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    
    // Remove anything that was concatenated
    rv = gfmString_setLength(pCtx->pBinPath, pCtx->binPathLen);
    ASSERT_NR(rv == GFMRV_OK);
    // Return the string
    *ppStr = pCtx->pBinPath;
#endif
    
    rv = GFMRV_OK;
__ret:
    return rv;
}


/**
 * Set the game's title and organization
 * 
 * @param  pCtx     The game's context
 * @param  pOrg     Organization's name (used by log and save file)
 * @param  orgLen   Organization's name's length
 * @param  pName    Game's title (also used as window's title)
 * @param  nameLen  Game's title's length
 * @return          GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_TITLE_ALREADY_SET,
 *                  GFMRV_ALLOC_FAILED
 */
gfmRV gfm_setTitle(gfmCtx *pCtx, char *pOrg, int orgLen, char *pName,
        int nameLen) {
    gfmRV rv;
    int doCopy;
    
    // Sanitize the arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    ASSERT(pOrg, GFMRV_ARGUMENTS_BAD);
    ASSERT(orgLen > 0, GFMRV_ARGUMENTS_BAD);
    ASSERT(pName, GFMRV_ARGUMENTS_BAD);
    ASSERT(nameLen > 0, GFMRV_ARGUMENTS_BAD);
    
    // Check that the game's title wasn't initialized
    ASSERT(!pCtx->pGameOrg, GFMRV_TITLE_ALREADY_SET);
    ASSERT(!pCtx->pGameTitle, GFMRV_TITLE_ALREADY_SET);
    
    // Alloc both strings
    rv = gfmString_getNew(&(pCtx->pGameOrg));
    ASSERT_NR(rv == GFMRV_OK);
    rv = gfmString_getNew(&(pCtx->pGameTitle));
    ASSERT_NR(rv == GFMRV_OK);
    
    // Initialize the strings
    doCopy = 1;
    rv = gfmString_init(pCtx->pGameOrg, pOrg, orgLen, doCopy);
    ASSERT_NR(rv == GFMRV_OK);
    rv = gfmString_init(pCtx->pGameTitle, pName, nameLen, doCopy);
    ASSERT_NR(rv == GFMRV_OK);
    
    // Get the default file path
    rv = gfmPath_getLocalPath(&(pCtx->pSaveFilename), pCtx);
    ASSERT_NR(rv == GFMRV_OK);
    rv = gfmString_getLength(&(pCtx->saveFilenameLen), pCtx->pSaveFilename);
    ASSERT_NR(rv == GFMRV_OK);
    
    rv = GFMRV_OK;
__ret:
    if (rv != GFMRV_OK && rv != GFMRV_TITLE_ALREADY_SET &&
            rv != GFMRV_ARGUMENTS_BAD) {
        gfmString_free(&(pCtx->pGameOrg));
        gfmString_free(&(pCtx->pGameTitle));
        gfmString_free(&(pCtx->pSaveFilename));
    }
    return rv;
}

/**
 * Get the game's title and organization
 * 
 * @param  ppOrg      Organization's name (used by log and save file)
 * @param  ppTitle    Game's title (also used as window's title)
 * @param  pCtx       The game's context
 * @return            GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_TITLE_NOT_SET
 */
gfmRV gfm_getTitle(char **ppOrg, char **ppTitle, gfmCtx *pCtx) {
    gfmRV rv;
    
    // Sanitize the arguments
    ASSERT(ppOrg, GFMRV_ARGUMENTS_BAD);
    ASSERT(ppTitle, GFMRV_ARGUMENTS_BAD);
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    
    // Check that the string were set
    ASSERT(pCtx->pGameOrg, GFMRV_TITLE_NOT_SET);
    ASSERT(pCtx->pGameTitle, GFMRV_TITLE_NOT_SET);
    
    // Retrieve the strings
    rv = gfmString_getString(ppOrg, pCtx->pGameOrg);
    ASSERT_NR(rv == GFMRV_OK);
    rv = gfmString_getString(ppTitle, pCtx->pGameTitle);
    ASSERT_NR(rv == GFMRV_OK);
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Get the game's local path
 * 
 * @param  ppLocalPath Local path (a new gfmString is alloc'ed!)
 * @param  pCtx        The game's context
 * @return             GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfm_getLocalPath(gfmString **ppLocalPath, gfmCtx *pCtx) {
    gfmRV rv;
    
    // Sanitize the arguments
    ASSERT(ppLocalPath, GFMRV_ARGUMENTS_BAD);
    ASSERT(!(*ppLocalPath), GFMRV_ARGUMENTS_BAD);
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    
    // Get the path
    rv = gfmPath_getLocalPath(ppLocalPath, pCtx);
    ASSERT_NR(rv == GFMRV_OK);
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Query the resolutions and add them to a internal buffer
 * 
 * @param  pCount How many resolutions were found
 * @param  pCtx   The game's context
 * @return        GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_INTERNAL_ERROR,
 *                GFMRV_ALLOC_FAILED
 */
gfmRV gfm_queryResolutions(int *pCount, gfmCtx *pCtx) {
    gfmRV rv;
    
    // Sanitize arguments
    ASSERT(pCount, GFMRV_ARGUMENTS_BAD);
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    
    // Alloc the window
    if (!(pCtx->pWindow)) {
        rv = gfmWindow_getNew(&(pCtx->pWindow));
        ASSERT_NR(rv == GFMRV_OK);
    }
    // Query the resolutions and set 'pCount'
    rv = gfmWindow_queryResolutions(pCount, pCtx->pWindow);
    ASSERT_NR(rv == GFMRV_OK);
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Get a resolution; if gfmWindow_queryResolutions wasn't previously called, it
 * will be automatically called
 * 
 * @param  pWidth   A possible window's width
 * @param  pHeight  A possible window's height
 * @param  pRefRate A possible window's refresh rate
 * @param  pCtx     The game's context
 * @param  index    Resolution to be read (0 is the default resolution)
 * @return          GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_INTERNAL_ERROR,
 *                  GFMRV_ALLOC_FAILED, GFMRV_INVALID_INDEX
 */
gfmRV gfm_getResolution(int *pWidth, int *pHeight, int *pRefRate,
        gfmCtx *pCtx, int index) {
    gfmRV rv;
    
    // Sanitize arguments
    ASSERT(pWidth, GFMRV_ARGUMENTS_BAD);
    ASSERT(pHeight, GFMRV_ARGUMENTS_BAD);
    ASSERT(pRefRate, GFMRV_ARGUMENTS_BAD);
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    
    // Get the desired resolution
    rv = gfmWindow_getResolution(pWidth, pHeight, pRefRate, pCtx->pWindow,
            index);
    ASSERT_NR(rv == GFMRV_OK);
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Initialize the game's window and backbuffer
 * 
 * *NOTE*: The game window may be later resized, but not the backbuffer!
 * 
 * @param  pCtx            The game's context
 * @param  bufWidth        Backbuffer's width
 * @param  bufHeight       Backbuffer's height
 * @param  wndWidth        Window's width
 * @param  wndHeight       Window's height
 * @param  isUserResizable Whether the user can resize the window through the OS
 * @return                 GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_TITLE_NOT_SET,
 *                         GFMRV_INVALID_WIDTH, GFMRV_INVALID_HEIGHT
 */
gfmRV gfm_initGameWindow(gfmCtx *pCtx, int bufWidth, int bufHeight,
        int wndWidth, int wndHeight, int isUserResizable) {
    char *pTitle, *pOrg;
    gfmRV rv;
    
    // Sanitize the arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    // Check that the window hasn't been initialized
    ASSERT(!(pCtx->pWindow) || gfmWindow_wasInit(pCtx->pWindow) == GFMRV_FALSE,
            GFMRV_WINDOW_ALREADY_INITIALIZED);
    // Check that the backbuffer hasn't been initialized
    ASSERT(!pCtx->pBackbuffer, GFMRV_BACKBUFFER_ALREADY_INITIALIZED);
    // Basic check for the resolution (it'll be later re-done, on window_init)
    ASSERT(wndWidth > 0, GFMRV_INVALID_WIDTH);
    ASSERT(wndHeight > 0, GFMRV_INVALID_HEIGHT);
    
    // Try to read the game's title
    pOrg = 0;
    pTitle = 0;
    rv = gfm_getTitle(&pOrg, &pTitle, pCtx);
    ASSERT_NR(rv == GFMRV_OK);
    
    // Alloc and initialize the window
    if (!(pCtx->pWindow)) {
        rv = gfmWindow_getNew(&(pCtx->pWindow));
        ASSERT_NR(rv == GFMRV_OK);
    }
    rv = gfmWindow_init(pCtx->pWindow, wndWidth, wndHeight, pTitle,
            isUserResizable);
    ASSERT_NR(rv == GFMRV_OK);
    
    // Alloc and initialize the backbuffer
    rv = gfmBackbuffer_getNew(&(pCtx->pBackbuffer));
    ASSERT_NR(rv == GFMRV_OK);
    rv = gfmBackbuffer_init(pCtx->pBackbuffer, pCtx->pWindow, bufWidth,
            bufHeight);
    ASSERT_NR(rv == GFMRV_OK);
    
    // Alloc and initialize the camera
    rv = gfmCamera_getNew(&(pCtx->pCamera));
    ASSERT_NR(rv == GFMRV_OK);
    rv = gfmCamera_init(pCtx->pCamera, pCtx, bufWidth, bufHeight);
    ASSERT_NR(rv == GFMRV_OK);
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Initialize the game's window (in fullscreen) and backbuffer
 * 
 * *NOTE*: The game window may be later resized, but not the backbuffer!
 * 
 * @param  pCtx            The game's context
 * @param  bufWidth        Backbuffer's width
 * @param  bufHeight       Backbuffer's height
 * @param  resIndex        Resolution to be used (0 is the default resolution)
 * @param  isUserResizable Whether the user can resize the window through the OS
 * @return                 GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_TITLE_NOT_SET,
 *                         GFMRV_INVALID_WIDTH, GFMRV_INVALID_HEIGHT
 */
gfmRV gfm_initGameFullScreen(gfmCtx *pCtx, int bufWidth, int bufHeight,
        int resIndex, int isUserResizable) {
    char *pTitle, *pOrg;
    gfmRV rv;
    
    // Sanitize the arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    ASSERT(resIndex >= 0, GFMRV_ARGUMENTS_BAD);
    // Check that the window hasn't been initialized
    ASSERT(!(pCtx->pWindow) || gfmWindow_wasInit(pCtx->pWindow) == GFMRV_FALSE,
            GFMRV_WINDOW_ALREADY_INITIALIZED);
    
    // Try to read the game's title
    pOrg = 0;
    pTitle = 0;
    rv = gfm_getTitle(&pOrg, &pTitle, pCtx);
    ASSERT_NR(rv == GFMRV_OK);
    
    // Alloc and initialize the window
    if (!(pCtx->pWindow)) {
        rv = gfmWindow_getNew(&(pCtx->pWindow));
        ASSERT_NR(rv == GFMRV_OK);
    }
    rv = gfmWindow_initFullScreen(pCtx->pWindow, resIndex, pTitle,
            isUserResizable);
    ASSERT_NR(rv == GFMRV_OK);
    
    // Alloc and initialize the backbuffer
    rv = gfmBackbuffer_getNew(&(pCtx->pBackbuffer));
    ASSERT_NR(rv == GFMRV_OK);
    rv = gfmBackbuffer_init(pCtx->pBackbuffer, pCtx->pWindow, bufWidth,
            bufHeight);
    ASSERT_NR(rv == GFMRV_OK);
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Set the game's fps resolution; This defines when will the game automatically
 * wake to update its timers and check if a new frame should be issued
 * (therefore, it's somewhat different from the stateFramerate)
 * 
 * This can be used to ease the game's resource (CPU) consuption, when focus is
 * lost
 * 
 * NOTE: This function will round the time to its nearest multiple of ten
 * 
 * @param  pCtx The game's context
 * @param  fps  The game's fps
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_ALLOC_FAILED,
 *              GFMRV_INTERNAL_ERROR, GFMRV_FPS_TOO_HIGH
 */
gfmRV gfm_setFPS(gfmCtx *pCtx, int fps) {
    gfmRV rv;
    
    // Sanitize arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    ASSERT(fps > 0, GFMRV_ARGUMENTS_BAD);
    
    if (!pCtx->pTimer) {
        // Create a new timer, if necessary
        rv = gfmTimer_getNew(&(pCtx->pTimer), pCtx);
        ASSERT_NR(rv == GFMRV_OK);
        // Initialize the timer
        rv = gfmTimer_init(pCtx->pTimer, fps);
        ASSERT_NR(rv == GFMRV_OK);
    }
    else {
        // Only modify the timer
        rv = gfmTimer_setFPS(pCtx->pTimer, fps);
        ASSERT_NR(rv == GFMRV_OK);
    }
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Set the game's fps resolution; This defines when will the game automatically
 * wake to update its timers and check if a new frame should be issued
 * (therefore, it's somewhat different from the stateFramerate)
 * 
 * This can be used to ease the game's resource (CPU) consuption, when focus is
 * lost
 * 
 * @param  pCtx The game's context
 * @param  fps  The game's fps
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_ALLOC_FAILED,
 *              GFMRV_INTERNAL_ERROR, GFMRV_FPS_TOO_HIGH
 */
gfmRV gfm_setRawFPS(gfmCtx *pCtx, int fps) {
    gfmRV rv;
    
    // Sanitize arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    ASSERT(fps > 0, GFMRV_ARGUMENTS_BAD);
    
    if (!pCtx->pTimer) {
        // Create a new timer, if necessary
        rv = gfmTimer_getNew(&(pCtx->pTimer), pCtx);
        ASSERT_NR(rv == GFMRV_OK);
        // Initialize the timer
        rv = gfmTimer_initRaw(pCtx->pTimer, fps);
        ASSERT_NR(rv == GFMRV_OK);
    }
    else {
        // Only modify the timer
        rv = gfmTimer_setFPSRaw(pCtx->pTimer, fps);
        ASSERT_NR(rv == GFMRV_OK);
    }
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Signal the game's context that it should quit
 * 
 * @param  pCtx The game's context
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfm_setQuitFlag(gfmCtx *pCtx) {
    gfmRV rv;
    
    // Sanitize arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    
    // Set the flag
    pCtx->doQuit = GFMRV_TRUE;
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Check whether the quit flag was received or not
 * 
 * @param  pCtx The game's context
 * @return      GFMRV_ARGUMENTS_BAD, GFMRV_FALSE, GFMRV_TRUE
 */
gfmRV gfm_didGetQuitFlag(gfmCtx *pCtx) {
    gfmRV rv;
    
    // Sanitize arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    
    // Return the flag
    rv = pCtx->doQuit;
__ret:
    return rv;
}

/**
 * Get the event context
 * 
 * @param ppEvent The event context
 * @param pCtx    The game's context
 * @return        GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfm_getEventCtx(gfmEvent **ppEvent, gfmCtx *pCtx) {
    gfmRV rv;
    
    // Sanitize arguments
    ASSERT(ppEvent, GFMRV_ARGUMENTS_BAD);
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    // The event context is initialize with the game (no extra assert needed)
    
    // Return the event context
    *ppEvent = pCtx->pEvent;
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Get the current backbuffer
 * 
 * @param  ppBbuf The backbuffer
 * @param  pCtx   The game's context
 * @return        GFMRV_OK, GFMRV_ARGUMENTS_BAD,
 *                GFMRV_BACKBUFFER_NOT_INITIALIZED
 */
gfmRV gfm_getBackbuffer(gfmBackbuffer **ppBbuf, gfmCtx *pCtx) {
    gfmRV rv;
    
    // Sanitize arguments
    ASSERT(ppBbuf, GFMRV_ARGUMENTS_BAD);
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    // Check that the renderer was initialized
    ASSERT(pCtx->pBackbuffer, GFMRV_BACKBUFFER_NOT_INITIALIZED);
    
    *ppBbuf = pCtx->pBackbuffer;
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Get the backbuffer's dimension
 * 
 * @param  pWidth  The backbuffer's width
 * @param  pHeigth The backbuffer's height
 * @param  pCtx    The game's contex
 * @return         GFMRV_OK, GFMRV_ARGUMENTS_BAD,
 *                 GFMRV_BACKBUFFER_NOT_INITIALIZED
 */
gfmRV gfm_getBackbufferDimensions(int *pWidth, int *pHeight, gfmCtx *pCtx) {
    gfmRV rv;
    
    // Sanitize arguments
    ASSERT(pWidth, GFMRV_ARGUMENTS_BAD);
    ASSERT(pHeight, GFMRV_ARGUMENTS_BAD);
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    // Check that the renderer was initialized
    ASSERT(pCtx->pBackbuffer, GFMRV_BACKBUFFER_NOT_INITIALIZED);
    
    // Get the dimenensions
    rv = gfmBackbuffer_getDimensions(pWidth, pHeight, pCtx->pBackbuffer);
    ASSERT_NR(rv == GFMRV_OK);
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Resize the window to the desired dimensions
 * 
 * @param  pCtx   The window context
 * @param  width  The desired width
 * @param  height The desired height
 * @return        GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_INTERNAL_ERROR,
 *                GFMRV_WINDOW_NOT_INITIALIZED, GFMRV_WINDOW_IS_FULLSCREEN,
 *                GFMRV_BACKBUFFER_NOT_INITIALIZED,
 *                GFMRV_BACKBUFFER_WINDOW_TOO_SMALL
 */
gfmRV gfm_setDimensions(gfmCtx *pCtx, int width, int height) {
    gfmRV rv;
    
    // Sanitize the arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    // Check that the window has already been initialized
    ASSERT(pCtx->pWindow && gfmWindow_wasInit(pCtx->pWindow) == GFMRV_TRUE,
            GFMRV_WINDOW_NOT_INITIALIZED);
    // Check that the backbuffer was initialized
    ASSERT(pCtx->pBackbuffer, GFMRV_BACKBUFFER_NOT_INITIALIZED);
    
    // Cache the backbuffer's output dimensions
    rv = gfmBackbuffer_cacheDimensions(pCtx->pBackbuffer, width, height);
    ASSERT_NR(rv == GFMRV_OK);
    // Set the window's dimentions
    rv = gfmWindow_setDimensions(pCtx->pWindow, width, height);
    ASSERT_NR(rv == GFMRV_OK);
    
    rv = GFMRV_OK;
__ret:
    return rv;
}


/**
 * Make the game go full-screen
 * 
 * @param  pCtx   The game's context
 * @return        GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_INTERNAL_ERROR,
 *                GFMRV_WINDOW_NOT_INITIALIZED, GFMRV_WINDOW_MODE_UNCHANGED
 */
gfmRV gfm_setFullscreen(gfmCtx *pCtx) {
    gfmRV rv;
    int wndWidth, wndHeight;
    
    // Sanitize the arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    // Check that the window has already been initialized
    ASSERT(pCtx->pWindow && gfmWindow_wasInit(pCtx->pWindow) == GFMRV_TRUE,
            GFMRV_WINDOW_NOT_INITIALIZED);
    
    // Try to make it fullscreen
    rv = gfmWindow_setFullScreen(pCtx->pWindow);
    ASSERT_NR(rv == GFMRV_OK);
    // Cache the backbuffer's output dimensions
    rv = gfmWindow_getDimensions(&wndWidth, &wndHeight, pCtx->pWindow);
    ASSERT_NR(rv == GFMRV_OK);
    rv = gfmBackbuffer_cacheDimensions(pCtx->pBackbuffer, wndWidth, wndHeight);
    ASSERT_NR(rv == GFMRV_OK);
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Make the game go windowed;
 * The window's dimensions will be kept!
 * 
 * @param  pCtx   The game's context
 * @return        GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_INTERNAL_ERROR,
 *                GFMRV_WINDOW_NOT_INITIALIZED, GFMRV_WINDOW_MODE_UNCHANGED
 */
gfmRV gfm_setWindowed(gfmCtx *pCtx) {
    gfmRV rv;
    int wndWidth, wndHeight;
    
    // Sanitize the arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    // Check that the window has already been initialized
    ASSERT(pCtx->pWindow && gfmWindow_wasInit(pCtx->pWindow) == GFMRV_TRUE,
            GFMRV_WINDOW_NOT_INITIALIZED);
    
    // Try to make it fullscreen
    rv = gfmWindow_setWindowed(pCtx->pWindow);
    ASSERT_NR(rv == GFMRV_OK);
    
    // Cache the backbuffer's output dimensions
    rv = gfmWindow_getDimensions(&wndWidth, &wndHeight, pCtx->pWindow);
    ASSERT_NR(rv == GFMRV_OK);
    rv = gfmBackbuffer_cacheDimensions(pCtx->pBackbuffer, wndWidth, wndHeight);
    ASSERT_NR(rv == GFMRV_OK);
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Set the window's resolution;
 * If the window is in full screen mode, it's resolution and refresh rate will
 * be modified; Otherwise, only it's dimension's will be modified
 * 
 * @param  pCtx  The game's context
 * @param  index Resolution to be used (0 is the default resolution)
 * @return       GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_INTERNAL_ERROR,
 *               GFMRV_INVALID_INDEX
 */
gfmRV gfm_setResolution(gfmCtx *pCtx, int resIndex) {
    gfmRV rv;
    int wndWidth, wndHeight;
    
    // Sanitize the arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    // Check that the window has already been initialized
    ASSERT(pCtx->pWindow && gfmWindow_wasInit(pCtx->pWindow) == GFMRV_TRUE,
            GFMRV_WINDOW_NOT_INITIALIZED);
    
    // Set the window's resolution
    rv = gfmWindow_setResolution(pCtx->pWindow, resIndex);
    ASSERT_NR(rv == GFMRV_OK);
    
    // Cache the backbuffer's output dimensions
    rv = gfmWindow_getDimensions(&wndWidth, &wndHeight, pCtx->pWindow);
    ASSERT_NR(rv == GFMRV_OK);
    rv = gfmBackbuffer_cacheDimensions(pCtx->pBackbuffer, wndWidth, wndHeight);
    ASSERT_NR(rv == GFMRV_OK);
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

gfmRV gfm_initAll() {
    return GFMRV_FUNCTION_NOT_SUPPORTED;
}

/**
 * Set the background color
 * 
 * @param  pCtx  The game's context
 * @param  color The background color (in ARGB, 32 bits, format)
 * @return       GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_BACKBUFFER_NOT_INITIALIZED
 */
gfmRV gfm_setBackground(gfmCtx *pCtx, int color) {
    gfmRV rv;
    
    // Sanitize arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    // Check that the backbuffer was initialized
    ASSERT(pCtx->pBackbuffer, GFMRV_BACKBUFFER_NOT_INITIALIZED);
    
    // Set the bg color
    rv = gfmBackbuffer_setBackground(pCtx->pBackbuffer, color);
    ASSERT_NR(rv == GFMRV_OK);
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Create and load a texture; the lib will keep track of it and release its
 * memory, on exit
 * 
 * @param  pIndex      The texture's index
 * @param  pCtx        The game's contex
 * @param  pFilename   The image's filename (must be a '.bmp')
 * @param  filenameLen The filename's length
 * @param  colorKey    Color to be treat as transparent (in RGB, 24 bits)
 * @return             GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_TEXTURE_NOT_BITMAP,
 *                     GFMRV_TEXTURE_FILE_NOT_FOUND,
 *                     GFMRV_TEXTURE_INVALID_WIDTH,
 *                     GFMRV_TEXTURE_INVALID_HEIGHT, GFMRV_ALLOC_FAILED,
 *                     GFMRV_INTERNAL_ERROR
 */
gfmRV gfm_loadTexture(int *pIndex, gfmCtx *pCtx, char *pFilename,
        int filenameLen, int colorKey) {
    gfmRV rv;
    gfmTexture *pTex;
    int incRate;
    
    // Sanitize arguments
    ASSERT(pIndex, GFMRV_ARGUMENTS_BAD);
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    ASSERT(pFilename, GFMRV_ARGUMENTS_BAD);
    ASSERT(filenameLen, GFMRV_ARGUMENTS_BAD);
    
    // Try to get a new texture
    incRate = 1;
    // This macro already ASSERT errors
    gfmGenArr_getNextRef(gfmTexture, pCtx->pTextures, incRate, pTex, gfmTexture_getNew);
    
    // Load the texture
    rv = gfmTexture_load(pTex, pCtx, pFilename, filenameLen, colorKey);
    ASSERT_NR(rv == GFMRV_OK);
    
    // Get the texture's index
    *pIndex = gfmGenArr_getUsed(pCtx->pTextures);
    // Push the texture into the array
    gfmGenArr_push(pCtx->pTextures);
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Get a texture
 * 
 * @param  ppTex The texture
 * @param  pCtx  The game's contex
 * @param  index The texture's index
 * @return       GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_INVALID_INDEX
 */
gfmRV gfm_getTexture(gfmTexture **ppTex, gfmCtx *pCtx, int index) {
    gfmRV rv;
    
    // Sanitize arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    ASSERT(ppTex, GFMRV_ARGUMENTS_BAD);
    ASSERT(index >= 0, GFMRV_ARGUMENTS_BAD);
    // Check that the texture exists
    ASSERT(index < gfmGenArr_getUsed(pCtx->pTextures), GFMRV_INVALID_INDEX);
    
    // Return the texture
    *ppTex = gfmGenArr_getObject(pCtx->pTextures, index);
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Create a new (automatically managed) spriteset
 * 
 * @param  ppSset     The spriteset
 * @param  pCtx       The game's context
 * @param  pTex       The texture
 * @param  tileWidth  The width of each tile
 * @param  tileHeight The height of each tile
 * @return            GFMRV_OK, GFMRV_ARGUMENTS_BAD,
 *                    GFMRV_SPRITESET_INVALID_WIDTH,
 *                    GFMRV_SPRITESET_INVALID_HEIGHT,
 *                    GFMRV_TEXTURE_NOT_INITIALIZED
 */
gfmRV gfm_createSpriteset(gfmSpriteset **ppSset, gfmCtx *pCtx, gfmTexture *pTex,
        int tileWidth, int tileHeight) {
    gfmRV rv;
    gfmSpriteset *pSset;
    int incRate;
    
    // Sanitize only the context (as the rest is checked on the inner function)
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    ASSERT(ppSset, GFMRV_ARGUMENTS_BAD);
    
    // Initialize so it can ben cleaned on error
    pSset = 0;
    
    // Try to get a new spriteset
    incRate = 1;
    // This macro already ASSERT errors
    gfmGenArr_getNextRef(gfmSpriteset, pCtx->pSpritesets, incRate, pSset,
            gfmSpriteset_getNew);
    
    // Initialize it
    rv = gfmSpriteset_init(pSset, pTex, tileWidth, tileHeight);
    ASSERT_NR(rv == GFMRV_OK);
    
    // Push the spriteset into the array
    gfmGenArr_push(pCtx->pSpritesets);
    
    // Set the return
    *ppSset = pSset;
    rv = GFMRV_OK;
__ret:
    if (rv != GFMRV_OK && rv != GFMRV_ARGUMENTS_BAD) {
        gfmSpriteset_free(&pSset);
    }
    
    return rv;
}

/**
 * Create a new (automatically managed) spriteset
 * 
 * @param  ppSset     The spriteset
 * @param  pCtx       The game's context
 * @param  index      The texture's index
 * @param  tileWidth  The width of each tile
 * @param  tileHeight The height of each tile
 * @return            GFMRV_OK, GFMRV_ARGUMENTS_BAD,
 *                    GFMRV_SPRITESET_INVALID_WIDTH,
 *                    GFMRV_SPRITESET_INVALID_HEIGHT,
 *                    GFMRV_TEXTURE_NOT_INITIALIZED
 */
gfmRV gfm_createSpritesetCached(gfmSpriteset **ppSset, gfmCtx *pCtx, int index,
        int tileWidth, int tileHeight) {
    gfmRV rv;
    gfmSpriteset *pSset;
    int incRate;
    
    // Sanitize only the context (as the rest is checked on the inner function)
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    ASSERT(ppSset, GFMRV_ARGUMENTS_BAD);
    
    // Initialize so it can ben cleaned on error
    pSset = 0;
    
    // Try to get a new spriteset
    incRate = 1;
    // This macro already ASSERT errors
    gfmGenArr_getNextRef(gfmSpriteset, pCtx->pSpritesets, incRate, pSset,
            gfmSpriteset_getNew);
    // Initialize it
    rv = gfmSpriteset_initCached(pSset, pCtx, index, tileWidth, tileHeight);
    ASSERT_NR(rv == GFMRV_OK);
    
    // Push the spriteset into the array
    gfmGenArr_push(pCtx->pSpritesets);
    
    // Set the return
    *ppSset = pSset;
    rv = GFMRV_OK;
__ret:
    if (rv != GFMRV_OK && rv != GFMRV_ARGUMENTS_BAD) {
        gfmSpriteset_free(&pSset);
    }
    
    return rv;
}

/**
 * Set a texture as default; this texture will always be loaded before drawing
 * anything
 * 
 * @param  pCtx  The game's context
 * @param  index The texture index
 * @return       GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_INVALID_INDEX
 */
gfmRV gfm_setDefaultTexture(gfmCtx *pCtx, int index) {
    gfmRV rv;
    
    // Sanitize arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    ASSERT(index >= 0, GFMRV_ARGUMENTS_BAD);
    // Check that the texture exists
    ASSERT(index < gfmGenArr_getUsed(pCtx->pTextures), GFMRV_INVALID_INDEX);
    
    // Cache the default texture
    pCtx->defaultTexture = index;
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Get the default camera's current position
 * 
 * @param  pX    The horizontal position
 * @param  pY    The vertical position
 * @param  pCtx  The game's context
 * @return       GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_CAMERA_NOT_INITIALIZED
 */
gfmRV gfm_getCameraPosition(int *pX, int *pY, gfmCtx *pCtx) {
    gfmRV rv;
    
    // Sanitize arguments
    ASSERT(pX, GFMRV_ARGUMENTS_BAD);
    ASSERT(pY, GFMRV_ARGUMENTS_BAD);
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    // Check that the camera was initialized
    ASSERT(pCtx->pCamera, GFMRV_CAMERA_NOT_INITIALIZED);
    
    // Get the position
    rv = gfmCamera_getPosition(pX, pY, pCtx->pCamera);
    ASSERT_NR(rv == GFMRV_OK);
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Get the default camera's current position
 * 
 * @param  pWidth  The camera's width
 * @param  pHeight The camera's height
 * @param  pCtx    The game's context
 * @return         GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_CAMERA_NOT_INITIALIZED
 */
gfmRV gfm_getCameraDimensions(int *pWidth, int *pHeight, gfmCtx *pCtx) {
    gfmRV rv;
    
    // Sanitize arguments
    ASSERT(pWidth, GFMRV_ARGUMENTS_BAD);
    ASSERT(pHeight, GFMRV_ARGUMENTS_BAD);
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    // Check that the camera was initialized
    ASSERT(pCtx->pCamera, GFMRV_CAMERA_NOT_INITIALIZED);
    
    // Get the dimensions
    rv = gfmCamera_getDimensions(pWidth, pHeight, pCtx->pCamera);
    ASSERT_NR(rv == GFMRV_OK);
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Check if a sprite is inside the camera
 * 
 * @param  pCtx The game's context
 * @param  pSpr The sprite
 * @return      GFMRV_TRUE, GFMRV_FALSE, GFMRV_ARGUMENTS_BAD,
 *              GFMRV_CAMERA_NOT_INITIALIZED
 */
gfmRV gfm_isSpriteInsideCamera(gfmCtx *pCtx, gfmSprite *pSpr) {
    gfmRV rv;
    
    // Sanitize arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    ASSERT(pSpr, GFMRV_ARGUMENTS_BAD);
    // Check that the camera was initialized
    ASSERT(pCtx->pCamera, GFMRV_CAMERA_NOT_INITIALIZED);
    
    // Check if it's inside
    rv = gfmCamera_isSpriteInside(pCtx->pCamera, pSpr);
__ret:
    return rv;
}

/**
 * Set the state's framerate
 * 
 * @param  pCtx The game's context
 * @param  ups  Number of updates per seconds
 * @param  dps  Number of draws per seconds
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_ACC_FPS_TOO_HIGH
 */
gfmRV gfm_setStateFrameRate(gfmCtx *pCtx, int ups, int dps) {
    gfmRV rv;
    int maxFrames;
    
    // Sanitize arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    ASSERT(ups > 0, GFMRV_ARGUMENTS_BAD);
    ASSERT(dps > 0, GFMRV_ARGUMENTS_BAD);
    
    // Alloc the objects, if necessary
    if (!pCtx->pUpdateAcc) {
        rv = gfmAccumulator_getNew(&(pCtx->pUpdateAcc));
        ASSERT_NR(rv == GFMRV_OK);
    }
    if (!pCtx->pDrawAcc) {
        rv = gfmAccumulator_getNew(&(pCtx->pDrawAcc));
        ASSERT_NR(rv == GFMRV_OK);
    }
    
    // Set max Frames to avoid crash (and force slow down) on laggy parts
    maxFrames = ups / 10;
    if (maxFrames == 0)
        maxFrames = 1;
    // Initialize both accumulators
    rv = gfmAccumulator_setFPS(pCtx->pUpdateAcc, ups, maxFrames);
    ASSERT_NR(rv == GFMRV_OK);
    // Accumulating various draw frames make no sense, so force maxFrames to 1
    maxFrames = 1;
    gfmAccumulator_setFPS(pCtx->pDrawAcc, dps, maxFrames);
    ASSERT_NR(rv == GFMRV_OK);
    
    rv = GFMRV_OK;
__ret:
    // Make sure to clean both accumulators on error
    if (rv != GFMRV_OK && rv != GFMRV_ARGUMENTS_BAD) {
        gfmAccumulator_free(&(pCtx->pUpdateAcc));
        gfmAccumulator_free(&(pCtx->pDrawAcc));
    }
    
    return rv;
}

/**
 * Set the state's framerate
 * 
 * @param  pUps  Number of updates per seconds
 * @param  pDps  Number of draws per seconds
 * @param  pCtx  The game's context
 * @return       GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_ACC_NOT_INITIALIZED
 */
gfmRV gfm_getStateFrameRate(int *pUps, int *pDps, gfmCtx *pCtx) {
    gfmRV rv;
    
    // Sanitize arguments
    ASSERT(pUps, GFMRV_ARGUMENTS_BAD);
    ASSERT(pDps, GFMRV_ARGUMENTS_BAD);
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    // Check that it was initialized
    ASSERT(pCtx->pUpdateAcc, GFMRV_ACC_NOT_INITIALIZED);
    
    rv = gfmAccumulator_getFPS(pUps, pCtx->pUpdateAcc);
    ASSERT_NR(rv == GFMRV_OK);
    rv = gfmAccumulator_getFPS(pDps, pCtx->pDrawAcc);
    ASSERT_NR(rv == GFMRV_OK);
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Get how many updates frames have been issued since last call
 * 
 * @param  pAcc The number of frames
 * @param  pCtx The game's context
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_ACC_NOT_INITIALIZED
 */
gfmRV gfm_getUpdates(int *pAcc, gfmCtx *pCtx) {
    gfmRV rv;
    
    // Sanitize arguments
    ASSERT(pAcc, GFMRV_ARGUMENTS_BAD);
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    // Check that the accumulator was initialized
    ASSERT(pCtx->pUpdateAcc, GFMRV_ACC_NOT_INITIALIZED);
    
    // Get the number of frames
    rv = gfmAccumulator_getFrames(pAcc, pCtx->pUpdateAcc);
    ASSERT_NR(rv == GFMRV_OK);
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Get how many draw frames have been issued since last call; This number will
 * never go higher than '1'
 * 
 * @param  pAcc The number of frames
 * @param  pCtx The game's context
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_ACC_NOT_INITIALIZED
 */
gfmRV gfm_getDraws(int *pAcc, gfmCtx *pCtx) {
    gfmRV rv;
    
    // Sanitize arguments
    ASSERT(pAcc, GFMRV_ARGUMENTS_BAD);
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    // Check that the accumulator was initialized
    ASSERT(pCtx->pDrawAcc, GFMRV_ACC_NOT_INITIALIZED);
    
    // Get the number of frames
    rv = gfmAccumulator_getFrames(pAcc, pCtx->pDrawAcc);
    ASSERT_NR(rv == GFMRV_OK);
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Get how many time elapsed on each frame, in milliseconds; If static time loop
 * is used, this number will always be the same, for variable time loop, this
 * time will be the mean of how many frames were elapsed
 * 
 * NOTE: Only static time loop is implemented, as of now!
 * 
 * @param  pElapsed The elapsed time, in milliseconds
 * @param  pCtx     The game's context
 * @return          GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_ACC_NOT_INITIALIZED
 */
gfmRV gfm_getElapsedTime(int *pElapsed, gfmCtx *pCtx) {
    gfmRV rv;
    
    // Sanitize arguments
    ASSERT(pElapsed, GFMRV_ARGUMENTS_BAD);
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    // Check that the accumulator was initialized
    ASSERT(pCtx->pUpdateAcc, GFMRV_ACC_NOT_INITIALIZED);
    
    // Retrieve the elapsed time
    rv = gfmAccumulator_getDelay(pElapsed, pCtx->pUpdateAcc);
    ASSERT_NR(rv == GFMRV_OK);
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Get how many time elapsed on each frame, in seconds; If static time loop is
 * used, this number will always be the same, for variable time loop, this time
 * will be the mean of how many frames were elapsed
 * 
 * NOTE: Only static time loop is implemented, as of now!
 * 
 * @param  pElapsed The elapsed time, in seconds
 * @param  pCtx     The game's context
 * @return          GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_ACC_NOT_INITIALIZED
 */
gfmRV gfm_getElapsedTimef(float *pElapsed, gfmCtx *pCtx) {
    gfmRV rv;
    int delay;
    
    // Sanitize arguments
    ASSERT(pElapsed, GFMRV_ARGUMENTS_BAD);
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    // Check that the accumulator was initialized
    ASSERT(pCtx->pUpdateAcc, GFMRV_ACC_NOT_INITIALIZED);
    
    // Retrieve the elapsed time
    rv = gfmAccumulator_getDelay(&delay, pCtx->pUpdateAcc);
    ASSERT_NR(rv == GFMRV_OK);
    
    *pElapsed = 1000.0f / (float)delay;
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Get how many time elapsed on each frame, in seconds; If static time loop is
 * used, this number will always be the same, for variable time loop, this time
 * will be the mean of how many frames were elapsed
 * 
 * NOTE: Only static time loop is implemented, as of now!
 * 
 * @param  pElapsed The elapsed time, in seconds
 * @param  pCtx     The game's context
 * @return          GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_ACC_NOT_INITIALIZED
 */
gfmRV gfm_getElapsedTimed(double *pElapsed, gfmCtx *pCtx) {
    gfmRV rv;
    int delay;
    
    // Sanitize arguments
    ASSERT(pElapsed, GFMRV_ARGUMENTS_BAD);
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    // Check that the accumulator was initialized
    ASSERT(pCtx->pUpdateAcc, GFMRV_ACC_NOT_INITIALIZED);
    
    // Retrieve the elapsed time
    rv = gfmAccumulator_getDelay(&delay, pCtx->pUpdateAcc);
    ASSERT_NR(rv == GFMRV_OK);
    
    *pElapsed = 1000.0 / (double)delay;
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Update both accumulators
 * 
 * @param  pCtx The game's context
 * @param  ms   Time elapsed (in milliseconds)
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_ACC_NOT_INITIALIZED
 */
gfmRV gfm_updateAccumulators(gfmCtx *pCtx, int ms) {
    gfmRV rv;
    
    // Sanitize arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    ASSERT(ms > 0, GFMRV_ARGUMENTS_BAD);
    // Check that the accumulators was initialized
    ASSERT(pCtx->pUpdateAcc, GFMRV_ACC_NOT_INITIALIZED);
    ASSERT(pCtx->pDrawAcc, GFMRV_ACC_NOT_INITIALIZED);
    
    // Update the accumulators
    rv = gfmAccumulator_update(pCtx->pUpdateAcc, ms);
    ASSERT_NR(rv == GFMRV_OK);
    rv = gfmAccumulator_update(pCtx->pDrawAcc, ms);
    ASSERT_NR(rv == GFMRV_OK);
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Sleep until any event is received and handle everything
 * 
 * @param  pCtx The game's context
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfm_handleEvents(gfmCtx *pCtx) {
    gfmRV rv;
    
    // Sanitize arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    
    // Wait for the first event and process everything
    rv = gfmEvent_waitEvent(pCtx->pEvent);
    ASSERT_NR(rv == GFMRV_OK);
    rv = gfmEvent_processQueued(pCtx->pEvent, pCtx);
    ASSERT_NR(rv == GFMRV_OK);
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Initialize the FPS counter; On the release version, this function does
 * nothing but returns GFMRV_OK
 * 
 * @param  pCtx      The game's context
 * @param  pSset     The spriteset
 * @param  firstTile The first ASCII character's tile ('!') on the spriteset
 * @return           GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfm_initFPSCounter(gfmCtx *pCtx, gfmSpriteset *pSset, int firstTile) {
    gfmRV rv;
    
#if !defined(DEBUG) && !defined(FORCE_FPS)
    rv = GFMRV_OK;
#else
    // Sanitize arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    ASSERT(pSset, GFMRV_ARGUMENTS_BAD);
    ASSERT(firstTile > 0, GFMRV_ARGUMENTS_BAD);
    
    // Initialize the FPS counter
    rv = gfmFPSCounter_init(pCtx->pCounter, pSset, firstTile);
    ASSERT_NR(rv == GFMRV_OK);
    
    // Enable displaying the FPS
    pCtx->showFPS = 1;
    rv = GFMRV_OK;
__ret:
#endif
    return rv;
}

/**
 * Make the FPS counter visible
 * 
 * @param  pCtx      The game's context
 * @return           GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfm_showFPSCounter(gfmCtx *pCtx) {
    gfmRV rv;
    
    // Sanitize arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
#if defined(DEBUG) || defined(FORCE_FPS)
    // Check that it was initialized, on debug mode
    ASSERT(pCtx->pCounter, GFMRV_FPSCOUNTER_NOT_INITIALIZED);
    // Enable displaying the FPS
    pCtx->showFPS = 1;
#endif
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Hide the FPS counter
 * 
 * @param  pCtx      The game's context
 * @return           GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfm_hideFPSCounter(gfmCtx *pCtx) {
    gfmRV rv;
    
    // Sanitize arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
#if defined(DEBUG) || defined(FORCE_FPS)
    // Check that it was initialized, on debug mode
    ASSERT(pCtx->pCounter, GFMRV_FPSCOUNTER_NOT_INITIALIZED);
    // Enable displaying the FPS
    pCtx->showFPS = 0;
#endif
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Signal the counter that an update will happen; On the release version, this
 * function does nothing but returns GFMRV_OK
 * 
 * @param  pCtx      The game's context
 * @return           GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfm_fpsCounterUpdateBegin(gfmCtx *pCtx) {
    gfmRV rv;
    
#if !defined(DEBUG) && !defined(FORCE_FPS)
    rv = GFMRV_OK;
#else
    // Sanitize arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    // Check that it was initialized
    ASSERT(pCtx->pCounter, GFMRV_FPSCOUNTER_NOT_INITIALIZED);
    
    rv = gfmFPSCounter_updateBegin(pCtx->pCounter);
    ASSERT_NR(rv == GFMRV_OK);
    
    rv = GFMRV_OK;
__ret:
#endif
    return rv;
}

/**
 * Signal the counter that an update happened; On the release version, this
 * function does nothing but returns GFMRV_OK
 * 
 * @param  pCtx      The game's context
 * @return           GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfm_fpsCounterUpdateEnd(gfmCtx *pCtx) {
    gfmRV rv;
    
#if !defined(DEBUG) && !defined(FORCE_FPS)
    rv = GFMRV_OK;
#else
    // Sanitize arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    // Check that it was initialized
    ASSERT(pCtx->pCounter, GFMRV_FPSCOUNTER_NOT_INITIALIZED);
    
    rv = gfmFPSCounter_updateEnd(pCtx->pCounter);
    ASSERT_NR(rv == GFMRV_OK);
    
    rv = GFMRV_OK;
__ret:
#endif
    return rv;
}

/**
 * Takes a snapshot as soon as the frame finishes rendering and saves it as a
 * GIF image; If this function is called more than once in a frame, it will
 * ignore the second call and save according to the first call
 * 
 * @param  pCtx         The game's context
 * @param  pFilepath    Path (and filename) where it will be saved (depends on
 *                      useLocalPath); The extension isn't required, but, if
 *                      present, must be .gif!
 * @param  len          Filename's length
 * @param  useLocalPath Whether the path should be appended to the local path
 *                      (e.g., %APPDATA%\concat(organization, title)\, on
 *                      windows); or "as-is" (relative or absolute, depending on
 *                      the actual path)
 * @return              GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_OPERATION_ACTIVE,
 *                      GFMRV_ALLOC_FAILED, ...
 */
gfmRV gfm_snapshot(gfmCtx *pCtx, char *pFilepath, int len, int useLocalPath) {
    gfmRV rv;
    int height, width;
    volatile int newLen;
    
    // Sanitize arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    ASSERT(pFilepath, GFMRV_ARGUMENTS_BAD);
    ASSERT(len > 0, GFMRV_ARGUMENTS_BAD);
    // Check that the operation is available
    ASSERT(gfmGif_isSupported() == GFMRV_TRUE, GFMRV_FUNCTION_NOT_SUPPORTED);
    // Check that the operation isn't active
    ASSERT(!pCtx->takeSnapshot, GFMRV_OPERATION_ACTIVE);
    
    // Create the GIF exporter, if needed)
    if (!pCtx->pGif) {
        rv = gfmGif_getNew(&(pCtx->pGif));
        ASSERT_NR(rv == GFMRV_OK);
    }
    // Get the backbuffer's dimensions
    rv = gfmBackbuffer_getDimensions(&width, &height, pCtx->pBackbuffer);
    ASSERT_NR(rv == GFMRV_OK);
    // Initialize the gif exporter to the current backbuffer
    rv = gfmGif_init(pCtx->pGif, pCtx, width, height);
    ASSERT_NR(rv == GFMRV_OK);
    
    // Alloc as many bytes as required (or fail if not possible/supported)
    newLen = pCtx->ssDataLen;
    rv = gfmBackbuffer_getBackbufferData(0, (int*)&newLen, pCtx->pBackbuffer);
    ASSERT_NR(rv == GFMRV_OK);
    
    // Expand the buffer, as necessary
    if (newLen > pCtx->ssDataLen) {
        pCtx->pSsData = (unsigned char*)realloc(pCtx->pSsData,
                newLen * sizeof(unsigned char));
        ASSERT(pCtx->pSsData, GFMRV_ALLOC_FAILED);
        
        // Must store the new buffer len
        pCtx->ssDataLen = newLen;
    }
    
    // Create the path string, if necessary
    if (!pCtx->pSsPath) {
        rv = gfmString_getNew(&(pCtx->pSsPath));
        ASSERT_NR(rv == GFMRV_OK);
    }
    
    // store the path
    if (useLocalPath) {
        char *pLocalPath;
        int doCopy;
        
        // Retrieve the local path from the save file path
        rv = gfmString_getString(&pLocalPath, pCtx->pSaveFilename);
        ASSERT_NR(rv == GFMRV_OK);
        
        doCopy = 1;
        rv = gfmString_init(pCtx->pSsPath, pLocalPath, pCtx->saveFilenameLen,
                doCopy);
        ASSERT_NR(rv == GFMRV_OK);
        
        // Append the file's path
        rv = gfmString_concat(pCtx->pSsPath, pFilepath, len);
        ASSERT_NR(rv == GFMRV_OK);
    }
    else {
        int doCopy;
        
        // Create the string with the path
        doCopy = 1;
        rv = gfmString_init(pCtx->pSsPath, pFilepath, len, doCopy);
        ASSERT_NR(rv == GFMRV_OK);
    }
    
    // TODO check if there's an extension and add it
    
    // Request the operation
    pCtx->takeSnapshot = 1;
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Record a few milliseconds as a animated GIF
 * 
 * @param  pCtx         The game's context
 * @param  ms           How long should be recorded, in milliseconds
 * @param  pFilepath    Path (and filename) where it will be saved (depends on
 *                      useLocalPath); The extension isn't required, but, if
 *                      present, must be .gif!
 * @param  len          Filename's length
 * @param  useLocalPath Whether the path should be appended to the local path
 *                      (e.g., %APPDATA%\concat(organization, title)\, on
 *                      windows); or "as-is" (relative or absolute, depending on
 *                      the actual path)
 * @return              GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_OPERATION_ACTIVE,
 *                      GFMRV_ALLOC_FAILED, ...
 */
gfmRV gfm_recordGif(gfmCtx *pCtx, int ms, char *pFilepath, int len,
        int useLocalPath) {
    gfmRV rv;
    
    // Initialize it as if it's snapshot
    rv = gfm_snapshot(pCtx, pFilepath, len, useLocalPath);
    ASSERT_NR(rv == GFMRV_OK);
    
    // Set this as an animation
    pCtx->isAnimation = 1;
    pCtx->animationTime = ms;
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Initialize a rendering operation
 * 
 * @param  pCtx  The game's context
 * @return       GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_BACKBUFFER_NOT_INITIALIZED
 */
gfmRV gfm_drawBegin(gfmCtx *pCtx) {
    gfmRV rv;
    
    // Sanitize arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    // Check that the backbuffer was initialized
    ASSERT(pCtx->pBackbuffer, GFMRV_BACKBUFFER_NOT_INITIALIZED);
    
#if defined(DEBUG) || defined(FORCE_FPS)
    // Store when drawing was initialized
    rv = gfmFPSCounter_initDraw(pCtx->pCounter);
    ASSERT_NR(rv == GFMRV_OK);
#endif
    
    // If there's a default texture, load it
    if (pCtx->defaultTexture >= 0) {
        rv = gfm_drawLoadCachedTexture(pCtx, pCtx->defaultTexture);
        ASSERT_NR(rv == GFMRV_OK);
    }
    
    rv = gfmBackbuffer_drawBegin(pCtx->pBackbuffer);
    ASSERT_NR(rv == GFMRV_OK);
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Loads a texture into the backbuffer; The texture must be managed by the
 * framework
 * 
 * @param  pCtx  The game's context
 * @param  iTex Texture index (the value returned when created)
 * @return       GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_INVALID_INDEX
 */
gfmRV gfm_drawLoadCachedTexture(gfmCtx *pCtx, int iTex) {
    gfmRV rv;
    
    // Sanitize arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    ASSERT(iTex >= 0, GFMRV_ARGUMENTS_BAD);
    // Check that the texture exists
    ASSERT(iTex < gfmGenArr_getUsed(pCtx->pTextures), GFMRV_INVALID_INDEX);
    
    // Load the texture
    rv = gfm_drawLoadTexture(pCtx, gfmGenArr_getObject(pCtx->pTextures, iTex));
    ASSERT_NR(rv == GFMRV_OK);
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Loads a texture into the backbuffer
 * 
 * @param  pCtx  The game's context
 * @param  pTex  The texture
 * @return       GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_TEXTURE_NOT_INITIALIZED
 */
gfmRV gfm_drawLoadTexture(gfmCtx *pCtx, gfmTexture *pTex) {
    gfmRV rv;
    
    // Sanitize arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    ASSERT(pTex, GFMRV_ARGUMENTS_BAD);
    // Check that the backbuffer was initialized
    ASSERT(pCtx->pBackbuffer, GFMRV_BACKBUFFER_NOT_INITIALIZED);
    
    // Load it into the backbuffer
    rv = gfmBackbuffer_drawLoadTexture(pCtx->pBackbuffer, pTex);
    ASSERT_NR(rv == GFMRV_OK);
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Initialize a batch of renders (i.e., render many sprites in a single draw
 * call)
 * 
 * @param  pCtx  The game's context
 * @return       GFMRV_OK, GFMRV_ARGUMENTS_BAD, ...
 */
gfmRV gfm_batchBegin(gfmCtx *pCtx) {
    return GFMRV_FUNCTION_NOT_IMPLEMENTED;
}

/**
 * Renders a tile into the backbuffer
 * 
 * @param  pCtx  The game's context
 * @param  pSSet The spriteset containing the tile
 * @param  x     Horizontal position in screen space
 * @param  y     Vertical position in screen space
 * @param  tile  Tile to be rendered
 * @return       GFMRV_OK, GFMRV_ARGUMENTS_BAD,
 *               GFMRV_BACKBUFFER_NOT_INITIALIZED,
 */
gfmRV gfm_drawTile(gfmCtx *pCtx, gfmSpriteset *pSset, int x, int y, int tile) {
    gfmRV rv;
    
    // Sanitize arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    ASSERT(pSset, GFMRV_ARGUMENTS_BAD);
    ASSERT(tile >= 0, GFMRV_ARGUMENTS_BAD);
    // Check that the backbuffer was initialized
    ASSERT(pCtx->pBackbuffer, GFMRV_BACKBUFFER_NOT_INITIALIZED);
    
    // Render the tile
    rv = gfmBackbuffer_drawTile(pCtx->pBackbuffer, pSset, x, y, tile);
    ASSERT_NR(rv == GFMRV_OK);
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Renders a number at the desired position; The spriteset's texture must have
 * a bitmap font (in the ASCII sequence)
 * 
 * @param  pCtx      The game's context
 * @param  pSSet     The spriteset containing the tile
 * @param  x         Horizontal position in screen space
 * @param  y         Vertical position in screen space
 * @param  num       Number to be rendered
 * @param  res       Number of digits
 * @param  firstTile First ASCII tile in the spriteset
 * @return           GFMRV_OK, GFMRV_ARGUMENTS_BAD,
 *                   GFMRV_BACKBUFFER_NOT_INITIALIZED,
 */
gfmRV gfm_drawNumber(gfmCtx *pCtx, gfmSpriteset *pSset, int x, int y, int num,
        int res, int firstTile) {
    gfmRV rv;
    int digits, tileWidth, tileHeight, tile;
    
    // Sanitize arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    ASSERT(pSset, GFMRV_ARGUMENTS_BAD);
    // Check that the backbuffer was initialized
    ASSERT(pCtx->pBackbuffer, GFMRV_BACKBUFFER_NOT_INITIALIZED);
    
    // Get the spriteset dimensions
    rv = gfmSpriteset_getDimension(&tileWidth, &tileHeight, pSset);
    ASSERT_NR(rv == GFMRV_OK);
    
    // Get 10^(res-1) (to get separate each digit later)
    digits = 1;
    while (res > 1) {
        digits *= 10;
        res--;
    }
    
    // Renders a '-' sign, if necessary
    if (num < 0) {
        // Get the tile position on the texture
        tile = '-' - '!' + firstTile;
        // Render it
        rv = gfmBackbuffer_drawTile(pCtx->pBackbuffer, pSset, x, y, tile);
        ASSERT_NR(rv == GFMRV_OK);
        // Update the number and its position
        num *= -1;
        x += tileWidth;
    }
    
    // Render every digit
    while (digits > 0) {
        // Get the current digit
        tile = num / digits % 10;
        // Get its position on the texture
        tile = tile + '0' - '!' + firstTile;
        // Render it
        rv = gfmBackbuffer_drawTile(pCtx->pBackbuffer, pSset, x, y, tile);
        ASSERT_NR(rv == GFMRV_OK);
        // Update its position and the digit
        x += tileWidth;
        digits /= 10;
    }
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Renders a sprite into the backbuffer
 * 
 * @param  pCtx  The game's context
 * @param  pSpr  The sprite
 * @return       GFMRV_OK, GFMRV_ARGUMENTS_BAD, ...
 */
gfmRV gfm_drawSprite(gfmCtx *pCtx, gfmSprite *pSpr);

/**
 * Finalize a batch of renders (i.e., render many sprites in a single draw call)
 * 
 * @param  pCtx  The game's context
 * @return       GFMRV_OK, GFMRV_ARGUMENTS_BAD, ...
 */
gfmRV gfm_batchEnd(gfmCtx *pCtx) {
    return GFMRV_FUNCTION_NOT_IMPLEMENTED;
}

/**
 * Finalize a rendering operation
 * 
 * @param  pCtx  The game's context
 * @return       GFMRV_OK, GFMRV_ARGUMENTS_BAD, ...
 */
gfmRV gfm_drawEnd(gfmCtx *pCtx) {
    gfmRV rv;
    
    // Sanitize arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    // Check that the backbuffer was initialized
    ASSERT(pCtx->pBackbuffer, GFMRV_BACKBUFFER_NOT_INITIALIZED);
    
#if defined(DEBUG) || defined(FORCE_FPS)
    // Display the current fps
    if (pCtx->showFPS) {
        rv = gfmFPSCounter_draw(pCtx->pCounter, pCtx);
        ASSERT_NR(rv == GFMRV_OK);
    }
#endif
    
    rv = gfmBackbuffer_drawEnd(pCtx->pBackbuffer, pCtx->pWindow);
    ASSERT_NR(rv == GFMRV_OK);
    
    // If requested, take the snapshot
    if (pCtx->takeSnapshot) {
        volatile int len;
        
        // Retrieve the data
        len = pCtx->ssDataLen;
        rv = gfmBackbuffer_getBackbufferData(pCtx->pSsData, (int*)&len,
                pCtx->pBackbuffer);
        ASSERT_NR(rv == GFMRV_OK);
        
        // Store it in a GIF image
        rv = gfmGif_storeFrame(pCtx->pGif, pCtx->pSsData, len);
        ASSERT_NR(rv == GFMRV_OK);
        
        if (!pCtx->isAnimation) {
            // If it's a snapshot, simply save the animation
            rv = gfmGif_exportImage(pCtx->pGif, pCtx->pSsPath);
            ASSERT_NR(rv == GFMRV_OK);
        
            pCtx->takeSnapshot = 0;
        }
        else {
            int delay;
            
            // Update the animation timer
            rv = gfmAccumulator_getDelay(&delay, pCtx->pDrawAcc);
            ASSERT_NR(rv == GFMRV_OK);
            
            pCtx->animationTime -= delay;
            
            // If enough frames were recorded, export it
            if (pCtx->animationTime <= 0) {
                rv = gfmGif_exportAnimation(pCtx->pGif, pCtx->pSsPath);
                ASSERT_NR(rv == GFMRV_OK);
                
                pCtx->takeSnapshot = 0;
                pCtx->isAnimation = 0;
            }
        }
    }
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Clean up a context
 * 
 * @param  pCtx The context
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfm_clean(gfmCtx *pCtx) {
    gfmRV rv;
    
    // Sanitize the arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    
    // Clean every allocated 'object'
    gfmString_free(&(pCtx->pGameOrg));
    gfmString_free(&(pCtx->pGameTitle));
    gfmString_free(&(pCtx->pSaveFilename));
#ifndef GFRAME_MOBILE
    gfmString_free(&(pCtx->pBinPath));
#endif
    gfmBackbuffer_free(&(pCtx->pBackbuffer));
    gfmWindow_free(&(pCtx->pWindow));
    gfmCamera_free(&(pCtx->pCamera));
    gfmTimer_free(&(pCtx->pTimer));
    gfmGenArr_clean(pCtx->pTextures, gfmTexture_free);
    gfmGenArr_clean(pCtx->pSpritesets, gfmSpriteset_free);
    gfmAccumulator_free(&(pCtx->pUpdateAcc));
    gfmAccumulator_free(&(pCtx->pDrawAcc));
    gfmEvent_free(&(pCtx->pEvent));
#if defined(DEBUG) || defined(FORCE_FPS)
    gfmFPSCounter_free(&(pCtx->pCounter));
#endif
    gfmGif_free(&(pCtx->pGif));
    if (pCtx->pSsData) {
        free(pCtx->pSsData);
        pCtx->pSsData = 0;
    }
    gfmString_free(&(pCtx->pSsPath));
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

