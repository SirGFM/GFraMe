/**
 * @file src/util/gfmTrie.c
 * 
 * Trie implementation; Used by the GIF exporter to store the LZW dictionary;
 * Only insertion and search are implemented, as those are the only functions
 * required by the GIF exporter
 */
#include <GFraMe/gfmAssert.h>
#include <GFraMe/gfmError.h>
#include <GFraMe_int/gfmTrie.h>

#include <stdlib.h>
#include <string.h>

/** The gfmTrie structure (a node) */
struct stGFMTrie {
    /** The node's key */
    unsigned char key;
    /** The node's value */
    int value;
    /** The node's child */
    gfmTrie *pChild;
    /** The node's next sibling */
    gfmTrie *pSibling;
};

/**
 * Alloc a new gfmTrie node
 * 
 * @param  ppCtx The new node
 * @return       GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_ALLOC_FAILED
 */
gfmRV gfmTrie_getNew(gfmTrie **ppCtx) {
    gfmRV rv;
    
    // Sanitize arguments
    ASSERT(ppCtx, GFMRV_ARGUMENTS_BAD);
    ASSERT(!(*ppCtx), GFMRV_ARGUMENTS_BAD);
    
    // Alloc the object
    *ppCtx = (gfmTrie*)malloc(sizeof(gfmTrie));
    ASSERT(*ppCtx, GFMRV_ALLOC_FAILED);
    
    // Clean the object
    memset(*ppCtx, 0x0, sizeof(gfmTrie));
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Free a single node; Useful if recycling nodes!
 * 
 * @param  ppCtx The node to be freed
 * @param        GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmTrie_free(gfmTrie **ppCtx) {
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
 * Free all child and sibling nodes recursively; It must be called on the root
 * node and all nodes must have been previously initialized (otherwise, FUN
 * stuff will happen)
 * 
 * @param  ppCtx The first node to be freed
 * @param        GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmTrie_freeAll(gfmTrie **ppCtx) {
    gfmRV rv;
    
    // Sanitize arguments
    ASSERT(ppCtx, GFMRV_ARGUMENTS_BAD);
    ASSERT(*ppCtx, GFMRV_ARGUMENTS_BAD);
    
    // If there are any children
    gfmTrie_freeAll(&((*ppCtx)->pChild));
    // If there are any siblings
    gfmTrie_freeAll(&((*ppCtx)->pSibling));
    
    // Free the object itself
    rv = gfmTrie_free(ppCtx);
    ASSERT_NR(rv == GFMRV_OK);
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Clean up everything
 * 
 * @param  pCtx  The current node
 * @param  key   The node's key
 * @param  value The node's value
 * @param        GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmTrie_init(gfmTrie *pCtx, unsigned char key, int value) {
    gfmRV rv;
    
    // Sanitize arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    
    // Initialize the context
    pCtx->pChild = 0;
    pCtx->pSibling = 0;
    pCtx->key = key;
    pCtx->value = value;
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Insert a node as a child to the current one
 * 
 * @param  pCtx   The current node
 * @param  pChild The node that will be added
 * @param  key    The node's key
 * @param  value  The node's value
 * @return        GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmTrie_insertChild(gfmTrie *pCtx, gfmTrie *pChild, unsigned char key, int value) {
    gfmRV rv;
    
    // Sanitize arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    ASSERT(pChild, GFMRV_ARGUMENTS_BAD);
    
    // Insert the node as a child
    pCtx->pChild = pChild;
    // Initialize this child
    pChild->pChild = 0;
    pChild->pSibling = 0;
    pChild->key = key;
    pChild->value = value;
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Insert a node as a sibling to the current one
 * 
 * @param  pCtx     The current node
 * @param  pSibling The node that will be added
 * @param  key      The node's key
 * @param  value    The node's value
 * @return          GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmTrie_insertSibling(gfmTrie *pCtx, gfmTrie *pSibling, unsigned char key, int value) {
    gfmRV rv;
    
    // Sanitize arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    ASSERT(pSibling, GFMRV_ARGUMENTS_BAD);
    
    // Find the last sibling
    while (pCtx->pSibling)
        pCtx = pCtx->pSibling;
    // Insert the node as a sibling
    pCtx->pSibling = pSibling;
    // Initialize this sibling
    pSibling->pChild = 0;
    pSibling->pSibling = 0;
    pSibling->key = key;
    pSibling->value = value;
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Search for a given key through this node's siblings
 * 
 * @param  ppSibling The sibling with the desired key
 * @param  pCtx      The current node
 * @param  key       The desired key
 * @return           GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_TRIE_KEY_NOT_FOUND
 */
gfmRV gfmTrie_searchSiblings(gfmTrie **ppSibling, gfmTrie *pCtx,
        unsigned char key) {
    gfmRV rv;
    
    // Sanitize arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    ASSERT(ppSibling, GFMRV_ARGUMENTS_BAD);
   
    // Iterate through every siblings
    while (pCtx) {
        if (pCtx->key == key)
            break;
        // Go to the next one
        pCtx = pCtx->pSibling;
    }
    ASSERT(pCtx, GFMRV_TRIE_KEY_NOT_FOUND);
    
    // Set the return
    *ppSibling = pCtx;
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Get this node's child
 * 
 * @param  ppChild The node's child
 * @param  pCtx    The current node
 * @return         GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_TRIE_IS_LEAF
 */
gfmRV gfmTrie_getChild(gfmTrie **ppChild, gfmTrie *pCtx) {
    gfmRV rv;
    
    // Sanitize arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    ASSERT(ppChild, GFMRV_ARGUMENTS_BAD);
    // Check that there is a child
    ASSERT(pCtx->pChild, GFMRV_TRIE_IS_LEAF);
    
    // Set the return
    *ppChild = pCtx->pChild;
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Read a node's value
 * 
 * @param  pValue The node's value
 * @param  pCtx   The current node
 * @return        GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmTrie_getValue(int *pValue, gfmTrie *pCtx) {
    gfmRV rv;
    
    // Sanitize arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    ASSERT(pValue, GFMRV_ARGUMENTS_BAD);
    
    // Set the return
    *pValue = pCtx->value;
    rv = GFMRV_OK;
__ret:
    return rv;
}

