/**
 * @file src/include/GFraMe_int/gfmTileType.h
 * 
 * Simple module to be used by the tilemap; It's separated to keep that file
 * cleaner
 */
#ifndef __GFMTILETYPE_STRUCT__
#define __GFMTILETYPE_STRUCT__

/** 'Export' the tile type */
typedef struct stGFMTileType gfmTileType;

#endif /* __GFMTILETYPE_STRUCT__ */

#ifndef __GFMTILETYPE_H__
#define __GFMTILETYPE_H__

#include <GFraMe/gfmError.h>

/** Simple structure to associate a tile with a type */
struct stGFMTileType {
    /** The tile */
    int tile;
    /** Type to be assigned on area generation */
    int type;
};

/**
 * Alloc a new gfmTileType
 * 
 * @param  **ppCtx The allocated context
 * @return         GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_ALLOC_FAILED
 */
gfmRV gfmTileType_getNew(gfmTileType **ppCtx);

/**
 * Free a gfmTileType
 * 
 * @param  **ppCtx The context
 * @return         GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmTileType_free(gfmTileType **ppCtx);

#endif /* __GFMTILETYPE_H__ */

