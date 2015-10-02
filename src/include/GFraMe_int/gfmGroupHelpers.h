/**
 * Defines a few structures and helper functions to keep gfmGroup clearner
 * (albeit a little less easy to read)
 */
#ifndef __GFMGROUPHELPERS_STRUCT__
#define __GFMGROUPHELPERS_STRUCT__

/** 'Exports' the gfmDrawTree structure */
typedef struct stGFMDrawTree gfmDrawTree;

/** The gfmGroupNode structure */
struct stGFMGroupNode {
    /** Next pointer on the list */
    struct stGFMGroupNode *pNext;
    /** Next visible node on the list */
    struct stGFMGroupNode *pNextVisible;
    /* Actualy pointer to the object */
    gfmSprite *pSelf;
    /** For how long this node has been on the alive list */
    int timeAlive;
    /** Whether this reference should be automatically freed or not */
    int autoFree;
};

/** The gfmDrawTree structure */
struct stGFMDrawTree {
    /** Left node (less or equal some value) */
    gfmDrawTree *pLeft;
    /** Right node (greater than some value) */
    gfmDrawTree *pRight;
    /** Current node */
    struct stGFMGroupNode *pSelf;
};

#endif /* __GFMGROUPHELPERS_STRUCT__ */

#ifndef __GFMGROUPHELPERS_H__
#define __GFMGROUPHELPERS_H__

#include <GFraMe/gfmError.h>
#include <GFraMe/gfmGroup.h>

/**
 * Alloc a new node
 * 
 * @param  ppCtx The new node
 * @return       GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFM_ALLOC_FAILED
 */
gfmRV gfmGroupNode_getNew(gfmGroupNode **ppCtx);

/**
 * Release a node and its sprite, if it's to be automatically managed
 * 
 * @param  ppCtx The node
 * @return       GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmGroupNode_free(gfmGroupNode **pCtx);

/**
 * Alloc a new tree node
 * 
 * @param  ppCtx The new tree node
 * @return       GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFM_ALLOC_FAILED
 */
gfmRV gfmDrawTree_getNew(gfmDrawTree **ppCtx);

/**
 * Release a tree node
 * 
 * @param  ppCtx The tree node
 * @return       GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmDrawTree_free(gfmDrawTree **ppCtx);

#endif /* __GFMGROUPHELPERS_H__ */

