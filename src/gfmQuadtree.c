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
#include <GFraMe/gfmHitbox.h>
#include <GFraMe/gfmObject.h>
#include <GFraMe/gfmQuadtree.h>
#include <GFraMe/gfmSprite.h>
#include <GFraMe/gfmTilemap.h>
#include <GFraMe/gfmTypes.h>

#include <GFraMe_int/gfmHitbox.h>
#include <GFraMe_int/gfmObject.h>

#include <stdlib.h>
#include <string.h>

static unsigned char gfmQt_defColors[(gfmType_max) * 3] =
{
/* red ,green, blue */
  0x99 ,0xe5 , 0x50,  /* quadtree_node       (light green) */
  0xac ,0x32 , 0x32,  /* gfmType_object      (red) */
  0xcb ,0xdb , 0xfc,  /* gfmType_sprite      (lightest blue) */
  0x00 ,0x00 , 0x00,  /* gfmType_tilemap     (isn't actually drawn) */
  0x22 ,0x20 , 0x34,  /* gfmType_group       (isn't actually drawn) */
  0xd7 ,0x7b , 0xba,  /* gfmType_reserved_2  (pink) */
  0x5f ,0xcd , 0xe4,  /* gfmType_reserved_3  (light blue) */
  0x6a ,0xbe , 0x30,  /* gfmType_reserved_4  (green) */
  0x76 ,0x42 , 0x8a,  /* gfmType_reserved_5  (purple) */
  0xfb ,0xf2 , 0x36,  /* gfmType_reserved_6  (yellow) */
  0xd9 ,0x57 , 0x63,  /* gfmType_reserved_7  (light red) */
  0xee ,0xc3 , 0x9a,  /* gfmType_reserved_8  (beige) */
  0x63 ,0x9b , 0xff,  /* gfmType_reserved_9  (blue) */
  0x8f ,0x97 , 0x4a,  /* gfmType_reserved_10 (dirty yellow/green) */
  0xff ,0xff , 0xff,  /* gfmType_reserved_11 (white) */
  0x80 ,0x72 , 0x15,  /* gfmType_reserved_12 */
  0x80 ,0x38 , 0x15,  /* gfmType_reserved_13 */
  0x00 ,0x39 , 0x26,  /* gfmType_reserved_14 */
  0x18 ,0x06 , 0x3a,  /* gfmType_reserved_15 */
  0x55 ,0x4a , 0x00,  /* gfmType_reserved_16 */
  0x55 ,0x4a , 0x00,  /* gfmType_reserved_17 */
  0x55 ,0x4a , 0x00,  /* gfmType_reserved_18 */
  0x55 ,0x4a , 0x00,  /* gfmType_reserved_19 */
  0x55 ,0x4a , 0x00,  /* gfmType_reserved_20 */
  0x55 ,0x4a , 0x00,  /* gfmType_reserved_21 */
  0x55 ,0x4a , 0x00,  /* gfmType_reserved_22 */
  0x55 ,0x4a , 0x00,  /* gfmType_reserved_23 */
  0x55 ,0x4a , 0x00,  /* gfmType_reserved_24 */
  0x55 ,0x4a , 0x00,  /* gfmType_reserved_25 */
  0x55 ,0x4a , 0x00,  /* gfmType_reserved_26 */
  0x55 ,0x4a , 0x00,  /* gfmType_reserved_27 */
  0x55 ,0x4a , 0x00,  /* gfmType_reserved_28 */
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

enum enGFMQuadtreeFlags {
    gfmQT_isStatic           = 0x0000001
  , gfmQT_isActive           = 0x0000002
  , gfmQT_justOverlaped      = 0x0000004
  , gfmQT_continuousCollision = 0x0000008
};

/** A quadtree-managed hitbox, with a user defined object */
#define gfmType_quadtreeHitbox     0xC0000000
/** A quadtree-managed hitbox, with static object (i.e., this object mustn't
 * be copied) */
#define gfmType_quadtreeRootHitbox 0xE0000000

/** Index of a child relative to its parent */
struct stGFMQuadtreeLL {
    /** This node's area */
    gfmHitbox area;
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
    /** Hitboxe's area. If necessary/desired, depth and numObjects may be packed
     * into the area's 'type' attribute. */
    gfmHitbox area;
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
    /** Whether this is a static quadtree (one that may be populated but
     * colliding doesn't insert nodes */
    int flags;
    /** Pool of quadtree nodes */
    gfmGenArr_var(gfmQuadtree, pQTPool);
    /** Pool of quadtree LL nodes */
    gfmGenArr_var(gfmQuadtreeLL, pQTLLPool);
    /** List of collideables objects from a group */
    gfmGroupNode *pGroupList;
    /** The actual root node of the quadtree */
    gfmQuadtree *pSelf;
    /** List of available LL nodes */
    gfmQuadtreeLL *pAvailable;
    /** List of nodes to be collided */
    gfmQuadtreeLL *pColliding;
    /** Stack of nodes which the object must still be added to */
    gfmQuadtreeStack stack;
    /** The object that was just overlapped */
    gfmHitbox *pOther;
    /** Object being collided */
    gfmHitbox object;
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
    int h, w, x, y;
    
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
    /* Retrieve the parent's position */
    gfmObject_getDimensions(&w, &h, (gfmObject*)&pParent->area);
    gfmObject_getPosition(&x, &y, (gfmObject*)&pParent->area);
    /* Adjust it according to the child relative position */
    // Get the offset from the parent's position
    w /= 2;
    h /= 2;
    switch (pos) {
        case gfmQT_nw: {
            /* Do nothing */
        } break;
        case gfmQT_ne: {
            x += w;
        } break;
        case gfmQT_sw: {
            y += h;
        } break;
        case gfmQT_se: {
            x += w;
            y += h;
        } break;
        case gfmQT_max: {
            ASSERT(0, GFMRV_INTERNAL_ERROR);
        } break;
    }
    gfmHitbox_init(&pCtx->area, pCtx, x, y, w, h, gfmType_none);
    
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
 * Convert a user-managed object to a quadtree-managed area.
 *
 * This area considers the full extensions of the object (so it may be longer
 * than the object, if continuous collision is enabled).
 *
 * @param  [out]pArea Area to be initialized with the collision boundary
 * @param  [ in]pCtx  The quadtree's root
 * @param  [ in]pObj  The object
 */
static void gfmQuadtree_convertObjectToHitbox(gfmHitbox *pArea
        , gfmQuadtreeRoot *pCtx, gfmObject *pObj) {
    int h, x, y, w;

    if (pCtx->flags & gfmQT_continuousCollision) {
        _gfmObject_getContinousCollisionBoundary(&x, &y, &w, &h, pObj);
    }
    else {
        gfmObject_getCollisionBoundary(&x, &y, &w, &h, pObj);
    }

    gfmHitbox_init(pArea, pObj, x, y, w, h, gfmType_quadtreeHitbox);
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
    do {
        void *pVoid;
        int type;

        gfmObject_getChild(&pVoid, &type, pObj);
        if (type == gfmType_quadtreeHitbox) {
            /* Object has already been converted. Simply copy it */
            memcpy(&pQTLL->area, pObj, sizeof(gfmHitbox));
        }
        else if (type == gfmType_quadtreeRootHitbox) {
            /* Since we are dealing with a unique object (belonging to the
             * quadtree's root), stores its inner object (user-managed) into the
             * node */
            gfmQuadtree_convertObjectToHitbox(&pQTLL->area, pCtx
                    , (gfmObject*)pVoid);
        }
        else {
            /* Convert it into the internal object */
            gfmQuadtree_convertObjectToHitbox(&pQTLL->area, pCtx, pObj);
        }
    } while (0);
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
 * @param  [ in]pCtx The quadtree node
 * @param  [ in]pObj The overlaping object
 * @return           GFMRV_TRUE, GFMRV_FALSE, GFMRV_ARGUMENTS_BAD
 */
static inline gfmRV gfmQuadtree_overlap(gfmQuadtree *pCtx, gfmObject *pObj) {
    return gfmObject_isOverlaping((gfmObject*)&pCtx->area, pObj);
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
            rv = gfmQuadtree_overlap(pChild, (gfmObject*)&pTmp->area);
            if (rv == GFMRV_TRUE) {
                // Add it to the child
                rv = gfmQuadtree_insertObject(pCtx, pChild,
                        (gfmObject*)&pTmp->area);
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
    pCtx->pOther = 0;
    pCtx->pGroupList = 0;
    /* Remove the static flag */
    pCtx->flags &= ~(gfmQT_isStatic | gfmQT_isActive);
    
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
    gfmHitbox_init(&pCtx->pSelf->area, pCtx, x, y, width, height, gfmType_none);
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Make the quadtree static
 *
 * @param  [ in]pCtx The quadtree's root
 * @return      GFMRV_ARGUMENTS_BAD, GFMRV_QUADTREE_NOT_INITIALIZED, GFMRV_OK
 */
gfmRV gfmQuadtree_setStatic(gfmQuadtreeRoot *pCtx) {
    gfmRV rv;

    /* Sanitize arguments */
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    /* Check if initialized */
    ASSERT(pCtx->maxDepth > 0, GFMRV_QUADTREE_NOT_INITIALIZED);

    pCtx->flags |= gfmQT_isStatic;

    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Enable continuous collision for every object
 *
 * @param  [ in]pCtx The quadtree's root
 * @return           GFMRV_ARGUMENTS_BAD, GFMRV_QUADTREE_NOT_INITIALIZED
 *                   , GFMRV_OK
 */
gfmRV gfmQuadtree_enableContinuosCollision(gfmQuadtreeRoot *pCtx) {
    gfmRV rv;

    /* Sanitize arguments */
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    /* Check if initialized */
    ASSERT(pCtx->maxDepth > 0, GFMRV_QUADTREE_NOT_INITIALIZED);

    pCtx->flags |= gfmQT_continuousCollision;

    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Disable continuous collision for every object
 *
 * @param  [ in]pCtx The quadtree's root
 * @return           GFMRV_ARGUMENTS_BAD, GFMRV_QUADTREE_NOT_INITIALIZED
 *                   , GFMRV_OK
 */
gfmRV gfmQuadtree_disableContinuosCollision(gfmQuadtreeRoot *pCtx) {
    gfmRV rv;

    /* Sanitize arguments */
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    /* Check if initialized */
    ASSERT(pCtx->maxDepth > 0, GFMRV_QUADTREE_NOT_INITIALIZED);

    pCtx->flags &= ~gfmQT_continuousCollision;

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
    gfmRV rv;
    
    // Sanitize arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    ASSERT(pGrp, GFMRV_ARGUMENTS_BAD);
    // Check if initialized
    ASSERT(pCtx->maxDepth > 0, GFMRV_QUADTREE_NOT_INITIALIZED);
    
    // Get the list of collideable objects
    rv = gfmGroup_getCollideableList(&(pCtx->pGroupList), pGrp);
    ASSERT(rv == GFMRV_OK || rv == GFMRV_GROUP_LIST_EMPTY, rv);
    
    // Start the collision
    if (rv == GFMRV_OK) {
        // Clear the call stack
        pCtx->stack.pushPos = 0;
        // Clear any previous overlap
        pCtx->pOther = 0;
        
        pCtx->flags |= gfmQT_isActive;
        rv = gfmQuadtree_continue(pCtx);
    }
    else {
        rv = GFMRV_QUADTREE_DONE;
    }
__ret:
    return rv;
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
    gfmQuadtree_convertObjectToHitbox(&pCtx->object, pCtx, pObj);
    gfmObject_setType((gfmObject*)&pCtx->object, gfmType_quadtreeRootHitbox);
    // Clear the call stack
    pCtx->stack.pushPos = 0;
    // Clear any previous overlap
    pCtx->pOther = 0;
    
    // Push the root node to start colliding
    rv = gfmQuadtree_pushNode(pCtx, pCtx->pSelf);
    ASSERT_NR(rv == GFMRV_OK);
    
    // Collide it
    pCtx->flags |= gfmQT_isActive;
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
    // TODO Simply iterate through every object and add it
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
    gfmObject *pList;
    gfmRV rv;
    int len;

    /* Sanitize arguments */
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    ASSERT(pTMap, GFMRV_ARGUMENTS_BAD);

    /* Get how many areas there are */
    rv = gfmTilemap_getAreasLength(&len, pTMap);
    ASSERT_NR(rv == GFMRV_OK);
    rv = gfmTilemap_getArea(&pList, pTMap, 0);
    ASSERT_NR(rv == GFMRV_OK);

    rv = gfmHitbox_populateQuadtree((gfmHitbox*)pList, pCtx, len);
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
    int type;
    
    // Sanitize arguments
    ASSERT(ppObj1, GFMRV_ARGUMENTS_BAD);
    ASSERT(ppObj2, GFMRV_ARGUMENTS_BAD);
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    // Check that an overlap just happened
    ASSERT(pCtx->flags & gfmQT_justOverlaped, GFMRV_QUADTREE_NO_OVERLAP);
    
    // Return both objects
    gfmObject_getChild((void**)ppObj1, &type, (gfmObject*)&pCtx->object);
    gfmObject_getChild((void**)ppObj2, &type, (gfmObject*)pCtx->pOther);
    
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
    ASSERT(pCtx->flags & gfmQT_isActive, GFMRV_QUADTREE_OPERATION_NOT_ACTIVE);
    
    // Continue adding the object
    pCtx->flags &= ~gfmQT_justOverlaped;
    while (pCtx->stack.pushPos > 0 || pCtx->pColliding) {
        gfmQuadtree *pNode;
        
        // If we were colliding againts objects
        if (pCtx->pColliding) {
            gfmQuadtreeLL *pTmp;
            
            // Retrieve the current object and update the list
            pTmp = pCtx->pColliding;
            pCtx->pColliding = pCtx->pColliding->pNext;
            
            // Check if both objects overlaps
            pCtx->pOther = &pTmp->area;
            rv = gfmObject_isOverlaping((gfmObject*)&pCtx->object,
                    (gfmObject*)pCtx->pOther);
            
            // -- Exit point --
            // If they did overlap, return with that status
            if (rv == GFMRV_TRUE) {
                pCtx->flags |= gfmQT_justOverlaped;
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
                    rv = gfmQuadtree_overlap(pChild, (gfmObject*)&pCtx->object);
                    if (rv == GFMRV_TRUE) {
                        // Push it (so it will collide later)
                        rv = gfmQuadtree_pushNode(pCtx, pChild);
                        ASSERT_NR(rv == GFMRV_OK);
                    }
                    i++;
                }
            }
            else {
                // If it's static, collide against the node's children
                if (pCtx->flags & gfmQT_isStatic) {
                    pCtx->pColliding = pNode->pNodes;
                }
                // Otherwise, check if inserting the node would subdivide
                // the node (and if there's still room for that
                else if (pNode->numObjects + 1 > pCtx->maxNodes &&
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
                    rv = gfmQuadtree_insertObject(pCtx, pNode,
                            (gfmObject*)&pCtx->object);
                    ASSERT_NR(rv == GFMRV_OK);
                }
            }
        }
    }
    
    // Check if there's another object from a group
    if (pCtx->pGroupList) {
        gfmSprite *pSpr;
        
        while (pCtx->pGroupList) {
            // Retrieve the next sprite...
            rv = gfmGroup_getNextSprite(&pSpr, &(pCtx->pGroupList));
            ASSERT(rv == GFMRV_OK, rv);
            
            // Collide it!
            rv = gfmQuadtree_collideSprite(pCtx, pSpr);
            if (rv != GFMRV_QUADTREE_DONE) {
                // If this sprite didn't finish, return its status
                return rv;
            }
            // If this sprite didn't collide, go to the next
        }
    }
    
    // If the loop stoped, the operation finished
    pCtx->flags &= ~gfmQT_isActive;
    rv = GFMRV_QUADTREE_DONE;
__ret:
    return rv;
}

/**
 * Draw the quadtree to the screen.
 *
 * The object's type acts as index for the color. The first five colors are
 * reserved to represent the framework's types. It loops at gfmType_max
 * (currently 32).
 *
 * It's possible (and quite easy) to force different types to use the same
 * color. The following bitmask combination can be used:
 *
 *   27 bits for dif. entities | 5 bits for the different types
 * |---------------------------|-----|
 *
 * This way, the user may specify 27 different types, each of which may be used
 * in any of 2^27 different entities.
 * NOTE: This functions will be most likely slow!! Be careful when calling it!
 * 
 * @param  pQt     The quadtree's root
 * @param  pCtx    The game's context
 * @param  pColors The colors to be used; Must have 32 colors, each with RGB
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
        int h, w, x, y;
        
        // Pop the current node
        rv = gfmQuadtree_popNode(&pNode, pQt);
        ASSERT_NR(rv == GFMRV_OK);
        
        // Get the colors for the qt node
        pNodeColor = pColors;
        // Draw the current node
        gfmObject_getPosition(&x, &y, (gfmObject*)&pNode->area);
        gfmObject_getDimensions(&w, &h, (gfmObject*)&pNode->area);
        rv = gfm_drawRect(pCtx, x, y, w, h, pNodeColor[0], pNodeColor[1],
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
                unsigned int type;
                int height, width, x, y;
                void *pChild;

                /* The colliding object is stored within a inner object. First
                 * retrieve the inner one and then try to retrieve the type of
                 * the least-abstract (i.e., the user defined type'd) one. */
                rv = gfmObject_getChild(&pChild, (int*)&type,
                        (gfmObject*)&pTmp->area);
                ASSERT_NR(rv == GFMRV_OK);
                
                // Get the object's child
                rv = gfmObject_getChild(&pChild, (int*)&type, pChild);
                ASSERT_NR(rv == GFMRV_OK);
                if (type == gfmType_sprite) {
                    rv = gfmSprite_getChild(&pChild, (int*)&type,
                            (gfmSprite*)pChild);
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
                    type = (type % gfmType_max);
                pNodeColor = pColors + type * 3;
                
                // Get the object's position
                rv = gfmObject_getPosition(&x, &y, (gfmObject*)&pTmp->area);
                ASSERT_NR(rv == GFMRV_OK);
                // Get the object's dimensions
                rv = gfmObject_getDimensions(&width, &height,
                        (gfmObject*)&pTmp->area);
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

/**
 * List how many nodes and buckets there currently are in a quadtree
 *
 * @param  [out]pNodes   The number of nodes in all buckets (i.e., QT nodes)
 * @param  [out]pBuckets The number of sub-quadtrees
 * @param  [ in]pQt      The quadtree
 * @return               The number of nodes found
 */
int gfmQuadtree_getNumNodes(int *pNodes, int *pBuckets, gfmQuadtreeRoot *pQt) {
    gfmRV rv;
    int nodes = 0, buckets = 0;

    if (pQt == 0) {
        goto end;
    }

    // Clear the call stack
    pQt->stack.pushPos = 0;

    // Push the root node to start colliding
    rv = gfmQuadtree_pushNode(pQt, pQt->pSelf);
    if (rv != GFMRV_OK) {
        nodes = -1;
        goto end;
    }

    // Iterate through all nodes
    nodes = 0;
    buckets = 0;
    while (pQt->stack.pushPos > 0 || pQt->pColliding) {
        gfmQuadtree *pNode;

        // Pop the current node
        rv = gfmQuadtree_popNode(&pNode, pQt);
        if (rv != GFMRV_OK) {
            nodes = -1;
            goto end;
        }

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
                if (rv != GFMRV_OK) {
                    nodes = -1;
                    goto end;
                }
                i++;
            }
        }
        else {
            gfmQuadtreeLL *pTmp;

            /* Otherwise, count the number of nodes */
            pTmp = pNode->pNodes;
            while (pTmp) {
                nodes++;
                pTmp = pTmp->pNext;
            }
        }

        buckets++;
    }

end:
    if (pNodes) {
        *pNodes = nodes;
    }
    if (pBuckets) {
        *pBuckets = buckets;
    }

    return nodes;
}

