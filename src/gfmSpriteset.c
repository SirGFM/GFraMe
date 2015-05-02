/**
 * @file include/GFraMe/gfmSpriteset.h
 * 
 * Module to break up a texture into tiles
 */
#include <GFraMe/gfmAssert.h>
#include <GFraMe/gfmError.h>
#include <GFraMe/gfmSpriteset.h>
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

gfmRV gfmSpriteset_getNew(gfmSpriteset **ppCtx);
gfmRV gfmSpriteset_getFree(gfmSpriteset **ppCtx);
gfmRV gfmSpriteset_init(gfmSpriteset *pCtx, gfmTexture *pTex);
gfmRV gfmSpriteset_initCached(gfmSpriteset *pCtx, int index);
gfmRV gfmSpriteset_getDimension(int *pWidth, int *pHeight, gfmSpriteset *pCtx);
gfmRV gfmSpriteset_getPosition(int *pX, int *pY, gfmSpriteset *pCtx, int tile);

