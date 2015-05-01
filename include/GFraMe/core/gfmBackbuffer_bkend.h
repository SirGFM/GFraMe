/**
 * @file include/GFraMe/core/gfmBackbuffer_bkend.h
 * 
 * Defines the backbuffer ("virtual buffer") and the actual are it's rendered to
 */
#ifndef __GFMBACKBUFFER_BKEND_H__
#define __GFMBACKBUFFER_BKEND_H__

#include <GFraMe/gfmError.h>
#include <GFraMe/core/gfmWindow_bkend.h>

/** "Export" the backbuffer structure's type */
typedef struct stGFMBackbuffer gfmBackbuffer;
/** 'Exportable' size of gfmBackbuffer */
extern const int sizeofGFMBackbuffer;

/**
 * Alloc a new gfmBackbuffer
 * 
 * @param  ppCtx The "alloc'ed" backbuffer context
 * @return       GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_ALLOC_FAILED
 */
gfmRV gfmBackbuffer_getNew(gfmBackbuffer **ppCtx);

/**
 * Free a previously allocated gfmBackbuffer
 * 
 * @param  ppCtx The "alloc'ed" backbuffer context
 * @return       GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmBackbuffer_free(gfmBackbuffer **ppCtx);

/**
 * Initializes the backbuffer
 * 
 * @param  pCtx   The backbuffer context
 * @param  pWnd   The window context
 * @param  width  The backbuffer's width
 * @param  height The backbuffer's height
 * @return        GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_WINDOW_NOT_INITIALIZED,
 *                GFMRV_BACKBUFFER_ALREADY_INITIALIZED,
 *                GFMRV_BACKBUFFER_WIDTH_INVALID,
 *                GFMRV_BACKBUFFER_HEIGHT_INVALID, GFMRV_INTERNAL_ERROR,
 *                GFMRV_BACKBUFFER_WINDOW_TOO_SMALL
 */
gfmRV gfmBackbuffer_init(gfmBackbuffer *pCtx, gfmWindow *pWnd, int width,
        int height);

/**
 * Clean up a backbuffer
 * 
 * @param  pCtx The "alloc'ed" backbuffer context
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmBackbuffer_clean(gfmBackbuffer *pCtx);

/**
 * Update the backbuffer's destiny screen, as to mantain the aspect ratio
 * 
 * @param  pCtx   The backbuffer context
 * @param  width  The window'w width
 * @param  height The window's height
 * @return        GFMRV_OK, GFMRV_ARGUMENTS_BAD,
 *                GFMRV_BACKBUFFER_NOT_INITIALIZED,
 *                GFMRV_BACKBUFFER_WINDOW_TOO_SMALL
 */
gfmRV gfmBackbuffer_cacheDimensions(gfmBackbuffer *pCtx, int width, int height);

/**
 * Get the backbuffer's internal representation; this is highly dependant on
 * the backend
 * 
 * @param  ppCtx The returned context
 * @param  pBbuf The backbuffer
 * @return       GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_BACKBUFFER_NOT_INITIALIZED
 */
gfmRV gfmBackbuffer_getContext(void **ppCtx, gfmBackbuffer *pBbuf);

#endif /* __GFMBACKBUFFER_BKEND_H__ */

