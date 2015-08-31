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
#include <GFraMe/gfmTypes.h>

#include <stdlib.h>
#include <string.h>

static unsigned char gfmQt_defColors[(gfmType_max + 1) * 3] =
{
/* red ,green, blue */
  0x72 ,0xab , 0x97,  /* quadtree_node */
  0x8c ,0x7a , 0xae,  /* gfmType_object */
  0xff ,0xf4 , 0xaa,  /* gfmType_sprite */
  0xff ,0xc6 , 0xaa,  /* gfmType_tilemap */
  0x47 ,0x8e , 0x75,  /* gfmType_group */
  0x67 ,0x50 , 0x91,  /* gfmType_reserved_2 */
  0xd4 ,0xc7 , 0x6a,  /* gfmType_reserved_3 */
  0xd4 ,0x8d , 0x6a,  /* gfmType_reserved_4 */
  0x26 ,0x72 , 0x57,  /* gfmType_reserved_5 */
  0xff ,0xff , 0xff,  /* gfmType_reserved_6 */
  0x47 ,0x2e , 0x74,  /* gfmType_reserved_7 */
  0xaa ,0x9c , 0x39,  /* gfmType_reserved_8 */
  0xaa ,0x5d , 0x39,  /* gfmType_reserved_9 */
  0x0e ,0x55 , 0x3c,  /* gfmType_reserved_10 */
  0x2d ,0x16 , 0x57,  /* gfmType_reserved_11 */
  0x80 ,0x72 , 0x15,  /* gfmType_reserved_12 */
  0x80 ,0x38 , 0x15,  /* gfmType_reserved_13 */
  0x00 ,0x39 , 0x26,  /* gfmType_reserved_14 */
  0x18 ,0x06 , 0x3a,  /* gfmType_reserved_15 */
  0x55 ,0x4a , 0x00,  /* gfmType_reserved_16 */
  0x55 ,0x1c , 0x00,  /* gfmType_max */
};

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
    int halfHeight;
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

/******************************************************************************/
/*                                                                            */
/* Static functions                                                           */
/*                                                                            */
/******************************************************************************/

/**
 * Alloc a new quadtree linked-list node
 * 
 * @param  ppCtx The linked list node
 * @return       GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_ALLOC_FAILED
 */
static gfmRV gfmQuadtreeLL_getNew(gfmQuadtreeLL **ppCtx) {
    gfmRV rv;
    
    // Sanitize arguments
    ASSERT(ppCtx, GFMRV_ARGUMENTS_BAD);
    ASSERT(!(*ppCtx), GFMRV_ARGUMENTS_BAD);
    
    // Alloc it
    *ppCtx = (gfmQuadtreeLL*)malloc(sizeof(gfmQuadtreeLL));
    ASSERT(*ppCtx, GFMRV_ALLOC_FAILED);
    // Clean it
    memset(*ppCtx, 0x0, sizeof(gfmQuadtreeLL));
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Frees a quadtree linked-list node
 * 
 * @param  ppCtx The linked list node
 * @return       GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
static gfmRV gfmQuadtreeLL_free(gfmQuadtreeLL **ppCtx) {
    gfmRV rv;
    
    // Sanitize arguments
    ASSERT(ppCtx, GFMRV_ARGUMENTS_BAD);
    
    // Remove all of its pointers
    memset(*ppCtx, 0x0, sizeof(gfmQuadtreeLL));
    // Release the memory
    free(*ppCtx);
    *ppCtx = 0;
    
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
 * Frees a quadtree node
 * 
 * @param  ppCtx The quadtree node
 * @return       GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
static gfmRV gfmQuadtreeNode_free(gfmQuadtree **ppCtx) {
    gfmRV rv;
    
    // Sanitize arguments
    ASSERT(ppCtx, GFMRV_ARGUMENTS_BAD);
    
    // Remove all of its pointers
    memset(*ppCtx, 0x0, sizeof(gfmQuadtree));
    // Release the memory
    free(*ppCtx);
    *ppCtx = 0;
    
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
        gfmQuadtreePosition pos) {
    gfmQuadtreePosition i;
    gfmRV rv;
    int offX;
    int offY;
    
    // Sanitize arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    ASSERT(pParent, GFMRV_ARGUMENTS_BAD);
    ASSERT(pos >= gfmQT_nw, GFMRV_ARGUMENTS_BAD);
    ASSERT(pos < gfmQT_max, GFMRV_ARGUMENTS_BAD);
    
    // Clear all children
    i = gfmQT_nw;
    while (i < gfmQT_max) {
        pCtx->ppChildren[i] = 0;
        i++;
    }
    // Clear the node's objects
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
        case gfmQT_max: {
            ASSERT(0, GFMRV_INTERNAL_ERROR);
        } break;
    }
    // Set the position
    pCtx->centerX = pParent->centerX + offX;
    pCtx->centerY = pParent->centerY + offY;
    
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
    // Increase the counter
    pNode->numObjects++;
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Checks if a quadtree node overlaps an object
 * 
 * @param  pCtx The quadtree node
 * @param  pObj The gfmObject
 * @return      GFMRV_TRUE, GFMRV_FALSE, GFMRV_ARGUMENTS_BAD
 */
static gfmRV gfmQuadtree_overlap(gfmQuadtree *pCtx, gfmObject *pObj) {
    gfmRV rv;
    int cX, cY, dist, hWidth, hHeight, maxDist;
    
    // Sanitize arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    ASSERT(pObj, GFMRV_ARGUMENTS_BAD);
    // Get the object's dimensions
    rv = gfmObject_getCenter(&cX, &cY, pObj);
    ASSERT_NR(rv == GFMRV_OK);
    rv = gfmObject_getDimensions(&hWidth, &hHeight, pObj);
    ASSERT_NR(rv == GFMRV_OK);
    // Get half the dimensions (rounded up)
    hWidth = hWidth / 2 + (hWidth % 2);
    hHeight = hHeight / 2 + (hHeight % 2);
    
    // Check that they are overlaping (horizontally)
    dist = cX - pCtx->centerX;
    if (dist < 0) {
        dist = -dist;
    }
    maxDist = hWidth + pCtx->halfWidth;
    if (dist > maxDist) {
        return GFMRV_FALSE;
    }
    //ASSERT(dist <= maxDist, GFMRV_FALSE);
    
    // Check vertically...
    dist = cY - pCtx->centerY;
    if (dist < 0) {
        dist = -dist;
    }
    maxDist = hHeight + pCtx->halfHeight;
    if (dist > maxDist) {
        return GFMRV_FALSE;
    }
    //ASSERT(dist <= maxDist, GFMRV_FALSE);
    
    rv = GFMRV_TRUE;
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
        gfmGenArr_getNextRef(gfmQuadtree, pCtx->pQTPool, 5, pChild,
                gfmQuadtreeNode_getNew);
        gfmGenArr_push(pCtx->pQTPool);
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
            // Check if the object collides this node
            rv = gfmQuadtree_overlap(pChild, pTmp->pSelf);
            if (rv == GFMRV_TRUE) {
                // Add it to the child
                rv = gfmQuadtree_insertObject(pCtx, pChild, pTmp->pSelf);
                ASSERT_NR(rv == GFMRV_OK);
            }
            
            i++;
        }
        
        // Go to the next node
        pNode->pNodes = pTmp->pNext;
        // Prepend the node to the free list
        pTmp->pNext = pCtx->pAvailable;
        pCtx->pAvailable = pTmp;
    }
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/******************************************************************************/
/*                                                                            */
/* Public functions                                                           */
/*                                                                            */
/******************************************************************************/

/**
 * Alloc a new root quadtree
 * 
 * @param  ppCtx The root quadtree
 * @return       GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_ALLOC_FAILED
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
 * @param  ppCtx The quadtree root
 * @return       GFMRV_OK, GFMRV_ARGUMENTS_BAD
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
    *ppCtx = 0;
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Clean all memory used by the entire quadtree
 * 
 * @param  pCtx The quadtree root
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmQuadtree_clean(gfmQuadtreeRoot *pCtx) {
    gfmRV rv;
    
    // Sanitize arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    
    // Clean all generic arrays
    gfmGenArr_clean(pCtx->pQTPool, gfmQuadtreeNode_free);
    gfmGenArr_clean(pCtx->pQTLLPool, gfmQuadtreeLL_free);
    // Clean the stack, if any
    if (pCtx->stack.ppStack) {
        free(pCtx->stack.ppStack);
    }
    memset(pCtx, 0x0, sizeof(gfmQuadtreeRoot));
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

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
    gfmQuadtreePosition i;
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
    if (pCtx->stack.len < maxDepth * gfmQT_max) {
        // Expand the stack so it's big enough
        pCtx->stack.ppStack = (gfmQuadtree**)realloc(pCtx->stack.ppStack,
                sizeof(gfmQuadtree*) * maxDepth * gfmQT_max);
        ASSERT(pCtx->stack.ppStack, GFMRV_ALLOC_FAILED);
        // Set the stack's size
        pCtx->stack.len = maxDepth * gfmQT_max;
    }
    // Clear the stack
    pCtx->stack.pushPos = 0;
    
    // Retrieve the root from the qt pool
    gfmGenArr_getNextRef(gfmQuadtree, pCtx->pQTPool, 5, pCtx->pSelf,
            gfmQuadtreeNode_getNew);
    gfmGenArr_push(pCtx->pQTPool);
    // Initialize this node
    i = gfmQT_nw;
    while (i < gfmQT_max) {
        pCtx->pSelf->ppChildren[i] = 0;
        i++;
    }
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
 * Adds a new gfmGroup to the quadtree, subdividing it as necessary and
 * colliding with every possible node
 * 
 * @param  pCtx The quadtree's root
 * @param  pGrp The gfmGroup
 * @return      GFMRV_ARGUMENTS_BAD, GFMRV_QUADTREE_NOT_INITIALIZED, 
 *              GFMRV_QUADTREE_OVERLAPED, GFMRV_QUADTREE_DONE
 */
gfmRV gfmQuadtree_collideGroup(gfmQuadtreeRoot *pCtx, gfmGroup *pGrp) {
    return GFMRV_FUNCTION_NOT_IMPLEMENTED;
}

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
    
    // Sanitize arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    ASSERT(pObj, GFMRV_ARGUMENTS_BAD);
    // Check if initialized
    ASSERT(pCtx->maxDepth > 0, GFMRV_QUADTREE_NOT_INITIALIZED);
    // Check if this node overlaps the root
    rv = gfmQuadtree_overlap(pCtx->pSelf, pObj);
    if (rv != GFMRV_TRUE) {
        rv = GFMRV_QUADTREE_DONE;
        goto __ret;
    }
    //ASSERT(rv == GFMRV_TRUE, GFMRV_QUADTREE_DONE);
    
    // Store the object to be added
    pCtx->pObject = pObj;
    // Clear the call stack
    pCtx->stack.pushPos = 0;
    // Clear any previous overlap
    pCtx->pOther = 0;
    
    // Push the root node to start colliding
    rv = gfmQuadtree_pushNode(pCtx, pCtx->pSelf);
    ASSERT_NR(rv == GFMRV_OK);
    
    // Collide it
    rv = gfmQuadtree_continue(pCtx);
__ret:
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
    rv = gfmQuadtree_collideObject(pCtx, pObj);
__ret:
    return rv;
}

/**
 * Adds a new gfmTilemap to the quadtree, subdividing it as necessary and
 * colliding with every possible node
 * 
 * @param  pCtx  The quadtree's root
 * @param  pTMap The gfmTilemap
 * @return       GFMRV_ARGUMENTS_BAD, GFMRV_QUADTREE_NOT_INITIALIZED, 
 *               GFMRV_QUADTREE_OVERLAPED, GFMRV_QUADTREE_DONE
 */
gfmRV gfmQuadtree_collideTilemap(gfmQuadtreeRoot *pCtx, gfmTilemap *pTMap) {
    return GFMRV_FUNCTION_NOT_IMPLEMENTED;
}

/**
 * Populates a quadtree with a group's objects
 * 
 * @param  pCtx The quadtree's root
 * @param  pGrp The tilemap
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_QUADTREE_NOT_INITIALIZED
 */
gfmRV gfmQuadtree_populateGroup(gfmQuadtreeRoot *pCtx, gfmGroup *pGrp) {
    return GFMRV_FUNCTION_NOT_IMPLEMENTED;
}

/**
 * Add an object to the quadtree without collinding it against the tree's objs
 * 
 * @param  pCtx The quadtree's root
 * @param  pObj The gfmObject
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_QUADTREE_NOT_INITIALIZED
 */
gfmRV gfmQuadtree_populateObject(gfmQuadtreeRoot *pCtx, gfmObject *pObj) {
    gfmRV rv;
    
    // Sanitize arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    ASSERT(pObj, GFMRV_ARGUMENTS_BAD);
    // Check if initialized
    ASSERT(pCtx->maxDepth > 0, GFMRV_QUADTREE_NOT_INITIALIZED);
    
    // Check that the object overlaps the root node
    rv = gfmQuadtree_overlap(pCtx->pSelf, pObj);
    ASSERT(rv == GFMRV_TRUE, GFMRV_OK);
    
    // Clear the call stack
    pCtx->stack.pushPos = 0;
    // Clear any previous overlap
    pCtx->pOther = 0;
    
    // Push the root node to start overlaping
    rv = gfmQuadtree_pushNode(pCtx, pCtx->pSelf);
    ASSERT_NR(rv == GFMRV_OK);
    
    // Continue adding the object
    while (pCtx->stack.pushPos > 0) {
        gfmQuadtree *pNode;
        
        // Pop the current node
        rv = gfmQuadtree_popNode(&pNode, pCtx);
        ASSERT_NR(rv == GFMRV_OK);

        // If it has children, push its children
        if (pNode->ppChildren[gfmQT_nw]) {
            gfmQuadtreePosition i;
            gfmQuadtree *pChild;

            i = gfmQT_nw;
            while (i < gfmQT_max) {
                // Get the current child
                pChild = pNode->ppChildren[i];
                // Check if the object overlaps this node
                rv = gfmQuadtree_overlap(pChild, pObj);
                if (rv == GFMRV_TRUE) {
                    // Push it (so it will collide later)
                    rv = gfmQuadtree_pushNode(pCtx, pChild);
                    ASSERT_NR(rv == GFMRV_OK);
                }
                i++;
            }
        }
        else {
            // Check if adding the node will subdivide the tree and if it
            // can still be subdivided
            if (pNode->numObjects + 1 > pCtx->maxNodes &&
                    pNode->depth + 1 < pCtx->maxDepth) {
                // Subdivide the tree
                rv = gfmQuadtree_subdivide(pCtx, pNode);
                ASSERT_NR(rv == GFMRV_OK);
                // Push the node again so its children are overlapped/pushed
                rv = gfmQuadtree_pushNode(pCtx, pNode);
                ASSERT_NR(rv == GFMRV_OK);
            }
            else {
                // Add the object to this node 
                rv = gfmQuadtree_insertObject(pCtx, pNode, pObj);
                ASSERT_NR(rv == GFMRV_OK);
            }
        }
    }
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Add a sprite to the quadtree without collinding it against the tree's objs
 * 
 * @param  pCtx The quadtree's root
 * @param  pSpr The gfmSprite
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_QUADTREE_NOT_INITIALIZED
 */
gfmRV gfmQuadtree_populateSprite(gfmQuadtreeRoot *pCtx, gfmSprite *pSpr) {
    gfmObject *pObj;
    gfmRV rv;
    
    // Sanitize sprite (other checks are done in sub-functions)
    ASSERT(pSpr, GFMRV_ARGUMENTS_BAD);
    // Retrieve the sprite's object
    rv = gfmSprite_getObject(&pObj, pSpr);
    ASSERT_NR(rv == GFMRV_OK);
    // Add it to the quadtree
    rv = gfmQuadtree_populateObject(pCtx, pObj);
__ret:
    return rv;
}

/**
 * Populates a quadtree with tilemap's areas
 * 
 * @param  pCtx  The quadtree's root
 * @param  pTMap The tilemap
 * @return       GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_QUADTREE_NOT_INITIALIZED
 */
gfmRV gfmQuadtree_populateTilemap(gfmQuadtreeRoot *pCtx, gfmTilemap *pTMap) {
    gfmRV rv;
    int i, len;
    
    // Sanitize arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    ASSERT(pTMap, GFMRV_ARGUMENTS_BAD);
    
    // Get how many areas there are
    rv = gfmTilemap_getAreasLength(&len, pTMap);
    ASSERT_NR(rv == GFMRV_OK);
    
    // Add every object
    i = 0;
    while (i < len) {
        gfmObject *pObj;
        
        // Retrieve the object
        rv = gfmTilemap_getArea(&pObj, pTMap, i);
        ASSERT_NR(rv == GFMRV_OK);
        
        // Add it to the quadtree
        rv = gfmQuadtree_populateObject(pCtx, pObj);
        ASSERT_NR(rv == GFMRV_OK);
        
        i++;
    }
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

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
 * Continue colliding and adding the node to the quadtree
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
    while (pCtx->stack.pushPos > 0 || pCtx->pColliding) {
        gfmQuadtree *pNode;
        
        // If we were colliding againts objects
        if (pCtx->pColliding) {
            gfmQuadtreeLL *pTmp;
            
            // Retrieve the current object and update the list
            pTmp = pCtx->pColliding;
            pCtx->pColliding = pCtx->pColliding->pNext;
            
            // Check if both objects overlaps
            pCtx->pOther = pTmp->pSelf;
            rv = gfmObject_isOverlaping(pCtx->pObject, pCtx->pOther);
            
            // -- Exit point --
            // If they did overlap, return with that status
            if (rv == GFMRV_TRUE) {
                return GFMRV_QUADTREE_OVERLAPED;
            }
            //ASSERT(rv != GFMRV_TRUE, GFMRV_QUADTREE_OVERLAPED);
        }
        else {
            // Pop the current node
            rv = gfmQuadtree_popNode(&pNode, pCtx);
            ASSERT_NR(rv == GFMRV_OK);
            
            // If it has children, push its children
            if (pNode->ppChildren[gfmQT_nw]) {
                gfmQuadtreePosition i;
                gfmQuadtree *pChild;

                i = gfmQT_nw;
                while (i < gfmQT_max) {
                    // Get the current child
                    pChild = pNode->ppChildren[i];
                    // Check if the object overlaps this node
                    rv = gfmQuadtree_overlap(pChild, pCtx->pObject);
                    if (rv == GFMRV_TRUE) {
                        // Push it (so it will collide later)
                        rv = gfmQuadtree_pushNode(pCtx, pChild);
                        ASSERT_NR(rv == GFMRV_OK);
                    }
                    i++;
                }
            }
            else {
                // Check if adding the node will subdivide the tree and if it
                // can still be subdivided
                if (pNode->numObjects + 1 > pCtx->maxNodes &&
                        pNode->depth + 1 < pCtx->maxDepth) {
                    // Subdivide the tree
                    rv = gfmQuadtree_subdivide(pCtx, pNode);
                    ASSERT_NR(rv == GFMRV_OK);
                    // Push the node again so its children are overlapped/pushed
                    rv = gfmQuadtree_pushNode(pCtx, pNode);
                    ASSERT_NR(rv == GFMRV_OK);
                }
                else {
                    // Otherwise, collide with its nodes
                    pCtx->pColliding = pNode->pNodes;
                    // Add the object to this node 
                    // NOTE: It's added to the begin, so it won't overlap itself
                    rv = gfmQuadtree_insertObject(pCtx, pNode, pCtx->pObject);
                    ASSERT_NR(rv == GFMRV_OK);
                }
            }
        }
    }
    
    // If the loop stoped, the operation finished
    pCtx->pObject = 0;
    rv = GFMRV_QUADTREE_DONE;
__ret:
    return rv;
}

/**
 * Draw the quadtree to the screen; Colors are configured accordingly to the
 * object's type (therefore, there are 20 basic colors and a default one)
 * NOTE: This functions will be most likely slow!! Be careful when calling it!
 * 
 * @param  pQt     The quadtree's root
 * @param  pCtx    The game's context
 * @param  pColors The colors to be used; Must have 21 colors, each with RGB
 *                 components; The first set is for quadtree nodes, while the
 *                 others respect the types on gfmTypes.h
 * @return         GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmQuadtree_drawBounds(gfmQuadtreeRoot *pQt, gfmCtx *pCtx,
        unsigned char *pColors) {
    gfmRV rv;
    
    // Sanitize arguments
    ASSERT(pQt, GFMRV_ARGUMENTS_BAD);
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    
    // If no color was set, use the default ones
    if (pColors == 0) {
        pColors = gfmQt_defColors;
    }
    
    // Clear the call stack
    pQt->stack.pushPos = 0;
    
    // Push the root node to start colliding
    rv = gfmQuadtree_pushNode(pQt, pQt->pSelf);
    ASSERT_NR(rv == GFMRV_OK);
    
    // Iterate through all nodes
    while (pQt->stack.pushPos > 0 || pQt->pColliding) {
        gfmQuadtree *pNode;
        unsigned char *pNodeColor;
        
        // Pop the current node
        rv = gfmQuadtree_popNode(&pNode, pQt);
        ASSERT_NR(rv == GFMRV_OK);
        
        // Get the colors for the qt node
        pNodeColor = pColors;
        // Draw the current node
        rv = gfm_drawRect(pCtx, pNode->centerX - pNode->halfWidth,
                pNode->centerY - pNode->halfHeight, pNode->halfWidth * 2,
                pNode->halfHeight * 2, pNodeColor[0], pNodeColor[1],
                pNodeColor[2]);
        ASSERT_NR(rv == GFMRV_OK);
        
        // If it has children, push its children
        if (pNode->ppChildren[gfmQT_nw]) {
            gfmQuadtreePosition i;
            gfmQuadtree *pChild;

            i = gfmQT_nw;
            while (i < gfmQT_max) {
                // Get the current child
                pChild = pNode->ppChildren[i];
                // Push it (so it will be drawn later)
                rv = gfmQuadtree_pushNode(pQt, pChild);
                ASSERT_NR(rv == GFMRV_OK);
                i++;
            }
        }
        else {
            gfmQuadtreeLL *pTmp;
            
            // Otherwise, draw its nodes
            pTmp = pNode->pNodes;
            
            while (pTmp) {
                int height, type, width, x, y;
                void *pChild;
                
                // Get the object's child
                rv = gfmObject_getChild(&pChild, &type, pTmp->pSelf);
                ASSERT_NR(rv == GFMRV_OK);
                if (type == gfmType_sprite) {
                    rv = gfmSprite_getChild(&pChild, &type, (gfmSprite*)pChild);
                    ASSERT_NR(rv == GFMRV_OK);
                    
                    if (type == gfmType_none) {
                        // If no custom type was specified, set it to sprite
                        type = gfmType_sprite;
                    }
                }
                else if (type == gfmType_none) {
                    // If no custom type was specified, set it to object
                    type = gfmType_object;
                }
                
                // Get the object's color
                if (type >= gfmType_max)
                    type = gfmType_max;
                pNodeColor = pColors + type * 3;
                
                // Get the object's position
                rv = gfmObject_getPosition(&x, &y, pTmp->pSelf);
                ASSERT_NR(rv == GFMRV_OK);
                // Get the object's dimensions
                rv = gfmObject_getDimensions(&width, &height, pTmp->pSelf);
                ASSERT_NR(rv == GFMRV_OK);
                
                // Draw the current node
                rv = gfm_drawRect(pCtx, x, y, width, height, pNodeColor[0],
                        pNodeColor[1], pNodeColor[2]);
                ASSERT_NR(rv == GFMRV_OK);
                
                pTmp = pTmp->pNext;
            }
        }
    }
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

