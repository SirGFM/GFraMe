/**
 * @file src/include/GFraMe_int/gfmVirtualKey.h
 * 
 * Virtual key used by gfmInput
 */
#include <GFraMe/gfmAssert.h>
#include <GFraMe/gfmError.h>
#include <GFraMe/gfmInput.h>
#include <GFraMe_int/gfmVirtualKey.h>

#include <stdlib.h>
#include <string.h>

/**
 * Alloc a new virtual key
 * 
 * @param  ppCtx The virtual key
 * @return       GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_ALLOC_FAILED
 */
gfmRV gfmVirtualKey_getNew(gfmVirtualKey **ppCtx) {
    gfmRV rv;
    
    // Sanitize arguments
    ASSERT(ppCtx, GFMRV_ARGUMENTS_BAD);
    ASSERT(!(*ppCtx), GFMRV_ARGUMENTS_BAD);
    
    // Alloc and zero the key
    *ppCtx = (gfmVirtualKey*)malloc(sizeof(gfmVirtualKey));
    ASSERT(*ppCtx, GFMRV_ALLOC_FAILED);
    memset(*ppCtx, 0x0, sizeof(gfmVirtualKey));
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Releases a virtual key
 * 
 * @param  ppCtx The virtual key
 * @return       GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmVirtualKey_free(gfmVirtualKey **ppCtx) {
    gfmRV rv;
    
    // Sanitize arguments
    ASSERT(ppCtx, GFMRV_ARGUMENTS_BAD);
    ASSERT(*ppCtx, GFMRV_ARGUMENTS_BAD);
    
    // Free the virtual key
    free(*ppCtx);
    *ppCtx = 0;
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Initialize a virtual key (with all zeros)
 * 
 * @param  ppCtx The virtual key
 * @return       GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmVirtualKey_init(gfmVirtualKey *pCtx) {
    gfmRV rv;
    
    // Sanitize arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    
    memset(pCtx, 0x0, sizeof(gfmVirtualKey));
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

