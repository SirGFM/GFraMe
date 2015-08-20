/**
 * @file src/core/noip/gfmGifExporter.c
 * 
 * Module that exports both GIF images and animations; This implementation
 * actually makes it do nothing (useful for mobile, for example)
 * 
 * "The Graphics Interchange Format(c) is the Copyright property of
 *  CompuServe Incorporated. GIF(sm) is a Service Mark property of
 *  CompuServe Incorporated."
 */
#include <GFraMe/gframe.h>
#include <GFraMe/gfmError.h>
#include <GFraMe/gfmString.h>
#include <GFraMe/core/gfmGifExporter_bkend.h>

/** The gfmGifExporter structure */
struct stGFMGifExporter {
    /** A single integer so it's not empty */
    int null;
};

/**
 * Check whether exporting GIF is supported
 * 
 * @return GFMRV_TRUE, GFMRV_FALSE
 */
gfmRV gfmGif_isSupported() {
    return GFMRV_FALSE;
}

/**
 * Alloc a new GIF exporter
 * 
 * @param  ppCtx The alloc'ed object
 * @return       GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_ALLOC_FAILED
 */
gfmRV gfmGif_getNew(gfmGifExporter **ppCtx) {
    return GFMRV_FUNCTION_NOT_SUPPORTED;
}

/**
 * Clean up and free an previously alloc'ed GIF exporter
 * 
 * @param  ppCtx The GIF exporter
 * @return       GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmGif_free(gfmGifExporter **ppCtx) {
    return GFMRV_FUNCTION_NOT_SUPPORTED;
}

/**
 * Clean up a GIF exporter; If an operation was active, it will be stopped and
 * all temporary files will be deleted
 * 
 * @param  pCtx The GIF exporter
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmGif_clean(gfmGifExporter *pCtx) {
    return GFMRV_FUNCTION_NOT_SUPPORTED;
}

/**
 * Initialize a GIF exporter
 * 
 * @param  pGif   The GIF exporter
 * @param  pCtx   The game's context
 * @param  width  The image's width
 * @param  height The image's height
 * @return        GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_GIF_OPERATION_ACTIVE,
 *                GFMRV_GIF_IMAGE_TOO_LARGE, GFMRV_GIF_IMAGE_TOO_TALL
 */
gfmRV gfmGif_init(gfmGifExporter *pGif, gfmCtx *pCtx, int width, int height) {
    return GFMRV_FUNCTION_NOT_SUPPORTED;
}

/**
 * Store a single frame to be later converted into a GIF; To create an
 * animation, this function should be called on every frame
 * 
 * @param  pCtx  The GIF exporter
 * @param  pData Image's data, in 24 bits RGB (8 bits per color)
 * @param  len   Length of the image's data (must be a multiple of 3)
 * @return       GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_GIF_OPERATION_NOT_ACTIVE,
 *               GFMRV_INVALID_BUFFER_LEN
 */
gfmRV gfmGif_storeFrame(gfmGifExporter *pCtx, unsigned char *pData, int len) {
    return GFMRV_FUNCTION_NOT_SUPPORTED;
}

/**
 * Exports the stored frame to a GIF image; This function call spawns a new
 * thread to actually create the GIF image
 * 
 * @param  pCtx  The GIF exporter
 * @param  pPath Path where the image should be saved (will overwrite!)
 * @return       GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_GIF_OPERATION_NOT_ACTIVE,
 *               GFMRV_GIF_TOO_MANY_FRAMES, GFMRV_INTERNAL_ERROR
 */
gfmRV gfmGif_exportImage(gfmGifExporter *pCtx, gfmString *pPath) {
    return GFMRV_FUNCTION_NOT_SUPPORTED;
}

/**
 * Exports the stored frames to a GIF animation; This function call spawns a new
 * thread to actually create the GIF animation
 * 
 * @param  pCtx  The GIF exporter
 * @param  pPath Path where the image should be saved (will overwrite!)
 * @return       GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_GIF_OPERATION_NOT_ACTIVE,
 *               GFMRV_GIF_NOT_ENOUGH_FRAMES, GFMRV_INTERNAL_ERROR
 */
gfmRV gfmGif_exportAnimation(gfmGifExporter *pCtx, gfmString *pPath) {
    return GFMRV_FUNCTION_NOT_SUPPORTED;
}

