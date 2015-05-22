/**
 * @file src/gfmSprite.c
 * 
 * Represent a retangular 'object' that can be rendered to the screen; It has
 * its own gfmObject to handle the physics
 */
#include <GFraMe/gframe.h>
#include <GFraMe/gfmAssert.h>
#include <GFraMe/gfmError.h>
#include <GFraMe/gfmObject.h>
#include <GFraMe/gfmSprite.h>

#include <stdlib.h>
#include <string.h>

/** The gfmSprite structure */
struct stGFMSprite {
    /** The sprite's physical object */
    gfmObject *pObject;
    /** The sprite's 'child-class' (e.g., a player struct) */
    void *pChild;
    /** The sprite child's type */
    int childType;
    /** Horizontal offset between the object's top-left corner and the tile's */
    int offsetX;
    /** Vertical offset between the object's top-left corner and the tile's */
    int offsetY;
    // TODO Animation...
};

/** Size of gfmSprite */
const int sizeofGFMSprite = (int)sizeof(gfmSprite);

/**
 * Set the sprite's dimensions
 * 
 * @param  pCtx   The sprite
 * @param  width  The sprite's width
 * @param  height The sprite's height
 * @return        GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmSprite_setDimensions(gfmSprite *pCtx, int width, int height) {
    gfmRV rv;
    
    // Check only the sprites
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    // Call the 'super-class' function
    rv = gfmObject_setDimensions(pCtx->pObject, width, height);
__ret:
    return rv;
}

/**
 * Set the sprite's dimension
 * 
 * @param  pCtx  The sprite
 * @param  width The sprite's width
 * @return       GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmSprite_setHorizontalDimension(gfmSprite *pCtx, int width) {
    gfmRV rv;
    
    // Check only the sprites
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    // Call the 'super-class' function
    rv = gfmObject_setHorizontalDimension(pCtx->pObject, width);
__ret:
    return rv;
}

/**
 * Set the sprite's dimension
 * 
 * @param  pCtx   The sprite
 * @param  height The sprite's height
 * @return        GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmSprite_setVerticalDimension(gfmSprite *pCtx, int height) {
    gfmRV rv;
    
    // Check only the sprites
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    // Call the 'super-class' function
    rv = gfmObject_setVerticalDimension(pCtx->pObject, height);
__ret:
    return rv;
}

/**
 * Get the sprite's dimensions
 * 
 * @param  pWidth  The sprite's width
 * @param  pHeight The sprite's height
 * @param  pCtx    The sprite
 * @return         GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmSprite_getDimensions(int *pWidth, int *pHeight, gfmSprite *pCtx) {
    gfmRV rv;
    
    // Check only the sprites
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    // Call the 'super-class' function
    rv = gfmObject_getDimensions(pWidth, pHeight, pCtx->pObject);
__ret:
    return rv;
}

/**
 * Get the sprite's width
 * 
 * @param  pWidth The sprite's width
 * @param  pCtx   The sprite
 * @return        GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmSprite_getWidth(int *pWidth, gfmSprite *pCtx) {
    gfmRV rv;
    
    // Check only the sprites
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    // Call the 'super-class' function
    rv = gfmObject_getWidth(pWidth, pCtx->pObject);
__ret:
    return rv;
}

/**
 * Get the sprite's height
 * 
 * @param  pHeight The sprite's height
 * @param  pCtx    The sprite
 * @return         GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmSprite_getHeight(int *pHeight, gfmSprite *pCtx) {
    gfmRV rv;
    
    // Check only the sprites
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    // Call the 'super-class' function
    rv = gfmObject_getHeight(pHeight, pCtx->pObject);
__ret:
    return rv;
}

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
gfmRV gfmSprite_setPosition(gfmSprite *pCtx, int x, int y) {
    gfmRV rv;
    
    // Check only the sprites
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    // Call the 'super-class' function
    rv = gfmObject_setPosition(pCtx->pObject, x, y);
__ret:
    return rv;
}

/**
 * Set a sprite's horizontal position
 * 
 * NOTE: The anchor is the upper-left corner!
 * 
 * @param  pCtx The sprite
 * @param  x    The horizontal position
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmSprite_setHorizontalPosition(gfmSprite *pCtx, int x) {
    gfmRV rv;
    
    // Check only the sprites
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    // Call the 'super-class' function
    rv = gfmObject_setHorizontalPosition(pCtx->pObject, x);
__ret:
    return rv;
}

/**
 * Set a sprite's vertical position
 * 
 * NOTE: The anchor is the upper-left corner!
 * 
 * @param  pCtx The sprite
 * @param  y    The vertical position
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmSprite_setVerticalPosition(gfmSprite *pCtx, int y) {
    gfmRV rv;
    
    // Check only the sprites
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    // Call the 'super-class' function
    rv = gfmObject_setVerticalPosition(pCtx->pObject, y);
__ret:
    return rv;
}

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
gfmRV gfmSprite_getPosition(int *pX, int *pY, gfmSprite *pCtx) {
    gfmRV rv;
    
    // Check only the sprites
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    // Call the 'super-class' function
    rv = gfmObject_getPosition(pX, pY, pCtx->pObject);
__ret:
    return rv;
}

/**
 * Get the sprite's horizontal position
 * 
 * NOTE: The anchor is the upper-left corner!
 * 
 * @param  pX   The horizontal position
 * @param  pCtx The sprite
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmSprite_getHorizontalPosition(int *pX, gfmSprite *pCtx) {
    gfmRV rv;
    
    // Check only the sprites
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    // Call the 'super-class' function
    rv = gfmObject_getHorizontalPosition(pX, pCtx->pObject);
__ret:
    return rv;
}

/**
 * Get the sprite's vertical position
 * 
 * NOTE: The anchor is the upper-left corner!
 * 
 * @param  pY   The vertical position
 * @param  pCtx The sprite
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmSprite_getVerticalPosition(int *pY, gfmSprite *pCtx) {
    gfmRV rv;
    
    // Check only the sprites
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    // Call the 'super-class' function
    rv = gfmObject_getVerticalPosition(pY, pCtx->pObject);
__ret:
    return rv;
}

/**
 * Get the sprite's central position
 * 
 * @param  pX   The horizontal position
 * @param  pY   The vertical position
 * @param  pCtx The sprite
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_OBJECT_NOT_INITIALIZED
 */
gfmRV gfmSprite_getCenter(int *pX, int *pY, gfmSprite *pCtx) {
    gfmRV rv;
    
    // Check only the sprites
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    // Call the 'super-class' function
    rv = gfmObject_getCenter(pX, pY, pCtx->pObject);
__ret:
    return rv;
}

/**
 * Get the sprite's central position on the previous frame
 * 
 * @param  pX   The horizontal position
 * @param  pY   The vertical position
 * @param  pCtx The sprite
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_OBJECT_NOT_INITIALIZED
 */
gfmRV gfmSprite_getLastCenter(int *pX, int *pY, gfmSprite *pCtx) {
    gfmRV rv;
    
    // Check only the sprites
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    // Call the 'super-class' function
    rv = gfmObject_getLastCenter(pX, pY, pCtx->pObject);
__ret:
    return rv;
}

/**
 * Set the sprite's velocity
 * 
 * @param  pCtx The sprite
 * @param  vx   The horizontal velocity
 * @param  vy   The vertical velocity
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmSprite_setVelocity(gfmSprite *pCtx, double vx, double vy) {
    gfmRV rv;
    
    // Check only the sprites
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    // Call the 'super-class' function
    rv = gfmObject_setVelocity(pCtx->pObject, vx, vy);
__ret:
    return rv;
}

/**
 * Set the sprite's velocity
 * 
 * @param  pCtx The sprite
 * @param  vx   The horizontal velocity
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmSprite_setHorizontalVelocity(gfmSprite *pCtx, double vx) {
    gfmRV rv;
    
    // Check only the sprites
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    // Call the 'super-class' function
    rv = gfmObject_setHorizontalVelocity(pCtx->pObject, vx);
__ret:
    return rv;
}

/**
 * Set the sprite's velocity
 * 
 * @param  pCtx The sprite
 * @param  vy   The vertical velocity
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmSprite_setVerticalVelocity(gfmSprite *pCtx, double vy) {
    gfmRV rv;
    
    // Check only the sprites
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    // Call the 'super-class' function
    rv = gfmObject_setVerticalVelocity(pCtx->pObject, vy);
__ret:
    return rv;
}

/**
 * Get the sprite's velocity
 * 
 * @param  pVx  The horizontal velocity
 * @param  pVy  The vertical velocity
 * @param  pCtx The sprite
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmSprite_getVelocity(double *pVx, double *pVy, gfmSprite *pCtx) {
    gfmRV rv;
    
    // Check only the sprites
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    // Call the 'super-class' function
    rv = gfmObject_getVelocity(pVx, pVy, pCtx->pObject);
__ret:
    return rv;
}

/**
 * Get the sprite's velocity
 * 
 * @param  pVx  The horizontal velocity
 * @param  pCtx The sprite
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmSprite_getHorizontalVelocity(double *pVx, gfmSprite *pCtx) {
    gfmRV rv;
    
    // Check only the sprites
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    // Call the 'super-class' function
    rv = gfmObject_getHorizontalVelocity(pVx, pCtx->pObject);
__ret:
    return rv;
}

/**
 * Get the sprite's velocity
 * 
 * @param  pVy  The vertical velocity
 * @param  pCtx The sprite
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmSprite_getVerticalVelocity(double *pVy, gfmSprite *pCtx) {
    gfmRV rv;
    
    // Check only the sprites
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    // Call the 'super-class' function
    rv = gfmObject_getVerticalVelocity(pVy, pCtx->pObject);
__ret:
    return rv;
}

/**
 * Set the sprite's acceleration
 * 
 * @param  pCtx The sprite
 * @param  ax   The sprite's horizontal acceleration
 * @param  ay   The sprite's vertical acceleration
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmSprite_setAcceleration(gfmSprite *pCtx, double ax, double ay) {
    gfmRV rv;
    
    // Check only the sprites
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    // Call the 'super-class' function
    rv = gfmObject_setAcceleration(pCtx->pObject, ax, ay);
__ret:
    return rv;
}

/**
 * Set the sprite's acceleration
 * 
 * @param  pCtx The sprite
 * @param  ax   The sprite's horizontal acceleration
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmSprite_setHorizontalAcceleration(gfmSprite *pCtx, double ax) {
    gfmRV rv;
    
    // Check only the sprites
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    // Call the 'super-class' function
    rv = gfmObject_setHorizontalAcceleration(pCtx->pObject, ax);
__ret:
    return rv;
}

/**
 * Set the sprite's acceleration
 * 
 * @param  pCtx The sprite
 * @param  ay   The sprite's vertical acceleration
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmSprite_setVerticalAcceleration(gfmSprite *pCtx, double ay) {
    gfmRV rv;
    
    // Check only the sprites
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    // Call the 'super-class' function
    rv = gfmObject_setVerticalAcceleration(pCtx->pObject, ay);
__ret:
    return rv;
}

/**
 * Get the sprite's acceleration
 * 
 * @param  pAx  The sprite's horizontal acceleration
 * @param  pAy  The sprite's vertical acceleration
 * @param  pCtx The sprite
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmSprite_getAcceleration(double *pAx, double *pAy, gfmSprite *pCtx) {
    gfmRV rv;
    
    // Check only the sprites
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    // Call the 'super-class' function
    rv = gfmObject_getAcceleration(pAx, pAy, pCtx->pObject);
__ret:
    return rv;
}

/**
 * Get the sprite's acceleration
 * 
 * @param  pAx  The sprite's horizontal acceleration
 * @param  pCtx The sprite
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmSprite_getHorizontalAcceleration(double *pAx, gfmSprite *pCtx) {
    gfmRV rv;
    
    // Check only the sprites
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    // Call the 'super-class' function
    rv = gfmObject_getHorizontalAcceleration(pAx, pCtx->pObject);
__ret:
    return rv;
}

/**
 * Get the sprite's acceleration
 * 
 * @param  pAy  The sprite's vertical acceleration
 * @param  pCtx The sprite
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmSprite_getVerticalAcceleration(double *pAy, gfmSprite *pCtx) {
    gfmRV rv;
    
    // Check only the sprites
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    // Call the 'super-class' function
    rv = gfmObject_getVerticalAcceleration(pAy, pCtx->pObject);
__ret:
    return rv;
}

/**
 * Set the sprite's drag (i.e., how fast it will stop when there's no acc)
 * 
 * @param  pCtx The sprite
 * @param  dx   The horizontal drag
 * @param  dy   The vertical drag
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_NEGATIVE_DRAG
 */
gfmRV gfmSprite_setDrag(gfmSprite *pCtx, double dx, double dy) {
    gfmRV rv;
    
    // Check only the sprites
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    // Call the 'super-class' function
    rv = gfmObject_setDrag(pCtx->pObject, dx, dy);
__ret:
    return rv;
}

/**
 * Set the sprite's drag (i.e., how fast it will stop when there's no acc)
 * 
 * @param  pCtx The sprite
 * @param  dx   The horizontal drag
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_NEGATIVE_DRAG
 */
gfmRV gfmSprite_setHorizontalDrag(gfmSprite *pCtx, double dx) {
    gfmRV rv;
    
    // Check only the sprites
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    // Call the 'super-class' function
    rv = gfmObject_setHorizontalDrag(pCtx->pObject, dx);
__ret:
    return rv;
}

/**
 * Set the sprite's drag (i.e., how fast it will stop when there's no acc)
 * 
 * @param  pCtx The sprite
 * @param  dy   The vertical drag
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_NEGATIVE_DRAG
 */
gfmRV gfmSprite_setVerticalDrag(gfmSprite *pCtx, double dy) {
    gfmRV rv;
    
    // Check only the sprites
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    // Call the 'super-class' function
    rv = gfmObject_setVerticalDrag(pCtx->pObject, dy);
__ret:
    return rv;
}

/**
 * Get the sprite's drag (i.e., how fast it will stop when there's no acc)
 * 
 * @param  pDx  The horizontal drag
 * @param  pDy  The vertical drag
 * @param  pCtx The sprite
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmSprite_getDrag(double *pDx, double *pDy, gfmSprite *pCtx) {
    gfmRV rv;
    
    // Check only the sprites
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    // Call the 'super-class' functionSSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    
    rv = gfmObject_getDrag(pDx, pDy, pCtx->pObject);
__ret:
    return rv;
}

/**
 * Get the sprite's drag (i.e., how fast it will stop when there's no acc)
 * 
 * @param  pDx  The horizontal drag
 * @param  pCtx The sprite
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmSprite_getHorizontalDrag(double *pDx, gfmSprite *pCtx) {
    gfmRV rv;
    
    // Check only the sprites
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    // Call the 'super-class' function
    rv = gfmObject_getHorizontalDrag(pDx, pCtx->pObject);
__ret:
    return rv;
}

/**
 * Get the sprite's drag (i.e., how fast it will stop when there's no acc)
 * 
 * @param  pDy  The vertical drag
 * @param  pCtx The sprite
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmSprite_getVerticalDrag(double *pDy, gfmSprite *pCtx) {
    gfmRV rv;
    
    // Check only the sprites
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    // Call the 'super-class' function
    rv = gfmObject_getVerticalDrag(pDy, pCtx->pObject);
__ret:
    return rv;
}

/**
 * Get the sprite's child and type; ppChild mustn't be NULL, even if the sprite
 * has no "sub-class"
 * 
 * @param  ppChild The sprite's "sub-class"
 * @param  pType   The sprite's type
 * @param  pCtx The sprite
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmSprite_getChild(void **ppChild, int *pType, gfmSprite *pCtx) {
    gfmRV rv;
    
    // Check only the sprites
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    // TODO Implemente this
__ret:
    return rv;
}

/**
 * Force this sprite to stand immovable on collision
 * 
 * NOTE: An sprite can move through its physics even if fixed!
 * 
 * @param  pCtx The sprite
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmSprite_setFixed(gfmSprite *pCtx) {
    gfmRV rv;
    
    // Check only the sprites
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    // Call the 'super-class' function
    rv = gfmObject_setFixed(pCtx->pObject);
__ret:
    return rv;
}
/**
 * Allow this sprite to move on collision
 * 
 * @param  pCtx The sprite
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmSprite_setMovable(gfmSprite *pCtx) {
    gfmRV rv;
    
    // Check only the sprites
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    // Call the 'super-class' function
    rv = gfmObject_setMovable(pCtx->pObject);
__ret:
    return rv;
}

/**
 * Update the sprite; Its last collision status is cleared and the sprite's
 * properties are integrated using the Euler method
 * 
 * @param  pSpr The sprite
 * @param  pCtx The game's context
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_OBJECT_NOT_INITIALIZED
 */
gfmRV gfmSprite_update(gfmSprite *pSpr, gfmCtx *pCtx) {
    gfmRV rv;
    
    // Sanitize arguments
    ASSERT(pSpr, GFMRV_ARGUMENTS_BAD);
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    // Call the 'super-class' function
    rv = gfmObject_update(pSpr->pObject, pCtx);
    ASSERT_NR(rv == GFMRV_OK);
    
    // TODO Update animation
__ret:
    return rv;
}

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
        gfmSprite *pOther) {
    gfmRV rv;
    
    // Check only the sprites
    ASSERT(pSelf, GFMRV_ARGUMENTS_BAD);
    ASSERT(pOther, GFMRV_ARGUMENTS_BAD);
    // Call the 'super-class' function
    rv = gfmObject_getDistance(pDx, pDy, pSelf->pObject, pOther->pObject);
__ret:
    return rv;
}

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
        gfmSprite *pOther) {
    gfmRV rv;
    
    // Check only the sprites
    ASSERT(pSelf, GFMRV_ARGUMENTS_BAD);
    ASSERT(pOther, GFMRV_ARGUMENTS_BAD);
    // Call the 'super-class' function
    rv = gfmObject_getHorizontalDistance(pDx, pSelf->pObject, pOther->pObject);
__ret:
    return rv;
}

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
        gfmSprite *pOther) {
    gfmRV rv;
    
    // Check only the sprites
    ASSERT(pSelf, GFMRV_ARGUMENTS_BAD);
    ASSERT(pOther, GFMRV_ARGUMENTS_BAD);
    // Call the 'super-class' function
    rv = gfmObject_getVerticalDistance(pDy, pSelf->pObject, pOther->pObject);
__ret:
    return rv;
}

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
        gfmSprite *pOther) {
    gfmRV rv;
    
    // Check only the sprites
    ASSERT(pSelf, GFMRV_ARGUMENTS_BAD);
    ASSERT(pOther, GFMRV_ARGUMENTS_BAD);
    // Call the 'super-class' function
    rv = gfmObject_getHorizontalDistanced(pDx, pSelf->pObject, pOther->pObject);
__ret:
    return rv;
}

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
        gfmSprite *pOther) {
    gfmRV rv;
    
    // Check only the sprites
    ASSERT(pSelf, GFMRV_ARGUMENTS_BAD);
    ASSERT(pOther, GFMRV_ARGUMENTS_BAD);
    // Call the 'super-class' function
    rv = gfmObject_getVerticalDistanced(pDy, pSelf->pObject, pOther->pObject);
__ret:
    return rv;
}

/**
 * Check if a give point is inside the sprite
 * 
 * @param  pCtx The sprite
 * @param  x    The point's horizontal position
 * @param  y    The point's vertical position
 * @return      GFMRV_TRUE, GFMRV_FALSE, GFMRV_ARGUMENTS_BAD,
 *              GFMRV_OBJECT_NOT_INITIALIZED
 */
gfmRV gfmSprite_isPointInside(gfmSprite *pCtx, int x, int y) {
    gfmRV rv;
    
    // Check only the sprites
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    // Call the 'super-class' function
    rv = gfmObject_isPointInside(pCtx->pObject, x, y);
__ret:
    return rv;
}

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
gfmRV gfmSprite_isOverlaping(gfmSprite *pSelf, gfmSprite *pOther) {
    gfmRV rv;
    
    // Check only the sprites
    ASSERT(pSelf, GFMRV_ARGUMENTS_BAD);
    ASSERT(pOther, GFMRV_ARGUMENTS_BAD);
    // Call the 'super-class' function
    rv = gfmObject_isOverlaping(pSelf->pObject, pOther->pObject);
__ret:
    return rv;
}

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
gfmRV gfmSprite_collide(gfmSprite *pSelf, gfmSprite *pOther) {
    gfmRV rv;
    
    // Check only the sprites
    ASSERT(pSelf, GFMRV_ARGUMENTS_BAD);
    ASSERT(pOther, GFMRV_ARGUMENTS_BAD);
    // Call the 'super-class' function
    rv = gfmObject_collide(pSelf->pObject, pOther->pObject);
__ret:
    return rv;
}

/**
 * Separate two sprite in the X axis
 * 
 * @param  pSelf  An sprite
 * @param  pOther An sprite
 * @return        GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_OBJECT_NOT_INITIALIZED,
 *                GFMRV_OBJECTS_CANT_COLLIDE, GFMRV_COLLISION_NOT_TRIGGERED
 */
gfmRV gfmSprite_separateHorizontal(gfmSprite *pSelf, gfmSprite *pOther) {
    gfmRV rv;
    
    // Check only the sprites
    ASSERT(pSelf, GFMRV_ARGUMENTS_BAD);
    ASSERT(pOther, GFMRV_ARGUMENTS_BAD);
    // Call the 'super-class' function
    rv = gfmObject_separateHorizontal(pSelf->pObject, pOther->pObject);
__ret:
    return rv;
}

/**
 * Separate two sprite in the Y axis
 * 
 * @param  pSelf  An sprite
 * @param  pOther An sprite
 * @return        GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_OBJECT_NOT_INITIALIZED,
 *                GFMRV_OBJECTS_CANT_COLLIDE, GFMRV_COLLISION_NOT_TRIGGERED
 */
gfmRV gfmSprite_separateVertical(gfmSprite *pSelf, gfmSprite *pOther) {
    gfmRV rv;
    
    // Check only the sprites
    ASSERT(pSelf, GFMRV_ARGUMENTS_BAD);
    ASSERT(pOther, GFMRV_ARGUMENTS_BAD);
    // Call the 'super-class' function
    rv = gfmObject_separateVertical(pSelf->pObject, pOther->pObject);
__ret:
    return rv;
}

/**
 * Get the current collision/overlap flags
 * 
 * @param  pDir The collision direction
 * @param  pCtx The sprite
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmSprite_getCollision(gfmCollision *pDir, gfmSprite *pCtx) {
    gfmRV rv;
    
    // Check only the sprites
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    // Call the 'super-class' function
    rv = gfmObject_getCollision(pDir, pCtx->pObject);
__ret:
    return rv;
}

/**
 * Get the last frame's collision/overlap flags
 * 
 * @param  pDir The collision direction
 * @param  pCtx The sprite
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmSprite_getLastCollision(gfmCollision *pDir, gfmSprite *pCtx) {
    gfmRV rv;
    
    // Check only the sprites
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    // Call the 'super-class' function
    rv = gfmObject_getLastCollision(pDir, pCtx->pObject);
__ret:
    return rv;
}

/**
 * Get the collision/overlap flags from the last 'gfmSprite_isOverlaping' call
 * 
 * @param  pDir The collision direction
 * @param  pCtx The sprite
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmSprite_getCurrentCollision(gfmCollision *pDir, gfmSprite *pCtx) {
    gfmRV rv;
    
    // Check only the sprites
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    // Call the 'super-class' function
    rv = gfmObject_getCurrentCollision(pDir, pCtx->pObject);
__ret:
    return rv;
}

