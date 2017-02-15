/**
 * @file src/gfmHitbox.c
 *
 * Define a hitbox, which is also part of objects and may be (in some
 * circunstances) used interchangebly as such.
 */
#include <GFraMe/gfmAssert.h>
#include <GFraMe/gfmError.h>
#include <GFraMe/gfmHitbox.h>
#include <GFraMe/gfmObject.h>
#include <GFraMe/gfmQuadtree.h>
#include <GFraMe_int/gfmHitbox.h>
#include <stdlib.h>
#include <string.h>

/**
 * Spawn a number of sequentially alloc'ed hitboxes
 *
 * @param  [ in]ppCtx The alloc'ed list
 * @param  [ in]count How many hitboxes should be alloc'ed
 */
gfmRV gfmHitbox_getNewList(gfmHitbox **ppCtx, int count) {
    gfmRV rv;

    ASSERT(ppCtx, GFMRV_ARGUMENTS_BAD);
    ASSERT(!(*ppCtx), GFMRV_ARGUMENTS_BAD);

    *ppCtx = (gfmHitbox*)malloc(sizeof(gfmHitbox) * count);
    ASSERT(*ppCtx, GFMRV_ALLOC_FAILED);
    memset(*ppCtx, 0x0, sizeof(gfmHitbox) * count);

    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Expand a previously alloc'ed list of hitboxes, without destroying the
 * previous one.
 *
 * @param  [ in]ppCtx    The alloc'ed list
 * @param  [ in]oldCount How many hitboxes had been alloc'ed
 * @param  [ in]newCount How many hitboxes should be alloc'ed
 */
gfmRV gfmHitbox_expandList(gfmHitbox **ppCtx, int oldCount, int newCount) {
    gfmRV rv;

    ASSERT(ppCtx, GFMRV_ARGUMENTS_BAD);

    *ppCtx = (gfmHitbox*)realloc(*ppCtx, sizeof(gfmHitbox) * newCount);
    ASSERT(*ppCtx, GFMRV_ALLOC_FAILED);
    if (newCount > oldCount) {
        memset((*ppCtx) + oldCount, 0x0, sizeof(gfmHitbox) * (newCount - oldCount));
    }

    rv = GFMRV_OK;
__ret:
    return rv;
}

/** Free either a list or a single hitbox */
gfmRV gfmHitbox_free(gfmHitbox **ppCtx) {
    if (ppCtx) {
        free(*ppCtx);
        *ppCtx = 0;
    }

    return GFMRV_OK;
}

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
        , int height, int type) {
    gfmRV rv;

    ASSERT(pObj != 0, GFMRV_ARGUMENTS_BAD);
    ASSERT(width > 0, GFMRV_ARGUMENTS_BAD);
    ASSERT(height > 0, GFMRV_ARGUMENTS_BAD);

    pObj->x = x;
    pObj->y = y;
    pObj->hw = width / 2;
    pObj->hh = height / 2;
    pObj->pContext = pCtx;
    pObj->type = type;
    pObj->innerType = gfmType_hitbox;

    rv = GFMRV_OK;
__ret:
    return rv;
}

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
        , int height, int type, int index) {
    if (pList == 0 || index < 0) {
        return GFMRV_ARGUMENTS_BAD;
    }

    return gfmHitbox_init(pList + index, pCtx, x, y, width, height, type);
}

/**
 * Populate a quadtree with a list of hitboxes
 *
 * @param  [ in]pList The list of hitboxes
 * @param  [ in]pRoot The quadtree
 * @param  [ in]count How many hitboxes there are on the list
 */
gfmRV gfmHitbox_populateQuadtree(gfmHitbox *pList, gfmQuadtreeRoot *pRoot
        , int count) {
    int i;

    i = 0;
    while (i < count) {
        gfmObject *pObj;
        gfmRV rv;

        /* Conversion from gfmHitbox to gfmObject is valid if only the first
         * field (a gfmHitbox) from the object will be used */
        pObj = (gfmObject*)(pList + i);
        rv = gfmQuadtree_populateObject(pRoot, pObj);
        if (rv != GFMRV_OK) {
            return rv;
        }

        i++;
    }

    return GFMRV_OK;
}

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
        , gfmQuadtreeRoot *pRoot, int last) {
    int i;

    i = *pFirst;
    while (i <= last) {
        gfmObject *pObj;
        gfmRV rv;

        /* Conversion from gfmHitbox to gfmObject is valid if only the first
         * field (a gfmHitbox) from the object will be used */
        pObj = (gfmObject*)(pList + i);
        i++;

        rv = gfmQuadtree_collideObject(pRoot, pObj);
        if (rv != GFMRV_QUADTREE_DONE) {
            *pFirst = i;
            return rv;
        }
    }

    *pFirst = i;
    return GFMRV_QUADTREE_DONE;
}

