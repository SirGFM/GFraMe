/**
 * @file src/core/common/gfmGifExporter.c
 * 
 * Module that exports both GIF images and animations
 * 
 * "The Graphics Interchange Format(c) is the Copyright property of
 *  CompuServe Incorporated. GIF(sm) is a Service Mark property of
 *  CompuServe Incorporated."
 */
#include <GFraMe/gframe.h>
#include <GFraMe/gfmAssert.h>
#include <GFraMe/gfmError.h>
#include <GFraMe/gfmGenericArray.h>
#include <GFraMe/gfmString.h>
#include <GFraMe/core/gfmGifExporter_bkend.h>
#include <GFraMe_int/gfmTrie.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/** Declare the 'trie nodes array' type */
//gfmGenArray_define(gfmTrie);

/** The gfmGifExporter structure */
struct stGFMGifExporter {
    /** Whether to operation is active or not */
    int isActive;
    /** Image's width */
    int width;
    /** Image's height */
    int height;
    /** How many frames have been stored */
    int frameCount;
    /** Base path for every frame (e.g., [local_path]/gif_[frame] */
    gfmString *pBasePath;
    /** Length of the base path */
    int basePathLen;
    /** Length of a frame name (i.e., len(basePath+framename) ) */
    int framenameLen;
    
    /** Variable used by the thread */
    
    /** Output file */
    FILE *pOut;
    /** The value returned from the thread */
    gfmRV threadRV;
    /** Path were the image should be stored (just a reference!) */
    gfmString *pImagePath;
    /** Current frame being parsed */
    int curFrame;
    /** Number of bits needed to be able to represent every color */
    int colorBits;
    /** Number of colors in the image */
    int colorCount;
    /** Total number of colors in the palette (rounded up from colorCount) */
    int totalColorCount;
    /** How many slots there are in the palette */
    int paletteLen;
    /** Palette in a 00RRGGBB format */
    int *pPalette;
    /** How many useful bytes there are in pData */
    int dataUsed;
    /** How many bytes there are in pData */
    int dataLen;
    /** Buffer where frames are stored (using the palette) */
    unsigned char *pData;
#if 0
    /** Image's output file */
    FILE *pFp;
    /** Image's width */
    int width;
    /** Image's height */
    int height;
    /** Number of bits needed to be able to represent every color */
    int colorBits;
    /** Number of colors in the image */
    int colorCount;
    /** Total number of colors in the palette (rounded up from colorCount) */
    int totalColorCount;
    /** Palette in a 00RRGGBB format */
    int *pPalette;
    /** Node of the dictionary's trie */
    gfmGenArr_var(gfmTrie, pNodes);
#endif
};

/**
 * Generates a GIF image from the passed context
 * 
 * @param pCtx The GIF context
 */
static void _gfmGif_threadHandler(void *pCtx);

/**
 * Check whether exporting GIF is supported
 * 
 * @return GFMRV_TRUE, GFMRV_FALSE
 */
gfmRV gfmGif_isSupported() {
    return GFMRV_TRUE;
}

/**
 * Alloc a new GIF exporter
 * 
 * @param  ppCtx The alloc'ed object
 * @return       GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_ALLOC_FAILED
 */
gfmRV gfmGif_getNew(gfmGifExporter **ppCtx) {
    gfmRV rv;
    
    // Sanitize arguments
    ASSERT(ppCtx, GFMRV_ARGUMENTS_BAD);
    ASSERT(!(*ppCtx), GFMRV_ARGUMENTS_BAD);
    
    // Alloc the object
    *ppCtx = (gfmGifExporter*)malloc(sizeof(gfmGifExporter));
    ASSERT(*ppCtx, GFMRV_ALLOC_FAILED);
    // Initialize init
    memset(*ppCtx, 0x0, sizeof(gfmGifExporter));
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Clean up and free an previously alloc'ed GIF exporter
 * 
 * @param  ppCtx The GIF exporter
 * @return       GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmGif_free(gfmGifExporter **ppCtx) {
    gfmRV rv;
    
    // Sanitize arguments
    ASSERT(ppCtx, GFMRV_ARGUMENTS_BAD);
    ASSERT(*ppCtx, GFMRV_ARGUMENTS_BAD);
    
    // Clean up the object
    rv = gfmGif_clean(*ppCtx);
    ASSERT_NR(rv == GFMRV_OK);
    // And free it
    free(*ppCtx);
    *ppCtx = 0;
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Clean up a GIF exporter; If an operation was active, it will be stopped and
 * all temporary files will be deleted
 * 
 * @param  pCtx The GIF exporter
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmGif_clean(gfmGifExporter *pCtx) {
    gfmRV rv;
    
    // Sanitize arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    
    // TODO Free everything
    gfmString_free(&(pCtx->pBasePath));
    if (pCtx->pPalette) {
        free(pCtx->pPalette);
        pCtx->pPalette = 0;
    }
    if (pCtx->pData) {
        free(pCtx->pData);
        pCtx->pData = 0;
    }
    memset(pCtx, 0x0, sizeof(gfmGifExporter));
    
    rv = GFMRV_OK;
__ret:
    return rv;
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
    gfmRV rv;
    
    // Sanitize arguments
    ASSERT(pGif, GFMRV_ARGUMENTS_BAD);
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    ASSERT(width > 0, GFMRV_ARGUMENTS_BAD);
    ASSERT(height > 0, GFMRV_ARGUMENTS_BAD);
    // Check if the operation is already active
    ASSERT(!pGif->isActive, GFMRV_ARGUMENTS_BAD);
    // Width and height can't be bigger than 0xffff (65535) pixels
    ASSERT(width <= 0x0000ffff, GFMRV_GIF_IMAGE_TOO_LARGE);
    ASSERT(height <= 0x0000ffff, GFMRV_GIF_IMAGE_TOO_TALL);
    
    // Set the base frame's path
    if (!pGif->pBasePath) {
        rv = gfm_getLocalPath(&(pGif->pBasePath), pCtx);
        ASSERT_NR(rv == GFMRV_OK);
        rv = gfmString_getLength(&(pGif->basePathLen), pGif->pBasePath);
        ASSERT_NR(rv == GFMRV_OK);
    }
    else {
        // Recycle the previously used name
        rv = gfmString_setLength(pGif->pBasePath, pGif->basePathLen);
        ASSERT_NR(rv == GFMRV_OK);
    }
    
    // TODO Concatenate a random string to the frame's names
    rv = gfmString_concat(pGif->pBasePath, "gifFrame_", 9);
    ASSERT_NR(rv == GFMRV_OK);
    // Get the position where the frame number is inserted
    rv = gfmString_getLength(&(pGif->framenameLen), pGif->pBasePath);
    ASSERT_NR(rv == GFMRV_OK);
    
    // Set the image's dimensions
    pGif->width = width;
    pGif->height = height;
    // TODO Make sure everything is cleared (so it can be recycled later)
    pGif->frameCount = 0;
    // Set the operation as active
    pGif->isActive = 1;
    
    rv = GFMRV_OK;
__ret:
    return rv;
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
    char *pPath;
    FILE *pFp;
    gfmRV rv;
    
    // Initialize the file
    pFp = 0;
    
    // Sanitize arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    ASSERT(pData, GFMRV_ARGUMENTS_BAD);
    ASSERT(len > 0, GFMRV_ARGUMENTS_BAD);
    ASSERT(len % 3 == 0, GFMRV_ARGUMENTS_BAD);
    // Check that the operation was initialized
    ASSERT(pCtx->isActive, GFMRV_GIF_OPERATION_NOT_ACTIVE);
    // Check that the buffer dimension is valid
    ASSERT(len / 3 == pCtx->width * pCtx->height, GFMRV_INVALID_BUFFER_LEN);
    
    // Append the frame number to the filename
    rv = gfmString_insertNumberAt(pCtx->pBasePath, pCtx->frameCount,
            pCtx->framenameLen);
    ASSERT_NR(rv == GFMRV_OK);
    // Get the path as a string
    rv = gfmString_getString(&pPath, pCtx->pBasePath);
    ASSERT_NR(rv == GFMRV_OK);
    
    // Open the file and insert the data there
    pFp = fopen(pPath, "wb");
    ASSERT(pFp, GFMRV_INTERNAL_ERROR);
    fwrite(pData, 1, len, pFp);
    
    // Increase the number of frames
    pCtx->frameCount++;
    
    rv = GFMRV_OK;
__ret:
    if (pFp) {
        fclose(pFp);
    }
    
    return rv;
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
    gfmRV rv;
    
    // Sanitize arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    ASSERT(pPath, GFMRV_ARGUMENTS_BAD);
    // Check that the operation is active
    ASSERT(pCtx->isActive, GFMRV_GIF_OPERATION_NOT_ACTIVE);
    // Check that there is only one frame
    ASSERT(pCtx->frameCount == 1, GFMRV_GIF_TOO_MANY_FRAMES);
    // TODO Check that there's no thread
    
    // Set the path (so it can be seen by the thread)
    pCtx->pImagePath = pPath;
    pCtx->pOut = 0;
    
    // TODO create thread to handle this
    _gfmGif_threadHandler((void*)pCtx);
    ASSERT_NR(pCtx->threadRV == GFMRV_OK);
    
    rv = GFMRV_OK;
__ret:
    return rv;
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
gfmRV gfmGif_exportAnimation(gfmGifExporter *pCtx, gfmString *pPath);

/**
 * Generates a GIF image from the passed context
 * 
 * @param pCtx The GIF context
 */
static void _gfmGif_threadHandler(void *pArg) {
    char *pPath;
    gfmGifExporter *pGif;
    gfmRV rv;
    
    // Sanitize arguments
    ASSERT(pArg, GFMRV_ARGUMENTS_BAD);
    
    // Retrieve the GIF context
    pGif = (gfmGifExporter *)pArg;
    
    // Check if the operation is active
    ASSERT(pGif->isActive, GFMRV_GIF_OPERATION_NOT_ACTIVE);
    // Check that the path was set
    ASSERT(pGif->pImagePath, GFMRV_GIF_PATH_NOT_SET);
    
    // Retrieve the path to the actual output image
    rv = gfmString_getString(&pPath, pGif->pImagePath);
    ASSERT_NR(rv == GFMRV_OK);
    
    // Open the output file
    pGif->pOut = fopen(pPath, "wb");
    ASSERT(pGif->pOut, GFMRV_INTERNAL_ERROR);
    
    // Write Header
    rv = gfmGif_writeHeader(pGif);
    ASSERT_NR(rv == GFMRV_OK);
    
    // Write Logical screen descriptor
    rv = gfmGif_writeLogicalDesc(pGif);
    ASSERT_NR(rv == GFMRV_OK);
    
    // Write each frame
    pGif->curFrame = 0;
    while (pGif->curFrame < pGif->frameCount) {
        // Buffer this frame
        rv = gfmGif_readFrame(pGif);
        ASSERT_NR(rv == GFMRV_OK);
        
        // Write the frame
        rv = gfmGif_writeFrame(pGif);
        ASSERT_NR(rv == GFMRV_OK);
        
        // Go to the next frame
        pGif->curFrame++;
    }
    
    // Write Comment extension
    rv = gfmGif_writeComment(pGif);
    ASSERT_NR(rv == GFMRV_OK);
    
    // Write Trailer
    rv = gfmGif_writeTrailer(pGif);
    ASSERT_NR(rv == GFMRV_OK);
    
    rv = GFMRV_OK;
__ret:
    if (rv != GFMRV_ARGUMENTS_BAD)
        pGif->threadRV = rv;
    if (pGif->pOut) {
        fclose(pGif->pOut);
        pGif->pOut = 0;
    }
    
    // TODO return to the thread
}

#if 0

/**
 * Exports a single image to the requested path
 * 
 * @param  pCtx   The game's context
 * @param  pData  Image's data, in 24 bits RGB (8 bits per color)
 * @param  len    Length of the image's data
 * @param  width  Image's width
 * @param  height Image's height
 * @param  pPath  Path where the image should be saved (will overwrite!)
 * @return        GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_INVALID_BUFFER_LEN,
 *                GFMRV_GIF_TOO_MANY_COLORS, GFMRV_COULDNT_OPEN_FILE,
 *                GFMRV_GIF_IMAGE_TOO_LARGE, GFMRV_GIF_IMAGE_TOO_TALL,
 */
gfmRV gfmGif_exportImage(gfmCtx *pCtx, unsigned char *pData, int len, int width,
        int height, gfmString *pPath) {
    char *pFilename;
    gfmGifExporter ctx;
    gfmRV rv;
    
    // Initialize this to clean on error
    ctx.pFp = 0;
    ctx.pPalette = 0;
    gfmGenArr_zero(ctx.pNodes);
    
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
    
    // Get the palette
    rv = gfmGif_getColors(&ctx, pData, len);
    ASSERT_NR(rv == GFMRV_OK);
    // Set the image's dimensions
    ctx.width = width;
    ctx.height = height;
    
    // Get the path
    rv = gfmString_getString(&pFilename, pPath);
    ASSERT_NR(rv == GFMRV_OK);
    
    // Try to open the output file
    ctx.pFp = fopen(pFilename, "wb");
    ASSERT(ctx.pFp, GFMRV_COULDNT_OPEN_FILE);
    
    // Write Header
    rv = gfmGif_writeHeader(&ctx);
    ASSERT_NR(rv == GFMRV_OK);
    
    // Write Logical screen descriptor
    rv = gfmGif_writeLogicalDesc(&ctx);
    ASSERT_NR(rv == GFMRV_OK);
    
    // Write Global color table
    rv = gfmGif_writeGlobalPalette(&ctx);
    ASSERT_NR(rv == GFMRV_OK);
    
    rv = gfmGif_writeImage(&ctx, pData, len);
    ASSERT_NR(rv == GFMRV_OK);
    
    // Write Comment extension
    rv = gfmGif_writeComment(&ctx, pCtx);
    ASSERT_NR(rv == GFMRV_OK);
    // Write Trailer
    rv = gfmGif_writeTrailer(&ctx);
    ASSERT_NR(rv == GFMRV_OK);
    
    rv = GFMRV_OK;
__ret:
    // Close the file
    if (ctx.pFp) {
        fclose(ctx.pFp);
        ctx.pFp = 0;
    }
    if (ctx.pPalette) {
        free(ctx.pPalette);
        ctx.pPalette = 0;
    }
    // Free all nodes
    gfmGenArr_clean(pNodes, gfmTrie_free);
    
    return rv;
}
#endif

/**
 * Read the current frame, storing it and its palette info
 * 
 * @param  pCtx  The GIF context
 * @return       GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_ALLOC_FAILED,
 *               GFMRV_GIF_TOO_MANY_COLORS
 */
gfmRV gfmGif_readFrame(gfmGifExporter *pCtx) {
    char *pFramePath;
    FILE *pFp;
    gfmRV rv;
    int i, len;
    
    // Initialize things that can be cleaned later
    pFp = 0;
    
    // Sanitize arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    
    // Append the frame number to the filename
    rv = gfmString_insertNumberAt(pCtx->pBasePath, pCtx->curFrame,
            pCtx->framenameLen);
    ASSERT_NR(rv == GFMRV_OK);
    // Get the path as a string
    rv = gfmString_getString(&pFramePath, pCtx->pBasePath);
    ASSERT_NR(rv == GFMRV_OK);
    
    // Open the frame
    pFp = fopen(pFramePath, "rb");
    ASSERT(pFp, GFMRV_INTERNAL_ERROR);
    
    // Initialize those
    pCtx->colorBits = 1;
    pCtx->colorCount = 0;
    pCtx->totalColorCount = 2;
    pCtx->dataUsed = 0;
    
    // If the palette is empty, alloc it
    if (!pCtx->pPalette) {
        pCtx->paletteLen = pCtx->totalColorCount;
        
        pCtx->pPalette = (int*)malloc(sizeof(int)*pCtx->paletteLen);
        ASSERT(pCtx->pPalette, GFMRV_ALLOC_FAILED);
    }
    
    // Get the buffer length
    len = pCtx->width * pCtx->height;
    // Expand the frame buffer, if necessary
    if (pCtx->dataLen < len) {
        pCtx->dataLen = len;
        
        pCtx->pData = (unsigned char*)realloc(pCtx->pData,
                sizeof(unsigned char*) * pCtx->dataLen);
        ASSERT(pCtx->pData, GFMRV_ALLOC_FAILED);
    }
    
    // Loop through every color >_<
    i = 0;
    while (i < len) {
        int curColor, irv, index;
        unsigned char pData[3];
        
        // Read the data from the file
        irv = fread(pData, 1, 3, pFp);
        ASSERT(irv == 3, GFMRV_INTERNAL_ERROR);
        // Convert the data to palette format: 0x00RRGGBB
        curColor = (pData[0] << 16) | (pData[1] << 8) | (pData[2]);
        
        // Check if the color is in the palette (this is *SLOW*)
        index = 0;
        while (index < pCtx->colorCount) {
            // If the color was found, stop
            if (pCtx->pPalette[index] == curColor)
                break;
            // Otherwise, go to the next one
            index++;
        }
        // Check if the color should be added
        if (index >= pCtx->colorCount) {
            // Check if the palette needs to be expanded
            if (index >= pCtx->totalColorCount) {
                // Double the palette's size (i.e., use one more bit)
                pCtx->totalColorCount *= 2;
                pCtx->colorBits++;
                // Check that the maximum number of colors wasn't reached
                ASSERT(pCtx->colorBits <= 8, GFMRV_GIF_TOO_MANY_COLORS);
                // Expand the palette (if necessary)
                if (pCtx->totalColorCount > pCtx->paletteLen) {
                    pCtx->paletteLen = pCtx->totalColorCount;
                    
                    pCtx->pPalette = (int*)realloc(pCtx->pPalette,
                            sizeof(int)*pCtx->paletteLen);
                    ASSERT(pCtx->pPalette, GFMRV_ALLOC_FAILED);
                }
            }
            // Insert the color on the palette
            pCtx->pPalette[index] = curColor;
            pCtx->colorCount++;
        }
        // Insert the color on the buffer
        pCtx->pData[i] = index;
        
        // Go to the next color
        i++;
    }
    
    rv = GFMRV_OK;
__ret:
    if (pFp)
        fclose(pFp);
    
    return rv;
}

/**
 * Writes the GIF's header
 * 
 * @param  pCtx The GIF exporter
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_GIF_NOT_INITIALIZED
 */
gfmRV gfmGif_writeHeader(gfmGifExporter *pCtx) {
    gfmRV rv;
    
    // Sanitize arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    // Check that it was initialized
    ASSERT(pCtx->pOut, GFMRV_GIF_NOT_INITIALIZED);
    
    // Always use the later version (to enable comments and animation)
    fwrite("GIF89a", 6, 1, pCtx->pOut);
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Writes the GIF's logical screen descriptor
 * 
 * @param  pCtx The GIF exporter
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_GIF_NOT_INITIALIZED,
 *              GFMRV_GIF_IMAGE_TOO_LARGE, GFMRV_GIF_IMAGE_TOO_TALL
 */
gfmRV gfmGif_writeLogicalDesc(gfmGifExporter *pCtx) {
    gfmRV rv;
    unsigned char pBuf[7];
    
    // Sanitize arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    // Check that it was initialized
    ASSERT(pCtx->pOut, GFMRV_GIF_NOT_INITIALIZED);
    // Width and height can't be bigger than 0xffff (65535) pixels
    ASSERT(pCtx->width <= 0x0000ffff, GFMRV_GIF_IMAGE_TOO_LARGE);
    ASSERT(pCtx->height <= 0x0000ffff, GFMRV_GIF_IMAGE_TOO_TALL);
    
    // Prepare the logical screen data
    
    // Set the width
    pBuf[0] = pCtx->width & 0xff;
    pBuf[1] = (pCtx->width >> 8) & 0xff;
    
    // Set the height
    pBuf[2] = pCtx->height & 0xff;
    pBuf[3] = (pCtx->height >> 8) & 0xff;
    
    // Clean this bitfield
    pBuf[4] = 0;
    // Set the existance of a global palette (bit 0x80)
    pBuf[4] |= 0x00;
    // Set the source as 8 bits per color (bits 0x70)
    pBuf[4] |= 0x70;
    // Set the palette as not sorted (bit 0x08)
    pBuf[4] |= 0x00;
    // Set the size of the global color table (bits 0x07)
    pBuf[4] |= 0x00;
    
    // Set the bg color (should be useless, but still...)
    pBuf[5] = 0;
    // Set the pixel aspect ratio (should be perfectly square!)
    pBuf[6] = 0;
    
    // Actually write the data
    fwrite(pBuf, 7, 1, pCtx->pOut);
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

#if 0
/**
 * Writes the GIF's global color table
 * 
 * @param  pCtx The GIF exporter
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_GIF_NOT_INITIALIZED
 */
gfmRV gfmGif_writeGlobalPalette(gfmGifExporter *pCtx) {
    gfmRV rv;
    int i;
    
    // Sanitize arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    // Check that it was initialized
    ASSERT(pCtx->pFp, GFMRV_GIF_NOT_INITIALIZED);
    
    // Write the colors to the palette
    i = 0;
    while (i < pCtx->colorCount) {
        unsigned char pBuf[3];
        
        // Pallete is organized as 0x00RRGGBB
        pBuf[0] = (pCtx->pPalette[i] >> 16) & 0xff;
        pBuf[1] = (pCtx->pPalette[i] >> 8) & 0xff;
        pBuf[2] = pCtx->pPalette[i] & 0xff;
        fwrite(pBuf, 3, 1, pCtx->pFp);
        
        i++;
    }
    // Fill the palette with 0
    while (i < pCtx->totalColorCount) {
        unsigned char pBuf[3];
        
        pBuf[0] = 0;
        pBuf[1] = 0;
        pBuf[2] = 0;
        fwrite(pBuf, 3, 1, pCtx->pFp);
        
        i++;
    }
    
    rv = GFMRV_OK;
__ret:
    return rv;
}
#endif

/**
 * Write the frame's data (following its image descriptor)
 * 
 * @param  pCtx The GIF exporter
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_GIF_NOT_INITIALIZED
 */
gfmRV gfmGif_writeFrame(gfmGifExporter *pCtx) {
    gfmRV rv;
    
    // Sanitize arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    // Check that it was initialized
    ASSERT(pCtx->pOut, GFMRV_GIF_NOT_INITIALIZED);
    
    // TODO Finish this
    ASSERT(0, GFMRV_FUNCTION_NOT_IMPLEMENTED);
    
    if (pCtx->frameCount > 1) {
        // TODO Write the Graphic Control Extension (?)
    }
    
    // Write the Image Descriptor
    rv = gfmGif_writeImageDescriptor(pCtx);
    ASSERT_NR(rv == GFMRV_OK);
    
    // Write the LZW-compresed data
    rv = gfmGif_writeLZWData(pCtx);
    ASSERT_NR(rv == GFMRV_OK);
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Write the GIF's Image descriptor
 * 
 * @param  pCtx The GIF exporter
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_GIF_NOT_INITIALIZED
 */
gfmRV gfmGif_writeImageDescriptor(gfmGifExporter *pCtx) {
    gfmRV rv;
    unsigned char pBuf[10];
    
    // Sanitize arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    // Check that it was initialized
    ASSERT(pCtx->pOut, GFMRV_GIF_NOT_INITIALIZED);
    
    pBuf[0] = 0x2c; /* Set the image Separator */
    pBuf[1] = 0;    /* Image's horizontal position lsb */
    pBuf[2] = 0;    /* Image's horizontal position msb */
    pBuf[3] = 0;    /* Image's vertical position lsb */
    pBuf[4] = 0;    /* Image's vertical position msb */
    pBuf[5] = pCtx->width & 0xff;         /* Set width's lsb */
    pBuf[6] = (pCtx->width >> 8) & 0xff;  /* Set width's msb */
    pBuf[7] = pCtx->height & 0xff;        /* Set height's lsb */
    pBuf[8] = (pCtx->height >> 8) & 0xff; /* Set height's msb */
    
    // TODO Set this a having a palette
    pBuf[9] = 0;      /* Clean this bitfield */
    pBuf[9] &= ~0x80; /* Remove the local color table flag */
    pBuf[9] &= ~0x40; /* Remove the interlaced flag */
    pBuf[9] &= ~0x20; /* Remove the sorted flag */
    pBuf[9] &= ~0x18; /* Reserved... not sure what goes here */
    pBuf[9] &= ~0x07; /* Set size local color table to 0 */
    
    // Actually write the data
    fwrite(pBuf, 10, 1, pCtx->pOut);
    
    // TODO write local palette
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Write a word of a given length to a buffer of bits
 * 
 * @param  pRemBit How many bits still gotta be written (i.e., overflown)
 * @param  pData   Buffer of bits
 * @param  dataLen How many bits there are in the buffer
 * @param  dataPos Current position in the buffer
 * @param  word    Word to be written to the buffer
 * @param  wordLen Length of the word in bits
 * @return         GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_GIF_BITS_REMAINING
 */
gfmRV gfmGif_writeBitwiseWord(int *pRemBit, unsigned char *pData, int dataLen, int dataPos, int word, int wordLen) {
}

/**
 * Write the GIF's Image data (compressed with variable length LZW)
 * 
 * @param  pCtx The GIF exporter
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_GIF_NOT_INITIALIZED,
 *              GFMRV_ALLOC_FAILED
 */
gfmRV gfmGif_writeLZWData(gfmGifExporter *pCtx) {
    // The buffer length must be the size of the data sub-block
#define bufLen 255
    gfmRV rv;
    int bufPos, dataPos, lzwSize, remBits;
    unsigned char pBuf[bufLen];
    
    // Sanitize arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    // Check that it was initialized
    ASSERT(pCtx->pOut, GFMRV_GIF_NOT_INITIALIZED);
    
    // Write LZW minimum size (i.e., number of bits + 1)
    lzwSize = pCtx->colorBits;
    // The algorithm says this is necessary, so...
    if (lzwSize == 1)
        lzwSize++;
    pBuf[0] = lzwSize;
    fwrite(pBuf, 1, 1, pCtx->pOut);
    
    // (GIF's) LZW requires one more bit from the start (stupid algorithm)
    lzwSize++;
    // TODO Write LZW clean code
    //rv = gfmGif_writeBitwiseWord(&remBits, pData, bufLen, dataPos, (1 << pCtx->colorBits), lzwSize);
    // This first word must be completelly written
    ASSERT_NR(rv == GFMRV_OK);
    
    // TODO Write compressed Image's data
    // TODO REMEBE gfmGif_writeDataSubBlock
    // TODO Write LZW clean code + 1
    
#undef bufLen
    rv = GFMRV_OK;
__ret:
    return rv;
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
    gfmRV rv;
    
    // Sanitize arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    ASSERT(pData, GFMRV_ARGUMENTS_BAD);
    ASSERT(len > 0, GFMRV_ARGUMENTS_BAD);
    // Check that it was initialized
    ASSERT(pCtx->pOut, GFMRV_GIF_NOT_INITIALIZED);
    
    // Write every sub-block
    while (len > 0) { 
        unsigned char curLen;
        
        // Set the current length to be written
        if (len > 0xff)
            curLen = 0xff;
        else
            curLen = (unsigned char)len;
        
        // Write the block's length
        fwrite(&curLen, 1, 1, pCtx->pOut);
        // Write the current data
        fwrite(pData, curLen, 1, pCtx->pOut);
        
        // Go to the next position
        pData += curLen;
        len -= curLen;
    }
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Writes a comment with the library name & version and the game's title
 * 
 * @param  pGif The GIF exporter
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_GIF_NOT_INITIALIZED
 */
gfmRV gfmGif_writeComment(gfmGifExporter *pGif) {
    gfmRV rv;
    volatile int len;
    unsigned char c, *pOrg, *pTitle;
    
    // Sanitize arguments
    ASSERT(pGif, GFMRV_ARGUMENTS_BAD);
    // Check that it was initialized
    ASSERT(pGif->pOut, GFMRV_GIF_NOT_INITIALIZED);
    
    // TODO Get the game's title and organization
/*
    rv = gfm_getTitle((char**)&pOrg, (char**)&pTitle, pCtx);
    ASSERT_NR(rv == GFMRV_OK);
*/
    
    // Initialize the comment
    c = 0x21;
    fwrite(&c, 1, 1, pGif->pOut);
    c = 0xfe;
    fwrite(&c, 1, 1, pGif->pOut);
    
    // Write a 'header' for the comment
    rv = gfmGif_writeDataSubBlockStatic(pGif, "GIF made with GFraMe "
            gfmVersion"\n");
    ASSERT_NR(rv == GFMRV_OK);
    
    // TODO Write the game's title
/*    
    // Get the organization's length
    len = 0;
    while (pOrg[len] != '\0') len++;
    // Write the organization
    rv = gfmGif_writeDataSubBlock(pGif, pOrg, len);
    ASSERT_NR(rv == GFMRV_OK);
    
    // Get the title's length
    len = 0;
    while (pTitle[len] != '\0') len++;
    // Write the title
    rv = gfmGif_writeDataSubBlock(pGif, pTitle, len);
    ASSERT_NR(rv == GFMRV_OK);
*/
    
    // Writes the block terminator
    c = 0x0;
    fwrite(&c, 1, 1, pGif->pOut);
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Writes the GIF's trailer (the last part)
 * 
 * @param  pCtx The GIF exporter
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_GIF_NOT_INITIALIZED
 */
gfmRV gfmGif_writeTrailer(gfmGifExporter *pCtx) {
    unsigned char c;
    gfmRV rv;
    
    // Sanitize arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    // Check that it was initialized
    ASSERT(pCtx->pOut, GFMRV_GIF_NOT_INITIALIZED);
    
    // Write the trailer
    c = 0x3b;
    fwrite(&c, 1, 1, pCtx->pOut);
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

