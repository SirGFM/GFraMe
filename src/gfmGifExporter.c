/**
 * @file src/gfmGifExporter.c
 * 
 * Module that exports both gif images and animations
 */
#include <GFraMe/gfmAssert.h>
#include <GFraMe/gfmError.h>
#include <GFraMe/gfmString.h>
#include <GFraMe_int/gfmGifExporter.h>

#include <stdio.h>

/** The gfmGifExporter structure */
struct stGFMGifExporter {
    FILE *pFp;
};

/**
 * Exports a single image to the requested path
 * 
 * @param  pData  Image's data, in 24 bits RGB (8 bits per color)
 * @param  len    Length of the image's data
 * @param  width  Image's width
 * @param  height Image's height
 * @param  pPath  Path where the image should be saved (will overwrite!)
 * @return        GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_INVALID_BUFFER_LEN,
 *                GFMRV_GIF_TOO_MANY_COLORS, GFMRV_COULDNT_OPEN_FILE,
 *                GFMRV_GIF_IMAGE_TOO_LARGE, GFMRV_GIF_IMAGE_TOO_TALL,
 */
gfmRV gfmGif_exportImage(unsigned char *pData, int len, int width, int height,
        gfmString *pPath) {
    char *pFilename;
    gfmGifExporter ctx;
    gfmRV rv;
    int colorCount;
    
    // Initialize this to clean on error
    ctx.pFp = 0;
    
    // Sanitize arguments
    ASSERT(pData, GFMRV_ARGUMENTS_BAD);
    ASSERT(len > 0, GFMRV_ARGUMENTS_BAD);
    ASSERT(pPath, GFMRV_ARGUMENTS_BAD);
    // Check that the length is valid
    ASSERT(len == width * height * sizeof(unsigned char) * 3,
            GFMRV_INVALID_BUFFER_LEN);
    // Width and height can't be bigger than 0xffff (65535) pixels
    ASSERT(width <= 0x0000ffff, GFMRV_GIF_IMAGE_TOO_LARGE);
    ASSERT(height <= 0x0000ffff, GFMRV_GIF_IMAGE_TOO_TALL);
    
    // TODO Get the palette
    
    // Get the path
    rv = gfmString_getString(&pFilename, pPath);
    ASSERT_NR(rv == GFMRV_OK);
    
    // Try to open the output file
    ctx.pFp = fopen(pFilename, "wb");
    ASSERT(ctx.pFp, GFMRV_COULDNT_OPEN_FILE);
    
    // Write GIF header
    rv = gfmGif_writeHeader(&ctx, 0/*isAnimated*/);
    ASSERT_NR(rv == GFMRV_OK);
    
    // Write Logical screen descriptor
    rv = gfmGif_writeLogicalDesc(&ctx, width, height, colorCount);
    ASSERT_NR(rv == GFMRV_OK);
    
    // Write data
    // Write Trailer
    
    rv = GFMRV_OK;
__ret:
    if (rv != GFMRV_OK && rv != GFMRV_ARGUMENTS_BAD) {
        // Close the file on error
        if (ctx.pFp) {
            fclose(ctx.pFp);
            ctx.pFp = 0;
        }
    }
    
    return rv;
}

/**
 * Writes the GIF's header
 * 
 * @param  pCtx       The GIF exporter
 * @param  isAnimated Whether it's animated
 * @return            GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_GIF_NOT_INITIALIZED
 */
gfmRV gfmGif_writeHeader(gfmGifExporter *pCtx, int isAnimated) {
    gfmRV rv;
    
    // Sanitize arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    // Check that it was initialized
    ASSERT(pCtx->pFp, GFMRV_GIF_NOT_INITIALIZED);
    
    // Since it's a single frame, doesn't require newer version
    if (!isAnimated)
        fwrite("GIF87a", 6, 1, pCtx->pFp);
    else
        fwrite("GIF89a", 6, 1, pCtx->pFp);
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

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
        int colorCount) {
    gfmRV rv;
    int colorBits;
    unsigned char pBuf[7];
    
    // Sanitize arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    ASSERT(width > 0, GFMRV_ARGUMENTS_BAD);
    ASSERT(height > 0, GFMRV_ARGUMENTS_BAD);
    // Check that it was initialized
    ASSERT(pCtx->pFp, GFMRV_GIF_NOT_INITIALIZED);
    // Width and height can't be bigger than 0xffff (65535) pixels
    ASSERT(width <= 0x0000ffff, GFMRV_GIF_IMAGE_TOO_LARGE);
    ASSERT(height <= 0x0000ffff, GFMRV_GIF_IMAGE_TOO_TALL);
    // Check if the colorCount is OK
    ASSERT(colorCount <= 255, GFMRV_GIF_TOO_MANY_COLORS);
    
    // Prepare the logical screen data
    
    // Write the width
    pBuf[0] = width & 0xff;
    pBuf[1] = (width >> 8) & 0xff;
    
    // Write the height
    pBuf[2] = height & 0xff;
    pBuf[3] = (height >> 8) & 0xff;
    
    // Clean this bitfield
    pBuf[4] = 0;
    // Set the existance of a global palette
    pBuf[4] |= 0x80;
    // Set the source as 8 bits per color
    pBuf[4] |= 0x70;
    // Set the palette as not sorted
    pBuf[4] &= ~0x08;
    
    // Count the number of 'bits - 1' required by the palette
    // (i.e., min(n), 2^(n+1) >= colorCount)
    colorCount >>= 1;
    colorBits = 0;
    while (colorCount > 0) {
        colorCount >>= 1;
        colorBits++;
    }
    // Set the size of the global color table
    pBuf[4] |= colorBits & 0x07;
    
    // Set the bg color (should be useless, but still...)
    pBuf[5] = 0;
    // Set the pixel aspect ratio (should be perfectly square!)
    pBuf[6] = 0;
    
    // Actually write the data
    fwrite(pBuf, 7, 1, pCtx->pFp);
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

