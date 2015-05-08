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
#include <GFraMe/gfmSpriteset.h>
#include <GFraMe/gfmTilemap.h>
#include <GFraMe_int/gfmTileType.h>

#include <stdlib.h>
#include <string.h>

/** Define an array for tile types */
gfmGenArr_define(gfmTileType);
/** Define an array for gfmObjects */
//gfmGenArr_define(gfmObject);

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
//    gfmGenArr_var(gfmObject, pAreas);
    /** Tiles and theirs respective types */
    gfmGenArr_var(gfmTileType, pTypes);
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
        pCtx->dataLen[i] = defTile;
        i++;
    }
    
    // Set the spriteset
    pCtx->pSset = pSset;
    
    // Clean every buffer (so it can be recycled)
//    gfmGenArr_reset(pAreas);
    gfmGenArr_reset(pTypes);
    
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
//    gfmGenArr_clean(pAreas, gfmObject_free);
    gfmGenArr_clean(pTypes, gfmTileType_free);
    
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
    gfmGenArr_getNextRef(gfmTileType, pTypes, 1/*INC*/, pTType, gfmTileType_getNew);
    gfmGenArr_push(gfmTileType);
    
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
 */
gfmRV gfmTilemap_update(gfmTilemap *pCtx, int ms) {
    gfmRV rv;
    
    ASSERT(0, GFMRV_FUNCTION_NOT_IMPLEMENTED);
    
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
gfmRV gfmTilemap_draw(gfmTilemap *pTMap, gfmCtx *pCtx);
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
    rv = gfmSpriteset_getDimension(&tileWidth, &tileHeight, pCtx->pSset);
    ASSERT_NR(rv == GFMRV_OK);
    // Get camera's dimension
    rv = gfmCamera_getPosition(&camX, &camY, gfmCamera *pCtx);
    rv = gfmCamera_getDimensions(&camWidth, &camHeight, gfmCamera *pCtx);
    
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
    
    // TODO check if should batch
    
    // Render every tile
    i = 0;
    x = iniX;
    offX = 0;
    while (firstTile + offX + i < pTMap->widthInTiles * pTMap->heightInTiles
            && y < camH) {
        // Get the tile
        tile = pTMap[firstTile + offX + i];
        
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
    // TODO check if should end batch
    
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
