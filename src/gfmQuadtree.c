/**
 * @file include/GFraMe/gfmQuadtree.h
 * 
 * Interactive quadtree implementation to be used for collision detection; After
 * initializing the root node (through a gfmQuadtree_initRoot call), there are
 * two ways to add more nodes: using gfmQuadtree_collide* and
 * gfmQuadtree_populate*;
 *   - The functions gfmQuadtree_populate* will adds a new 'object' (in the
 *   broader sense) to the quadtree, but doesn't collide anything;
 *   - gfmQuadtree_collide*, on the other hand, will adds a new 'object' to the
 *   quadtree and will collide it against every other object in it;
 * Therefore, the proper way to use this would be to create the root node,
 * populate it with all the static 'objects' (e.g., tilemap, particles, etc) and
 * then add all the collideable 'objects';
 * On success, gfmQuadtree_collide and gfmQuadtree_continue  can return in one
 * of a two ways:
 *   - GFMRV_QUADTREE_OVERLAPED: signs that an overlap just happened and should
 *   be handled; This returns prior to completely adding the 'object' to the
 *   tree, so gfmQuadtree_continue *must* be called afterward! Calling 
 *   gfmQuadtree_getOverlaping will return both gfmObjects that overlaped; From
 *   those, one could call gfmObject_getChild to get the child 'object' (if any)
 *   and its type; If this child is a gfmSprite (that was correctly setuped),
 *   calling gfmSprite_getChild would return its 'sub-class'; This was a way to
 *   emulate (in a quite simple way) OOP in C;
 *   - GFMRV_QUADTREE_DONE: the 'object' was successfully added to the quadtree;
 */
#include <GFraMe/gfmAssert.h>
#include <GFraMe/gfmError.h>
#include <GFraMe/gfmGenericArray.h>
#include <GFraMe/gfmGroup.h>
#include <GFraMe/gfmObject.h>
#include <GFraMe/gfmQuadtree.h>
#include <GFraMe/gfmSprite.h>
#include <GFraMe/gfmTilemap.h>

/** Quadtree node type */
typedef struct stGFMQuadtree gfmQuadtree;
/** Quadtree nodes linked-list */
typedef struct stGFMQuadtreeLL gfmQuadtreeLL;
/** Stack of quadtree nodes */
typedef struct stGFMQuadtreeStack gfmQuadtreeStack;
/** Index of a child relative to its parent */
typedef enum enGFMQuadtreePosition gfmQuadtreePosition;
/** Define array of quadtree nodes */
gfmGenArr_define(gfmQuadtree);
/** Define array of quadtree linked-list nodes */
gfmGenArr_define(gfmQuadtreeLL);

/** Index of a child relative to its parent */
typedef enum enGFMQuadtreePosition gfmQuadtreePosition;
enum enGFMQuadtreePosition {
    gfmQT_nw = 0,
    gfmQT_ne,
    gfmQT_sw,
    gfmQT_se,
    gfmQT_max,
};

/** Index of a child relative to its parent */
struct stGFMQuadtreeLL {
    /** This nodes object */
    gfmObject *pSelf;
    /** The next node */
    struct stGFMQuadtreeLL *pNext;
};

/** Stack of quadtree nodes */
struct stGFMQuadtreeStack {
    /** The nodes that were pushed */
    gfmQuadtree **ppStack;
    /** Position where pushes may happen (pop happens on this - 1) */
    int pushPos;
    /** How many nodes this can handle */
    int len;
};

/** Quadtree node type */
struct stGFMQuadtree {
    /** The tree's sub-nodes */
    gfmQuadtree *children;
    /** List of all objects inside this node */
    gfmQuadtreeLL *pNodes;
    /** Center of the hitbox */
    int centerX;
    /** Center of the hitbox */
    int centerY;
    /** Half the hitbox's width */
    int halfWidth;
    /** Half the hitbox's height */
    int hafHeight;
    /** The node's depth */
    int depth;
    /** How many objects were added to this node */
    int numObjects;
};

/** Quadtree's context, with the current stack and the the root node */
struct stGFMQuadtreeRoot {
    /** How many nodes can a node have until it must subdivide */
    int maxNodes;
    /** How many depths the quadtree can have */
    int maxDepth;
    /** Pool of quadtree nodes */
    gfmGenArr_var(gfmQuadtree, pQTPool);
    /** Pool of quadtree LL nodes */
    gfmGenArr_var(gfmQuadtreeLL, pQTLLPool);
    /** The actual root node of the quadtree */
    gfmQuadtree *self;
    /** List of available LL nodes */
    gfmQuadtreeLL *pAvailable;
    /** Stack of nodes which the object must still be added to */
    gfmQuadtreeStack stack;
    /** Object being collided */
    gfmObject *pObject;
    /** One of the objects that just overlaped */
    gfmObject *pObj1;
    /** The other of the objects that just overlaped */
    gfmObject *pObj2;
    /** TODO Members for the interactive addition */
};

gfmRV gfmQuadtree_getNew(gfmQuadtreeRoot **ppCtx);
gfmRV gfmQuadtree_free(gfmQuadtreeRoot **ppCtx);
gfmRV gfmQuadtree_clean(gfmQuadtreeRoot *pCtx);
gfmRV gfmQuadtree_initRoot(gfmQuadtreeRoot *pCtx, int x, int y, int width,
        int height, int maxDepths, int maxNodes);
static gfmRV gfmQuadtree_init(gfmQuadtree *pCtx, gfmQuadtree *pParent,
        gfmQuadtreePos pos);

/**
 * Push a node into the stack, so it'll be checked later
 * 
 * @param  pCtx  The quadtree's root/ctx
 * @param  pNode The node to be pushed
 * @return       GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_QUADTREE_NOT_INITIALIZED,
 *               GFMRV_QUADTREE_STACK_OVERFLOW
 */
static gfmRV gfmQuadtree_pushNode(gfmQuadtreeRoot *pCtx, gfmQuadtree *pNode) {
    gfmRV rv;
    
    // Sanitize arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    ASSERT(pNode, GFMRV_ARGUMENTS_BAD);
    // Check if initialized
    ASSERT(pCtx->maxDepth > 0, GFMRV_QUADTREE_NOT_INITIALIZED);
    // Check that pushed can still be done
    ASSERT(pCtx->stack.pushPos < pCtx->stack.len,
            GFMRV_QUADTREE_STACK_OVERFLOW);
    
    // Push the node
    pCtx->stack.ppStack[pCtx->stack.pushPos] = pNode;
    pCtx->stack.pushPos++;
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Pop a node from the quadtree's context
 * 
 * @param  ppNode The popped node
 * @param  pCtx   The quadtree's root/ctx
 * @return        GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_QUADTREE_NOT_INITIALIZED,
 *                GFMRV_QUADTREE_EMPTY
 */
static gfmRV gfmQuadtree_popNode(gfmQuadtree **ppNode, gfmQuadtreeRoot *pCtx) {
    gfmRV rv;
    
    // Sanitize arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    ASSERT(ppNode, GFMRV_ARGUMENTS_BAD);
    // Check if initialized
    ASSERT(pCtx->maxDepth > 0, GFMRV_QUADTREE_NOT_INITIALIZED);
    // Check if there are any nodes to be popped
    ASSERT(pCtx->stack.pushPos > 0, GFMRV_QUADTREE_EMPTY);
    
    // Pop the node
    pCtx->stack.pushPos--;
    *ppNode = pCtx->stack.ppStack[pCtx->stack.pushPos];
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Subdivides a quadtree
 * 
 * @param  pCtx The node to be subdivided
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_ALLOC_FAILED
 */
static gfmRV gfmQuadtree_subdivied(gfmQuadtreeRoot *pCtx, gfmQuadtree *pNode) {
    gfmRV rv;
    
    // Sanitize arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    ASSERT(pNode, GFMRV_ARGUMENTS_BAD);
    
    // TODO Alloc all the children
    // TODO Initialize every child
    
    // Insert every child to the tree it's contained
    while (pNode->pNodes) {
        gfmQuadtreeLL *pTmp;
        
        // Get the current node
        pTmp = pNode->pNodes;
        
        // TODO Add it to every child (that it overlaps)
        
        // Go to the next node
        pNode->pNodes = pTmp->next;
        // Prepend the node to the free list
        pTmp->next = pCtx->pAvailable;
        pCtx->pAvailable = pTmp;
    }
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Prepare the 'stack' and initialize the collisiong for a new node
 * 
 * @param  pCtx The quadtree's root
 * @param  pPtr Pointer to whatever is being added
 * @param  type Type of whatever is being added
 * @return      GFMRV_ARGUMENTS_BAD, GFMRV_QUADTREE_NOT_INITIALIZED, 
 *              GFMRV_QUADTREE_OVERLAPED, GFMRV_QUADTREE_DONE
 */
static gfmRV gfmQuadtree_collide(gfmQuadtreeRoot *pCtx, void *pPtr, int type) {
    gfmRV rv;
    
    // Sanitize arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    ASSERT(pPtr, GFMRV_ARGUMENTS_BAD);
    // Check if initialized
    ASSERT(pCtx->maxDepth > 0, GFMRV_QUADTREE_NOT_INITIALIZED);
    // TODO Check if this node overlaps the root
    
    // Set the initial context (to be able to call 'continue', later)
    pCtx->curDepth = 0;
    pCtx->curNode = 0;
    pCtx->pQtStack[0] = &(pCtx->self);
    pCtx->pQtPosStack[0] = gfmQT_nw;
    pCtx->type = type;
    pCtx->pObject = pPtr;
    
    // Collide it
    rv = gfmQuadtree_continue(pCtx);
__ret:
    return rv;
}

gfmRV gfmQuadtree_collideGroup(gfmQuadtreeRoot *pCtx, gfmGroup *pGrp);

/**
 * Adds a new gfmObject to the quadtree, subdividing it as necessary and
 * colliding with every possible node
 * 
 * @param  pCtx The quadtree's root
 * @param  pObj The gfmObject
 * @return      GFMRV_ARGUMENTS_BAD, GFMRV_QUADTREE_NOT_INITIALIZED, 
 *              GFMRV_QUADTREE_OVERLAPED, GFMRV_QUADTREE_DONE
 */
gfmRV gfmQuadtree_collideObject(gfmQuadtreeRoot *pCtx, gfmObject *pObj) {
    gfmRV rv;
    
    rv = gfmQuadtree_collide(pCtx, (void*)pObj, gfmType_object);
    
    return rv;
}

/**
 * Adds a new gfmSprite to the quadtree, subdividing it as necessary and
 * colliding with every possible node
 * 
 * @param  pCtx The quadtree's root
 * @param  pObj The gfmObject
 * @return      GFMRV_ARGUMENTS_BAD, GFMRV_QUADTREE_NOT_INITIALIZED, 
 *              GFMRV_QUADTREE_OVERLAPED, GFMRV_QUADTREE_DONE
 */
gfmRV gfmQuadtree_collideSprite(gfmQuadtreeRoot *pCtx, gfmSprite *pSpr) {
    gfmObject *pObj;
    gfmRV rv;
    
    // Sanitize sprite (other checks are done in sub-functions)
    ASSERT(pSpr, GFMRV_ARGUMENTS_BAD);
    // Retrieve the sprite's object
    rv = gfmSprite_getObject(&pObj, pSpr);
    ASSERT_NR(rv == GFMRV_OK);
    // Add it to the quadtree
    rv = gfmQuadtree_collide(pCtx, (void*)pObj, gfmType_object);
__ret:
    return rv;
}

gfmRV gfmQuadtree_collideTilemap(gfmQuadtreeRoot *pCtx, gfmTilemap *pTMap);
gfmRV gfmQuadtree_populateGroup(gfmQuadtreeRoot *pCtx, gfmGroup *pGrp);
gfmRV gfmQuadtree_populateObject(gfmQuadtreeRoot *pCtx, gfmObject *pObj);
gfmRV gfmQuadtree_populateSprite(gfmQuadtreeRoot *pCtx, gfmSprite *pSpr);
gfmRV gfmQuadtree_populateTilemap(gfmQuadtreeRoot *pCtx, gfmTilemap *pTMap);
gfmRV gfmQuadtree_getOverlaping(gfmObjectRoot **ppObj1, gfmObject **ppObj2,
        gfmQuadtree *pCtx);

/**
 * Adds a node to the quadtree
 * 
 * @param  pCtx The quadtree's root
 * @return      GFMRV_ARGUMENTS_BAD, GFMRV_QUADTREE_OPERATION_NOT_ACTIVE,
 *              GFMRV_QUADTREE_OVERLAPED, GFMRV_QUADTREE_DONE
 */
gfmRV gfmQuadtree_continue(gfmQuadtreeRoot *pCtx) {
    gfmRV rv;
    
    // Sanitize arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    // Check that the operation is active
    ASSERT(pCtx->pObject, GFMRV_QUADTREE_OPERATION_NOT_ACTIVE);
    
    // TODO Continue adding the object
    while (1) {
        gfmQuadtree *pNode;
        
        // Pop the current node
        rv = gfmQuadtree_popNode(&pNode, pCtx);
        ASSERT_NR(rv == GFMRV_OK);
        // TODO If it has children, push its children
        rv = gfmQuadtree_pushNode(pCtx, gfmQuadtree *pNode);
        // TODO Otherwise, collide with its nodes
    }
    
    rv = GFMRV_QUADTREE_DONE;
__ret:
    return rv;
}

gfmRV gfmQuadtree_drawBounds(gfmQuadtreeRoot *pQt, gfmCtx *pCtx);

