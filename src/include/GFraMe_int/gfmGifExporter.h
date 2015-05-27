/**
 * @file src/include/GFraMe_int/gfmGifExporter.h
 * 
 * Module that exports both gif images and animations
 */
#ifndef __GFMGIFEXPORTER_STRUCT__
#define __GFMGIFEXPORTER_STRUCT__

/** 'Exports' the gfmGifExporter structure */
typedef struct stGFMGifExporter gfmGifExporter;

#endif /* __GFMGIFEXPORTER_STRUCT__ */

#ifndef __GFMGIFEXPORTER_H__
#define __GFMGIFEXPORTER_H__

#include <GFraMe/gframe.h>
#include <GFraMe/gfmError.h>
#include <GFraMe/gfmString.h>

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
gfmRV gfmGif_exportImage(gfmCtx *pCtx, unsigned char *pData, int len, int width,
        int height, gfmString *pPath);

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
 *              GFMRV_GIF_IMAGE_TOO_LARGE, GFMRV_GIF_IMAGE_TOO_TALL,
 *              GFMRV_GIF_TOO_MANY_COLORS
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
 * @param  pCtx The game's context
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_GIF_NOT_INITIALIZED
 */
gfmRV gfmGif_writeComment(gfmGifExporter *pGif, gfmCtx *pCtx);

/**
 * Writes the GIF's trailer (the last part)
 * 
 * @param  pCtx The GIF exporter
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_GIF_NOT_INITIALIZED
 */
gfmRV gfmGif_writeTrailer(gfmGifExporter *pCtx);

#endif /* __GFMGIFEXPORTER_H__ */

