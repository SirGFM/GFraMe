/**
 * @file src/util/gfmTileAnimation.c
 * 
 * Wrapper for gfmTileAnimation and gfmTileAnimationInfo
 */
#include <GFraMe/gfmAssert.h>
#include <GFraMe/gfmError.h>
#include <GFraMe_int/gfmTileAnimation.h>

#include <stdlib.h>
#include <string.h>

/**
 * Alloc a new gfmTileAnimation
 * 
 * @param  **ppCtx The allocated context
 * @return         GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_ALLOC_FAILED
 */
gfmRV gfmTileAnimation_getNew(gfmTileAnimation **ppCtx) {
    gfmRV rv;
    
    // Sanitize arguments
    ASSERT(ppCtx, GFMRV_ARGUMENTS_BAD);
    ASSERT(!(*ppCtx), GFMRV_ARGUMENTS_BAD);
    
    // Alloc the tilemap and clean it
    *ppCtx = (gfmTileAnimation*)malloc(sizeof(gfmTileAnimation));
    ASSERT(*ppCtx, GFMRV_ALLOC_FAILED);
    memset(*ppCtx, 0x0, sizeof(gfmTileAnimation));
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Free a gfmTileAnimation
 * 
 * @param  **ppCtx The context
 * @return         GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmTileAnimation_free(gfmTileAnimation **ppCtx) {
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

/**
 * Alloc a new gfmTileAnimationInfo
 * 
 * @param  **ppCtx The allocated context
 * @return         GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_ALLOC_FAILED
 */
gfmRV gfmTileAnimationInfo_getNew(gfmTileAnimationInfo **ppCtx) {
    gfmRV rv;
    
    // Sanitize arguments
    ASSERT(ppCtx, GFMRV_ARGUMENTS_BAD);
    ASSERT(!(*ppCtx), GFMRV_ARGUMENTS_BAD);
    
    // Alloc the tilemap and clean it
    *ppCtx = (gfmTileAnimationInfo*)malloc(sizeof(gfmTileAnimationInfo));
    ASSERT(*ppCtx, GFMRV_ALLOC_FAILED);
    memset(*ppCtx, 0x0, sizeof(gfmTileAnimationInfo));
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Free a gfmTileAnimationInfo
 * 
 * @param  **ppCtx The context
 * @return         GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmTileAnimationInfo_free(gfmTileAnimationInfo **ppCtx) {
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

