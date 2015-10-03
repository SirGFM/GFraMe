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
#ifndef __GFMGROUP_STRUCT__
#define __GFMGROUP_STRUCT__

/** 'Exports' the gfmGroup structure */
typedef struct stGFMGroup gfmGroup;
/** 'Exports' the gfmGroupNode structure */
typedef struct stGFMGroupNode gfmGroupNode;
/** 'Exports' the gfmDrawOrder enumeration */
typedef enum enGFMDrawOrder gfmDrawOrder;
/** 'Exports' the gfmGroupCollision enumeration */
typedef enum enGFMGroupCollision gfmGroupCollision;

#endif /* __GFMGROUP_STRUCT__ */

#ifndef __GFMGROUP_H__
#define __GFMGROUP_H__

#include <GFraMe/gframe.h>
#include <GFraMe/gfmError.h>
#include <GFraMe/gfmSprite.h>
#include <GFraMe/gfmSpriteset.h>

/** 'Exportable' size of gfmGroup */
extern const int sizeofGFMGroup;

/** The gfmDrawOrder enumeration */
enum enGFMDrawOrder {
    /**  Draw every object in the order they were added */
    gfmDrawOrder_linear = 0,
    /** Draw the objects at the top of the screen first */
    gfmDrawOrder_topFirst,
    /** Draw the objects at the bottom of the screen first */
    gfmDrawOrder_bottomFirst,
    /** Draw the newest objects first */
    gfmDrawOrder_newestFirst,
    /** Draw the older objects first */
    gfmDrawOrder_oldestFirst,
    /** Number of possibles draw orders */
    gfmDrawOrder_max
};

/** The gfmGroupCollision enumeration */
enum enGFMGroupCollision {
    /** Don't collide at all */
    gfmCollisionQuality_none = 0,
    /** Collide only the visible objects */
    gfmCollisionQuality_visibleOnly,
    /** Collide every second visible objects (may cause weird stuff) */
    gfmCollisionQuality_everySecond,
    /** Collide every third visible objects (may cause even weirder stuff) */
    gfmCollisionQuality_everyThird,
    /** Please, don't! */
    gfmCollisionQuality_collideEverything,
    /** Number of possibles collision qualities */
    gfmCollisionQuality_max
};

/**
 * Alloc a new group
 * 
 * @param  ppCtx The group
 * @return       GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_ALLOC_FAILED
 */
gfmRV gfmGroup_getNew(gfmGroup **ppCtx);

/**
 * Frees a previously alloc'ed group (and any of its memory)
 * 
 * @param  ppCtx The group
 * @return       GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmGroup_free(gfmGroup **ppCtx);

/**
 * Pre cache the group; Instantiate and initialize a few sprites; The sprite's
 * default attributes must be set before calling this function, as to correctly
 * initialize the sprites
 * 
 * @param  pCtx     The group
 * @param  initLen  How many nodes should be pre-allocated
 * @param  maxLen   Group's maximum length (0 if there's no limit)
 * @return          GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_ALLOC_FAILED
 */
gfmRV gfmGroup_preCache(gfmGroup *pCtx, int initLen, int maxLen);

/**
 * Cache more sprite
 * 
 * @param  pCtx The group
 * @param  num  How many new sprites should be cached
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_ALLOC_FAILED,
 *              GFMRV_GROUP_MAX_SPRITES
 */
gfmRV gfmGroup_cacheSprites(gfmGroup *pCtx, int num);

/**
 * Clean up the group and all of its member
 * 
 * @param  pCtx The group
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmGroup_clean(gfmGroup *pCtx);

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
 * @return       GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_ALLOC_FAILED,
 *               GFMRV_GROUP_MAX_SPRITES
 */
gfmRV gfmGroup_recycle(gfmSprite **ppSpr, gfmGroup *pCtx);

/**
 * Set the default type on every recycled sprite
 * 
 * @param  pCtx The group
 * @param  type The type
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD, ...
 */
gfmRV gfmGroup_setDefType(gfmGroup *pCtx, int type);

/**
 * Set the default spriteset on every recycled sprite
 * 
 * @param  pCtx  The group
 * @param  pSset The spriteset
 * @return       GFMRV_OK, GFMRV_ARGUMENTS_BAD, ...
 */
gfmRV gfmGroup_setDefSpriteset(gfmGroup *pCtx, gfmSpriteset *pSset);

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
gfmRV gfmGroup_setDefAnimData(gfmGroup *pCtx, int *pData, int len);

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
        int offY);

/**
 * Set the default velocity on every recycled sprite
 * 
 * @param  pCtx The group
 * @param  vx   The horizontal velocity
 * @param  vy   The vertical velocity
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD, ...
 */
gfmRV gfmGroup_setDefVelocity(gfmGroup *pCtx, int vx, int vy);

/**
 * Set the default acceleration on every recycled sprite
 * 
 * @param  pCtx The group
 * @param  ax   The horizontal acceleration
 * @param  ay   The vertical acceleration
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD, ...
 */
gfmRV gfmGroup_setDefAcceleration(gfmGroup *pCtx, int ax, int ay);

/**
 * Set whether every recycle sprite should 'die' when it leaves the screen
 * 
 * NOTE: This is a "global" attribute and it will take effect on every sprite on
 * the group AS SOON as it's assigned
 * 
 * @param  pCtx  The group
 * @param  doDie Whether the recycled sprites should die or not
 * @return       GFMRV_OK, GFMRV_ARGUMENTS_BAD, ...
 */
gfmRV gfmGroup_setDeathOnLeave(gfmGroup *pCtx, int doDie);

/**
 * Set for how long every recycled sprite should live
 * 
 * NOTE: This is a "global" attribute and it will take effect on every sprite on
 * the group AS SOON as it's assigned
 * 
 * @param  pCtx The group
 * @param  ttl  Time to live (0 for infinite)
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD, ...
 */
gfmRV gfmGroup_setDeathOnTime(gfmGroup *pCtx, int ttl);

/**
 * Set the position of the last recycled/added sprite
 * 
 * @param  pCtx The group
 * @param  x    Horizontal position
 * @param  y    Vertical position
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD, ...
 */
gfmRV gfmGroup_setPosition(gfmGroup *pCtx, int x, int y);

/**
 * Set the frame of the last recycled/added sprite
 * 
 * @param  pCtx  The group
 * @param  frame The frame
 * @return       GFMRV_OK, GFMRV_ARGUMENTS_BAD, ...
 */
gfmRV gfmGroup_setFrame(gfmGroup *pCtx, int frame);

/**
 * Set the animation of the last recycled/added sprite
 * 
 * @param  pCtx The group
 * @param  anim The animation's index
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD, ...
 */
gfmRV gfmGroup_setAnimation(gfmGroup *pCtx, int anim);

/**
 * Set the velocity of the last recycled/added sprite
 * 
 * @param  pCtx The group
 * @param  vx   The horizontal velocity
 * @param  vy   The vertical velocity
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD, ...
 */
gfmRV gfmGroup_setVelocity(gfmGroup *pCtx, int vx, int vy);

/**
 * Set the acceleration of the last recycled/added sprite
 * 
 * @param  pCtx The group
 * @param  ax   The horizontal acceleration
 * @param  ay   The vertical acceleration
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD, ...
 */
gfmRV gfmGroup_setAcceleration(gfmGroup *pCtx, int ax, int ay);

/**
 * Set the group's draw order
 * 
 * @param  pCtx  The group
 * @param  order The new draw order
 * @return       GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_GROUP_INVALID_TYPE
 */
gfmRV gfmGroup_setDrawOrder(gfmGroup *pCtx, gfmDrawOrder order);

/**
 * Set the collision mode
 * 
 * @param  pCtx The group
 * @param  col  The new collision quality
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_GROUP_INVALID_TYPE
 */
gfmRV gfmGroup_setCollisionQuality(gfmGroup *pCtx, gfmGroupCollision col);

/**
 * Force a node to be removed on the next update
 * 
 * @param  pCtx The node
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmGroup_removeNode(gfmGroupNode *pCtx);

/**
 * Iterate through every sprite and update'em
 * 
 * @param  pGrp The group
 * @param  pCtx The game's context
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmGroup_update(gfmGroup *pGroup, gfmCtx *pCtx);

/**
 * Draw every sprite, following the desired mode
 * 
 * @param  pGrp The group
 * @param  pCtx The game's context
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmGroup_draw(gfmGroup *pGroup,  gfmCtx *pCtx);

#endif /* __GFMGROUP_H__ */

