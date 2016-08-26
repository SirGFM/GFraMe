/**
 * Defines a few structures and helper functions to keep gfmGroup clearner
 * (albeit a little less easy to read)
 */
#ifndef __GFMGROUPHELPERS_STRUCT__
#define __GFMGROUPHELPERS_STRUCT__

/** 'Exports' the gfmDrawTree structure */
typedef struct stGFMDrawTree gfmDrawTree;

enum {
    gfmGroup_keepAlive = -4321,
    gfmGroup_forceKill = -4322
};

typedef struct stGFMGroupDrawNode gfmGroupDrawNode;

/** All possible values used by a DrawNode */
union unGFMGroupRenderUnion {
    /** Vertical position (used when sorting by vertical position) */
    int y;
    /** Time alive (used when sorting by 'age') */
    int timeAlive;
    /** Next DrawNode on the stack (i.e., one that has to be rendered and
     * traversed right) */
    gfmGroupDrawNode *pStackNext;
};
typedef union unGFMGroupRenderUnion gfmGroupRenderUnion;

/** Helper struct used to sort the group */
struct stGFMGroupDrawNode {
    /** All possible values used by a DrawNode */
    gfmGroupRenderUnion data;
    /** Nodes to the left of this one (i.e., will be rendered first) */
    gfmGroupDrawNode *pLeft;
    /** Nodes to the right of this one (i.e., will be rendered afterward) */
    gfmGroupDrawNode *pRight;
    /** Nodes actual sprite */
    gfmSprite *pSelf;
};

/** The gfmGroupNode structure */
struct stGFMGroupNode {
    /** Next pointer on the list */
    struct stGFMGroupNode *pNext;
    /** Next collideable node on the list */
    struct stGFMGroupNode *pNextCollideable;
    /** Actualy pointer to the object */
    gfmSprite *pSelf;
    /** For how long this node may keep living; If set to gfmGroup_keepAlive,
        the node won't be removed ever */
    int timeAlive;
    /** Whether this reference should be automatically freed or not */
    int autoFree;
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

#endif /* __GFMGROUPHELPERS_H__ */

