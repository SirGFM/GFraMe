/**
 * @file src/core/sdl2/gfmTexture.c
 * 
 */
#include <GFraMe/gfmAssert.h>
#include <GFraMe/gfmError.h>
#include <GFraMe/gfmString.h>
#include <GFraMe/gfmUtils.h>
#include <GFraMe/gframe.h>
#include <GFraMe/core/gfmBackbuffer_bkend.h>
#include <GFraMe/core/gfmTexture_bkend.h>

#include <SDL2/SDL_render.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BMP_OFFSET_POS 0x0a
#define BMP_WIDTH_POS  0x12
#define BMP_HEIGHT_POS 0x16

struct stGFMTexture {
    /** The actual SDL texture */
    SDL_Texture *pTexture;
    /** Texture's width */
    int width;
    /** Texture's height */
    int height;
};

/** 'Exportable' size of gfmTexture */
const int sizeofGFMTexture = (int)sizeof(gfmTexture);

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
    gfmRV rv;
    SDL_Renderer *pRenderer;
    
    // Sanitize arguments
    ASSERT(pTex, GFMRV_ARGUMENTS_BAD);
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    ASSERT(width > 0, GFMRV_ARGUMENTS_BAD);
    ASSERT(height > 0, GFMRV_ARGUMENTS_BAD);
    // Check that the texture wasn't already initialized
    ASSERT(!pTex->pTexture, GFMRV_TEXTURE_ALREADY_INITIALIZED);
    // Check the dimensions
    ASSERT(gfmUtils_isPow2(width) == GFMRV_TRUE, GFMRV_TEXTURE_INVALID_WIDTH);
    ASSERT(gfmUtils_isPow2(height) == GFMRV_TRUE,
            GFMRV_TEXTURE_INVALID_HEIGHT);
    
    // Get the renderer
    rv = gfm_getBackbuffer(&pBbuf, pCtx);
    ASSERT_NR(rv == GFMRV_OK);
    rv = gfmBackbuffer_getContext((void**)&pRenderer, pBbuf);
    ASSERT_NR(rv == GFMRV_OK);
    // Create the texture
    pTex->pTexture = SDL_CreateTexture(pRenderer, SDL_PIXELFORMAT_ABGR8888,
            SDL_TEXTUREACCESS_STATIC, width, height);
    ASSERT(pTex->pTexture, GFMRV_INTERNAL_ERROR);
    
    pTex->width = width;
    pTex->height = height;
    
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
    char *pData, *pPath, pBuffer[4];
    FILE *pFile;
    gfmRV rv;
    gfmString *pStr;
    //int bytesInRow, height, i, irv, dataOffset, rowOffset, width;
    int bytesInRow, i, irv, rowOffset;
    volatile int height, dataOffset, width;
    
    // Zero variable that must be cleaned on error
    pData = 0;
    pFile = 0;
    
    // Sanitize arguments
    ASSERT(pTex, GFMRV_ARGUMENTS_BAD);
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    ASSERT(pFilename, GFMRV_ARGUMENTS_BAD);
    ASSERT(filenameLen > 0, GFMRV_ARGUMENTS_BAD);
    
    // Check the file extension (the dump and lazy way)
    ASSERT(pFilename[filenameLen - 4] == '.', GFMRV_TEXTURE_NOT_BITMAP);
    ASSERT(pFilename[filenameLen - 3] == 'b', GFMRV_TEXTURE_NOT_BITMAP);
    ASSERT(pFilename[filenameLen - 2] == 'm', GFMRV_TEXTURE_NOT_BITMAP);
    ASSERT(pFilename[filenameLen - 1] == 'p', GFMRV_TEXTURE_NOT_BITMAP);
    
    // Get the correct file path
    rv = gfm_getBinaryPath(&pStr, pCtx);
    ASSERT_NR(rv == GFMRV_OK);
    rv = gfmString_concatStatic(pStr, "assets/");
    ASSERT_NR(rv == GFMRV_OK);
    rv = gfmString_concat(pStr, pFilename, filenameLen);
    ASSERT_NR(rv == GFMRV_OK);
    rv = gfmString_getString(&pPath, pStr);
    ASSERT_NR(rv == GFMRV_OK);
    
    // Open the file
    pFile = fopen(pPath, "rb");
    ASSERT(pFile, GFMRV_TEXTURE_FILE_NOT_FOUND);
    
    // Get the offset to the image's "data section"
    fseek(pFile, BMP_OFFSET_POS, SEEK_SET);
    irv = fread(pBuffer, 4, 1, pFile);
    dataOffset = READ_UINT(pBuffer);
    
    // Get the image's dimensions
    fseek(pFile, BMP_HEIGHT_POS, SEEK_SET);
    irv = fread(pBuffer, 4, 1, pFile);
    height = READ_UINT(pBuffer);
    
    fseek(pFile, BMP_WIDTH_POS, SEEK_SET);
    irv = fread(pBuffer, 4, 1, pFile);
    width = READ_UINT(pBuffer);
    
    // Check if the image fit on the current texture
    if (pTex->pTexture) {
        ASSERT(pTex->width == width, GFMRV_TEXTURE_INVALID_WIDTH);
        ASSERT(pTex->height == height, GFMRV_TEXTURE_INVALID_HEIGHT);
    }
    
    // Alloc the data array
    pData = (char*)malloc(width*height*sizeof(char)*4);
    ASSERT(pData, GFMRV_ALLOC_FAILED);
    
    // Calculate how many bytes there are in a row of pixels
    bytesInRow = width * 3;
    rowOffset = bytesInRow % 4;
    bytesInRow += rowOffset;
    
    // Buffer the data (in the desired format)
    fseek(pFile, dataOffset, SEEK_SET);
    // Data is written starting by the last line
    i = width * (height - 1);
    // Start "alpha" with 0 (since the image is in RGB-24)
    pBuffer[3] = 0;
    while (1) {
        //int color, r, g, b, pos, n;
        int r, g, b, pos, n;
        volatile int color;
        
        // Read until the EOF
        n = fread(pBuffer, 3, 1, pFile);
        if (n == 0 || i < 0)
            break;
        
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
            fseek(pFile, rowOffset, SEEK_CUR);
        }
    }
    
    // Initialize the texture 
    if (!pTex->pTexture) {
        rv = gfmTexture_init(pTex, pCtx, width, height);
        ASSERT_NR(rv == GFMRV_OK);
    }
    
    // Load data into texture
    irv = SDL_UpdateTexture(pTex->pTexture, NULL, (const void*)pData,
            width*SDL_BYTESPERPIXEL(SDL_PIXELFORMAT_ARGB8888));
    ASSERT(irv == 0, GFMRV_INTERNAL_ERROR);
    irv = SDL_SetTextureBlendMode(pTex->pTexture, SDL_BLENDMODE_BLEND);
    ASSERT(irv == 0, GFMRV_INTERNAL_ERROR);
    
    rv = GFMRV_OK;
__ret:
    // Clean up memory
    if (pFile)
        fclose(pFile);
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

