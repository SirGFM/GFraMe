/**
 * @file include/GFraMe/core/gfmTexture_bkend.h
 * 
 * 
 */
#ifndef __GFMTEXTURE_STRUCT__
#define __GFMTEXTURE_STRUCT__

/** "Export" the texture structure's type */
typedef struct stGFMTexture gfmTexture;

#endif /* __GFMTEXTURE_STRUCT__ */

#ifndef __GFMTEXTURE_BKEND_H__
#define __GFMTEXTURE_BKEND_H__

#include <GFraMe/gfmError.h>
#include <GFraMe/gframe.h>

/** 'Exportable' size of gfmTexture */
extern const int sizeofGFMTexture;

/**
 * Alloc a new texture
 * 
 * @param  ppCtx The alocated texture
 * @return       GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_ALLOC_FAILED
 */
gfmRV gfmTexture_getNew(gfmTexture **ppCtx);

/**
 * Frees and cleans a previously allocated texture
 * 
 * @param  ppCtx The alocated texture
 * @return       GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_ALLOC_FAILED
 */
gfmRV gfmTexture_free(gfmTexture **ppCtx);

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
gfmRV gfmTexture_init(gfmTexture *pTex, gfmCtx *pCtx, int width, int height);

/**
 * Clean up the texture
 * 
 * @param  pCtx The texture
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmTexture_clean(gfmTexture *pCtx);

/**
 * Loads an image (24 bits RGB, bitmap) into a texture; the image's filename
 * must be a static buffer
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
 * @param  pTex      The texture
 * @param  pCtx      The game's context
 * @param  pFilename Statically allocated filename's title (i.e., char name[])
 * @param  colorKey    Color to be treat as transparent (in ARGB, 32 bits)
 */
#define gfmTexture_loadStatic(pTex, pCtx, pFilename, colorKey) \
        gfmTexture_load(pTex, pCtx, pFilename, sizeof(pFilename)-1, colorKey)

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
        int filenameLen, int colorKey);

/**
 * Get a texture's internal representation
 * 
 * @param  ppCtx The returned context
 * @param  pTex  The texture
 * @return       GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_TEXTURE_NOT_INITIALIZED
 */
gfmRV gfmTexture_getContext(void **ppCtx, gfmTexture *pTex);

/**
 * Get the texture's dimensions
 * 
 * @param  pWidth  The texture's width
 * @param  pHeight The texture's height
 * @param  pCtx  The texture
 * @return       GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_TEXTURE_NOT_INITIALIZED
 */
gfmRV gfmTexture_getDimensions(int *pWidth, int *pHeight, gfmTexture *pCtx);

#endif /* __GFMTEXTURE_BKEND_H__ */

