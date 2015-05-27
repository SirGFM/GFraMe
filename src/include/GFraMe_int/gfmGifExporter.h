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

#include <GFraMe/gfmError.h>
#include <GFraMe/gfmString.h>

/**
 * Exports a single image to the requested path
 * 
 * @param  pData  Image's data, in 24 bits RGB (8 bits per color)
 * @param  len    Length of the image's data
 * @param  width  Image's width
 * @param  height Image's height
 * @param  pPath  Path where the image should be saved (will overwrite!)
 */
gfmRV gfmGif_exportImage(unsigned char *pData, int len, int width, int height,
        gfmString *pPath);

/**
 * Writes the GIF's header
 * 
 * @param  pCtx       The GIF exporter
 * @param  isAnimated Whether it's animated
 * @return            GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_GIF_NOT_INITIALIZED
 */
gfmRV gfmGif_writeHeader(gfmGifExporter *pCtx, int isAnimated);

/**
 * Writes the GIF's logical screen descriptor
 * 
 * @param  pCtx       The GIF exporter
 * @param  width      The image's width
 * @param  height     The image's height
 * @param  colorCount How many colors there are in the palette
 * @return            GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_GIF_NOT_INITIALIZED,
 *                    GFMRV_GIF_IMAGE_TOO_LARGE, GFMRV_GIF_IMAGE_TOO_TALL,
 *                    GFMRV_GIF_TOO_MANY_COLORS
 */
gfmRV gfmGif_writeLogicalDesc(gfmGifExporter *pCtx, int width, int height,
        int colorCount);

#endif /* __GFMGIFEXPORTER_H__ */

