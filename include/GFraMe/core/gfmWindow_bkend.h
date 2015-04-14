/**
 * @file include/GFraMe/core/gfmWindow_bkend.h
 * 
 * Handle operations on the window
 */
#ifndef __GFMWINDOW_BKEND_H__
#define __GFMWINDOW_BKEND_H__

#include <GFraMe/gfmError.h>

/** "Export" the window structure's type */
typedef struct stGFMWindow gfmWindow;
/** 'Exportable' size of gfmWindow */
extern const int sizeofGFMWindow;

/**
 * Alloc a new gfmWindow structure
 * 
 * @param  ppCtx The allocated 'object'
 * @return       GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_ALLOC_FAILED, ...
 */
gfmRV gfmWindow_getNew(gfmWindow **ppCtx);

/**
 * Free a previously allocated gfmWindow
 * 
 * @param  ppCtx The allocated 'object'
 * @return       GFMRV_OK, GFMRV_ARGUMENTS_BAD, ...
 */
gfmRV gfmWindow_free(gfmWindow **ppCtx);

/**
 * Create a list with all possible window resolutions and refresh rate; This
 * will depend on the actual backend, but the refresh rate may only be
 * meaningful when on full-screen
 * 
 * @param  pCount How many resolutions were found
 * @param  pCtx   Window context (will store the resolutions list)
 * @return        GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_INTERNAL_ERROR,
 *                GFMRV_ALLOC_FAILED, ...
 */
gfmRV gfmWindow_queryResolutions(int *pCount, gfmWindow *pCtx);

/**
 * Get a resolution; if gfmWindow_queryResolutions wasn't previously called, it
 * will be automatically called
 * 
 * @param  pWidth   A possible window's width
 * @param  pHeight  A possible window's height
 * @param  pRefRate A possible window's refresh rate
 * @param  pCtx     The window context
 * @param  index    Resolution to be read (0 is the default resolution)
 * @return          GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_INTERNAL_ERROR,
 *                  GFMRV_ALLOC_FAILED, GFMRV_INVALID_INDEX, ...
 */
gfmRV gfmWindow_getResolution(int *pWidth, int *pHeight, int *pRefRate,
        gfmWindow *pCtx, int index);

/**
 * Initialize (i.e., create) a window with the desired dimensions; If the
 * resolution is greater than the device's, then the device's resolution shall
 * be used
 * 
 * @param  pCtx   The window context
 * @param  width  The desired width
 * @param  height The desired height
 * @param  pName  The game's title, in a NULL terminated string
 * @return        GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_ALLOC_FAILED,
 *                GFMRV_INTERNAL_ERROR
 */
gfmRV gfmWindow_init(gfmWindow *pCtx, int width, int height, char *pName);

/**
 * Initialize (i.e., create) a full screen window with the desired resolution
 * 
 * @param  pCtx  The window context
 * @param  index Resolution to be used (0 is the default resolution)
 * @param  pName  The game's title, in a NULL terminated string
 * @return       GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_INTERNAL_ERROR,
 *               GFMRV_ALLOC_FAILED, GFMRV_INVALID_INDEX
 */
gfmRV gfmWindow_initFullScreen(gfmWindow *pCtx, int resIndex, char *pName);

/**
 * Clean up (i.e., close) the window
 * 
 * @param  pCtx The window context
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmWindow_clean(gfmWindow *pCtx);

/**
 * Resize the window to the desired dimensions
 * 
 * @param  pCtx   The window context
 * @param  width  The desired width
 * @param  height The desired height
 * @return        GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_INTERNAL_ERROR
 */
gfmRV gfmWindow_setDimensions(gfmWindow *pCtx, int width, int height);

/**
 * Make the game go full-screen
 * 
 * @param  pCtx   The window context
 * @return        GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_INTERNAL_ERROR,
 *                GFMRV_WINDOW_MODE_UNCHANGED
 */
gfmRV gfmWindow_setFullScreen(gfmWindow *pCtx);

/**
 * Make the game go windowed
 * 
 * @param  pCtx   The window context
 * @return        GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_INTERNAL_ERROR,
 *                GFMRV_WINDOW_MODE_UNCHANGED
 */
gfmRV gfmWindow_setWindowed(gfmWindow *pCtx);

/**
 * Set the window's resolution; The window must be in full-screen mode!
 * 
 * @param  pCtx  The window context
 * @param  index Resolution to be used (0 is the default resolution)
 * @return       GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_INTERNAL_ERROR,
 *               GFMRV_ALLOC_FAILED, GFMRV_INVALID_INDEX
 */
gfmRV gfmWindow_setResolution(gfmWindow *pCtx, int resIndex);

#endif

