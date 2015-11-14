/**
 * Define a generic video backend that shall be loaded from a shared lib (so the
 * user may switch it)
 */
#ifndef __GFMVIDEO_STRUCT__
#define __GFMVIDEO_STRUCT__

#include <GFraMe/gfmError.h>

/* Struct that store the loaded functions */
typedef struct stGFMVideoFuncs gfmVideoFuncs;
/* The video context */
typedef struct stGFMVideo gfmVideo;

#endif /* __GFMVIDEO_STRUCT__ */

#ifndef __GFMVIDEO_BKEND_H__
#define __GFMVIDEO_BKEND_H__

#include <GFraMe/gfmSpriteset.h>
#include <GFraMe/core/gfmTexture_bkend.h>

/** Struct to store all the loaded functions */
struct stGFMVideoFuncs {

    /**
     * Initializes a new gfmVideo
     * 
     * @param  [out]ppCtx The alloc'ed gfmVideo context
     * @return            GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_ALLOC_FAILED, ...
     */
    gfmRV (*gfmVideo_init)(gfmVideo **ppCtx);

    /**
     * Releases a previously alloc'ed/initialized gfmVideo
     * 
     * @param  [out]ppCtx The gfmVideo context
     * @return            GFMRV_OK, GFMRV_ARGUMENTS_BAD, ...
     */
    gfmRV (*gfmVideo_free)(gfmVideo **ppCtx);

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
    gfmRV (*gfmVideo_queryResolutions)(int *pCount, gfmVideo *pCtx);

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
    gfmRV (*gfmVideo_getResolution)(int *pWidth, int *pHeight, int *pRefRate,
            gfmWindow *pCtx, int index);

    /**
     * Create the only window for the game
     * 
     * NOTE 1: The window may switch to fullscreen mode later
     * 
     * NOTE 2: The window's dimensions shall be clamped to the device's ones.
     * The resolution (i.e., width X height X refresh rate) may only take effect
     * when in fullscreen mode, so, in order to set all that on init, use
     * instead gfmVideo_initFullscreen.
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
    gfmRV (*gfmVideo_initWindow)(gfmVideo *pCtx, int width, int height,
            char *pName, int isUserResizable);

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
    gfmRV (*gfmVideo_initWindowFullscreen)(gfmVideo *pCtx, int resolution,
            char *pName, int isUserResizable);
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
    gfmRV (*gfmVideo_initBackbuffer)(gfmVideo *pCtx);

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
    gfmRV (*gfmVideo_setDimensions)(gfmVideo *pCtx, int width, int height);

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
    gfmRV (*gfmVideo_getDimensions)(int *pWidth, int *pHeight, gfmVideo *pCtx);

    /**
     * Swith the current window mode to fullscreen
     * 
     * @param  [ in]pCtx The video context
     * @return           GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_INTERNAL_ERROR,
     *                   GFMRV_WINDOW_MODE_UNCHANGED
     */
    gfmRV (*gfmVideo_setFullscreen)(gfmVideo *pCtx);

    /**
     * Swith the current window mode to windowed
     * 
     * @param  [ in]pCtx The video context
     * @return           GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_INTERNAL_ERROR,
     *                   GFMRV_WINDOW_MODE_UNCHANGED
     */
    gfmRV (*gfmVideo_setWindowed)(gfmVideo *pCtx);

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
    gfmRV (*gfmVideo_setResolution)(gfmVideo *pCtx);

    /**
     * Retrieve the backbuffer's dimensions
     * 
     * @param  [out]pWidth  The width
     * @param  [out]pHeight The height
     * @param  [ in]pCtx    The video context
     * @return              GFMRV_OK, GFMRV_ARGUMENTS_BAD, ...
     */
    gfmRV (*gfmVideo_getBackbufferDimensions)(int *pWidth, int *pHeight,
            gfmVideo *pCtx);

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
    gfmRV (*gfmVideo_windowToBackbuffer)(int *pX, int *pY, gfmVideo *pCtx);

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
    gfmRV (*gfmVideo_setBackgroundColor)(gfmVideo *pCtx, int color);

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
    gfmRV (*gfmVideo_loadTextureBMP)(gfmTexture **ppTex, gfmVideo *pCtx,
            char *pFilename, int colorKey);

    /**
     * Enable batched draws, if supported
     * 
     * @param  [ in]pCtx The video context
     * @return           GFMRV_OK, GFMRV_ARGUMENTS_BAD,
     *                   GFMRV_FUNCTION_NOT_SUPPORTED
     */
    gfmRV (*gfmVideo_setBatched)(gfmVideo *pCtx);

    /**
     * Initialize the rendering operation
     * 
     * @param  [ in]pCtx The video context
     * @return           GFMRV_OK, GFMRV_ARGUMENTS_BAD, ...
     */
    gfmRV (*gfmVideo_drawBegin)(gfmVideo *pCtx);

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
    gfmRV (*gfmVideo_drawTile)(gfmVideo *pCtx, gfmSpriteset *pSset, int x,
            int y, int tile, int isFlipped);

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
    gfmRV (*gfmVideo_drawRectangle)(gfmVideo *pCtx, int x, int y, int width,
            int height, int color);

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
    gfmRV (*gfmVideo_drawFillRectangle)(gfmVideo *pCtx, int x, int y, int width,
            int height, int color);

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
    gfmRV (*gfmVideo_getBackbufferData)(unsigned char *pData, int *pLen,
            gfmVideo *pCtx);

    /**
     * Finalize the rendering operation
     * 
     * @param  [ in]pCtx The video context
     * @return           GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_INTERNAL_ERROR
     */
    gfmRV (*gfmVideo_drawEnd)(gfmVideo *pCtx);
};

#endif /* __GFMVIDEO_BKEND_H__ */

