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
#include <GFraMe/gfmInput.h>
#include <GFraMe/gfmLog.h>
#include <GFraMe/gfmSprite.h>
#include <GFraMe/gfmSpriteset.h>
#include <GFraMe/gfmString.h>
#include <GFraMe/core/gfmAudio_bkend.h>
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
     * append stuff) */
    int saveFilenameLen;
    /** Whether the backend was initialized */
    int isBackendInit;
    /** Flag to easily disable the audio; must be set after initialized the
     * lib */
    int isAudioEnabled;
    /** Audio sub-system context */
    gfmAudioCtx *pAudio;
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
    /** Input context */
    gfmInput *pInput;
    /** The logger */
    gfmLog *pLog;
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
    /** How many update frames were accumulated */
    int updateFrames;
    /** How many draw frames were accumulated */
    int drawFrames;
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
    
    rv = GFMRV_OK;
__ret:
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
 * Initialize and alloc every one of this object's members
 * 
 * @param  pCtx    The allocated context
 * @param  pOrg    Organization's name (used by log and save file)
 * @param  orgLen  Organization's name's length
 * @param  pName   Game's title (also used as window's title)
 * @param  nameLen Game's title's length
 * @return         GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfm_init(gfmCtx *pCtx, char *pOrg, int orgLen, char *pName, int nameLen) {
    gfmRV rv;
    
    // Sanitize the arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    // Check that it still wasn't initialized
    ASSERT(!(pCtx->pLog), GFMRV_ALREADY_INITIALIZED);
    
#ifndef GFRAME_MOBILE
	// Get current directory
    rv = gfmPath_getRunningPath(&(pCtx->pBinPath));
    ASSERT_NR(rv == GFMRV_OK);
    rv = gfmString_getLength(&(pCtx->binPathLen), pCtx->pBinPath);
    ASSERT_NR(rv == GFMRV_OK);
#endif
    
    // Init the current backend
    // TODO allow more than one backend?
    rv = gfmBackend_init();
    ASSERT_NR(rv == GFMRV_OK);
    // Set the backend as initialized
    pCtx->isBackendInit = 1;
    
    // Initialize the logger
    rv = gfmLog_getNew(&(pCtx->pLog));
    ASSERT_NR(rv == GFMRV_OK);
    
    // Initialize the fps counter, if debug
#if defined(DEBUG) || defined(FORCE_FPS)
    rv = gfmFPSCounter_getNew(&(pCtx->pCounter));
    ASSERT_NR(rv == GFMRV_OK);
#endif
    
    // Set the game's title
    rv = gfm_setTitle(pCtx, pOrg, orgLen, pName, nameLen);
    ASSERT_NR(rv == GFMRV_OK);
    
    // Initialize the logger
#if defined(DEBUG) || defined(FORCE_FPS)
    rv = gfmLog_init(pCtx->pLog, pCtx, gfmLog_debug);
#else
    rv = gfmLog_init(pCtx->pLog, pCtx, gfmLog_info);
#endif
    ASSERT_NR(rv == GFMRV_OK);
    
    rv = gfmLog_log(pCtx->pLog, gfmLog_info, "");
    ASSERT_NR(rv == GFMRV_OK);
    rv = gfmLog_log(pCtx->pLog, gfmLog_info, "---------------------------------"
            "-----------------------------------------------");
    ASSERT_NR(rv == GFMRV_OK);
    rv = gfmLog_log(pCtx->pLog, gfmLog_info, "Initializing GFraMe...");
    ASSERT_NR(rv == GFMRV_OK);
    
    // Initialize the event's context
    rv = gfmEvent_getNew(&(pCtx->pEvent));
    ASSERT_NR(rv == GFMRV_OK);
    rv = gfmEvent_init(pCtx->pEvent, pCtx);
    ASSERT_NR(rv == GFMRV_OK);
    
    // Initialize the input system
    rv = gfmInput_getNew(&(pCtx->pInput));
    ASSERT_NR(rv == GFMRV_OK);
    rv = gfmInput_init(pCtx->pInput);
    ASSERT_NR(rv == GFMRV_OK);
    
    rv = gfmLog_log(pCtx->pLog, gfmLog_info, "GFraMe initialized!");
    ASSERT_NR(rv == GFMRV_OK);
    
    // Set the game as running
    pCtx->isAudioEnabled = 1;
    pCtx->doQuit = GFMRV_FALSE;
    pCtx->defaultTexture = -1;
    rv = GFMRV_OK;
__ret:
    // Clean up the context, on error
    if (rv != GFMRV_OK && rv != GFMRV_ARGUMENTS_BAD) {
        gfm_clean(pCtx);
    }
    
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
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    // Check that the lib was initialized
    ASSERT(pCtx->pLog, GFMRV_NOT_INITIALIZED);
    // Continue to sanitize arguments
    ASSERT_LOG(ppStr, GFMRV_ARGUMENTS_BAD, pCtx->pLog);
    
    // Remove anything that was concatenated
    rv = gfmString_setLength(pCtx->pBinPath, pCtx->binPathLen);
    ASSERT_LOG(rv == GFMRV_OK, rv, pCtx->pLog);
    // Return the string
    *ppStr = pCtx->pBinPath;
#endif
    
    rv = GFMRV_OK;
__ret:
    return rv;
}


/**
 * Set the game's title and organization; Also enable logging
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
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    // Check that the lib was initialized
    ASSERT(pCtx->pLog, GFMRV_NOT_INITIALIZED);
    // Continue to sanitize arguments
    ASSERT_LOG(ppOrg, GFMRV_ARGUMENTS_BAD, pCtx->pLog);
    ASSERT_LOG(ppTitle, GFMRV_ARGUMENTS_BAD, pCtx->pLog);
    
    // Check that the string were set
    ASSERT_LOG(pCtx->pGameOrg, GFMRV_TITLE_NOT_SET, pCtx->pLog);
    ASSERT_LOG(pCtx->pGameTitle, GFMRV_TITLE_NOT_SET, pCtx->pLog);
    
    // Retrieve the strings
    rv = gfmString_getString(ppOrg, pCtx->pGameOrg);
    ASSERT_LOG(rv == GFMRV_OK, rv, pCtx->pLog);
    rv = gfmString_getString(ppTitle, pCtx->pGameTitle);
    ASSERT_LOG(rv == GFMRV_OK, rv, pCtx->pLog);
    
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
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    // Check that the lib was initialized
    ASSERT(pCtx->pLog, GFMRV_NOT_INITIALIZED);
    // Continue to sanitize arguments
    ASSERT_LOG(pCount, GFMRV_ARGUMENTS_BAD, pCtx->pLog);
    
    // Alloc the window
    if (!(pCtx->pWindow)) {
        rv = gfmWindow_getNew(&(pCtx->pWindow));
        ASSERT_LOG(rv == GFMRV_OK, rv, pCtx->pLog);
    }
    // Query the resolutions and set 'pCount'
    rv = gfmWindow_queryResolutions(pCount, pCtx->pWindow);
    ASSERT_LOG(rv == GFMRV_OK, rv, pCtx->pLog);
    
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
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    // Check that the lib was initialized
    ASSERT(pCtx->pLog, GFMRV_NOT_INITIALIZED);
    // Continue to sanitize arguments
    ASSERT_LOG(pWidth, GFMRV_ARGUMENTS_BAD, pCtx->pLog);
    ASSERT_LOG(pHeight, GFMRV_ARGUMENTS_BAD, pCtx->pLog);
    ASSERT_LOG(pRefRate, GFMRV_ARGUMENTS_BAD, pCtx->pLog);
    
    // Get the desired resolution
    rv = gfmWindow_getResolution(pWidth, pHeight, pRefRate, pCtx->pWindow,
            index);
    ASSERT_LOG(rv == GFMRV_OK, rv, pCtx->pLog);
    
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
    // Check that the lib was initialized
    ASSERT(pCtx->pLog, GFMRV_NOT_INITIALIZED);
    
    // Check that the window hasn't been initialized
    ASSERT_LOG(!(pCtx->pWindow) ||
            gfmWindow_wasInit(pCtx->pWindow) == GFMRV_FALSE,
            GFMRV_WINDOW_ALREADY_INITIALIZED, pCtx->pLog);
    // Check that the backbuffer hasn't been initialized
    ASSERT_LOG(!pCtx->pBackbuffer, GFMRV_BACKBUFFER_ALREADY_INITIALIZED,
            pCtx->pLog);
    // Basic check for the resolution (it'll be later re-done, on window_init)
    ASSERT_LOG(wndWidth > 0, GFMRV_INVALID_WIDTH, pCtx->pLog);
    ASSERT_LOG(wndHeight > 0, GFMRV_INVALID_HEIGHT, pCtx->pLog);
    
    // Try to read the game's title
    pOrg = 0;
    pTitle = 0;
    rv = gfm_getTitle(&pOrg, &pTitle, pCtx);
    ASSERT_LOG(rv == GFMRV_OK, rv, pCtx->pLog);
    
    rv = gfmLog_log(pCtx->pLog, gfmLog_info, "Initializing window to %ix%i "
            "(backbuffer: %ix%i)", wndWidth, wndHeight, bufWidth, bufHeight);
    ASSERT_LOG(rv == GFMRV_OK, rv, pCtx->pLog);
    
    // Alloc and initialize the window
    if (!(pCtx->pWindow)) {
        rv = gfmWindow_getNew(&(pCtx->pWindow));
        ASSERT_LOG(rv == GFMRV_OK, rv, pCtx->pLog);
    }
    rv = gfmWindow_init(pCtx->pWindow, wndWidth, wndHeight, pTitle,
            isUserResizable);
    ASSERT_LOG(rv == GFMRV_OK, rv, pCtx->pLog);
    
    // Alloc and initialize the backbuffer
    rv = gfmBackbuffer_getNew(&(pCtx->pBackbuffer));
    ASSERT_LOG(rv == GFMRV_OK, rv, pCtx->pLog);
    rv = gfmBackbuffer_init(pCtx->pBackbuffer, pCtx->pWindow, bufWidth,
            bufHeight);
    ASSERT_LOG(rv == GFMRV_OK, rv, pCtx->pLog);
    
    // Alloc and initialize the camera
    rv = gfmCamera_getNew(&(pCtx->pCamera));
    ASSERT_LOG(rv == GFMRV_OK, rv, pCtx->pLog);
    rv = gfmCamera_init(pCtx->pCamera, pCtx, bufWidth, bufHeight);
    ASSERT_LOG(rv == GFMRV_OK, rv, pCtx->pLog);
    
    rv = gfmLog_log(pCtx->pLog, gfmLog_info, "Window initialized!");
    ASSERT_LOG(rv == GFMRV_OK, rv, pCtx->pLog);
    
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
    // Check that the lib was initialized
    ASSERT(pCtx->pLog, GFMRV_NOT_INITIALIZED);
    // Continue to sanitize arguments
    ASSERT_LOG(resIndex >= 0, GFMRV_ARGUMENTS_BAD, pCtx->pLog);
    // Check that the window hasn't been initialized
    ASSERT_LOG(!(pCtx->pWindow) ||
            gfmWindow_wasInit(pCtx->pWindow) == GFMRV_FALSE,
            GFMRV_WINDOW_ALREADY_INITIALIZED, pCtx->pLog);
    
    // Try to read the game's title
    pOrg = 0;
    pTitle = 0;
    rv = gfm_getTitle(&pOrg, &pTitle, pCtx);
    ASSERT_LOG(rv == GFMRV_OK, rv, pCtx->pLog);
    
    rv = gfmLog_log(pCtx->pLog, gfmLog_info, "Initializing window in "
            "fullscreen mode (backbuffer: %ix%i)", bufWidth, bufHeight);
    ASSERT_LOG(rv == GFMRV_OK, rv, pCtx->pLog);
    
    // Alloc and initialize the window
    if (!(pCtx->pWindow)) {
        rv = gfmWindow_getNew(&(pCtx->pWindow));
        ASSERT_LOG(rv == GFMRV_OK, rv, pCtx->pLog);
    }
    rv = gfmWindow_initFullScreen(pCtx->pWindow, resIndex, pTitle,
            isUserResizable);
    ASSERT_LOG(rv == GFMRV_OK, rv, pCtx->pLog);
    
    // Alloc and initialize the backbuffer
    rv = gfmBackbuffer_getNew(&(pCtx->pBackbuffer));
    ASSERT_LOG(rv == GFMRV_OK, rv, pCtx->pLog);
    rv = gfmBackbuffer_init(pCtx->pBackbuffer, pCtx->pWindow, bufWidth,
            bufHeight);
    ASSERT_LOG(rv == GFMRV_OK, rv, pCtx->pLog);
    
    // Alloc and initialize the camera
    rv = gfmCamera_getNew(&(pCtx->pCamera));
    ASSERT_LOG(rv == GFMRV_OK, rv, pCtx->pLog);
    rv = gfmCamera_init(pCtx->pCamera, pCtx, bufWidth, bufHeight);
    ASSERT_LOG(rv == GFMRV_OK, rv, pCtx->pLog);
    
    rv = gfmLog_log(pCtx->pLog, gfmLog_info, "Window initialized!");
    ASSERT_LOG(rv == GFMRV_OK, rv, pCtx->pLog);
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Disable the audio subsystem; Any further call to any audio function will be
 * ignored
 * NOTE: It must be called before gfm_initAudio!
 * 
 * @param  pCtx The game's context
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_NOT_INITIALIZED,
 *              GFMRV_AUDIO_ALREADY_INITIALIZED
 */
gfmRV gfm_disableAudio(gfmCtx *pCtx) {
    gfmRV rv;
    
    // Sanitize arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    // Check that the lib was initialized
    ASSERT(pCtx->pLog, GFMRV_NOT_INITIALIZED);
    ASSERT_LOG(pCtx->pAudio == 0, GFMRV_AUDIO_ALREADY_INITIALIZED, pCtx->pLog);
    
    pCtx->isAudioEnabled = 0;
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Initialize the audio sub-system; This function must be called before loading
 * any song
 * 
 * @param  pCtx     The game's context
 * @param  settings Audio quality (sampling rate, etc)
 * @return          GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_ALLOC_FAILED
 */
gfmRV gfm_initAudio(gfmCtx *pCtx, gfmAudioQuality settings) {
    gfmRV rv;
    
    // Sanitize arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    // Check that the lib was initialized
    ASSERT(pCtx->pLog, GFMRV_NOT_INITIALIZED);
    // Check that audio is enabled
    if (pCtx->isAudioEnabled != 1) {
        rv = GFMRV_OK;
        goto __ret;
    }
    
    rv = gfmLog_log(pCtx->pLog, gfmLog_info, "Initializing audio subsystem...");
    ASSERT_LOG(rv == GFMRV_OK, rv, pCtx->pLog);
    
    // Alloc the audio sub-system
    rv = gfmAudio_getNew(&(pCtx->pAudio));
    ASSERT_LOG(rv == GFMRV_OK, rv, pCtx->pLog);
    
    // Initialize it
    rv = gfmAudio_initSubsystem(pCtx->pAudio, pCtx, settings);
    ASSERT_LOG(rv == GFMRV_OK, rv, pCtx->pLog);
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Set to which sample the song must loop
 * 
 * @param  pCtx   The game's context
 * @param  handle The handle of the looped audio
 * @param  pos    Sample to which the song should go back when looping
 * @return        GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_INVALID_INDEX,
 *                GFMRV_INVALID_BUFFER_LEN
 */
gfmRV gfm_setRepeat(gfmCtx *pCtx, int handle, int pos) {
    gfmRV rv;
    
    // Sanitize arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    // Check that the lib was initialized
    ASSERT(pCtx->pLog, GFMRV_NOT_INITIALIZED);
    // Sanitize the other arguments on the actual call
    
    // Check that audio is enabled
    if (pCtx->isAudioEnabled != 1) {
        rv = GFMRV_OK;
        goto __ret;
    }
    
    rv = gfmAudio_setRepeat(pCtx->pAudio, handle, pos);
    ASSERT_LOG(rv == GFMRV_OK, rv, pCtx->pLog);
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Plays an audio and returns its instance
 * 
 * @param  ppHnd  The audio instance (may be NULL, if one simply doesn't care)
 * @param  pCtx The game's context
 * @param  handle The handle of the audio to be played
 * @param  volume How loud should the audio be played (in the range (0.0, 1.0])
 * @return        GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_INVALID_INDEX,
 *                GFMRV_AUDIO_NOT_INITIALIZED, GFMRV_ALLOC_FAILED, 
 */
gfmRV gfm_playAudio(gfmAudioHandle **ppHnd, gfmCtx *pCtx, int handle,
        double volume) {
    gfmRV rv;
    
    // Sanitize arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    // Check that the lib was initialized
    ASSERT(pCtx->pLog, GFMRV_NOT_INITIALIZED);
    // Sanitize the other arguments on the actual call
    
    // Check that audio is enabled
    if (pCtx->isAudioEnabled != 1) {
        rv = GFMRV_OK;
        goto __ret;
    }
    
    rv = gfmAudio_playAudio(ppHnd, pCtx->pAudio, handle, volume);
    ASSERT_LOG(rv == GFMRV_OK, rv, pCtx->pLog);
    
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
    // Check that the lib was initialized
    ASSERT(pCtx->pLog, GFMRV_NOT_INITIALIZED);
    // Continue to sanitize arguments
    ASSERT_LOG(fps > 0, GFMRV_ARGUMENTS_BAD, pCtx->pLog);
    
    if (!pCtx->pTimer) {
        // Create a new timer, if necessary
        rv = gfmTimer_getNew(&(pCtx->pTimer), pCtx);
        ASSERT_LOG(rv == GFMRV_OK, rv, pCtx->pLog);
        // Initialize the timer
        rv = gfmTimer_init(pCtx->pTimer, fps);
        ASSERT_LOG(rv == GFMRV_OK, rv, pCtx->pLog);
    }
    else {
        // Only modify the timer
        rv = gfmTimer_setFPS(pCtx->pTimer, fps);
        ASSERT_LOG(rv == GFMRV_OK, rv, pCtx->pLog);
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
    // Check that the lib was initialized
    ASSERT(pCtx->pLog, GFMRV_NOT_INITIALIZED);
    // Continue to sanitize arguments
    ASSERT_LOG(fps > 0, GFMRV_ARGUMENTS_BAD, pCtx->pLog);
    
    if (!pCtx->pTimer) {
        // Create a new timer, if necessary
        rv = gfmTimer_getNew(&(pCtx->pTimer), pCtx);
        ASSERT_LOG(rv == GFMRV_OK, rv, pCtx->pLog);
        // Initialize the timer
        rv = gfmTimer_initRaw(pCtx->pTimer, fps);
        ASSERT_LOG(rv == GFMRV_OK, rv, pCtx->pLog);
    }
    else {
        // Only modify the timer
        rv = gfmTimer_setFPSRaw(pCtx->pTimer, fps);
        ASSERT_LOG(rv == GFMRV_OK, rv, pCtx->pLog);
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
    // Check that the lib was initialized
    ASSERT(pCtx->pLog, GFMRV_NOT_INITIALIZED);
    
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
    // Check that the lib was initialized
    ASSERT(pCtx->pLog, GFMRV_NOT_INITIALIZED);
    
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
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    // Check that the lib was initialized
    ASSERT(pCtx->pLog, GFMRV_NOT_INITIALIZED);
    // Continue to sanitize arguments
    ASSERT_LOG(ppEvent, GFMRV_ARGUMENTS_BAD, pCtx->pLog);
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
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    // Check that the lib was initialized
    ASSERT(pCtx->pLog, GFMRV_NOT_INITIALIZED);
    // Continue to sanitize arguments
    ASSERT_LOG(ppBbuf, GFMRV_ARGUMENTS_BAD, pCtx->pLog);
    // Check that the renderer was initialized
    ASSERT_LOG(pCtx->pBackbuffer, GFMRV_BACKBUFFER_NOT_INITIALIZED, pCtx->pLog);
    
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
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    // Check that the lib was initialized
    ASSERT(pCtx->pLog, GFMRV_NOT_INITIALIZED);
    // Continue to sanitize arguments
    ASSERT_LOG(pWidth, GFMRV_ARGUMENTS_BAD, pCtx->pLog);
    ASSERT_LOG(pHeight, GFMRV_ARGUMENTS_BAD, pCtx->pLog);
    // Check that the renderer was initialized
    ASSERT_LOG(pCtx->pBackbuffer, GFMRV_BACKBUFFER_NOT_INITIALIZED, pCtx->pLog);
    
    // Get the dimenensions
    rv = gfmBackbuffer_getDimensions(pWidth, pHeight, pCtx->pBackbuffer);
    ASSERT_LOG(rv == GFMRV_OK, rv, pCtx->pLog);
    
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
    // Check that the lib was initialized
    ASSERT(pCtx->pLog, GFMRV_NOT_INITIALIZED);
    // Check that the window has already been initialized
    ASSERT_LOG(pCtx->pWindow && gfmWindow_wasInit(pCtx->pWindow) == GFMRV_TRUE,
            GFMRV_WINDOW_NOT_INITIALIZED, pCtx->pLog);
    // Check that the backbuffer was initialized
    ASSERT_LOG(pCtx->pBackbuffer, GFMRV_BACKBUFFER_NOT_INITIALIZED, pCtx->pLog);
    
    // Cache the backbuffer's output dimensions
    rv = gfmBackbuffer_cacheDimensions(pCtx->pBackbuffer, width, height);
    ASSERT_LOG(rv == GFMRV_OK, rv, pCtx->pLog);
    // Set the window's dimentions
    rv = gfmWindow_setDimensions(pCtx->pWindow, width, height);
    ASSERT_LOG(rv == GFMRV_OK, rv, pCtx->pLog);
    
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
    // Check that the lib was initialized
    ASSERT(pCtx->pLog, GFMRV_NOT_INITIALIZED);
    // Check that the window has already been initialized
    ASSERT_LOG(pCtx->pWindow && gfmWindow_wasInit(pCtx->pWindow) == GFMRV_TRUE,
            GFMRV_WINDOW_NOT_INITIALIZED, pCtx->pLog);
    
    // Try to make it fullscreen
    rv = gfmWindow_setFullScreen(pCtx->pWindow);
    ASSERT_LOG(rv == GFMRV_OK, rv, pCtx->pLog);
    // Cache the backbuffer's output dimensions
    rv = gfmWindow_getDimensions(&wndWidth, &wndHeight, pCtx->pWindow);
    ASSERT_LOG(rv == GFMRV_OK, rv, pCtx->pLog);
    rv = gfmBackbuffer_cacheDimensions(pCtx->pBackbuffer, wndWidth, wndHeight);
    ASSERT_LOG(rv == GFMRV_OK, rv, pCtx->pLog);
    
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
    // Check that the lib was initialized
    ASSERT(pCtx->pLog, GFMRV_NOT_INITIALIZED);
    // Check that the window has already been initialized
    ASSERT_LOG(pCtx->pWindow && gfmWindow_wasInit(pCtx->pWindow) == GFMRV_TRUE,
            GFMRV_WINDOW_NOT_INITIALIZED, pCtx->pLog);
    
    // Try to make it fullscreen
    rv = gfmWindow_setWindowed(pCtx->pWindow);
    ASSERT_LOG(rv == GFMRV_OK, rv, pCtx->pLog);
    
    // Cache the backbuffer's output dimensions
    rv = gfmWindow_getDimensions(&wndWidth, &wndHeight, pCtx->pWindow);
    ASSERT_LOG(rv == GFMRV_OK, rv, pCtx->pLog);
    rv = gfmBackbuffer_cacheDimensions(pCtx->pBackbuffer, wndWidth, wndHeight);
    ASSERT_LOG(rv == GFMRV_OK, rv, pCtx->pLog);
    
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
    // Check that the lib was initialized
    ASSERT(pCtx->pLog, GFMRV_NOT_INITIALIZED);
    // Check that the window has already been initialized
    ASSERT_LOG(pCtx->pWindow && gfmWindow_wasInit(pCtx->pWindow) == GFMRV_TRUE,
            GFMRV_WINDOW_NOT_INITIALIZED, pCtx->pLog);
    
    // Set the window's resolution
    rv = gfmWindow_setResolution(pCtx->pWindow, resIndex);
    ASSERT_LOG(rv == GFMRV_OK, rv, pCtx->pLog);
    
    // Cache the backbuffer's output dimensions
    rv = gfmWindow_getDimensions(&wndWidth, &wndHeight, pCtx->pWindow);
    ASSERT_LOG(rv == GFMRV_OK, rv, pCtx->pLog);
    rv = gfmBackbuffer_cacheDimensions(pCtx->pBackbuffer, wndWidth, wndHeight);
    ASSERT_LOG(rv == GFMRV_OK, rv, pCtx->pLog);
    
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
    // Check that the lib was initialized
    ASSERT(pCtx->pLog, GFMRV_NOT_INITIALIZED);
    // Check that the backbuffer was initialized
    ASSERT_LOG(pCtx->pBackbuffer, GFMRV_BACKBUFFER_NOT_INITIALIZED, pCtx->pLog);
    
    // Set the bg color
    rv = gfmBackbuffer_setBackground(pCtx->pBackbuffer, color);
    ASSERT_LOG(rv == GFMRV_OK, rv, pCtx->pLog);
    
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
    int incRate, height, width;
    
    // Sanitize arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    // Check that the lib was initialized
    ASSERT(pCtx->pLog, GFMRV_NOT_INITIALIZED);
    // Continue to sanitize arguments
    ASSERT_LOG(pIndex, GFMRV_ARGUMENTS_BAD, pCtx->pLog);
    ASSERT_LOG(pFilename, GFMRV_ARGUMENTS_BAD, pCtx->pLog);
    ASSERT_LOG(filenameLen, GFMRV_ARGUMENTS_BAD, pCtx->pLog);
    
    rv = gfmLog_log(pCtx->pLog, gfmLog_info, "Loading texture \"%*s\"",
            filenameLen, pFilename);
    ASSERT_NR(rv == GFMRV_OK);
    
    // Try to get a new texture
    incRate = 1;
    // This macro already ASSERT errors
    gfmGenArr_getNextRef(gfmTexture, pCtx->pTextures, incRate, pTex, gfmTexture_getNew);
    
    // Load the texture
    rv = gfmTexture_load(pTex, pCtx, pFilename, filenameLen, colorKey);
    ASSERT_LOG(rv == GFMRV_OK, rv, pCtx->pLog);
    
    // Get the texture's index
    *pIndex = gfmGenArr_getUsed(pCtx->pTextures);
    // Push the texture into the array
    gfmGenArr_push(pCtx->pTextures);
    
    // Get the texture's dimensions
    rv = gfmTexture_getDimensions(&width, &height, pTex);
    ASSERT_LOG(rv == GFMRV_OK, rv, pCtx->pLog);
    
    rv = gfmLog_log(pCtx->pLog, gfmLog_info, "Texture \"%*s\" loaded (w=%i, "
            "h=%i) at index %i!", filenameLen, pFilename, width, height,
            *pIndex);
    ASSERT_NR(rv == GFMRV_OK);
    
    
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
    // Check that the lib was initialized
    ASSERT(pCtx->pLog, GFMRV_NOT_INITIALIZED);
    // Continue to sanitize arguments
    ASSERT_LOG(ppTex, GFMRV_ARGUMENTS_BAD, pCtx->pLog);
    ASSERT_LOG(index >= 0, GFMRV_ARGUMENTS_BAD, pCtx->pLog);
    // Check that the texture exists
    ASSERT_LOG(index < gfmGenArr_getUsed(pCtx->pTextures), GFMRV_INVALID_INDEX, pCtx->pLog);
    
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
    // Check that the lib was initialized
    ASSERT(pCtx->pLog, GFMRV_NOT_INITIALIZED);
    // Continue to sanitize arguments
    ASSERT_LOG(ppSset, GFMRV_ARGUMENTS_BAD, pCtx->pLog);
    
    // Initialize so it can ben cleaned on error
    pSset = 0;
    
    // Try to get a new spriteset
    incRate = 1;
    // This macro already ASSERT errors
    gfmGenArr_getNextRef(gfmSpriteset, pCtx->pSpritesets, incRate, pSset,
            gfmSpriteset_getNew);
    
    // Initialize it
    rv = gfmSpriteset_init(pSset, pTex, tileWidth, tileHeight);
    ASSERT_LOG(rv == GFMRV_OK, rv, pCtx->pLog);
    
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
    // Check that the lib was initialized
    ASSERT(pCtx->pLog, GFMRV_NOT_INITIALIZED);
    // Continue to sanitize arguments
    ASSERT_LOG(ppSset, GFMRV_ARGUMENTS_BAD, pCtx->pLog);
    
    // Initialize so it can ben cleaned on error
    pSset = 0;
    
    rv = gfmLog_log(pCtx->pLog, gfmLog_info, "Creating %ix%i spriteset for "
            "texture %i", tileWidth, tileHeight, index);
    ASSERT_NR(rv == GFMRV_OK);
    
    // Try to get a new spriteset
    incRate = 1;
    // This macro already ASSERT errors
    gfmGenArr_getNextRef(gfmSpriteset, pCtx->pSpritesets, incRate, pSset,
            gfmSpriteset_getNew);
    // Initialize it
    rv = gfmSpriteset_initCached(pSset, pCtx, index, tileWidth, tileHeight);
    ASSERT_LOG(rv == GFMRV_OK, rv, pCtx->pLog);
    
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
    // Check that the lib was initialized
    ASSERT(pCtx->pLog, GFMRV_NOT_INITIALIZED);
    // Continue to sanitize arguments
    ASSERT_LOG(index >= 0, GFMRV_ARGUMENTS_BAD, pCtx->pLog);
    // Check that the texture exists
    ASSERT_LOG(index < gfmGenArr_getUsed(pCtx->pTextures), GFMRV_INVALID_INDEX, pCtx->pLog);
    
    // Cache the default texture
    pCtx->defaultTexture = index;
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Loads an audio
 * 
 * @param  pHandle     Handle of the loaded audio
 * @param  pCtx        The game's context
 * @param  pFilename   The filename
 * @param  filenameLen Length of the filename
 * @return             GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_COULDNT_OPEN_FILE,
 *                     GFMRV_READ_ERROR, GFMRV_AUDIO_FILE_NOT_SUPPORTED,
 *                     GFMRV_ALLOC_FAILED, GFMRV_INTERNAL_ERROR
 */
gfmRV gfm_loadAudio(int *pHandle, gfmCtx *pCtx, char *pFilename, int filenameLen) {
    gfmRV rv;
    
    // Sanitize arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    // Check that the lib was initialized
    ASSERT(pCtx->pLog, GFMRV_NOT_INITIALIZED);
    // Continue to sanitize arguments
    ASSERT_LOG(pHandle, GFMRV_ARGUMENTS_BAD, pCtx->pLog);
    ASSERT_LOG(pFilename, GFMRV_ARGUMENTS_BAD, pCtx->pLog);
    ASSERT_LOG(filenameLen > 0, GFMRV_ARGUMENTS_BAD, pCtx->pLog);
    // Check that audio is enabled
    if (pCtx->isAudioEnabled != 1) {
        rv = GFMRV_OK;
        goto __ret;
    }
    
    rv = gfmLog_log(pCtx->pLog, gfmLog_info, "Loading audio \"%*s\"...",
            filenameLen, pFilename);
    ASSERT_NR(rv == GFMRV_OK);
    
    // Try to load the audio
    rv = gfmAudio_loadAudio(pHandle, pCtx->pAudio, pCtx, pFilename,
            filenameLen);
    ASSERT_LOG(rv == GFMRV_OK, rv, pCtx->pLog);
    
    rv = gfmLog_log(pCtx->pLog, gfmLog_info, "Audio \"%*s\" loaded as handle "
            "%i!", filenameLen, pFilename, *pHandle);
    ASSERT_NR(rv == GFMRV_OK);
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Retrieve the current camera
 * 
 * @param  ppCam The camera
 * @param  pCtx  The game's context
 * @return       GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_CAMERA_NOT_INITIALIZED
 */
gfmRV gfm_getCamera(gfmCamera **ppCam, gfmCtx *pCtx) {
    gfmRV rv;
    
    // Sanitize arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    // Check that the lib was initialized
    ASSERT(pCtx->pLog, GFMRV_NOT_INITIALIZED);
    // Continue to sanitize arguments
    ASSERT_LOG(ppCam, GFMRV_ARGUMENTS_BAD, pCtx->pLog);
    // Check that the camera was initialized
    ASSERT_LOG(pCtx->pCamera, GFMRV_CAMERA_NOT_INITIALIZED, pCtx->pLog);
    
    // Retrieve the camera
    *ppCam = pCtx->pCamera;
    
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
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    // Check that the lib was initialized
    ASSERT(pCtx->pLog, GFMRV_NOT_INITIALIZED);
    // Continue to sanitize arguments
    ASSERT_LOG(pX, GFMRV_ARGUMENTS_BAD, pCtx->pLog);
    ASSERT_LOG(pY, GFMRV_ARGUMENTS_BAD, pCtx->pLog);
    // Check that the camera was initialized
    ASSERT_LOG(pCtx->pCamera, GFMRV_CAMERA_NOT_INITIALIZED, pCtx->pLog);
    
    // Get the position
    rv = gfmCamera_getPosition(pX, pY, pCtx->pCamera);
    ASSERT_LOG(rv == GFMRV_OK, rv, pCtx->pLog);
    
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
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    // Check that the lib was initialized
    ASSERT(pCtx->pLog, GFMRV_NOT_INITIALIZED);
    // Continue to sanitize arguments
    ASSERT_LOG(pWidth, GFMRV_ARGUMENTS_BAD, pCtx->pLog);
    ASSERT_LOG(pHeight, GFMRV_ARGUMENTS_BAD, pCtx->pLog);
    // Check that the camera was initialized
    ASSERT_LOG(pCtx->pCamera, GFMRV_CAMERA_NOT_INITIALIZED, pCtx->pLog);
    
    // Get the dimensions
    rv = gfmCamera_getDimensions(pWidth, pHeight, pCtx->pCamera);
    ASSERT_LOG(rv == GFMRV_OK, rv, pCtx->pLog);
    
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
    // Check that the lib was initialized
    ASSERT(pCtx->pLog, GFMRV_NOT_INITIALIZED);
    // Continue to sanitize arguments
    ASSERT_LOG(pSpr, GFMRV_ARGUMENTS_BAD, pCtx->pLog);
    // Check that the camera was initialized
    ASSERT_LOG(pCtx->pCamera, GFMRV_CAMERA_NOT_INITIALIZED, pCtx->pLog);
    
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
    // Check that the lib was initialized
    ASSERT(pCtx->pLog, GFMRV_NOT_INITIALIZED);
    // Continue to sanitize arguments
    ASSERT_LOG(ups > 0, GFMRV_ARGUMENTS_BAD, pCtx->pLog);
    ASSERT_LOG(dps > 0, GFMRV_ARGUMENTS_BAD, pCtx->pLog);
    
    // Alloc the objects, if necessary
    if (!pCtx->pUpdateAcc) {
        rv = gfmAccumulator_getNew(&(pCtx->pUpdateAcc));
        ASSERT_LOG(rv == GFMRV_OK, rv, pCtx->pLog);
    }
    if (!pCtx->pDrawAcc) {
        rv = gfmAccumulator_getNew(&(pCtx->pDrawAcc));
        ASSERT_LOG(rv == GFMRV_OK, rv, pCtx->pLog);
    }
    
    // Set max Frames to avoid crash (and force slow down) on laggy parts
    maxFrames = ups / 10;
    if (maxFrames == 0)
        maxFrames = 1;
    // Initialize both accumulators
    rv = gfmAccumulator_setFPS(pCtx->pUpdateAcc, ups, maxFrames);
    ASSERT_LOG(rv == GFMRV_OK, rv, pCtx->pLog);
    // Accumulating various draw frames make no sense, so force maxFrames to 1
    maxFrames = 1;
    gfmAccumulator_setFPS(pCtx->pDrawAcc, dps, maxFrames);
    ASSERT_LOG(rv == GFMRV_OK, rv, pCtx->pLog);
    
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
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    // Check that the lib was initialized
    ASSERT(pCtx->pLog, GFMRV_NOT_INITIALIZED);
    // Continue to sanitize arguments
    ASSERT_LOG(pUps, GFMRV_ARGUMENTS_BAD, pCtx->pLog);
    ASSERT_LOG(pDps, GFMRV_ARGUMENTS_BAD, pCtx->pLog);
    // Check that it was initialized
    ASSERT_LOG(pCtx->pUpdateAcc, GFMRV_ACC_NOT_INITIALIZED, pCtx->pLog);
    
    rv = gfmAccumulator_getFPS(pUps, pCtx->pUpdateAcc);
    ASSERT_LOG(rv == GFMRV_OK, rv, pCtx->pLog);
    rv = gfmAccumulator_getFPS(pDps, pCtx->pDrawAcc);
    ASSERT_LOG(rv == GFMRV_OK, rv, pCtx->pLog);
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Check if there are any frames left and updates the inputs
 * 
 * @param  pCtx The game's context
 * @return      GFMRV_ARGUMENTS_BAD, GFMRV_FALSE, GFMRV_TRUE
 */
gfmRV gfm_isUpdating(gfmCtx *pCtx) {
    gfmRV rv;
    
    // Sanitize arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    // Check that the lib was initialized
    ASSERT(pCtx->pLog, GFMRV_NOT_INITIALIZED);
    // Check if there are any updates left
    if (pCtx->updateFrames <= 0) {
        rv = GFMRV_FALSE;
        goto __ret;
    }
    
    // Remove a frame
    pCtx->updateFrames--;
    
    // Handle any events that happened since the start of the frame
    rv = gfmEvent_processQueued(pCtx->pEvent, pCtx);
    ASSERT_LOG(rv == GFMRV_OK, rv, pCtx->pLog);
    // Update every input
    rv = gfmInput_update(pCtx->pInput);
    ASSERT_LOG(rv == GFMRV_OK, rv, pCtx->pLog);
    
    rv = GFMRV_TRUE;
__ret:
    return rv;
}

/**
 * Check if there are any frames left to be drawn
 * 
 * @param  pCtx The game's context
 * @return      GFMRV_ARGUMENTS_BAD, GFMRV_FALSE, GFMRV_TRUE
 */
gfmRV gfm_isDrawing(gfmCtx *pCtx) {
    gfmRV rv;
    
    // Sanitize arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    // Check that the lib was initialized
    ASSERT(pCtx->pLog, GFMRV_NOT_INITIALIZED);
    // Check if there are any updates left
    if (pCtx->drawFrames <= 0) {
        rv = GFMRV_FALSE;
        goto __ret;
    }
    
    // Remove a frame
    pCtx->drawFrames--;
    
    rv = GFMRV_TRUE;
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
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    // Check that the lib was initialized
    ASSERT(pCtx->pLog, GFMRV_NOT_INITIALIZED);
    // Continue to sanitize arguments
    ASSERT_LOG(pElapsed, GFMRV_ARGUMENTS_BAD, pCtx->pLog);
    // Check that the accumulator was initialized
    ASSERT_LOG(pCtx->pUpdateAcc, GFMRV_ACC_NOT_INITIALIZED, pCtx->pLog);
    
    // Retrieve the elapsed time
    rv = gfmAccumulator_getDelay(pElapsed, pCtx->pUpdateAcc);
    ASSERT_LOG(rv == GFMRV_OK, rv, pCtx->pLog);
    
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
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    // Check that the lib was initialized
    ASSERT(pCtx->pLog, GFMRV_NOT_INITIALIZED);
    // Continue to sanitize arguments
    ASSERT_LOG(pElapsed, GFMRV_ARGUMENTS_BAD, pCtx->pLog);
    // Check that the accumulator was initialized
    ASSERT_LOG(pCtx->pUpdateAcc, GFMRV_ACC_NOT_INITIALIZED, pCtx->pLog);
    
    // Retrieve the elapsed time
    rv = gfmAccumulator_getDelay(&delay, pCtx->pUpdateAcc);
    ASSERT_LOG(rv == GFMRV_OK, rv, pCtx->pLog);
    
    *pElapsed = (float)delay / 1000.0f;
    
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
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    // Check that the lib was initialized
    ASSERT(pCtx->pLog, GFMRV_NOT_INITIALIZED);
    // Continue to sanitize arguments
    ASSERT_LOG(pElapsed, GFMRV_ARGUMENTS_BAD, pCtx->pLog);
    // Check that the accumulator was initialized
    ASSERT_LOG(pCtx->pUpdateAcc, GFMRV_ACC_NOT_INITIALIZED, pCtx->pLog);
    
    // Retrieve the elapsed time
    rv = gfmAccumulator_getDelay(&delay, pCtx->pUpdateAcc);
    ASSERT_LOG(rv == GFMRV_OK, rv, pCtx->pLog);
    
    *pElapsed = (double)delay / 1000.0;
    
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
    // Check that the lib was initialized
    ASSERT(pCtx->pLog, GFMRV_NOT_INITIALIZED);
    // Continue to sanitize arguments
    ASSERT_LOG(ms > 0, GFMRV_ARGUMENTS_BAD, pCtx->pLog);
    // Check that the accumulators was initialized
    ASSERT_LOG(pCtx->pUpdateAcc, GFMRV_ACC_NOT_INITIALIZED, pCtx->pLog);
    ASSERT_LOG(pCtx->pDrawAcc, GFMRV_ACC_NOT_INITIALIZED, pCtx->pLog);
    
    // Update the accumulators
    rv = gfmAccumulator_update(pCtx->pUpdateAcc, ms);
    ASSERT_LOG(rv == GFMRV_OK, rv, pCtx->pLog);
    rv = gfmAccumulator_update(pCtx->pDrawAcc, ms);
    ASSERT_LOG(rv == GFMRV_OK, rv, pCtx->pLog);
    
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
    // Check that the lib was initialized
    ASSERT(pCtx->pLog, GFMRV_NOT_INITIALIZED);
    
    // Wait for the first event and process everything
    rv = gfmEvent_waitEvent(pCtx->pEvent);
    ASSERT_LOG(rv == GFMRV_OK, rv, pCtx->pLog);
    rv = gfmEvent_processQueued(pCtx->pEvent, pCtx);
    ASSERT_LOG(rv == GFMRV_OK, rv, pCtx->pLog);
    
    // Update the frames to be updated
    rv = gfmAccumulator_getFrames(&(pCtx->updateFrames), pCtx->pUpdateAcc);
    ASSERT_LOG(rv == GFMRV_OK, rv, pCtx->pLog);
    // Update the frames to be drawn
    rv = gfmAccumulator_getFrames(&(pCtx->drawFrames), pCtx->pDrawAcc);
    ASSERT_LOG(rv == GFMRV_OK, rv, pCtx->pLog);
    
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
    // Check that the lib was initialized
    ASSERT(pCtx->pLog, GFMRV_NOT_INITIALIZED);
    // Continue to sanitize arguments
    ASSERT_LOG(pSset, GFMRV_ARGUMENTS_BAD, pCtx->pLog);
    ASSERT_LOG(firstTile >= 0, GFMRV_ARGUMENTS_BAD, pCtx->pLog);
    
    // Initialize the FPS counter
    rv = gfmFPSCounter_init(pCtx->pCounter, pSset, firstTile);
    ASSERT_LOG(rv == GFMRV_OK, rv, pCtx->pLog);
    
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
    // Check that the lib was initialized
    ASSERT(pCtx->pLog, GFMRV_NOT_INITIALIZED);
#if defined(DEBUG) || defined(FORCE_FPS)
    // Check that it was initialized, on debug mode
    ASSERT_LOG(pCtx->pCounter, GFMRV_FPSCOUNTER_NOT_INITIALIZED, pCtx->pLog);
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
    // Check that the lib was initialized
    ASSERT(pCtx->pLog, GFMRV_NOT_INITIALIZED);
#if defined(DEBUG) || defined(FORCE_FPS)
    // Check that it was initialized, on debug mode
    ASSERT_LOG(pCtx->pCounter, GFMRV_FPSCOUNTER_NOT_INITIALIZED, pCtx->pLog);
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
    // Check that the lib was initialized
    ASSERT(pCtx->pLog, GFMRV_NOT_INITIALIZED);
    // Check that it was initialized
    ASSERT_LOG(pCtx->pCounter, GFMRV_FPSCOUNTER_NOT_INITIALIZED, pCtx->pLog);
    
    rv = gfmFPSCounter_updateBegin(pCtx->pCounter);
    ASSERT_LOG(rv == GFMRV_OK, rv, pCtx->pLog);
    
    rv = GFMRV_OK;
__ret:
#endif
    return rv;
}

/**
 * Adds a new virtual key to the game's context
 * 
 * @param  pHandle Handle to the action
 * @param  pCtx    The game's context
 * @return         GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_ALLOC_FAILED
 */
gfmRV gfm_addVirtualKey(int *pHandle, gfmCtx *pCtx) {
    gfmRV rv;
    
    // Sanitize arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    // Check that the lib was initialized
    ASSERT(pCtx->pLog, GFMRV_NOT_INITIALIZED);
    // Continue to sanitize arguments
    ASSERT_LOG(pHandle, GFMRV_ARGUMENTS_BAD, pCtx->pLog);
    // Check that the input system was initialized
    ASSERT_LOG(pCtx->pInput, GFMRV_INPUT_NOT_INITIALIZED, pCtx->pLog);
    
    // Add a new virtual key to the input
    rv = gfmInput_addVirtualKey(pHandle, pCtx->pInput);
    ASSERT_LOG(rv == GFMRV_OK, rv, pCtx->pLog);
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Bind a key/button to an action
 * 
 * @param  pCtx   The game's context
 * @param  handle The action's handle
 * @param  key    The key/button
 * @return        GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_INPUT_INVALID_HANDLE,
 *                GFMRV_INPUT_ALREADY_BOUND
 */
gfmRV gfm_bindInput(gfmCtx *pCtx, int handle, gfmInputIface key) {
    gfmRV rv;
    
    // Sanitize arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    // Check that the lib was initialized
    ASSERT(pCtx->pLog, GFMRV_NOT_INITIALIZED);
    // Everything else will be checked inside
    // Check that the input system was initialized
    ASSERT_LOG(pCtx->pInput, GFMRV_INPUT_NOT_INITIALIZED, pCtx->pLog);
    
    // Bind the new input
    rv = gfmInput_bind(pCtx->pInput, handle, key);
    ASSERT_LOG(rv == GFMRV_OK, rv, pCtx->pLog);
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Reset all bindings from the input
 * 
 * @param  pCtx   The game's context
 * @return        GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfm_resetInput(gfmCtx *pCtx) {
    gfmRV rv;
    
    // Sanitize arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    // Check that the lib was initialized
    ASSERT(pCtx->pLog, GFMRV_NOT_INITIALIZED);
    // Check that the input system was initialized
    ASSERT_LOG(pCtx->pInput, GFMRV_INPUT_NOT_INITIALIZED, pCtx->pLog);
    
    rv = gfmInput_reset(pCtx->pInput);
    ASSERT_LOG(rv == GFMRV_OK, rv, pCtx->pLog);
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Retrieves a virtual key state
 * 
 * @param  pState The current state
 * @param  pNum   How many consecutive times the key has been pressed
 * @param  pCtx   The game's context
 * @param  handle The action's handle
 * @param        GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_INPUT_INVALID_HANDLE
 */
gfmRV gfm_getKeyState(gfmInputState *pState, int *pNum, gfmCtx *pCtx,
        int handle) {
    gfmRV rv;
    
    // Sanitize arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    // Check that the lib was initialized
    ASSERT(pCtx->pLog, GFMRV_NOT_INITIALIZED);
    // Everything else will be checked inside
    // Check that the input system was initialized
    ASSERT_LOG(pCtx->pInput, GFMRV_INPUT_NOT_INITIALIZED, pCtx->pLog);
    
    // Get the key state
    rv = gfmInput_getKeyState(pState, pNum, pCtx->pInput, handle);
    ASSERT_LOG(rv == GFMRV_OK, rv, pCtx->pLog);
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Get the last key/button/whatever pressed; This function doesn't block but,
 * unless it's ready, it will return GFMRV_WAITING; The 'key' pressed is only
 * valid when the function return GFMRV_OK
 * 
 * @param  pIface The last 'iface' pressed
 * @param  pCtx   The game's context
 * @return        GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_WAITING
 */
gfmRV gfm_getLastPressed(gfmInputIface *pIface, gfmCtx *pCtx) {
    gfmRV rv;
    
    // Sanitize arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    // Check that the lib was initialized
    ASSERT(pCtx->pLog, GFMRV_NOT_INITIALIZED);
    // Continue to sanitize arguments
    ASSERT_LOG(pIface, GFMRV_ARGUMENTS_BAD, pCtx->pLog);
    // Check that the input system was initialized
    ASSERT_LOG(pCtx->pInput, GFMRV_INPUT_NOT_INITIALIZED, pCtx->pLog);
    
    // Try to retrieve the last 'key' pressed
    rv = gfmInput_getLastPressed(pIface, pCtx->pInput);
    if (rv == GFMRV_OPERATION_NOT_ACTIVE) {
        // If the operation hadn't been initialized, initialize it
        rv =  gfmInput_requestLastPressed(pCtx->pInput);
        ASSERT_LOG(rv == GFMRV_OK, rv, pCtx->pLog);
        // Force the function to be run at last another time
        rv = GFMRV_WAITING;
    }
    ASSERT_LOG(rv == GFMRV_OK || rv == GFMRV_WAITING, rv, pCtx->pLog);
    
    // Return with either GFMRV_OK or GFMRV_WAITING (whichever was return by
    // the function
__ret:
    return rv;
}

/**
 * Retrieve the current input context
 * 
 * @param  ppInput The input context
 * @param  pCtx    The game's context
 * @return         GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfm_getInput(gfmInput **ppInput, gfmCtx *pCtx) {
    gfmRV rv;
    
    // Sanitize arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    // Check that the lib was initialized
    ASSERT(pCtx->pLog, GFMRV_NOT_INITIALIZED);
    // Continue to sanitize arguments
    ASSERT_LOG(ppInput, GFMRV_ARGUMENTS_BAD, pCtx->pLog);
    
    // Retrieve the context
    *ppInput = pCtx->pInput;
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Get the logger instance, so we can log elsewhere
 * 
 * @param  ppLog The logger
 * @param  pCtx  The game's context
 * @return       GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_NOT_INITIALIZED
 */
gfmRV gfm_getLogger(gfmLog **ppLog, gfmCtx *pCtx) {
    gfmRV rv;
    
    // Sanitize arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    // Check that the lib was initialized
    ASSERT(pCtx->pLog, GFMRV_NOT_INITIALIZED);
    // Continue to sanitize arguments
    ASSERT_LOG(ppLog, GFMRV_ARGUMENTS_BAD, pCtx->pLog);
    
    *ppLog = pCtx->pLog;
    
    rv = GFMRV_OK;
__ret:
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
    // Check that the lib was initialized
    ASSERT(pCtx->pLog, GFMRV_NOT_INITIALIZED);
    // Check that it was initialized
    ASSERT_LOG(pCtx->pCounter, GFMRV_FPSCOUNTER_NOT_INITIALIZED, pCtx->pLog);
    
    rv = gfmFPSCounter_updateEnd(pCtx->pCounter);
    ASSERT_LOG(rv == GFMRV_OK, rv, pCtx->pLog);
    
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
    // Check that the lib was initialized
    ASSERT(pCtx->pLog, GFMRV_NOT_INITIALIZED);
    // Continue to sanitize arguments
    ASSERT_LOG(pFilepath, GFMRV_ARGUMENTS_BAD, pCtx->pLog);
    ASSERT_LOG(len > 0, GFMRV_ARGUMENTS_BAD, pCtx->pLog);
    // Check that the operation is available
    ASSERT_LOG(gfmGif_isSupported() == GFMRV_TRUE, GFMRV_FUNCTION_NOT_SUPPORTED, pCtx->pLog);
    // Check that the operation isn't active
    ASSERT_LOG(!pCtx->takeSnapshot, GFMRV_OPERATION_ACTIVE, pCtx->pLog);
    
    // Create the GIF exporter, if needed)
    if (!pCtx->pGif) {
        rv = gfmGif_getNew(&(pCtx->pGif));
        ASSERT_LOG(rv == GFMRV_OK, rv, pCtx->pLog);
    }
    // Get the backbuffer's dimensions
    rv = gfmBackbuffer_getDimensions(&width, &height, pCtx->pBackbuffer);
    ASSERT_LOG(rv == GFMRV_OK, rv, pCtx->pLog);
    // Initialize the gif exporter to the current backbuffer
    rv = gfmGif_init(pCtx->pGif, pCtx, width, height);
    ASSERT_LOG(rv == GFMRV_OK, rv, pCtx->pLog);
    
    // Alloc as many bytes as required (or fail if not possible/supported)
    newLen = pCtx->ssDataLen;
    rv = gfmBackbuffer_getBackbufferData(0, (int*)&newLen, pCtx->pBackbuffer);
    ASSERT_LOG(rv == GFMRV_OK, rv, pCtx->pLog);
    
    // Expand the buffer, as necessary
    if (newLen > pCtx->ssDataLen) {
        pCtx->pSsData = (unsigned char*)realloc(pCtx->pSsData,
                newLen * sizeof(unsigned char));
        ASSERT_LOG(pCtx->pSsData, GFMRV_ALLOC_FAILED, pCtx->pLog);
        
        // Must store the new buffer len
        pCtx->ssDataLen = newLen;
    }
    
    // Create the path string, if necessary
    if (!pCtx->pSsPath) {
        rv = gfmString_getNew(&(pCtx->pSsPath));
        ASSERT_LOG(rv == GFMRV_OK, rv, pCtx->pLog);
    }
    
    // store the path
    if (useLocalPath) {
        char *pLocalPath;
        int doCopy;
        
        // Retrieve the local path from the save file path
        rv = gfmString_getString(&pLocalPath, pCtx->pSaveFilename);
        ASSERT_LOG(rv == GFMRV_OK, rv, pCtx->pLog);
        
        doCopy = 1;
        rv = gfmString_init(pCtx->pSsPath, pLocalPath, pCtx->saveFilenameLen,
                doCopy);
        ASSERT_LOG(rv == GFMRV_OK, rv, pCtx->pLog);
        
        // Append the file's path
        rv = gfmString_concat(pCtx->pSsPath, pFilepath, len);
        ASSERT_LOG(rv == GFMRV_OK, rv, pCtx->pLog);
    }
    else {
        int doCopy;
        
        // Create the string with the path
        doCopy = 1;
        rv = gfmString_init(pCtx->pSsPath, pFilepath, len, doCopy);
        ASSERT_LOG(rv == GFMRV_OK, rv, pCtx->pLog);
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
    
    // TODO Sanitize arguments
    
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
    // Check that the lib was initialized
    ASSERT(pCtx->pLog, GFMRV_NOT_INITIALIZED);
    // Check that the backbuffer was initialized
    ASSERT_LOG(pCtx->pBackbuffer, GFMRV_BACKBUFFER_NOT_INITIALIZED, pCtx->pLog);
    
#if defined(DEBUG) || defined(FORCE_FPS)
    // Store when drawing was initialized
    rv = gfmFPSCounter_initDraw(pCtx->pCounter);
    ASSERT_LOG(rv == GFMRV_OK, rv, pCtx->pLog);
#endif
    
    // If there's a default texture, load it
    if (pCtx->defaultTexture >= 0) {
        rv = gfm_drawLoadCachedTexture(pCtx, pCtx->defaultTexture);
        ASSERT_LOG(rv == GFMRV_OK, rv, pCtx->pLog);
    }
    
    rv = gfmBackbuffer_drawBegin(pCtx->pBackbuffer);
    ASSERT_LOG(rv == GFMRV_OK, rv, pCtx->pLog);
    
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
    // Check that the lib was initialized
    ASSERT(pCtx->pLog, GFMRV_NOT_INITIALIZED);
    // Continue to sanitize arguments
    ASSERT_LOG(iTex >= 0, GFMRV_ARGUMENTS_BAD, pCtx->pLog);
    // Check that the texture exists
    ASSERT_LOG(iTex < gfmGenArr_getUsed(pCtx->pTextures), GFMRV_INVALID_INDEX, pCtx->pLog);
    
    // Load the texture
    rv = gfm_drawLoadTexture(pCtx, gfmGenArr_getObject(pCtx->pTextures, iTex));
    ASSERT_LOG(rv == GFMRV_OK, rv, pCtx->pLog);
    
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
    // Check that the lib was initialized
    ASSERT(pCtx->pLog, GFMRV_NOT_INITIALIZED);
    // Continue to sanitize arguments
    ASSERT_LOG(pTex, GFMRV_ARGUMENTS_BAD, pCtx->pLog);
    // Check that the backbuffer was initialized
    ASSERT_LOG(pCtx->pBackbuffer, GFMRV_BACKBUFFER_NOT_INITIALIZED, pCtx->pLog);
    
    // Load it into the backbuffer
    rv = gfmBackbuffer_drawLoadTexture(pCtx->pBackbuffer, pTex);
    ASSERT_LOG(rv == GFMRV_OK, rv, pCtx->pLog);
    
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
 * @param  pCtx      The game's context
 * @param  pSSet     The spriteset containing the tile
 * @param  x         Horizontal position in screen space
 * @param  y         Vertical position in screen space
 * @param  tile      Tile to be rendered
 * @param  isFlipped Whether the tile should be drawn flipped
 * @return           GFMRV_OK, GFMRV_ARGUMENTS_BAD,
 *                   GFMRV_BACKBUFFER_NOT_INITIALIZED,
 */
gfmRV gfm_drawTile(gfmCtx *pCtx, gfmSpriteset *pSset, int x, int y, int tile,
        int isFlipped) {
    gfmRV rv;
    
    // Sanitize arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    // Check that the lib was initialized
    ASSERT(pCtx->pLog, GFMRV_NOT_INITIALIZED);
    // Continue to sanitize arguments
    ASSERT_LOG(pSset, GFMRV_ARGUMENTS_BAD, pCtx->pLog);
    // Check that the backbuffer was initialized
    ASSERT_LOG(pCtx->pBackbuffer, GFMRV_BACKBUFFER_NOT_INITIALIZED, pCtx->pLog);
    // Check that the tile can be rendered
    if (tile < 0) {
        rv = GFMRV_OK;
        goto __ret;
    }
    
    // Render the tile
    rv = gfmBackbuffer_drawTile(pCtx->pBackbuffer, pSset, x, y, tile,
            isFlipped);
    ASSERT_LOG(rv == GFMRV_OK, rv, pCtx->pLog);
    
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
    // Check that the lib was initialized
    ASSERT(pCtx->pLog, GFMRV_NOT_INITIALIZED);
    // Continue to sanitize arguments
    ASSERT_LOG(pSset, GFMRV_ARGUMENTS_BAD, pCtx->pLog);
    // Check that the backbuffer was initialized
    ASSERT_LOG(pCtx->pBackbuffer, GFMRV_BACKBUFFER_NOT_INITIALIZED, pCtx->pLog);
    
    // Get the spriteset dimensions
    rv = gfmSpriteset_getDimension(&tileWidth, &tileHeight, pSset);
    ASSERT_LOG(rv == GFMRV_OK, rv, pCtx->pLog);
    
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
        rv = gfmBackbuffer_drawTile(pCtx->pBackbuffer, pSset, x, y, tile,
                0/*flipped*/);
        ASSERT_LOG(rv == GFMRV_OK, rv, pCtx->pLog);
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
        rv = gfmBackbuffer_drawTile(pCtx->pBackbuffer, pSset, x, y, tile,
                0/*flipped*/);
        ASSERT_LOG(rv == GFMRV_OK, rv, pCtx->pLog);
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
 * Renders a rectangle (only its vertices);
 * NOTE: This function isn't guaranteed to be fast, so use it wisely
 * 
 * @param  pCtx   The game's context
 * @param  x      Top-left position, in world-space
 * @param  y      Top-left position, in world-space
 * @param  width  Rectangle's width
 * @param  height Rectangle's height
 * @param  red    Color's red component
 * @param  green  Color's green component
 * @param  blue   Color's blue component
 * @return        GFMRV_OK, GFMRV_ARGUMENTS_BAD,
 *                GFMRV_BACKBUFFER_NOT_INITIALIZED
 */
gfmRV gfm_drawRect(gfmCtx *pCtx, int x, int y, int width, int height,
        unsigned char red, unsigned char green, unsigned char blue) {
    gfmRV rv;
    int camX, camY;
    
    // Sanitize arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    // Check that the lib was initialized
    ASSERT(pCtx->pLog, GFMRV_NOT_INITIALIZED);
    // Continue to sanitize arguments
    ASSERT_LOG(width > 0, GFMRV_ARGUMENTS_BAD, pCtx->pLog);
    ASSERT_LOG(height > 0, GFMRV_ARGUMENTS_BAD, pCtx->pLog);
    // Check that the backbuffer was initialized
    ASSERT_LOG(pCtx->pBackbuffer, GFMRV_BACKBUFFER_NOT_INITIALIZED, pCtx->pLog);
    
    // Get the camera's position
    rv = gfm_getCameraPosition(&camX, &camY, pCtx);
    ASSERT_LOG(rv == GFMRV_OK, rv, pCtx->pLog);
    
    // Convert the position from world-space to screen-space
    x -= camX;
    y -= camY;
    
    // Draw the rectangle
    rv = gfmBackbuffer_drawRect(pCtx->pBackbuffer, x, y, width, height, red,
            green, blue);
    ASSERT_LOG(rv == GFMRV_OK, rv, pCtx->pLog);
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

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
    // Check that the lib was initialized
    ASSERT(pCtx->pLog, GFMRV_NOT_INITIALIZED);
    // Check that the backbuffer was initialized
    ASSERT_LOG(pCtx->pBackbuffer, GFMRV_BACKBUFFER_NOT_INITIALIZED, pCtx->pLog);
    
#if defined(DEBUG) || defined(FORCE_FPS)
    // Display the current fps
    if (pCtx->showFPS) {
        rv = gfmFPSCounter_draw(pCtx->pCounter, pCtx);
        ASSERT_LOG(rv == GFMRV_OK, rv, pCtx->pLog);
    }
#endif
    
    rv = gfmBackbuffer_drawEnd(pCtx->pBackbuffer, pCtx->pWindow);
    ASSERT_LOG(rv == GFMRV_OK, rv, pCtx->pLog);
    
    // If requested, take the snapshot
    if (pCtx->takeSnapshot) {
        volatile int len;
        
        // Retrieve the data
        len = pCtx->ssDataLen;
        rv = gfmBackbuffer_getBackbufferData(pCtx->pSsData, (int*)&len,
                pCtx->pBackbuffer);
        ASSERT_LOG(rv == GFMRV_OK, rv, pCtx->pLog);
        
        // Store it in a GIF image
        rv = gfmGif_storeFrame(pCtx->pGif, pCtx->pSsData, len);
        ASSERT_LOG(rv == GFMRV_OK, rv, pCtx->pLog);
        
        if (!pCtx->isAnimation) {
            // If it's a snapshot, simply save the animation
            rv = gfmGif_exportImage(pCtx->pGif, pCtx->pSsPath);
            ASSERT_LOG(rv == GFMRV_OK, rv, pCtx->pLog);
        
            pCtx->takeSnapshot = 0;
        }
        else {
            int delay;
            
            // Update the animation timer
            rv = gfmAccumulator_getDelay(&delay, pCtx->pDrawAcc);
            ASSERT_LOG(rv == GFMRV_OK, rv, pCtx->pLog);
            
            pCtx->animationTime -= delay;
            
            // If enough frames were recorded, export it
            if (pCtx->animationTime <= 0) {
                rv = gfmGif_exportAnimation(pCtx->pGif, pCtx->pSsPath);
                ASSERT_LOG(rv == GFMRV_OK, rv, pCtx->pLog);
                
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
 * Issue a new frame; Should only be used on singled threaded environments
 * 
 * @param  pCtx  The game's context
 * @return       GFMRV_OK, GFMRV_ARGUMENTS_BAD, ...
 */
gfmRV gfm_issueFrame(gfmCtx *pCtx) {
    gfmRV rv;
    
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    ASSERT(pCtx->pTimer, GFMRV_TIMER_NOT_INITIALIZED);
    
    rv = gfmTimer_issue(pCtx->pTimer);
__ret:
    return rv;
}

/**
 * Wait for a new frame; Should only be used on singled threaded environments
 * 
 * @param  pCtx  The game's context
 * @return       GFMRV_OK, GFMRV_ARGUMENTS_BAD, ...
 */
gfmRV gfm_waitFrame(gfmCtx *pCtx) {
    gfmRV rv;
    
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    ASSERT(pCtx->pTimer, GFMRV_TIMER_NOT_INITIALIZED);
    
    rv = gfmTimer_wait(pCtx->pTimer);
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
    
    if (pCtx->pLog) {
        gfmLog_log(pCtx->pLog, gfmLog_info, "Finalizing GFraMe...");
    }
    
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
    gfmInput_free(&(pCtx->pInput));
    gfmGif_free(&(pCtx->pGif));
    if (pCtx->pSsData) {
        free(pCtx->pSsData);
        pCtx->pSsData = 0;
    }
    gfmString_free(&(pCtx->pSsPath));
    gfmAudio_free(&(pCtx->pAudio));
    
    if (pCtx->pLog) {
        gfmLog_log(pCtx->pLog, gfmLog_info, "GFraMe finalized!");
        gfmLog_log(pCtx->pLog, gfmLog_info, "-----------------------------"
                "---------------------------------------------------");
        gfmLog_log(pCtx->pLog, gfmLog_info, "");
    }
    
    gfmLog_free(&(pCtx->pLog));
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

