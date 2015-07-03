/**
 * @file src/include/GFraMe_int/gfmKeyNode.h
 * 
 * Nodes of a binary tree of keys (and its bound virtual keys)
 */
#ifndef __GFMKEYNODE_STRUCT_H__
#define __GFMKEYNODE_STRUCT_H__

#include <GFraMe/gfmInput.h>
#include <GFraMe_int/gfmVirtualKey.h>

/** Create the nodes of keys to be used within a tree */
struct enGFMKeyNode {
    /** The key bound to the virtual key */
    gfmInputIface key;
    /** Virtual key that this is bound to */
    gfmVirtualKey *pVKey;
    /** Child with a key value of less than this */
    struct enGFMKeyNode *pLeft;
    /** Child with a key value of more than this */
    struct enGFMKeyNode *pRight;
};
typedef struct enGFMKeyNode gfmKeyNode;

#endif /* __GFMKEYNODE_STRUCT_H__ */

#ifndef __GFMKEYNODE_H__
#define __GFMKEYNODE_H__

#include <GFraMe/gfmError.h>

/**
 * Alloc a new key node
 * 
 * @param  ppCtx The node
 * @return       GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_ALLOC_FAILED
 */
gfmRV gfmKeyNode_getNew(gfmKeyNode **ppCtx);

/**
 * Release a key node (but not its childs!
 * 
 * @param  ppCtx The node
 * @return       GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmKeyNode_free(gfmKeyNode **ppCtx);

/**
 * Initalize a key node
 * 
 * @param  pCtx  The key node
 * @param  key   The physical key/device
 * @param  pVKey The bound virtual key
 * @return       GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmKeyNode_init(gfmKeyNode *pCtx, gfmInputIface key, gfmVirtualKey *pVKey);

/**
 * Insert an already initialized key node into a tree (iteratively)
 * 
 * @param  pCtx   The key node
 * @param  ppRoot Root of the bound key's tree
 * @return        GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_INPUT_ALREADY_BOUND
 */
gfmRV gfmKeyNode_insert(gfmKeyNode *pCtx, gfmKeyNode **ppRoot);

/**
 * Check if an input is bound to any virtual key
 * 
 * @param  pRoot Root of the bound key's tree
 * @param  key   Physical key/device to be checked
 * @param        GFMRV_TRUE, GFMRV_FALSE, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmKeyNode_isBound(gfmKeyNode *pRoot, gfmInputIface key);

/**
 * Balance the tree as to optimize search
 * 
 * @param  ppRoot Root of the bound key's tree
 * @return       GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmKeyNode_balance(gfmKeyNode **ppRoot);

#endif /* __GFMKEYNODE_H__*/ 

