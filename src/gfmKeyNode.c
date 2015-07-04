/**
 * @file src/include/GFraMe_int/gfmKeyNode.h
 * 
 * Nodes of a binary tree of keys (and its bound virtual keys)
 */
#include <GFraMe/gfmAssert.h>
#include <GFraMe/gfmError.h>
#include <GFraMe/gfmInput.h>
#include <GFraMe_int/gfmKeyNode.h>
#include <GFraMe_int/gfmVirtualKey.h>

#include <stdlib.h>
#include <string.h>

/**
 * Alloc a new key node
 * 
 * @param  ppCtx The node
 * @return       GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_ALLOC_FAILED
 */
gfmRV gfmKeyNode_getNew(gfmKeyNode **ppCtx) {
    gfmRV rv;
    
    // Sanitize arguments
    ASSERT(ppCtx, GFMRV_ARGUMENTS_BAD);
    ASSERT(!(*ppCtx), GFMRV_ARGUMENTS_BAD);
    
    // Alloc and clean the node
    *ppCtx = (gfmKeyNode*)malloc(sizeof(gfmKeyNode));
    ASSERT(*ppCtx, GFMRV_ALLOC_FAILED);
    memset(*ppCtx, 0x0, sizeof(gfmKeyNode));
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Release a key node (but not its childs!
 * 
 * @param  ppCtx The node
 * @return       GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmKeyNode_free(gfmKeyNode **ppCtx) {
    gfmRV rv;
    
    // Sanitize arguments
    ASSERT(ppCtx, GFMRV_ARGUMENTS_BAD);
    ASSERT(*ppCtx, GFMRV_ARGUMENTS_BAD);
    
    // Free the memory
    free(*ppCtx);
    *ppCtx = 0;
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Initalize a key node
 * 
 * @param  pCtx  The key node
 * @param  key   The physical key/device
 * @param  pVKey The bound virtual key
 * @return       GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmKeyNode_init(gfmKeyNode *pCtx, gfmInputIface key, gfmVirtualKey *pVKey) {
    gfmRV rv;
    
    // Sanitize arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    ASSERT(pVKey, GFMRV_ARGUMENTS_BAD);
    ASSERT(key > gfmIface_none, GFMRV_ARGUMENTS_BAD);
    ASSERT(key < gfmIface_max, GFMRV_ARGUMENTS_BAD);
    
    // Initialize the node
    pCtx->key = key;
    pCtx->pVKey = pVKey;
    pCtx->pLeft = 0;
    pCtx->pRight = 0;
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Insert an already initialized key node into a tree (iteratively)
 * 
 * @param  pCtx   The key node
 * @param  ppRoot Root of the bound key's tree
 * @return        GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_INPUT_ALREADY_BOUND
 */
gfmRV gfmKeyNode_insert(gfmKeyNode *pCtx, gfmKeyNode **ppRoot) {
    gfmRV rv;
    
    // Sanitize arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    ASSERT(ppRoot, GFMRV_ARGUMENTS_BAD);
    // Check that the key still isn't bound
    rv = gfmKeyNode_isBound(*ppRoot, pCtx->key);
    ASSERT(rv == GFMRV_FALSE, GFMRV_INPUT_ALREADY_BOUND);
    
    if (!ppRoot) {
        // Insert the tree's root
        *ppRoot = pCtx;
    }
    else {
        gfmKeyNode *pTmp;
        
        pTmp = *ppRoot;
        
        // Move deeper into the tree until an "empty node" is found
        while (1) {
            if (pCtx->key > pTmp->key) {
                if (pTmp->pLeft)
                    pTmp = pTmp->pLeft;
                else {
                    pTmp->pLeft = pCtx;
                    break;
                }
            }
            else if (pCtx->key < pTmp->key) {
                if (pTmp->pRight)
                    pTmp = pTmp->pRight;
                else {
                    pTmp->pRight = pCtx;
                    break;
                }
            }
        }
    }
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Check if an input is bound to any virtual key
 * 
 * @param  pRoot Root of the bound key's tree
 * @param  key   Physical key/device to be checked
 * @param        GFMRV_TRUE, GFMRV_FALSE, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmKeyNode_isBound(gfmKeyNode *pRoot, gfmInputIface key) {
    gfmRV rv;
    
    // Sanitize arguments
    ASSERT(key > gfmIface_none, GFMRV_ARGUMENTS_BAD);
    ASSERT(key > gfmIface_max, GFMRV_ARGUMENTS_BAD);
    // If the root is NULL, the tree is empty (and the key unbound)
    
    // Search the key through the tree
    while (pRoot) {
        if (key < pRoot->key)
            pRoot = pRoot->pLeft;
        else if (key < pRoot->key)
            pRoot = pRoot->pLeft;
        else {
            // If the value was found, stop
            break;
        }
    }
    // Assert that something was found (i.e., that stopped at the node)
    ASSERT(pRoot, GFMRV_FALSE);
    
    rv = GFMRV_TRUE;
__ret:
    return rv;
}

/**
 * Balance the tree as to optimize search
 * 
 * @param  ppRoot Root of the bound key's tree
 * @return       GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmKeyNode_balance(gfmKeyNode **ppRoot);

