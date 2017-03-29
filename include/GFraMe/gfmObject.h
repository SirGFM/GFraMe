/**
 * @file include/GFraMe/gfmObject.h
 * 
 * Basic physical object; It has an AABB, position, velocity, acceleration, drag
 * and current (and previous frame) collision info;
 * Since this is the base type to be passed to the quadtree for
 * overlaping/collision, it also has info about it's "child type" (e.g., a
 * gfmSprite pointer and the type T_GFMSPRITE)
 */
#ifndef __GFMOBJECT_STRUCT__
#define __GFMOBJECT_STRUCT__

/** 'Exports' the gfmObject structure */
typedef struct stGFMObject gfmObject;
/** 'Exports' the gfmCollision type */
typedef enum enGFMCollision gfmCollision;

#endif  /* __GFMOBJECT_STRUCT__ */

#ifndef __GFMOBJECT_H__
#define __GFMOBJECT_H__

#include <GFraMe/gframe.h>
#include <GFraMe/gfmError.h>
#include <GFraMe/gfmHitbox.h>

/** 'Exportable' size of gfmObject */
extern const int sizeofGFMObject;

/** Collision directions */
enum enGFMCollision {
    gfmCollision_none      = 0x0000,
    gfmCollision_left      = 0x0001,
    gfmCollision_right     = 0x0002,
    gfmCollision_up        = 0x0004,
    gfmCollision_down      = 0x0008,
    gfmCollision_lastLeft  = 0x0010,
    gfmCollision_lastRight = 0x0020,
    gfmCollision_lastUp    = 0x0040,
    gfmCollision_lastDown  = 0x0080,
    gfmCollision_instLeft  = 0x0100,
    gfmCollision_instRight = 0x0200,
    gfmCollision_instUp    = 0x0400,
    gfmCollision_instDown  = 0x0800,
    gfmCollision_cur       = 0x000F,
    gfmCollision_last      = 0x00F0,
    gfmCollision_inst      = 0x0F00,
    gfmCollision_hor       = 0x0003,
    gfmCollision_ver       = 0x000C,
    gfmCollision_instHor   = 0x0300,
    gfmCollision_instVer   = 0x0C00,
};

/**
 * Alloc a new gfmObject
 * 
 * @param  ppCtx The object
 * @return       GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_ALLOC_FAILED
 */
gfmRV gfmObject_getNew(gfmObject **ppCtx);

/**
 * Free a previously allocated gfmObject
 * 
 * @param  ppCtx The object
 * @return       GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmObject_free(gfmObject **ppCtx);

/**
 * Initialize a object given its top-left position and its dimensions
 * 
 * NOTE: Every one of the object's attributes are cleared out, on calling this
 * function
 * 
 * @param  pCtx   The object
 * @param  x      The object's horizontal position
 * @param  y      The object's vertical position
 * @param  width  The object's width
 * @param  height The object's height
 * @param  pChild The object's "sub-class" (e.g., a gfmSprite)
 * @param  type   The type of the object's "sub-class"
 * @return        GFMRV_OK, GFMRV_ARGUMENTS_BAD, ...
 */
gfmRV gfmObject_init(gfmObject *pCtx, int x, int y, int width, int height,
        void *pChild, int type);

/**
 * Clear the object
 * 
 * @param  pCtx   The object
 * @return        GFMRV_OK, GFMRV_ARGUMENTS_BAD, ...
 */
gfmRV gfmObject_clean(gfmObject *pCtx);

/**
 * Set the object's dimensions
 * 
 * @param  pCtx   The object
 * @param  width  The object's width
 * @param  height The object's height
 * @return        GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmObject_setDimensions(gfmObject *pCtx, int width, int height);

/**
 * Set the object's dimension
 * 
 * @param  pCtx  The object
 * @param  width The object's width
 * @return       GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmObject_setHorizontalDimension(gfmObject *pCtx, int width);

/**
 * Set the object's dimension
 * 
 * @param  pCtx   The object
 * @param  height The object's height
 * @return        GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmObject_setVerticalDimension(gfmObject *pCtx, int height);

/**
 * Get the object's dimensions
 * 
 * @param  pWidth  The object's width
 * @param  pHeight The object's height
 * @param  pCtx    The object
 * @return         GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmObject_getDimensions(int *pWidth, int *pHeight, gfmObject *pCtx);

/**
 * Get the object's width
 * 
 * @param  pWidth The object's width
 * @param  pCtx   The object
 * @return        GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmObject_getWidth(int *pWidth, gfmObject *pCtx);

/**
 * Get the object's height
 * 
 * @param  pHeight The object's height
 * @param  pCtx    The object
 * @return         GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmObject_getHeight(int *pHeight, gfmObject *pCtx);

/**
 * Set a object's position
 * 
 * NOTE: The anchor is the upper-left corner!
 * 
 * @param  pCtx The object
 * @param  x    The horizontal position
 * @param  y    The vertical position
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmObject_setPosition(gfmObject *pCtx, int x, int y);

/**
 * Set a object's horizontal position
 * 
 * NOTE: The anchor is the upper-left corner!
 * 
 * @param  pCtx The object
 * @param  x    The horizontal position
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmObject_setHorizontalPosition(gfmObject *pCtx, int x);

/**
 * Set a object's vertical position
 * 
 * NOTE: The anchor is the upper-left corner!
 * 
 * @param  pCtx The object
 * @param  y    The vertical position
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmObject_setVerticalPosition(gfmObject *pCtx, int y);

/**
 * Get the object's position
 * 
 * NOTE: The anchor is the upper-left corner!
 * 
 * @param  pX   The horizontal position
 * @param  pY   The vertical position
 * @param  pCtx The object
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmObject_getPosition(int *pX, int *pY, gfmObject *pCtx);

/**
 * Get the object's horizontal position
 * 
 * NOTE: The anchor is the upper-left corner!
 * 
 * @param  pX   The horizontal position
 * @param  pCtx The object
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmObject_getHorizontalPosition(int *pX, gfmObject *pCtx);

/**
 * Get the object's vertical position
 * 
 * NOTE: The anchor is the upper-left corner!
 * 
 * @param  pY   The vertical position
 * @param  pCtx The object
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmObject_getVerticalPosition(int *pY, gfmObject *pCtx);

/**
 * Set the object's central position
 * 
 * @param  [ in]pCtx The object
 * @param  [ in]x    The horizontal position
 * @param  [ in]y    The vertical position
 * @return           GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_OBJECT_NOT_INITIALIZED
 */
gfmRV gfmObject_setCenter(gfmObject *pCtx, int x, int y);

/**
 * Get the object's central position
 *
 * @param  pX   The horizontal position
 * @param  pY   The vertical position
 * @param  pCtx The object
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_OBJECT_NOT_INITIALIZED
 */
gfmRV gfmObject_getCenter(int *pX, int *pY, gfmObject *pCtx);

/**
 * Get the object's central position on the previous frame
 * 
 * @param  pX   The horizontal position
 * @param  pY   The vertical position
 * @param  pCtx The object
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_OBJECT_NOT_INITIALIZED
 */
gfmRV gfmObject_getLastCenter(int *pX, int *pY, gfmObject *pCtx);

/**
 * Set the object's velocity
 * 
 * @param  pCtx The object
 * @param  vx   The horizontal velocity
 * @param  vy   The vertical velocity
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmObject_setVelocity(gfmObject *pCtx, double vx, double vy);

/**
 * Set the object's velocity
 * 
 * @param  pCtx The object
 * @param  vx   The horizontal velocity
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmObject_setHorizontalVelocity(gfmObject *pCtx, double vx);

/**
 * Set the object's velocity
 * 
 * @param  pCtx The object
 * @param  vy   The vertical velocity
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmObject_setVerticalVelocity(gfmObject *pCtx, double vy);

/**
 * Get the object's velocity
 * 
 * @param  pVx  The horizontal velocity
 * @param  pVy  The vertical velocity
 * @param  pCtx The object
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmObject_getVelocity(double *pVx, double *pVy, gfmObject *pCtx);

/**
 * Get the object's velocity
 * 
 * @param  pVx  The horizontal velocity
 * @param  pCtx The object
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmObject_getHorizontalVelocity(double *pVx, gfmObject *pCtx);

/**
 * Get the object's velocity
 * 
 * @param  pVy  The vertical velocity
 * @param  pCtx The object
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmObject_getVerticalVelocity(double *pVy, gfmObject *pCtx);

/**
 * Set the object's acceleration
 * 
 * @param  pCtx The object
 * @param  ax   The object's horizontal acceleration
 * @param  ay   The object's vertical acceleration
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmObject_setAcceleration(gfmObject *pCtx, double ax, double ay);

/**
 * Set the object's acceleration
 * 
 * @param  pCtx The object
 * @param  ax   The object's horizontal acceleration
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmObject_setHorizontalAcceleration(gfmObject *pCtx, double ax);

/**
 * Set the object's acceleration
 * 
 * @param  pCtx The object
 * @param  ay   The object's vertical acceleration
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmObject_setVerticalAcceleration(gfmObject *pCtx, double ay);

/**
 * Get the object's acceleration
 * 
 * @param  pAx  The object's horizontal acceleration
 * @param  pAy  The object's vertical acceleration
 * @param  pCtx The object
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmObject_getAcceleration(double *pAx, double *pAy, gfmObject *pCtx);

/**
 * Get the object's acceleration
 * 
 * @param  pAx  The object's horizontal acceleration
 * @param  pCtx The object
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmObject_getHorizontalAcceleration(double *pAx, gfmObject *pCtx);

/**
 * Get the object's acceleration
 * 
 * @param  pAy  The object's vertical acceleration
 * @param  pCtx The object
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmObject_getVerticalAcceleration(double *pAy, gfmObject *pCtx);

/**
 * Set the object's drag (i.e., how fast it will stop when there's no acc)
 * 
 * @param  pCtx The object
 * @param  dx   The horizontal drag
 * @param  dy   The vertical drag
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_NEGATIVE_DRAG
 */
gfmRV gfmObject_setDrag(gfmObject *pCtx, double dx, double dy);

/**
 * Set the object's drag (i.e., how fast it will stop when there's no acc)
 * 
 * @param  pCtx The object
 * @param  dx   The horizontal drag
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_NEGATIVE_DRAG
 */
gfmRV gfmObject_setHorizontalDrag(gfmObject *pCtx, double dx);

/**
 * Set the object's drag (i.e., how fast it will stop when there's no acc)
 * 
 * @param  pCtx The object
 * @param  dy   The vertical drag
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_NEGATIVE_DRAG
 */
gfmRV gfmObject_setVerticalDrag(gfmObject *pCtx, double dy);

/**
 * Get the object's drag (i.e., how fast it will stop when there's no acc)
 * 
 * @param  pDx  The horizontal drag
 * @param  pDy  The vertical drag
 * @param  pCtx The object
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmObject_getDrag(double *pDx, double *pDy, gfmObject *pCtx);

/**
 * Get the object's drag (i.e., how fast it will stop when there's no acc)
 * 
 * @param  pDx  The horizontal drag
 * @param  pCtx The object
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmObject_getHorizontalDrag(double *pDx, gfmObject *pCtx);

/**
 * Get the object's drag (i.e., how fast it will stop when there's no acc)
 * 
 * @param  pDy  The vertical drag
 * @param  pCtx The object
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmObject_getVerticalDrag(double *pDy, gfmObject *pCtx);

/**
 * Get the object's child and type; ppChild mustn't be NULL, even if the object
 * has no "sub-class"
 * 
 * @param  ppChild The object's "sub-class"
 * @param  pType   The object's type
 * @param  pCtx    The object
 * @return         GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmObject_getChild(void **ppChild, int *pType, gfmObject *pCtx);

/**
 * Force this object to stand immovable on collision
 * 
 * NOTE: An object can move through its physics even if fixed!
 * 
 * @param  pCtx The object
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmObject_setFixed(gfmObject *pCtx);

/**
 * Allow this object to move on collision
 * 
 * @param  pCtx The object
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmObject_setMovable(gfmObject *pCtx);

/**
 * Enable continous collision for the given object.
 *
 * @param  [ in]pCtx The object
 * @return           GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmObject_enableContinousCollision(gfmObject *pCtx);

/**
 * Apply another object's translation into this object
 *
 * This is differente from manually calculating it because it doesn't clamp the
 * current position.
 *
 * @param  [ in]pCtx   The object
 * @param  [ in]pOther The other object
 * @return             GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmObject_applyDelta(gfmObject *pCtx, gfmObject *pOther);

/**
 * Apply another object's horizontal translation into this object
 *
 * This is differente from manually calculating it because it doesn't clamp the
 * current position.
 *
 * @param  [ in]pCtx   The object
 * @param  [ in]pOther The other object
 * @return             GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmObject_applyDeltaX(gfmObject *pCtx, gfmObject *pOther);

/**
 * Apply another object's vertical translation into this object
 *
 * This is differente from manually calculating it because it doesn't clamp the
 * current position.
 *
 * @param  [ in]pCtx   The object
 * @param  [ in]pOther The other object
 * @return             GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmObject_applyDeltaY(gfmObject *pCtx, gfmObject *pOther);

/**
 * Update the object; Its last collision status is cleared and the object's
 * properties are integrated using the Euler method
 * 
 * @param  pObj The object
 * @param  pCtx The game's context
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_OBJECT_NOT_INITIALIZED
 */
gfmRV gfmObject_update(gfmObject *pObj, gfmCtx *pCtx);

/**
 * Get the distance between two objects' centers
 * 
 * @param  pX     The horizontal distance
 * @param  pY     The vertical distance
 * @param  pSelf  An object
 * @param  pOther An object
 * @return        GFMRV_TRUE, GFMRV_FALSE, GFMRV_ARGUMENTS_BAD,
 *                GFMRV_OBJECT_NOT_INITIALIZED
 */
gfmRV gfmObject_getDistance(int *pDx, int *pDy, gfmObject *pSelf,
        gfmObject *pOther);

/**
 * Get the horizontal distance between two objects' centers
 * 
 * @param  pX     The horizontal distance
 * @param  pSelf  An object
 * @param  pOther An object
 * @return        GFMRV_TRUE, GFMRV_FALSE, GFMRV_ARGUMENTS_BAD,
 *                GFMRV_OBJECT_NOT_INITIALIZED
 */
gfmRV gfmObject_getHorizontalDistance(int *pDx, gfmObject *pSelf,
        gfmObject *pOther);

/**
 * Get the vertical distance between two objects' centers
 * 
 * @param  pY     The vertical distance
 * @param  pSelf  An object
 * @param  pOther An object
 * @return        GFMRV_TRUE, GFMRV_FALSE, GFMRV_ARGUMENTS_BAD,
 *                GFMRV_OBJECT_NOT_INITIALIZED
 */
gfmRV gfmObject_getVerticalDistance(int *pDy, gfmObject *pSelf,
        gfmObject *pOther);

/**
 * Get the horizontal distance between two objects' centers
 * 
 * @param  pX     The horizontal distance
 * @param  pSelf  An object
 * @param  pOther An object
 * @return        GFMRV_TRUE, GFMRV_FALSE, GFMRV_ARGUMENTS_BAD,
 *                GFMRV_OBJECT_NOT_INITIALIZED
 */
gfmRV gfmObject_getHorizontalDistanced(double *pDx, gfmObject *pSelf,
        gfmObject *pOther);

/**
 * Get the vertical distance between two objects' centers
 * 
 * @param  pY     The vertical distance
 * @param  pSelf  An object
 * @param  pOther An object
 * @return        GFMRV_TRUE, GFMRV_FALSE, GFMRV_ARGUMENTS_BAD,
 *                GFMRV_OBJECT_NOT_INITIALIZED
 */
gfmRV gfmObject_getVerticalDistanced(double *pDy, gfmObject *pSelf,
        gfmObject *pOther);

/**
 * Check if a give point is inside the object
 * 
 * @param  pCtx The object
 * @param  x    The point's horizontal position
 * @param  y    The point's vertical position
 * @return      GFMRV_TRUE, GFMRV_FALSE, GFMRV_ARGUMENTS_BAD,
 *              GFMRV_OBJECT_NOT_INITIALIZED
 */
gfmRV gfmObject_isPointInside(gfmObject *pCtx, int x, int y);

/**
 * Check if two objects are overlaping
 * 
 * NOTE: It fails to detect if an object was inside another one and is leaving
 * 
 * @param  pSelf  An object
 * @param  pOther An object
 * @return        GFMRV_TRUE, GFMRV_FALSE, GFMRV_ARGUMENTS_BAD,
 *                GFMRV_OBJECT_NOT_INITIALIZED
 */
gfmRV gfmObject_isOverlaping(gfmObject *pSelf, gfmObject *pOther);

/**
 * Check if an object just overlaped a hitbox
 *
 * @param  [ in]pObj    The object
 * @param  [ in]pHitbox The hitbox
 */
gfmRV gfmObject_justOverlapedHitbox(gfmObject *pObj, gfmHitbox *pHitbox);

/**
 * Check if two objects just started overlaping
 * 
 * NOTE: It fails to detect if an object was inside another one and is leaving
 * 
 * @param  pSelf  An object
 * @param  pOther An object
 * @return        GFMRV_TRUE, GFMRV_FALSE, GFMRV_ARGUMENTS_BAD,
 *                GFMRV_OBJECT_NOT_INITIALIZED
 */
gfmRV gfmObject_justOverlaped(gfmObject *pSelf, gfmObject *pOther);

/**
 * Separate an object and a hitbox in the Y axis
 * 
 * @param  [ in]pObj    The object
 * @param  [ in]pHitbox The hitbox
 */
gfmRV gfmObject_separateHorizontalHitbox(gfmObject *pObj, gfmHitbox *pHitbox);

/**
 * Separate an object and a hitbox in the Y axis
 * 
 * @param  [ in]pObj    The object
 * @param  [ in]pHitbox The hitbox
 */
gfmRV gfmObject_separateVerticalHitbox(gfmObject *pObj, gfmHitbox *pHitbox);

/**
 * Collide two objects
 * 
 * NOTE: It fails to detect if an object was inside another one and is leaving
 * 
 * @param  [ in]pObj    The object
 * @param  [ in]pHitbox The hitbox
 */
gfmRV gfmObject_collideHitbox(gfmObject *pObj, gfmHitbox *pHitbox);

/**
 * Collide two objects
 * 
 * NOTE: It fails to detect if an object was inside another one and is leaving
 * 
 * @param  pSelf  An object
 * @param  pOther An object
 * @return        GFMRV_TRUE, GFMRV_FALSE, GFMRV_ARGUMENTS_BAD,
 *                GFMRV_OBJECT_NOT_INITIALIZED, GFMRV_OBJECTS_CANT_COLLIDE
 */
gfmRV gfmObject_collide(gfmObject *pSelf, gfmObject *pOther);

/**
 * Separate two object in the X axis
 * 
 * @param  pSelf  An object
 * @param  pOther An object
 * @return        GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_OBJECT_NOT_INITIALIZED,
 *                GFMRV_OBJECTS_CANT_COLLIDE, GFMRV_COLLISION_NOT_TRIGGERED
 */
gfmRV gfmObject_separateHorizontal(gfmObject *pSelf, gfmObject *pOther);

/**
 * Separate two object in the Y axis
 * 
 * @param  pSelf  An object
 * @param  pOther An object
 * @return        GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_OBJECT_NOT_INITIALIZED,
 *                GFMRV_OBJECTS_CANT_COLLIDE, GFMRV_COLLISION_NOT_TRIGGERED
 */
gfmRV gfmObject_separateVertical(gfmObject *pSelf, gfmObject *pOther);

/**
 * Get the current collision/overlap flags
 * 
 * @param  pDir The collision direction
 * @param  pCtx The object
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmObject_getCollision(gfmCollision *pDir, gfmObject *pCtx);

/**
 * Get the last frame's collision/overlap flags
 * 
 * @param  pDir The collision direction
 * @param  pCtx The object
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmObject_getLastCollision(gfmCollision *pDir, gfmObject *pCtx);

/**
 * Get the collision/overlap flags from the last 'gfmObject_isOverlaping' call
 * 
 * @param  pDir The collision direction
 * @param  pCtx The object
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmObject_getCurrentCollision(gfmCollision *pDir, gfmObject *pCtx);

/**
 * Check if the object is overlaping with a line
 *
 * NOTE: The current implementation can't deal with lines that are too big. If
 * the algorithm detects the line as being too far from the object, it will
 * fail!
 *
 * @param  [ in]pCtx The object
 * @param  [ in]x0   Initial positional of the line (left-most)
 * @param  [ in]y0   Initial positional of the line
 * @param  [ in]x1   Final positional of the line (right-most)
 * @param  [ in]y1   Final positional of the line
 */
gfmRV gfmObject_overlapLine(gfmObject *pCtx, int x0, int y0, int x1, int y1);

/**
 * Set the type of the object's child
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
 * @param  [ in]pCtx The object
 * @param  [ in]type The object's child new type
 * @return           GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmObject_setType(gfmObject *pCtx, int type);

/**
 * Retrieve an object's boundary. If continous collision is enabled for the
 * object, the entire area it may have occupied from the previous frame to this
 * one is returned.
 *
 * @param  [out]pX      Object's position
 * @param  [out]pY      Object's position
 * @param  [out]pWidth  Object's dimensions
 * @param  [out]pHeight Object's dimensions
 * @param  [ in]pCtx    The object
 */
gfmRV gfmObject_getCollisionBoundary(int *pX, int *pY, int *pWidth
       , int *pHeight, gfmObject *pCtx);

#endif  /* __GFMOBJECT_H__ */

