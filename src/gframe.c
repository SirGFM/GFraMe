/**
 * @src/gframe.c
 */
#include <GFraMe/gframe.h>
#include <GFraMe/gfmAssert.h>
#include <GFraMe/gfmError.h>
#include <GFraMe/gfmString.h>
#include <GFraMe/core/gfmBackend_bkend.h>
#include <GFraMe/core/gfmTimer_bkend.h>
#include <GFraMe/core/gfmPath_bkend.h>
#include <GFraMe/core/gfmWindow_bkend.h>

#include <stdlib.h>

struct stGFMCtx {
    // TODO specify backend(?)
    /** "Organization" name; It's used as part of paths. */
    gfmString *pGameOrg;
    /** Game's title; It's used as part of paths. */
    gfmString *pGameTitle;
#ifndef GFRAME_MOBILE
    /** Directory where the game binary is being run from */
    gfmString *pBinPath;
#endif
    /** Buffer for storing a save file's filename */
    gfmString *pSaveFilename;
    /** Length until the end of the filename's path */
    int saveFilenameLen;
    /** Whether the backend was initialized */
    int isBackendInit;
    /** Game's window */
    gfmWindow *pWindow;
    /** Timer used to issue new frames */
    gfmTimer *pTimer;
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
    (*ppCtx)->pGameOrg = 0;
    (*ppCtx)->pGameTitle = 0;
#ifndef GFRAME_MOBILE
    (*ppCtx)->pBinPath = 0;
#endif
    (*ppCtx)->pSaveFilename = 0;
    (*ppCtx)->pTimer = 0;
    (*ppCtx)->isBackendInit = 0;
    (*ppCtx)->pWindow = 0;
    
#ifndef GFRAME_MOBILE
	// Get current directory
    rv = gfmPath_getRunningPath(&((*ppCtx)->pBinPath));
    ASSERT_NR(rv == GFMRV_OK);
#endif
    
    // Init the current backend
    // TODO allow more than one backend?
    rv = gfmBackend_init();
    ASSERT_NR(rv == GFMRV_OK);
    // Set the backend as initialized
    (*ppCtx)->isBackendInit = 1;
    
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
    
    // TODO init backbuffer
    
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
 *                GFMRV_WINDOW_NOT_INITIALIZED, GFMRV_WINDOW_IS_FULLSCREEN
 */
gfmRV gfm_setDimensions(gfmCtx *pCtx, int width, int height) {
    gfmRV rv;
    
    // Sanitize the arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    // Check that the window has already been initialized
    ASSERT(pCtx->pWindow && gfmWindow_wasInit(pCtx->pWindow) == GFMRV_TRUE, GFMRV_WINDOW_NOT_INITIALIZED);
    
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
    
    // Sanitize the arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    // Check that the window has already been initialized
    ASSERT(pCtx->pWindow && gfmWindow_wasInit(pCtx->pWindow) == GFMRV_TRUE, GFMRV_WINDOW_NOT_INITIALIZED);
    
    // Try to make it fullscreen
    rv = gfmWindow_setFullScreen(pCtx->pWindow);
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
    
    // Sanitize the arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    // Check that the window has already been initialized
    ASSERT(pCtx->pWindow && gfmWindow_wasInit(pCtx->pWindow) == GFMRV_TRUE, GFMRV_WINDOW_NOT_INITIALIZED);
    
    // Try to make it fullscreen
    rv = gfmWindow_setWindowed(pCtx->pWindow);
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
    
    // Sanitize the arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    // Check that the window has already been initialized
    ASSERT(pCtx->pWindow && gfmWindow_wasInit(pCtx->pWindow) == GFMRV_TRUE, GFMRV_WINDOW_NOT_INITIALIZED);
    
    // Set the window's resolution
    rv = gfmWindow_setResolution(pCtx->pWindow, resIndex);
    ASSERT_NR(rv == GFMRV_OK);
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

gfmRV gfm_initAll() {
    return GFMRV_FUNCTION_NOT_SUPPORTED;
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
    gfmWindow_free(&(pCtx->pWindow));
    gfmTimer_free(&(pCtx->pTimer));
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/* ========================================================================== */
/* |                                                                        | */
/* |  OLD STUFF                                                             | */
/* |                                                                        | */
/* ========================================================================== */
#if 0

#include <GFraMe/GFraMe_error.h>
#include <GFraMe/GFraMe_keys.h>
#include <GFraMe/GFraMe_log.h>
#include <GFraMe/GFraMe_opengl.h>
#include <GFraMe/GFraMe_screen.h>
#include <GFraMe/GFraMe_util.h>
#include <SDL2/SDL.h>


int GFraMe_gl;
/**
 * "Organization" name. Is used as part of paths.
 */
char GFraMe_org[GFraMe_max_org_len];
/**
 * Game's title. Is used as part of paths.
 */
char GFraMe_title[GFraMe_max_game_title_len];
/**
 * Path to the directory where the game is running
 */
char GFraMe_path[GFraMe_max_path_len];

/**
 * Timer used to issue new frames
 */
static GFraMe_timer timer = 0;

/**
 * Initialize SDL, already creating a window and a backbuffer.
 * @param	vw	Buffer's width (virtual width)
 * @param	vh	Buffer's height (virtual height)
 * @param	sw	Window's width (screen width); if 0, uses the device width
 * @param	sh	Window's height(screen height);if 0, uses the device height
 * @param	org	Organization's name (used by the log and save file)
 * @param	name	Game's name (also used as window's title)
 * @param	flags	Window creation flags
 * @param	fps		At how many frames per second the game should run;
 *				  notice that this is independent from update and render
 *				  rate, those should be set on each state
 * @param	log_to_file	Whether should log to a file or to the terminal
 * @param	log_append	Whether should overwrite or append to an existing log
 * @return	0 - Success; Anything else - Failure
 */
GFraMe_ret GFraMe_init(int vw, int vh, int sw, int sh, char *org,
	char *name, GFraMe_window_flags flags, GFraMe_wndext *ext,
	int fps, int log_to_file, int log_append) {
	
	GFraMe_ret rv = GFraMe_ret_ok;
	int ms = 0, len;
	
#ifdef GFRAME_OPENGL
	GFraMe_gl = 1;
#else
	GFraMe_gl = 0;
#endif
	// Store organization name and game's title so it can be used for
	//logging and saving
	len = GFraMe_max_org_len;
	GFraMe_util_strcat(GFraMe_org, org, &len);
	len = GFraMe_max_game_title_len;
	GFraMe_util_strcat(GFraMe_title, name, &len);
	
#ifndef GFRAME_MOBILE
	// Also, get current directory
	char *tmp = SDL_GetBasePath();
	GFraMe_SDLassertRV(tmp, "Couldn't get current running path",
		rv = GFraMe_ret_failed, _ret);
	len = GFraMe_max_path_len;
	GFraMe_util_strcat(GFraMe_path, tmp, &len);
	SDL_free(tmp);
#endif
    
#if !defined(GFRAME_MOBILE)
    GFraMe_key_init();
#endif
	
	if (log_to_file)
		GFraMe_log_init(log_append);
#ifdef GFRAME_DEBUG
	// Set logging, if debug
	SDL_LogSetAllPriority(SDL_LOG_PRIORITY_VERBOSE);
#endif
	
	// Initialize SDL2
	rv = SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER);
	GFraMe_SDLassertRV(rv >= 0, "Couldn't initialize SDL",
		rv = GFraMe_ret_sdl_init_failed, _ret);
	
	// Initialize the screen
	rv = GFraMe_screen_init(vw, vh, sw, sh, name, flags, ext);
	GFraMe_assertRV(rv == GFraMe_ret_ok, "Failed to initialize the screen",
		rv=rv, _ret);
	
	// Create a timer
	ms = GFraMe_timer_get_ms(fps);
	GFraMe_assertRV(ms > 0, "Requested FPS is too low",
		rv = GFraMe_ret_fps_req_low, _ret);
	rv = GFraMe_timer_init(ms, &timer);
	GFraMe_assertRet(rv == GFraMe_ret_ok, "Failed to create timer", _ret);
_ret:
	return rv;
}

/**
 * Clean up memory allocated by init
 */
void GFraMe_quit() {
	if (timer) {
		GFraMe_timer_stop(timer);
		timer = 0;
	}
	GFraMe_screen_clean();
	GFraMe_log_close();
	SDL_Quit();
}

#endif

