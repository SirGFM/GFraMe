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

#include <stdlib.h>
#include <string.h>

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
    gfmQuadtree *ppChildren[gfmQT_max];
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
    gfmQuadtree *pSelf;
    /** List of available LL nodes */
    gfmQuadtreeLL *pAvailable;
    /** List of nodes to be collided */
    gfmQuadtreeLL *pColliding;
    /** Stack of nodes which the object must still be added to */
    gfmQuadtreeStack stack;
    /** Object being collided */
    gfmObject *pObject;
    /** The object that was just overlapped */
    gfmObject *pOther;
};

/**
 * Alloc a new quadtree linked-list node
 * 
 * @param  ppCtx The linked list node
 * @return       GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_ALLOC_FAILED
 */
static gfmRV gfmQuadtreeLL_getNew(gfmQuadtreLL **ppCtx) {
    gfmRV rv;
    
    // Sanitize arguments
    ASSERT(ppCtx, GFMRV_ARGUMENTS_BAD);
    ASSERT(!(*ppCtx), GFMRV_ARGUMENTS_BAD);
    
    // Alloc it
    *ppCtx = (gfmQuadtreLL*)malloc(sizeof(gfmQuadtreLL));
    ASSERT(*ppCtx, GFMRV_ALLOC_FAILED);
    // Clean it
    memset(*ppCtx, 0x0, sizeof(gfmQuadtreLL));
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Alloc a new quadtree structure
 * 
 * @param  ppCtx The quadtree
 * @return       GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_ALLOC_FAILED
 */
static gfmRV gfmQuadtreeNode_getNew(gfmQuadtree **ppCtx) {
    gfmRV rv;
    
    // Sanitize arguments
    ASSERT(ppCtx, GFMRV_ARGUMENTS_BAD);
    ASSERT(!(*ppCtx), GFMRV_ARGUMENTS_BAD);
    
    // Alloc it
    *ppCtx = (gfmQuadtree*)malloc(sizeof(gfmQuadtree));
    ASSERT(*ppCtx, GFMRV_ALLOC_FAILED);
    // Clean it
    memset(*ppCtx, 0x0, sizeof(gfmQuadtree));
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Alloc a new root quadtree
 * 
 * @param ppCtx The root quadtree
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_ALLOC_FAILED
 */
gfmRV gfmQuadtree_getNew(gfmQuadtreeRoot **ppCtx) {
    gfmRV rv;
    
    // Sanitize arguments
    ASSERT(ppCtx, GFMRV_ARGUMENTS_BAD);
    ASSERT(!(*ppCtx), GFMRV_ARGUMENTS_BAD);
    
    // Alloc and clean it
    *ppCtx = (gfmQuadtreeRoot*)malloc(sizeof(gfmQuadtreeRoot));
    ASSERT(*ppCtx, GFMRV_ALLOC_FAILED);
    memset(*ppCtx, 0x0, sizeof(gfmQuadtreeRoot));
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Release a quadtree's root and all its members
 * 
 * @param ppCtx The quadtree root
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmQuadtree_free(gfmQuadtreeRoot **ppCtx) {
    gfmRV rv;
    
    // Sanitize arguments
    ASSERT(ppCtx, GFMRV_ARGUMENTS_BAD);
    ASSERT(*ppCtx, GFMRV_ARGUMENTS_BAD);
    
    // Clean the quadtree
    gfmQuadtree_clean(*ppCtx);
    // Free the struct
    free(*ppCtx);
    *pCtx = 0;
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

gfmRV gfmQuadtree_clean(gfmQuadtreeRoot *pCtx);

/**
 * Clean up the previous state and ready the quadtree for collision
 * 
 * @param  pCtx     The quadtree root context
 * @param  x        The quadtree top-left position
 * @param  y        The quadtree top-left position
 * @param  width    The quadtree width
 * @param  height   The quadtree height
 * @param  maxDepth How many levels can the quadtree branch
 * @param  maxNodes How many objects a subtree can have until it must split
 * @return          GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_ALLOC_FAILED
 */
gfmRV gfmQuadtree_initRoot(gfmQuadtreeRoot *pCtx, int x, int y, int width,
        int height, int maxDepth, int maxNodes) {
    gfmRV rv;
    
    // Sanitize argument
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    ASSERT(width > 0, GFMRV_ARGUMENTS_BAD);
    ASSERT(height > 0, GFMRV_ARGUMENTS_BAD);
    ASSERT(maxDepth > 0, GFMRV_ARGUMENTS_BAD);
    ASSERT(maxNodes > 0, GFMRV_ARGUMENTS_BAD);
    
    // Set the quadtree's limits
    pCtx->maxNodes = maxNodes;
    pCtx->maxDepth = maxDepth;
    // Reset all array
    gfmGenArr_reset(pCtx->pQTPool);
    gfmGenArr_reset(pCtx->pQTLLPool);
    // Clear all dynamic list
    pCtx->pAvailable = 0;
    pCtx->pColliding = 0;
    // Remove context object's
    pCtx->pObject = 0;
    pCtx->pOther = 0;
    
    // Check that the stack is big enough
    if (pCtx->stack.len < maxNodes * gfmQT_max) {
        // Expand the stack so it's big enough
        pCtx->stack.ppStack = (gfmQuadtree**)realloc(pCtx->stack.ppStack,
                sizeof(gfmQuadtree*) * maxNodes * gfmQT_max);
        ASSERT(pCtx->stack.ppStack, GFMRV_ALLOC_FAILED);
        // Set the stack's size
        pCtx->stack.len = maxNodes * gfmQT_max;
    }
    // Clear the stack
    pCtx->stack.pushPos = 0;
    
    // Retrieve the root from the qt pool
    gfmGenArr_getNextRef(gfmQuadtree, pCtx->pQTPoll, 5, pCtx->pSelf,
            gfmQuadtreeNode_getNew);
    gfmGenArr_push(pCtx->pQTPoll);
    // Initialize this node
    pCtx->pSelf->pChildren = 0;
    pCtx->pSelf->pNodes = 0;
    pCtx->pSelf->depth = 0;
    pCtx->pSelf->numObjects = 0;
    pCtx->pSelf->centerX = x + width / 2;
    pCtx->pSelf->centerY = y + height / 2;
    // Round the dimension up
    pCtx->pSelf->halfWidth = width / 2 + (width % 2);
    pCtx->pSelf->halfHeight = height / 2 + (height % 2);
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Initialize a node according with its position relative to its parent
 * 
 * @param  pCtx    The node to be initialized
 * @param  pParent The parent node
 * @param  pos     The node's relative position
 */
static gfmRV gfmQuadtree_init(gfmQuadtree *pCtx, gfmQuadtree *pParent,
        gfmQuadtreePos pos) {
    gfmRV rv;
    int offX;
    int offY;
    
    // Sanitize arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    ASSERT(pParent, GFMRV_ARGUMENTS_BAD);
    ASSERT(pos > 0, GFMRV_ARGUMENTS_BAD);
    ASSERT(pos < gfmQT_max, GFMRV_ARGUMENTS_BAD);
    
    // Clear the node's objects
    pCtx->pChildren = 0;
    pCtx->pNodes = 0;
    pCtx->numObjects = 0;
    // Set the node's depth
    pCtx->depth = pParent->depth + 1;
    // Set the node's dimensions (rounded up)
    pCtx->halfWidth = pParent->halfWidth / 2 + (pParent->halfWidth % 2);
    pCtx->halfHeight = pParent->halfHeight / 2 + (pParent->halfHeight % 2);
    // Get the offset from the parent's position
    switch (pos) {
        case gfmQT_nw: {
            offX = -pParent->halfWidth / 2;
            offY = -pParent->halfHeight / 2;
        } break;
        case gfmQT_ne: {
            offX = pParent->halfWidth / 2;
            offY = -pParent->halfHeight / 2;
        } break;
        case gfmQT_sw: {
            offX = -pParent->halfWidth / 2;
            offY = pParent->halfHeight / 2;
        } break;
        case gfmQT_se: {
            offX = pParent->halfWidth / 2;
            offY = pParent->halfHeight / 2;
        } break;
    }
    // Set the position
    pCtx->centerX = pParent->x + offX;
    pCtx->centerY = pParent->y + offY;
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

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
 * Adds an object to a node
 * 
 * @param  pCtx  The quadtree root context
 * @param  pNode The node where insertion should happen
 * @param  pObj  The object to be added
 * @return       GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_ALLOC_FAILED
 */
static gfmRV gfmQuadtree_insertObject(gfmQuadtreeRoot *pCtx, gfmQuadtree *pNode,
        gfmObject *pObj) {
    gfmQuadtreeLL *pQTLL;
    gfmRV rv;
    
    // Sanitize arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    ASSERT(pNode, GFMRV_ARGUMENTS_BAD);
    ASSERT(pObj, GFMRV_ARGUMENTS_BAD);
    
    // Retrieve a new linked-list node
    if (pCtx->pAvailable) {
        // Recycle a used node
        pQTLL = pCtx->pAvailable;
        pCtx->pAvailable = pCtx->pAvailable->pNext;
    }
    else {
        // Retrieve a new node
        gfmGenArr_getNextRef(gfmQuadtreeLL, pCtx->pQTLLPool, pCtx->maxNodes,
                pQTLL, gfmQuadtreeLL_getNew);
        gfmGenArr_push(pCtx->pQTLLPool);
    }
    
    // Add the object to the LL node
    pQTLL->pSelf = pObj;
    // Prepend the node to the list
    pQTLL->pNext = pNode->pNodes;
    pNode->pNodes = pQTLL;
    
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
static gfmRV gfmQuadtree_subdivide(gfmQuadtreeRoot *pCtx, gfmQuadtree *pNode) {
    gfmQuadtree *pChild;
    gfmQuadtreePosition i;
    gfmRV rv;
    
    // Sanitize arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    ASSERT(pNode, GFMRV_ARGUMENTS_BAD);
    
    // Alloc and initialize all the children
    i = gfmQT_nw;
    while (i < gfmQT_max) {
        gfmGenArr_getNextRef(gfmQuadtree, pCtx->pQTPoll, 5, pChild[i],
                gfmQuadtreeNode_getNew);
        gfmGenArr_push(pCtx->pQTPoll);
        // Initialize the child
        rv = gfmQuadtree_init(pChild, pNode, i);
        ASSERT_NR(rv == GFMRV_OK);
        // Set the node's child
        pNode->ppChildren[i] = pChild;
        // Go to the next one
        i++;
    }
    
    // Insert every child to the tree it's contained
    while (pNode->pNodes) {
        gfmQuadtreeLL *pTmp;
        
        // Get the current node
        pTmp = pNode->pNodes;
        
        // Add it to every child (that it overlaps)
        i = gfmQT_nw;
        while (i < gfmQT_max) {
            // Get the current child
            pChild = pNode->ppChildren[i];
            // TODO Check if the object collides this node
            // Add it to the child
            rv = gfmQuadtree_insertObject(pCtx, pChild, pTmp->pSelf);
            ASSERT_NR(rv == GFMRV_OK);
            
            i++;
        }
        
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
 * @param  pObj Pointer to whatever is being added (as a gfmObject)
 * @return      GFMRV_ARGUMENTS_BAD, GFMRV_QUADTREE_NOT_INITIALIZED, 
 *              GFMRV_QUADTREE_OVERLAPED, GFMRV_QUADTREE_DONE
 */
static gfmRV gfmQuadtree_collide(gfmQuadtreeRoot *pCtx, gfmObject *pObj) {
    gfmRV rv;
    
    // Sanitize arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    ASSERT(pPtr, GFMRV_ARGUMENTS_BAD);
    // Check if initialized
    ASSERT(pCtx->maxDepth > 0, GFMRV_QUADTREE_NOT_INITIALIZED);
    // TODO Check if this node overlaps the root
    
    // Store the object to be added
    pCtx->pObject = pObj;
    // Clear the call stack
    pCtx->pStack.pushPos = 0;
    // Push the root node to start colliding
    rv = gfmQuadtree_pushNode(pCtx, pSelf);
    ASSERT_NR(rv == GFMRV_OK);
    
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
    
    rv = gfmQuadtree_collide(pCtx, pObj);
    
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
    rv = gfmQuadtree_collide(pCtx, pObj);
__ret:
    return rv;
}

gfmRV gfmQuadtree_collideTilemap(gfmQuadtreeRoot *pCtx, gfmTilemap *pTMap);
gfmRV gfmQuadtree_populateGroup(gfmQuadtreeRoot *pCtx, gfmGroup *pGrp);
gfmRV gfmQuadtree_populateObject(gfmQuadtreeRoot *pCtx, gfmObject *pObj);
gfmRV gfmQuadtree_populateSprite(gfmQuadtreeRoot *pCtx, gfmSprite *pSpr);
gfmRV gfmQuadtree_populateTilemap(gfmQuadtreeRoot *pCtx, gfmTilemap *pTMap);

/**
 * Return both objects that overlaped
 * 
 * @param  ppObj1 A object that just overlapped another
 * @param  ppObj2 A object that just overlapped another
 * @param  pCtx   The quadtree's root
 * @return        GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_QUADTREE_NO_OVERLAP
 */
gfmRV gfmQuadtree_getOverlaping(gfmObject **ppObj1, gfmObject **ppObj2,
        gfmQuadtreeRoot  *pCtx) {
    gfmRV rv;
    
    // Sanitize arguments
    ASSERT(ppObj1, GFMRV_ARGUMENTS_BAD);
    ASSERT(ppObj2, GFMRV_ARGUMENTS_BAD);
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    // Check that an overlap just happened
    ASSERT(pCtx->pOther, GFMRV_QUADTREE_NO_OVERLAP);
    
    // Return both objects
    *ppObj1 = pCtx->pObject;
    *ppObj2 = pCtx->pOther;
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

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
    
    // Continue adding the object
    while (1) {
        gfmQuadtree *pNode;
        
        // If we were colliding againts objects
        if (pCtx->pColliding) {
            gfmQuadtreeLL *pTmp;
            
            // Retrieve the current object and update the list
            pTmp = pCtx->pColliding;
            pCtx->pColliding = pCtx->pColliding->pNext;
            
            // Check if both objects overlaps
            pCtx->pOther = pTmp->pSelf;
            rv = gfmObject_isOverlaping(pCtx->pObject, pCtx->Other);
            // If they did overlap, return with that status
            ASSERT(rv != GFMRV_TRUE, GFMRV_QUADTREE_OVERLAPED);
        }
        else {
            // Pop the current node
            rv = gfmQuadtree_popNode(&pNode, pCtx);
            ASSERT_NR(rv == GFMRV_OK);

            // If it has children, push its children
            if (pNode->pChildren) {
                gfmQuadtreePosition i;

                i = gfmQT_nw;
                while (i < gfmQT_max) {
                    // TODO Check if the object overlaps this node
                    // Push it (so it will collide later)
                    rv = gfmQuadtree_pushNode(pCtx, &(pNode->pChildren[i]));
                    ASSERT_NR(rv == GFMRV_OK);
                    
                    i++;
                }
            }
            else {
                // TODO Check if adding the node will subdivide the tree
                // Otherwise, collide with its nodes
                pCtx->pColliding = pNode->pNodes;
            }
        }
    }
    
    rv = GFMRV_QUADTREE_DONE;
__ret:
    return rv;
}

gfmRV gfmQuadtree_drawBounds(gfmQuadtreeRoot *pQt, gfmCtx *pCtx);

