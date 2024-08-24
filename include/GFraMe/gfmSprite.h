/**
 * @file include/GFraMe/gfmSprite.h
 * 
 * Represent a retangular 'object' that can be rendered to the screen; It has
 * its own gfmObject to handle the physics
 */
#ifndef __GFMSPRITE_STRUCT__
#define __GFMSPRITE_STRUCT__

/** 'Exports' the gfmSprite structure */
typedef struct stGFMSprite gfmSprite;

#endif /* __GFMSPRITE_STRUCT__ */

#ifndef __GFMSPRITE_H__
#define __GFMSPRITE_H__

#include <GFraMe/gframe.h>
#include <GFraMe/gfmError.h>
#include <GFraMe/gfmObject.h>
#include <GFraMe/gfmSpriteset.h>

/** 'Exportable' size of gfmSprite */
extern const int sizeofGFMSprite;

/**
 * Alloc a new gfmSprite
 * 
 * @param  ppCtx The sprite
 * @return       GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_ALLOC_FAILED
 */
gfmRV gfmSprite_getNew(gfmSprite **ppCtx);

/**
 * Free a previously allocated gfmSprite
 * 
 * @param  ppCtx The sprite
 * @return       GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmSprite_free(gfmSprite **ppCtx);

/**
 * Initialize a sprite given its top-left position and its dimensions
 * 
 * NOTE: Every one of the sprite's attributes are cleared out, on calling this
 * function
 * 
 * @param  pCtx   The sprite
 * @param  x      The sprite's horizontal position
 * @param  y      The sprite's vertical position
 * @param  width  The sprite's width
 * @param  height The sprite's height
 * @param  pSset  The sprite's spriteset
 * @param  offX   Tile's horizontal offset from the object's position
 * @param  offY   Tile's vertical offset from the object's position
 * @param  pChild The sprite's "sub-class" (e.g., a gfmSprite)
 * @param  type   The type of the sprite's "sub-class"
 * @return        GFMRV_OK, GFMRV_ARGUMENTS_BAD, ...
 */
gfmRV gfmSprite_init(gfmSprite *pCtx, int x, int y, int width, int height,
        gfmSpriteset *pSset, int offX, int offY, void *pChild, int type);

/**
 * Clear the sprite
 * 
 * @param  pCtx   The sprite
 * @return        GFMRV_OK, GFMRV_ARGUMENTS_BAD, ...
 */
gfmRV gfmSprite_clean(gfmSprite *pCtx);

/**
 * Reset the sprite's object (clearing it's velocity, acceleration, collision
 * flags, etc.); The object is kept at its current place
 * 
 * @param  pCtx The sprite
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmSprite_resetObject(gfmSprite *pCtx);

/**
 * Set the sprite's dimensions
 * 
 * @param  pCtx   The sprite
 * @param  width  The sprite's width
 * @param  height The sprite's height
 * @return        GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmSprite_setDimensions(gfmSprite *pCtx, int width, int height);

/**
 * Set the sprite's dimension
 * 
 * @param  pCtx  The sprite
 * @param  width The sprite's width
 * @return       GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmSprite_setHorizontalDimension(gfmSprite *pCtx, int width);

/**
 * Set the sprite's dimension
 * 
 * @param  pCtx   The sprite
 * @param  height The sprite's height
 * @return        GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmSprite_setVerticalDimension(gfmSprite *pCtx, int height);

/**
 * Get the sprite's dimensions
 * 
 * @param  pWidth  The sprite's width
 * @param  pHeight The sprite's height
 * @param  pCtx    The sprite
 * @return         GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmSprite_getDimensions(int *pWidth, int *pHeight, gfmSprite *pCtx);

/**
 * Get the sprite's width
 * 
 * @param  pWidth The sprite's width
 * @param  pCtx   The sprite
 * @return        GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmSprite_getWidth(int *pWidth, gfmSprite *pCtx);

/**
 * Get the sprite's height
 * 
 * @param  pHeight The sprite's height
 * @param  pCtx    The sprite
 * @return         GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmSprite_getHeight(int *pHeight, gfmSprite *pCtx);

/**
 * Set a sprite's position
 * 
 * NOTE: The anchor is the upper-left corner!
 * 
 * @param  pCtx The sprite
 * @param  x    The horizontal position
 * @param  y    The vertical position
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmSprite_setPosition(gfmSprite *pCtx, int x, int y);

/**
 * Set a sprite's horizontal position
 * 
 * NOTE: The anchor is the upper-left corner!
 * 
 * @param  pCtx The sprite
 * @param  x    The horizontal position
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmSprite_setHorizontalPosition(gfmSprite *pCtx, int x);

/**
 * Set a sprite's vertical position
 * 
 * NOTE: The anchor is the upper-left corner!
 * 
 * @param  pCtx The sprite
 * @param  y    The vertical position
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmSprite_setVerticalPosition(gfmSprite *pCtx, int y);

/**
 * Get the sprite's position
 * 
 * NOTE: The anchor is the upper-left corner!
 * 
 * @param  pX   The horizontal position
 * @param  pY   The vertical position
 * @param  pCtx The sprite
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmSprite_getPosition(int *pX, int *pY, gfmSprite *pCtx);

/**
 * Get the sprite's horizontal position
 * 
 * NOTE: The anchor is the upper-left corner!
 * 
 * @param  pX   The horizontal position
 * @param  pCtx The sprite
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmSprite_getHorizontalPosition(int *pX, gfmSprite *pCtx);

/**
 * Get the sprite's vertical position
 * 
 * NOTE: The anchor is the upper-left corner!
 * 
 * @param  pY   The vertical position
 * @param  pCtx The sprite
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmSprite_getVerticalPosition(int *pY, gfmSprite *pCtx);

/**
 * Set the sprite's central position
 * 
 * @param  [ in]pCtx The sprite
 * @param  [ in]x    The horizontal position
 * @param  [ in]y    The vertical position
 * @return           GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_OBJECT_NOT_INITIALIZED
 */
gfmRV gfmSprite_setCenter(gfmSprite *pCtx, int x, int y);

/**
 * Get the sprite's central position
 *
 * @param  pX   The horizontal position
 * @param  pY   The vertical position
 * @param  pCtx The sprite
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_OBJECT_NOT_INITIALIZED
 */
gfmRV gfmSprite_getCenter(int *pX, int *pY, gfmSprite *pCtx);

/**
 * Get the sprite's central position on the previous frame
 * 
 * @param  pX   The horizontal position
 * @param  pY   The vertical position
 * @param  pCtx The sprite
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_OBJECT_NOT_INITIALIZED
 */
gfmRV gfmSprite_getLastCenter(int *pX, int *pY, gfmSprite *pCtx);

/**
 * Set the sprite's velocity
 * 
 * @param  pCtx The sprite
 * @param  vx   The horizontal velocity
 * @param  vy   The vertical velocity
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmSprite_setVelocity(gfmSprite *pCtx, double vx, double vy);

/**
 * Set the sprite's velocity
 * 
 * @param  pCtx The sprite
 * @param  vx   The horizontal velocity
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmSprite_setHorizontalVelocity(gfmSprite *pCtx, double vx);

/**
 * Set the sprite's velocity
 * 
 * @param  pCtx The sprite
 * @param  vy   The vertical velocity
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmSprite_setVerticalVelocity(gfmSprite *pCtx, double vy);

/**
 * Get the sprite's velocity
 * 
 * @param  pVx  The horizontal velocity
 * @param  pVy  The vertical velocity
 * @param  pCtx The sprite
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmSprite_getVelocity(double *pVx, double *pVy, gfmSprite *pCtx);

/**
 * Get the sprite's velocity
 * 
 * @param  pVx  The horizontal velocity
 * @param  pCtx The sprite
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmSprite_getHorizontalVelocity(double *pVx, gfmSprite *pCtx);

/**
 * Get the sprite's velocity
 * 
 * @param  pVy  The vertical velocity
 * @param  pCtx The sprite
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmSprite_getVerticalVelocity(double *pVy, gfmSprite *pCtx);

/**
 * Set the sprite's acceleration
 * 
 * @param  pCtx The sprite
 * @param  ax   The sprite's horizontal acceleration
 * @param  ay   The sprite's vertical acceleration
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmSprite_setAcceleration(gfmSprite *pCtx, double ax, double ay);

/**
 * Set the sprite's acceleration
 * 
 * @param  pCtx The sprite
 * @param  ax   The sprite's horizontal acceleration
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmSprite_setHorizontalAcceleration(gfmSprite *pCtx, double ax);

/**
 * Set the sprite's acceleration
 * 
 * @param  pCtx The sprite
 * @param  ay   The sprite's vertical acceleration
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmSprite_setVerticalAcceleration(gfmSprite *pCtx, double ay);

/**
 * Get the sprite's acceleration
 * 
 * @param  pAx  The sprite's horizontal acceleration
 * @param  pAy  The sprite's vertical acceleration
 * @param  pCtx The sprite
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmSprite_getAcceleration(double *pAx, double *pAy, gfmSprite *pCtx);

/**
 * Get the sprite's acceleration
 * 
 * @param  pAx  The sprite's horizontal acceleration
 * @param  pCtx The sprite
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmSprite_getHorizontalAcceleration(double *pAx, gfmSprite *pCtx);

/**
 * Get the sprite's acceleration
 * 
 * @param  pAy  The sprite's vertical acceleration
 * @param  pCtx The sprite
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmSprite_getVerticalAcceleration(double *pAy, gfmSprite *pCtx);

/**
 * Set the sprite's drag (i.e., how fast it will stop when there's no acc)
 * 
 * @param  pCtx The sprite
 * @param  dx   The horizontal drag
 * @param  dy   The vertical drag
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_NEGATIVE_DRAG
 */
gfmRV gfmSprite_setDrag(gfmSprite *pCtx, double dx, double dy);

/**
 * Set the sprite's drag (i.e., how fast it will stop when there's no acc)
 * 
 * @param  pCtx The sprite
 * @param  dx   The horizontal drag
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_NEGATIVE_DRAG
 */
gfmRV gfmSprite_setHorizontalDrag(gfmSprite *pCtx, double dx);

/**
 * Set the sprite's drag (i.e., how fast it will stop when there's no acc)
 * 
 * @param  pCtx The sprite
 * @param  dy   The vertical drag
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_NEGATIVE_DRAG
 */
gfmRV gfmSprite_setVerticalDrag(gfmSprite *pCtx, double dy);

/**
 * Get the sprite's drag (i.e., how fast it will stop when there's no acc)
 * 
 * @param  pDx  The horizontal drag
 * @param  pDy  The vertical drag
 * @param  pCtx The sprite
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmSprite_getDrag(double *pDx, double *pDy, gfmSprite *pCtx);

/**
 * Get the sprite's drag (i.e., how fast it will stop when there's no acc)
 * 
 * @param  pDx  The horizontal drag
 * @param  pCtx The sprite
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmSprite_getHorizontalDrag(double *pDx, gfmSprite *pCtx);

/**
 * Get the sprite's drag (i.e., how fast it will stop when there's no acc)
 * 
 * @param  pDy  The vertical drag
 * @param  pCtx The sprite
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmSprite_getVerticalDrag(double *pDy, gfmSprite *pCtx);

/**
 * Get the sprite's child and type; ppChild mustn't be NULL, even if the sprite
 * has no "sub-class"
 * 
 * @param  ppChild The sprite's "sub-class"
 * @param  pType   The sprite's type
 * @param  pCtx The sprite
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmSprite_getChild(void **ppChild, int *pType, gfmSprite *pCtx);

/**
 * Force this sprite to stand immovable on collision
 * 
 * NOTE: An sprite can move through its physics even if fixed!
 * 
 * @param  pCtx The sprite
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmSprite_setFixed(gfmSprite *pCtx);
/**
 * Allow this sprite to move on collision
 * 
 * @param  pCtx The sprite
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmSprite_setMovable(gfmSprite *pCtx);

/**
 * Apply another sprite's translation into this sprite
 *
 * This is differente from manually calculating it because it doesn't clamp the
 * current position.
 *
 * @param  [ in]pCtx   The sprite
 * @param  [ in]pOther The other sprite
 * @return             GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmSprite_applyDelta(gfmSprite *pCtx, gfmSprite *pOther);

/**
 * Apply another sprite's horizontal translation into this sprite
 *
 * This is differente from manually calculating it because it doesn't clamp the
 * current position.
 *
 * @param  [ in]pCtx   The sprite
 * @param  [ in]pOther The other sprite
 * @return             GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmSprite_applyDeltaX(gfmSprite *pCtx, gfmSprite *pOther);

/**
 * Apply another sprite's vertical translation into this sprite
 *
 * This is differente from manually calculating it because it doesn't clamp the
 * current position.
 *
 * @param  [ in]pCtx   The sprite
 * @param  [ in]pOther The other sprite
 * @return             GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmSprite_applyDeltaY(gfmSprite *pCtx, gfmSprite *pOther);

/**
 * Update the sprite; Its last collision status is cleared and the sprite's
 * properties are integrated using the Euler method
 * 
 * @param  pSpr The sprite
 * @param  pCtx The game's context
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_OBJECT_NOT_INITIALIZED
 */
gfmRV gfmSprite_update(gfmSprite *pSpr, gfmCtx *pCtx);

/**
 * Get the distance between two sprites' centers
 * 
 * @param  pX     The horizontal distance
 * @param  pY     The vertical distance
 * @param  pSelf  An sprite
 * @param  pOther An sprite
 * @return        GFMRV_TRUE, GFMRV_FALSE, GFMRV_ARGUMENTS_BAD,
 *                GFMRV_OBJECT_NOT_INITIALIZED
 */
gfmRV gfmSprite_getDistance(int *pDx, int *pDy, gfmSprite *pSelf,
        gfmSprite *pOther);

/**
 * Get the horizontal distance between two sprites' centers
 * 
 * @param  pX     The horizontal distance
 * @param  pSelf  An sprite
 * @param  pOther An sprite
 * @return        GFMRV_TRUE, GFMRV_FALSE, GFMRV_ARGUMENTS_BAD,
 *                GFMRV_OBJECT_NOT_INITIALIZED
 */
gfmRV gfmSprite_getHorizontalDistance(int *pDx, gfmSprite *pSelf,
        gfmSprite *pOther);

/**
 * Get the vertical distance between two sprites' centers
 * 
 * @param  pY     The vertical distance
 * @param  pSelf  An sprite
 * @param  pOther An sprite
 * @return        GFMRV_TRUE, GFMRV_FALSE, GFMRV_ARGUMENTS_BAD,
 *                GFMRV_OBJECT_NOT_INITIALIZED
 */
gfmRV gfmSprite_getVerticalDistance(int *pDy, gfmSprite *pSelf,
        gfmSprite *pOther);

/**
 * Get the horizontal distance between two sprites' centers
 * 
 * @param  pX     The horizontal distance
 * @param  pSelf  An sprite
 * @param  pOther An sprite
 * @return        GFMRV_TRUE, GFMRV_FALSE, GFMRV_ARGUMENTS_BAD,
 *                GFMRV_OBJECT_NOT_INITIALIZED
 */
gfmRV gfmSprite_getHorizontalDistanced(double *pDx, gfmSprite *pSelf,
        gfmSprite *pOther);

/**
 * Get the vertical distance between two sprites' centers
 * 
 * @param  pY     The vertical distance
 * @param  pSelf  An sprite
 * @param  pOther An sprite
 * @return        GFMRV_TRUE, GFMRV_FALSE, GFMRV_ARGUMENTS_BAD,
 *                GFMRV_OBJECT_NOT_INITIALIZED
 */
gfmRV gfmSprite_getVerticalDistanced(double *pDy, gfmSprite *pSelf,
        gfmSprite *pOther);

/**
 * Check if a give point is inside the sprite
 * 
 * @param  pCtx The sprite
 * @param  x    The point's horizontal position
 * @param  y    The point's vertical position
 * @return      GFMRV_TRUE, GFMRV_FALSE, GFMRV_ARGUMENTS_BAD,
 *              GFMRV_OBJECT_NOT_INITIALIZED
 */
gfmRV gfmSprite_isPointInside(gfmSprite *pCtx, int x, int y);

/**
 * Check if two sprites are overlaping
 * 
 * NOTE: It fails to detect if an sprite was inside another one and is leaving
 * 
 * @param  pSelf  An sprite
 * @param  pOther An sprite
 * @return        GFMRV_TRUE, GFMRV_FALSE, GFMRV_ARGUMENTS_BAD,
 *                GFMRV_OBJECT_NOT_INITIALIZED
 */
gfmRV gfmSprite_isOverlaping(gfmSprite *pSelf, gfmSprite *pOther);

/**
 * Check if two sprites just started overlaping
 * 
 * NOTE: It fails to detect if an sprite was inside another one and is leaving
 * 
 * @param  pSelf  An sprite
 * @param  pOther An sprite
 * @return        GFMRV_TRUE, GFMRV_FALSE, GFMRV_ARGUMENTS_BAD,
 *                GFMRV_OBJECT_NOT_INITIALIZED
 */
gfmRV gfmSprite_justOverlaped(gfmSprite *pSelf, gfmSprite *pOther);

/**
 * Collide two sprites
 * 
 * NOTE: It fails to detect if an sprite was inside another one and is leaving
 * 
 * @param  pSelf  An sprite
 * @param  pOther An sprite
 * @return        GFMRV_TRUE, GFMRV_FALSE, GFMRV_ARGUMENTS_BAD,
 *                GFMRV_OBJECT_NOT_INITIALIZED, GFMRV_OBJECTS_CANT_COLLIDE
 */
gfmRV gfmSprite_collide(gfmSprite *pSelf, gfmSprite *pOther);

/**
 * Separate two sprite in the X axis
 * 
 * @param  pSelf  An sprite
 * @param  pOther An sprite
 * @return        GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_OBJECT_NOT_INITIALIZED,
 *                GFMRV_OBJECTS_CANT_COLLIDE, GFMRV_COLLISION_NOT_TRIGGERED
 */
gfmRV gfmSprite_separateHorizontal(gfmSprite *pSelf, gfmSprite *pOther);

/**
 * Separate two sprite in the Y axis
 * 
 * @param  pSelf  An sprite
 * @param  pOther An sprite
 * @return        GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_OBJECT_NOT_INITIALIZED,
 *                GFMRV_OBJECTS_CANT_COLLIDE, GFMRV_COLLISION_NOT_TRIGGERED
 */
gfmRV gfmSprite_separateVertical(gfmSprite *pSelf, gfmSprite *pOther);

/**
 * Get the current collision/overlap flags
 * 
 * @param  pDir The collision direction
 * @param  pCtx The sprite
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmSprite_getCollision(gfmCollision *pDir, gfmSprite *pCtx);

/**
 * Get the last frame's collision/overlap flags
 * 
 * @param  pDir The collision direction
 * @param  pCtx The sprite
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmSprite_getLastCollision(gfmCollision *pDir, gfmSprite *pCtx);

/**
 * Get the collision/overlap flags from the last 'gfmSprite_isOverlaping' call
 * 
 * @param  pDir The collision direction
 * @param  pCtx The sprite
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmSprite_getCurrentCollision(gfmCollision *pDir, gfmSprite *pCtx);

/**
 * Set the sprite's (i.e., image/tile) offset from the object's hitbox
 * 
 * @param  pCtx The sprite
 * @param  offX Tile's horizontal offset from the object's position
 * @param  offY Tile's vertical offset from the object's position
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_SPRITE_NOT_INITIALIZED
 */
gfmRV gfmSprite_setOffset(gfmSprite *pCtx, int offX, int offY);

/**
 * Set the sprite's (i.e., image/tile) offset from the object's hitbox
 * 
 * @param  pOffX Tile's horizontal offset from the object's position
 * @param  pOffY Tile's vertical offset from the object's position
 * @param  pCtx  The sprite
 * @return       GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_SPRITE_NOT_INITIALIZED
 */
gfmRV gfmSprite_getOffset(int *pOffX, int *pOffY, gfmSprite *pCtx);

/**
 * Get the sprite's sub-class (e.g, a player, an enemy, etc)
 * 
 * NOTE: Even if the sprite has no sub-class, this function's ppChild param
 * mustn't be NULL
 * 
 * @param  ppChild The sub-class struct
 * @param  pType   The sub-class type
 * @param  pCtx    The sprite
 * @return         GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_SPRITE_NOT_INITIALIZED
 */
gfmRV gfmSprite_getChild(void **ppChild, int *pType, gfmSprite *pCtx);

/**
 * Get the sprite's super-class (i.e., a gfmObject)
 * 
 * @param  ppObj The object
 * @param  pCtx  The sprite
 * @return       GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_SPRITE_NOT_INITIALIZED
 */
gfmRV gfmSprite_getObject(gfmObject **ppObj, gfmSprite *pCtx);

/**
 * Set the sprite's spriteset
 * 
 * @param  pCtx  The sprite
 * @param  pSset The sprite's spriteset
 * @return       GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_SPRITE_NOT_INITIALIZED
 */
gfmRV gfmSprite_setSpriteset(gfmSprite *pCtx, gfmSpriteset *pSset);

/**
 * Get the sprite's spriteset
 * 
 * @param  ppSset The sprite's spriteset
 * @param  pCtx   The sprite
 * @return        GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_SPRITE_NOT_INITIALIZED
 */
gfmRV gfmSprite_getSpriteset(gfmSpriteset **ppSset, gfmSprite *pCtx);

/**
 * Set the current frame; Any playing animation will be stopped!
 * 
 * @param  pCtx  The sprite
 * @param  frame The frame
 * @return       GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_SPRITE_NOT_INITIALIZED
 */
gfmRV gfmSprite_setFrame(gfmSprite *pCtx, int frame);

/**
 * Get the current frame
 * 
 * @param  pFrame The frame
 * @param  pCtx   The sprite
 * @return        GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_SPRITE_NOT_INITIALIZED
 */
gfmRV gfmSprite_getFrame(int *pFrame, gfmSprite *pCtx);

/**
 * Set the direction the sprite is facing (either forward or backward)
 * 
 * @param  pCtx      The sprite
 * @param  isFlipped Whether it's flipped
 * @return           GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_SPRITE_NOT_INITIALIZED
 */
gfmRV gfmSprite_setDirection(gfmSprite *pCtx, int isFlipped);

/**
 * Get the direction the sprite is facing (either forward or backward)
 * 
 * @param  pFlipped Whether it's flipped
 * @param  pCtx     The sprite
 * @return          GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_SPRITE_NOT_INITIALIZED
 */
gfmRV gfmSprite_getDirection(int *pFlipped, gfmSprite *pCtx);

/**
 * Render the sprite on the screen (if it's inside it)
 * 
 * @param  pSpr The sprite
 * @param  pCtx The game's context
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_SPRITE_NOT_INITIALIZED
 */
gfmRV gfmSprite_draw(gfmSprite *pSpr, gfmCtx *pCtx);

/**
 * Add a animation to the sprite
 * 
 * @param  pIndex    Animation's index on this sprite
 * @param  pCtx      The sprite
 * @param  pData     Animation's frames
 * @param  numFrames Number of frame on animation
 * @param  fps       Animation's framerate
 * @param  doLoop    Whether the animation should loop or not
 */
gfmRV gfmSprite_addAnimation(int *pIndex, gfmSprite *pCtx, int *pData,
        int numFrames, int fps, int doLoop);

/**
 * Add a batch of animations to the sprite;
 * The array must be organized in the following format:
 * 
 * pData[0] = anim_0.numFrames
 * pData[1] = anim_0.fps
 * pData[2] = anim_0.doLoop
 * pData[3] = anim_0.frame_0
 * ...
 * pData[3 + anim_0.numFrames - 1] = anim_0.lastFrame
 * pData[3 + anim_0.numFrames] = anim_1.numFrames
 * ...
 * 
 * And so on.
 * 
 * @param  pCtx    The sprite
 * @param  pData   The batch of animations
 * @param  dataLen How many integers there are in pData
 * @return           GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_ALLOC_FAILED
 */
gfmRV gfmSprite_addAnimations(gfmSprite *pCtx, int *pData, int dataLen);

/**
 * Add a batch of animations from a static buffer; Read
 * gfmSprite_addAnimations's documentation from the expected format
 * 
 * @param  pCtx  The sprite
 * @param  pData The batch of animations
 * @return       GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_TILEANIM_EXTANT,
 *               GFMRV_ALLOC_FAILED
 */
#define gfmSprite_addAnimationsStatic(pCtx, pData) \
        gfmSprite_addAnimations(pCtx, pData, sizeof(pData) / sizeof(int))

/**
 * Resets the currently playing animation
 * 
 * @param  pCtx  The sprite
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_NO_ANIMATION_PLAYING
 */
gfmRV gfmSprite_resetAnimation(gfmSprite *pCtx);

/**
 * Resets and plays an animation
 * 
 * @param  pCtx  The sprite
 * @param  index Index of the animation to be played
 * @return       GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_INVALID_INDEX
 */
gfmRV gfmSprite_playAnimation(gfmSprite *pCtx, int index);

/**
 * Get the index of the currently playing animation
 * 
 * @param  pIndex Index of the animation
 * @param  pCtx   The sprite
 * @return        GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_NO_ANIMATION_PLAYING
 */
gfmRV gfmSprite_getAnimationIndex(int *pIndex, gfmSprite *pCtx);

/**
 * Returns whether the animation have already looped
 * 
 * @param  pCtx The sprite
 * @return      GFMRV_TRUE, GFMRV_FALSE, GFMRV_ARGUMENTS_BAD,
 *              GFMRV_NO_ANIMATION_PLAYING
 */
gfmRV gfmSprite_didAnimationLoop(gfmSprite *pCtx);

/**
 * Returns whether the animation just looped
 * 
 * @param  pCtx The sprite
 * @return      GFMRV_TRUE, GFMRV_FALSE, GFMRV_ARGUMENTS_BAD,
 *              GFMRV_NO_ANIMATION_PLAYING
 */
gfmRV gfmSprite_didAnimationJustLoop(gfmSprite *pCtx);

/**
 * Returns whether the animation just change the frame
 * 
 * @param  pCtx The sprite
 * @return      GFMRV_TRUE, GFMRV_FALSE, GFMRV_ARGUMENTS_BAD,
 *              GFMRV_NO_ANIMATION_PLAYING
 */
gfmRV gfmSprite_didAnimationJustChangeFrame(gfmSprite *pCtx);

/**
 * Returns whether the animation finished running
 * 
 * @param  pCtx The sprite
 * @return      GFMRV_TRUE, GFMRV_FALSE, GFMRV_ARGUMENTS_BAD,
 *              GFMRV_NO_ANIMATION_PLAYING
 */
gfmRV gfmSprite_didAnimationFinish(gfmSprite *pCtx);

/**
 * Set the type of the sprite's child
 *
 * TL;DR: Use this functions only if you know what you are doing! (and if you
 * are colliding members of a gfmGroup)
 *
 * To simulate inheritance, every sprite has a pointer to a child object. A
 * 'type' is used to define how that pointer should be dereferenced. Therefore,
 * one usually needn't modify a sprite's type after it was initialized.
 *
 * However, there are cases in which it might be interesting to change a sprite
 * type. If, for example, there's a gfmGroup for object's hitbox, one could want
 * to represent different hitboxes as different types (e.g., a type for the
 * player's bullet, another for a sword slash and another for enemies bullets).
 *
 * In that case, all types would still represent the same child object (a
 * gfmGroupNode). However, after changing its types, one could treat them
 * differently while colliding.
 *
 * @param  [ in]pCtx The sprite
 * @param  [ in]type The sprite's child new type
 * @return           GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmSprite_setType(gfmSprite *pCtx, int type);

/**
 * Change the sprite's specialized object.
 *
 * This function is only intended to be used
 * if the sprite is initialized to a temporary, stack-allocated object.
 *
 * @param  [ in]pCtx   The sprite
 * @param  [ in]pChild The sprite's "sub-class" (e.g., a player)
 * @param  [ in]type   The sprite's child new type
 * @return             GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmSprite_setChild(gfmSprite *pCtx, void *pChild, int type);

/**
 * Check if the sprite is overlaping with a line
 *
 * NOTE: The current implementation can't deal with lines that are too big. If
 * the algorithm detects the line as being too far from the sprite, it will
 * fail!
 *
 * @param  [ in]pCtx The sprite
 * @param  [ in]x0   Initial positional of the line (left-most)
 * @param  [ in]y0   Initial positional of the line
 * @param  [ in]x1   Final positional of the line (right-most)
 * @param  [ in]y1   Final positional of the line
 */
gfmRV gfmSprite_overlapLine(gfmSprite *pCtx, int x0, int y0, int x1, int y1);

/**
 * Check if two sprites overlaped each other since the last frame, even if they
 * fully passed through each other.
 *
 * Somewhat based on this:
 *   http://www.gamasutra.com/view/feature/131790/simple_intersection_tests_for_games.php?page=3
 *
 * NOTE: This function doesn't require continous collision to be enabled on
 * sprites.
 *
 * @param  [ in]pSelf  One of the sprites
 * @param  [ in]pOther The other sprite
 */
gfmRV gfmSprite_sweepJustOverlaped(gfmSprite *pSelf, gfmSprite *pOther);

/**
 * Try to collide two sprites, even if they fully passed each other since the
 * last frame.
 *
 * For this test, one of the sprites must be considered the reference for the
 * collision. If a collision is detected, this sprite will be moved normally,
 * but the other sprite's position will be displaced to respect the reference's
 * movement.
 *
 * NOTE 1: gfmSprite_sweepJustOverlaped must have been called before hand
 * NOTE 2: This function doesn't require continous collision to be enabled on
 * sprites.
 *
 * @param  [ in]pRef   The reference sprite
 * @param  [ in]pOther The other sprite (that may be pushed)
 */
gfmRV gfmSprite_sweepCollision(gfmSprite *pRef, gfmSprite *pOther);


#endif /* __GFMSPRITE_H__ */

