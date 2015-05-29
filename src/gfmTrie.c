/**
 * @file src/gfmTrie.c
 * 
 * Trie implementation; Used by the GIF exporter to store the LZW dictionary
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
gfmRV gfmTrie_getNew(gfmTrie **ppCtx);

/**
 * Free a single node; Useful if recycling nodes!
 * 
 * @param  ppCtx The node to be freed
 * @param        GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmTrie_free(gfmTrie **ppCtx);

/**
 * Free all child and sibling nodes recursively
 * 
 * @param  ppCtx The first node to be freed
 * @param        GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmTrie_freeAll(gfmTrie **ppCtx);

/**
 * Insert a node as a child to the current one
 * 
 * @param  pCtx   The current node
 * @param  pChild The node that will be added
 * @param  key    The node's key
 * @param  value  The node's value
 * @return        GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmTrie_insertChild(gfmTrie *pCtx, gfmTrie *pChild, unsigned char key, int value);

/**
 * Insert a node as a sibling to the current one
 * 
 * @param  pCtx     The current node
 * @param  pSibling The node that will be added
 * @param  key      The node's key
 * @param  value    The node's value
 * @return          GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmTrie_insertSibling(gfmTrie *pCtx, gfmTrie *pSibling, unsigned char key, int value);

/**
 * Search for a given key through this node's siblings
 * 
 * @param  ppSibling The sibling with the desired key
 * @param  pCtx      The current node
 * @param  key       The desired key
 * @return           GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_TRIE_KEY_NOT_FOUND
 */
gfmRV gfmTrie_searchSiblings(gfmTrie **ppSibling, gfmTrie *pCtx,
        unsigned char key);

/**
 * Get this node's child
 * 
 * @param  ppChild The node's child
 * @param  pCtx    The current node
 * @return        GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_TRIE_IS_LEAF
 */
gfmRV gfmTrie_getChild(gfmTrie **ppChild, gfmTrie *pCtx);

/**
 * Read a node's value
 * 
 * @param  pValue The node's value
 * @param  pCtx   The current node
 * @return        GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmTrie_getValue(int *pValue, gfmTrie *pCtx);

