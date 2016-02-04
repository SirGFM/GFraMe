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
 * @param  width  The image's width (must be less than 65535 pixels)
 * @param  height The image's height (must be less than 65535 pixels)
 * @return        GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_GIF_OPERATION_ACTIVE,
 *                GFMRV_GIF_IMAGE_TOO_LARGE, GFMRV_GIF_IMAGE_TOO_TALL
 */
gfmRV gfmGif_init(gfmGifExporter *pGif, gfmCtx *pCtx, int width, int height) {
    return GFMRV_FUNCTION_NOT_SUPPORTED;
}

/**
 * Check whether the current export finished
 * 
 * @param  pCtx The GIF exporter
 * @return      GFMRV_ARGUMENTS_BAD, GFMRV_GIF_OPERATION_NOT_ACTIVE, GFMRV_TRUE,
 *              GFMRV_FALSE
 */
gfmRV gfmGif_didExport(gfmGifExporter *pCtx) {
    return GFMRV_FUNCTION_NOT_SUPPORTED;
}

/**
 * Wait until the current export finished
 * 
 * @param  pCtx The GIF exporter
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_GIF_OPERATION_NOT_ACTIVE
 */
gfmRV gfmGif_waitExport(gfmGifExporter *pCtx) {
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
 *               GFMRV_INVALID_BUFFER_LEN, GFMRV_INTERNAL_ERROR
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

/**
 * Read the current frame, storing it and its palette info
 * 
 * @param  pCtx  The GIF context
 * @return       GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_ALLOC_FAILED,
 *               GFMRV_GIF_TOO_MANY_COLORS
 */
gfmRV gfmGif_readFrame(gfmGifExporter *pCtx) {
    return GFMRV_FUNCTION_NOT_SUPPORTED;
}

/**
 * Writes the GIF's header
 * 
 * @param  pCtx The GIF exporter
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_GIF_NOT_INITIALIZED
 */
gfmRV gfmGif_writeHeader(gfmGifExporter *pCtx) {
    return GFMRV_FUNCTION_NOT_SUPPORTED;
}

/**
 * Writes the GIF's logical screen descriptor
 * 
 * @param  pCtx The GIF exporter
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_GIF_NOT_INITIALIZED,
 *              GFMRV_GIF_IMAGE_TOO_LARGE, GFMRV_GIF_IMAGE_TOO_TALL
 */
gfmRV gfmGif_writeLogicalDesc(gfmGifExporter *pCtx) {
    return GFMRV_FUNCTION_NOT_SUPPORTED;
}

/**
 * Write the Netscape Applicaton Block (so the animation loops infinitely)
 * 
 * @param  pCtx The GIF exporter
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_GIF_NOT_INITIALIZED
 */
gfmRV gfmGif_writeNAB(gfmGifExporter *pCtx) {
    return GFMRV_FUNCTION_NOT_SUPPORTED;
}

/**
 * Write the frame's data (following its image descriptor)
 * 
 * @param  pCtx The GIF exporter
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_GIF_NOT_INITIALIZED
 */
gfmRV gfmGif_writeFrame(gfmGifExporter *pCtx) {
    return GFMRV_FUNCTION_NOT_SUPPORTED;
}

/**
 * Write the frame's Graphic Control Extension
 * 
 * @param  pCtx The GIF exporter
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_GIF_NOT_INITIALIZED
 */
gfmRV gfmGif_writeGCE(gfmGifExporter *pCtx) {
    return GFMRV_FUNCTION_NOT_SUPPORTED;
}

/**
 * Write the GIF's Image descriptor
 * 
 * @param  pCtx The GIF exporter
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_GIF_NOT_INITIALIZED
 */
gfmRV gfmGif_writeImageDescriptor(gfmGifExporter *pCtx) {
    return GFMRV_FUNCTION_NOT_SUPPORTED;
}

/**
 * Write a word of a given length to a buffer of bits
 * 
 * @param  pCtx   The GIF exporter
 * @param  word   Word to be written to the buffer
 * @return        GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmGif_writeBitwiseWord(gfmGifExporter *pCtx, int word) {
    return GFMRV_FUNCTION_NOT_SUPPORTED;
}

/**
 * Write the GIF's Image data (compressed with variable length LZW)
 * 
 * @param  pCtx The GIF exporter
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_GIF_NOT_INITIALIZED,
 *              GFMRV_ALLOC_FAILED
 */
gfmRV gfmGif_writeLZWData(gfmGifExporter *pCtx) {
    return GFMRV_FUNCTION_NOT_SUPPORTED;
}

/**
 * Write a data sub-block; Although a data sub-block should be at most 255 bytes
 * long, this function already breaks it in smaller blocks, if needed
 * 
 * @param  pCtx  The GIF exporter
 * @param  pData The data to be written
 * @param  len   The length of the data
 * @return       GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_GIF_NOT_INITIALIZED
 */
gfmRV gfmGif_writeDataSubBlock(gfmGifExporter *pCtx, unsigned char *pData,
        int len) {
    return GFMRV_FUNCTION_NOT_SUPPORTED;
}

/**
 * Writes a comment with the library name & version and the game's title
 * 
 * @param  pGif The GIF exporter
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_GIF_NOT_INITIALIZED
 */
gfmRV gfmGif_writeComment(gfmGifExporter *pGif) {
    return GFMRV_FUNCTION_NOT_SUPPORTED;
}

/**
 * Writes the GIF's trailer (the last part)
 * 
 * @param  pCtx The GIF exporter
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_GIF_NOT_INITIALIZED
 */
gfmRV gfmGif_writeTrailer(gfmGifExporter *pCtx) {
    return GFMRV_FUNCTION_NOT_SUPPORTED;
}

