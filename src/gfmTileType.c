/**
 * @file src/include/GFraMe_int/gfmTileType.h
 * 
 * Simple module to be used by the tilemap; It's separated to keep that file
 * cleaner
 */
#include <GFraMe/gfmAssert.h>
#include <GFraMe/gfmError.h>
#include <GFraMe_int/gfmTileType.h>

#include <stdlib.h>
#include <string.h>

/**
 * Alloc a new gfmTileType
 * 
 * @param  **ppCtx The allocated context
 * @return         GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_ALLOC_FAILED
 */
gfmRV gfmTileType_getNew(gfmTileType **ppCtx) {
    gfmRV rv;
    
    // Sanitize arguments
    ASSERT(ppCtx, GFMRV_ARGUMENTS_BAD);
    ASSERT(!(*ppCtx), GFMRV_ARGUMENTS_BAD);
    
    // Alloc the tilemap and clean it
    *ppCtx = (gfmTileType*)malloc(sizeof(gfmTileType));
    ASSERT(*ppCtx, GFMRV_ALLOC_FAILED);
    memset(*ppCtx, 0x0, sizeof(gfmTileType));
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Free a gfmTileType
 * 
 * @param  **ppCtx The context
 * @return         GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmTileType_free(gfmTileType **ppCtx) {
    gfmRV rv;
    
    // Sanitize arguments
    ASSERT(ppCtx, GFMRV_ARGUMENTS_BAD);
    ASSERT(*ppCtx, GFMRV_ARGUMENTS_BAD);
    
    // Free it up
    free(*ppCtx);
    *ppCtx = 0;
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

