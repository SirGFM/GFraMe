/**
 * @file src/core/common/gfmGifExporter.c
 * 
 * Module that exports both GIF images and animations; This implementation
 * requires pthread!!
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

#include <pthread.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/** Declare the 'trie nodes array' type */
gfmGenArr_define(gfmTrie);

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
    /** List to easily store dictionary nodes */
    gfmGenArr_var(gfmTrie, pTries);
    /** Dictionary root */
    gfmTrie *pDict;
    /** The game's 'organization' */
    unsigned char *pOrg;
    /** The game's title */
    unsigned char *pTitle;
    
    /** Variable used by the thread */
    
    /** The thread handle */
    pthread_t threadHnd;
    /** Output file */
    FILE *pOut;
    /** The value returned from the thread */
    gfmRV threadRV;
    /** Path were the image should be stored (just a reference!) */
    gfmString *pImagePath;
    /** Delay, in hundredths of a second (60fps should be rounded down to 50) */
    int animDelay;
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
    
    /** Stuff used only by the LZW */
    
    /** Minimum size in bits... */
    int lzwMinSize;
    /** Current number of bits-per-lzwCode */
    int lzwCurSize;
    /** Next code to be inserted on the dictionary */
    int lzwNextCode;
    /** Position in the buffer, in bits */
    int lzwBufBitPos;
    /** Position in the buffer, in bytes */
    int lzwBufBytePos;
    /** Number of bytes used in the lzw buffer */
    int lzwBufUsed;
    /** Number of bytes in the lzw buffer */
    int lzwBufLen;
    /** Buffer to store the lzw compressed image */
    unsigned char *pLzwBuf;
};

/**
 * Generates a GIF image from the passed context
 * 
 * @param pCtx The GIF context
 */
static void* _gfmGif_threadHandler(void *pCtx);

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
    
    // TODO If a thread is active, signal it to stop and wait
    gfmGif_waitExport(pCtx);
    
    // TODO remove any temp file
    
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
    if (pCtx->pLzwBuf) {
        free(pCtx->pLzwBuf);
        pCtx->pLzwBuf = 0;
    }
    gfmGenArr_clean(pCtx->pTries, gfmTrie_free);
    if (pCtx->pOut) {
        fclose(pCtx->pOut);
        pCtx->pOut = 0;
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
    int ups, dps;
    
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
    
    // Get the current fps
    rv = gfm_getStateFrameRate(&ups, &dps, pCtx);
    ASSERT_NR(rv == GFMRV_OK);
    // Set the default delay
    pGif->animDelay = 100 / dps;
    // Round it up, if necessary
    if ((1000 / dps) % 10 >= 5)
        pGif->animDelay++;
    
    // Get the game's title and organization
    rv = gfm_getTitle((char**)&(pGif->pOrg), (char**)&(pGif->pTitle), pCtx);
    ASSERT_NR(rv == GFMRV_OK);
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Check whether the current export finished
 * 
 * @param  pCtx The GIF exporter
 * @return      GFMRV_ARGUMENTS_BAD, GFMRV_GIF_OPERATION_NOT_ACTIVE, GFMRV_TRUE,
 *              GFMRV_FALSE
 */
gfmRV gfmGif_didExport(gfmGifExporter *pCtx) {
    gfmRV rv;
    
    // Sanitize arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    // Check that the operation was initialized
    ASSERT(pCtx->isActive, GFMRV_GIF_OPERATION_NOT_ACTIVE);
    
    // Check that the thread finished
    ASSERT(pCtx->threadRV != GFMRV_GIF_THREAD_IS_RUNNING, GFMRV_FALSE);
    
    // Clean up resources and enable this to be reused
    rv = gfmGif_waitExport(pCtx);
    ASSERT_NR(rv == GFMRV_OK);
    
    rv = GFMRV_TRUE;
__ret:
    return rv;
}

/**
 * Wait until the current export finished
 * 
 * @param  pCtx The GIF exporter
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_GIF_OPERATION_NOT_ACTIVE
 */
gfmRV gfmGif_waitExport(gfmGifExporter *pCtx) {
    gfmRV rv;
    
    // Sanitize arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    // Check that the operation was initialized
    ASSERT(pCtx->isActive, GFMRV_GIF_OPERATION_NOT_ACTIVE);
    
    // Initialize the return value
    rv = GFMRV_OK;
    
    // Check that the thread was running
    if (pCtx->threadHnd) {
        gfmGifExporter *pGif;
        int irv;
        
        // Wait until the thread exits
        irv = pthread_join(pCtx->threadHnd, (void**)(&pGif));
        ASSERT(irv == 0, GFMRV_INTERNAL_ERROR);
        // Cleans the handle
        pCtx->threadHnd = 0;
        
        // Retrieve the error code from the object returned by the thread
        ASSERT(pGif, GFMRV_INTERNAL_ERROR);
        rv = pGif->threadRV;
    }
    
    // Set this as inactive
    pCtx->isActive = 0;
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
    int irv;
    pthread_attr_t attr;
    
    // Sanitize arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    ASSERT(pPath, GFMRV_ARGUMENTS_BAD);
    // Check that the operation is active
    ASSERT(pCtx->isActive, GFMRV_GIF_OPERATION_NOT_ACTIVE);
    // Check that there is only one frame
    ASSERT(pCtx->frameCount == 1, GFMRV_GIF_TOO_MANY_FRAMES);
    // Check that there's no thread
    ASSERT(pCtx->threadRV != GFMRV_GIF_THREAD_IS_RUNNING,
            GFMRV_GIF_THREAD_IS_RUNNING);
    
    // Set the path (so it can be seen by the thread)
    pCtx->pImagePath = pPath;
    pCtx->pOut = 0;
    
    // Set the thread attributes
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
    // Create thread to handle this
    irv = pthread_create(&(pCtx->threadHnd), &attr, _gfmGif_threadHandler,
            (void*)pCtx);
    ASSERT(irv == 0, GFMRV_INTERNAL_ERROR);
    
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
gfmRV gfmGif_exportAnimation(gfmGifExporter *pCtx, gfmString *pPath) {
    gfmRV rv;
    int irv;
    pthread_attr_t attr;
    
    // Sanitize arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    ASSERT(pPath, GFMRV_ARGUMENTS_BAD);
    // Check that the operation is active
    ASSERT(pCtx->isActive, GFMRV_GIF_OPERATION_NOT_ACTIVE);
    // Check that there are some frames
    ASSERT(pCtx->frameCount > 1, GFMRV_GIF_TOO_MANY_FRAMES);
    // Check that there's no thread
    ASSERT(pCtx->threadRV != GFMRV_GIF_THREAD_IS_RUNNING,
            GFMRV_GIF_THREAD_IS_RUNNING);
    
    // Set the path (so it can be seen by the thread)
    pCtx->pImagePath = pPath;
    pCtx->pOut = 0;
    
    // Set the thread attributes
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
    // Create thread to handle this
    irv = pthread_create(&(pCtx->threadHnd), 0, _gfmGif_threadHandler, (void*)pCtx);
    ASSERT(irv == 0, GFMRV_INTERNAL_ERROR);
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Generates a GIF image from the passed context
 * 
 * @param pCtx The GIF context
 */
static void* _gfmGif_threadHandler(void *pArg) {
    char *pPath;
    gfmGifExporter *pGif;
    gfmRV rv;
    
    // Initialize this
    pGif = 0;
    
    // Sanitize arguments
    ASSERT(pArg, GFMRV_ARGUMENTS_BAD);
    
    // Retrieve the GIF context
    pGif = (gfmGifExporter *)pArg;
    
    // Check if the operation is active
    ASSERT(pGif->isActive, GFMRV_GIF_OPERATION_NOT_ACTIVE);
    // Check that the path was set
    ASSERT(pGif->pImagePath, GFMRV_GIF_PATH_NOT_SET);
    
    // Set the thread as active
    pGif->threadRV = GFMRV_GIF_THREAD_IS_RUNNING;
    
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
    
    // If it's an animation, write the Netscape application block
    if (pGif->frameCount > 1) {
        rv = gfmGif_writeNAB(pGif);
        ASSERT_NR(rv == GFMRV_OK);
    }
    
    // Write Comment extension
    rv = gfmGif_writeComment(pGif);
    ASSERT_NR(rv == GFMRV_OK);
    
    // Write each frame
    pGif->curFrame = 0;
    while (pGif->curFrame < pGif->frameCount) {
        // Buffer this frame
        rv = gfmGif_readFrame(pGif);
        ASSERT_NR(rv == GFMRV_OK);
        
        // Reset the dictionary
        gfmGenArr_reset(pGif->pTries);
        pGif->pDict = 0;
        
        // Write the frame
        rv = gfmGif_writeFrame(pGif);
        ASSERT_NR(rv == GFMRV_OK);
        
        // Go to the next frame
        pGif->curFrame++;
    }
    
    // Write Trailer
    rv = gfmGif_writeTrailer(pGif);
    ASSERT_NR(rv == GFMRV_OK);
    
    rv = GFMRV_OK;
__ret:
    // Set the return and close the generated image
    if (rv != GFMRV_ARGUMENTS_BAD) {
        pGif->threadRV = rv;
        
        if (pGif->pOut) {
            fclose(pGif->pOut);
            pGif->pOut = 0;
        }
    }
    
    // Return to the thread
    pthread_exit(pGif);
}

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
    int i, len, x, y;
    
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
    x = 0;
    y = pCtx->height - 1;
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
        pCtx->pData[x + y * pCtx->width] = index;
        //pCtx->pData[i] = index;
        
        // Go to the next color
        i++;
        x++;
        // Write the image from the last line to the first
        if (x >= pCtx->width) {
            x = 0;
            y--;
        }
    }
    // If the palette is full, add another bit (go figure, GIF is stupid)
    if (pCtx->colorCount == pCtx->totalColorCount) {
        pCtx->totalColorCount *= 2;
        pCtx->colorBits++;
    }
    // Update how many bytes there are in the image buffer
    pCtx->dataUsed = len;
    
    rv = GFMRV_OK;
__ret:
    // If there's a file, close it and delete it
    if (pFp) {
        fclose(pFp);
        remove(pFramePath);
    }
    
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
    pBuf[0] = pCtx->width & 0xff;        /* Set width lsb */
    pBuf[1] = (pCtx->width >> 8) & 0xff; /* Set width msb */
    pBuf[2] = pCtx->height & 0xff;        /* Set height lsb */
    pBuf[3] = (pCtx->height >> 8) & 0xff; /* Set height msb */
    pBuf[4]  = 0x00; /* Clean this bitfield */
    pBuf[4] |= 0x00; /* Remove any global palette (bit 0x80) */
    pBuf[4] |= 0x70; /* Set the source as 8 bits per color (bits 0x70) */
    pBuf[4] |= 0x00; /* Whether the palette is sorted (bit 0x08) */
    pBuf[4] |= 0x00; /* Size of the global color table (bits 0x07) */
    pBuf[5] = 0; /* Set the bg color (should be useless, but still...) */
    pBuf[6] = 0; /* Set the pixel aspect ratio (should be perfectly square!) */
    
    // Actually write the data
    fwrite(pBuf, 7, 1, pCtx->pOut);
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Write the Netscape Applicaton Block (so the animation loops infinitely)
 * 
 * @param  pCtx The GIF exporter
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_GIF_NOT_INITIALIZED
 */
gfmRV gfmGif_writeNAB(gfmGifExporter *pCtx) {
    gfmRV rv;
    unsigned char pBuf[19];
    
    // Sanitize arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    // Check that it was initialized
    ASSERT(pCtx->pOut, GFMRV_GIF_NOT_INITIALIZED);
    
    pBuf[0] = 0x21; /** Extension introducer (must be 0x21) */
    pBuf[1] = 0xff; /** Application extension label (must be 0xff) */
    pBuf[2]  =  11; /** Size of this block */
    pBuf[3]  = 'N'; /** Application identifier (byte 1 of 8) */
    pBuf[4]  = 'E'; /** Application identifier (byte 2 of 8) */
    pBuf[5]  = 'T'; /** Application identifier (byte 3 of 8) */
    pBuf[6]  = 'S'; /** Application identifier (byte 4 of 8) */
    pBuf[7]  = 'C'; /** Application identifier (byte 5 of 8) */
    pBuf[8]  = 'A'; /** Application identifier (byte 6 of 8) */
    pBuf[9]  = 'P'; /** Application identifier (byte 7 of 8) */
    pBuf[10] = 'E'; /** Application identifier (byte 8 of 8) */
    pBuf[11] = '2'; /** Application Authentication Code (byte 1 of 3) */
    pBuf[12] = '.'; /** Application Authentication Code (byte 2 of 3) */
    pBuf[13] = '0'; /** Application Authentication Code (byte 3 of 3) */
    pBuf[14] = 3;   /** Size of this next sub-block */
    pBuf[15] = 1;   /** Data sub-block index (must be 1) */
    pBuf[16] = 0;   /** Number of repetition lsb (0 = infinite) */
    pBuf[17] = 0;   /** Number of repetition msb (0 = infinite) */
    pBuf[18] = 0;   /** Block terminator */
    
    // Actually write the data
    fwrite(pBuf, 19, 1, pCtx->pOut);
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

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
    
    // Write the Graphic Control Extension (if it's an animation)
    if (pCtx->frameCount > 1) {
        rv = gfmGif_writeGCE(pCtx);
        ASSERT_NR(rv == GFMRV_OK);
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
 * Write the frame's Graphic Control Extension
 * 
 * @param  pCtx The GIF exporter
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_GIF_NOT_INITIALIZED
 */
gfmRV gfmGif_writeGCE(gfmGifExporter *pCtx) {
    gfmRV rv;
    unsigned char pBuf[8];
    
    // Sanitize arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    // Check that it was initialized
    ASSERT(pCtx->pOut, GFMRV_GIF_NOT_INITIALIZED);
    
    pBuf[0] = 0x21; /* Write the 'Extension Introducer */
    pBuf[1] = 0xf9; /* Set this as a GCE block */
    pBuf[2] = 4;    /* This block's size */
    pBuf[3]  = 0x00; /* Clean this bit field */
    pBuf[3] |= 0x00; /* Reserved (bits 0xe0) */
    pBuf[3] |= 0x00; /* Set disposal method as undefined (bits 0x1C) */
    pBuf[3] |= 0x00; /* Whether there should be user input (bit 0x02) */
    pBuf[3] |= 0x00; /* Whether there's a transparent color (bit 0x01) */
    pBuf[4] = pCtx->animDelay & 0xff;        /* Delay's lsb */
    pBuf[5] = (pCtx->animDelay >> 8) & 0xff; /* Delay's lsb */
    pBuf[6] = 0; /* Transparent color index */
    pBuf[7] = 0; /* Block terminator */
    
    // Actually write the data
    fwrite(pBuf, 8, 1, pCtx->pOut);
    
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
    gfmTrie *pCurNode;
    int i;
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
    pBuf[9] = 0;     /* Clean this bitfield */
    pBuf[9] |= 0x80; /* Set a local color table flag */
    pBuf[9] |= 0x00; /* Whether it's interlaced (bit 0x40) */
    pBuf[9] |= 0x00; /* Wether it's sorted (bit 0x20) */
    pBuf[9] |= 0x00; /* Reserved... not sure what goes here (bits 0x18) */
    pBuf[9] |= (pCtx->colorBits - 1) & 0x07; /* Size of palette (bits 0x07) */
    
    // Actually write the data
    fwrite(pBuf, 10, 1, pCtx->pOut);
    
    // Write the local palette
    i = 0;
    while (i < pCtx->colorCount) {
        // Set the palette buffer (in R,G,B order)
        pBuf[0] = (pCtx->pPalette[i] >> 16) & 0xff;
        pBuf[1] = (pCtx->pPalette[i] >> 8) & 0xff;
        pBuf[2] = pCtx->pPalette[i] & 0xff;
        
        // Write it
        fwrite(pBuf, 1, 3, pCtx->pOut);
        
        i++;
    }
    // Fill the empty spaces
    while (i < pCtx->totalColorCount) {
        // Write the last entry to fill the palette
        fwrite(pBuf, 1, 3, pCtx->pOut);
        
        i++;
    }
    
    // Get the dictionary's root
    gfmGenArr_getNextRef(gfmTrie, pCtx->pTries, pCtx->totalColorCount,
            pCtx->pDict, gfmTrie_getNew);
    gfmGenArr_push(pCtx->pTries);
    // Initialize it
    i = 0;
    rv = gfmTrie_init(pCtx->pDict, i, i);
    ASSERT_NR(rv == GFMRV_OK);
    // Get the root
    pCurNode = pCtx->pDict;
    
    // Initialize the dictionary
    i = 1;
    while (i < pCtx->totalColorCount) {
        gfmTrie *pNextNode;
        
        // Get a new node
        gfmGenArr_getNextRef(gfmTrie, pCtx->pTries, pCtx->totalColorCount,
                pNextNode, gfmTrie_getNew);
        gfmGenArr_push(pCtx->pTries);
        
        // Insert it as a sibling
        rv = gfmTrie_insertSibling(pCurNode, pNextNode, i, i);
        ASSERT_NR(rv == GFMRV_OK);
        
        // Go to the next node
        pCurNode = pNextNode;
        i++;
    }
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Write a word of a given length to a buffer of bits
 * 
 * @param  pCtx   The GIF exporter
 * @param  word   Word to be written to the buffer
 * @return        GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmGif_writeBitwiseWord(gfmGifExporter *pCtx, int word) {
    gfmRV rv;
    int curLen;
    
    // Sanitize arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    
    // Check if the buffer was initialized (and do so if needed)
    if (!pCtx->pLzwBuf) {
        pCtx->lzwBufLen = 255;
        
        pCtx->pLzwBuf = (unsigned char*)malloc(sizeof(unsigned char)
                * pCtx->lzwBufLen);
        ASSERT(pCtx->pLzwBuf, GFMRV_ALLOC_FAILED);
        
        // Clear the first byte
        pCtx->pLzwBuf[0] = 0;
    }
    
    curLen = pCtx->lzwCurSize;
    while (curLen > 0) {
        unsigned char c;
        int bitsWritten, bytesWritten;
        
        // Write part of the word to the buffer
        c = (word << pCtx->lzwBufBitPos) & 0xff;
        pCtx->pLzwBuf[pCtx->lzwBufBytePos] |= c;
        
        // Get the maximum number of bits written
        bitsWritten = 8 - pCtx->lzwBufBitPos;
        if (bitsWritten >= curLen) {
            // The whole word was written
            
            // Update the position moving the word
            pCtx->lzwBufBitPos += curLen;
        }
        else {
            // There're some bits missing
            
            // Update the position moving the written bits
            pCtx->lzwBufBitPos += bitsWritten;
            // Remove the already written bits
            word >>= bitsWritten;
        }
        // Update how many bits there are left
        curLen -= bitsWritten;
        
        // Check how many bytes will be moved
        bytesWritten = 0;
        if (pCtx->lzwBufBitPos >= 8) {
            // Only one byte is written at a time
            pCtx->lzwBufBitPos -= 8;
            bytesWritten = 1;
            
        }
        
        // Expand the buffer, as necessary
        if (pCtx->lzwBufBytePos + bytesWritten > pCtx->lzwBufLen) {
            pCtx->lzwBufLen *= 2;
            
            pCtx->pLzwBuf = (unsigned char*)realloc(pCtx->pLzwBuf,
                   sizeof(unsigned char) * pCtx->lzwBufLen * 2);
            ASSERT(pCtx->pLzwBuf, GFMRV_ALLOC_FAILED);
        }
        
        if (bytesWritten) {
            // Zero the new byte
            pCtx->pLzwBuf[pCtx->lzwBufBytePos + 1] = 0;
            // Update the position, in bytes
            pCtx->lzwBufBytePos++;
        }
    }
    
    // Update how many bytes have been used
    pCtx->lzwBufUsed = pCtx->lzwBufBytePos + 1;
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Write the GIF's Image data (compressed with variable length LZW)
 * 
 * @param  pCtx The GIF exporter
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_GIF_NOT_INITIALIZED,
 *              GFMRV_ALLOC_FAILED
 */
gfmRV gfmGif_writeLZWData(gfmGifExporter *pCtx) {
    gfmRV rv;
    int i;
    unsigned char c;
    
    // Sanitize arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    // Check that it was initialized
    ASSERT(pCtx->pOut, GFMRV_GIF_NOT_INITIALIZED);
    
    /* Note to future self: The GIF documentation is shit! Although it says
       "[...] Normally this will be the same as the number of color bits [...]
       This code size value also implies that the compression codes must start
       out one bit longer.", the minimum size must be 'pCtx->colorBits + 1' and
       you MUST use 'pCtx->colorBits + 2' as the minimum word... FUCK whoever
       wrote that shit! */
    
    // Get the LZW minimum code size
    pCtx->lzwMinSize = pCtx->colorBits;
    // The algorithm says this is necessary, so...
    if (pCtx->lzwMinSize == 1) {
        pCtx->lzwMinSize++;
    }
    // Write LZW minimum size (i.e., number of bits in palette)
    c = pCtx->lzwMinSize;
    fwrite(&c, 1, 1, pCtx->pOut);
    
    // Initialize the write operation
    pCtx->lzwCurSize = pCtx->lzwMinSize + 1;
    // Clean the buffer
    pCtx->lzwBufBitPos = 0;
    pCtx->lzwBufBytePos = 0;
    pCtx->lzwBufUsed = 0;
    // Set the first code to be added
    pCtx->lzwNextCode = (1 << pCtx->lzwMinSize) + 2;
    
    // Clear the buffer's first byte, if any
    if (pCtx->pLzwBuf)
        pCtx->pLzwBuf[0] = 0;
    // Write LZW clean code
    rv = gfmGif_writeBitwiseWord(pCtx, 1 << pCtx->lzwMinSize);
    ASSERT_NR(rv == GFMRV_OK);
    
    // Write compressed Image's data
    i = 0;
    while (i < pCtx->dataUsed) {
        gfmRV status;
        /** Points to end of the longest sequence (and current code) */
        gfmTrie *pAnchor;
        /** Used as temporary, both to search and to insert at the anchor */
        gfmTrie *pNode;
        /** Only used as the new child or sibling node */
        gfmTrie *pNewNode;
        /** Last code found */
        int curCode;
        
        // Get the longest string possible
        pAnchor = 0;
        pNode = pCtx->pDict;
        while (1) {
            // Search the current 
            rv = gfmTrie_searchSiblings(&pNode, pNode, pCtx->pData[i]);
            if (rv == GFMRV_TRIE_KEY_NOT_FOUND)
                break;
            ASSERT_NR(rv == GFMRV_OK);
            // Update the anchor
            pAnchor = pNode;
            // Update the current character being read
            i++;
            if (i >= pCtx->dataUsed)
                break;
            // Search through its child
            rv = gfmTrie_getChild(&pNode, pAnchor);
            if (rv == GFMRV_TRIE_IS_LEAF)
                break;
            ASSERT_NR(rv == GFMRV_OK);
        }
        status = rv;
        // Check that a valid sequence was found
        ASSERT(pAnchor, GFMRV_FUNCTION_FAILED);
        
        // Create a new node
        gfmGenArr_getNextRef(gfmTrie, pCtx->pTries, 8/*INC*/,
                pNewNode, gfmTrie_getNew);
        gfmGenArr_push(pCtx->pTries);
        
        if (status == GFMRV_TRIE_KEY_NOT_FOUND) {
            // Add it as a sibling
            rv = gfmTrie_insertSibling(pNode, pNewNode, pCtx->pData[i],
                    pCtx->lzwNextCode);
            ASSERT_NR(rv == GFMRV_OK);
        }
        else if (status == GFMRV_TRIE_IS_LEAF) {
            // Add it as a child
            rv = gfmTrie_insertChild(pNode, pNewNode, pCtx->pData[i],
                    pCtx->lzwNextCode);
            ASSERT_NR(rv == GFMRV_OK);
        }
        else if (i < pCtx->dataUsed) {
            // Shouldn't happen, but...
            ASSERT(0, GFMRV_FUNCTION_FAILED);
        }
        
        // Retrieve the current code
        rv = gfmTrie_getValue(&curCode, pAnchor);
        ASSERT_NR(rv == GFMRV_OK);
        
        // Write the current code
        rv = gfmGif_writeBitwiseWord(pCtx, curCode);
        ASSERT_NR(rv == GFMRV_OK);
        
        // Check if the number of bits-per-code should be expanded
        if (pCtx->lzwNextCode > (1 << pCtx->lzwCurSize) - 1) {
            pCtx->lzwCurSize++;
            // LZW code size can't be greater than 12 bits
            ASSERT(pCtx->lzwCurSize <= 12, GFMRV_GIF_FAILED_TO_COMPRESS);
        }
        pCtx->lzwNextCode++;
    }
    // Write LZW end code (LZW clean code + 1)
    rv = gfmGif_writeBitwiseWord(pCtx, (1 << pCtx->lzwMinSize) + 1);
    ASSERT_NR(rv == GFMRV_OK);
    
    // Write the compressed data
    rv = gfmGif_writeDataSubBlock(pCtx, pCtx->pLzwBuf, pCtx->lzwBufUsed);
    // Write block Terminator (0x00)
    c = 0x00;
    fwrite(&c, 1, 1, pCtx->pOut);
    
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
    unsigned char c;
    
    // Sanitize arguments
    ASSERT(pGif, GFMRV_ARGUMENTS_BAD);
    // Check that it was initialized
    ASSERT(pGif->pOut, GFMRV_GIF_NOT_INITIALIZED);
    
    // Initialize the comment
    c = 0x21;
    fwrite(&c, 1, 1, pGif->pOut);
    c = 0xfe;
    fwrite(&c, 1, 1, pGif->pOut);
    
    // Write a 'header' for the comment
    rv = gfmGif_writeDataSubBlockStatic(pGif, "GIF made with GFraMe "
            gfmVersion"\n");
    ASSERT_NR(rv == GFMRV_OK);
    
    // Write the game's title
    // Get the organization's length
    len = 0;
    while (pGif->pOrg[len] != '\0') len++;
    // Write the organization
    rv = gfmGif_writeDataSubBlock(pGif, pGif->pOrg, len);
    ASSERT_NR(rv == GFMRV_OK);
    
    // Get the title's length
    len = 0;
    while (pGif->pTitle[len] != '\0') len++;
    // Write the title
    rv = gfmGif_writeDataSubBlock(pGif, pGif->pTitle, len);
    ASSERT_NR(rv == GFMRV_OK);
    
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

