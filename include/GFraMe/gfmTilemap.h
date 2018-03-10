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
#include <GFraMe/gfmObject.h>
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
        int mapHeight);

/**
 * Loads a tilemap from a static buffer; for more info, read gfmTilemap_load's
 * documentation
 * 
 * @param  pCtx      The tilemap
 * @param  pData     The tiles data
 * @param  mapWidth  Width of the map, in tiles
 * @param  mapHeight Height of the map, in tiles
 * @return           GFMRV_OK, GFMRV_ARGUMENTS_BAD,
 *                   GFMRV_TILEMAP_NOT_INITIALIZED, GFMRV_ALLOC_FAILED
 */
#define gfmTilemap_loadStatic(pCtx, pData, mapWidth, mapHeight) \
    gfmTilemap_load(pCtx, pData, (int)(sizeof(pData)/sizeof(int)), mapWidth, \
            mapHeight)

/**
 * Loads a tilemap from a file; It must be described as:
 * Tilemap := (<TileType>|<Area>)* <TilemapData>
 * TileType := type_str tile_index '\n'
 * Area := NOT_YET_IMPLEMENTED
 * TilemapData := "map" width_in_tiles height_in_tiles '\n'
 *                tile_1_1 ',' tile_2_1 ',' ... ','
 *                      tile_(width_in_tiles - 1)_1 '\n'
 *                ...
 *                tile_1_(height_in_tiles - 1) ',' tile_2_(height_in_tiles - 1)
 *                      ',' ... ',' tile_(width_in_tiles - 1)_(height_in_tiles - 1)
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
        int filenameLen, char *pDictNames[], int pDictTypes [], int dictLen);

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
        , int *pSidedTypes, int sidedLen);

/**
 * Modify a tilemap position
 * 
 * @param  pCtx   The tilemap
 * @param  x      The tilemap top-left position
 * @param  y      The tilemap to-left position
 * @return        GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmTilemap_setPosition(gfmTilemap *pCtx, int x, int y);

/**
 * Retrieves the tilemap data, so it can be modified (BE SURE TO RECALCULATE THE
 * AREA AFTEWARD)
 * 
 * @param  ppData The tilemap data
 * @param  pCtx   The tilemap
 * @return        GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_TILEMAP_NOT_INITIALIZED
 */
gfmRV gfmTilemap_getData(int **ppData, gfmTilemap *pCtx);

/**
 * Adds a single rectangular area of a given type
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
        int height, int type);
gfmRV gfmTilemap_addAreas(gfmTilemap *pCtx, int *pData, int dataLen);
#define gfmTilemap_addAreasStatic(pCtx, pData) \
    gfmTilemap_addAreasStatic(pCtx, pData, (int)(sizeof(pData) / sizeof(int)))

/**
 * Get how many areas there are in the tilemap
 * 
 * @param  pLen The number of areas
 * @param  pCtx The tilemap
 * @return        GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_TILEMAP_NOT_INITIALIZED
 */
gfmRV gfmTilemap_getAreasLength(int *pLen, gfmTilemap *pCtx);

/**
 * Get an area from the tilemap
 * 
 * @param  ppObj The retrieved area
 * @param  pCtx  The tilemap
 * @param  i     Index of the desired area
 * @return       GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_INVALID_INDEX
 */
gfmRV gfmTilemap_getArea(gfmObject **ppObj, gfmTilemap *pCtx, int i);

/**
 * Add a two frames animation (i.e., from one tile to another)
 * 
 * @param  pCtx     The tilemap
 * @param  tile     The tile
 * @param  delay    How long (in milliseconds) until the next tile
 * @param  nextTile Tile to which it will change
 * @return          GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_TILEANIM_EXTANT,
 *                  GFMRV_ALLOC_FAILED
 */
gfmRV gfmTilemap_addTileAnimation(gfmTilemap *pCtx, int tile, int delay,
        int nextTile);

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
gfmRV gfmTilemap_addAnimation(gfmTilemap *pCtx, int *pData, int numFrames,
        int fps, int doLoop);

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
gfmRV gfmTilemap_addAnimations(gfmTilemap *pCtx, int *pData, int dataLen);

/**
 * Add a batch of animations from a static buffer; Read
 * gfmTilemap_addAnimations's documentation from the expected format
 * 
 * @param  pCtx  The tilemap
 * @param  pData The batch of animations
 * @return           GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_TILEANIM_EXTANT,
 *                   GFMRV_ALLOC_FAILED
 */
#define gfmTilemap_addAnimationsStatic(pCtx, pData) \
    gfmTilemap_addAnimations(pCtx, pData, sizeof(pData) / sizeof(int))

/**
 * Go through the map and cache every tile with an animation; It also calculate
 * the nextAnimIndex for every animation info
 * 
 * @param  pCtx  The tilemap
 * @return       GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_TILEMAP_NOT_INITIALIZED,
 *               GFMRV_TILEMAP_NO_TILEANIM
 */
gfmRV gfmTilemap_recacheAnimations(gfmTilemap *pCtx);

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
gfmRV gfmTilemap_getTileType(int *pType, gfmTilemap *pCtx, int tile);

/**
 * Check if the indexed tile is already inside any of the areas
 * 
 * @param  pCtx      The tilemap
 * @param  tileIndex The index of the tile
 * @return           GFMRV_TRUE, GFMRV_FALSE, GFMRV_ARGUMENTS_BAD,
 *                   GFMRV_TILEMAP_NOT_INITIALIZED
 */
gfmRV gfmTilemap_isTileInAnyArea(gfmTilemap *pCtx, int tileIndex);

/**
 * Traverse the map, from a given tile, getting the biggest rectangle that contains
 * all neighboring tiles of the same type
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
        gfmTilemap *pCtx, int tileIndex);

/**
 * Automatically generates all areas in the tilemap. This version creates one
 * rectangle per side of the supplied types. Otherwise, the old algorithm is
 * used.
 * 
 * @param  pCtx        The tilemap
 * @param  pSidedTypes Types that should be converted into polygons sides.
 * @return             GFMRV_OK, GFMRV_ARGUMENTS_BAD,
 *                     GFMRV_TILEMAP_NOT_INITIALIZED, GFMRV_TILEMAP_NO_TILETYPE
 */
gfmRV gfmTilemap_newRecalculateAreas(gfmTilemap *pCtx, int *pSidedTypes
        , int dictLen);

/**
 * Automatically generates all areas in the tilemap
 * 
 * @param  pCtx The tilemap
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_TILEMAP_NOT_INITIALIZED,
 *              GFMRV_TILEMAP_NO_TILETYPE
 */
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
 * Retrieve the type of the tile at a give pixel position
 *
 * @param  [out]pType The type
 * @param  [ in]pCtx  The tilemap
 * @param  [ in]x     Horizontal position of the tile
 * @param  [ in]y     Vertical position of the tile
 */
gfmRV gfmTilemap_getTypeAt(int *pType, gfmTilemap *pCtx, int x, int y);

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
 * @param  pTMap The tilemap
 * @param  pCtx  The game's context
 * @return       GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_TILEMAP_NOT_INITIALIZED
 */
gfmRV gfmTilemap_update(gfmTilemap *pTMap, gfmCtx *pCtx);

/**
 * Draw every tile that's inside tha world's camera
 * 
 * @param  pTMap The tilemap
 * @param  pCtx  The game's context
 * @return       GFMRV_OK, GFMRV_ARGUMENTS_BAD, ...
 */
gfmRV gfmTilemap_draw(gfmTilemap *pTMap, gfmCtx *pCtx);

#endif /* __GFMTILEMAP_H__ */

