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
 *   - Initial velocity
 *   - Initial acceleration
 *   - ??? (anything else?)
 * Also, when a new sprite is added (either by adding it or recycling it), it's
 * possible to set those values:
 *   - Position
 *   - Maximum time alive
 *   - Whether should die on leaving the screen
 *   - Current animation
 */
#include <GFraMe/gframe.h>
#include <GFraMe/gfmAssert.h>
#include <GFraMe/gfmError.h>
#include <GFraMe/gfmGenericArray.h>
#include <GFraMe/gfmGroup.h>
#include <GFraMe/gfmObject.h>
#include <GFraMe/gfmSprite.h>

#include <stdlib.h>
#include <string.h>

/** 'Exports' the gfmDrawTree structure */
typedef struct stGFMDrawTree gfmDrawTree;
/** 'Exports' the gfmGroupNode structure */
typedef struct stGFMGroupNode gfmGroupNode;

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
    /** Points to the last retrieved sprite */
    gfmSprite *pLast;
};

/** The gfmDrawTree structure */
struct stGFMDrawTree {
    /** Left node (less or equal some value) */
    gfmDrawTree *pLeft;
    /** Right node (greater than some value) */
    gfmDrawTree *pRight;
    /** Current node */
    gfmGroupNode *pSelf;
};

/** The gfmGroupNode structure */
struct stGFMGroupNode {
    /** Next pointer on the list */
    gfmGroupNode *pNext;
    /* Actualy pointer to the object */
    gfmSprite *pSelf;
    /** For how long this node has been on the alive list */
    unsigned int timeAlive;
    /** Whether this reference should be automatically freed or not */
    int autoFree;
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
    ASSERT(rv == GFMRV_OK);
    
    // Free the memory
    free(*ppCtx);
    *ppCtx = 0;
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Initialize the group; The group can be initialized with a few
 * pre-instantiated sprites; Those (and any other marked as such) will be
 * automatically freed by the group
 * 
 * NOTE: This function needn't be called! You should only do so to when your
 * game (or scene) starts to avoid expanding the buffer during the game (which,
 * in turn, avoids unnecessary 'malloc's followed by 'memcpy's and 'free's)
 * 
 * @param  pCtx     The group
 * @param  initLen  How many nodes should be pre-allocated (it may be 0)
 * @param  maxLen   Group's maximum length (0 if there's no limit)
 * @return          GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_ALLOC_FAILED
 */
gfmRV gfmGroup_init(gfmGroup *pCtx, int initLen, int maxLen) {
    gfmRV rv;
    int i, inc;
    
    // Set this 'const variable'
    inc = 1;
    
    // Sanitize arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    ASSERT(initLen >= 0, GFMRV_ARGUMENTS_BAD);
    ASSERT(maxLen >= 0, GFMRV_ARGUMENTS_BAD);
    // TODO Check if it was already initialized
    // Check that the initial number of sprites is valid
    ASSERT(maxLen == 0 || initLen <= maxLen, GFMRV_GROUP_MAX_SPRITES);
    
    // Set the group's max len
    pCtx->maxLen = maxLen;
    
    // Initialize the group, if desired
    gfmGenArr_setMinSize(gfmGroupNode, pCtx->pNodes, initLen,
            gfmGroupNode_getNew);
    // Also instantiate one tree node per "group node"
    gfmGenArr_setMinSize(gfmDrawTree, pCtx->pTrees, initLen,
            gfmDrawTree_getNew);
    
    // Pre-alloc every sprite
    i = 0;
    while (i < initLen) {
        gfmGroupNode *pNode;
        gfmGroupNode *pNextNode;
        
        // Retrieve a node
        gfmGenArr_getNextRef(gfmGroupNode, pCtx->pNodes, inc, pNode,
                gfmGroupNode_getNew);
        gfmGenArr_push(pCtx->pNodes);
        // Alloc its sprite (and set it for auto dealloc'ing)
        rv = gfmGroupNode_allocSprite(pNode);
        ASSERT(rv == GFMRV_OK);
        
        // The only "elegant" solution I can think of is to do this while for
        // initLen - 1 and the alloc the last object outside the loop... oh well
        if (i < initLen - 1)
            pNextNode = gfmGenArr_getObject(pCtx->pNodes, i + 1);
        else
            pNextNode = 0;
        
        // Set the next node in the list
        pNode->pNext = pNextNode;
        // Alloc its sprite
        rv = gfmSprite_getNew(&(pNode->pSelf));
        ASSERT(rv == GFMRV_OK);
        // Set it as autoDeallocate'able
        pNode->autoFree = 1;
        
        i++;
    }
    
    // Clean the active list
    pCtx->pActive = 0;
    // Set the inactive list (i.e., first node from the list)
    pCtx->pInactive = gfmGenArr_getObject(pCtx->pNodes, 0);
    
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
    // TODO Check that it was initialized (?)
    
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

/**
 * Reuse an inactive sprite; If none is found, a new one is allocated
 * 
 * @param  ppSpr The sprite
 * @param  pCtx  The group
 * @return       GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_ALLOC_FAILED
 */
gfmRV gfmGroup_recycle(gfmSprite **ppSpr, gfmGroup *pCtx) {
    gfmRV rv;
    gfmGroupNode *pTmp;
    
    // Sanitize arguments
    ASSERT(ppSpr, GFMRV_ARGUMENTS_BAD);
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    
    // Check if the inactive list isn't empty
    if (pCtx->pInactive) {
        pTmp = pCtx->pInactive;
        // Remove the current node from the list
        pCtx->pInactive = pCtx->pInactive->pNext;
    }
    else {
        // Check that there's space for more sprites
        ASSERT(pCtx->maxLen == 0
                || gfmGenArr_getUsed(pCtx->pNodes) < pCtx->maxLen,
                GFMRV_GROUP_MAX_SPRITES);
        // TODO Alloc a new node & sprite
    }
    
    // Reset the node's timer
    pTmp->timeAlive = 0;
    // TODO Set the sprite's default values
    // Insert it at the begining of the active list
    pTmp->pNext = pCtx->pActive;
    pCtx->pActive = pTmp;
    
    // Store this last retrieved sprite so it can be modified later
    pCtx->pLast = pTemp->pSelf;
    
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
    gfmRV rv;
    gfmGroupNode *pTmp;
    
    // Sanitize arguments
    ASSERT(pGroup, GFMRV_ARGUMENTS_BAD);
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    
    // Loop through every node
    pTmp = pGroup->pActive;
    while (pTmp) {
        gfmObject *pObj;
        
        // Update the sprite
        rv = gfmSprite_update(pTmp->pSelf, pCtx);
        ASSERT_NR(rv == GFMRV_OK);
        
        // Retrieve its object
        rv = gfmSprite_getObject(&pObj, pTmp->pSelf);
        ASSERT_NR(rv == GFMRV_OK);
        
        // TODO Check if the sprite should be "killed"
        
        // Go to the next node
        pTmp = pTmp->pNext;
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
    gfmRV rv;
    
    // Sanitize arguments
    ASSERT(pGroup, GFMRV_ARGUMENTS_BAD);
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

#endif /* __GFMGROUP_H__ */

