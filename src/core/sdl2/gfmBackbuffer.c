/**
 * @file src/core/sdl2/gfmBackbuffer.c
 * 
 * Defines the backbuffer ("virtual buffer") and the actual are it's rendered to
 */
#include <GFraMe/gfmAssert.h>
#include <GFraMe/gfmError.h>
#include <GFraMe/core/gfmBackbuffer_bkend.h>
#include <GFraMe/core/gfmWindow_bkend.h>

#include <SDL2/SDL_render.h>
#include <SDL2/SDL_video.h>

#include <stdlib.h>
#include <string.h>

struct stGFMBackbuffer {
    /** Intermediate context used to render things to the backbuffer and, then,
      to the screen */
    SDL_Renderer *pRenderer;
    /** Buffer used to render everything */
    SDL_Texture *pBackbuffer;
    /** Cached dimensions to help rendering */
    SDL_Rect outRect;
    /** Backbuffer's width */
    int bbufWidth;
    /** Backbuffer's height */
    int bbufHeight;
    /** Width of the actual rendered buffer */
    int scrWidth;
    /** Height of the actual rendered buffer */
    int scrHeight;
    /** Position of the rendered buffer on the window */
    int scrPosX;
    /** Position of the rendered buffer on the window */
    int scrPosY;
    /** Factor by which the (output) screen is bigger than the backbuffer */
    int scrZoom;
};
/** 'Exportable' size of gfmBackbuffer */
const int sizeofGFMBackbuffer = sizeof(gfmBackbuffer);

/**
 * Alloc a new gfmBackbuffer
 * 
 * @param  ppCtx The "alloc'ed" backbuffer context
 * @return       GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_ALLOC_FAILED
 */
gfmRV gfmBackbuffer_getNew(gfmBackbuffer **ppCtx) {
    gfmRV rv;
    
    // Sanitize arguments
    ASSERT(ppCtx, GFMRV_ARGUMENTS_BAD);
    ASSERT(!(*ppCtx), GFMRV_ARGUMENTS_BAD);
    
    // Alloc the struct
    *ppCtx = (gfmBackbuffer*)malloc(sizeof(gfmBackbuffer));
    ASSERT(*ppCtx, GFMRV_ALLOC_FAILED);
    
    // Clear every 'member'
    memset(*ppCtx, 0x0, sizeof(gfmBackbuffer));
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Free a previously allocated gfmBackbuffer
 * 
 * @param  ppCtx The "alloc'ed" backbuffer context
 * @return       GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmBackbuffer_free(gfmBackbuffer **ppCtx) {
    gfmRV rv;
    
    // Sanitize arguments
    ASSERT(ppCtx, GFMRV_ARGUMENTS_BAD);
    ASSERT(*ppCtx, GFMRV_ARGUMENTS_BAD);
    
    // Clean the backbuffer
    gfmBackbuffer_clean(*ppCtx);
    
    // Free it
    free(*ppCtx);
    *ppCtx = 0;
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

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
        int height) {
    gfmRV rv;
    int wndWidth, wndHeight;
    SDL_Window* pSDLWindow;
    
    // Sanitize arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    ASSERT(pWnd, GFMRV_ARGUMENTS_BAD);
    ASSERT(width > 0, GFMRV_ARGUMENTS_BAD);
    ASSERT(height > 0, GFMRV_ARGUMENTS_BAD);
    // Check that the window is already initialized
    ASSERT(gfmWindow_wasInit(pWnd), GFMRV_WINDOW_NOT_INITIALIZED);
    // Check that the backbuffer is still uninitialized
    ASSERT(!pCtx->pRenderer, GFMRV_BACKBUFFER_ALREADY_INITIALIZED);
    
    // Check that the window is big enough to support the backbuffer
    rv = gfmWindow_getDimensions(&wndWidth, &wndHeight, pWnd);
    ASSERT_NR(rv == GFMRV_OK);
    ASSERT(width <= wndWidth, GFMRV_BACKBUFFER_WIDTH_INVALID);
    ASSERT(height <= wndHeight, GFMRV_BACKBUFFER_HEIGHT_INVALID);
    
    // Get the SDL_Window context
    rv = gfmWindow_getContext((void**)&pSDLWindow, pWnd);
    ASSERT_NR(rv == GFMRV_OK);
    
    // Initialize both renderer and buffer
    pCtx->pRenderer = SDL_CreateRenderer(pSDLWindow, -1,
            SDL_RENDERER_ACCELERATED | SDL_RENDERER_TARGETTEXTURE);
    ASSERT(pCtx->pRenderer, GFMRV_INTERNAL_ERROR);
    pCtx->pBackbuffer = SDL_CreateTexture(pCtx->pRenderer,
            SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_TARGET, width, height);
    ASSERT(pCtx->pBackbuffer , GFMRV_INTERNAL_ERROR);
    
    // Set the backbuffer's dimensions
    pCtx->bbufWidth = width;
    pCtx->bbufHeight = height;
    // Cache the screen's dimensions (i.e., on how much, and where, of the
    //screen the backbuffer will be rendered)
    rv = gfmBackbuffer_cacheDimensions(pCtx, wndWidth, wndHeight);
    ASSERT_NR(rv == GFMRV_OK);
    
    rv = GFMRV_OK;
__ret:
    // Clean things up, on error
    if (rv != GFMRV_OK && rv != GFMRV_ARGUMENTS_BAD && 
            rv != GFMRV_BACKBUFFER_ALREADY_INITIALIZED) {
        if (pCtx->pRenderer)
		    SDL_DestroyRenderer(pCtx->pRenderer);
        pCtx->pRenderer = 0;
        if (pCtx->pBackbuffer)
    		SDL_DestroyTexture(pCtx->pBackbuffer);
        pCtx->pBackbuffer = 0;
    }
    return rv;
}

/**
 * Clean up a backbuffer
 * 
 * @param  pCtx The "alloc'ed" backbuffer context
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmBackbuffer_clean(gfmBackbuffer *pCtx) {
    gfmRV rv;
    
    // Sanitize arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    
    // Clean up everything
    if (pCtx->pRenderer)
        SDL_DestroyRenderer(pCtx->pRenderer);
    if (pCtx->pBackbuffer)
        SDL_DestroyTexture(pCtx->pBackbuffer);
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

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
gfmRV gfmBackbuffer_cacheDimensions(gfmBackbuffer *pCtx, int width, int height) {
    gfmRV rv;
    int horRatio, verRatio;
    
    // Sanitize arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    ASSERT(width > 0, GFMRV_ARGUMENTS_BAD);
    ASSERT(height > 0, GFMRV_ARGUMENTS_BAD);
    // Check that the backbuffer is still uninitialized
    ASSERT(pCtx->pRenderer, GFMRV_BACKBUFFER_NOT_INITIALIZED);
    // Check that the window's dimension is valid
    ASSERT(width >= pCtx->bbufWidth, GFMRV_BACKBUFFER_WINDOW_TOO_SMALL);
    ASSERT(height >= pCtx->bbufHeight, GFMRV_BACKBUFFER_WINDOW_TOO_SMALL);
    
	// Check each possible ratio
	horRatio = (int)( (double)width / (double)pCtx->bbufWidth);
	verRatio = (int)( (double)height / (double)pCtx->bbufHeight);
    // Get the lesser of the two
    if (horRatio < verRatio)
        pCtx->scrZoom = horRatio;
    else
        pCtx->scrZoom = verRatio;
    ASSERT(pCtx->scrZoom > 0, GFMRV_BACKBUFFER_WINDOW_TOO_SMALL);
    
    // Center the output
	pCtx->scrPosX = (width - pCtx->bbufWidth * pCtx->scrZoom) / 2;
	pCtx->scrPosY = (height - pCtx->bbufHeight * pCtx->scrZoom) / 2;
	pCtx->scrWidth = pCtx->bbufWidth * pCtx->scrZoom;
	pCtx->scrHeight = pCtx->bbufHeight * pCtx->scrZoom;
    
    // Cache it into a SDL_Rect (used for rendering)
	pCtx->outRect.x = pCtx->scrPosX;
	pCtx->outRect.y = pCtx->scrPosY;
	pCtx->outRect.w = pCtx->scrWidth;
	pCtx->outRect.h = pCtx->scrHeight;
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

