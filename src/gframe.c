/**
 * @src/gframe.c
 */
#include <GFraMe/gframe.h>
#include <GFraMe/gfmAccumulator.h>
#include <GFraMe/gfmAssert.h>
#include <GFraMe/gfmCamera.h>
#include <GFraMe/gfmError.h>
#include <GFraMe/gfmGenericArray.h>
#include <GFraMe/gfmSpriteset.h>
#include <GFraMe/gfmString.h>
#include <GFraMe/core/gfmBackbuffer_bkend.h>
#include <GFraMe/core/gfmBackend_bkend.h>
#include <GFraMe/core/gfmEvent_bkend.h>
#include <GFraMe/core/gfmTexture_bkend.h>
#include <GFraMe/core/gfmTimer_bkend.h>
#include <GFraMe/core/gfmPath_bkend.h>
#include <GFraMe/core/gfmWindow_bkend.h>

#include <stdlib.h>
#include <string.h>

/**
 * Define the texture array type
 */
gfmGenArr_define(gfmTexture);

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
    ASSERT(!(*ppOrg), GFMRV_ARGUMENTS_BAD);
    ASSERT(ppTitle, GFMRV_ARGUMENTS_BAD);
    ASSERT(!(*ppTitle), GFMRV_ARGUMENTS_BAD);
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
 * Renders a sprite into the backbuffer
 * 
 * @param  pCtx  The game's context
 * @param  pSpr  The sprite
 * @return       GFMRV_OK, GFMRV_ARGUMENTS_BAD, ...
 */
//gfmRV gfm_drawSprite(gfmCtx *pCtx, gfmSprite *pSpr);

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
    
    rv = gfmBackbuffer_drawEnd(pCtx->pBackbuffer, pCtx->pWindow);
    ASSERT_NR(rv == GFMRV_OK);
    
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
    gfmAccumulator_free(&(pCtx->pUpdateAcc));
    gfmAccumulator_free(&(pCtx->pDrawAcc));
    gfmEvent_free(&(pCtx->pEvent));
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

