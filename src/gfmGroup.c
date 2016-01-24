/**
 * @file include/GFraMe/gfmGroup.h
 *
 * Manages a list of sprites, automatically updating and drawing everything;
 * This module's name may be somewhat misleading, as it mostly about "things the
 * spawns, dies and are later recycled" (i.e., a particle system), but its
 * original objective was somewhat different and maybe, someday, it will be
 * changed... So, yeah... bear with it.
 * This module will try to draw its sprite in a batch (if available by the
 * backend); If another batch was already in execution, instead of finishing the
 * previous and starting a new one, it simply keeps drawing to that previous
 * batch;
 * There are four methods for drawing the sprites:
 *   - linear: Draw sprites in the order they appear in the buffer
 *   - topFirst: First draw the sprites from top to bottom
 *   - bottomFirst: First draw the sprites from bottom to top
 *   - newestFirst: Draw from the newest to the oldest sprites
 *   - oldestFirst: Draw from the oldest to the newest sprites
 * Updating is done by simply iterating through every object sprite and calling
 * its update function; It's possible to set a these default values that will be
 * set on any new sprite:
 *   - Default texture
 *   - Animation data (i.e., a array of int describing the animations)
 *   - The sprite's hitbox
 *   - Maximum time alive
 *   - Whether should die on leaving the screen
 *   - Velocity
 *   - Acceleration (useful for gravity; e.g. accX = 0px/s^2, accY = 500px/s^2)
 * Also, when a new sprite is added (either by adding it or recycling it), it's
 * possible to set those values:
 *   - Position
 *   - Current animation
 *   - Velocity
 *   - Acceleration
 */
#include <GFraMe/gframe.h>
#include <GFraMe/gfmAssert.h>
#include <GFraMe/gfmCamera.h>
#include <GFraMe/gfmError.h>
#include <GFraMe/gfmGenericArray.h>
#include <GFraMe/gfmGroup.h>
#include <GFraMe/gfmObject.h>
#include <GFraMe/gfmSprite.h>
#include <GFraMe/gfmTypes.h>
#include <GFraMe_int/gfmGroupHelpers.h>

#include <stdlib.h>
#include <string.h>

/** Define an array for the nodes */
gfmGenArr_define(gfmGroupNode);

/** Store all configuration for new/recycled instances */
struct stGFMGroupConf {
    /** Default spriteset; assigned to every sprite *ON ALLOCATION* */
    gfmSpriteset *pDefSset;
    /** Default animation; assigned to every sprite *ON ALLOCATION* */
    int *pDefAnimData;
    /** How many objects the group handles, at most */
    int maxLen;
    /** For how long the sprites should live; 'gfmGroup_keepAlive' disables
        this */
    int ttl;
    /** Default type; assigned to every sprite *ON ALLOCATION* */
    int defType;
    /** Default anim buffer len; assigned to every sprite *ON ALLOCATION* */
    int defAnimLen;
    /** Default width; assigned to every sprite *ON ALLOCATION* */
    int defWidth;
    /** Default height; assigned to every sprite *ON ALLOCATION* */
    int defHeight;
    /** Default horizontal offset; assigned to every sprite *ON ALLOCATION* */
    int defOffX;
    /** Default vertical offset; assigned to every sprite *ON ALLOCATION* */
    int defOffY;
    /** Default horizontal velocity */
    int defVx;
    /** Default vertical velocity */
    int defVy;
    /** Default horizontal acc */
    int defAx;
    /** Default vertical acc */
    int defAy;
};
typedef struct stGFMGroupConf gfmGroupConf;

/**
 * Struct used to ease sorting/drawing
 *
 * All visible nodes are sequentially stored in pDrawArr. So, if a binary tree
 * is built from its elements, its root will be the first element.
 */
struct stGFMGroupDrawCtx {
    /** Array with all draw elements */
    gfmGroupDrawNode *pDrawArr;
    /** Current draw order */
    gfmDrawOrder drawOrder;
    /** Total number of elements on the array */
    int totalElements;
    /** Number of elements to be drawn */
    int usedElements;
};
typedef struct stGFMGroupDrawCtx gfmGroupDrawCtx;

/** The gfmGroup structure */
struct stGFMGroup {
    /** This group's configurations */
    gfmGroupConf *pConf;
    /** This group's draw context */
    gfmGroupDrawCtx *pDrawCtx;
    /** Array with every group node */
    gfmGenArr_var(gfmGroupNode, pNodes);

    /** List of currently active nodes */
    gfmGroupNode *pActive;
    /** List of currently inactive nodes */
    gfmGroupNode *pInactive;
    /** List of currently collideable nodes */
    gfmGroupNode *pCollideable;
    /** Collision quality when using quadtrees */
    gfmGroupCollision collisionQuality;
    /** Points to the last retrieved sprite */
    gfmSprite *pLast;
    /** How many sprites have been skipped */
    int skippedCollision;
    /** Whether should die on leaving the screen */
    int dieOnLeave;
};

/**
 * Alloc a new group
 *
 * @param  ppCtx The group
 * @return       GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_ALLOC_FAILED
 */
gfmRV gfmGroup_getNew(gfmGroup **ppCtx) {
    gfmGroup *pCtx;
    gfmRV rv;
    int len;

    /* Sanitize arguments */
    ASSERT(ppCtx, GFMRV_ARGUMENTS_BAD);
    ASSERT(!(*ppCtx), GFMRV_ARGUMENTS_BAD);

    /* Calculate the required number of bytes to alloc all structs at once,
     * while respecting alignment */
    len = sizeof(gfmGroup) + sizeof(gfmGroup) % ALIGN;
    len += sizeof(gfmGroupConf) + sizeof(gfmGroupConf) % ALIGN;
    len += sizeof(gfmGroupDrawCtx) + sizeof(gfmGroupDrawCtx) % ALIGN;

    /* Alloc the object */
    pCtx = (gfmGroup*)malloc(len);
    ASSERT(pCtx, GFMRV_ALLOC_FAILED);
    /* Initialize it */
    memset(pCtx, 0x00, len);

    /* Retrieve the gfmGroupConf within the gfmGroup */
    len = sizeof(gfmGroup) + sizeof(gfmGroup) % ALIGN;
    pCtx->pConf = (gfmGroupConf*)(((char*)pCtx) + len);
    /* Set the default time to live (since it's not 0) */
    pCtx->pConf->ttl = gfmGroup_keepAlive;

    /* Retrieve the gfmGroupDrawCtx within the gfmGroup */
    len += sizeof(gfmGroupConf) + sizeof(gfmGroupConf) % ALIGN;
    pCtx->pDrawCtx = (gfmGroupDrawCtx*)(((char*)pCtx) + len);

    *ppCtx = pCtx;
    rv = GFMRV_OK;
__ret:

    return rv;
}

/**
 * Frees a previously alloc'ed group (and any of its memory)
 *
 * @param  ppCtx The group
 * @return       GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmGroup_free(gfmGroup **ppCtx) {
    gfmRV rv;

    /* Sanitize arguments */
    ASSERT(ppCtx, GFMRV_ARGUMENTS_BAD);
    ASSERT(*ppCtx, GFMRV_ARGUMENTS_BAD);

    /* Clean up the group */
    rv = gfmGroup_clean(*ppCtx);
    ASSERT_NR(rv == GFMRV_OK);

    /* Free the memory */
    free(*ppCtx);
    *ppCtx = 0;

    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Pre cache the group; Instantiate and initialize a few sprites; The sprite's
 * default attributes must be set before calling this function, as to correctly
 * initialize the sprites
 *
 * @param  pCtx     The group
 * @param  initLen  How many nodes should be pre-allocated (it may be 0)
 * @param  maxLen   Group's maximum length (0 if there's no limit)
 * @return          GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_ALLOC_FAILED
 */
gfmRV gfmGroup_preCache(gfmGroup *pCtx, int initLen, int maxLen) {
    gfmRV rv;

    // Sanitize arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    ASSERT(initLen >= 0, GFMRV_ARGUMENTS_BAD);
    ASSERT(maxLen >= 0, GFMRV_ARGUMENTS_BAD);
    // Check if it was already initialized
    ASSERT(pCtx->pConf->pDefSset, GFMRV_GROUP_SPRITESET_NOT_SET);
    ASSERT(pCtx->pConf->defWidth > 0, GFMRV_GROUP_WIDTH_NOT_SET);
    ASSERT(pCtx->pConf->defHeight > 0, GFMRV_GROUP_HEIGHT_NOT_SET);
    // Check that the initial number of sprites is valid
    ASSERT(maxLen == 0 || initLen <= maxLen, GFMRV_GROUP_MAX_SPRITES);

    // Set the group's max len
    pCtx->pConf->maxLen = maxLen;
    // If requested, pre-cache some sprites
    if (initLen > 0) {
        rv = gfmGroup_cacheSprites(pCtx, initLen);
        ASSERT_NR(rv == GFMRV_OK);
    }

    // Clean the active list
    pCtx->pActive = 0;

    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Cache more sprite
 *
 * @param  pCtx The group
 * @param  num  How many new sprites should be cached
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_ALLOC_FAILED,
 *              GFMRV_GROUP_MAX_SPRITES
 */
gfmRV gfmGroup_cacheSprites(gfmGroup *pCtx, int num) {
    gfmRV rv;
    int newLen, pos, i;

    // Sanitize arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    ASSERT(num > 0, GFMRV_ARGUMENTS_BAD);
    // Check if it was already initialized
    ASSERT(pCtx->pConf->pDefSset, GFMRV_GROUP_SPRITESET_NOT_SET);
    ASSERT(pCtx->pConf->defWidth > 0, GFMRV_GROUP_WIDTH_NOT_SET);
    ASSERT(pCtx->pConf->defHeight > 0, GFMRV_GROUP_HEIGHT_NOT_SET);
    // Check if the buffer can be expanded
    pos = gfmGenArr_getUsed(pCtx->pNodes);
    newLen = pos + num;
    ASSERT(pCtx->pConf->maxLen == 0 || newLen <= pCtx->pConf->maxLen,
            GFMRV_GROUP_MAX_SPRITES);

    // Expand the nodes buffer
    gfmGenArr_setMinSize(gfmGroupNode, pCtx->pNodes, pos + num,
            gfmGroupNode_getNew);

    // Initialize every new sprite
    i = pos;
    while (i < newLen) {
        gfmGroupNode *pNode;
        gfmGroupNode *pNextNode;

        // Retrieve a node
        gfmGenArr_getNextRef(gfmGroupNode, pCtx->pNodes, 1/*inc*/, pNode,
                gfmGroupNode_getNew);
        gfmGenArr_push(pCtx->pNodes);

        // Alloc its sprite (and set it for auto dealloc'ing)
        pNode->autoFree = 1;
        rv = gfmSprite_getNew(&(pNode->pSelf));
        ASSERT_NR(rv == GFMRV_OK);
        // Initialize the sprite
        rv = gfmSprite_init(pNode->pSelf, 0/*x*/, 0/*y*/, pCtx->pConf->defWidth,
                pCtx->pConf->defHeight, pCtx->pConf->pDefSset,
                pCtx->pConf->defOffX, pCtx->pConf->defOffY, pNode/*child*/,
                pCtx->pConf->defType);
        ASSERT_NR(rv == GFMRV_OK);
        // Load the animation data
        if (pCtx->pConf->pDefAnimData) {
            rv = gfmSprite_addAnimations(pNode->pSelf,
                    pCtx->pConf->pDefAnimData, pCtx->pConf->defAnimLen);
        ASSERT_NR(rv == GFMRV_OK);
        }

        if (i < newLen - 1) {
            // Get the next node, if any
            pNextNode = gfmGenArr_getObject(pCtx->pNodes, i + 1);
        }
        else {
            // Otherwise, prepend this new list to the inactive list
            pNextNode = pCtx->pInactive;
        }

        // Set the next node in the list
        pNode->pNext = pNextNode;

        i++;
    }

    // Inactive list must point to the first of the newly allocated nodes
    pCtx->pInactive = gfmGenArr_getObject(pCtx->pNodes, pos);

    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Clean up the group and all of its member
 *
 * @param  pCtx The group
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmGroup_clean(gfmGroup *pCtx) {
    gfmRV rv;

    /* Sanitize arguments */
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);

    /* Free every node on the list */
    gfmGenArr_clean(pCtx->pNodes, gfmGroupNode_free);

    /* Free the nodes from the draw context */
    if (pCtx->pDrawCtx->pDrawArr) {
        free(pCtx->pDrawCtx->pDrawArr);
    }

    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Insert a sprite into the group; One can add "super-classes-sprites" to the
 * group (e.g., a player's sprite), but care should be taken about who frees
 * what
 *
 * @param  pCtx     The group
 * @param  pSpr     The sprite
 * @param  autoFree Whether the sprite should be automatically freed
 * @return          GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_ALLOC_FAILED
 */
gfmRV gfmGroup_insert(gfmGroup *pCtx, gfmSprite *pSpr, int autoFree) {
    // TODO gfmGroup_insert
    return GFMRV_FUNCTION_NOT_IMPLEMENTED;
}

/**
 * Reuse an inactive sprite; If none is found, a new one is allocated
 *
 * @param  ppSpr The sprite
 * @param  pCtx  The group
 * @return       GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_ALLOC_FAILED,
 *               GFMRV_GROUP_MAX_SPRITES
 */
gfmRV gfmGroup_recycle(gfmSprite **ppSpr, gfmGroup *pCtx) {
    gfmRV rv;
    gfmGroupNode *pTmp;

    // Sanitize arguments
    ASSERT(ppSpr, GFMRV_ARGUMENTS_BAD);
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);

    // Check if the inactive list is empty
    if (!pCtx->pInactive) {
        int curLen, newLen;

        // Check that there's space for more sprites
        ASSERT(pCtx->pConf->maxLen == 0
                || gfmGenArr_getUsed(pCtx->pNodes) < pCtx->pConf->maxLen,
                GFMRV_GROUP_MAX_SPRITES);

        // Get the new length of the buffer
        curLen = gfmGenArr_getUsed(pCtx->pNodes);
        if (curLen == 0) {
            // If the list was empty, initialize it
            newLen = 1;
        }
        else if (pCtx->pConf->maxLen > 0 && curLen * 2 > pCtx->pConf->maxLen) {
            // If at the limit, add as many sprites as possible
            newLen = pCtx->pConf->maxLen - curLen;
        }
        else {
            // If possible, double the size
            newLen = curLen;
        }

        // Expand the buffer and alloc its sprites
        rv = gfmGroup_cacheSprites(pCtx, newLen);
        ASSERT_NR(rv == GFMRV_OK);
    }
    // Remove the current node from the list
    pTmp = pCtx->pInactive;
    pCtx->pInactive = pCtx->pInactive->pNext;

    // Reset the node's timer
    pTmp->timeAlive = pCtx->pConf->ttl;

    // Set the sprite's default values
    rv = gfmSprite_resetObject(pTmp->pSelf);
    ASSERT_NR(rv == GFMRV_OK);
    rv = gfmSprite_setVelocity(pTmp->pSelf, pCtx->pConf->defVx,
            pCtx->pConf->defVy);
    ASSERT_NR(rv == GFMRV_OK);
    rv = gfmSprite_setAcceleration(pTmp->pSelf, pCtx->pConf->defAx,
            pCtx->pConf->defAy);
    ASSERT_NR(rv == GFMRV_OK);

    // Insert it at the begining of the active list
    pTmp->pNext = pCtx->pActive;
    pCtx->pActive = pTmp;

    // Store this last retrieved sprite so it can be modified later
    pCtx->pLast = pTmp->pSelf;

    *ppSpr = pTmp->pSelf;
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Set the default type on every recycled sprite
 *
 * @param  pCtx The group
 * @param  type The type
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD, ...
 */
gfmRV gfmGroup_setDefType(gfmGroup *pCtx, int type) {
    gfmRV rv;

    // Sanitize arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    ASSERT(type >= 0, GFMRV_ARGUMENTS_BAD);

    // Set the default spriteset
    pCtx->pConf->defType = type;

    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Set the default spriteset on every recycled sprite
 *
 * @param  pCtx  The group
 * @param  pSset The spriteset
 * @return       GFMRV_OK, GFMRV_ARGUMENTS_BAD, ...
 */
gfmRV gfmGroup_setDefSpriteset(gfmGroup *pCtx, gfmSpriteset *pSset) {
    gfmRV rv;

    // Sanitize arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    ASSERT(pSset, GFMRV_ARGUMENTS_BAD);

    // Set the default spriteset
    pCtx->pConf->pDefSset = pSset;

    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Set the default animation data on every recycled sprite
 *
 * NOTE:  The user must keep this buffer in memory! (i.e., it's not copied)
 * NOTE2: This should be called before gfmGroup_init, as to correctly set all
 *        sprites' animations only once
 *
 * @param  pCtx  The group
 * @param  pData The animation data (same format as gfmSprite_addAnimations)
 * @param  len   The number of ints in the pData array
 * @return       GFMRV_OK, GFMRV_ARGUMENTS_BAD, ...
 */
gfmRV gfmGroup_setDefAnimData(gfmGroup *pCtx, int *pData, int len) {
    gfmRV rv;

    // Sanitize arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    ASSERT(pData, GFMRV_ARGUMENTS_BAD);
    ASSERT(len > 0, GFMRV_ARGUMENTS_BAD);

    // Set the default animation data (simply copy the pointer!)
    pCtx->pConf->pDefAnimData = pData;
    pCtx->pConf->defAnimLen = len;

    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Set the default hitbox and offset on every recycled sprite
 *
 * @param  pCtx   The group
 * @param  width  The sprite's width (if it's to be collided)
 * @param  height The sprite's height (if it's to be collided)
 * @param  offX   Sprite (i.e., 'image') offset from the object's position
 * @param  offY   Sprite (i.e., 'image') offset from the object's position
 * @return        GFMRV_OK, GFMRV_ARGUMENTS_BAD, ...
 */
gfmRV gfmGroup_setDefDimensions(gfmGroup *pCtx, int width, int height, int offX,
        int offY) {
    gfmRV rv;

    // Sanitize arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);

    // Set the default hitbox and offset
    pCtx->pConf->defWidth = width;
    pCtx->pConf->defHeight = height;
    pCtx->pConf->defOffX = offX;
    pCtx->pConf->defOffY = offY;

    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Set the default velocity on every recycled sprite
 *
 * @param  pCtx The group
 * @param  vx   The horizontal velocity
 * @param  vy   The vertical velocity
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD, ...
 */
gfmRV gfmGroup_setDefVelocity(gfmGroup *pCtx, int vx, int vy) {
    gfmRV rv;

    // Sanitize arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);

    // Set the default velocity
    pCtx->pConf->defVx = vx;
    pCtx->pConf->defVy = vy;

    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Set the default acceleration on every recycled sprite
 *
 * @param  pCtx The group
 * @param  ax   The horizontal acceleration
 * @param  ay   The vertical acceleration
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD, ...
 */
gfmRV gfmGroup_setDefAcceleration(gfmGroup *pCtx, int ax, int ay) {
    gfmRV rv;

    // Sanitize arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);

    // Set the default acceleration
    pCtx->pConf->defAx = ax;
    pCtx->pConf->defAy = ay;

    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Set whether every recycle sprite should 'die' when it leaves the screen
 *
 * @param  pCtx  The group
 * @param  doDie Whether the recycled sprites should die or not
 * @return       GFMRV_OK, GFMRV_ARGUMENTS_BAD, ...
 */
gfmRV gfmGroup_setDeathOnLeave(gfmGroup *pCtx, int doDie) {
    gfmRV rv;

    // Sanitize arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);

    // Set whether should die when leaving the screen
    pCtx->dieOnLeave = doDie;

    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Set for how long every recycled sprite should live
 *
 * @param  pCtx The group
 * @param  ttl  Time to live (-1 for infinite)
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD, ...
 */
gfmRV gfmGroup_setDeathOnTime(gfmGroup *pCtx, int ttl) {
    gfmRV rv;

    // Sanitize arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);

    // Small fix for 'infinite magic number'
    if (ttl == -1) {
        ttl = gfmGroup_keepAlive;
    }
    // Set the default "time to live"
    pCtx->pConf->ttl = ttl;

    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Set the position of the last recycled/added sprite
 *
 * @param  pCtx The group
 * @param  x    Horizontal position
 * @param  y    Vertical position
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD, ...
 */
gfmRV gfmGroup_setPosition(gfmGroup *pCtx, int x, int y) {
    gfmRV rv;

    // Sanitize arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    // Check that there is a previous node
    ASSERT(pCtx->pLast, GFMRV_GROUP_NO_LAST_SPRITE);

    // Set the sprite's position
    rv = gfmSprite_setPosition(pCtx->pLast, x, y);
    ASSERT_NR(rv == GFMRV_OK);

    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Set the frame of the last recycled/added sprite
 *
 * @param  pCtx  The group
 * @param  frame The frame
 * @return       GFMRV_OK, GFMRV_ARGUMENTS_BAD, ...
 */
gfmRV gfmGroup_setFrame(gfmGroup *pCtx, int frame) {
    gfmRV rv;

    // Sanitize arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    // Check that there is a previous node
    ASSERT(pCtx->pLast, GFMRV_GROUP_NO_LAST_SPRITE);

    // Set the frame
    rv = gfmSprite_setFrame(pCtx->pLast, frame);
    ASSERT_NR(rv == GFMRV_OK);

    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Set the animation of the last recycled/added sprite
 *
 * @param  pCtx The group
 * @param  anim The animation's index
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD, ...
 */
gfmRV gfmGroup_setAnimation(gfmGroup *pCtx, int anim) {
    gfmRV rv;

    // Sanitize arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    // Check that there is a previous node
    ASSERT(pCtx->pLast, GFMRV_GROUP_NO_LAST_SPRITE);

    // Play the animation
    rv = gfmSprite_playAnimation(pCtx->pLast, anim);
    ASSERT_NR(rv == GFMRV_OK);
    rv = gfmSprite_resetAnimation(pCtx->pLast);
    ASSERT_NR(rv == GFMRV_OK);

    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Set the velocity of the last recycled/added sprite
 *
 * @param  pCtx The group
 * @param  vx   The horizontal velocity
 * @param  vy   The vertical velocity
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD, ...
 */
gfmRV gfmGroup_setVelocity(gfmGroup *pCtx, int vx, int vy) {
    gfmRV rv;

    // Sanitize arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    // Check that there is a previous node
    ASSERT(pCtx->pLast, GFMRV_GROUP_NO_LAST_SPRITE);

    // Set the sprite's velocity
    rv = gfmSprite_setVelocity(pCtx->pLast, vx, vy);
    ASSERT_NR(rv == GFMRV_OK);

    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Set the acceleration of the last recycled/added sprite
 *
 * @param  pCtx The group
 * @param  ax   The horizontal acceleration
 * @param  ay   The vertical acceleration
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD, ...
 */
gfmRV gfmGroup_setAcceleration(gfmGroup *pCtx, int ax, int ay) {
    gfmRV rv;

    // Sanitize arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    // Check that there is a previous node
    ASSERT(pCtx->pLast, GFMRV_GROUP_NO_LAST_SPRITE);

    // Set the sprite's acceleration
    rv = gfmSprite_setAcceleration(pCtx->pLast, ax, ay);
    ASSERT_NR(rv == GFMRV_OK);

    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Set the type of the last recycled/added sprite
 *
 * WARNING: If this function is called even once, you'll have to call it after
 * recycling any sprite. Otherwise, there's no guarantee about the recycled
 * sprite's type. (unless you never "kill/release" the only sprite that had its
 * type modified...)
 *
 * @param  [ in]pCtx The group
 * @param  [ in]type The new type
 * @return           GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_GROUP_NO_LAST_SPRITE
 */
gfmRV gfmGroup_setType(gfmGroup *pCtx, int type) {
    gfmRV rv;

    /* Sanitize arguments */
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    /* Check that there is a previous node */
    ASSERT(pCtx->pLast, GFMRV_GROUP_NO_LAST_SPRITE);

    /* Set the sprite's new type */
    rv = gfmSprite_setType(pCtx->pLast, type);
    ASSERT(rv == GFMRV_OK, rv);

    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Set the group's draw order
 *
 * @param  pCtx  The group
 * @param  order The new draw order
 * @return       GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_GROUP_INVALID_TYPE
 */
gfmRV gfmGroup_setDrawOrder(gfmGroup *pCtx, gfmDrawOrder order) {
    gfmRV rv;

    // Sanitize arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    ASSERT(order >= 0, GFMRV_ARGUMENTS_BAD);
    // Check that it's a valid value
    ASSERT(order < gfmDrawOrder_max, GFMRV_GROUP_INVALID_TYPE);

    pCtx->pDrawCtx->drawOrder = order;

    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Set the collision mode
 *
 * @param  pCtx The group
 * @param  col  The new collision quality
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_GROUP_INVALID_TYPE
 */
gfmRV gfmGroup_setCollisionQuality(gfmGroup *pCtx, gfmGroupCollision col) {
    gfmRV rv;

    // Sanitize arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    ASSERT(col >= 0, GFMRV_ARGUMENTS_BAD);
    // Check that it's a valid value
    ASSERT(col < gfmCollisionQuality_max, GFMRV_GROUP_INVALID_TYPE);

    pCtx->collisionQuality = col;

    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Get the list of collideable objects
 *
 * @param  ppList The list of nodes
 * @param  pCtx   The group
 * @return        GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_GROUP_LIST_EMPTY
 */
gfmRV gfmGroup_getCollideableList(gfmGroupNode **ppList, gfmGroup *pCtx) {
    gfmRV rv;

    // Sanitize the arguments
    ASSERT(ppList, GFMRV_ARGUMENTS_BAD);
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    // Check that the list isn't empty
    if (pCtx->pCollideable == 0) {
        rv = GFMRV_GROUP_LIST_EMPTY;
        goto __ret;
    }

    // Retrieve the list
    *ppList = pCtx->pCollideable;

    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Get the next collideable sprite
 *
 * @param  ppSpr  The retrieved sprite
 * @param  ppList The list of nodes (passed as ref. to be updated)
 * @return        GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmGroup_getNextSprite(gfmSprite **ppSpr, gfmGroupNode **ppList) {
    gfmRV rv;

    // Sanitize arguments
    ASSERT(ppSpr, GFMRV_ARGUMENTS_BAD);
    ASSERT(ppList, GFMRV_ARGUMENTS_BAD);

    // Retrieve the current node's sprite
    *ppSpr = (*ppList)->pSelf;
    // Update the list
    *ppList = (*ppList)->pNextCollideable;

    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Check whether a node is alive (checks based on the time!!)
 *
 * @param  pCtx The node
 * @return      GFMRV_TRUE, GFMRV_FALSE, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmGroup_isNodeAlive(gfmGroupNode *pCtx) {
    gfmRV rv;

    // Sanitize arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);

    // Check if the node is still alive
    if (pCtx->timeAlive != gfmGroup_forceKill) {
        rv = GFMRV_TRUE;
    }
    else {
        rv = GFMRV_FALSE;
    }
__ret:
    return rv;
}

/**
 * Force a node to be removed on the next update
 *
 * @param  pCtx The node
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmGroup_removeNode(gfmGroupNode *pCtx) {
    gfmRV rv;

    /* Sanitize arguments */
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);

    /* Force the node to be removed */
    pCtx->timeAlive = gfmGroup_forceKill;

    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Iterate through every sprite and update'em
 *
 * @param  pGrp The group
 * @param  pCtx The game's context
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmGroup_update(gfmGroup *pGroup, gfmCtx *pCtx) {
    gfmCamera *pCam;
    gfmGroupDrawCtx *pDrawCtx;
    gfmGroupNode *pTmp;
    gfmGroupNode *pPrev;
    gfmRV rv;
    int elapsed;

    /* Sanitize arguments */
    ASSERT(pGroup, GFMRV_ARGUMENTS_BAD);
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);

    pDrawCtx = pGroup->pDrawCtx;

    /* Remove the reference to the last recycled/added sprite */
    pGroup->pLast = 0;
    /* Reset the list of visible sprites */
    pDrawCtx->usedElements = 0;
    /* Reset the list of collideable sprites */
    pGroup->pCollideable = 0;
    pGroup->skippedCollision = 0;
    /* Retrieve the current camera */
    rv = gfm_getCamera(&pCam, pCtx);
    ASSERT_NR(rv == GFMRV_OK);

    /* Make sure the draw context is big enough */
    if (pDrawCtx->totalElements < gfmGenArr_getUsed(pGroup->pNodes)) {
        pDrawCtx->totalElements = gfmGenArr_getUsed(pGroup->pNodes);
        pDrawCtx->pDrawArr = (gfmGroupDrawNode*)realloc(pDrawCtx->pDrawArr,
                sizeof(gfmGroupDrawNode) * pDrawCtx->totalElements);
        ASSERT(pDrawCtx->pDrawArr, GFMRV_ALLOC_FAILED);
    }

    /* Get the time elapsed from the previous frame */
    rv = gfm_getElapsedTime(&elapsed, pCtx);
    ASSERT_NR(rv == GFMRV_OK);

    /* Loop through every node */
    pPrev = 0;
    pTmp = pGroup->pActive;
    while (pTmp) {
        gfmGroupNode *pNext;
        int isInside, isAlive;

        /* Update the sprite */
        rv = gfmSprite_update(pTmp->pSelf, pCtx);
        ASSERT_NR(rv == GFMRV_OK);

        /* Update the node's timer */
        if (pTmp->timeAlive > 0) {
            pTmp->timeAlive -= elapsed;
        }

        /* Check if the sprite is inside the camera */
        rv = gfmCamera_isSpriteInside(pCam, pTmp->pSelf);
        ASSERT_NR(rv == GFMRV_TRUE || rv == GFMRV_FALSE);
        isInside = (rv == GFMRV_TRUE);

        /* Get the next node */
        pNext = pTmp->pNext;

        /* Check if the sprite should be "killed" */
        isAlive = (!pGroup->dieOnLeave || isInside) &&
                (pTmp->timeAlive == gfmGroup_keepAlive || pTmp->timeAlive > 0);
        if (!isAlive) {
            /* 'Kill' the node */
            pTmp->timeAlive = gfmGroup_forceKill;
            /* Remove the node */
            if (pPrev) {
                /* Simply bypass the 'dead' node */
                pPrev->pNext = pTmp->pNext;
            }
            else {
                /* It was the first node, modify the 'list root' */
                pGroup->pActive = pGroup->pActive->pNext;
            }
            /* Prepend the 'dead' node to the inactive list */
            pTmp->pNext = pGroup->pInactive;
            pGroup->pInactive = pTmp;
        }
        else if (isInside) {
            gfmGroupDrawNode *pNode;

            /* Otherwise, add it to the visible list (on the same order) */
            ASSERT(pDrawCtx->usedElements < pDrawCtx->totalElements,
                    GFMRV_BUFFER_TOO_SMALL);

            /* Retrieve the node's pointer */
            pNode = &(pDrawCtx->pDrawArr[pDrawCtx->usedElements]);
            pDrawCtx->usedElements++;

            /* Initialize it */
            pNode->pSelf = pTmp->pSelf;
            pNode->pRight = 0;
            pNode->pLeft = 0;
            switch (pDrawCtx->drawOrder) {
                /* Retrieve the vertical position for vertical sorting */
                case gfmDrawOrder_topFirst:
                case gfmDrawOrder_bottomFirst: {
                    rv = gfmSprite_getVerticalPosition(&(pNode->data.y),
                            pNode->pSelf);
                    ASSERT(rv == GFMRV_OK, rv);
                }; break;
                /* Retrieve the time alive for 'age sorting' */
                case gfmDrawOrder_newestFirst:
                case gfmDrawOrder_oldestFirst: {
                    pNode->data.timeAlive = pTmp->timeAlive;
                } break;
                default: { /* Otherwise, do nothing */ }
            }
        }

        /* Add it to the collideable list */
        switch (pGroup->collisionQuality) {
            /*
             * If you are heading this, I'm sorry... but I couldn't waste the
             * chance to write this *-*
             */
            case gfmCollisionQuality_everyThird:
                if (pGroup->skippedCollision < 2) {
                    pGroup->skippedCollision++;
                    break;
                }
            case gfmCollisionQuality_everySecond:
                if (pGroup->skippedCollision < 1) {
                    pGroup->skippedCollision++;
                    break;
                }
            case gfmCollisionQuality_visibleOnly:
                if (!isInside) {
                    break;
                }
            case gfmCollisionQuality_collideEverything:
                if (!isAlive) {
                    break;
                }
                /* If we got here, it's something that is collideable */
                pGroup->skippedCollision = 0;
                /* So, update the list */
                pTmp->pNextCollideable = pGroup->pCollideable;
                pGroup->pCollideable = pTmp;
            default: {}
        }

        /* If the current node wasn't removed, make it the previous one */
        if (pGroup->pInactive != pTmp)
            pPrev = pTmp;
        /* Go to the next node */
        pTmp = pNext;
    }

    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Add nodes 'recursively' to a tree, placing the ones at a higher vertical
 * position (i.e., lower y value) earlier than the others
 *
 * @param  pRoot     Root node of a tree or sub-tree
 * @param  pTreeNode Node that will be added to the tree
 * @return           GFMRV_OK, GFMRV_ARGUMENTS_BAD, ...
 */
static gfmRV gfmGroup_addTopBottom(gfmGroupDrawNode *pRoot, gfmGroupDrawNode *pNode) {
    gfmRV rv;

    while (1) {
        /* Make the first visited node have the lowest position */
        if (pNode->data.y <= pRoot->data.y) {
            if (pRoot->pLeft) {
                /* If not a leaf, move to the next node */
                pRoot = pRoot->pLeft;
            }
            else {
                /* Otherwise, add a new node */
                pRoot->pLeft = pNode;
                break;
            }
        }
        else {
            if (pRoot->pRight) {
                /* If not a leaf, move to the next node */
                pRoot = pRoot->pRight;
            }
            else {
                /* Otherwise, add a new node */
                pRoot->pRight = pNode;
                break;
            }
        }
    }

    rv = GFMRV_OK;
    return rv;
}

/**
 * Add nodes 'recursively' to a tree, placing the ones at a lower vertical
 * position (i.e., higher y value) earlier than the others
 *
 * @param  pRoot     Root node of a tree or sub-tree
 * @param  pTreeNode Node that will be added to the tree
 * @return           GFMRV_OK, GFMRV_ARGUMENTS_BAD, ...
 */
static gfmRV gfmGroup_addBottomTop(gfmGroupDrawNode *pRoot, gfmGroupDrawNode *pNode) {
    gfmRV rv;

    while (1) {
        /* Make the first visited node have the highest position */
        if (pNode->data.y > pRoot->data.y) {
            if (pRoot->pLeft) {
                /* If not a leaf, move to the next node */
                pRoot = pRoot->pLeft;
            }
            else {
                /* Otherwise, add a new node */
                pRoot->pLeft = pNode;
                break;
            }
        }
        else {
            if (pRoot->pRight) {
                /* If not a leaf, move to the next node */
                pRoot = pRoot->pRight;
            }
            else {
                /* Otherwise, add a new node */
                pRoot->pRight = pNode;
                break;
            }
        }
    }

    rv = GFMRV_OK;
    return rv;
}

/**
 * Add nodes 'recursively' to a tree, placing the latest recycled ones first
 *
 * @param  pRoot     Root node of a tree or sub-tree
 * @param  pTreeNode Node that will be added to the tree
 * @return           GFMRV_OK, GFMRV_ARGUMENTS_BAD, ...
 */
static gfmRV gfmGroup_addNewest(gfmGroupDrawNode *pRoot, gfmGroupDrawNode *pNode) {
    gfmRV rv;

    while (1) {
        /* Make the first visited node have the lowest timeAlive */
        if (pNode->data.timeAlive > pRoot->data.timeAlive) {
            if (pRoot->pLeft) {
                /* If not a leaf, move to the next node */
                pRoot = pRoot->pLeft;
            }
            else {
                /* Otherwise, add a new node */
                pRoot->pLeft = pNode;
                break;
            }
        }
        else {
            if (pRoot->pRight) {
                /* If not a leaf, move to the next node */
                pRoot = pRoot->pRight;
            }
            else {
                /* Otherwise, add a new node */
                pRoot->pRight = pNode;
                break;
            }
        }
    }

    rv = GFMRV_OK;
    return rv;
}

/**
 * Add nodes 'recursively' to a tree, placing the first recycled ones first
 *
 * @param  pRoot     Root node of a tree or sub-tree
 * @param  pTreeNode Node that will be added to the tree
 * @return           GFMRV_OK, GFMRV_ARGUMENTS_BAD, ...
 */
static gfmRV gfmGroup_addOldest(gfmGroupDrawNode *pRoot, gfmGroupDrawNode *pNode) {
    gfmRV rv;

    while (1) {
        /* Make the first visited node have the highest timeAlive */
        if (pNode->data.timeAlive <= pRoot->data.timeAlive) {
            if (pRoot->pLeft) {
                /* If not a leaf, move to the next node */
                pRoot = pRoot->pLeft;
            }
            else {
                /* Otherwise, add a new node */
                pRoot->pLeft = pNode;
                break;
            }
        }
        else {
            if (pRoot->pRight) {
                /* If not a leaf, move to the next node */
                pRoot = pRoot->pRight;
            }
            else {
                /* Otherwise, add a new node */
                pRoot->pRight = pNode;
                break;
            }
        }
    }

    rv = GFMRV_OK;
    return rv;
}

/**
 * Draw a (sub-)tree traversing it in pre-order
 *
 * @param  [ in]pRoot  The (sub-)tree root
 * @param  [ in]pCtx   The game's context
 * @return             GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
static gfmRV gfmGroup_drawTree(gfmGroupDrawNode *pRoot, gfmCtx *pCtx) {
    gfmGroupDrawNode *pTop;
    gfmRV rv;

    /* Initialize the tree's stack */
    pTop = pRoot;
    pTop->data.pStackNext = 0;

    /* Traverse the tree in pre-order */
    while (pTop) {
        if (pTop->pLeft) {
            /* Push the current node to be rendered after the current one */
            pTop->pLeft->data.pStackNext = pTop;
            pTop = pTop->pLeft;
            /* Remove the sub-tree to the left from the previous node */
            pTop->data.pStackNext->pLeft = 0;
        }
        else {
            /* Render the current node */
            rv = gfmSprite_draw(pTop->pSelf, pCtx);
            ASSERT_NR(rv == GFMRV_OK);

            if (pTop->pRight) {
                /* Render the sub-tree to the right before anything else on the
                 * stack (and remove the current node) */
                pTop->pRight->data.pStackNext = pTop->data.pStackNext;
                pTop = pTop->pRight;
            }
            else {
                /* Pop the next object to be renderd */
                pTop = pTop->data.pStackNext;
            }
        }
    }

    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Draw every sprite, following the desired mode
 *
 * @param  pGrp The group
 * @param  pCtx The game's context
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmGroup_draw(gfmGroup *pGroup,  gfmCtx *pCtx) {
    gfmGroupDrawCtx *pDrawCtx;
    gfmRV rv;
    int i;

    /**
     * Macro to insert all visible nodes into a tree and draw it; It expects the
     * first visible node to be at pNode;
     * This macro already ASSERT errors
     *
     * @param insertFunction The function to insert a node into the tree (must
     *                       receive two gfmDrawTree*, the first being the
     *                       tree's root and the second the current node)
     */
#define insertAndDrawTree(insertFunction) \
    do { \
        gfmGroupDrawNode *pRoot; \
        /* Since index 0 is the tree's root, start at the next one */ \
        i++; \
        pRoot = &(pDrawCtx->pDrawArr[0]); \
        /* Add all nodes iteratively */ \
        while (i < pDrawCtx->usedElements) { \
            /* Insert it into the tree */ \
            rv = insertFunction(pRoot, &(pDrawCtx->pDrawArr[i])); \
            ASSERT(rv == GFMRV_OK, rv); \
            i++; \
        } \
        /* Draw the tree */ \
        rv = gfmGroup_drawTree(pRoot, pCtx); \
        ASSERT(rv == GFMRV_OK, rv); \
    } while (0);

    /* Sanitize arguments */
    ASSERT(pGroup, GFMRV_ARGUMENTS_BAD);
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);

    pDrawCtx = pGroup->pDrawCtx;
    /* Assert there's anything to draw, at all */
    if (pDrawCtx->usedElements <= 0) {
        rv = GFMRV_OK;
        goto __ret;
    }

    i = 0;
    switch (pDrawCtx->drawOrder) {
        case gfmDrawOrder_linear: {
            /* Simply draw in the order they appear */
            while (i < pDrawCtx->usedElements) {
                rv = gfmSprite_draw(pDrawCtx->pDrawArr[i].pSelf, pCtx);
                ASSERT_NR(rv == GFMRV_OK);

                i++;
            }
        } break;
        case gfmDrawOrder_topFirst: {
            insertAndDrawTree(gfmGroup_addTopBottom);
        } break;
        case gfmDrawOrder_bottomFirst: {
            insertAndDrawTree(gfmGroup_addBottomTop);
        } break;
        case gfmDrawOrder_newestFirst: {
            insertAndDrawTree(gfmGroup_addNewest);
        } break;
        case gfmDrawOrder_oldestFirst: {
            insertAndDrawTree(gfmGroup_addOldest);
        } break;
        case gfmDrawOrder_max: {
            /* Shouldn't happen! */
            ASSERT(0, GFMRV_INTERNAL_ERROR);
        }
    }

    rv = GFMRV_OK;
__ret:
    return rv;
}

