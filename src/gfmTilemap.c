/**
 * @file include/GFraMe/gfmTilemap.h
 * 
 * Tilemap module
 * Its most basic functionality is to render a tilemap into the screen. If
 * supported by the backend, it should be rendered in a single pass (e.g., using
 * OpenGL instantiation), though that can be modified (if more things should be 
 * drawn in a single draw call, for instance). This module takes into account
 * the screen space and the current camera (if any) to select which part should
 * be rendered.
 * Since this is expected to be used as a map, it can also keep track of
 * 'areas', which can be collideable tiles, hazards, events, etc. This can be
 * manually set (addings rectangles and its types) or automatically, adding tile
 * info to the map and requesting it to recalculate the 'areas'.
 * Another feature is to have animated tiles. This is created through the
 * association of a tile with its next frame and the delay before the transition
 */
#include <GFraMe/gfmAssert.h>
#include <GFraMe/gfmError.h>
#include <GFraMe/gfmGenericArray.h>
#include <GFraMe/gfmObject.h>
#include <GFraMe/gfmSpriteset.h>
#include <GFraMe/gfmTilemap.h>
#include <GFraMe_int/gfmTileAnimation.h>
#include <GFraMe_int/gfmTileType.h>

#include <stdlib.h>
#include <string.h>

/** Define an array for tile types */
gfmGenArr_define(gfmTileType);
/** Define an array for tile animations */
gfmGenArr_define(gfmTileAnimation);
/** Define an array for info about tile animation */
gfmGenArr_define(gfmTileAnimationInfo);
/** Define an array for gfmObjects */
gfmGenArr_define(gfmObject);

/** gfmTilemap structure */
struct stGFMTilemap {
    /** The tilemap's tileset */
    gfmSpriteset *pSset;
    /** Tiles info (yay, upgraded from unsigned char) */
    int *pData;
    /** How many tiles there are in pData (not how many are in use!) */
    int dataLen;
    /** How many tiles there are horizontally */
    int widthInTiles;
    /** How many tiles there are vertically */
    int heightInTiles;
    /** Whether the tilemap should automatically batch its tiles */
    int doBatched;
    /** Areas in the tilemap */
    gfmGenArr_var(gfmObject, pAreas);
    /** Tiles and theirs respective types */
    gfmGenArr_var(gfmTileType, pTTypes);
    /** Every animation on the current map */
    gfmGenArr_var(gfmTileAnimation, pTAnims);
    /** 'Description' for each possible animation */
    gfmGenArr_var(gfmTileAnimationInfo, pTAnimInfos);
};

/** 'Exportable' size of gfmTilemap */
const int sizeofGFMTilemap = (int)sizeof(gfmTilemap);

/**
 * Alloc a new tilemap
 * 
 * @param  ppCtx  The allocated tilemap
 * @return        GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_ALLOC_FAILED
 */
gfmRV gfmTilemap_getNew(gfmTilemap **ppCtx) {
    gfmRV rv;
    
    // Sanitize arguments
    ASSERT(ppCtx, GFMRV_ARGUMENTS_BAD);
    ASSERT(!(*ppCtx), GFMRV_ARGUMENTS_BAD);
    
    // Alloc the tilemap and clean it
    *ppCtx = (gfmTilemap*)malloc(sizeof(gfmTilemap));
    ASSERT(*ppCtx, GFMRV_ALLOC_FAILED);
    memset(*ppCtx, 0x0, sizeof(gfmTilemap));
    
    // Set anything that should be different from 0
    (*ppCtx)->doBatched = 1;
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Free and clean a previously allocated tilemap
 * 
 * @param  ppCtx  The tilemap
 * @return        GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmTilemap_free(gfmTilemap **ppCtx) {
    gfmRV rv;
    
    // Sanitize arguments
    ASSERT(ppCtx, GFMRV_ARGUMENTS_BAD);
    ASSERT(*ppCtx, GFMRV_ARGUMENTS_BAD);
    
    // Clean it up
    rv = gfmTilemap_clean(*ppCtx);
    ASSERT_NR(rv == GFMRV_OK);
    
    // Free it up
    free(*ppCtx);
    *ppCtx = 0;
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Pre-allocate and reset everything used by a tilemap
 * 
 * @param  pCtx          The tilemap
 * @param  pSset         The tilemap's spriteset (also used to get the texture)
 * @param  widthInTiles  Tilemap's width in tiles
 * @param  heightInTiles Tilemap's height in tiles
 * @param  defTile       Tile with which the tilemap will be filled
 * @return               GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_ALLOC_FAILED
 */
gfmRV gfmTilemap_init(gfmTilemap *pCtx, gfmSpriteset *pSset, int widthInTiles,
        int heightInTiles, int defTile) {
    gfmRV rv;
    int i;
    
    // Sanitize arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    ASSERT(pSset, GFMRV_ARGUMENTS_BAD);
    ASSERT(widthInTiles > 0, GFMRV_ARGUMENTS_BAD);
    ASSERT(heightInTiles > 0, GFMRV_ARGUMENTS_BAD);
    
    // Check if the tilemap should be expanded (and do so)
    if (widthInTiles * heightInTiles > pCtx->dataLen) {
        pCtx->pData = (int*)realloc(pCtx->pData,
                widthInTiles * heightInTiles * sizeof(int));
        ASSERT(pCtx->pData, GFMRV_ALLOC_FAILED);
        pCtx->dataLen = widthInTiles * heightInTiles;
    }
    pCtx->widthInTiles = widthInTiles;
    pCtx->heightInTiles = heightInTiles;
    
    // Set all tiles to a default value
    i = 0;
    while (i < pCtx->dataLen) {
        pCtx->pData[i] = defTile;
        i++;
    }
    
    // Set the spriteset
    pCtx->pSset = pSset;
    // Reset some buffers
    gfmGenArr_reset(pCtx->pAreas);
    gfmGenArr_reset(pCtx->pTAnims);
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Free everything allocated by init
 * 
 * @param  pCtx   The tilemap
 * @return        GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmTilemap_clean(gfmTilemap *pCtx) {
    gfmRV rv;
    
    // Sanitize arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    
    // Free the data buffer
    if (pCtx->pData) {
        free(pCtx->pData);
        pCtx->pData = 0;
        pCtx->dataLen = 0;
        pCtx->widthInTiles = 0;
        pCtx->heightInTiles = 0;
    }
    pCtx->pSset = 0;
    
    // Free the other buffers
    gfmGenArr_clean(pCtx->pAreas, gfmObject_free);
    gfmGenArr_clean(pCtx->pTTypes, gfmTileType_free);
    gfmGenArr_clean(pCtx->pTAnims, gfmTileAnimation_free);
    gfmGenArr_clean(pCtx->pTAnimInfos, gfmTileAnimationInfo_free);
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Loads a tilemap from a buffer;
 * Only the tile data is loaded! To also load areas and animations, store those
 * info in a file anduse gfmTilemap_loadf and a file, instead!
 * 
 * NOTE: The tilemap data will be copied into the context, so modifying the
 * original buffer does nothing! If it's dynamic, it can safely be deallocated
 * 
 * @param  pCtx      The tilemap
 * @param  pData     The tiles data
 * @param  dataLen   How many tiles there are in data
 * @param  mapWidth  Width of the map, in tiles
 * @param  mapHeight Height of the map, in tiles
 * @return           GFMRV_OK, GFMRV_ARGUMENTS_BAD,
 *                   GFMRV_TILEMAP_NOT_INITIALIZED, GFMRV_ALLOC_FAILED
 */
gfmRV gfmTilemap_load(gfmTilemap *pCtx, int *pData, int dataLen, int mapWidth,
        int mapHeight) {
    gfmRV rv;
    
    // Sanitize arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    ASSERT(pData, GFMRV_ARGUMENTS_BAD);
    ASSERT(dataLen > 0, GFMRV_ARGUMENTS_BAD);
    ASSERT(mapWidth > 0, GFMRV_ARGUMENTS_BAD);
    ASSERT(mapHeight > 0, GFMRV_ARGUMENTS_BAD);
    ASSERT(dataLen == mapWidth * mapHeight, GFMRV_ARGUMENTS_BAD);
    // Check that it was initialized
    ASSERT(pCtx->pSset, GFMRV_TILEMAP_NOT_INITIALIZED);
    
    // Reinitialize the map, to be sure it can fit this new tilemap
    rv = gfmTilemap_init(pCtx, pCtx->pSset, mapWidth, mapHeight, 0/*defTile*/);
    ASSERT_NR(rv == GFMRV_OK);
    
    // Copy the new map into the tilemap
    // Can't think of a good reason not to use memcpy, so...
    memcpy(pCtx->pData, pData, dataLen*sizeof(int));
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

gfmRV gfmTilemap_addArea(gfmTilemap *pCtx, int x, int y, int width,
        int height, int type) {
    return GFMRV_FUNCTION_NOT_IMPLEMENTED;
}

/**
 * Add an animation from one tile to another
 * 
 * @param  pCtx     The tilemap
 * @param  tile     The tile
 * @param  delay    How long (in milliseconds) until the next tile
 * @param  nextTile Tile to which it will change
 * @return          GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_TILEANIM_EXTANT
 */
gfmRV gfmTilemap_addTileAnimation(gfmTilemap *pCtx, int tile, int delay, int nextTile) {
    gfmRV rv;
    gfmTileAnimationInfo *pTAInfo;
    
    // Sanitize arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    ASSERT(tile >= 0, GFMRV_ARGUMENTS_BAD);
    ASSERT(delay > 0, GFMRV_ARGUMENTS_BAD);
    ASSERT(nextTile >= 0, GFMRV_ARGUMENTS_BAD);
    ASSERT(nextTile != tile, GFMRV_ARGUMENTS_BAD);
    
    // TODO Check that the tile still doesn't have an animation
    
    // Get a new tile animation
    gfmGenArr_getNextRef(gfmTileAnimationInfo, pCtx->pTAnimInfos, 1/*INC*/,
            pTAInfo, gfmTileAnimationInfo_getNew);
    gfmGenArr_push(pCtx->pTAnimInfos);
    
    // Set the animation info
    pTAInfo->tile = tile;
    pTAInfo->delay = delay;
    pTAInfo->nextTile = nextTile;
    // nextTileIndex is only set during recache of the animations
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Add an animation with various (or only two) frames
 * 
 * @param  pCtx      The tilemap
 * @param  pData     Array with the sequence of frames/tiles
 * @param  numFrames How many frames there are in pData
 * @param  fps       How fast (in frames per second) should the animation be
 * @param  doLoop    Whether the animation should loop (1) or not (0)
 * @return           GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_TILEANIM_EXTANT,
 *                   GFMRV_ALLOC_FAILED
 */
gfmRV gfmTilemap_addAnimation(gfmTilemap *pCtx, int *pData, int numFrames, int fps, int doLoop) {
    gfmRV rv;
    int delay, i;
    
    // Sanitize arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    ASSERT(pData, GFMRV_ARGUMENTS_BAD);
    ASSERT(numFrames >= 2, GFMRV_ARGUMENTS_BAD);
    ASSERT(fps > 0, GFMRV_ARGUMENTS_BAD);
    
    // Calculate the delay between frames
    delay = 1000 / numFrames;
    
    // Add every frame
    i = 0;
    while (i < numFrames - 1) {
        // Add an animation from the current index to the next
        rv = gfmTilemap_addTileAnimation(pCtx, pData[i], delay, pData[i + 1]);
        ASSERT_NR(rv == GFMRV_OK);
        
        i++;
    }
    // If it's looped, add an animation from the last index back to the first
    if (doLoop) {
        rv = gfmTilemap_addTileAnimation(pCtx, pData[numFrames - 1], delay,
                pData[0]);
        ASSERT_NR(rv == GFMRV_OK);
    }
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Add a batch of animations to the tilemap;
 * The array must be organized in the following format:
 * 
 * pData[0] = anim_0.numFrames
 * pData[1] = anim_0.fps
 * pData[2] = anim_0.doLoop
 * pData[3] = anim_0.frame_0
 * ...
 * pData[3 + anim_0.numFrames - 1] = anim_0.lastFrame
 * pData[3 + anim_0.numFrames] = anim_1.numFrames
 * ...
 * 
 * And so on.
 * 
 * @param  pCtx    The  tilemap
 * @param  pData   The batch of animations
 * @param  dataLen How many integers there are in pData
 * @return           GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_TILEANIM_EXTANT,
 *                   GFMRV_ALLOC_FAILED
 */
gfmRV gfmTilemap_addAnimations(gfmTilemap *pCtx, int *pData, int dataLen) {
    gfmRV rv;
    int *pAnimData, doLoop, fps, i, numFrames;
    
    // Sanitize arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    ASSERT(pData, GFMRV_ARGUMENTS_BAD);
    // A Batch with a single two-frames animation would have 5 ints
    ASSERT(dataLen >= 5, GFMRV_ARGUMENTS_BAD);
    
    // Add every animation
    i = 0;
    while (i < dataLen) {
        // Get the animation parameters
        numFrames = pData[i + 0];
        fps       = pData[i + 1];
        doLoop    = pData[i + 2];
        pAnimData = pData + i + 3;
        // Add the animation
        rv = gfmTilemap_addAnimation(pCtx, pAnimData, numFrames, fps, doLoop);
        ASSERT_NR(rv == GFMRV_OK);
        // Go to the next animation
        i += numFrames + 3;
    }
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Go through the map and cache every tile with an animation; It also calculate
 * the nextAnimIndex for every animation info
 * 
 * This function is really slow!
 * 
 * @param  pCtx  The tilemap
 * @return       GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_TILEMAP_NOT_INITIALIZED,
 *               GFMRV_TILEMAP_NO_TILEANIM
 */
gfmRV gfmTilemap_recacheAnimations(gfmTilemap *pCtx) {
    gfmRV rv;
    gfmTileAnimation *pTAnim;
    gfmTileAnimationInfo *pTAInfo;
    int i, j, tile;
    
    // Sanitize arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    // Check that it was initialized
    ASSERT(pCtx->pSset, GFMRV_TILEMAP_NOT_INITIALIZED);
    // Check that there's at least one animation to be parsed
    ASSERT(gfmGenArr_getUsed(pCtx->pTAnimInfos) > 0, GFMRV_TILEMAP_NO_TILEANIM);
    
    // Reset the previous animations
    gfmGenArr_reset(pCtx->pTAnims);
    
    // Go through every anim info and search for its "nextAnimTile" in O(n^2)
    i = 0;
    while (i < gfmGenArr_getUsed(pCtx->pTAnimInfos)) {
        // Get the next anim info
        pTAInfo = gfmGenArr_getObject(pCtx->pTAnimInfos, i);
        
        // Search for the tile that's it's next
        j = 0;
        while (j < gfmGenArr_getUsed(pCtx->pTAnimInfos)) {
            gfmTileAnimationInfo *pTmpTAInfo;
            
            // Get the next anim info
            pTmpTAInfo = gfmGenArr_getObject(pCtx->pTAnimInfos, j);
            
            // If the index for the next tile was found, stop
            if (pTmpTAInfo->tile == pTAInfo->nextTile) {
                break;
            }
            
            j++;
        }
        // Check that it was found and store its value
        if (j < gfmGenArr_getUsed(pCtx->pTAnimInfos)) {
            pTAInfo->nextTileIndex = j;
        }
        else {
            // -1 is used if the next tile isn't animated
            pTAInfo->nextTileIndex = -1;
        }
        
        i++;
    }
    
    // Go through every tile in the map and cache its animation in O(n^2)
    i = 0;
    while (i < pCtx->widthInTiles * pCtx->heightInTiles) {
        // Get the current tile
        tile = pCtx->pData[i];
        
        // Check if it's animated
        j = 0;
        while (j < gfmGenArr_getUsed(pCtx->pTAnimInfos)) {
            // Get the next anim info
            pTAInfo = gfmGenArr_getObject(pCtx->pTAnimInfos, j);
            
            // If it's a match, stop
            if (pTAInfo->tile == tile) {
                break;
            }
            
            j++;
        }
        
        // Check that it was found and add a new animation
        if (j < gfmGenArr_getUsed(pCtx->pTAnimInfos)) {
            // Get a new tile animation
            gfmGenArr_getNextRef(gfmTileAnimation, pCtx->pTAnims, 1/*INC*/,
                    pTAnim, gfmTileAnimation_getNew);
            gfmGenArr_push(pCtx->pTAnims);
            
            // Set its values
            pTAnim->index = i;
            pTAnim->delay = pTAInfo->delay;
            pTAnim->infoIndex = j;
        }
        
        i++;
    }
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Add a tile type, used when automatically generating areas
 * 
 * @param  pCtx The tilemap
 * @param  tile The tile
 * @param  type The tile's type
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_TILEMAP_NOT_INITIALIZED,
 *              GFMRV_TILETYPE_EXTANT
 */
gfmRV gfmTilemap_addTileType(gfmTilemap *pCtx, int tile, int type) {
    gfmRV rv;
    gfmTileType *pTType;
    
    // Sanitize arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    ASSERT(tile >= 0, GFMRV_ARGUMENTS_BAD);
    
    // TODO Check if the tile is already in the array
    
    // Get a new tile type
    gfmGenArr_getNextRef(gfmTileType, pCtx->pTTypes, 1/*INC*/, pTType, gfmTileType_getNew);
    gfmGenArr_push(pCtx->pTTypes);
    
    // Set the tile's type
    pTType->tile = tile;
    pTType->type = type;
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Add various tile types at once;
 * The data array should have dataLen/2 'entries' with two values: the tile and
 * its type;
 * 
 * Therefore if collideable is represented by 0 and spike by 1; and tiles 0, 1,
 * 2, are collideables, tile 3 is spike and tiles 4 and 5 aren't part of areas,
 * the array (and the function call) would look as follows:
 * 
 * <SNIPPET>
 * 
 * //           |   collideable   |spike|
 * int data[] = { 0,0 , 1,0 , 2,0 , 3,1 };
 * int dataLen = sizeof(dataLen) / sizeof(int); // calculates entries, not bytes
 * gfmRV rv;
 * gfmTilemap *pCtx = NULL;
 * 
 * ...
 * 
 * rv = gfmTilemap_addTileTypes(pCtx, data, dataLen);
 * ASSERT_NR(rv == GFMRV_OK);
 * 
 * ...
 * 
 * rv = GFMRV_OK;
 * __ret:
 * return rv;
 * 
 * </SNIPPET>
 * 
 * @param  pCtx    The tilemap
 * @param  pData   The tile types data
 * @param  dataLen How many integers there are in pData
 * @return         GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_TILEMAP_NOT_INITIALIZED,
 *                 GFMRV_TILETYPE_EXTANT
 */
gfmRV gfmTilemap_addTileTypes(gfmTilemap *pCtx, int *pData, int dataLen) {
    gfmRV rv;
    int i;
    
    // Sanitize arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    ASSERT(pData, GFMRV_ARGUMENTS_BAD);
    ASSERT(dataLen > 0, GFMRV_ARGUMENTS_BAD);
    ASSERT(dataLen % 2 == 0, GFMRV_ARGUMENTS_BAD);
    
    // Get how many entries there actually are
    dataLen /= 2;
    
    // Add every tile
    i = 0;
    while (i < dataLen) {
        // Tile is on index 0 and type on index 1
        rv = gfmTilemap_addTileType(pCtx, pData[i*2 + 0], pData[i*2 + 1]);
        ASSERT_NR(rv == GFMRV_OK);
        i++;
    }
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Get the tile's type; It's not optimized if there are many tiles, but it
 * should work just fine if there are only a few one; Search is done linearly,
 * for your interest;
 * If efficiency is ever a concern, maybe modifying the tile types array to a
 * binary tree would help, already
 * 
 * @param  pType The tile's type
 * @param  pCtx  The tilemap
 * @param  tile  The tile (NOT the index of the tile in the tilemap)
 * @return       GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_TILEMAP_NO_TILETYPE
 */
gfmRV gfmTilemap_getTileType(int *pType, gfmTilemap *pCtx, int tile) {
    gfmRV rv;
    int i;
    
    // Sanitize arguments
    ASSERT(pType, GFMRV_ARGUMENTS_BAD);
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    
    // Iterate the array looking for the tile
    i = 0;
    while (i < gfmGenArr_getUsed(pCtx->pTTypes)) {
        gfmTileType *pTType;
        
        // Get the next tile type
        pTType = gfmGenArr_getObject(pCtx->pTTypes, i);
        
        // Check if it's a match
        if (pTType->tile == tile) {
            // And store the type
            *pType = pTType->type;
            break;
        }
        
        i++;
    }
    // Check that the type was found
    ASSERT(i < gfmGenArr_getUsed(pCtx->pTTypes), GFMRV_TILEMAP_NO_TILETYPE);
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Check if the indexed tile is already inside any of the areas
 * 
 * @param  pCtx      The tilemap
 * @param  tileIndex The index of the tile
 * @return           GFMRV_TRUE, GFMRV_FALSE, GFMRV_ARGUMENTS_BAD,
 *                   GFMRV_TILEMAP_NOT_INITIALIZED
 */
gfmRV gfmTilemap_isTileInAnyArea(gfmTilemap *pCtx, int tileIndex) {
    gfmRV rv;
    int height, i, width, x, y;
    
    // Sanitize arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    // Check that it was initialized
    ASSERT(pCtx->pSset, GFMRV_TILEMAP_NOT_INITIALIZED);
    
    // Get the tile's dimensions and position
    x = tileIndex % pCtx->widthInTiles;
    y = tileIndex / pCtx->widthInTiles;
    // Get the tile's dimension
    rv = gfmSpriteset_getDimension(&width, &height, pCtx->pSset);
    ASSERT_NR(rv == GFMRV_OK);
    
    // Iterate the array, checking every tile
    i = 0;
    while (i < gfmGenArr_getUsed(pCtx->pAreas)) {
        gfmObject *pObj;
        
        // Get the next tile type
        pObj = gfmGenArr_getObject(pCtx->pAreas, i);
        
        // Check if the tiles is inside this object
        rv = gfmObject_isPointInside(pObj, x + width / 2, y + height / 2);
        ASSERT_NR(rv == GFMRV_FALSE);
        
        i++;
    }
    
    rv = GFMRV_FALSE;
__ret:
    return rv;
}

/**
 * Traverse the map, from a given tile, getting the biggest rectangle that
 * contains all neighboring tiles of the same type; Since the traversal is first
 * done in the horizontal and then in the vertical, area's are wider than they
 * are tall
 * 
 * @param  pX        The area's horizontal position
 * @param  pY        The area's vertical position
 * @param  pWidth    The area's height
 * @param  pHeight   The area's width
 * @param  pCtx      The tilemap
 * @param  tileIndex The area's first tile
 * @return           GFMRV_OK, GFMRV_ARGUMENTS_BAD,
 *                   GFMRV_TILEMAP_NOT_INITIALIZED
 */
gfmRV gfmTilemap_getAreaBounds(int *pX, int *pY, int *pWidth, int *pHeight,
        gfmTilemap *pCtx, int tileIndex) {
    gfmRV rv;
    int height, i, j, tileWidth, tileHeight, type, nextType, width, x, y;
    
    // Sanitize arguments
    ASSERT(pX, GFMRV_ARGUMENTS_BAD);
    ASSERT(pY, GFMRV_ARGUMENTS_BAD);
    ASSERT(pWidth, GFMRV_ARGUMENTS_BAD);
    ASSERT(pHeight, GFMRV_ARGUMENTS_BAD);
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    ASSERT(tileIndex >= 0, GFMRV_ARGUMENTS_BAD);
    // Check that it was initialized
    ASSERT(pCtx->pSset, GFMRV_TILEMAP_NOT_INITIALIZED);
    // Had to check if it was initialized, before this assert
    ASSERT(tileIndex <= pCtx->widthInTiles * pCtx->heightInTiles,
            GFMRV_ARGUMENTS_BAD);
    
    // Get the tile's position (in tiles)
    x = tileIndex % pCtx->widthInTiles;
    y = tileIndex / pCtx->widthInTiles;
    
    // Get the tile's type
    rv = gfmTilemap_getTileType(&type, pCtx, pCtx->pData[tileIndex]);
    ASSERT_NR(rv == GFMRV_OK);
    
    // Get the first tile, on the horizontal
    i = 0;
    while (x + i < pCtx->widthInTiles) {
        // Get the next tile's type
        rv = gfmTilemap_getTileType(&nextType, pCtx,
                pCtx->pData[tileIndex + i]);
        ASSERT_NR(rv == GFMRV_OK);
        // Stop if it's a different type of tile
        if (nextType != type)
            break;
        
        i++;
    }
    // Store the area's width
    width = i;
    
    // Get the area's height (the tallest possible)
    i = 0;
    // Height must be initialize with a really big number... so, yeah...
    height = 0x7fffffff;
    while (i < width) {
        // Check the height of this 'column'
        j = 0;
        while (y + j < pCtx->heightInTiles) {
            // Get the next tile's type
            rv = gfmTilemap_getTileType(&nextType, pCtx,
                    pCtx->pData[tileIndex + i + j * pCtx->widthInTiles]);
            ASSERT_NR(rv == GFMRV_OK);
            // Stop if it's a different type of tile
            if (nextType != type)
                break;
            j++;
        }
        // If this 'column' is shorter than the previous, update the height
        if (j < height)
            height = j;
        
        i++;
    }
    
    // Get the tile's dimension
    rv = gfmSpriteset_getDimension(&tileWidth, &tileHeight, pCtx->pSset);
    ASSERT_NR(rv == GFMRV_OK);
    // Return the gotten area
    *pX = x * tileWidth;
    *pY = y * tileHeight;
    *pWidth = width * tileWidth;
    *pHeight = height * tileHeight;
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Automatically generates all areas in the tilemap
 * 
 * @param  pCtx The tilemap
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_TILEMAP_NOT_INITIALIZED,
 *              GFMRV_TILEMAP_NO_TILETYPE
 */
gfmRV gfmTilemap_recalculateAreas(gfmTilemap *pCtx) {
    gfmRV rv;
    int i;
    
    // Sanitize arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    // Check that it was initialized
    ASSERT(pCtx->pSset, GFMRV_TILEMAP_NOT_INITIALIZED);
    // Check that there is at least one tile type
    ASSERT(gfmGenArr_getUsed(pCtx->pTTypes), GFMRV_TILEMAP_NO_TILETYPE);
    
    // Reset the previous areas
    gfmGenArr_reset(pCtx->pAreas);
    
    // Traverse every tile
    i = -1;
    while (++i < pCtx->widthInTiles * pCtx->heightInTiles) {
        int height, tile, type, width, x, y;
        
        // Get the current tile
        tile = pCtx->pData[i];
        
        // Check if the tile is a valid area
        rv = gfmTilemap_getTileType(&type, pCtx, tile);
        if (rv == GFMRV_TILEMAP_NO_TILETYPE)
            continue;
        // Check if the tile is already inside an area
        rv = gfmTilemap_isTileInAnyArea(pCtx, tile);
        if (rv == GFMRV_FALSE)
            continue;
        
        // Then, get this area's bounds
        rv = gfmTilemap_getAreaBounds(&x, &y, &width, &height, pCtx, i);
        ASSERT_NR(rv == GFMRV_OK);
        
        // And add it to the tilemap
        rv = gfmTilemap_addArea(pCtx, x, y, width, height, type);
        ASSERT_NR(rv == GFMRV_OK);
    }
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Get the map's dimensions in pixels
 * 
 * @param  pWidth  The tilemap's width
 * @param  pHeight The tilemap's height
 * @param  pCtx    The tilemap
 * @return         GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_TILEMAP_NOT_INITIALIZED
 */
gfmRV gfmTilemap_getDimension(int *pWidth, int *pHeight, gfmTilemap *pCtx) {
    gfmRV rv;
    int tileWidth, tileHeight;
    
    // Sanitize arguments
    ASSERT(pWidth, GFMRV_ARGUMENTS_BAD);
    ASSERT(pHeight, GFMRV_ARGUMENTS_BAD);
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    // Check that it was initialized
    ASSERT(pCtx->pSset, GFMRV_TILEMAP_NOT_INITIALIZED);
    ASSERT(pCtx->widthInTiles > 0, GFMRV_TILEMAP_NOT_INITIALIZED);
    ASSERT(pCtx->heightInTiles > 0, GFMRV_TILEMAP_NOT_INITIALIZED);
    
    // Get the tile's dimension
    rv = gfmSpriteset_getDimension(&tileWidth, &tileHeight, pCtx->pSset);
    ASSERT_NR(rv == GFMRV_OK);
    // Return the map dimension
    *pWidth = pCtx->widthInTiles * tileWidth;
    *pHeight = pCtx->heightInTiles * tileHeight;
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Disable batched draw; It should be used when it's desired to batch more tiles
 * at once
 * 
 * @param  pCtx The tilemap
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmTilemap_disableBatchedDraw(gfmTilemap *pCtx) {
    gfmRV rv;
    
    // Sanitize arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    
    pCtx->doBatched = 0;
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Enable batched draw; It should be used when it's desired to batch more tiles
 * at once
 * 
 * @param  pCtx The tilemap
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmTilemap_enableBatchedDraw(gfmTilemap *pCtx) {
    gfmRV rv;
    
    // Sanitize arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    
    pCtx->doBatched = 1;
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Run through every animated tile and update its time and the tile itself
 * 
 * @param  pCtx The tilemap
 * @param  ms   Time, in milliseconds, elapsed from the previous frame
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_TILEMAP_NOT_INITIALIZED
 */
gfmRV gfmTilemap_update(gfmTilemap *pCtx, int ms) {
    gfmRV rv;
    gfmTileAnimation *pTAnim;
    gfmTileAnimationInfo *pTAInfo;
    int i;
    
    // Sanitize arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    ASSERT(ms > 0, GFMRV_ARGUMENTS_BAD);
    // Check that the tilemap was initialzied
    ASSERT(pCtx->pSset, GFMRV_TILEMAP_NOT_INITIALIZED);
    // If there no animations, do nothing
    ASSERT(gfmGenArr_getUsed(pCtx->pTAnims) > 0, GFMRV_OK);
    
    // Loop through every animation and updates it
    i = 0;
    while (i < gfmGenArr_getUsed(pCtx->pTAnims)) {
        // Get the next animation
        pTAnim = gfmGenArr_getObject(pCtx->pTAnims, i);
        
        // Check if it's a valid animation
        CASSERT_NR(pTAnim->index >= 0, __nextAnim);
        // Updates its delay
        pTAnim->delay -= ms;
        
        // If a new frame was issued, update the animation and the tile
        while (pTAnim->delay <= 0) {
            // Get the current animation's info
            pTAInfo = gfmGenArr_getObject(pCtx->pTAnimInfos, pTAnim->infoIndex);
            
            // Update the tilemap
            pCtx->pData[pTAnim->index] = pTAInfo->nextTile;
            
            // Update the animation
            if (pTAInfo->nextTileIndex >= 0) {
                pTAnim->infoIndex = pTAInfo->nextTileIndex;
                // Get the next animation's info
                pTAInfo = gfmGenArr_getObject(pCtx->pTAnimInfos,
                        pTAnim->infoIndex);
                // Update the animation's delay (accumulate over the previous)
                pTAnim->delay += pTAInfo->delay;
            }
            else {
                // "Remove" the animation from the list
                pTAnim->index = -1;
            }
        }
__nextAnim:
        i++;
    }
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Draw every tile that's inside tha world's camera
 * 
 * @param  pTMap The tilemap
 * @param  pCtx  The game's context
 * @return       GFMRV_OK, GFMRV_ARGUMENTS_BAD, ...
 */
gfmRV gfmTilemap_draw(gfmTilemap *pTMap, gfmCtx *pCtx) {
    gfmRV rv;
    int camX, camY, camWidth, camHeight, dX, firstTile, i, iniX, offX,
            tile, tileWidth, tileHeight, x, y;
    
    // Sanitize arguments
    ASSERT(pTMap, GFMRV_ARGUMENTS_BAD);
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    // Check if initialized
    ASSERT(pTMap->pSset, GFMRV_TILEMAP_NOT_INITIALIZED);
    ASSERT(pTMap->widthInTiles > 0, GFMRV_TILEMAP_NOT_INITIALIZED);
    ASSERT(pTMap->heightInTiles > 0, GFMRV_TILEMAP_NOT_INITIALIZED);
    
    // TODO implement for tilemaps smaller than the screen
    
    // Get the tile's dimension
    rv = gfmSpriteset_getDimension(&tileWidth, &tileHeight, pTMap->pSset);
    ASSERT_NR(rv == GFMRV_OK);
    // Get camera's dimension
    rv = gfm_getCameraPosition(&camX, &camY, pCtx);
    rv = gfm_getCameraDimensions(&camWidth, &camHeight, pCtx);
    
    // Get the first tile's position on the screen
    iniX = -(camX % tileWidth);
    y = -(camY % tileHeight);
    // Get the first tile's index
    firstTile = camX / tileWidth + camY / tileHeight * pTMap->widthInTiles;
    // Get how many tiles are skipped until the next row's first visible tile
    dX = pTMap->widthInTiles - camWidth / tileWidth;
    // If the camera's position doesn't match a tile, it will render 1 extra one
    if (iniX != 0)
        dX--;
    
    // Check if should batch
    if (pTMap->doBatched)
        gfm_batchBegin(pCtx);
    
    // Render every tile
    i = 0;
    x = iniX;
    offX = 0;
    while (firstTile + offX + i < pTMap->widthInTiles * pTMap->heightInTiles
            && y < camHeight) {
        // Get the tile
        tile = pTMap->pData[firstTile + offX + i];
        
        // Render the tile to the screen
        rv = gfm_drawTile(pCtx, pTMap->pSset, x, y, tile);
        // TODO Add a debug ASSERT (?)
        
        // Update the tile position
        i++;
        x += tileWidth;
        if (x >= camWidth) {
            x = iniX;
            y += tileHeight;
            offX += dX;
        }
    }
    
    // Check if should end batch
    if (pTMap->doBatched)
        gfm_batchEnd(pCtx);
    
    rv = GFMRV_OK;
__ret:
    return rv;
}


/*
{
    gfmRV rv;
    
    rv = GFMRV_OK;
__ret:
    return rv;
}
*/
