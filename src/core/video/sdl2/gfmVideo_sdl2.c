/**
 * SDL2 backend for the video functionalities
 * 
 * @file src/core/video/sdl2/gfmVideo_sdl2.h
 */
#include <GFraMe/gfmAssert.h>
#include <GFraMe/gfmError.h>
#include <GFraMe_int/core/gfmVideo_bkend.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_video.h>

#include <stdlib.h>
#include <string.h>

#include "gfmVideo_sdl2.h"

struct stGFMVideoSDL2 {
/* ==== WINDOW FIELDS ======================================================= */
    /** Actual window (managed by SDL2) */
    SDL_Window *pSDLWindow;
    /** Device's width */
    int devWidth;
    /** Device's height */
    int devHeight;
    /** Window's width (useful only in windowed mode) */
    int wndWidth;
    /** Window's height (useful only in windowed mode) */
    int wndHeight;
    /** Current resolution (useful only in fullscreen) */
    int curResolution;
    /** Whether we are currently in full-screen mode */
    int isFullscreen;
    /** How many resolutions are supported by this device */
    int resCount;
};
typedef struct stGFMVideoSDL2 gfmVideoSDL2;

/**
 * Load all SDL2 video functions into the struct
 * 
 * @param  [ in]pCtx The video function context
 * @return           GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmVideo_SDL2_loadFunctions(gfmVideoFuncs *pCtx) {
    gfmRV rv;

    /* Sanitize arguments */
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);

    /* Simply copy all function pointer */
    pCtx->gfmVideo_init = gfmVideo_SDL2_init;
    pCtx->gfmVideo_free = gfmVideo_SDL2_free;
    pCtx->gfmVideo_countResolutions = gfmVideo_SDL2_countResolutions;
    pCtx->gfmVideo_getResolution = gfmVideo_SDL2_getResolution;
    pCtx->gfmVideo_initWindow = gfmVideo_SDL2_initWindow;
    pCtx->gfmVideo_initWindowFullscreen = gfmVideo_SDL2_initWindowFullscreen;
    pCtx->gfmVideo_initBackbuffer = gfmVideo_SDL2_initBackbuffer;
    pCtx->gfmVideo_setDimensions = gfmVideo_SDL2_setDimensions;
    pCtx->gfmVideo_getDimensions = gfmVideo_SDL2_getDimensions;
    pCtx->gfmVideo_setFullscreen = gfmVideo_SDL2_setFullscreen;
    pCtx->gfmVideo_setWindowed = gfmVideo_SDL2_setWindowed;
    pCtx->gfmVideo_setResolution = gfmVideo_SDL2_setResolution;
    pCtx->gfmVideo_getBackbufferDimensions = gfmVideo_SDL2_getBackbufferDimensions;
    pCtx->gfmVideo_windowToBackbuffer = gfmVideo_SDL2_windowToBackbuffer;
    pCtx->gfmVideo_setBackgroundColor = gfmVideo_SDL2_setBackgroundColor;
    pCtx->gfmVideo_loadTextureBMP = gfmVideo_SDL2_loadTextureBMP;
    pCtx->gfmVideo_setBatched = gfmVideo_SDL2_setBatched;
    pCtx->gfmVideo_drawBegin = gfmVideo_SDL2_drawBegin;
    pCtx->gfmVideo_drawTile = gfmVideo_SDL2_drawTile;
    pCtx->gfmVideo_drawRectangle = gfmVideo_SDL2_drawRectangle;
    pCtx->gfmVideo_drawFillRectangle = gfmVideo_SDL2_drawFillRectangle;
    pCtx->gfmVideo_getBackbufferData = gfmVideo_SDL2_getBackbufferData;
    pCtx->gfmVideo_drawEnd = gfmVideo_SDL2_drawEnd;

    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Initializes a new gfmVideo
 * 
 * @param  [out]ppCtx The alloc'ed gfmVideo context
 * @return            GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_ALLOC_FAILED, ...
 */
gfmRV gfmVideo_SDL2_init(gfmVideo **ppCtx) {
    gfmRV rv;
    gfmVideoSDL2 *pCtx;
    int didInit, irv;
    SDL_DisplayMode sdlMode;

    didInit = 0;
    pCtx = 0;

    /* Sanitize arguments */
    ASSERT(ppCtx, GFMRV_ARGUMENTS_BAD);

    /* Alloc the video context */
    pCtx = (gfmVideoSDL2*)malloc(sizeof(gfmVideoSDL2));
    ASSERT(pCtx, GFMRV_ALLOC_FAILED);

    /* Clean the struct */
    memset(pCtx, 0x0, sizeof(gfmVideoSDL2));

    /* Initialize the SDL2 video subsystem */
    irv = SDL_InitSubSystem(SDL_INIT_VIDEO);
    ASSERT(irv == 0, GFMRV_INTERNAL_ERROR);

    /* Mark SDL2 as initialized, in case anything happens */
    didInit = 1;

    /* Get the device's default resolution */
    irv = SDL_GetDisplayMode(0 /*displayIndex*/, 0/*defResolution*/, &sdlMode);
    ASSERT(irv == 0, GFMRV_INTERNAL_ERROR);
    pCtx->devWidth = sdlMode.w;
    pCtx->devHeight = sdlMode.h;

    /* Retrieve the number of available resolutions */
    pCtx->resCount = SDL_GetNumDisplayModes(0);
    ASSERT(pCtx->resCount, GFMRV_INTERNAL_ERROR);

    /* Set the return variables */
    *ppCtx = (gfmVideo*)pCtx;
    rv = GFMRV_OK;
__ret:
    if (rv != GFMRV_OK) {
        /* On error, shutdown SDL and free the alloc'ed memory */
        if (didInit) {
            SDL_QuitSubSystem(SDL_INIT_VIDEO);
        }
        if (pCtx) {
            free(pCtx);
        }
    }

    return rv;
}

/**
 * Releases a previously alloc'ed/initialized gfmVideo
 * 
 * @param  [out]ppCtx The gfmVideo context
 * @return            GFMRV_OK, GFMRV_ARGUMENTS_BAD, ...
 */
gfmRV gfmVideo_SDL2_free(gfmVideo **ppCtx) {
    return GFMRV_FUNCTION_NOT_IMPLEMENTED;
}

/**
 * Count how many resolution modes there are available when in fullscreen
 * 
 * @param  [out]pCount How many resolutions were found
 * @param  [ in]pVideo The video context (will store the resolutions list)
 * @return             GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmVideo_SDL2_countResolutions(int *pCount, gfmVideo *pVideo) {
    gfmRV rv;
    gfmVideoSDL2 *pCtx;

    /* Retrieve the internal video context */
    pCtx = (gfmVideoSDL2*)pVideo;

    /* Sanitize arguments */
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);

    /* Retrieve the number of resolutions */
    *pCount = pCtx->resCount;

    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Get one of the possibles window's resolution
 * 
 * If the resolutions hasn't been queried, this function will do so
 * 
 * @param  [out]pWidth   A possible window's width
 * @param  [out]pHeight  A possible window's height
 * @param  [out]pRefRate A possible window's refresh rate
 * @param  [ in]pVideo   The video context
 * @param  [ in]index    Resolution to be read (0 is the default resolution)
 * @return               GFMRV_OK, GFMRV_ARGUMENTS_BAD,
 *                       GFMRV_INTERNAL_ERROR, GFMRV_INVALID_INDEX
 */
gfmRV gfmVideo_SDL2_getResolution(int *pWidth, int *pHeight, int *pRefRate,
        gfmWindow *pVideo, int index) {
    gfmRV rv;
    gfmVideoSDL2 *pCtx;
    int irv;
    SDL_DisplayMode sdlMode;

    /* Retrieve the internal video context */
    pCtx = (gfmVideoSDL2*)pVideo;

    /* Sanitize arguments */
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    ASSERT(pWidth, GFMRV_ARGUMENTS_BAD);
    ASSERT(pHeight, GFMRV_ARGUMENTS_BAD);
    ASSERT(pRefRate, GFMRV_ARGUMENTS_BAD);
    ASSERT(index >= 0, GFMRV_ARGUMENTS_BAD);
    /* Check that the resolution is valid */
    ASSERT(index < pCtx->resCount, GFMRV_INVALID_INDEX);

    /* Retrieve the dimensions for the current resolution mode */
    irv = SDL_GetDisplayMode(0 /*displayIndex*/, pCtx->curResolution, &sdlMode);
    ASSERT(irv == 0, GFMRV_INTERNAL_ERROR);

    /* Set the return */
    *pWidth = sdlMode.w;
    *pHeight = sdlMode.h;
    *pRefRate = sdlMode.refresh_rate;
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Create the only window for the game
 * 
 * NOTE 1: The window may switch to fullscreen mode later
 * 
 * NOTE 2: The window's dimensions shall be clamped to the device's ones.
 * The resolution (i.e., width X height X refresh rate) may only take effect
 * when in fullscreen mode, so, in order to set all that on init, use
 * instead gfmVideo_SDL2_initFullscreen
 * 
 * NOTE 3: The argument 'isUserResizable' defines whether a user may
 * manually stretch/shrink, but doesn't control whether or not a window's
 * dimensions may be modified programatically
 * 
 * @param  [ in]pCtx   The video context
 * @param  [ in]width  The desired width
 * @param  [ in]height The desired height
 * @param  [ in]pName  The game's title
 * @param  [ in]flags  Whether the user can resize the window
 * @return             GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_ALLOC_FAILED,
 *                     GFMRV_INTERNAL_ERROR
 */
static gfmRV gfmVideo_SDL2_createWindow(gfmVideoSDL2 *pCtx, int width,
        int height, char *pName, SDL_WindowFlags flags) {
    gfmRV rv;

    /* if pName is NULL, the window should have no title */
    if (!pName) {
        pName = "";
    }

    /* Clamp the dimensions to the device's */
    if (width > pCtx->devWidth) {
        width = pCtx->devWidth;
    }
    if (height > pCtx->devHeight) {
        height = pCtx->devHeight;
    }

    /* Create the window */
    pCtx->pSDLWindow = SDL_CreateWindow(pName, SDL_WINDOWPOS_UNDEFINED,
            SDL_WINDOWPOS_UNDEFINED, width, height, flags);
    ASSERT(pCtx->pSDLWindow, GFMRV_INTERNAL_ERROR);

    /* Store the window (in windowed mode) dimensions */
    pCtx->wndWidth = width;
    pCtx->wndHeight = height;
    /* Set it at the default resolution (since it's the default behaviour) */
    pCtx->curResolution = 0;

    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Create the only window for the game
 * 
 * NOTE 1: The window may switch to fullscreen mode later
 * 
 * NOTE 2: The window's dimensions shall be clamped to the device's ones.
 * The resolution (i.e., width X height X refresh rate) may only take effect
 * when in fullscreen mode, so, in order to set all that on init, use
 * instead gfmVideo_SDL2_initFullscreen
 * 
 * NOTE 3: The argument 'isUserResizable' defines whether a user may
 * manually stretch/shrink, but doesn't control whether or not a window's
 * dimensions may be modified programatically
 * 
 * @param  [ in]pVideo          The video context
 * @param  [ in]width           The desired width
 * @param  [ in]height          The desired height
 * @param  [ in]pName           The game's title (must be NULL terminated)
 * @param  [ in]isUserResizable Whether the user can resize the window
 * @return                      GFMRV_OK, GFMRV_ARGUMENTS_BAD,
 *                              GFMRV_ALLOC_FAILED, GFMRV_INTERNAL_ERROR
 */
gfmRV gfmVideo_SDL2_initWindow(gfmVideo *pVideo, int width, int height,
        char *pName, int isUserResizable) {
    gfmRV rv;
    gfmVideoSDL2 *pCtx;
    SDL_WindowFlags flags;

    /* Retrieve the internal video context */
    pCtx = (gfmVideoSDL2*)pVideo;

    /* Sanitize arguments */
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    ASSERT(width > 0, GFMRV_ARGUMENTS_BAD);
    ASSERT(height > 0, GFMRV_ARGUMENTS_BAD);
    ASSERT(width <= 16384, GFMRV_ARGUMENTS_BAD);
    ASSERT(height <= 16384, GFMRV_ARGUMENTS_BAD);

    /* Set the SDL flag */
    flags = 0;
    if (isUserResizable) {
        flags |= SDL_WINDOW_RESIZABLE;
    }

    /* Actually create the window */
    rv = gfmVideo_SDL2_createWindow(pCtx, width, height, pName, flags);
    ASSERT(rv == GFMRV_OK, rv);

    /* Set it as in windowed mode */
    pCtx->isFullscreen = 0;

    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Create the only window for the game in fullscreen mode
 * 
 * NOTE 1: The resolution is the index to one of the previously queried
 * resolutions
 * 
 * NOTE 2: The window may switch to windowed mode later
 * 
 * @param  [ in]pVideo          The video context
 * @param  [ in]resolution      The desired resolution
 * @param  [ in]pName           The game's title (must be NULL terminated)
 * @param  [ in]isUserResizable Whether the user can resize the window
 * @return                      GFMRV_OK, GFMRV_ARGUMENTS_BAD,
 *                              GFMRV_ALLOC_FAILED, GFMRV_INTERNAL_ERROR,
 *                              GFMRV_INVALID_INDEX
 */
gfmRV gfmVideo_SDL2_initWindowFullscreen(gfmVideo *pVideo, int resolution,
        char *pName, int isUserResizable) {
    gfmRV rv;
    gfmVideoSDL2 *pCtx;
    SDL_WindowFlags flags;

    /* Retrieve the internal video context */
    pCtx = (gfmVideoSDL2*)pVideo;

    /* Sanitize arguments */
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    ASSERT(resolution >= 0, GFMRV_ARGUMENTS_BAD);
    /* Check that the resolution is valid */
    ASSERT(resolution < pCtx->resCount, GFMRV_INVALID_INDEX);

    /* Set the SDL flag */
    flags = SDL_WINDOW_FULLSCREEN_DESKTOP;
    if (isUserResizable) {
        flags |= SDL_WINDOW_RESIZABLE;
    }

    /* Actually create the window */
    rv = gfmVideo_SDL2_createWindow(pCtx, pCtx->devWidth, pCtx->devHeight,
            pName, flags);
    ASSERT(rv == GFMRV_OK, rv);

    /* Set it as in fullscreen mode */
    pCtx->isFullscreen = 1;

    /* Set the current resolution */
    rv = gfmVideo_SDL2_setResolution(pCtx, resolution);
    ASSERT(rv == GFMRV_OK, rv);

    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Initialize the game's backbuffer
 * 
 * In order the decouple the window's resolution and the game's internal
 * resolution, a backbuffer is used. This is a view into the world-space in
 * "native" (i.e. per-pixel) resolution.
 * 
 * NOTE: The backbuffer resolution can't be changed after initialized.
 * 
 * @param  [ in]pVideo The video context
 * @param  [ in]width  The backbuffer's width
 * @param  [ in]height The backbuffer's height
 * @param  [ in]vsync  Whether vsync is enabled or not
 * @return             GFMRV_OK, GFMRV_ARGUMENTS_BAD,
 *                     GFMRV_WINDOW_NOT_INITIALIZED,
 *                     GFMRV_BACKBUFFER_ALREADY_INITIALIZED,
 *                     GFMRV_BACKBUFFER_WIDTH_INVALID,
 *                     GFMRV_BACKBUFFER_HEIGHT_INVALID,
 *                     GFMRV_INTERNAL_ERROR,
 *                     GFMRV_BACKBUFFER_WINDOW_TOO_SMALL
 */
gfmRV gfmVideo_SDL2_initBackbuffer(gfmVideo *pVideo) {
    return GFMRV_FUNCTION_NOT_IMPLEMENTED;
}

/**
 * Set the window's dimensions
 * 
 * This modification will only take effect when in windowed mode. If the
 * window is currently in fullscreen mode, the modification will be delayed
 * until the switch is made
 * 
 * @param  [ in]pVideo The video context
 * @param  [ in]width  The desired width
 * @param  [ in]height The desired height
 * @return             GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_INTERNAL_ERROR,
 *                     GFMRV_WINDOW_NOT_INITIALIZED
 */
gfmRV gfmVideo_SDL2_setDimensions(gfmVideo *pVideo, int width, int height) {
    gfmRV rv;
    gfmVideoSDL2 *pCtx;

    /* Retrieve the internal video context */
    pCtx = (gfmVideoSDL2*)pVideo;

    /* Sanitize arguments */
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    ASSERT(width > 0, GFMRV_ARGUMENTS_BAD);
    ASSERT(height > 0, GFMRV_ARGUMENTS_BAD);
    /* Check that the window was initialized */
    ASSERT(pCtx->pSDLWindow, GFMRV_WINDOW_NOT_INITIALIZED);

    /* Clamp the dimensions to the device's */
    if (width > pCtx->devWidth) {
        width = pCtx->devWidth;
    }
    if (height > pCtx->devHeight) {
        height = pCtx->devHeight;
    }

    /* Try to change the dimensions */
    SDL_SetWindowSize(pCtx->pSDLWindow, width, height);

    /* Store the new  dimensions */
    pCtx->wndWidth = width;
    pCtx->wndHeight = height;

    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Retrieve the window's dimensions
 * 
 * If the window is in fullscreen mode, retrieve the dimensions for the
 * current resolution
 * 
 * @param  [out]pWidth  The current width
 * @param  [out]pHeight The current height
 * @param  [ in]pVideo  The video context
 * @return              GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_INTERNAL_ERROR,
 *                      GFMRV_WINDOW_NOT_INITIALIZED
 */
gfmRV gfmVideo_SDL2_getDimensions(int *pWidth, int *pHeight, gfmVideo *pVideo) {
    gfmRV rv;
    gfmVideoSDL2 *pCtx;
    int irv;

    /* Retrieve the internal video context */
    pCtx = (gfmVideoSDL2*)pVideo;

    /* Sanitize arguments */
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    ASSERT(pWidth, GFMRV_ARGUMENTS_BAD);
    ASSERT(pHeight, GFMRV_ARGUMENTS_BAD);
    /* Check that the window was initialized */
    ASSERT(pCtx->pSDLWindow, GFMRV_WINDOW_NOT_INITIALIZED);

    if (!pCtx->isFullscreen) {
        /* Retrieve the window's dimensions */
        *pWidth = pCtx->wndWidth;
        *pHeight = pCtx->wndHeight;
    }
    else {
        SDL_DisplayMode sdlMode;

        /* Retrieve the dimensions for the current resolution mode */
        irv = SDL_GetDisplayMode(0 /*displayIndex*/, pCtx->curResolution,
                &sdlMode);
        ASSERT(irv == 0, GFMRV_INTERNAL_ERROR);

        *pWidth = sdlMode.w;
        *pHeight = sdlMode.h;
    }

    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Swith the current window mode to fullscreen
 * 
 * @param  [ in]pVideo The video context
 * @return             GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_INTERNAL_ERROR,
 *                     GFMRV_WINDOW_MODE_UNCHANGED, GFMRV_WINDOW_NOT_INITIALIZED
 */
gfmRV gfmVideo_SDL2_setFullscreen(gfmVideo *pVideo) {
    gfmRV rv;
    gfmVideoSDL2 *pCtx;
    int irv;

    /* Retrieve the internal video context */
    pCtx = (gfmVideoSDL2*)pVideo;

    /* Sanitize arguments */
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    /* Check that the window was initialized */
    ASSERT(pCtx->pSDLWindow, GFMRV_WINDOW_NOT_INITIALIZED);

    /* Check that the window isn't in fullscreen mode */
    ASSERT(!pCtx->isFullscreen, GFMRV_WINDOW_MODE_UNCHANGED);

    /* Try to make it fullscrren */
    irv = SDL_SetWindowFullscreen(pCtx->pSDLWindow, SDL_WINDOW_FULLSCREEN_DESKTOP);
    ASSERT(irv == 0, GFMRV_INTERNAL_ERROR);
    pCtx->isFullscreen = 1;

    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Swith the current window mode to windowed
 * 
 * @param  [ in]pVideo The video context
 * @return             GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_INTERNAL_ERROR,
 *                     GFMRV_WINDOW_MODE_UNCHANGED, GFMRV_WINDOW_NOT_INITIALIZED
 */
gfmRV gfmVideo_SDL2_setWindowed(gfmVideo *pVideo) {
    gfmRV rv;
    gfmVideoSDL2 *pCtx;
    int irv;

    /* Retrieve the internal video context */
    pCtx = (gfmVideoSDL2*)pVideo;

    /* Sanitize arguments */
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    /* Check that the window was initialized */
    ASSERT(pCtx->pSDLWindow, GFMRV_WINDOW_NOT_INITIALIZED);

    /* Check that the window isn't in windowed mode */
    ASSERT(pCtx->isFullscreen, GFMRV_WINDOW_MODE_UNCHANGED);

    /* Try to make it fullscrren */
    irv = SDL_SetWindowFullscreen(pCtx->pSDLWindow, 0);
    ASSERT(irv == 0, GFMRV_INTERNAL_ERROR);
    pCtx->isFullscreen = 0;

    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Change the fullscreen resolution of the window
 * 
 * NOTE 1: The resolution is the index to one of the previously queried
 * resolutions
 * 
 * NOTE 2: This modification will only take effect when switching to
 * fullscreen mode
 * 
 * @param  [ in]pVideo The video context
 * @param  [ in]index  The resolution's index
 * @return             GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_INTERNAL_ERROR,
 *                     GFMRV_INVALID_INDEX, GFMRV_WINDOW_NOT_INITIALIZED
 */
gfmRV gfmVideo_SDL2_setResolution(gfmVideo *pVideo, int index) {
    gfmRV rv;
    gfmVideoSDL2 *pCtx;
    int irv;
    SDL_DisplayMode sdlMode;

    /* Retrieve the internal video context */
    pCtx = (gfmVideoSDL2*)pVideo;

    /* Sanitize arguments */
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    ASSERT(index >= 0, GFMRV_ARGUMENTS_BAD);
    /* Check that the index is valid */
    ASSERT(index < pCtx->resCount, GFMRV_INVALID_INDEX);
    /* Check that the window was already initialized */
    ASSERT(pCtx->pSDLWindow, GFMRV_WINDOW_NOT_INITIALIZED);

    /* Retrieve the desired mode */
    irv = SDL_GetDisplayMode(0 /*displayIndex*/, index, &sdlMode);
    ASSERT(irv == 0, GFMRV_INTERNAL_ERROR);

    /* Switch the fullscreen resolution */
    irv = SDL_SetWindowDisplayMode(pCtx->pSDLWindow, &sdlMode);
    ASSERT(irv == 0, GFMRV_INTERNAL_ERROR);

    /* Store the resolution */
    pCtx->curResolution = index;

    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Retrieve the backbuffer's dimensions
 * 
 * @param  [out]pWidth  The width
 * @param  [out]pHeight The height
 * @param  [ in]pVideo  The video context
 * @return              GFMRV_OK, GFMRV_ARGUMENTS_BAD, ...
 */
gfmRV gfmVideo_SDL2_getBackbufferDimensions(int *pWidth, int *pHeight,
        gfmVideo *pVideo) {
    return GFMRV_FUNCTION_NOT_IMPLEMENTED;
}

/**
 * Convert a point in window-space to backbuffer-space
 * 
 * NOTE: Both pX and pY must be initialized with the window-space point
 * 
 * @param  [out]pX   The horizontal position, in backbuffer-space
 * @param  [out]pY   The vertical position, in backbuffer-space
 * @param  [ in]pVideo The video context
 * @return             GFMRV_OK, GFMRV_ARGUMENTS_BAD, ...
 */
gfmRV gfmVideo_SDL2_windowToBackbuffer(int *pX, int *pY, gfmVideo *pVideo) {
    return GFMRV_FUNCTION_NOT_IMPLEMENTED;
}

/**
 * Set the background color
 * 
 * NOTE: This color is used only when cleaning the backbuffer. If the
 * backbuffer has to be letter-boxed into the window, it will cleaned with
 * black.
 * 
 * @param  [ in]pVideo The video context
 * @param  [ in]color  The background color (in 0xAARRGGBB format)
 * @return             GFMRV_OK, GFMRV_ARGUMENTS_BAD, ...
 */
gfmRV gfmVideo_SDL2_setBackgroundColor(gfmVideo *pVideo, int color) {
    return GFMRV_FUNCTION_NOT_IMPLEMENTED;
}

/**
 * Loads a 24 bits bitmap file into a texture
 * 
 * NOTE: The image's dimensions must be power of two (e.g., 256x256)
 * 
 * @param  [out]ppTex     The loaded (and alloc'ed) texture
 * @param  [ in]pVideo    The video context
 * @param  [ in]pFilename The complete path to the file (must be NULL
 *                        terminated)
 * @param  [ in]colorKey   24 bits, RGB Color to be treated as transparent
 */
gfmRV gfmVideo_SDL2_loadTextureBMP(gfmTexture **ppTex, gfmVideo *pVideo,
        char *pFilename, int colorKey) {
    return GFMRV_FUNCTION_NOT_IMPLEMENTED;
}

/**
 * Enable batched draws, if supported
 * 
 * @param  [ in]pVideo The video context
 * @return             GFMRV_OK, GFMRV_ARGUMENTS_BAD,
 *                     GFMRV_FUNCTION_NOT_SUPPORTED
 */
gfmRV gfmVideo_SDL2_setBatched(gfmVideo *pVideo) {
    return GFMRV_FUNCTION_NOT_IMPLEMENTED;
}

/**
 * Initialize the rendering operation
 * 
 * @param  [ in]pVideo The video context
 * @return             GFMRV_OK, GFMRV_ARGUMENTS_BAD, ...
 */
gfmRV gfmVideo_SDL2_drawBegin(gfmVideo *pVideo) {
    return GFMRV_FUNCTION_NOT_IMPLEMENTED;
}

/**
 * Draw a tile into the backbuffer
 * 
 * @param  [ in]pVideo    The video context
 * @param  [ in]pSset     Spriteset containing the tile
 * @param  [ in]x         Horizontal (top-left) position in screen-space
 * @param  [ in]y         Vertical (top-left) position in screen-space
 * @param  [ in]tile      Index of the tile
 * @param  [ in]isFlipped Whether the tile should be flipped
 * @return                GFMRV_OK, GFMRV_ARGUMENTS_BAD, ...
 */
gfmRV gfmVideo_SDL2_drawTile(gfmVideo *pVideo, gfmSpriteset *pSset, int x,
        int y, int tile, int isFlipped) {
    return GFMRV_FUNCTION_NOT_IMPLEMENTED;
}

/**
 * Draw the borders of a rectangle into the backbuffer
 * 
 * @param  [ in]pVideo The video context
 * @param  [ in]x      Horizontal (top-left) position in screen-space
 * @param  [ in]y      Vertical (top-left) position in screen-space
 * @param  [ in]width  The rectangle's width
 * @param  [ in]height The rectangle's height
 * @param  [ in]color  The background color (in 0xAARRGGBB format)
 * @return             GFMRV_OK, GFMRV_ARGUMENTS_BAD, ...
 */
gfmRV gfmVideo_SDL2_drawRectangle(gfmVideo *pVideo, int x, int y, int width,
        int height, int color) {
    return GFMRV_FUNCTION_NOT_IMPLEMENTED;
}

/**
 * Draw a solid rectangle into the backbuffer
 * 
 * @param  [ in]pVideo The video context
 * @param  [ in]x      Horizontal (top-left) position in screen-space
 * @param  [ in]y      Vertical (top-left) position in screen-space
 * @param  [ in]width  The rectangle's width
 * @param  [ in]height The rectangle's height
 * @param  [ in]color  The background color (in 0xAARRGGBB format)
 * @return             GFMRV_OK, GFMRV_ARGUMENTS_BAD, ...
 */
gfmRV gfmVideo_SDL2_drawFillRectangle(gfmVideo *pVideo, int x, int y, int width,
        int height, int color) {
    return GFMRV_FUNCTION_NOT_IMPLEMENTED;
}

/**
 * Get the backbuffer's data (i.e., composite of everything rendered to it
 * since the last gfmBackbuffer_drawBegin)
 * 
 * NOTE 1: Data is returned as 24 bits colors, with 8 bits per color and
 *         RGB order
 * 
 * NOTE 2: This function must be called twice. If pData is NULL, pLen will
 *         return the necessary length for the buffer. If pData isn't NULL,
 *         then pLen must be the length of pData
 * 
 * @param  [out]pData  Buffer where the data should be retrieved (caller
 *                     allocated an freed)
 * @param  [out]pLen   Returns the buffer length, in bytes
 * @param  [ in]pVideo The video context
 * @return             GFMRV_OK, GFMRV_ARGUMENTS_BAD,
 *                     GFMRV_BACKBUFFER_NOT_INITIALIZED,
 *                     GFMRV_BUFFER_TOO_SMALL, GFMRV_INTERNAL_ERROR
 */
gfmRV gfmVideo_SDL2_getBackbufferData(unsigned char *pData, int *pLen,
        gfmVideo *pVideo) {
    return GFMRV_FUNCTION_NOT_IMPLEMENTED;
}

/**
 * Finalize the rendering operation
 * 
 * @param  [ in]pVideo The video context
 * @return             GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_INTERNAL_ERROR
 */
gfmRV gfmVideo_SDL2_drawEnd(gfmVideo *pVideo) {
    return GFMRV_FUNCTION_NOT_IMPLEMENTED;
}

