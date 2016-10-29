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
#ifndef __GFMQUADTREE_STRUCT__
#define __GFMQUADTREE_STRUCT__

/** Quadtree's context, with the current stack and the the root node */
typedef struct stGFMQuadtreeRoot gfmQuadtreeRoot;

#endif /* __GFMQUADTREE_STRUCT__ */

#ifndef __GFMQUADTREE_H__
#define __GFMQUADTREE_H__

#include <GFraMe/gfmError.h>
#include <GFraMe/gfmGroup.h>
#include <GFraMe/gfmObject.h>
#include <GFraMe/gfmSprite.h>
#include <GFraMe/gfmTilemap.h>

/**
 * Alloc a new root quadtree
 * 
 * @param  ppCtx The root quadtree
 * @return       GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_ALLOC_FAILED
 */
gfmRV gfmQuadtree_getNew(gfmQuadtreeRoot **ppCtx);

/**
 * Release a quadtree's root and all its members
 * 
 * @param  ppCtx The quadtree root
 * @return       GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmQuadtree_free(gfmQuadtreeRoot **ppCtx);

/**
 * Clean all memory used by the entire quadtree
 * 
 * @param  pCtx The quadtree root
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
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
        int height, int maxDepth, int maxNodes);

/**
 * Make the quadtree static
 *
 * @param  [ in]pCtx The quadtree's root
 * @return      GFMRV_ARGUMENTS_BAD, GFMRV_QUADTREE_NOT_INITIALIZED, GFMRV_OK
 */
gfmRV gfmQuadtree_setStatic(gfmQuadtreeRoot *pCtx);

/**
 * Adds a new gfmGroup to the quadtree, subdividing it as necessary and
 * colliding with every possible node
 * 
 * @param  pCtx The quadtree's root
 * @param  pGrp The gfmGroup
 * @return      GFMRV_ARGUMENTS_BAD, GFMRV_QUADTREE_NOT_INITIALIZED, 
 *              GFMRV_QUADTREE_OVERLAPED, GFMRV_QUADTREE_DONE
 */
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
gfmRV gfmQuadtree_collideObject(gfmQuadtreeRoot *pCtx, gfmObject *pObj);

/**
 * Adds a new gfmSprite to the quadtree, subdividing it as necessary and
 * colliding with every possible node
 * 
 * @param  pCtx The quadtree's root
 * @param  pObj The gfmObject
 * @return      GFMRV_ARGUMENTS_BAD, GFMRV_QUADTREE_NOT_INITIALIZED, 
 *              GFMRV_QUADTREE_OVERLAPED, GFMRV_QUADTREE_DONE
 */
gfmRV gfmQuadtree_collideSprite(gfmQuadtreeRoot *pCtx, gfmSprite *pSpr);

/**
 * Adds a new gfmTilemap to the quadtree, subdividing it as necessary and
 * colliding with every possible node
 * 
 * @param  pCtx  The quadtree's root
 * @param  pTMap The gfmTilemap
 * @return       GFMRV_ARGUMENTS_BAD, GFMRV_QUADTREE_NOT_INITIALIZED, 
 *               GFMRV_QUADTREE_OVERLAPED, GFMRV_QUADTREE_DONE
 */
gfmRV gfmQuadtree_collideTilemap(gfmQuadtreeRoot *pCtx, gfmTilemap *pTMap);

/**
 * Populates a quadtree with a group's objects
 * 
 * @param  pCtx The quadtree's root
 * @param  pGrp The tilemap
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_QUADTREE_NOT_INITIALIZED
 */
gfmRV gfmQuadtree_populateGroup(gfmQuadtreeRoot *pCtx, gfmGroup *pGrp);

/**
 * Add an object to the quadtree without collinding it against the tree's objs
 * 
 * @param  pCtx The quadtree's root
 * @param  pObj The gfmObject
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_QUADTREE_NOT_INITIALIZED
 */
gfmRV gfmQuadtree_populateObject(gfmQuadtreeRoot *pCtx, gfmObject *pObj);

/**
 * Add a sprite to the quadtree without collinding it against the tree's objs
 * 
 * @param  pCtx The quadtree's root
 * @param  pSpr The gfmSprite
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_QUADTREE_NOT_INITIALIZED
 */
gfmRV gfmQuadtree_populateSprite(gfmQuadtreeRoot *pCtx, gfmSprite *pSpr);

/**
 * Populates a quadtree with tilemap's areas
 * 
 * @param  pCtx  The quadtree's root
 * @param  pTMap The tilemap
 * @return       GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_QUADTREE_NOT_INITIALIZED
 */
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
        gfmQuadtreeRoot *pCtx);

/**
 * Continue colliding and adding the node to the quadtree
 * 
 * @param  pCtx The quadtree's root
 * @return      GFMRV_ARGUMENTS_BAD, GFMRV_QUADTREE_OPERATION_NOT_ACTIVE,
 *              GFMRV_QUADTREE_OVERLAPED, GFMRV_QUADTREE_DONE
 */
gfmRV gfmQuadtree_continue(gfmQuadtreeRoot *pCtx);

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
        unsigned char *pColors);

#endif /* __GFMQUADTREE_H__ */

