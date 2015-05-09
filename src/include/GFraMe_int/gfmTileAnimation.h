/**
 * @file src/include/GFraMe/gfmTileAnimation.h
 * 
 * Wrapper for gfmTileAnimation and gfmTileAnimationInfo
 */
#ifndef __GFMTILEANIMATION_STRUCT__
#define __GFMTILEANIMATION_STRUCT__

/** 'Exports' gfmTileAnimation */
typedef struct stGFMTileAnimation gfmTileAnimation;
/** 'Exports' gfmTileAnimationInfo */
typedef struct stGFMTileAnimationInfo gfmTileAnimationInfo;

#endif /* __GFMTILEANIMATION_STRUCT__ */

#ifndef __GFMTILEANIMATION_H__
#define __GFMTILEANIMATION_H__

#include <GFraMe/gfmError.h>

/** Struct to keep track of all animations on the current tilemap */
struct stGFMTileAnimation {
    /** Tile's index on the tilemap data */
    int index;
    /** Time until the tile changes */
    int delay;
    /** Index on the tileAnimationInfo array with this tile's info */
    int typeIndex;
};

/** Struct to keep track of all possible animations */
struct stGFMTileAnimationInfo {
    /** The tile */
    int tile;
    /** Time until the tile changes */
    int delay;
    /** Tile to which it changes */
    int nextTile;
    /** Index on the tileAnimationInfo array with the next tile's info (or -1)*/
    int nextTileIndex;
};

/**
 * Alloc a new gfmTileAnimation
 * 
 * @param  **ppCtx The allocated context
 * @return         GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_ALLOC_FAILED
 */
gfmRV gfmTileAnimation_getNew(gfmTileAnimation **ppCtx);

/**
 * Free a gfmTileAnimation
 * 
 * @param  **ppCtx The context
 * @return         GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmTileAnimation_free(gfmTileAnimation **ppCtx);

/**
 * Alloc a new gfmTileAnimationInfo
 * 
 * @param  **ppCtx The allocated context
 * @return         GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_ALLOC_FAILED
 */
gfmRV gfmTileAnimationInfo_getNew(gfmTileAnimationInfo **ppCtx);

/**
 * Free a gfmTileAnimationInfo
 * 
 * @param  **ppCtx The context
 * @return         GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmTileAnimationInfo_free(gfmTileAnimationInfo **ppCtx);

#endif /* __GFMTILEANIMATION_H__ */

