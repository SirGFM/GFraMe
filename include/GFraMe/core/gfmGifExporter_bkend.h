/**
 * @file include/GFraMe/core/gfmGifExporter_bkend.h
 * 
 * Module that exports both GIF images and animations
 * 
 * "The Graphics Interchange Format(c) is the Copyright property of
 *  CompuServe Incorporated. GIF(sm) is a Service Mark property of
 *  CompuServe Incorporated."
 */
#ifndef __GFMGIFEXPORTER_BKEND_STRUCT__
#define __GFMGIFEXPORTER_BKEND_STRUCT__

/** 'Exports' the gfmGifExporter structure */
typedef struct stGFMGifExporter gfmGifExporter;

#endif /* __GFMGIFEXPORTER_BKEND_STRUCT__ */

#ifndef __GFMGIFEXPORTER_BKEND_H__
#define __GFMGIFEXPORTER_BKEND_H__

#include <GFraMe/gframe.h>
#include <GFraMe/gfmError.h>
#include <GFraMe/gfmString.h>

/**
 * Check whether exporting GIF is supported
 * 
 * @return GFMRV_TRUE, GFMRV_FALSE
 */
gfmRV gfmGif_isSupported();

/**
 * Alloc a new GIF exporter
 * 
 * @param  ppCtx The alloc'ed object
 * @return       GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_ALLOC_FAILED
 */
gfmRV gfmGif_getNew(gfmGifExporter **ppCtx);

/**
 * Clean up and free an previously alloc'ed GIF exporter
 * 
 * @param  ppCtx The GIF exporter
 * @return       GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmGif_free(gfmGifExporter **ppCtx);

/**
 * Clean up a GIF exporter; If an operation was active, it will be stopped and
 * all temporary files will be deleted
 * 
 * @param  pCtx The GIF exporter
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmGif_clean(gfmGifExporter *pCtx);

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
gfmRV gfmGif_init(gfmGifExporter *pGif, gfmCtx *pCtx, int width, int height);

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
gfmRV gfmGif_storeFrame(gfmGifExporter *pCtx, unsigned char *pData, int len);

/**
 * Exports the stored frame to a GIF image; This function call spawns a new
 * thread to actually create the GIF image
 * 
 * @param  pCtx  The GIF exporter
 * @param  pPath Path where the image should be saved (will overwrite!)
 * @return       GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_GIF_OPERATION_NOT_ACTIVE,
 *               GFMRV_GIF_TOO_MANY_FRAMES, GFMRV_INTERNAL_ERROR
 */
gfmRV gfmGif_exportImage(gfmGifExporter *pCtx, gfmString *pPath);

/**
 * Exports the stored frames to a GIF animation; This function call spawns a new
 * thread to actually create the GIF animation
 * 
 * @param  pCtx  The GIF exporter
 * @param  pPath Path where the image should be saved (will overwrite!)
 * @return       GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_GIF_OPERATION_NOT_ACTIVE,
 *               GFMRV_GIF_NOT_ENOUGH_FRAMES, GFMRV_INTERNAL_ERROR
 */
gfmRV gfmGif_exportAnimation(gfmGifExporter *pCtx, gfmString *pPath);

/**
 * Exports a single image to the requested path
 * 
 * @param  pCtx   The game's context
 * @param  pData  Image's data, in 24 bits RGB (8 bits per color)
 * @param  len    Length of the image's data
 * @param  width  Image's width
 * @param  height Image's height
 * @param  pPath  Path where the image should be saved (will overwrite!)
 */
//gfmRV gfmGif_exportImage(gfmCtx *pCtx, unsigned char *pData, int len, int width,
//        int height, gfmString *pPath);

/**
 * Read the current frame, storing it and its palette info
 * 
 * @param  pCtx  The GIF context
 * @return       GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_ALLOC_FAILED,
 *               GFMRV_GIF_TOO_MANY_COLORS
 */
gfmRV gfmGif_readFrame(gfmGifExporter *pCtx);

/**
 * Writes the GIF's header
 * 
 * @param  pCtx The GIF exporter
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_GIF_NOT_INITIALIZED
 */
gfmRV gfmGif_writeHeader(gfmGifExporter *pCtx);

/**
 * Writes the GIF's logical screen descriptor
 * 
 * @param  pCtx The GIF exporter
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_GIF_NOT_INITIALIZED,
 *              GFMRV_GIF_IMAGE_TOO_LARGE, GFMRV_GIF_IMAGE_TOO_TALL
 */
gfmRV gfmGif_writeLogicalDesc(gfmGifExporter *pCtx);

/**
 * Writes the GIF's global color table
 * 
 * @param  pCtx The GIF exporter
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_GIF_NOT_INITIALIZED
 */
gfmRV gfmGif_writeGlobalPalette(gfmGifExporter *pCtx);

/**
 * Write the frame's data (following its image descriptor)
 * 
 * @param  pCtx The GIF exporter
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_GIF_NOT_INITIALIZED
 */
gfmRV gfmGif_writeFrame(gfmGifExporter *pCtx);

/**
 * Write the GIF's Image descriptor
 * 
 * @param  pCtx The GIF exporter
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_GIF_NOT_INITIALIZED
 */
gfmRV gfmGif_writeImageDescriptor(gfmGifExporter *pCtx);

/**
 * Write the GIF's Image data (compressed with variable length LZW)
 * 
 * @param  pCtx The GIF exporter
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_GIF_NOT_INITIALIZED,
 *              GFMRV_ALLOC_FAILED
 */
gfmRV gfmGif_writeLZWData(gfmGifExporter *pCtx);

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
        int len);

/**
 * Write a data sub-block; Although a data sub-block should be at most 255 bytes
 * long, this function already breaks it in smaller blocks, if needed
 * 
 * @param  pCtx  The GIF exporter
 * @param  pData The data to be written, in a static buffer
 * @return       GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_GIF_NOT_INITIALIZED
 */
#define gfmGif_writeDataSubBlockStatic(pCtx, pData) \
    gfmGif_writeDataSubBlock(pCtx, (unsigned char*)pData, sizeof(pData)-1)

/**
 * Writes a comment with the library name & version and the game's title
 * 
 * @param  pGif The GIF exporter
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_GIF_NOT_INITIALIZED
 */
gfmRV gfmGif_writeComment(gfmGifExporter *pGif);

/**
 * Writes the GIF's trailer (the last part)
 * 
 * @param  pCtx The GIF exporter
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_GIF_NOT_INITIALIZED
 */
gfmRV gfmGif_writeTrailer(gfmGifExporter *pCtx);

#endif /* __GFMGIFEXPORTER_BKEND_H__ */

