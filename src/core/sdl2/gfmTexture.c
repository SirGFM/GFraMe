/**
 * @file src/core/sdl2/gfmTexture.c
 * 
 */
#include <GFraMe/gfmAssert.h>
#include <GFraMe/gfmError.h>
#include <GFraMe/gfmLog.h>
#include <GFraMe/gfmUtils.h>
#include <GFraMe/gframe.h>
#include <GFraMe/core/gfmBackbuffer_bkend.h>
#include <GFraMe/core/gfmFile_bkend.h>
#include <GFraMe/core/gfmTexture_bkend.h>

#include <SDL2/SDL_render.h>

#include <stdlib.h>
#include <string.h>

#define BMP_OFFSET_POS 0x0a
#define BMP_WIDTH_POS  0x12
#define BMP_HEIGHT_POS 0x16

#if 0
/**
 * Alloc a new texture
 * 
 * @param  ppCtx The alocated texture
 * @return       GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_ALLOC_FAILED
 */
gfmRV gfmTexture_getNew(gfmTexture **ppCtx) {
    gfmRV rv;
    
    // Sanitize arguments
    ASSERT(ppCtx, GFMRV_ARGUMENTS_BAD);
    ASSERT(!(*ppCtx), GFMRV_ARGUMENTS_BAD);
    
    // Try to alloc the texture
    *ppCtx = (gfmTexture*)malloc(sizeof(gfmTexture));
    ASSERT(*ppCtx, GFMRV_ALLOC_FAILED);
    
    // Initialize the object
    memset(*ppCtx, 0x0, sizeof(gfmTexture));
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Frees and cleans a previously allocated texture
 * 
 * @param  ppCtx The alocated texture
 * @return       GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_ALLOC_FAILED
 */
gfmRV gfmTexture_free(gfmTexture **ppCtx) {
    gfmRV rv;
    
    // Sanitize arguments
    ASSERT(ppCtx, GFMRV_ARGUMENTS_BAD);
    ASSERT(*ppCtx, GFMRV_ARGUMENTS_BAD);
    
    // Clean it up
    rv = gfmTexture_clean(*ppCtx);
    ASSERT_NR(rv == GFMRV_OK);
    
    // Free the memory
    free(*ppCtx);
    *ppCtx = 0;
    
    rv = GFMRV_OK;
__ret:
    return rv;
}
#endif

#if 0
/**
 * Initialize a texture so sprites can be loaded into it
 * 
 * NOTE: The texture's dimensions must be power of two (e.g., 256x256)
 * 
 * @param  pTex   The texture
 * @param  pCtx   The game's context
 * @param  width  The texture's width
 * @param  height The texture's height
 * @return        GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_INTERNAL_ERROR,
 *                GFMRV_TEXTURE_ALREADY_INITIALIZED,
 *                GFMRV_TEXTURE_INVALID_WIDTH, GFMRV_TEXTURE_INVALID_HEIGHT
 */
gfmRV gfmTexture_init(gfmTexture *pTex, gfmCtx *pCtx, int width, int height) {
    gfmBackbuffer *pBbuf;
    gfmLog *pLog;
    gfmRV rv;
    SDL_Renderer *pRenderer;
    
    // Sanitize arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    // Retrieve the logger
    rv = gfm_getLogger(&pLog, pCtx);
    ASSERT(rv == GFMRV_OK, rv);
    // Continue to sanitize arguments
    ASSERT_LOG(pTex, GFMRV_ARGUMENTS_BAD, pLog);
    ASSERT_LOG(width > 0, GFMRV_ARGUMENTS_BAD, pLog);
    ASSERT_LOG(height > 0, GFMRV_ARGUMENTS_BAD, pLog);
    // Check that the texture wasn't already initialized
    ASSERT_LOG(!pTex->pTexture, GFMRV_TEXTURE_ALREADY_INITIALIZED, pLog);
    // Check the dimensions
    ASSERT_LOG(gfmUtils_isPow2(width) == GFMRV_TRUE, GFMRV_TEXTURE_INVALID_WIDTH, pLog);
    ASSERT_LOG(gfmUtils_isPow2(height) == GFMRV_TRUE,
            GFMRV_TEXTURE_INVALID_HEIGHT, pLog);
    
    rv = gfmLog_log(pLog, gfmLog_info, "Initializing %ix%i texture...", width, height);
    ASSERT(rv == GFMRV_OK, rv);
    
    // Get the renderer
    rv = gfm_getBackbuffer(&pBbuf, pCtx);
    ASSERT_LOG(rv == GFMRV_OK, rv, pLog);
    rv = gfmBackbuffer_getContext((void**)&pRenderer, pBbuf);
    ASSERT_LOG(rv == GFMRV_OK, rv, pLog);
    // Create the texture
    pTex->pTexture = SDL_CreateTexture(pRenderer, SDL_PIXELFORMAT_ABGR8888,
            SDL_TEXTUREACCESS_STATIC, width, height);
    ASSERT_LOG(pTex->pTexture, GFMRV_INTERNAL_ERROR, pLog);
    
    pTex->width = width;
    pTex->height = height;
    
    rv = gfmLog_log(pLog, gfmLog_info, "Texture initialized!");
    ASSERT(rv == GFMRV_OK, rv);
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Clean up the texture
 * 
 * @param  pCtx The texture
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmTexture_clean(gfmTexture *pCtx) {
    gfmRV rv;
    
    // Sanitize arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    
    // Release any allocated texture
    if (pCtx->pTexture) {
        SDL_DestroyTexture(pCtx->pTexture);
    }
    pCtx->pTexture = 0;
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

#define READ_UINT(buffer) \
        ((0xff & buffer[0]) | ((buffer[1]<<8) & 0xff00) | \
        ((buffer[2]<<16) & 0xff0000) | ((buffer[3]<<24) & 0xff000000))

/**
 * Loads an image (24 bits, RGB bitmap) into a texture;
 * 
 * NOTE 1: If the texture hasn't been initialized, it will be;
 * 
 * NOTE 2: If the texture has been initialized, it won't be resized to fit image
 * (i.e., the image must be at most as big as the texture)
 * 
 * NOTE 3: The image's dimensions must be power of two (e.g., 256x256)
 * 
 * NOTE 4: The image must be in an 'assets' folder, in the same level as the
 * binary
 * 
 * @param  pTex        The texture
 * @param  pCtx        The game's context
 * @param  pFilename   The image's filename (must be a '.bmp')
 * @param  filenameLen The filename's length
 * @param  colorKey    Color to be treat as transparent (in RGB, 24 bits)
 * @return             GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_TEXTURE_NOT_BITMAP,
 *                     GFMRV_TEXTURE_FILE_NOT_FOUND,
 *                     GFMRV_TEXTURE_INVALID_WIDTH,
 *                     GFMRV_TEXTURE_INVALID_HEIGHT, GFMRV_ALLOC_FAILED,
 *                     GFMRV_INTERNAL_ERROR
 */
gfmRV gfmTexture_load(gfmTexture *pTex, gfmCtx *pCtx, char *pFilename,
        int filenameLen, int colorKey) {
    char *pData, pBuffer[4];
    gfmFile *pFile;
    gfmLog *pLog;
    gfmRV rv;
    //int bytesInRow, height, i, irv, dataOffset, rowOffset, width;
    int bytesInRow, i, irv, rowOffset;
    volatile int height, dataOffset, width;
    
    // Zero variable that must be cleaned on error
    pData = 0;
    pFile = 0;
    
    // Sanitize arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    // Retrieve the logger
    rv = gfm_getLogger(&pLog, pCtx);
    ASSERT(rv == GFMRV_OK, rv);
    // Continue to sanitize arguments
    ASSERT_LOG(pTex, GFMRV_ARGUMENTS_BAD, pLog);
    ASSERT_LOG(pFilename, GFMRV_ARGUMENTS_BAD, pLog);
    ASSERT_LOG(filenameLen > 0, GFMRV_ARGUMENTS_BAD, pLog);
    
    // Check the file extension (the dump and lazy way)
    ASSERT_LOG(pFilename[filenameLen - 4] == '.', GFMRV_TEXTURE_NOT_BITMAP, pLog);
    ASSERT_LOG(pFilename[filenameLen - 3] == 'b', GFMRV_TEXTURE_NOT_BITMAP, pLog);
    ASSERT_LOG(pFilename[filenameLen - 2] == 'm', GFMRV_TEXTURE_NOT_BITMAP, pLog);
    ASSERT_LOG(pFilename[filenameLen - 1] == 'p', GFMRV_TEXTURE_NOT_BITMAP, pLog);
    
    // Open the asset file
    rv = gfmFile_getNew(&pFile);
    ASSERT_LOG(rv == GFMRV_OK, rv, pLog);
    rv = gfmFile_openAsset(pFile, pCtx, pFilename, filenameLen, 0/*isText*/);
    ASSERT_LOG(rv == GFMRV_OK, rv, pLog);
    
    // Get the offset to the image's "data section"
    rv = gfmFile_rewind(pFile);
    ASSERT_LOG(rv == GFMRV_OK, rv, pLog);
    rv = gfmFile_seek(pFile, BMP_OFFSET_POS);
    ASSERT_LOG(rv == GFMRV_OK, rv, pLog);
    rv = gfmFile_readBytes(pBuffer, &irv, pFile, 4/*count*/);
    ASSERT_LOG(rv == GFMRV_OK, rv, pLog);
    ASSERT_LOG(irv == 4, GFMRV_READ_ERROR, pLog);
    
    dataOffset = READ_UINT(pBuffer);
    
    // Get the image's dimensions
    rv = gfmFile_rewind(pFile);
    ASSERT_LOG(rv == GFMRV_OK, rv, pLog);
    rv = gfmFile_seek(pFile, BMP_HEIGHT_POS);
    ASSERT_LOG(rv == GFMRV_OK, rv, pLog);
    rv = gfmFile_readBytes(pBuffer, &irv, pFile, 4/*count*/);
    ASSERT_LOG(rv == GFMRV_OK, rv, pLog);
    ASSERT_LOG(irv == 4, GFMRV_READ_ERROR, pLog);
    
    height = READ_UINT(pBuffer);
    
    rv = gfmFile_rewind(pFile);
    ASSERT_LOG(rv == GFMRV_OK, rv, pLog);
    rv = gfmFile_seek(pFile, BMP_WIDTH_POS);
    ASSERT_LOG(rv == GFMRV_OK, rv, pLog);
    rv = gfmFile_readBytes(pBuffer, &irv, pFile, 4/*count*/);
    ASSERT_LOG(rv == GFMRV_OK, rv, pLog);
    ASSERT_LOG(irv == 4, GFMRV_READ_ERROR, pLog);
    
    width = READ_UINT(pBuffer);
    
    rv = gfmLog_log(pLog, gfmLog_info, "Loading %ix%i image...", width, height);
    ASSERT(rv == GFMRV_OK, rv);
    
    // Check if the image fit on the current texture
    if (pTex->pTexture) {
        ASSERT_LOG(pTex->width == width, GFMRV_TEXTURE_INVALID_WIDTH, pLog);
        ASSERT_LOG(pTex->height == height, GFMRV_TEXTURE_INVALID_HEIGHT, pLog);
    }
    
    // Alloc the data array
    pData = (char*)malloc(width*height*sizeof(char)*4);
    ASSERT_LOG(pData, GFMRV_ALLOC_FAILED, pLog);
    
    // Calculate how many bytes there are in a row of pixels
    bytesInRow = width * 3;
    rowOffset = bytesInRow % 4;
    bytesInRow += rowOffset;
    
    // Buffer the data (in the desired format)
    rv = gfmFile_rewind(pFile);
    ASSERT_LOG(rv == GFMRV_OK, rv, pLog);
    rv = gfmFile_seek(pFile, dataOffset);
    ASSERT_LOG(rv == GFMRV_OK, rv, pLog);
    // Data is written starting by the last line
    i = width * (height - 1);
    // Start "alpha" with 0 (since the image is in RGB-24)
    pBuffer[3] = 0;
    while (1) {
        //int color, r, g, b, pos, n;
        int r, g, b, pos, n;
        volatile int color;
        
        // Read until the EOF
        rv = gfmFile_readBytes(pBuffer, &n, pFile, 3/*numBytes*/);
        ASSERT_LOG(rv == GFMRV_OK || rv == GFMRV_FILE_EOF_REACHED, rv, pLog);
        if (rv == GFMRV_FILE_EOF_REACHED) {
            break;
        }
        
        // Get the actual color
        color = READ_UINT(pBuffer);
        // Get each component
        r = color & 0xff;
        g = (color >> 8) & 0xff;
        b = (color >> 16) & 0xff;
        
        // Output the color
        pos = i * 4;
        if (color == colorKey) {
            pData[pos + 0] = 0x00;
            pData[pos + 1] = 0x00;
            pData[pos + 2] = 0x00;
            pData[pos + 3] = 0x00;
        }
        else {
            pData[pos + 0] = (char)b & 0xff;
            pData[pos + 1] = (char)g & 0xff;
            pData[pos + 2] = (char)r & 0xff;
            pData[pos + 3] =   0xff;
        }
        
        // Go to the next pixel in this row
        i++;
        // If a row was read, go back the current row and the "next" one
        // (actually the previous!)
        if (i % width == 0) {
            i -= width * 2;
            // Go to the next line on the file
            if (rowOffset != 0) {
                rv = gfmFile_seek(pFile, rowOffset);
                ASSERT_LOG(rv == GFMRV_OK, rv, pLog);
            }
        }
    }
    
    // Initialize the texture 
    if (!pTex->pTexture) {
        rv = gfmTexture_init(pTex, pCtx, width, height);
        ASSERT_LOG(rv == GFMRV_OK, rv, pLog);
    }
    
    // Load data into texture
    irv = SDL_UpdateTexture(pTex->pTexture, NULL, (const void*)pData,
            width*SDL_BYTESPERPIXEL(SDL_PIXELFORMAT_ARGB8888));
    ASSERT_LOG(irv == 0, GFMRV_INTERNAL_ERROR, pLog);
    irv = SDL_SetTextureBlendMode(pTex->pTexture, SDL_BLENDMODE_BLEND);
    ASSERT_LOG(irv == 0, GFMRV_INTERNAL_ERROR, pLog);
    
    rv = GFMRV_OK;
__ret:
    // Clean up memory
    gfmFile_free(&pFile);
    if (pData)
        free(pData);
    
    return rv;
}

/**
 * Get a texture's internal representation
 * 
 * @param  ppCtx The returned context
 * @param  pTex  The texture
 * @return       GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_TEXTURE_NOT_INITIALIZED
 */
gfmRV gfmTexture_getContext(void **ppCtx, gfmTexture *pTex) {
    gfmRV rv;
    
    // Sanitize arguments
    ASSERT(pTex, GFMRV_ARGUMENTS_BAD);
    ASSERT(ppCtx, GFMRV_ARGUMENTS_BAD);
    // Check that the texture was initialized
    ASSERT(pTex->pTexture, GFMRV_TEXTURE_NOT_INITIALIZED);
    
    // Return the context
    *ppCtx = pTex->pTexture;
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Get the texture's dimensions
 * 
 * @param  pWidth  The texture's width
 * @param  pHeight The texture's height
 * @param  pCtx    The texture
 * @return         GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_TEXTURE_NOT_INITIALIZED
 */
gfmRV gfmTexture_getDimensions(int *pWidth, int *pHeight, gfmTexture *pCtx) {
    gfmRV rv;
    
    // Sanitize arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    ASSERT(pWidth, GFMRV_ARGUMENTS_BAD);
    ASSERT(pHeight, GFMRV_ARGUMENTS_BAD);
    // Check that the texture was initialized
    ASSERT(pCtx->pTexture, GFMRV_TEXTURE_NOT_INITIALIZED);
    
    // Return the dimensions
    *pWidth = pCtx->width;
    *pHeight = pCtx->height;
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

#endif

