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

    return GFMRV_FUNCTION_NOT_IMPLEMENTED;
#if 0
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
#endif /* 0 */

    rv = GFMRV_OK;
_GL3_ret:
    return rv;
}

