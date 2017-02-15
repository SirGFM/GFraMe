/**
 * @file include/GFraMe/gfmHitbox.h
 *
 * Define a hitbox, which is also part of objects and may be (in some
 * circunstances) used interchangebly as such.
 */
#ifndef __GFMHITBOX_STRUCT__
#define __GFMHITBOX_STRUCT__

typedef struct stGFMHitbox gfmHitbox;

#endif /* __GFMHITBOX_STRUCT__ */

#ifndef __GFMHITBOX_H__
#define __GFMHITBOX_H__

#include <GFraMe/gfmError.h>
#include <GFraMe/gfmQuadtree.h>

/**
 * Spawn a number of sequentially alloc'ed hitboxes
 *
 * @param  [ in]ppCtx The alloc'ed list
 * @param  [ in]count How many hitboxes should be alloc'ed
 */
gfmRV gfmHitbox_getNewList(gfmHitbox **ppCtx, int count);

/** Free either a list or a single hitbox */
gfmRV gfmHitbox_free(gfmHitbox **ppCtx);

/**
 * Initialize a single hitbox within a list
 *
 * @param  [ in]pObj   The hitbox
 * @param  [ in]pCtx   The context to be stored within the hitbox (e.g., an
 *                     associated object, or level data)
 * @param  [ in]x      The hitbox's top-left corner position
 * @param  [ in]y      The hitbox's top-left corner position
 * @param  [ in]width  The hitbox's dimension
 * @param  [ in]height The hitbox's dimension
 * @param  [ in]type   The type of the hitbox's context
 */
gfmRV gfmHitbox_init(gfmHitbox *pObj, void *pCtx, int x, int y, int width
        , int height, int type);

/**
 * Initialize a single hitbox within a list
 *
 * @param  [ in]pList  The list of hitboxes
 * @param  [ in]pCtx   The context to be stored within the hitbox (e.g., an
 *                     associated object, or level data)
 * @param  [ in]x      The hitbox's top-left corner position
 * @param  [ in]y      The hitbox's top-left corner position
 * @param  [ in]width  The hitbox's dimension
 * @param  [ in]height The hitbox's dimension
 * @param  [ in]type   The type of the hitbox's context
 * @param  [ in]index  The index of the hitbox within the list
 */
gfmRV gfmHitbox_initItem(gfmHitbox *pList, void *pCtx, int x, int y, int width
        , int height, int type, int index);

/**
 * Populate a quadtree with a list of hitboxes
 *
 * @param  [ in]pList The list of hitboxes
 * @param  [ in]pRoot The quadtree
 * @param  [ in]count How many hitboxes there are on the list
 */
gfmRV gfmHitbox_populateQuadtree(gfmHitbox *pList, gfmQuadtreeRoot *pRoot
        , int count);

/**
 * Collide a sub-list of hitboxes with a quadtree
 *
 * This function should be used to collide one-frame hitboxes that were spawned
 * after the first call to gfmHitbox_populateQuadtree was made on the same
 * frame. It has the same return as gfmQuadtree_collideObject, which means that
 * it will halt execution as soon as a hitbox overlaps another node.
 *
 * pFirst is updated with the index when the function exits.
 *
 * Note that similar to gfmQuadtree_collideObject, it returns
 * GFMRV_QUADTREE_DONE when done.
 *
 * @param  [in/out]pFirst Index of the first hitbox to be collided
 * @param  [    in]pList  The list of hitboxes
 * @param  [    in]pRoot  The quadtree
 * @param  [    in]last   Index of the last hitbox to be collided
 */
gfmRV gfmHitbox_collideSubList(int *pFirst, gfmHitbox *pList
        , gfmQuadtreeRoot *pRoot, int last);

#endif /* __GFMHITBOX_H__ */

