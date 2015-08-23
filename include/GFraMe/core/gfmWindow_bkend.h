/**
 * @file include/GFraMe/core/gfmWindow_bkend.h
 * 
 * Handle operations on the window
 */
#ifndef __GFMWINDOW_STRUCT_H__
#define __GFMWINDOW_STRUCT_H__

/** "Export" the window structure's type */
typedef struct stGFMWindow gfmWindow;

#endif /* __GFMWINDOW_STRUCT_H__ */

#ifndef __GFMWINDOW_BKEND_H__
#define __GFMWINDOW_BKEND_H__

#include <GFraMe/gfmError.h>

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
 * Whether the window was initialized
 * 
 * @param  ppCtx The allocated 'object'
 * @return       GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_TRUE, GFMRV_FALSE
 */
gfmRV gfmWindow_wasInit(gfmWindow *pCtx);

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
 * @param  pCtx            The window context
 * @param  width           The desired width
 * @param  height          The desired height
 * @param  pName           The game's title, in a NULL terminated string
 * @param  isUserResizable Whether the user can resize the window through the OS
 * @return                 GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_ALLOC_FAILED,
 *                         GFMRV_INTERNAL_ERROR
 */
gfmRV gfmWindow_init(gfmWindow *pCtx, int width, int height, char *pName,
        int isUserResizable);

/**
 * Initialize (i.e., create) a full screen window with the desired resolution
 * 
 * @param  pCtx  The window context
 * @param  index Resolution to be used (0 is the default resolution)
 * @param  pName  The game's title, in a NULL terminated string
 * @param  isUserResizable Whether the user can resize the window through the OS
 * @return       GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_INTERNAL_ERROR,
 *               GFMRV_ALLOC_FAILED, GFMRV_INVALID_INDEX
 */
gfmRV gfmWindow_initFullScreen(gfmWindow *pCtx, int resIndex, char *pName,
        int isUserResizable);

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
 * @return        GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_INTERNAL_ERROR,
 *                GFMRV_WINDOW_IS_FULLSCREEN
 */
gfmRV gfmWindow_setDimensions(gfmWindow *pCtx, int width, int height);

/**
 * Get the window's dimensions
 * 
 * @param  pWidth  The desired width
 * @param  pHeight The desired height
 * @param  pCtx    The window context
 * @return         GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmWindow_getDimensions(int *pWidth, int *pHeight, gfmWindow *pCtx);

/**
 * Make the game go full-screen
 * 
 * @param  pCtx   The window context
 * @return        GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_INTERNAL_ERROR,
 *                GFMRV_WINDOW_MODE_UNCHANGED
 */
gfmRV gfmWindow_setFullScreen(gfmWindow *pCtx);

/**
 * Make the game go windowed;
 * The window's dimensions will be kept!
 * 
 * @param  pCtx   The window context
 * @return        GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_INTERNAL_ERROR,
 *                GFMRV_WINDOW_MODE_UNCHANGED
 */
gfmRV gfmWindow_setWindowed(gfmWindow *pCtx);

/**
 * Set the window's resolution;
 * If the window is in full screen mode, it's resolution and refresh rate will
 * be modified; Otherwise, only it's dimension's will be modified
 * 
 * @param  pCtx  The window context
 * @param  index Resolution to be used (0 is the default resolution)
 * @return       GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_INTERNAL_ERROR,
 *               GFMRV_INVALID_INDEX
 */
gfmRV gfmWindow_setResolution(gfmWindow *pCtx, int resIndex);

/**
 * Returns the window's context; this is highly dependant on the backend
 * 
 * @param  ppCtx The returned context
 * @param  pWnd  The window
 * @return       GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_WINDOW_NOT_INITIALIZED
 */
gfmRV gfmWindow_getContext(void **ppCtx, gfmWindow *pWnd);

/**
 * Initialize a rendering operation
 * 
 * @param  pCtx  The window
 * @return       GFMRV_OK, GFMRV_ARGUMENTS_BAD, ...
 */
gfmRV gfmWindow_beginDraw(gfmWindow *pCtx);

/**
 * Finalize a rendering operation
 * 
 * @param  pCtx  The window
 * @return       GFMRV_OK, GFMRV_ARGUMENTS_BAD, ...
 */
gfmRV gfmWindow_drawEnd(gfmWindow *pCtx);

#endif /* __GFMWINDOW_BKEND_H__ */
