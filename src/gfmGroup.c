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
/** Define an array for the tree nodes */
gfmGenArr_define(gfmDrawTree);

/** The gfmGroup structure */
struct stGFMGroup {
    /** How many objects the group handles, at most */
    int maxLen;
    /** Current draw order */
    gfmDrawOrder drawOrder;
    /** Array with every tree node */
    gfmGenArr_var(gfmDrawTree, pTree);
    /** Array with every group node */
    gfmGenArr_var(gfmGroupNode, pNodes);
    /** List of currently active nodes */
    gfmGroupNode *pActive;
    /** List of currently inactive nodes */
    gfmGroupNode *pInactive;
    /** List of currently visible nodes */
    gfmGroupNode *pVisible;
    /** Position where insertions are made, to keep the visible list order */
    gfmGroupNode *pLastVisible;
    /** Points to the last retrieved sprite */
    gfmSprite *pLast;
    /** Whether should die on leaving the screen */
    int dieOnLeave;
    /** For how long the sprites should live */
    int ttl;
    /** Default spriteset; assigned to every sprite *ON ALLOCATION* */
    gfmSpriteset *pDefSset;
    /** Default animation; assigned to every sprite *ON ALLOCATION* */
    int *pDefAnimData;
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

/**
 * Alloc a new group
 * 
 * @param  ppCtx The group
 * @return       GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_ALLOC_FAILED
 */
gfmRV gfmGroup_getNew(gfmGroup **ppCtx) {
    gfmRV rv;
    
    // Sanitize arguments
    ASSERT(ppCtx, GFMRV_ARGUMENTS_BAD);
    ASSERT(!(*ppCtx), GFMRV_ARGUMENTS_BAD);
    
    // Alloc the object
    *ppCtx = (gfmGroup*)malloc(sizeof(gfmGroup));
    ASSERT(*ppCtx, GFMRV_ALLOC_FAILED);
    // Initialize it
    memset(*ppCtx, 0x00, sizeof(gfmGroup));
    
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
    
    // Sanitize arguments
    ASSERT(ppCtx, GFMRV_ARGUMENTS_BAD);
    ASSERT(*ppCtx, GFMRV_ARGUMENTS_BAD);
    
    // Clean up the group
    rv = gfmGroup_clean(*ppCtx);
    ASSERT_NR(rv == GFMRV_OK);
    
    // Free the memory
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
    ASSERT(pCtx->pDefSset, GFMRV_GROUP_SPRITESET_NOT_SET);
    ASSERT(pCtx->defWidth > 0, GFMRV_GROUP_WIDTH_NOT_SET);
    ASSERT(pCtx->defHeight > 0, GFMRV_GROUP_HEIGHT_NOT_SET);
    // Check that the initial number of sprites is valid
    ASSERT(maxLen == 0 || initLen <= maxLen, GFMRV_GROUP_MAX_SPRITES);
    
    // Set the group's max len
    pCtx->maxLen = maxLen;
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
    ASSERT(pCtx->pDefSset, GFMRV_GROUP_SPRITESET_NOT_SET);
    ASSERT(pCtx->defWidth > 0, GFMRV_GROUP_WIDTH_NOT_SET);
    ASSERT(pCtx->defHeight > 0, GFMRV_GROUP_HEIGHT_NOT_SET);
    // Check if the buffer can be expanded
    pos = gfmGenArr_getUsed(pCtx->pNodes);
    newLen = pos + num;
    ASSERT(pCtx->maxLen == 0 || newLen <= pCtx->maxLen, 
            GFMRV_GROUP_MAX_SPRITES);
    
    // Expand the nodes buffer
    gfmGenArr_setMinSize(gfmGroupNode, pCtx->pNodes, pos + num,
            gfmGroupNode_getNew);
    // Expand the tree nodes buffer
    gfmGenArr_setMinSize(gfmDrawTree, pCtx->pTree, pos + num,
            gfmDrawTree_getNew);
    
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
        rv = gfmSprite_init(pNode->pSelf, 0/*x*/, 0/*y*/, pCtx->defWidth,
                pCtx->defHeight, pCtx->pDefSset, pCtx->defOffX, pCtx->defOffY,
                0/*child*/, gfmType_none);
        ASSERT_NR(rv == GFMRV_OK);
        
        
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
    
    // Sanitize arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    
    // Free every node on the list
    gfmGenArr_clean(pCtx->pTree, gfmDrawTree_free);
    gfmGenArr_clean(pCtx->pNodes, gfmGroupNode_free);
    
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
gfmRV gfmGroup_insert(gfmGroup *pCtx, gfmSprite *pSpr, int autoFree);
// TODO gfmGroup_insert

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
        ASSERT(pCtx->maxLen == 0
                || gfmGenArr_getUsed(pCtx->pNodes) < pCtx->maxLen,
                GFMRV_GROUP_MAX_SPRITES);
        
        // Get the new length of the buffer
        curLen = gfmGenArr_getUsed(pCtx->pNodes);
        if (curLen == 0) {
            // If the list was empty, initialize it
            newLen = 1;
        }
        else if (curLen * 2 > pCtx->maxLen) {
            // If at the limit, add as many sprites as possible
            newLen = pCtx->maxLen - curLen;
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
    pTmp->timeAlive = 0;
    
    // Set the sprite's default values
    rv = gfmSprite_resetObject(pTmp->pSelf);
    ASSERT_NR(rv == GFMRV_OK);
    rv = gfmSprite_setVelocity(pTmp->pSelf, pCtx->defVx, pCtx->defVy);
    ASSERT_NR(rv == GFMRV_OK);
    rv = gfmSprite_setAcceleration(pTmp->pSelf, pCtx->defAx, pCtx->defAy);
    ASSERT_NR(rv == GFMRV_OK);
    
    // Insert it at the begining of the active list
    pTmp->pNext = pCtx->pActive;
    pCtx->pActive = pTmp;
    
    // Store this last retrieved sprite so it can be modified later
    pCtx->pLast = pTmp->pSelf;
    
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
    pCtx->pDefSset = pSset;
    
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
    pCtx->pDefAnimData = pData;
    pCtx->defAnimLen = len;
    
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
    pCtx->defWidth = width;
    pCtx->defHeight = height;
    pCtx->defOffX = offX;
    pCtx->defOffY = offY;
    
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
    pCtx->defVx = vx;
    pCtx->defVy = vy;
    
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
    pCtx->defAx = ax;
    pCtx->defAy = ay;
    
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
 * @param  ttl  Time to live (0 for infinite)
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD, ...
 */
gfmRV gfmGroup_setDeathOnTime(gfmGroup *pCtx, int ttl) {
    gfmRV rv;
    
    // Sanitize arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    
    // Set the default "time to live"
    pCtx->ttl = ttl;
    
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
    
    pCtx->drawOrder = order;
    
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
    gfmGroupNode *pTmp;
    gfmGroupNode *pPrev;
    gfmRV rv;
    int elapsed;
    
    // Sanitize arguments
    ASSERT(pGroup, GFMRV_ARGUMENTS_BAD);
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    
    // Remove the reference to the last recycled/added sprite
    pGroup->pLast = 0;
    // Reset the list of visible sprites
    pGroup->pVisible = 0;
    pGroup->pLastVisible = 0;
    // Retrieve the current camera
    rv = gfm_getCamera(&pCam, pCtx);
    ASSERT_NR(rv == GFMRV_OK);
    
    // Get the time elapsed from the previous frame
    rv = gfm_getElapsedTime(&elapsed, pCtx);
    ASSERT_NR(rv == GFMRV_OK);
    
    // Loop through every node
    pPrev = 0;
    pTmp = pGroup->pActive;
    while (pTmp) {
        gfmGroupNode *pNext;
        int isInside;
        
        // Update the sprite
        rv = gfmSprite_update(pTmp->pSelf, pCtx);
        ASSERT_NR(rv == GFMRV_OK);
        
        // Update the node's timer
        pTmp->timeAlive += elapsed;
        
        // Check if the sprite is inside the camera
        rv = gfmCamera_isSpriteInside(pCam, pTmp->pSelf);
        ASSERT_NR(rv == GFMRV_TRUE || rv == GFMRV_FALSE);
        isInside = (rv == GFMRV_TRUE);
        
        // Get the next node
        pNext = pTmp->pNext;
        
        // Check if the sprite should be "killed"
        if ((pGroup->ttl != 0 && pTmp->timeAlive > pGroup->ttl)
                || (pGroup->dieOnLeave && !isInside)) {
            // Remove the node
            if (pPrev) {
                // Simply bypass the 'dead' node
                pPrev->pNext = pTmp->pNext;
            }
            else {
                // It was the first node, modify the 'list root'
                pGroup->pActive = pGroup->pActive->pNext;
            }
            // Prepend the 'dead' node to the inactive list
            pTmp->pNext = pGroup->pInactive;
            pGroup->pInactive = pTmp;
        }
        else if (isInside) {
            // Otherwise, add it to the visible list (on the same order)
            if (pGroup->pLastVisible)
                pGroup->pLastVisible->pNextVisible = pTmp;
            // Move the visible list to its next node
            pGroup->pLastVisible = pTmp;
            // Set the first visible object
            if (!pGroup->pVisible)
                pGroup->pVisible = pTmp;
        }
        
        // If the current node wasn't removed, make it the previous one
        if (pGroup->pInactive != pTmp)
            pPrev = pTmp;
        // Go to the next node
        pTmp = pNext;
    }
    // Put a NULL pointer so it can actually stop
    if (pGroup->pLastVisible)
        pGroup->pLastVisible->pNextVisible = 0;
    
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
static gfmRV gfmGroup_addTopBottom(gfmDrawTree *pRoot, gfmDrawTree *pTreeNode) {
    gfmRV rv;
    int y, otherY;
    
    // Sanitize arguments
    ASSERT(pRoot, GFMRV_ARGUMENTS_BAD);
    ASSERT(pTreeNode, GFMRV_ARGUMENTS_BAD);
    
    // Get the position of the sprite to be added
    rv = gfmSprite_getVerticalPosition(&y, pTreeNode->pSelf->pSelf);
    ASSERT_NR(rv == GFMRV_OK);
    while (1) {
        // Get the position of the sprite on the current node
        rv = gfmSprite_getVerticalPosition(&otherY, pRoot->pSelf->pSelf);
        ASSERT_NR(rv == GFMRV_OK);
        
        // Make the first visited node have the lowest position
        if (y <= otherY) {
            if (pRoot->pLeft) {
                // If not a leaf, move to the next node
                pRoot = pRoot->pLeft;
            }
            else {
                // Otherwise, add a new node
                pRoot->pLeft = pTreeNode;
                break;
            }
        }
        else {
            if (pRoot->pRight) {
                // If not a leaf, move to the next node
                pRoot = pRoot->pRight;
            }
            else {
                // Otherwise, add a new node
                pRoot->pRight = pTreeNode;
                break;
            }
        }
    }
    
    rv = GFMRV_OK;
__ret:
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
static gfmRV gfmGroup_addBottomTop(gfmDrawTree *pRoot, gfmDrawTree *pTreeNode) {
    gfmRV rv;
    int y, otherY;
    
    // Sanitize arguments
    ASSERT(pRoot, GFMRV_ARGUMENTS_BAD);
    ASSERT(pTreeNode, GFMRV_ARGUMENTS_BAD);
    
    // Get the position of the sprite to be added
    rv = gfmSprite_getVerticalPosition(&y, pTreeNode->pSelf->pSelf);
    ASSERT_NR(rv == GFMRV_OK);
    while (1) {
        // Get the position of the sprite on the current node
        rv = gfmSprite_getVerticalPosition(&otherY, pRoot->pSelf->pSelf);
        ASSERT_NR(rv == GFMRV_OK);
        
        // Make the first visited node have the highest position
        if (y > otherY) {
            if (pRoot->pLeft) {
                // If not a leaf, move to the next node
                pRoot = pRoot->pLeft;
            }
            else {
                // Otherwise, add a new node
                pRoot->pLeft = pTreeNode;
                break;
            }
        }
        else {
            if (pRoot->pRight) {
                // If not a leaf, move to the next node
                pRoot = pRoot->pRight;
            }
            else {
                // Otherwise, add a new node
                pRoot->pRight = pTreeNode;
                break;
            }
        }
    }
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Add nodes 'recursively' to a tree, placing the latest recycled ones first
 * 
 * @param  pRoot     Root node of a tree or sub-tree
 * @param  pTreeNode Node that will be added to the tree
 * @return           GFMRV_OK, GFMRV_ARGUMENTS_BAD, ...
 */
static gfmRV gfmGroup_addNewest(gfmDrawTree *pRoot, gfmDrawTree *pTreeNode) {
    gfmRV rv;
    
    // Sanitize arguments
    ASSERT(pRoot, GFMRV_ARGUMENTS_BAD);
    ASSERT(pTreeNode, GFMRV_ARGUMENTS_BAD);
    
    while (1) {
        // Make the first visited node have the lowest timeAlive
        if (pTreeNode->pSelf->timeAlive <= pRoot->pSelf->timeAlive) {
            if (pRoot->pLeft) {
                // If not a leaf, move to the next node
                pRoot = pRoot->pLeft;
            }
            else {
                // Otherwise, add a new node
                pRoot->pLeft = pTreeNode;
                break;
            }
        }
        else {
            if (pRoot->pRight) {
                // If not a leaf, move to the next node
                pRoot = pRoot->pRight;
            }
            else {
                // Otherwise, add a new node
                pRoot->pRight = pTreeNode;
                break;
            }
        }
    }
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Add nodes 'recursively' to a tree, placing the first recycled ones first
 * 
 * @param  pRoot     Root node of a tree or sub-tree
 * @param  pTreeNode Node that will be added to the tree
 * @return           GFMRV_OK, GFMRV_ARGUMENTS_BAD, ...
 */
static gfmRV gfmGroup_addOldest(gfmDrawTree *pRoot, gfmDrawTree *pTreeNode) {
    gfmRV rv;
    
    // Sanitize arguments
    ASSERT(pRoot, GFMRV_ARGUMENTS_BAD);
    ASSERT(pTreeNode, GFMRV_ARGUMENTS_BAD);
    
    while (1) {
        // Make the first visited node have the highest timeAlive
        if (pTreeNode->pSelf->timeAlive > pRoot->pSelf->timeAlive) {
            if (pRoot->pLeft) {
                // If not a leaf, move to the next node
                pRoot = pRoot->pLeft;
            }
            else {
                // Otherwise, add a new node
                pRoot->pLeft = pTreeNode;
                break;
            }
        }
        else {
            if (pRoot->pRight) {
                // If not a leaf, move to the next node
                pRoot = pRoot->pRight;
            }
            else {
                // Otherwise, add a new node
                pRoot->pRight = pTreeNode;
                break;
            }
        }
    }
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Draw a (sub-)tree traversing it in pre-order
 * 
 * @param  pRoot The (sub-)tree root
 * @param  pCtx  The game's context
 * @return       GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
static gfmRV gfmGroup_drawTree(gfmDrawTree *pRoot, gfmCtx *pCtx) {
    gfmRV rv;
    
    // Sanitize argument
    ASSERT(pRoot, GFMRV_ARGUMENTS_BAD);
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    
    if (pRoot->pLeft) {
        rv = gfmGroup_drawTree(pRoot->pLeft, pCtx);
        ASSERT_NR(rv == GFMRV_OK);
    }
    rv = gfmSprite_draw(pRoot->pSelf->pSelf, pCtx);
    ASSERT_NR(rv == GFMRV_OK);
    if (pRoot->pRight) {
        rv = gfmGroup_drawTree(pRoot->pRight, pCtx);
        ASSERT_NR(rv == GFMRV_OK);
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
    gfmDrawTree *pRoot;
    gfmGroupNode *pNode;
    gfmRV rv;
    
    // Sanitize arguments
    ASSERT(pGroup, GFMRV_ARGUMENTS_BAD);
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    
    // Reset the tree nodes
    gfmGenArr_reset(pGroup->pTree);
    
    // Manually set the first tree node
    gfmGenArr_getNextRef(gfmDrawTree, pGroup->pTree, 1/*inc*/, pRoot,
            gfmDrawTree_getNew);
    gfmGenArr_push(pGroup->pTree);
    pRoot->pLeft = 0;
    pRoot->pRight = 0;
    pRoot->pSelf = pGroup->pVisible;
    // Get the first node on the visible list
    pNode = pGroup->pVisible;
    
    // TODO Start batch
    
    switch (pGroup->drawOrder) {
        case gfmDrawOrder_linear: {
            // Simply draw in the order they appear
            while (pNode) {
                rv = gfmSprite_draw(pNode->pSelf, pCtx);
                ASSERT_NR(rv == GFMRV_OK);
                
                pNode = pNode->pNextVisible;
            }
        } break;
        case gfmDrawOrder_topFirst: {
            // Actually start the list at the second node
            if (pNode)
                pNode = pNode->pNext;
            // Sort the visible list from top to bottom
            while (pNode) {
                gfmDrawTree *pTreeNode;
                
                // Get the current tree node (and insert the actual node)
                gfmGenArr_getNextRef(gfmDrawTree, pGroup->pTree, 1/*inc*/, pTreeNode,
                        gfmDrawTree_getNew);
                gfmGenArr_push(pGroup->pTree);
                pTreeNode->pSelf = pNode;
                // Make sure the node is clean
                pTreeNode->pLeft = 0;
                pTreeNode->pRight = 0;
                // Insert it into the tree
                rv = gfmGroup_addTopBottom(pRoot, pTreeNode);
                ASSERT_NR(rv == GFMRV_OK);
                
                pNode = pNode->pNextVisible;
            }
            
            // Draw the tree
            rv = gfmGroup_drawTree(pRoot, pCtx);
            ASSERT_NR(rv == GFMRV_OK);
        } break;
        case gfmDrawOrder_bottomFirst: {
            // Actually start the list at the second node
            if (pNode)
                pNode = pNode->pNext;
            // Sort the visible list from top to bottom
            while (pNode) {
                gfmDrawTree *pTreeNode;
                
                // Get the current tree node (and insert the actual node)
                gfmGenArr_getNextRef(gfmDrawTree, pGroup->pTree, 1/*inc*/, pTreeNode,
                        gfmDrawTree_getNew);
                gfmGenArr_push(pGroup->pTree);
                pTreeNode->pSelf = pNode;
                // Make sure the node is clean
                pTreeNode->pLeft = 0;
                pTreeNode->pRight = 0;
                // Insert it into the tree
                rv = gfmGroup_addBottomTop(pRoot, pTreeNode);
                ASSERT_NR(rv == GFMRV_OK);
                
                pNode = pNode->pNextVisible;
            }
            
            // Draw the tree
            rv = gfmGroup_drawTree(pRoot, pCtx);
            ASSERT_NR(rv == GFMRV_OK);
        } break;
        case gfmDrawOrder_newestFirst: {
            // Actually start the list at the second node
            if (pNode)
                pNode = pNode->pNext;
            // Sort the visible list from top to bottom
            while (pNode) {
                gfmDrawTree *pTreeNode;
                
                // Get the current tree node (and insert the actual node)
                gfmGenArr_getNextRef(gfmDrawTree, pGroup->pTree, 1/*inc*/, pTreeNode,
                        gfmDrawTree_getNew);
                gfmGenArr_push(pGroup->pTree);
                pTreeNode->pSelf = pNode;
                // Make sure the node is clean
                pTreeNode->pLeft = 0;
                pTreeNode->pRight = 0;
                // Insert it into the tree
                rv = gfmGroup_addNewest(pRoot, pTreeNode);
                ASSERT_NR(rv == GFMRV_OK);
                
                pNode = pNode->pNextVisible;
            }
            
            // Draw the tree
            rv = gfmGroup_drawTree(pRoot, pCtx);
            ASSERT_NR(rv == GFMRV_OK);
        } break;
        case gfmDrawOrder_oldestFirst: {
            // Actually start the list at the second node
            if (pNode)
                pNode = pNode->pNext;
            // Sort the visible list from top to bottom
            while (pNode) {
                gfmDrawTree *pTreeNode;
                
                // Get the current tree node (and insert the actual node)
                gfmGenArr_getNextRef(gfmDrawTree, pGroup->pTree, 1/*inc*/, pTreeNode,
                        gfmDrawTree_getNew);
                gfmGenArr_push(pGroup->pTree);
                pTreeNode->pSelf = pNode;
                // Make sure the node is clean
                pTreeNode->pLeft = 0;
                pTreeNode->pRight = 0;
                // Insert it into the tree
                rv = gfmGroup_addOldest(pRoot, pTreeNode);
                ASSERT_NR(rv == GFMRV_OK);
                
                pNode = pNode->pNextVisible;
            }
            
            // Draw the tree
            rv = gfmGroup_drawTree(pRoot, pCtx);
            ASSERT_NR(rv == GFMRV_OK);
        } break;
        case gfmDrawOrder_max: {
            // Shouldn't happen!
            ASSERT(0, GFMRV_INTERNAL_ERROR);
        }
    }
    
    // TODO End batch
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

