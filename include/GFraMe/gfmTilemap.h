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
 * types to the map and requesting it to recalculate the 'areas'.
 * Another feature is to have animated tiles. This is created through the
 * association of a tile with its next frame and the delay before the transition
 */
#ifndef __GFMTILEMAP_STRUCT__
#define __GFMTILEMAP_STRUCT__

/** 'Exports' the gfmTilemap structure */
typedef struct stGFMTilemap gfmTilemap;

#endif /* __GFMTILEMAP_STRUCT__ */

#ifndef __GFMTILEMAP_H__
#define __GFMTILEMAP_H__

#include <GFraMe/gframe.h>
#include <GFraMe/gfmError.h>
//#include <GFraMe/gfmObject.h>
#include <GFraMe/gfmSpriteset.h>

/** 'Exportable' size of gfmTilemap */
extern const int sizeofGFMTilemap;

/**
 * Alloc a new tilemap
 * 
 * @param  ppCtx  The allocated tilemap
 * @return        GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_ALLOC_FAILED
 */
gfmRV gfmTilemap_getNew(gfmTilemap **ppCtx);

/**
 * Free and clean a previously allocated tilemap
 * 
 * @param  ppCtx  The tilemap
 * @return        GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmTilemap_free(gfmTilemap **ppCtx);

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
        int heightInTiles, int defTile);

/**
 * Free everything allocated by init
 * 
 * @param  pCtx   The tilemap
 * @return        GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmTilemap_clean(gfmTilemap *pCtx);

gfmRV gfmTilemap_load(gfmTilemap *pCtx, int *pData, int dataLen);
#define gfmTilemap_loadStatic(pCtx, pData) \
    gfmTilemap_load(pCtx, pData, (int)(sizeof(pData)/sizeof(int)));
gfmRV gfmTilemap_loadf(gfmTilemap *pCtx, char *pFilename, int filenameLen);
#define gfmTilemap_loadfStatic(pCtx, pFilename) \
    gfmTilemap_loadf(pCtx, pFilename, sizeof(pFilename)-1)
gfmRV gfmTilemap_addArea(gfmTilemap *pCtx, int x, int y, int width,
        int height, int type);
gfmRV gfmTilemap_addAreas(gfmTilemap *pCtx, int *pData, int dataLen);
#define gfmTilemap_addAreasStatic(pCtx, pData) \
    gfmTilemap_addAreasStatic(pCtx, pData, (int)(sizeof(pData) / sizeof(int)))

/**
 * Add a tile type, used when automatically generating areas
 * 
 * @param  pCtx The tilemap
 * @param  tile The tile
 * @param  type The tile's type
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_TILEMAP_NOT_INITIALIZED,
 *              GFMRV_TILETYPE_EXTANT
 */
gfmRV gfmTilemap_addTileType(gfmTilemap *pCtx, int tile, int type);

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
gfmRV gfmTilemap_addTileTypes(gfmTilemap *pCtx, int *pData, int dataLen);

/**
 * Add various tile types at once; 'pData' must be a static buffer (as in,
 * 'int data[] = { ... };');
 * 
 * Look at 'gfmTilemap_addTileTypes' documentation to see the expected format
 *
 * @param  pCtx    The tilemap
 * @param  pData   The tile types data
 * @return         GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_TILEMAP_NOT_INITIALIZED,
 *                 GFMRV_TILETYPE_EXTANT
 */
#define gfmTilemap_addTileTypesStatic(pCtx, pData) \
    gfmTilemap_addTileTypes(pCtx, pData, (int)(sizeof(pData) / sizeof(int)))

gfmRV gfmTilemap_isTileInArea(gfmTilemap *pCtx, int tile, int type);
gfmRV gfmTilemap_isTileInAnyArea(gfmTilemap *pCtx, int tile);
gfmRV gfmTilemap_recalculateAreas(gfmTilemap *pCtx);

/**
 * Get the map's dimensions in pixels
 * 
 * @param  pWidth  The tilemap's width
 * @param  pHeight The tilemap's height
 * @param  pCtx    The tilemap
 * @return         GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_TILEMAP_NOT_INITIALIZED
 */
gfmRV gfmTilemap_getDimension(int *pWidth, int *pHeight, gfmTilemap *pCtx);

/**
 * Disable batched draw; It should be used when it's desired to batch more tiles
 * at once
 * 
 * @param  pCtx The tilemap
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmTilemap_disableBatchedDraw(gfmTilemap *pCtx);

/**
 * Enable batched draw; It should be used when it's desired to batch more tiles
 * at once
 * 
 * @param  pCtx The tilemap
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmTilemap_enableBatchedDraw(gfmTilemap *pCtx);

/**
 * Run through every animated tile and update its time and the tile itself
 * 
 * @param  pCtx The tilemap
 * @param  ms   Time, in milliseconds, elapsed from the previous frame
 */
gfmRV gfmTilemap_update(gfmTilemap *pCtx, int ms);

/**
 * Draw every tile that's inside tha world's camera
 * 
 * @param  pTMap The tilemap
 * @param  pCtx  The game's context
 * @return       GFMRV_OK, GFMRV_ARGUMENTS_BAD, ...
 */
gfmRV gfmTilemap_draw(gfmTilemap *pTMap, gfmCtx *pCtx);

#endif /* __GFMTILEMAP_H__ */

