/**
 * OpenGL3 backend for the video functionalities
 * 
 * @file src/core/video/opengl3/gfmVideo_opengl3.h
 */
#include <GFraMe/gfmAssert.h>
#include <GFraMe/gfmError.h>
#include <GFraMe_int/core/gfmVideo_bkend.h>

#include "gfmVideo_opengl3.h"

/**
 * Load all OpenGL3 video functions into the struct
 * 
 * @param  [ in]pCtx The video function context
 * @return           GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmVideo_GL3_loadFunctions(gfmVideoFuncs *pCtx) {
    gfmRV rv;

    /* Sanitize arguments */
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);

    /* Simply copy all function pointer */
    pCtx->gfmVideo_init = gfmVideo_GL3_init;
    pCtx->gfmVideo_free = gfmVideo_GL3_free;
    pCtx->gfmVideo_queryResolutions = gfmVideo_GL3_queryResolutions;
    pCtx->gfmVideo_getResolution = gfmVideo_GL3_getResolution;
    pCtx->gfmVideo_initWindow = gfmVideo_GL3_initWindow;
    pCtx->gfmVideo_initWindowFullscreen = gfmVideo_GL3_initWindowFullscreen;
    pCtx->gfmVideo_initBackbuffer = gfmVideo_GL3_initBackbuffer;
    pCtx->gfmVideo_setDimensions = gfmVideo_GL3_setDimensions;
    pCtx->gfmVideo_getDimensions = gfmVideo_GL3_getDimensions;
    pCtx->gfmVideo_setFullscreen = gfmVideo_GL3_setFullscreen;
    pCtx->gfmVideo_setWindowed = gfmVideo_GL3_setWindowed;
    pCtx->gfmVideo_setResolution = gfmVideo_GL3_setResolution;
    pCtx->gfmVideo_getBackbufferDimensions = gfmVideo_GL3_getBackbufferDimensions;
    pCtx->gfmVideo_windowToBackbuffer = gfmVideo_GL3_windowToBackbuffer;
    pCtx->gfmVideo_setBackgroundColor = gfmVideo_GL3_setBackgroundColor;
    pCtx->gfmVideo_loadTextureBMP = gfmVideo_GL3_loadTextureBMP;
    pCtx->gfmVideo_setBatched = gfmVideo_GL3_setBatched;
    pCtx->gfmVideo_drawBegin = gfmVideo_GL3_drawBegin;
    pCtx->gfmVideo_drawTile = gfmVideo_GL3_drawTile;
    pCtx->gfmVideo_drawRectangle = gfmVideo_GL3_drawRectangle;
    pCtx->gfmVideo_drawFillRectangle = gfmVideo_GL3_drawFillRectangle;
    pCtx->gfmVideo_getBackbufferData = gfmVideo_GL3_getBackbufferData;
    pCtx->gfmVideo_drawEnd = gfmVideo_GL3_drawEnd;

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
gfmRV gfmVideo_GL3_init(gfmVideo **ppCtx) {
    return GFMRV_FUNCTION_NOT_IMPLEMENTED;
}

/**
 * Releases a previously alloc'ed/initialized gfmVideo
 * 
 * @param  [out]ppCtx The gfmVideo context
 * @return            GFMRV_OK, GFMRV_ARGUMENTS_BAD, ...
 */
gfmRV gfmVideo_GL3_free(gfmVideo **ppCtx) {
    return GFMRV_FUNCTION_NOT_IMPLEMENTED;
}

/**
 * Create a list with all possible window resolutions and refresh rate
 * 
 * This will depend on the actual backend, but the refresh rate may only be
 * meaningful when on full-screen
 * 
 * @param  [out]pCount How many resolutions were found
 * @param  [ in]pCtx   The video context (will store the resolutions list)
 * @return             GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_INTERNAL_ERROR,
 *                     GFMRV_ALLOC_FAILED, ...
 */
gfmRV gfmVideo_GL3_queryResolutions(int *pCount, gfmVideo *pCtx) {
    return GFMRV_FUNCTION_NOT_IMPLEMENTED;
}

/**
 * Get one of the possibles window's resolution
 * 
 * If the resolutions hasn't been queried, this function will do so
 * 
 * @param  [out]pWidth   A possible window's width
 * @param  [out]pHeight  A possible window's height
 * @param  [out]pRefRate A possible window's refresh rate
 * @param  [ in]pCtx     The video context
 * @param  [ in]index    Resolution to be read (0 is the default resolution)
 * @return               GFMRV_OK, GFMRV_ARGUMENTS_BAD,
 *                       GFMRV_INTERNAL_ERROR, GFMRV_ALLOC_FAILED,
 *                       GFMRV_INVALID_INDEX, ...
 */
gfmRV gfmVideo_GL3_getResolution(int *pWidth, int *pHeight, int *pRefRate,
        gfmWindow *pCtx, int index) {
    return GFMRV_FUNCTION_NOT_IMPLEMENTED;
}

/**
 * Create the only window for the game
 * 
 * NOTE 1: The window may switch to fullscreen mode later
 * 
 * NOTE 2: The window's dimensions shall be clamped to the device's ones.
 * The resolution (i.e., width X height X refresh rate) may only take effect
 * when in fullscreen mode, so, in order to set all that on init, use
 * instead gfmVideo_GL3_initFullscreen
 * 
 * NOTE 3: The argument 'isUserResizable' defines whether a user may
 * manually stretch/shrink, but doesn't control whether or not a window's
 * dimensions may be modified programatically
 * 
 * @param  [ in]pCtx            The video context
 * @param  [ in]width           The desired width
 * @param  [ in]height          The desired height
 * @param  [ in]pName           The game's title (must be NULL terminated)
 * @param  [ in]isUserResizable Whether the user can resize the window
 * @return                      GFMRV_OK, GFMRV_ARGUMENTS_BAD,
 *                              GFMRV_ALLOC_FAILED, GFMRV_INTERNAL_ERROR
 */
gfmRV gfmVideo_GL3_initWindow(gfmVideo *pCtx, int width, int height,
        char *pName, int isUserResizable) {
    return GFMRV_FUNCTION_NOT_IMPLEMENTED;
}

/**
 * Create the only window for the game in fullscreen mode
 * 
 * NOTE 1: The resolution is the index to one of the previously queried
 * resolutions
 * 
 * NOTE 2: The window may switch to windowed mode later
 * 
 * @param  [ in]pCtx            The video context
 * @param  [ in]resolution      The desired resolution
 * @param  [ in]pName           The game's title (must be NULL terminated)
 * @param  [ in]isUserResizable Whether the user can resize the window
 * @return                      GFMRV_OK, GFMRV_ARGUMENTS_BAD,
 *                              GFMRV_ALLOC_FAILED, GFMRV_INTERNAL_ERROR,
 *                              GFMRV_INVALID_INDEX
 */
gfmRV gfmVideo_GL3_initWindowFullscreen(gfmVideo *pCtx, int resolution,
        char *pName, int isUserResizable) {
    return GFMRV_FUNCTION_NOT_IMPLEMENTED;
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
 * @param  [ in]pCtx   The video context
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
gfmRV gfmVideo_GL3_initBackbuffer(gfmVideo *pCtx) {
    return GFMRV_FUNCTION_NOT_IMPLEMENTED;
}

/**
 * Set the window's dimensions
 * 
 * This modification will only take effect when in windowed mode. If the
 * window is currently in fullscreen mode, the modification will be delayed
 * until the switch is made
 * 
 * @param  [ in]pCtx   The video context
 * @param  [ in]width  The desired width
 * @param  [ in]height The desired height
 * @return             GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_INTERNAL_ERROR
 */
gfmRV gfmVideo_GL3_setDimensions(gfmVideo *pCtx, int width, int height) {
    return GFMRV_FUNCTION_NOT_IMPLEMENTED;
}

/**
 * Retrieve the window's dimensions
 * 
 * If the window is in fullscreen mode, retrieve the dimensions for the
 * current resolution
 * 
 * @param  [out]pWidth  The current width
 * @param  [out]pHeight The current height
 * @param  [ in]pCtx    The video context
 * @return              GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_INTERNAL_ERROR
 */
gfmRV gfmVideo_GL3_getDimensions(int *pWidth, int *pHeight, gfmVideo *pCtx) {
    return GFMRV_FUNCTION_NOT_IMPLEMENTED;
}

/**
 * Swith the current window mode to fullscreen
 * 
 * @param  [ in]pCtx The video context
 * @return           GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_INTERNAL_ERROR,
 *                   GFMRV_WINDOW_MODE_UNCHANGED
 */
gfmRV gfmVideo_GL3_setFullscreen(gfmVideo *pCtx) {
    return GFMRV_FUNCTION_NOT_IMPLEMENTED;
}

/**
 * Swith the current window mode to windowed
 * 
 * @param  [ in]pCtx The video context
 * @return           GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_INTERNAL_ERROR,
 *                   GFMRV_WINDOW_MODE_UNCHANGED
 */
gfmRV gfmVideo_GL3_setWindowed(gfmVideo *pCtx) {
    return GFMRV_FUNCTION_NOT_IMPLEMENTED;
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
 * @param  [ in]pCtx The video context
 * @return           GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_INTERNAL_ERROR,
 *                   GFMRV_INVALID_INDEX
 */
gfmRV gfmVideo_GL3_setResolution(gfmVideo *pCtx) {
    return GFMRV_FUNCTION_NOT_IMPLEMENTED;
}

/**
 * Retrieve the backbuffer's dimensions
 * 
 * @param  [out]pWidth  The width
 * @param  [out]pHeight The height
 * @param  [ in]pCtx    The video context
 * @return              GFMRV_OK, GFMRV_ARGUMENTS_BAD, ...
 */
gfmRV gfmVideo_GL3_getBackbufferDimensions(int *pWidth, int *pHeight,
        gfmVideo *pCtx) {
    return GFMRV_FUNCTION_NOT_IMPLEMENTED;
}

/**
 * Convert a point in window-space to backbuffer-space
 * 
 * NOTE: Both pX and pY must be initialized with the window-space point
 * 
 * @param  [out]pX   The horizontal position, in backbuffer-space
 * @param  [out]pY   The vertical position, in backbuffer-space
 * @param  [ in]pCtx The video context
 * @return           GFMRV_OK, GFMRV_ARGUMENTS_BAD, ...
 */
gfmRV gfmVideo_GL3_windowToBackbuffer(int *pX, int *pY, gfmVideo *pCtx) {
    return GFMRV_FUNCTION_NOT_IMPLEMENTED;
}

/**
 * Set the background color
 * 
 * NOTE: This color is used only when cleaning the backbuffer. If the
 * backbuffer has to be letter-boxed into the window, it will cleaned with
 * black.
 * 
 * @param  [ in]pCtx  The video context
 * @param  [ in]color The background color (in 0xAARRGGBB format)
 * @return            GFMRV_OK, GFMRV_ARGUMENTS_BAD, ...
 */
gfmRV gfmVideo_GL3_setBackgroundColor(gfmVideo *pCtx, int color) {
    return GFMRV_FUNCTION_NOT_IMPLEMENTED;
}

/**
 * Loads a 24 bits bitmap file into a texture
 * 
 * NOTE: The image's dimensions must be power of two (e.g., 256x256)
 * 
 * @param  [out]ppTex     The loaded (and alloc'ed) texture
 * @param  [ in]pCtx      The video context
 * @param  [ in]pFilename The complete path to the file (must be NULL
 *                        terminated)
 * @param  [ in]colorKey   24 bits, RGB Color to be treated as transparent
 */
gfmRV gfmVideo_GL3_loadTextureBMP(gfmTexture **ppTex, gfmVideo *pCtx,
        char *pFilename, int colorKey) {
    return GFMRV_FUNCTION_NOT_IMPLEMENTED;
}

/**
 * Enable batched draws, if supported
 * 
 * @param  [ in]pCtx The video context
 * @return           GFMRV_OK, GFMRV_ARGUMENTS_BAD,
 *                   GFMRV_FUNCTION_NOT_SUPPORTED
 */
gfmRV gfmVideo_GL3_setBatched(gfmVideo *pCtx) {
    return GFMRV_FUNCTION_NOT_IMPLEMENTED;
}

/**
 * Initialize the rendering operation
 * 
 * @param  [ in]pCtx The video context
 * @return           GFMRV_OK, GFMRV_ARGUMENTS_BAD, ...
 */
gfmRV gfmVideo_GL3_drawBegin(gfmVideo *pCtx) {
    return GFMRV_FUNCTION_NOT_IMPLEMENTED;
}

/**
 * Draw a tile into the backbuffer
 * 
 * @param  [ in]pCtx      The video context
 * @param  [ in]pSset     Spriteset containing the tile
 * @param  [ in]x         Horizontal (top-left) position in screen-space
 * @param  [ in]y         Vertical (top-left) position in screen-space
 * @param  [ in]tile      Index of the tile
 * @param  [ in]isFlipped Whether the tile should be flipped
 * @return                GFMRV_OK, GFMRV_ARGUMENTS_BAD, ...
 */
gfmRV gfmVideo_GL3_drawTile(gfmVideo *pCtx, gfmSpriteset *pSset, int x,
        int y, int tile, int isFlipped) {
    return GFMRV_FUNCTION_NOT_IMPLEMENTED;
}

/**
 * Draw the borders of a rectangle into the backbuffer
 * 
 * @param  [ in]pCtx   The video context
 * @param  [ in]x      Horizontal (top-left) position in screen-space
 * @param  [ in]y      Vertical (top-left) position in screen-space
 * @param  [ in]width  The rectangle's width
 * @param  [ in]height The rectangle's height
 * @param  [ in]color  The background color (in 0xAARRGGBB format)
 * @return             GFMRV_OK, GFMRV_ARGUMENTS_BAD, ...
 */
gfmRV gfmVideo_GL3_drawRectangle(gfmVideo *pCtx, int x, int y, int width,
        int height, int color) {
    return GFMRV_FUNCTION_NOT_IMPLEMENTED;
}

/**
 * Draw a solid rectangle into the backbuffer
 * 
 * @param  [ in]pCtx   The video context
 * @param  [ in]x      Horizontal (top-left) position in screen-space
 * @param  [ in]y      Vertical (top-left) position in screen-space
 * @param  [ in]width  The rectangle's width
 * @param  [ in]height The rectangle's height
 * @param  [ in]color  The background color (in 0xAARRGGBB format)
 * @return             GFMRV_OK, GFMRV_ARGUMENTS_BAD, ...
 */
gfmRV gfmVideo_GL3_drawFillRectangle(gfmVideo *pCtx, int x, int y, int width,
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
 * @param  [out]pData Buffer where the data should be retrieved (caller
 *                    allocated an freed)
 * @param  [out]pLen  Returns the buffer length, in bytes
 * @param  [ in]pCtx  The video context
 * @return            GFMRV_OK, GFMRV_ARGUMENTS_BAD,
 *                    GFMRV_BACKBUFFER_NOT_INITIALIZED,
 *                    GFMRV_BUFFER_TOO_SMALL, GFMRV_INTERNAL_ERROR
 */
gfmRV gfmVideo_GL3_getBackbufferData(unsigned char *pData, int *pLen,
        gfmVideo *pCtx) {
    return GFMRV_FUNCTION_NOT_IMPLEMENTED;
}

/**
 * Finalize the rendering operation
 * 
 * @param  [ in]pCtx The video context
 * @return           GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_INTERNAL_ERROR
 */
gfmRV gfmVideo_GL3_drawEnd(gfmVideo *pCtx) {
    return GFMRV_FUNCTION_NOT_IMPLEMENTED;
}

