/**
 * @file include/GFraMe/gfmSpriteset.h
 * 
 * Module to break up a texture into tiles
 */
#ifndef __GFMSPRITESET_STRUCT__
#define __GFMSPRITESET_STRUCT__

/** 'Exports' the gfmSpriteset structure */
typedef struct stGFMSpriteset gfmSpriteset;

#endif /* __GFMSPRITESET_STRUCT__ */

#ifndef __GFMSPRITESET_H__
#define __GFMSPRITESET_H__

#include <GFraMe/gfmError.h>
#include <GFraMe/gframe.h>
#include <GFraMe/core/gfmTexture_bkend.h>

/** 'Exportable' size of gfmSpritese */
extern const int sizeofGFMSpriteset;

/**
 * Alloc a new spriteset
 * 
 * @param  ppCtx The allocated object
 * @return       GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_ALLOC_FAILED
 */
gfmRV gfmSpriteset_getNew(gfmSpriteset **ppCtx);

/**
 * Free and clean a spriteset
 * 
 * @param  ppCtx The object to be freed
 * @return       GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmSpriteset_free(gfmSpriteset **ppCtx);

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
        int tileWidth, int tileHeight);

/**
 * Get the tile's dimensions
 * 
 * @param  pWidth  The tile's width
 * @param  pHeight The tile's height
 * @param  pCtx    The spriteset
 * @return         GFMRV_OK, GFMRV_ARGUMENTS_BAD,
 *                 GFMRV_SPRITESET_NOT_INITIALIZED
 */
gfmRV gfmSpriteset_getDimension(int *pWidth, int *pHeight, gfmSpriteset *pCtx);

/**
 * Get a tile's position in the texture
 * 
 * @param  pX   The tile's horizontal position
 * @param  pY   The tile's vertical position
 * @param  pCtx The spriteset
 * @param  tile The desired tile
 * @return         GFMRV_OK, GFMRV_ARGUMENTS_BAD,
 *                 GFMRV_SPRITESET_NOT_INITIALIZED
 */
gfmRV gfmSpriteset_getPosition(int *pX, int *pY, gfmSpriteset *pCtx, int tile);

/**
 * Retrieve a tile's texture
 * 
 * @param  ppTex The retrieved texture
 * @param  pCtx  The spriteset
 * @return       GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_SPRITESET_NOT_INITIALIZED
 */
gfmRV gfmSpriteset_getTexture(gfmTexture **ppTex, gfmSpriteset *pCtx);

#endif /* __GFMSPRITESET_H__ */

