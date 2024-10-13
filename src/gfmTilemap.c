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
#include <GFraMe/gframe.h>
#include <GFraMe/gfmAssert.h>
#include <GFraMe/gfmError.h>
#include <GFraMe/gfmGenericArray.h>
#include <GFraMe/gfmHitbox.h>
#include <GFraMe/gfmLog.h>
#include <GFraMe/gfmSpriteset.h>
#include <GFraMe/gfmTilemap.h>
#include <GFraMe/gfmTypes.h>
#include <GFraMe/core/gfmFile_bkend.h>
#include <GFraMe_int/gfmHitbox.h>
#include <GFraMe_int/gfmTileAnimation.h>
#include <GFraMe_int/gfmTileType.h>
#include <GFraMe_int/gfmParserCommon.h>

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/** Define an array for tile types */
gfmGenArr_define(gfmTileType);
/** Define an array for tile animations */
gfmGenArr_define(gfmTileAnimation);
/** Define an array for info about tile animation */
gfmGenArr_define(gfmTileAnimationInfo);

/** Used the check neighbouring tiles */
enum enNeighbour {
    neighbour_none  = 0x00,
    neighbour_left  = 0x01,
    neighbour_right = 0x02,
    neighbour_down  = 0x04,
    neighbour_up    = 0x08,
    /* The following flag is a short-cut used to check whether a tile belongs to
     * an inner (or outer) corner. */
    neighbour_checkCorner = neighbour_left | neighbour_up,
};
typedef enum enNeighbour neighbour;

/** gfmTilemap structure */
struct stGFMTilemap {
    gfmHitbox *pAreas;
    /** The tilemap's tileset */
    gfmSpriteset *pSset;
    /** Horizontal position of the tilemap */
    int x;
    /** Vertical position of the tilemap */
    int y;
    /** Tiles info (yay, upgraded from unsigned char) */
    int *pData;
    /** How many tiles there are in pData (not how many are in use!) */
    int dataLen;
    /** How many tiles there are horizontally */
    int widthInTiles;
    /** How many tiles there are vertically */
    int heightInTiles;
    /** How many areas have been used */
    uint16_t numAreas;
    /** How many areas were alloc'ed */
    uint16_t areaCount;
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
 * Set the tilemap's dimensions in tiles
 * 
 * @param  pCtx          The tilemap
 * @param  widthInTiles  Tilemap's width in tiles
 * @param  heightInTiles Tilemap's height in tiles
 * @return               GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_ALLOC_FAILED
 */
static gfmRV gfmTilemap_setDimensions(gfmTilemap *pCtx, int widthInTiles,
        int heightInTiles) {
    gfmRV rv;
    
    // Sanitize arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    ASSERT(widthInTiles > 0, GFMRV_ARGUMENTS_BAD);
    ASSERT(heightInTiles > 0, GFMRV_ARGUMENTS_BAD);
    
    // Check if the tilemap should be expanded (and do so)
    if (widthInTiles * heightInTiles > pCtx->dataLen) {
        pCtx->pData = (int*)realloc(pCtx->pData,
                widthInTiles * heightInTiles * sizeof(int));
        ASSERT(pCtx->pData, GFMRV_ALLOC_FAILED);
        
        pCtx->dataLen = widthInTiles * heightInTiles;
    }
    // Set the new dimensions
    pCtx->widthInTiles = widthInTiles;
    pCtx->heightInTiles = heightInTiles;
    
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
    
    // Set the tilemap dimensions
    rv = gfmTilemap_setDimensions(pCtx, widthInTiles, heightInTiles);
    ASSERT_NR(rv == GFMRV_OK);
    
    // Set all tiles to a default value
    i = 0;
    while (i < pCtx->dataLen) {
        pCtx->pData[i] = defTile;
        i++;
    }
    
    // Set the spriteset
    pCtx->pSset = pSset;
    // Reset some buffers
    gfmGenArr_reset(pCtx->pTAnims);
    pCtx->numAreas = 0;
    
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
    gfmGenArr_clean(pCtx->pTTypes, gfmTileType_free);
    gfmGenArr_clean(pCtx->pTAnims, gfmTileAnimation_free);
    gfmGenArr_clean(pCtx->pTAnimInfos, gfmTileAnimationInfo_free);
    gfmHitbox_free(&pCtx->pAreas);
    
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
    // -1 is used as default tile to fail the draw and 'draw' an invisible tile
    rv = gfmTilemap_init(pCtx, pCtx->pSset, mapWidth, mapHeight, -1/*defTile*/);
    ASSERT_NR(rv == GFMRV_OK);
    
    // Reset it back to the origin
    rv = gfmTilemap_setPosition(pCtx, 0, 0);
    ASSERT_NR(rv == GFMRV_OK);
    
    // Copy the new map into the tilemap
    // Can't think of a good reason not to use memcpy, so...
    memcpy(pCtx->pData, pData, dataLen*sizeof(int));
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Loads a tilemap from a file; It must be described as:
 * Tilemap := (<TileType>|<Area>)* <TilemapData>
 * TileType := type_str tile_index '\n'
 * Area := NOT_YET_IMPLEMENTED
 * TilemapData := "map" width_in_tiles height_in_tiles '\n'
 *                tile_1_1 tile_2_1 ... tile_(width_in_tiles - 1)_1
 *                ...
 *                tile_1_(height_in_tiles - 1) tile_2_(height_in_tiles - 1)
 *                      ... tile_(width_in_tiles - 1)_(height_in_tiles - 1)
 * 
 * Note that the 'type_str' will be searched in the passed dictionary
 * 
 * @param  pTMap       The tilemap
 * @param  pCtx        The game`s context
 * @param  pFilename   The file where the tile data is written
 * @param  filenameLen How many characters there are in the filename
 * @param  pDictNames  Dictionary with the types' names
 * @param  pDictTypes  Dictionary with the types's values
 * @param  dictLen     How many entries there are in the dictionary
 * @return             GFMRV_OK, GFMRV_ARGUMENTS_BAD,
 *                     GFMRV_TILEMAP_NOT_INITIALIZED, GFMRV_ALLOC_FAILED,
 *                     GFMRV_TILEMAP_PARSING_ERROR
 */
gfmRV gfmTilemap_loadf(gfmTilemap *pTMap, gfmCtx *pCtx, char *pFilename,
        int filenameLen, char *pDictNames[], int pDictTypes [], int dictLen) {
    return gfmTilemap_newLoadf(pTMap, pCtx, pFilename, filenameLen, pDictNames
            , pDictTypes, dictLen, 0, 0);
}

/**
 * Similar to gfmTilemap_loadf, but uses 'gfmTilemap_newRecalculateAreas' to
 * automatically generate the collision data.
 *
 * @param  [ in]pTMap       The tilemap
 * @param  [ in]pCtx        The game`s context
 * @param  [ in]pFilename   The file where the tile data is stored
 * @param  [ in]filenameLen How many characters there are in the filename
 * @param  [ in]pDictNames  Dictionary with the types' names
 * @param  [ in]pDictTypes  Dictionary with the types's values
 * @param  [ in]dictLen     How many entries there are in the dictionary
 * @param  [ in]pSidedTypes Types that should be converted into polygons sides.
 * @param  [ in]sidedLen    Number of entries in pSidedTypes.
 * @return                  The operation result.
 */
gfmRV gfmTilemap_newLoadf(gfmTilemap *pTMap, gfmCtx *pCtx, char *pFilename
        , int filenameLen, char *pDictNames[], int pDictTypes [], int dictLen
        , int *pSidedTypes, int sidedLen) {
    char *pTypeStr;
    gfmFile *pFp;
    gfmLog *pLog;
    gfmRV rv;
    int typeStrLen, didParseTilemap;
    
    // Set default values
    pFp = 0;
    pTypeStr = 0;
    typeStrLen = 0;
    didParseTilemap = 0;
    
    // Sanitize arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    // Retrieve the logger
    rv = gfm_getLogger(&pLog, pCtx);
    ASSERT(rv == GFMRV_OK, rv);
    // Continue to sanitize arguments
    ASSERT_LOG(pTMap, GFMRV_ARGUMENTS_BAD, pLog);
    ASSERT_LOG(pFilename, GFMRV_ARGUMENTS_BAD, pLog);
    ASSERT_LOG(filenameLen > 0, GFMRV_ARGUMENTS_BAD, pLog);
    ASSERT_LOG(pDictNames, GFMRV_ARGUMENTS_BAD, pLog);
    ASSERT_LOG(pDictTypes, GFMRV_ARGUMENTS_BAD, pLog);
    ASSERT_LOG(dictLen > 0, GFMRV_ARGUMENTS_BAD, pLog);
    
    rv = gfmLog_log(pLog, gfmLog_info, "Parsing tilemap \"%*s\"",
            filenameLen, pFilename);
    ASSERT_NR(rv == GFMRV_OK);
    
    // Open an asset file
    rv = gfmFile_getNew(&pFp);
    ASSERT_LOG(rv == GFMRV_OK, rv, pLog);
    rv = gfmFile_openAsset(pFp, pCtx, pFilename, filenameLen, 1/*isText*/);
    ASSERT_LOG(rv == GFMRV_OK, rv, pLog);
    
    // Reset all tile types
    gfmGenArr_reset(pTMap->pTTypes);
    
    // Loop through all characters in the files
    while (1) {
        // If we finished reading, stop
        rv = gfmFile_didFinish(pFp);
        ASSERT_LOG(rv == GFMRV_TRUE || rv == GFMRV_FALSE, rv, pLog);
        if (rv == GFMRV_TRUE) {
            break;
        }
        
        // Check if the current token is an "area"
        rv = gfmParser_parseStringStatic(pFp, "area");
        ASSERT_LOG(rv == GFMRV_TRUE || rv == GFMRV_FALSE, rv, pLog);
        if (rv == GFMRV_TRUE) {
            int i, height, width, x, y;
            
            // Read the current type
            rv = gfmParser_getString(&pTypeStr, &typeStrLen, pFp);
            ASSERT_LOG(rv == GFMRV_OK, rv, pLog);
            // Read the area's position
            rv = gfmParser_parseInt(&x, pFp);
            ASSERT_LOG(rv == GFMRV_OK, rv, pLog);
            rv = gfmParser_parseInt(&y, pFp);
            ASSERT_LOG(rv == GFMRV_OK, rv, pLog);
            // Read the area's dimension
            rv = gfmParser_parseInt(&width, pFp);
            ASSERT_LOG(rv == GFMRV_OK, rv, pLog);
            rv = gfmParser_parseInt(&height, pFp);
            ASSERT_LOG(rv == GFMRV_OK, rv, pLog);
            
            // Get its index from the dictionary
            i = 0;
            while (i < dictLen) {
                if (strcmp(pDictNames[i], pTypeStr) == 0) {
                    break;
                }
                i++;
            }
            ASSERT_LOG(i < dictLen, GFMRV_PARSER_ERROR, pLog);
            
            // Add the area
            rv = gfmTilemap_addArea(pTMap, x, y, width, height, pDictTypes[i]);
            ASSERT_NR(rv == GFMRV_OK);
            
            continue;
        }
        // Check if the current token is a "tile type" ("type")
        rv = gfmParser_parseStringStatic(pFp, "type");
        ASSERT_LOG(rv == GFMRV_TRUE || rv == GFMRV_FALSE, rv, pLog);
        if (rv == GFMRV_TRUE) {
            int i, tile;
            
            // Read the current type
            rv = gfmParser_getString(&pTypeStr, &typeStrLen, pFp);
            ASSERT_LOG(rv == GFMRV_OK, rv, pLog);
            // Read the type's tile
            rv = gfmParser_parseInt(&tile, pFp);
            ASSERT_LOG(rv == GFMRV_OK, rv, pLog);
            
            // Get its index from the dictionary
            i = 0;
            while (i < dictLen) {
                if (strcmp(pDictNames[i], pTypeStr) == 0) {
                    break;
                }
                i++;
            }
            ASSERT_LOG(i < dictLen, GFMRV_PARSER_ERROR, pLog);
            // Add it to the list
            rv = gfmTilemap_addTileType(pTMap, tile, pDictTypes[i]);
            ASSERT_LOG(rv == GFMRV_OK, rv, pLog);
            
            continue;
        }
        // Check if the current token is a "tilemap" ("map")
        rv = gfmParser_parseStringStatic(pFp, "anim");
        ASSERT_LOG(rv == GFMRV_TRUE || rv == GFMRV_FALSE, rv, pLog);
        if (rv == GFMRV_TRUE) {
            rv = gfmLog_log(pLog, gfmLog_info, "Got an 'anim' token but "
                    "can't handle it, yet");
            ASSERT_NR(rv == GFMRV_OK);
            // TODO Implement this
            continue;
        }
        // Check if the current token is a "tilemap" ("map")
        rv = gfmParser_parseStringStatic(pFp, "map");
        ASSERT_LOG(rv == GFMRV_TRUE || rv == GFMRV_FALSE, rv, pLog);
        if (rv == GFMRV_TRUE) {
            int height, i, width;
            
            // Get the tilemap's dimensions
            rv = gfmParser_parseInt(&width, pFp);
            ASSERT_LOG(rv == GFMRV_OK, rv, pLog);
            rv = gfmParser_parseInt(&height, pFp);
            ASSERT_LOG(rv == GFMRV_OK, rv, pLog);
            
            // Set the tilemap dimensions
            rv = gfmTilemap_setDimensions(pTMap, width, height);
            ASSERT_LOG(rv == GFMRV_OK, rv, pLog);
            
            // Read and set it the actual data
            i = 0;
            while (i < width * height) {
                int tile;
                
                // Get the tile
                rv = gfmParser_parseInt(&tile, pFp);
                ASSERT_LOG(rv == GFMRV_OK, rv, pLog);
                
                pTMap->pData[i] = tile;
                
                i++;
            }
            
            // Recalculate all areas that belong to the map
            if (pSidedTypes == 0 || sidedLen == 0) {
                rv = gfmTilemap_recalculateAreas(pTMap);
            }
            else {
                rv = gfmTilemap_newRecalculateAreas(pTMap, pSidedTypes, sidedLen);
            }
            ASSERT_LOG(rv == GFMRV_OK || rv == GFMRV_TILEMAP_NO_TILETYPE, rv, pLog);
            
            didParseTilemap = 1;
            continue;
        }
        ASSERT_LOG(0, GFMRV_READ_ERROR, pLog);
    }
    // Check that a tilemap was parsed
    ASSERT_LOG(didParseTilemap == 1, GFMRV_TILEMAP_NO_TILEMAP_PARSED, pLog);
    
    rv = gfmLog_log(pLog, gfmLog_info, "Tilemap parsed!");
    ASSERT_NR(rv == GFMRV_OK);
    
    // Recache the animations
    rv = gfmTilemap_recacheAnimations(pTMap);
    ASSERT_LOG(rv == GFMRV_OK || rv == GFMRV_TILEMAP_NO_TILEANIM, rv, pLog);
    
    rv = GFMRV_OK;
__ret:
    if (pFp) {
        gfmFile_free(&pFp);
    }
    if (pTypeStr) {
        free(pTypeStr);
    }
    
    return rv;
}

/**
 * Modify a tilemap position
 * 
 * @param  pCtx   The tilemap
 * @param  x      The tilemap top-left position
 * @param  y      The tilemap to-left position
 * @return        GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmTilemap_setPosition(gfmTilemap *pCtx, int x, int y) {
    gfmRV rv;
    
    // Sanitize arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    
    // Set the position
    pCtx->x = x;
    pCtx->y = y;
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Retrieves the tilemap data, so it can be modified (BE SURE TO RECALCULATE THE
 * AREA AFTEWARD)
 * 
 * @param  ppData The tilemap data
 * @param  pCtx   The tilemap
 * @return        GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_TILEMAP_NOT_INITIALIZED
 */
gfmRV gfmTilemap_getData(int **ppData, gfmTilemap *pCtx) {
    gfmRV rv;
    
    // Sanitize arguments
    ASSERT(ppData, GFMRV_ARGUMENTS_BAD);
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    // Check that it was initialized
    ASSERT(pCtx->pData, GFMRV_TILEMAP_NOT_INITIALIZED);
    
    *ppData = pCtx->pData;
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Adds a single rectangular area of a given type; Every object is set as fixed,
 * but if collision is not desired, simply don't call gfmObject_separate*
 * 
 * @param  pCtx   The tilemap
 * @param  x      The area top-left position
 * @param  y      The area to-left position
 * @param  width  The area width
 * @param  height The area height
 * @param  type   The area type (i.e., the gfmObject's child type)
 * @return        GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_ALLOC_FAILED
 */
gfmRV gfmTilemap_addArea(gfmTilemap *pCtx, int x, int y, int width,
        int height, int type) {
    gfmRV rv;

    /* Sanitize arguments */
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    ASSERT(x >= 0, GFMRV_ARGUMENTS_BAD);
    ASSERT(y >= 0, GFMRV_ARGUMENTS_BAD);
    ASSERT(width > 0, GFMRV_ARGUMENTS_BAD);
    ASSERT(height > 0, GFMRV_ARGUMENTS_BAD);
    /* The type must be something not used by the lib */
    ASSERT(type >= gfmType_reserved_2, GFMRV_ARGUMENTS_BAD);

    /* Alloc or expand the buffer as necessary */
    if (pCtx->pAreas == 0) {
        pCtx->areaCount = 16;
        pCtx->numAreas = 0;
        rv = gfmHitbox_getNewList(&pCtx->pAreas, pCtx->areaCount);
        ASSERT(rv == GFMRV_OK, rv);
    }
    else if (pCtx->numAreas == pCtx->areaCount) {
        rv = gfmHitbox_expandList(&pCtx->pAreas, pCtx->areaCount
                , pCtx->areaCount * 2);
        ASSERT(rv == GFMRV_OK, rv);
        pCtx->areaCount *= 2;
    }

    rv = gfmHitbox_initItem(pCtx->pAreas, 0/*child*/, x, y, width
        , height, type, pCtx->numAreas);
    ASSERT(rv == GFMRV_OK, rv);
    pCtx->numAreas++;

    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Adds a single rectangular area of a given type; Every object is set as fixed,
 * but if collision is not desired, simply don't call gfmObject_separate*
 *
 * @param  pCtx     The tilemap
 * @param  x        The area top-left position
 * @param  y        The area to-left position
 * @param  width    The area width
 * @param  height   The area height
 * @param  type     The area type (i.e., the gfmObject's child type)
 * @param  hitFlags Bitmask with the directions that trigger collision
 * @return          GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_ALLOC_FAILED
 */
static gfmRV gfmTilemap_addSidedArea(gfmTilemap *pCtx, int x, int y, int width,
        int height, int type, gfmCollision hitFlags) {
    gfmRV rv;

    rv = gfmTilemap_addArea(pCtx, x, y, width, height, type);
    ASSERT_NR(rv == GFMRV_OK);
    rv = gfmHitbox_setItemHitFlag(pCtx->pAreas, hitFlags, pCtx->numAreas - 1);
    ASSERT_NR(rv == GFMRV_OK);

    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Get how many areas there are in the tilemap
 * 
 * @param  pLen The number of areas
 * @param  pCtx The tilemap
 * @return        GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_TILEMAP_NOT_INITIALIZED
 */
gfmRV gfmTilemap_getAreasLength(int *pLen, gfmTilemap *pCtx) {
    gfmRV rv;
    
    // Sanitize arguments
    ASSERT(pLen, GFMRV_ARGUMENTS_BAD);
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    // Check that it was initialized
    ASSERT(pCtx->pSset, GFMRV_TILEMAP_NOT_INITIALIZED);
    
    // Retrieve the array length
    *pLen = (int)pCtx->numAreas;
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Get an area from the tilemap
 * 
 * @param  ppObj The retrieved area
 * @param  pCtx  The tilemap
 * @param  i     Index of the desired area
 * @return       GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_INVALID_INDEX
 */
gfmRV gfmTilemap_getArea(gfmObject **ppObj, gfmTilemap *pCtx, int i) {
    gfmRV rv;
    
    // Sanitize the arguments
    ASSERT(ppObj, GFMRV_ARGUMENTS_BAD);
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    // Check that the index is valid
    ASSERT(i >= 0, GFMRV_INVALID_INDEX);
    ASSERT(i < pCtx->numAreas, GFMRV_INVALID_INDEX);
    
    // Retrieve the object
    rv = gfmHitbox_getItem((gfmHitbox**)ppObj, pCtx->pAreas, i);
__ret:
    return rv;
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
    delay = 1000 / fps;
    
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
    if (gfmGenArr_getUsed(pCtx->pTAnimInfos) <= 0) {
        rv = GFMRV_TILEMAP_NO_TILEANIM;
        goto __ret;
    }
    
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
 * Add a tile type, used when automatically generating areas; Note that the
 * type must be an integer greater or equal to gfmType_reserved_2 (from
 * gfmTypes.h);
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
    // The type must be something not used by the lib
    ASSERT(type >= gfmType_reserved_2, GFMRV_ARGUMENTS_BAD);
    
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
    if (i < gfmGenArr_getUsed(pCtx->pTTypes)) {
        rv = GFMRV_OK;
    }
    else {
        rv = GFMRV_TILEMAP_NO_TILETYPE;
    }
    
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
    // Get the tile's center
    x = width * x + width / 2;
    y = height * y + height / 2;
    
    // Iterate the array, checking every tile
    i = 0;
    while (i < pCtx->numAreas) {
        gfmHitbox *pObj;
        
        // Get the next tile type
        rv = gfmHitbox_getItem(&pObj, pCtx->pAreas, i);
        ASSERT_NR(rv == GFMRV_OK);
        
        // Check if the tiles is inside this object
        rv = gfmObject_isPointInside((gfmObject*)pObj, x, y);
        // Exist on not inside, though it's not (necessarily) an error!
        if (rv != GFMRV_FALSE) {
            goto __ret;
        }
        
        i++;
    }
    
    rv = GFMRV_FALSE;
__ret:
    return rv;
}

/**
 * Retrieve the type of the tile at a give pixel position
 *
 * @param  [out]pType The type
 * @param  [ in]pCtx  The tilemap
 * @param  [ in]x     Horizontal position of the tile
 * @param  [ in]y     Vertical position of the tile
 */
gfmRV gfmTilemap_getTypeAt(int *pType, gfmTilemap *pCtx, int x, int y) {
    gfmRV rv;
    int tileWidth, tileHeight;

    /* Sanitize arguments */
    ASSERT(pType, GFMRV_ARGUMENTS_BAD);
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    ASSERT(pCtx->pSset, GFMRV_TILEMAP_NOT_INITIALIZED);

    rv = gfmSpriteset_getDimension(&tileWidth, &tileHeight, pCtx->pSset);
    ASSERT_NR(rv == GFMRV_OK);
    x /= tileWidth;
    y /= tileHeight;

    ASSERT(x >= 0 && x < pCtx->widthInTiles, GFMRV_ARGUMENTS_BAD);
    ASSERT(y >= 0 && y < pCtx->heightInTiles, GFMRV_ARGUMENTS_BAD);

    /* Get the tile's type */
    rv = gfmTilemap_getTileType(pType, pCtx
            , pCtx->pData[x + y * pCtx->widthInTiles]);
    ASSERT_NR(rv == GFMRV_OK);

    rv = GFMRV_OK;
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
        ASSERT_NR(rv == GFMRV_OK || rv == GFMRV_TILEMAP_NO_TILETYPE);
        // Stop if it's a different type of tile
        if (nextType != type || rv == GFMRV_TILEMAP_NO_TILETYPE)
            break;
        // Also check that this new tile doesn't belong to another area
        rv = gfmTilemap_isTileInAnyArea(pCtx, tileIndex + i);
        if (rv == GFMRV_TRUE)
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
            ASSERT_NR(rv == GFMRV_OK || rv == GFMRV_TILEMAP_NO_TILETYPE);
            // Stop if it's a different type of tile
            if (rv == GFMRV_TILEMAP_NO_TILETYPE || nextType != type)
                break;
            // Also check that this new tile doesn't belong to another area
            rv = gfmTilemap_isTileInAnyArea(pCtx,
                    tileIndex + i + j * pCtx->widthInTiles);
            if (rv == GFMRV_TRUE)
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
 * Automatically generates all areas in the tilemap. This version creates one
 * rectangle per side of the supplied types. Otherwise, the old algorithm is
 * used.
 * 
 * @param  pCtx        The tilemap
 * @param  pSidedTypes Types that should be converted into polygons sides.
 * @return             GFMRV_OK, GFMRV_ARGUMENTS_BAD,
 *                     GFMRV_TILEMAP_NOT_INITIALIZED, GFMRV_TILEMAP_NO_TILETYPE,
 *                     GFMRV_ALLOC_FAILED
 */
gfmRV gfmTilemap_newRecalculateAreas(gfmTilemap *pCtx, int *pSidedTypes, int dictLen) {
    int *left = 0, *right = 0;
    gfmRV rv;
    int y;

    // Sanitize arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    ASSERT(pSidedTypes == 0 || dictLen > 0, GFMRV_ARGUMENTS_BAD);
    // Check that it was initialized
    ASSERT(pCtx->pSset, GFMRV_TILEMAP_NOT_INITIALIZED);
    // Check that there is at least one tile type
    ASSERT(gfmGenArr_getUsed(pCtx->pTTypes), GFMRV_TILEMAP_NO_TILETYPE);

    // Prepare a temporary list of items to calculate vertical colliders in a single pass.
    left = (int*)malloc(sizeof(int) * pCtx->widthInTiles);
    ASSERT(left, GFMRV_ALLOC_FAILED);
    memset(left, -1, sizeof(int) * pCtx->widthInTiles);

    right = (int*)malloc(sizeof(int) * pCtx->widthInTiles);
    ASSERT(right, GFMRV_ALLOC_FAILED);
    memset(right, -1, sizeof(int) * pCtx->widthInTiles);

    // Reset the previous areas
    pCtx->numAreas = 0;

    // Traverse every tile, looking around it to detect edges.
    for (y = 0; y < pCtx->heightInTiles; y++) {
        int xTop = -1, xBot = -1;
        int x;

        for (x = 0; x < pCtx->widthInTiles; x++) {
            int isSame[9];
            int j, curType;

            memset(isSame, 0, sizeof(isSame));

            j = pCtx->pData[x + y * pCtx->widthInTiles];
            rv = gfmTilemap_getTileType(&curType, pCtx, j);
            if (rv == GFMRV_TILEMAP_NO_TILETYPE) {
                continue;
            }

            // Inspect the surroudings for tiles that
            // are of the same type of the current tile.
            for (j = 0; j < 3; j++) {
                int i, convY;

                convY = y + j - 1;
                if (convY < 0 || convY >= pCtx->heightInTiles) {
                    continue;
                }

                for (i = 0; i < 3; i++) {
                    int type;
                    int tile, convX;

                    convX = x + i - 1;
                    if (convX < 0 || convX >= pCtx->widthInTiles) {
                        continue;
                    }

                    tile = pCtx->pData[convX + convY * pCtx->widthInTiles];

                    rv = gfmTilemap_getTileType(&type, pCtx, tile);
                    isSame[i + j * 3] = (rv == GFMRV_OK && type == curType);
                }
            }

            // Generate corner tiles to fix potential issues.
            if (!isSame[3] && !isSame[1] && !isSame[7] && !isSame[5]) {
                // Single tile.
                rv = gfmTilemap_addSidedArea(pCtx, x * 8, y * 8, 8, 8, curType,
                        gfmCollision_up | gfmCollision_down | gfmCollision_left | gfmCollision_right);
                ASSERT_NR(rv == GFMRV_OK);
                continue;
            }
            else if (!isSame[3] && !isSame[1] && !isSame[5]) {
                // Individual top tile.
                rv = gfmTilemap_addSidedArea(pCtx, x * 8, y * 8, 8, 8, curType,
                        gfmCollision_up | gfmCollision_left | gfmCollision_right);
                ASSERT_NR(rv == GFMRV_OK);
            }
            else if (!isSame[3] && !isSame[7] && !isSame[5]) {
                // Individual bottom tile.
                rv = gfmTilemap_addSidedArea(pCtx, x * 8, y * 8, 8, 8, curType,
                        gfmCollision_down | gfmCollision_left | gfmCollision_right);
                ASSERT_NR(rv == GFMRV_OK);
            }
            else if (!isSame[3] && !isSame[1] && !isSame[7]) {
                // Individual left tile.
                rv = gfmTilemap_addSidedArea(pCtx, x * 8, y * 8, 8, 8, curType,
                        gfmCollision_up | gfmCollision_down | gfmCollision_left);
                ASSERT_NR(rv == GFMRV_OK);
            }
            else if (!isSame[3] && !isSame[1]) {
                // Top left corner.
                rv = gfmTilemap_addSidedArea(pCtx, x * 8, y * 8, 8, 8, curType,
                        gfmCollision_up | gfmCollision_left);
                ASSERT_NR(rv == GFMRV_OK);
            }
            else if (!isSame[3] && !isSame[7]) {
                // Bottom left corner.
                rv = gfmTilemap_addSidedArea(pCtx, x * 8, y * 8, 8, 8, curType,
                        gfmCollision_down | gfmCollision_left);
                ASSERT_NR(rv == GFMRV_OK);
            }
            else if (!isSame[5] && !isSame[1] && !isSame[7]) {
                // Individual right tile.
                rv = gfmTilemap_addSidedArea(pCtx, x * 8, y * 8, 8, 8, curType,
                        gfmCollision_up | gfmCollision_down | gfmCollision_right);
                ASSERT_NR(rv == GFMRV_OK);
            }
            else if (!isSame[5] && !isSame[1]) {
                // Top right corner.
                rv = gfmTilemap_addSidedArea(pCtx, x * 8, y * 8, 8, 8, curType,
                        gfmCollision_up | gfmCollision_right);
                ASSERT_NR(rv == GFMRV_OK);
            }
            else if (!isSame[5] && !isSame[7]) {
                // Bottom right corner.
                rv = gfmTilemap_addSidedArea(pCtx, x * 8, y * 8, 8, 8, curType,
                        gfmCollision_down | gfmCollision_right);
                ASSERT_NR(rv == GFMRV_OK);
            }

            if (!isSame[1] && (!isSame[3] || isSame[0])) {
                // Top begin.
                xTop = x;
            }
            if (!isSame[1] && (isSame[2] || !isSame[5])) {
                int w, h;

                // Top end.
                if (xTop == -1) {
                    xTop = x;
                }

                w = (1 + x - xTop) * 8;
                h = 8;
                rv = gfmTilemap_addSidedArea(pCtx, xTop * 8, y * 8, w, h
                        , curType, gfmCollision_up);
                ASSERT_NR(rv == GFMRV_OK);

                xTop = -1;
            }

            if (!isSame[7] && (!isSame[3] || isSame[6])) {
                // Bottom begin.
                xBot = x;
            }
            if (!isSame[7] && (isSame[8] || !isSame[5])) {
                int w, h;

                // Bottom end.
                if (xBot == -1) {
                    xBot = x;
                }

                w = (1 + x - xBot) * 8;
                h = 8;
                rv = gfmTilemap_addSidedArea(pCtx, xBot * 8, y * 8, w, h
                        , curType, gfmCollision_down);
                ASSERT_NR(rv == GFMRV_OK);

                xBot = -1;
            }

            if (!isSame[3] && (!isSame[1] || isSame[0])) {
                // Left begin.
                left[x] = y;
            }
            if (!isSame[3] && (isSame[6] || !isSame[7])) {
                int w, h;

                // Left end.
                if (left[x] == -1) {
                    left[x] = y;
                }

                w = 8;
                h = (1 + y - left[x]) * 8;
                rv = gfmTilemap_addSidedArea(pCtx, x * 8, left[x] * 8, w, h
                        , curType, gfmCollision_left);
                ASSERT_NR(rv == GFMRV_OK);

                left[x] = -1;
            }

            if (!isSame[5] && (!isSame[1] || isSame[2])) {
                // Right begin.
                right[x] = y;
            }
            if (!isSame[5] && (isSame[8] || !isSame[7])) {
                int w, h;

                // Right end.
                if (right[x] == -1) {
                    right[x] = y;
                }

                w = 8;
                h = (1 + y - right[x]) * 8;
                rv = gfmTilemap_addSidedArea(pCtx, x * 8, right[x] * 8, w, h
                        , curType, gfmCollision_right);
                ASSERT_NR(rv == GFMRV_OK);

                right[x] = -1;
            }
        }
    }

    rv = GFMRV_OK;
__ret:
    if (right) {
        free(right);
    }
    if (left) {
        free(left);
    }

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
    return gfmTilemap_newRecalculateAreas(pCtx, 0, 0);
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
 * Run through every animated tile and update its time and the tile itself
 * 
 * @param  pTMap The tilemap
 * @param  pCtx  The game's context
 * @return       GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_TILEMAP_NOT_INITIALIZED
 */
gfmRV gfmTilemap_update(gfmTilemap *pTMap, gfmCtx *pCtx) {
    gfmRV rv;
    gfmTileAnimation *pTAnim;
    gfmTileAnimationInfo *pTInfo;
    int i, ms;
    
    // Sanitize arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    ASSERT(pTMap, GFMRV_ARGUMENTS_BAD);
    // Check that the tilemap was initialzied
    ASSERT(pTMap->pSset, GFMRV_TILEMAP_NOT_INITIALIZED);
    // If there are no animations, do nothing
    if (gfmGenArr_getUsed(pTMap->pTAnims) <= 0) {
        rv = GFMRV_OK;
        goto __ret;
    }
    
    // Get the elapsed time
    rv = gfm_getElapsedTime(&ms, pCtx);
    ASSERT_NR(rv == GFMRV_OK);
    
    // Loop through every animation and updates it
    i = 0;
    while (i < gfmGenArr_getUsed(pTMap->pTAnims)) {
        // Get the next animation
        pTAnim = gfmGenArr_getObject(pTMap->pTAnims, i);
        
        // Check if it's a valid animation
        CASSERT_NR(pTAnim->index >= 0, __nextAnim);
        // Updates its delay
        pTAnim->delay -= ms;
        
        // If a new frame was issued, update the animation and the tile
        while (pTAnim->delay <= 0) {
            // Get the current animation's info
            pTInfo = gfmGenArr_getObject(pTMap->pTAnimInfos, pTAnim->infoIndex);
            
            // Update the tilemap
            pTMap->pData[pTAnim->index] = pTInfo->nextTile;
            
            // Update the animation
            if (pTInfo->nextTileIndex >= 0) {
                pTAnim->infoIndex = pTInfo->nextTileIndex;
                // Get the next animation's info
                pTInfo = gfmGenArr_getObject(pTMap->pTAnimInfos,
                        pTAnim->infoIndex);
                // Update the animation's delay (accumulate over the previous)
                pTAnim->delay += pTInfo->delay;
            }
            else {
                // "Remove" the animation from the list
                pTAnim->index = -1;
                break;
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
    int camX, camY, camWidth, camHeight;
    int i, j;
    int horTiles, verTiles;
    int screenX, screenY;
    int tileX, tileY, tileWidth, tileHeight;
    
    // Sanitize arguments
    ASSERT(pTMap, GFMRV_ARGUMENTS_BAD);
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    // Check if initialized
    ASSERT(pTMap->pSset, GFMRV_TILEMAP_NOT_INITIALIZED);
    ASSERT(pTMap->widthInTiles > 0, GFMRV_TILEMAP_NOT_INITIALIZED);
    ASSERT(pTMap->heightInTiles > 0, GFMRV_TILEMAP_NOT_INITIALIZED);
    
    // Get the tile's dimension
    rv = gfmSpriteset_getDimension(&tileWidth, &tileHeight, pTMap->pSset);
    ASSERT_NR(rv == GFMRV_OK);
    // Get camera's dimension
    rv = gfm_getCameraPosition(&camX, &camY, pCtx);
    rv = gfm_getCameraDimensions(&camWidth, &camHeight, pCtx);
    
    // Get the horizontal index for first visible tile on the screen and its in
    // screen position
    if (camX <= pTMap->x) {
        tileX = 0;
        screenX = pTMap->x - camX;
    }
    else {
        tileX = (camX - pTMap->x) / tileWidth;
        // The position will be either 0 or the start of the previous tile
        screenX = -((camX - pTMap->x) % tileWidth);
    }
    // Get the vertical index for first visible tile on the screen and its in
    // screen position
    if (camY <= pTMap->y) {
        tileY = 0;
        screenY = pTMap->y - camY;
    }
    else {
        tileY = (camY - pTMap->y) / tileHeight;
        // The position will be either 0 or the start of the previous tile
        screenY = -((camY -pTMap->y) % tileHeight);
    }
    
    // Get how many tiles must be drawn horizontally
    horTiles = pTMap->widthInTiles - tileX;
    if (horTiles > camWidth / tileWidth + 2) {
        horTiles = camWidth / tileWidth + 2;
    }
    
    // Get how many tiles must be drawn vertically
    verTiles = pTMap->heightInTiles - tileY;
    if (verTiles > camHeight / tileHeight + 2) {
        verTiles = camHeight / tileHeight + 2;
    }
    
    i = 0;
    j = 0;
    // Loop through every visible tile
    while (j < verTiles) {
        int tile;
        
        // Get the tile
        tile = pTMap->pData[(tileX + i) + (tileY + j) * pTMap->widthInTiles];
        // Render the tile to the screen (ignore errors)
        gfm_drawTile(pCtx, pTMap->pSset, screenX + i * tileWidth,
                screenY + j * tileHeight, tile, 0/*isFlipped*/);
        
        i++;
        if (i >= horTiles) {
            i = 0;
            j++;
        }
    }
    
    rv = GFMRV_OK;
__ret:
    return rv;
}
/*
    gfmRV rv;
    int camX, camY, camWidth, camHeight, dX, firstTile, i, iniX, offX,
            tile, tileX, tileY, tileWidth, tileHeight, x, y;
    
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
    // If the tilemap is smaller than the camera, we would have to go back some
    // tiles; Simply ignore that
    if (dX < 0)
        dX = 0;
    
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
        // Ignore the rv, as there are invalid tiles (-1) for "drawing 
        // invisible tiles"
        
        // Update the tile position
        i++;
        x += tileWidth;
        if (x >= camWidth ||
                ((firstTile + i + offX) % pTMap->widthInTiles) == 0) {
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
*/

