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
#include <GFraMe/core/gfmTexture_bkend.h>

/** 'Exportable' size of gfmSpritese */
extern const int sizeofGFMSpriteset;

gfmRV gfmSpriteset_getNew(gfmSpriteset **ppCtx);
gfmRV gfmSpriteset_getFree(gfmSpriteset **ppCtx);
gfmRV gfmSpriteset_init(gfmSpriteset *pCtx, gfmTexture *pTex);
gfmRV gfmSpriteset_initCached(gfmSpriteset *pCtx, int index);
gfmRV gfmSpriteset_getDimension(int *pWidth, int *pHeight, gfmSpriteset *pCtx);
gfmRV gfmSpriteset_getPosition(int *pX, int *pY, gfmSpriteset *pCtx, int tile);

#endif /* __GFMSPRITESET_H__ */

