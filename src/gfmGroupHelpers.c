/**
 * Defines a few structures and helper functions to keep gfmGroup clearner
 * (albeit a little less easy to read)
 */
#include <GFraMe/gfmAssert.h>
#include <GFraMe/gfmError.h>
#include <GFraMe/gfmSprite.h>
#include <GFraMe_int/gfmGroupHelpers.h>

#include <stdlib.h>
#include <string.h>

/**
 * Alloc a new node
 * 
 * @param  ppCtx The new node
 * @return       GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFM_ALLOC_FAILED
 */
gfmRV gfmGroupNode_getNew(gfmGroupNode **ppCtx) {
    gfmRV rv;
    
    // Sanitize arguments
    ASSERT(ppCtx, GFMRV_ARGUMENTS_BAD);
    ASSERT(!(*ppCtx), GFMRV_ARGUMENTS_BAD);
    
    // Alloc the node
    *ppCtx = (gfmGroupNode*)malloc(sizeof(gfmGroupNode));
    ASSERT(*ppCtx, GFMRV_ALLOC_FAILED);
    // Initialize it
    memset(*ppCtx, 0x00, sizeof(gfmGroupNode));
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Release a node and its sprite, if it's to be automatically managed
 * 
 * @param  ppCtx The node
 * @return       GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmGroupNode_free(gfmGroupNode **ppCtx) {
    gfmRV rv;
    
    // Sanitize arguments
    ASSERT(ppCtx, GFMRV_ARGUMENTS_BAD);
    ASSERT(*ppCtx, GFMRV_ARGUMENTS_BAD);
    
    // Release the sprite, if necessary
    if ((*ppCtx)->autoFree) {
        rv = gfmSprite_free(&((*ppCtx)->pSelf));
        ASSERT_NR(rv == GFMRV_OK);
    }
    // Release the node itself
    free(*ppCtx);
    *ppCtx = 0;
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

