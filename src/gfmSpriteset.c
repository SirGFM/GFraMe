/**
 * @file include/GFraMe/gfmSpriteset.h
 * 
 * Module to break up a texture into tiles
 */
#include <GFraMe/gfmAssert.h>
#include <GFraMe/gfmError.h>
#include <GFraMe/gfmSpriteset.h>
#include <GFraMe/gfmUtils.h>
#include <GFraMe/gframe.h>
#include <GFraMe/core/gfmTexture_bkend.h>

#include <stdlib.h>
#include <string.h>

/** The gfmSpriteset structure */
struct stGFMSpriteset {
    /** Referencing texture pointer */
    gfmTexture *pTex;
    /** Referencing cached texture */
    int iTex;
    /** Texture's width */
    int texWidth;
    /** Texture's height */
    int texHeight;
    /** Tile's width */
    int tileWidth;
    /** Tile's height */
    int tileHeight;
    /** Number of rows in spriteset */
    int rows;
    /** Number of columns in spriteset */
    int columns;
    /** Total number of tiles in spriteset */
    int tileCount;
};

/** 'Exportable' size of gfmSpritese */
const int sizeofGFMSpriteset = (int)sizeof(gfmSpriteset);

/**
 * Alloc a new spriteset
 * 
 * @param  ppCtx The allocated object
 * @return       GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_ALLOC_FAILED
 */
gfmRV gfmSpriteset_getNew(gfmSpriteset **ppCtx) {
    gfmRV rv;
    
    // Sanitize arguments
    ASSERT(ppCtx, GFMRV_ARGUMENTS_BAD);
    ASSERT(!(*ppCtx), GFMRV_ARGUMENTS_BAD);
    
    // Alloc and clean the spriteset
    *ppCtx = (gfmSpriteset*)malloc(sizeof(gfmSpriteset));
    ASSERT(*ppCtx, GFMRV_ALLOC_FAILED);
    memset(*ppCtx, 0x0, sizeof(gfmSpriteset));
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Free and clean a spriteset
 * 
 * @param  ppCtx The object to be freed
 * @return       GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmSpriteset_free(gfmSpriteset **ppCtx) {
    gfmRV rv;
    
    // Sanitize arguments
    ASSERT(ppCtx, GFMRV_ARGUMENTS_BAD);
    ASSERT(*ppCtx, GFMRV_ARGUMENTS_BAD);
    
    // No need to actually clean it up (as there's no allocated member)
    
    // Free it up
    free(*ppCtx);
    *ppCtx = 0;
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Initialize the spriteset from a texture
 * 
 * @param  pCtx       The spriteset
 * @param  pTex       The texture
 * @param  tileWidth  The width of each tile
 * @param  tileHeight The height of each tile
 * @return            GFMRV_OK, GFMRV_ARGUMENTS_BAD,
 *                    GFMRV_SPRITESET_INVALID_WIDTH,
 *                    GFMRV_SPRITESET_INVALID_HEIGHT,
 *                    GFMRV_TEXTURE_NOT_INITIALIZED
 */
gfmRV gfmSpriteset_init(gfmSpriteset *pCtx, gfmTexture *pTex, int tileWidth,
        int tileHeight) {
    gfmRV rv;
    int width, height;
    
    // Sanitize arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    ASSERT(pTex, GFMRV_ARGUMENTS_BAD);
    ASSERT(tileWidth > 0, GFMRV_ARGUMENTS_BAD);
    ASSERT(tileHeight > 0, GFMRV_ARGUMENTS_BAD);
    // Check that the dimensions are power of two
    ASSERT(gfmUtils_isPow2(tileWidth) == GFMRV_TRUE,
            GFMRV_SPRITESET_INVALID_WIDTH);
    ASSERT(gfmUtils_isPow2(tileHeight) == GFMRV_TRUE,
            GFMRV_SPRITESET_INVALID_HEIGHT);
    
    // Get the texture's dimensions
    rv = gfmTexture_getDimensions(&width, &height, pTex);
    ASSERT_NR(rv == GFMRV_OK);
    
    // Check that the tile dimensions is smaller than the texture
    ASSERT(tileWidth <= width, GFMRV_SPRITESET_INVALID_WIDTH);
    ASSERT(tileHeight <= height, GFMRV_SPRITESET_INVALID_HEIGHT);
    
    // Calculate the number of rows, columns and the total of tiles
    pCtx->columns = width / tileWidth;
    pCtx->rows = height / tileHeight;
    pCtx->tileCount = pCtx->columns * pCtx->rows;
    
    // Store info about the spriteset
    pCtx->texWidth = width;
    pCtx->texHeight = height;
    pCtx->tileWidth = tileWidth;
    pCtx->tileHeight = tileHeight;
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Initialize the spriteset from a internal texture
 * 
 * @param  pSset      The spriteset
 * @param  pCtx       The game's context
 * @param  index      The texture's index
 * @param  tileWidth  The width of each tile
 * @param  tileHeight The height of each tile
 * @return            GFMRV_OK, GFMRV_ARGUMENTS_BAD,
 *                    GFMRV_SPRITESET_INVALID_WIDTH,
 *                    GFMRV_SPRITESET_INVALID_HEIGHT,
 *                    GFMRV_TEXTURE_NOT_INITIALIZED
 */
gfmRV gfmSpriteset_initCached(gfmSpriteset *pSset, gfmCtx *pCtx, int index,
        int tileWidth, int tileHeight) {
    gfmRV rv;
    gfmTexture *pTex;
    
    // Sanitize arguments
    ASSERT(pSset, GFMRV_ARGUMENTS_BAD);
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    ASSERT(index >= 0, GFMRV_ARGUMENTS_BAD);
    ASSERT(tileWidth > 0, GFMRV_ARGUMENTS_BAD);
    ASSERT(tileHeight > 0, GFMRV_ARGUMENTS_BAD);
    
    // Try to get the texture
    rv = gfm_getTexture(&pTex, pCtx, index);
    ASSERT_NR(rv == GFMRV_OK);
    
    // Simply call the init texture
    rv = gfmSpriteset_init(pSset, pTex, tileWidth, tileHeight);
    ASSERT_NR(rv == GFMRV_OK);
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Get the tile's dimensions
 * 
 * @param  pWidth  The tile's width
 * @param  pHeight The tile's height
 * @param  pCtx    The spriteset
 * @return         GFMRV_OK, GFMRV_ARGUMENTS_BAD,
 *                 GFMRV_SPRITESET_NOT_INITIALIZED
 */
gfmRV gfmSpriteset_getDimension(int *pWidth, int *pHeight, gfmSpriteset *pCtx) {
    gfmRV rv;
    
    // Sanitize arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    ASSERT(pWidth, GFMRV_ARGUMENTS_BAD);
    ASSERT(pHeight, GFMRV_ARGUMENTS_BAD);
    // Check that the spriteset was already initialized
    ASSERT(pCtx->tileWidth > 0, GFMRV_SPRITESET_NOT_INITIALIZED);
    
    // Set the return
    *pWidth = pCtx->tileWidth;
    *pHeight = pCtx->tileHeight;
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Get a tile's position in the texture
 * 
 * @param  pX   The tile's horizontal position
 * @param  pY   The tile's vertical position
 * @param  pCtx The spriteset
 * @param  tile The desired tile
 * @return         GFMRV_OK, GFMRV_ARGUMENTS_BAD,
 *                 GFMRV_SPRITESET_NOT_INITIALIZED, GFMRV_INVALID_INDEX
 */
gfmRV gfmSpriteset_getPosition(int *pX, int *pY, gfmSpriteset *pCtx, int tile) {
    gfmRV rv;
    
    // Sanitize arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    ASSERT(pX, GFMRV_ARGUMENTS_BAD);
    ASSERT(pY, GFMRV_ARGUMENTS_BAD);
    // Check that the spriteset was already initialized
    ASSERT(pCtx->tileWidth > 0, GFMRV_SPRITESET_NOT_INITIALIZED);
    // Check that it's a valid tile
    ASSERT(tile < pCtx->tileCount, GFMRV_INVALID_INDEX);
    
    // Set the return
    *pX = (tile % pCtx->columns) * pCtx->tileWidth;
    *pY = (tile / pCtx->columns) * pCtx->tileHeight;
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

