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
#include <GFraMe_int/gfmTrie.h>
#include <GFraMe/core/gfmGifExporter_bkend.h>

#include <stdio.h>

/** Declare the 'trie nodes array' type */
gfmGenArray_define(gfmTrie);

/** The gfmGifExporter structure */
struct stGFMGifExporter {
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
};

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

/**
 * Get how many colors are needed by the whole image and store it in a palette
 * 
 * @param  pCtx  The GIF context
 * @param  pData Buffer with the colors (mustn't be overwritten)
 * @param  len   Length of the color buffer
 * @return       GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_ALLOC_FAILED,
 *               GFMRV_GIF_TOO_MANY_COLORS
 */
gfmRV gfmGif_getColors(gfmCtx *pCtx, unsigned char *pData, int len) {
    gfmRV rv;
    int i;
    
    // Sanitize arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    ASSERT(pData, GFMRV_ARGUMENTS_BAD);
    ASSERT(len > 0, GFMRV_ARGUMENTS_BAD);
    ASSERT(len % 3 == 0, GFMRV_ARGUMENTS_BAD);
    
    // Initialize those
    pCtx->colorBits = 1;
    pCtx->colorCount = 0;
    pCtx->totalColorCount = 2;
    pCtx->pPalette = (int*)malloc(sizeof(int)*pCtx->totalColorCount);
    ASSERT(pCtx->pPalette, GFMRV_ALLOC_FAILED);
    
    // Loop through every color >_<
    i = 0;
    while (i < len) {
        int curColor, newIndex;
        
        // Convert the data to palette format: 0x00RRGGBB
        curColor = (pData[i] << 16) | (pData[i+1] << 8) | (pData[i+2]);
        
        // Check if the color is in the palette (this is *SLOW*)
        newIndex = 0;
        while (newIndex < pCtx->colorCount) {
            // If the color was found, stop
            if (pCtx->pPalette[newIndex] == curColor)
                break;
            // Otherwise, go to the next one
            newIndex++;
        }
        // Check if the color should be added
        if (newIndex >= pCtx->colorCount) {
            // Check if the palette needs to be expanded
            if (newIndex >= pCtx->totalColorCount) {
                // Double the palette's size (i.e., use one more bit)
                pCtx->totalColorCount *= 2;
                pCtx->colorBits++;
                // Check that the maximum number of colors wasn't reached
                ASSERT(pCtx->colorBits <= 8, GFMRV_GIF_TOO_MANY_COLORS);
                // Expand the palette
                pCtx->pPalette = (int*)realloc(pCtx->pPalette,
                        sizeof(int)*pCtx->totalColorCount);
                ASSERT(pCtx->pPalette, GFMRV_ALLOC_FAILED);
            }
            // Insert the color on the palette
            pCtx->pPalette[newIndex] = curColor;
            pCtx->colorCount++;
        }
        
        // Go to the next color
        i += 3;
    }
    
    rv = GFMRV_OK;
__ret:
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
    ASSERT(pCtx->pFp, GFMRV_GIF_NOT_INITIALIZED);
    
    // Always use the later version (to enable comments and animation)
    fwrite("GIF89a", 6, 1, pCtx->pFp);
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Writes the GIF's logical screen descriptor
 * 
 * @param  pCtx The GIF exporter
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_GIF_NOT_INITIALIZED,
 *              GFMRV_GIF_IMAGE_TOO_LARGE, GFMRV_GIF_IMAGE_TOO_TALL,
 *              GFMRV_GIF_TOO_MANY_COLORS
 */
gfmRV gfmGif_writeLogicalDesc(gfmGifExporter *pCtx) {
    gfmRV rv;
    int colorBits, colorCount;
    unsigned char pBuf[7];
    
    // Sanitize arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    // Check that it was initialized
    ASSERT(pCtx->pFp, GFMRV_GIF_NOT_INITIALIZED);
    // Width and height can't be bigger than 0xffff (65535) pixels
    ASSERT(pCtx->width <= 0x0000ffff, GFMRV_GIF_IMAGE_TOO_LARGE);
    ASSERT(pCtx->height <= 0x0000ffff, GFMRV_GIF_IMAGE_TOO_TALL);
    // Check if the colorCount is OK
    ASSERT(pCtx->colorCount <= 255, GFMRV_GIF_TOO_MANY_COLORS);
    
    // Prepare the logical screen data
    
    // Set the width
    pBuf[0] = pCtx->width & 0xff;
    pBuf[1] = (pCtx->width >> 8) & 0xff;
    
    // Set the height
    pBuf[2] = pCtx->height & 0xff;
    pBuf[3] = (pCtx->height >> 8) & 0xff;
    
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
    colorCount = pCtx->colorCount;
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

/**
 * Write the image's data (following its image descriptor)
 * 
 * @param  pCtx The GIF exporter
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_GIF_NOT_INITIALIZED
 */
gfmRV gfmGif_writeImage(gfmGifExporter *pCtx, unsigned char *pData, int len) {
    gfmRV rv;
    
    // Sanitize arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    ASSERT(pData, GFMRV_ARGUMENTS_BAD);
    ASSERT(len > 0, GFMRV_ARGUMENTS_BAD);
    // Check that it was initialized
    ASSERT(pCtx->pFp, GFMRV_GIF_NOT_INITIALIZED);
    
    // TODO Write the Graphic Control Extension (?)
    
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
    ASSERT(pCtx->pFp, GFMRV_GIF_NOT_INITIALIZED);
    
    pBuf[0] = 0x2c; /* Set the image Separator */
    pBuf[1] = 0;    /* Image's horizontal position lsb */
    pBuf[2] = 0;    /* Image's horizontal position msb */
    pBuf[3] = 0;    /* Image's vertical position lsb */
    pBuf[4] = 0;    /* Image's vertical position msb */
    pBuf[5] = pCtx->width & 0xff;         /* Set width's lsb */
    pBuf[6] = (pCtx->width >> 8) & 0xff;  /* Set width's msb */
    pBuf[7] = pCtx->height & 0xff;        /* Set height's lsb */
    pBuf[8] = (pCtx->height >> 8) & 0xff; /* Set height's msb */
    
    pBuf[9] = 0;      /* Clean this bitfield */
    pBuf[9] &= ~0x80; /* Remove the local color table flag */
    pBuf[9] &= ~0x40; /* Remove the interlaced flag */
    pBuf[9] &= ~0x20; /* Remove the sorted flag */
    pBuf[9] &= ~0x18; /* Reserved... not sure what goes here */
    pBuf[9] &= ~0x07; /* Set size local color table to 0 */
    
    // Actually write the data
    fwrite(pBuf, 10, 1, pCtx->pFp);
    
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
    int bufPos, int lzwSize, remBits;
    unsigned char pBuf[bufLen];
    
    // Sanitize arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    // Check that it was initialized
    ASSERT(pCtx->pFp, GFMRV_GIF_NOT_INITIALIZED);
    
    // Write LZW minimum size (i.e., number of bits + 1)
    lzwSize = pCtx->colorBits;
    // The algorithm says this is necessary, so...
    if (lzwSize == 1)
        lzwSize++;
    pBuf[0] = lzwSize;
    fwrite(pBuf, 1, 1, pCtx->pFp);
    
    // (GIF's) LZW requires one more bit from the start (stupid algorithm)
    lzwSize++;
    // Write LZW clean code
    rv = gfmGif_writeBitwiseWord(&remBit, pData, bufLen, dataPos, (1 << pCtx->colorBits), lzwSize);
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
    ASSERT(pCtx->pFp, GFMRV_GIF_NOT_INITIALIZED);
    
    // Write every sub-block
    while (len > 0) { 
        unsigned char curLen;
        
        // Set the current length to be written
        if (len > 0xff)
            curLen = 0xff;
        else
            curLen = (unsigned char)len;
        
        // Write the block's length
        fwrite(&curLen, 1, 1, pCtx->pFp);
        // Write the current data
        fwrite(pData, curLen, 1, pCtx->pFp);
        
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
 * @param  pCtx The game's context
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_GIF_NOT_INITIALIZED
 */
gfmRV gfmGif_writeComment(gfmGifExporter *pGif, gfmCtx *pCtx) {
    gfmRV rv;
    volatile int len;
    unsigned char c, *pOrg, *pTitle;
    
    // Sanitize arguments
    ASSERT(pGif, GFMRV_ARGUMENTS_BAD);
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    // Check that it was initialized
    ASSERT(pGif->pFp, GFMRV_GIF_NOT_INITIALIZED);
    
    // Get the game's title and organization
    rv = gfm_getTitle((char**)&pOrg, (char**)&pTitle, pCtx);
    ASSERT_NR(rv == GFMRV_OK);
    
    // Initialize the comment
    c = 0x21;
    fwrite(&c, 1, 1, pGif->pFp);
    c = 0xfe;
    fwrite(&c, 1, 1, pGif->pFp);
    
    // Write a 'header' for the comment
    rv = gfmGif_writeDataSubBlockStatic(pGif, "GIF made with GFraMe "
            gfmVersion"\n");
    ASSERT_NR(rv == GFMRV_OK);
    
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
    
    // Writes the block terminator
    c = 0x0;
    fwrite(&c, 1, 1, pGif->pFp);
    
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
    ASSERT(pCtx->pFp, GFMRV_GIF_NOT_INITIALIZED);
    
    // Write the trailer
    c = 0x3b;
    fwrite(&c, 1, 1, pCtx->pFp);
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

