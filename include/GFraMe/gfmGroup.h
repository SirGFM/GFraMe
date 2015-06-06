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
/** 'Exports' the gfmDrawOrder enumeration */
typedef enum enGFMDrawOrder gfmDrawOrder;

#endif /* __GFMGROUP_STRUCT__ */

#ifndef __GFMGROUP_H__
#define __GFMGROUP_H__

#include <GFraMe/gframe.h>
#include <GFraMe/gfmError.h>
#include <GFraMe/gfmSprite.h>

/** 'Exportable' size of gfmGroup */
extern const int sizeofGFMGroup;

/** The gfmDrawOrder enumeration */
enum enGFMDrawOrder {
    /**  Draw every object in the order they were added */
    gfmDrawOrder_linear            = 0,
    /** Draw the objects at the top of the screen first */
    gfmDrawOrder_topFirst          = 1,
    /** Draw the objects at the bottom of the screen first */
    gfmDrawOrder_bottomFirst       = 2,
    /** Draw the newest objects first */
    gfmDrawOrder_newestFirst       = 3,
    /** Draw the older objects first */
    gfmDrawOrder_oldestFirst       = 4
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
 * Initialize the group; The group can be initialized with a few
 * pre-instantiated sprites; Those (and any other marked as such) will be
 * automatically freed by the group
 * 
 * @param  pCtx     The group
 * @param  initLen  How many nodes should be pre-allocated
 * @param  maxLen   Group's maximum length (0 if there's no limit)
 * @return          GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_ALLOC_FAILED
 */
gfmRV gfmGroup_init(gfmGroup *pCtx, int initLen, int maxLen);

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
 * @return       GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_ALLOC_FAILED
 */
gfmRV gfmGroup_recycle(gfmSprite **ppSpr, gfmGroup *pCtx);

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

