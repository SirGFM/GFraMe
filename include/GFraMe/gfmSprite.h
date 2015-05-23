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

gfmRV gfmSprite_setOffset(gfmSprite *pCtx, int offX, int offY);
gfmRV gfmSprite_getOffset(int *pOffX, int *pOffY, gfmSprite *pCtx);
gfmRV gfmSprite_getChild(void **ppChild, int *pType, gfmSprite *pCtx);
gfmRV gfmSprite_getObject(gfmObject **pObj, gfmSprite *pCtx);
gfmRV gfmSprite_setSpriteset(gfmSprite *pCtx, gfmSpriteset *pSset);
gfmRV gfmSprite_getSpriteset(gfmSpriteset **ppSset, gfmSprite *pCtx);
gfmRV gfmSprite_setFrame(gfmSprite *pCtx, int frame);
gfmRV gfmSprite_getFrame(int *pFrame, gfmSprite *pCtx);
gfmRV gfmSprite_draw(gfmSprite *pSpr, gfmCtx *pCtx);
gfmRV gfmSprite_addAnimation(gfmSprite *pCtx, int *pData, int numFrames,
        int fps, int doLoop);
gfmRV gfmSprite_addAnimations(gfmSprite *pCtx, int *pData, int dataLen);
gfmRV gfmSprite_playAnimation(gfmSprite *pCtx, int index);
//gfmRV gfmSprite_getAnimation(gfmAnimation **ppAnim, gfmSprite *pCtx);
gfmRV gfmSprite_didAnimationFinish(gfmSprite *pCtx);

#endif /* __GFMSPRITE_H__ */

