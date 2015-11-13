/**
 * @file src/core/sdl2/gfmBackbuffer.c
 * 
 * Defines the backbuffer ("virtual buffer") and the actual one where it's
 * rendered (displayed)
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
    /** Input texture for rendering */
    SDL_Texture *pCachedTexture;
    /** Cached dimensions to help rendering */
    SDL_Rect outRect;
    /** Whether the frame is batched or each tile is rendered separately */
    int isBatched;
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
    /** Background red component */
    Uint8 bgRed;
    /** Background green component */
    Uint8 bgGreen;
    /** Background blue component */
    Uint8 bgBlue;
    /** Background alpha component */
    Uint8 bgAlpha;
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
 * @param  vsync  Whether vsync is enabled or not
 * @return        GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_WINDOW_NOT_INITIALIZED,
 *                GFMRV_BACKBUFFER_ALREADY_INITIALIZED,
 *                GFMRV_BACKBUFFER_WIDTH_INVALID,
 *                GFMRV_BACKBUFFER_HEIGHT_INVALID, GFMRV_INTERNAL_ERROR,
 *                GFMRV_BACKBUFFER_WINDOW_TOO_SMALL
 */
gfmRV gfmBackbuffer_init(gfmBackbuffer *pCtx, gfmWindow *pWnd, int width,
        int height, int vsync) {
    gfmRV rv;
    int wndWidth, wndHeight;
    SDL_RendererFlags flags;
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
    
    // Select the renderer flags
    flags = SDL_RENDERER_ACCELERATED | SDL_RENDERER_TARGETTEXTURE;
    if (vsync) {
        flags |= SDL_RENDERER_PRESENTVSYNC;
    }
    
    // Initialize both renderer and buffer
    pCtx->pRenderer = SDL_CreateRenderer(pSDLWindow, -1, flags);
    ASSERT(pCtx->pRenderer, GFMRV_INTERNAL_ERROR);
    pCtx->pBackbuffer = SDL_CreateTexture(pCtx->pRenderer,
            SDL_PIXELFORMAT_RGB24, SDL_TEXTUREACCESS_TARGET, width, height);
    ASSERT(pCtx->pBackbuffer , GFMRV_INTERNAL_ERROR);
    
    // Set the backbuffer's dimensions
    pCtx->bbufWidth = width;
    pCtx->bbufHeight = height;
    // Cache the screen's dimensions (i.e., on how much, and where, of the
    //screen the backbuffer will be rendered)
    rv = gfmBackbuffer_cacheDimensions(pCtx, wndWidth, wndHeight);
    ASSERT_NR(rv == GFMRV_OK);
    
    // Set the default background color
    pCtx->bgRed   = 0x00;
    pCtx->bgGreen = 0x00;
    pCtx->bgBlue  = 0x00;
    pCtx->bgAlpha = 0xFF;
    
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
    // Check that the backbuffer was initialized
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

/**
 * Get the backbuffer's internal representation; this is highly dependant on
 * the backend
 * 
 * @param  ppCtx The returned context
 * @param  pBbuf The backbuffer
 * @return       GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_BACKBUFFER_NOT_INITIALIZED
 */
gfmRV gfmBackbuffer_getContext(void **ppCtx, gfmBackbuffer *pBbuf) {
    gfmRV rv;
    
    // Sanitize arguments
    ASSERT(ppCtx, GFMRV_ARGUMENTS_BAD);
    ASSERT(pBbuf, GFMRV_ARGUMENTS_BAD);
    // Check that it was initialized
    ASSERT(pBbuf->pRenderer, GFMRV_BACKBUFFER_NOT_INITIALIZED);
    
    *ppCtx = (void*)(pBbuf->pRenderer);
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Get the backbuffer's dimension
 * 
 * @param  pWidth  The backbuffer's width
 * @param  pHeigth The backbuffer's height
 * @param  pCtx    The backbuffer
 * @return         GFMRV_OK, GFMRV_ARGUMENTS_BAD,
 *                 GFMRV_BACKBUFFER_NOT_INITIALIZED
 */
gfmRV gfmBackbuffer_getDimensions(int *pWidth, int *pHeight, gfmBackbuffer *pCtx) {
    gfmRV rv;
    
    // Sanitize arguments
    ASSERT(pWidth, GFMRV_ARGUMENTS_BAD);
    ASSERT(pHeight, GFMRV_ARGUMENTS_BAD);
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    // Check that it was initialized
    ASSERT(pCtx->pRenderer, GFMRV_BACKBUFFER_NOT_INITIALIZED);
    
    // Get the backbuffer's dimensions
    *pWidth = pCtx->bbufWidth;
    *pHeight = pCtx->bbufHeight;
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Convert a point in 'screen space' to 'backbuffer space'
 * 
 * @param  pX   The current (and returned) position
 * @param  pY   The current (and returned) position
 * @param  pCtx The backbuffer
 * @return         GFMRV_OK, GFMRV_ARGUMENTS_BAD,
 *                 GFMRV_BACKBUFFER_NOT_INITIALIZED
 */
gfmRV gfmBackbuffer_screenToBackbuffer(int *pX, int *pY, gfmBackbuffer *pCtx) {
    gfmRV rv;
    
    // Sanitize arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    ASSERT(pX, GFMRV_ARGUMENTS_BAD);
    ASSERT(pY, GFMRV_ARGUMENTS_BAD);
    // Check that it was initialized
    ASSERT(pCtx->pRenderer, GFMRV_BACKBUFFER_NOT_INITIALIZED);
    
    // Convert the space
    *pX = (*pX - pCtx->scrPosX) / (float)pCtx->scrZoom;
    *pY = (*pY - pCtx->scrPosY) / (float)pCtx->scrZoom;
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Set the background color
 * 
 * @param  pCtx  The backbuffer
 * @param  color The background color (in ARGB, 32 bits, format)
 * @return       GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmBackbuffer_setBackground(gfmBackbuffer *pCtx, int color) {
    gfmRV rv;
    
    // Sanitize arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    
    // Separate each color component
    pCtx->bgAlpha = (Uint8)( (color >> 24) & 0xff);
    pCtx->bgRed   = (Uint8)( (color >> 16) & 0xff);
    pCtx->bgGreen = (Uint8)( (color >>  8) & 0xff);
    pCtx->bgBlue  = (Uint8)( (color      ) & 0xff);
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Loads a texture
 * 
 * @param  pCtx  The backbuffer
 * @param  pTex  The texture
 * @return       GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_TEXTURE_NOT_INITIALIZED
 */
gfmRV gfmBackbuffer_drawLoadTexture(gfmBackbuffer *pCtx, gfmTexture *pTex) {
    gfmRV rv;
    
    // Sanitize arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    ASSERT(pTex, GFMRV_ARGUMENTS_BAD);
    
    // Retrieve the SDL texture
    rv = gfmTexture_getContext((void**)&(pCtx->pCachedTexture), pTex);
    ASSERT_NR(rv == GFMRV_OK);
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Initialize drawing of a frame; The backbuffer must be cleared here!
 * 
 * @param  pCtx The backbuffer
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_BACKBUFFER_NOT_INITIALIZED
 */
gfmRV gfmBackbuffer_drawBegin(gfmBackbuffer *pCtx) {
    gfmRV rv;
    
    // Sanitize arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    // Check that it was initialized
    ASSERT(pCtx->pRenderer, GFMRV_BACKBUFFER_NOT_INITIALIZED);
    
    // Set backbuffer as rendering target
    SDL_SetRenderTarget(pCtx->pRenderer, pCtx->pBackbuffer);
    // Clear the backbuffer
    SDL_SetRenderDrawColor(pCtx->pRenderer, pCtx->bgRed, pCtx->bgGreen,
            pCtx->bgBlue, pCtx->bgAlpha);
    SDL_RenderClear(pCtx->pRenderer);
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Renders a tile
 * 
 * @param  pCtx      The backbuffer
 * @param  pSSet     The spriteset containing the tile
 * @param  x         Horizontal position in screen space
 * @param  y         Vertical position in screen space
 * @param  tile      Tile to be rendered
 * @param  isFlipped Whether the tile should be drawn flipped
 * @return           GFMRV_OK, GFMRV_ARGUMENTS_BAD,
 *                   GFMRV_BACKBUFFER_NOT_INITIALIZED,
 *                   GFMRV_BACKBUFFER_NO_TEXTURE_LOADED
 */
gfmRV gfmBackbuffer_drawTile(gfmBackbuffer *pCtx, gfmSpriteset *pSset, int x,
        int y, int tile, int isFlipped) {
    gfmRV rv;
    int irv, tileX, tileY, tileWidth, tileHeight;
    SDL_Rect src;
    SDL_Rect dst;
    SDL_Texture *pTex;
    
    // Sanitize arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    ASSERT(pSset, GFMRV_ARGUMENTS_BAD);
    ASSERT(tile >= 0, GFMRV_ARGUMENTS_BAD);
    // Check that the backbuffer was initialized
    ASSERT(pCtx->pRenderer, GFMRV_BACKBUFFER_NOT_INITIALIZED);
    
    // Check that it's either not batched or that a default texture was loaded
    ASSERT(pCtx->isBatched == 0 || pCtx->pCachedTexture != 0,
            GFMRV_BACKBUFFER_NO_TEXTURE_LOADED);
    // Retrieve the texture if necessary
    if (!pCtx->pCachedTexture) {
        gfmTexture *pGFMTex;
        
        rv = gfmSpriteset_getTexture(&pGFMTex, pSset);
        ASSERT(rv == GFMRV_OK, rv);
        rv = gfmTexture_getContext((void**)&(pTex), pGFMTex);
        ASSERT(rv == GFMRV_OK, rv);
    }
    else {
        pTex = pCtx->pCachedTexture;
    }
    
    // Get parameters from spriteset
    rv = gfmSpriteset_getDimension(&tileWidth, &tileHeight, pSset);
    ASSERT_NR(rv == GFMRV_OK);
    rv = gfmSpriteset_getPosition(&tileX, &tileY, pSset, tile);
    ASSERT_NR(rv == GFMRV_OK);
    
    // Set SDL's rendering parameters
    src.x = tileX;
    src.y = tileY;
    src.w = tileWidth;
    src.h = tileHeight;
    dst.x = x;
    dst.y = y;
    dst.w = tileWidth;
    dst.h = tileHeight;
    
    // Render the tile
    if (isFlipped) {
        irv = SDL_RenderCopyEx(pCtx->pRenderer, pTex, &src, &dst, 0.0/*angle*/,
                0/*center*/, SDL_FLIP_HORIZONTAL);
    }
    else {
        irv = SDL_RenderCopy(pCtx->pRenderer, pTex, &src, &dst);
    }
    ASSERT(irv == 0, GFMRV_INTERNAL_ERROR);
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Renders a rectangle (only its vertices);
 * NOTE: This function isn't guaranteed to be fast, so use it wisely
 * 
 * @param  pCtx   The game's context
 * @param  x      Top-left position, in screen-space
 * @param  y      Top-left position, in screen-space
 * @param  width  Rectangle's width
 * @param  height Rectangle's height
 * @param  red    Color's red component
 * @param  green  Color's green component
 * @param  blue   Color's blue component
 * @return        GFMRV_OK, GFMRV_ARGUMENTS_BAD,
 *                GFMRV_BACKBUFFER_NOT_INITIALIZED
 */
gfmRV gfmBackbuffer_drawRect(gfmBackbuffer *pCtx, int x, int y, int width,
        int height, unsigned char red, unsigned char green, unsigned char blue){
    gfmRV rv;
    int irv;
    SDL_Rect rect;
    
    // Sanitize arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    // Check that the backbuffer was initialized
    ASSERT(pCtx->pRenderer, GFMRV_BACKBUFFER_NOT_INITIALIZED);
    // Check that the rectangle is inside the screen
    ASSERT(x > 0 || x + width > 0, GFMRV_OK);
    ASSERT(y > 0 || y + height > 0, GFMRV_OK);
    ASSERT(x < pCtx->bbufWidth || x + width < pCtx->bbufWidth, GFMRV_OK);
    ASSERT(y < pCtx->bbufHeight || y + height < pCtx->bbufHeight, GFMRV_OK);
    
    // Set the rect's dimensions
    rect.x = x;
    rect.y = y;
    rect.w = width;
    rect.h = height;
    
    // Set the color to render it
    irv = SDL_SetRenderDrawColor(pCtx->pRenderer, red, green, blue, 0xff);
    ASSERT(irv == 0, GFMRV_INTERNAL_ERROR);
    irv = SDL_RenderDrawRect(pCtx->pRenderer, &rect);
    ASSERT(irv == 0, GFMRV_INTERNAL_ERROR);
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Render the current frame to the screen
 * 
 * @param  pCtx The backbuffer
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_BACKBUFFER_NOT_INITIALIZED
 */
gfmRV gfmBackbuffer_drawEnd(gfmBackbuffer *pCtx, gfmWindow *pWnd) {
    gfmRV rv;
    
    // Sanitize arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    // Check that it was initialized
    ASSERT(pCtx->pRenderer, GFMRV_BACKBUFFER_NOT_INITIALIZED);
    
    // Set the screen as rendering target
    SDL_SetRenderTarget(pCtx->pRenderer, 0);
    // Clear the screen
    SDL_SetRenderDrawColor(pCtx->pRenderer, 0/*r*/, 0/*g*/, 0/*b*/, 0/*a*/);
    SDL_RenderClear(pCtx->pRenderer);
    // Render the backbuffer to the screen
    SDL_RenderCopy(pCtx->pRenderer, pCtx->pBackbuffer, 0/*srcRect*/,
            &(pCtx->outRect));
    SDL_RenderPresent(pCtx->pRenderer);
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Get the backbuffer's data (i.e., composite of everything rendered to it since
 * the last gfmBackbuffer_drawBegin)
 * 
 * Data is returned as 24 bits colors, with 8 bits per color and RGB order;
 * Also, pixels are indexed from left to rigth and from top to bottom
 * 
 * @param  pData Buffer where the data should be stored (caller allocated an
 *               freed); If it's NULL, the required length is returned in pLen
 * @param  pLen  Returns the required number of bytes; If pData isn't NULL, it
 *               must have the number of bytes when calling the funciton
 * @param  pCtx  The backbuffer
 * @return       GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_BACKBUFFER_NOT_INITIALIZED,
 *               GFMRV_BUFFER_TOO_SMALL, GFMRV_INTERNAL_ERROR
 */
gfmRV gfmBackbuffer_getBackbufferData(unsigned char *pData, int *pLen,
        gfmBackbuffer *pCtx) {
    gfmRV rv;
    int len, irv;
    
    // Sanitize arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    ASSERT(pLen, GFMRV_ARGUMENTS_BAD);
    // Check that the backbuffer was initialized
    ASSERT(pCtx->pRenderer, GFMRV_BACKBUFFER_NOT_INITIALIZED);
    
    // Calculate the required length
    len = pCtx->bbufWidth * pCtx->bbufHeight * 3 * sizeof(unsigned char);
    
    // Check that either the buffer is big enough or it's requesting the len
    ASSERT(!pData || *pLen >= len, GFMRV_BUFFER_TOO_SMALL);
    // Store the return value
    *pLen = len;
    // If requested, return the required size
    ASSERT(pData, GFMRV_OK);
    
    // Make sure the current target is the backbuffer
    SDL_SetRenderTarget(pCtx->pRenderer, pCtx->pBackbuffer);
    // Actually retrieve the data
    irv = SDL_RenderReadPixels(pCtx->pRenderer, 0, SDL_PIXELFORMAT_RGB24,
            (void*)pData, pCtx->bbufWidth * 3 * sizeof(unsigned char));
    ASSERT(irv == 0, GFMRV_INTERNAL_ERROR);
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

