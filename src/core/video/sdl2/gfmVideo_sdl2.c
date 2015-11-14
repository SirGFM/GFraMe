/**
 * SDL2 backend for the video functionalities
 * 
 * @file src/core/video/sdl2/gfmVideo_sdl2.h
 */
#include <GFraMe/gfmAssert.h>
#include <GFraMe/gfmError.h>
#include <GFraMe_int/core/gfmVideo_bkend.h>

#include "gfmVideo_sdl2.h"

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

    return GFMRV_FUNCTION_NOT_IMPLEMENTED;
#if 0
    /* Simply copy all function pointer */
    pCtx->gfmVideo_init = gfmVideo_SDL2_init;
    pCtx->gfmVideo_free = gfmVideo_SDL2_free;
    pCtx->gfmVideo_queryResolutions = gfmVideo_SDL2_queryResolutions;
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
#endif /* 0 */

    rv = GFMRV_OK;
_SDL2_ret:
    return rv;
}

