/**
 * @src/gframe.c
 */
#include <GFraMe/GFraMe_error.h>
#include <GFraMe/GFraMe_keys.h>
#include <GFraMe/GFraMe_log.h>
#include <GFraMe/GFraMe_opengl.h>
#include <GFraMe/GFraMe_screen.h>
#include <GFraMe/GFraMe_util.h>
#include <SDL2/SDL.h>

#include <GFraMe/gframe.h>
#include <GFraMe/gfmAssert.h>
#include <GFraMe/gfmError.h>
#include <GFraMe/gfmString.h>
#include <GFraMe/core/gfmBackend_bkend.h>
#include <GFraMe/core/gfmTime_bkend.h>
#include <GFraMe/core/gfmPath_bkend.h>

struct stGFMCtx {
    // TODO specify backend
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
    /** Timer used to issue new frames */
    gfmTimer *pTimer;
};

/** 'Exportable' size of gfmStruct */
const size_t sizeofGFMCtx = sizeof(struct stGFMCtx);

/**
 * Alloc a new gfmContext
 * 
 * @param  ppCtx The allocated context
 * @return       GFMRV_OK, GFMRV_ARGUMENTS_BAD
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
        gfm_free(ppCtx);
    }
    
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
        gfmString_free(&(pCtx->pGameOrg);
        gfmString_free(&(pCtx->pGameTitle);
        gfmString_free(&(pCtx->pSaveFilename);
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
gfmRV gframe_getTitle(char **ppOrg, char **ppTitle, gfmCtx *pCtx) {
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
 * Set the game window and backbuffer initial resolution;
 * 
 * *NOTE*: The game window may be later modified, but not the backbuffer's!
 * 
 * @param  pCtx     The game's context
 * @param  bufWidth  Backbuffer's width
 * @param  bufHeight Backbuffer's height
 * @param  devWidth  Device's width
 * @param  devHeight Device's height
 * @return           GFMRV_OK, ...
 */
gfmRV gfm_setGameResolution(gfmCtx *pCtx, int bufWidth, int bufHeight,
        int devWidth, int devHeight) {
}

gfmRV gfm_initAll() {
}

/* ========================================================================== */
/* |                                                                        | */
/* |  OLD STUFF                                                             | */
/* |                                                                        | */
/* ========================================================================== */

#if 0

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

