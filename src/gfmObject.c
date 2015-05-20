/**
 * @file include/GFraMe/gfmObject.h
 * 
 * Basic physical object; It has an AABB, position, velocity, acceleration, drag
 * and current (and previous frame) collision info;
 * Since this is the base type to be passed to the quadtree for
 * overlaping/collision, it also has info about it's "child type" (e.g., a
 * gfmSprite pointer and the type T_GFMSPRITE)
 */
#include <GFraMe/gfmAssert.h>
#include <GFraMe/gfmError.h>
#include <GFraMe/gfmObject.h>

#include <stdlib.h>
#include <string.h>

/** The gfmObject structure */
struct stGFMObject {
    /** Current horizontal position */
    int x;
    /** Current vertical position */
    int y;
    /** Current accumulated (i.e., double) horizontal position */
    double dx;
    /** Current accumulated (i.e., double) vertical position */
    double dy;
    /** Previous accumulated (i.e., double) horizontal position */
    double ldx;
    /** Previous accumulated (i.e., double) vertical position */
    double ldy;
    /** Horizontal velocity */
    double vx;
    /** Vertical velocity */
    double vy;
    /** Horizontal acceleration */
    double ax;
    /** Vertical acceleration */
    double ay;
    /** Rate at which speed goed back to 0, if there's no horizontal acc */
    double dragX;
    /** Rate at which speed goed back to 0, if there's no vertical acc */
    double dragY;
    /** Object's half width; Part of its AABB */
    double halfWidth;
    /** Object's half height; Part of its AABB */
    double halfHeight;
    /** Whether the object's position was just set */
    int justMoved;
    /** Type of the child object */
    int type;
    /** Pointer to the child object */
    void *pChild;
};

/** Size of gfmObject */
const int sizeofGFMObject = (int)sizeof(gfmObject);

/**
 * Alloc a new gfmObject
 * 
 * @param  ppCtx The gfmObject
 * @return       GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_ALLOC_FAILED
 */
gfmRV gfmObject_getNew(gfmObject **ppCtx) {
    gfmRV rv;
    
    // Sanitize arguments
    ASSERT(ppCtx, GFMRV_ARGUMENTS_BAD);
    ASSERT(!(*ppCtx), GFMRV_ARGUMENTS_BAD);
    
    // Alloc the object
    *ppCtx = (gfmObject*)malloc(sizeof(gfmObject));
    ASSERT(*ppCtx, GFMRV_ALLOC_FAILED);
    // Clear it up
    memset(*ppCtx, 0x0, sizeof(gfmObject));
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Free a previously allocated gfmObject
 * 
 * @param  ppCtx The object
 * @return       GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmObject_free(gfmObject **ppCtx) {
    gfmRV rv;
    
    // Sanitize arguments
    ASSERT(ppCtx, GFMRV_ARGUMENTS_BAD);
    ASSERT(!(*ppCtx), GFMRV_ARGUMENTS_BAD);
    
    // Clean the object
    gfmObject_clean(*ppCtx);
    // And release it
    free(*ppCtx);
    *ppCtx = 0;
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Initialize a object given its top-left position and its dimensions
 * 
 * NOTE: Every one of the object's attributes are cleared out, on calling this
 * function
 * NOTE 2: It's actually OK for pChild to be NULL; e.g.: a spike tile on a
 * tilemap may have a type but no child
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
        void *pChild, int type) {
    gfmRV rv;
    
    // Sanitize arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    ASSERT(width > 0, GFMRV_ARGUMENTS_BAD);
    ASSERT(height > 0, GFMRV_ARGUMENTS_BAD);
    
    // Clear it up
    rv = gfmObject_clean(pCtx);
    ASSERT_NR(rv == GFMRV_OK);
    
    // Set the object's position
    pCtx->x = x;
    pCtx->y = y;
    // Must also set the previous position, to avoid collision errors
    pCtx->dx = x;
    pCtx->dy = y;
    pCtx->ldx = x;
    pCtx->ldy = y;
    
    // Set the object's dimensions
    pCtx->halfWidth = width / 2;
    pCtx->halfHeight = height / 2;
    
    // Set the object's child
    pCtx->pChild = pChild;
    pCtx->type = type;
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Clear the object
 * 
 * @param  pCtx   The object
 * @return        GFMRV_OK, GFMRV_ARGUMENTS_BAD, ...
 */
gfmRV gfmObject_clean(gfmObject *pCtx) {
    gfmRV rv;
    
    // Sanitize arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    
    // Clean it up
    memset(pCtx, 0x0, sizeof(gfmObject));
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Set the object's dimensions
 * 
 * @param  pCtx   The object
 * @param  width  The object's width
 * @param  height The object's height
 * @return        GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmObject_setDimensions(gfmObject *pCtx, int width, int height) {
    gfmRV rv;
    
    // Sanitize arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    ASSERT(width > 0, GFMRV_ARGUMENTS_BAD);
    ASSERT(height > 0, GFMRV_ARGUMENTS_BAD);
    
    // Set both dimensions
    rv = gfmObject_setHorizontalDimension(pCtx, width);
    ASSERT_NR(rv == GFMRV_OK);
    rv = gfmObject_setVerticalDimension(pCtx, height);
    ASSERT_NR(rv == GFMRV_OK);
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Set the object's dimension
 * 
 * @param  pCtx  The object
 * @param  width The object's width
 * @return       GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmObject_setHorizontalDimension(gfmObject *pCtx, int width) {
    gfmRV rv;
    
    // Sanitize arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    ASSERT(width > 0, GFMRV_ARGUMENTS_BAD);
    
    // Set the width
    pCtx->halfWidth = width / 2;
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Set the object's dimension
 * 
 * @param  pCtx   The object
 * @param  height The object's height
 * @return        GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmObject_setVerticalDimension(gfmObject *pCtx, int height) {
    gfmRV rv;
    
    // Sanitize arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    ASSERT(height > 0, GFMRV_ARGUMENTS_BAD);
    
    // Set the height
    pCtx->halfHeight = height / 2;
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Get the object's dimensions
 * 
 * @param  pWidth  The object's width
 * @param  pHeight The object's height
 * @param  pCtx    The object
 * @return         GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmObject_getDimensions(int *pWidth, int *pHeight, gfmObject *pCtx) {
    gfmRV rv;
    
    // Sanitize arguments
    ASSERT(pWidth, GFMRV_ARGUMENTS_BAD);
    ASSERT(pHeight, GFMRV_ARGUMENTS_BAD);
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    
    rv = gfmObject_getWidth(pWidth, pCtx);
    ASSERT_NR(rv == GFMRV_OK);
    rv = gfmObject_getHeight(pHeight, pCtx);
    ASSERT_NR(rv == GFMRV_OK);
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Get the object's width
 * 
 * @param  pWidth The object's width
 * @param  pCtx   The object
 * @return        GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmObject_getWidth(int *pWidth, gfmObject *pCtx) {
    gfmRV rv;
    
    // Sanitize arguments
    ASSERT(pWidth, GFMRV_ARGUMENTS_BAD);
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    
    *pWidth = pCtx->halfWidth * 2;
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Get the object's height
 * 
 * @param  pHeight The object's height
 * @param  pCtx    The object
 * @return         GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmObject_getHeight(int *pHeight, gfmObject *pCtx) {
    gfmRV rv;
    
    // Sanitize arguments
    ASSERT(pHeight, GFMRV_ARGUMENTS_BAD);
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    
    *pHeight = pCtx->halfHeight * 2;
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Set a object's position
 * 
 * @param  pCtx The object
 * @param  x    The horizontal position
 * @param  y    The vertical position
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmObject_setPosition(gfmObject *pCtx, int x, int y) {
    gfmRV rv;
    
    // Sanitize arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    
    // Set the position
    rv = gfmObject_setHorizontalPosition(pCtx, x);
    ASSERT_NR(rv == GFMRV_OK);
    rv = gfmObject_setVerticalPosition(pCtx, y);
    ASSERT_NR(rv == GFMRV_OK);
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Set a object's horizontal position
 * 
 * @param  pCtx The object
 * @param  x    The horizontal position
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmObject_setHorizontalPosition(gfmObject *pCtx, int x) {
    gfmRV rv;
    
    // Sanitize arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    
    // Set both the position and the previous position
    pCtx->x = x;
    pCtx->dx = (double)x;
    pCtx->ldx = (double)x;
    // Mark the object as having moved
    pCtx->justMoved = 1;
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Set a object's vertical position
 * 
 * @param  pCtx The object
 * @param  y    The vertical position
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmObject_setVerticalPosition(gfmObject *pCtx, int y) {
    gfmRV rv;
    
    // Sanitize arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    
    // Set both the position and the previous position
    pCtx->y = y;
    pCtx->dy = (double)y;
    pCtx->ldy = (double)y;
    // Mark the object as having moved
    pCtx->justMoved = 1;
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Get the object's position
 * 
 * @param  pX   The horizontal position
 * @param  pY   The vertical position
 * @param  pCtx The object
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmObject_getPosition(int *pX, int *pY, gfmObject *pCtx) {
    gfmRV rv;
    
    // Sanitize arguments
    ASSERT(pX, GFMRV_ARGUMENTS_BAD);
    ASSERT(pY, GFMRV_ARGUMENTS_BAD);
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    
    // Get the position
    rv = gfmObject_getHorizontalPosition(pX, pCtx);
    ASSERT_NR(rv == GFMRV_OK);
    rv = gfmObject_getVerticalPosition(pY, pCtx);
    ASSERT_NR(rv == GFMRV_OK);
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Get the object's horizontal position
 * 
 * @param  pX   The horizontal position
 * @param  pCtx The object
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmObject_getHorizontalPosition(int *pX, gfmObject *pCtx) {
    gfmRV rv;
    
    // Sanitize arguments
    ASSERT(pX, GFMRV_ARGUMENTS_BAD);
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    
    // Get the object's horizontal position
    *pX = pCtx->x;
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Get the object's vertical position
 * 
 * @param  pY   The vertical position
 * @param  pCtx The object
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmObject_getVerticalPosition(int *pY, gfmObject *pCtx) {
    gfmRV rv;
    
    // Sanitize arguments
    ASSERT(pY, GFMRV_ARGUMENTS_BAD);
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    
    // Get the object's horizontal position
    *pY = pCtx->y;
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Get the object's central position
 * 
 * @param  pX   The horizontal position
 * @param  pY   The vertical position
 * @param  pCtx The object
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmObject_getCenter(int *pX, int *pY, gfmObject *pCtx) {
    gfmRV rv;
    
    // Sanitize arguments
    ASSERT(pX, GFMRV_ARGUMENTS_BAD);
    ASSERT(pY, GFMRV_ARGUMENTS_BAD);
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    
    // Get the position
    rv = gfmObject_getHorizontalPosition(pX, pCtx);
    ASSERT_NR(rv == GFMRV_OK);
    rv = gfmObject_getVerticalPosition(pY, pCtx);
    ASSERT_NR(rv == GFMRV_OK);
    // Offset it to the center
    *pX += pCtx->halfWidth;
    *pY += pCtx->halfHeight;
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Set the object's velocity
 * 
 * @param  pCtx The object
 * @param  vx   The horizontal velocity
 * @param  vy   The vertical velocity
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmObject_setVelocity(gfmObject *pCtx, double vx, double vy) {
    gfmRV rv;
    
    // Sanitize arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    
    // Set the velocity
    rv = gfmObject_setHorizontalVelocity(pCtx, vx);
    ASSERT_NR(rv == GFMRV_OK);
    rv = gfmObject_setVerticalVelocity(pCtx, vy);
    ASSERT_NR(rv == GFMRV_OK);
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Set the object's velocity
 * 
 * @param  pCtx The object
 * @param  vx   The horizontal velocity
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmObject_setHorizontalVelocity(gfmObject *pCtx, double vx) {
    gfmRV rv;
    
    // Sanitize arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    
    // Set the velocity
    pCtx->vx = vx;
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Set the object's velocity
 * 
 * @param  pCtx The object
 * @param  vy   The vertical velocity
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmObject_setVerticalVelocity(gfmObject *pCtx, double vy) {
    gfmRV rv;
    
    // Sanitize arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    
    // Set the velocity
    pCtx->vy = vy;
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Get the object's velocity
 * 
 * @param  pVx  The horizontal velocity
 * @param  pVy  The vertical velocity
 * @param  pCtx The object
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmObject_getVelocity(double *pVx, double *pVy, gfmObject *pCtx) {
    gfmRV rv;
    
    // Sanitize arguments
    ASSERT(pVx, GFMRV_ARGUMENTS_BAD);
    ASSERT(pVy, GFMRV_ARGUMENTS_BAD);
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    
    // Get the velocity
    rv = gfmObject_getHorizontalVelocity(pVx, pCtx);
    ASSERT_NR(rv == GFMRV_OK);
    rv = gfmObject_getVerticalVelocity(pVy, pCtx);
    ASSERT_NR(rv == GFMRV_OK);
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Get the object's velocity
 * 
 * @param  pVx  The horizontal velocity
 * @param  pCtx The object
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmObject_getHorizontalVelocity(double *pVx, gfmObject *pCtx) {
    gfmRV rv;
    
    // Sanitize arguments
    ASSERT(pVx, GFMRV_ARGUMENTS_BAD);
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    
    // Get the velocity
    *pVx = pCtx->vx;
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Get the object's velocity
 * 
 * @param  pVy  The vertical velocity
 * @param  pCtx The object
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmObject_getVerticalVelocity(double *pVy, gfmObject *pCtx) {
    gfmRV rv;
    
    // Sanitize arguments
    ASSERT(pVy, GFMRV_ARGUMENTS_BAD);
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    
    // Get the velocity
    *pVy = pCtx->vy;
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Set the object's acceleration
 * 
 * @param  pCtx The object
 * @param  ax   The object's horizontal acceleration
 * @param  ay   The object's vertical acceleration
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmObject_setAcceleration(gfmObject *pCtx, double ax, double ay) {
    gfmRV rv;
    
    // Sanitize arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    
    // Set the object's acceleration
    rv = gfmObject_setHorizontalAcceleration(pCtx, ax);
    ASSERT_NR(rv == GFMRV_OK);
    rv = gfmObject_setVerticalAcceleration(pCtx, ay);
    ASSERT_NR(rv == GFMRV_OK);
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Set the object's acceleration
 * 
 * @param  pCtx The object
 * @param  ax   The object's horizontal acceleration
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmObject_setHorizontalAcceleration(gfmObject *pCtx, double ax) {
    gfmRV rv;
    
    // Sanitize arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    
    // Set the object's acceleration
    pCtx->ax = ax;
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Set the object's acceleration
 * 
 * @param  pCtx The object
 * @param  ay   The object's vertical acceleration
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmObject_setVerticalAcceleration(gfmObject *pCtx, double ay) {
    gfmRV rv;
    
    // Sanitize arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    
    // Set the object's acceleration
    pCtx->ay = ay;
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Get the object's acceleration
 * 
 * @param  pAx  The object's horizontal acceleration
 * @param  pAy  The object's vertical acceleration
 * @param  pCtx The object
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmObject_getAcceleration(double *pAx, double *pAy, gfmObject *pCtx) {
    gfmRV rv;
    
    // Sanitize arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    ASSERT(pAx, GFMRV_ARGUMENTS_BAD);
    ASSERT(pAy, GFMRV_ARGUMENTS_BAD);
    
    // Get the object's acceleration
    rv = gfmObject_getHorizontalAcceleration(pAx, pCtx);
    ASSERT_NR(rv == GFMRV_OK);
    rv = gfmObject_getVerticalAcceleration(pAy, pCtx);
    ASSERT_NR(rv == GFMRV_OK);
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Get the object's acceleration
 * 
 * @param  pAx  The object's horizontal acceleration
 * @param  pCtx The object
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmObject_getHorizontalAcceleration(double *pAx, gfmObject *pCtx) {
    gfmRV rv;
    
    // Sanitize arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    ASSERT(pAx, GFMRV_ARGUMENTS_BAD);
    
    // Get the object's acceleration
    *pAx = pCtx->ax;
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Get the object's acceleration
 * 
 * @param  pAy  The object's vertical acceleration
 * @param  pCtx The object
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmObject_getVerticalAcceleration(double *pAy, gfmObject *pCtx) {
    gfmRV rv;
    
    // Sanitize arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    ASSERT(pAy, GFMRV_ARGUMENTS_BAD);
    
    // Get the object's acceleration
    *pAy = pCtx->ay;
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Set the object's drag (i.e., how fast it will stop when there's no acc)
 * 
 * @param  pCtx The object
 * @param  dx   The horizontal drag
 * @param  dy   The vertical drag
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmObject_setDrag(gfmObject *pCtx, double dx, double dy) {
    gfmRV rv;
    
    // Sanitize arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    
    // Set the object's drag
    rv = gfmObject_setHorizontalDrag(pCtx, dx);
    ASSERT_NR(rv == GFMRV_OK);
    rv = gfmObject_setVerticalDrag(pCtx, dy);
    ASSERT_NR(rv == GFMRV_OK);
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Set the object's drag (i.e., how fast it will stop when there's no acc)
 * 
 * @param  pCtx The object
 * @param  dx   The horizontal drag
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmObject_setHorizontalDrag(gfmObject *pCtx, double dx) {
    gfmRV rv;
    
    // Sanitize arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    
    // Set the object's drag
    pCtx->dragX = dx;
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Set the object's drag (i.e., how fast it will stop when there's no acc)
 * 
 * @param  pCtx The object
 * @param  dy   The vertical drag
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmObject_setVerticalDrag(gfmObject *pCtx, double dy) {
    gfmRV rv;
    
    // Sanitize arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    
    // Set the object's drag
    pCtx->dragY = dy;
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Get the object's drag (i.e., how fast it will stop when there's no acc)
 * 
 * @param  pDx  The horizontal drag
 * @param  pDy  The vertical drag
 * @param  pCtx The object
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmObject_getDrag(double *pDx, double *pDy, gfmObject *pCtx) {
    gfmRV rv;
    
    // Sanitize arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    ASSERT(pDx, GFMRV_ARGUMENTS_BAD);
    ASSERT(pDy, GFMRV_ARGUMENTS_BAD);
    
    // Get the object's drag
    rv = gfmObject_getHorizontalDrag(pDx, pCtx);
    ASSERT_NR(rv == GFMRV_OK);
    rv = gfmObject_getVerticalDrag(pDy, pCtx);
    ASSERT_NR(rv == GFMRV_OK);
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Get the object's drag (i.e., how fast it will stop when there's no acc)
 * 
 * @param  pDx  The horizontal drag
 * @param  pCtx The object
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmObject_getHorizontalDrag(double *pDx, gfmObject *pCtx) {
    gfmRV rv;
    
    // Sanitize arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    ASSERT(pDx, GFMRV_ARGUMENTS_BAD);
    
    // Get the object's drag
    *pDx = pCtx->dragX;
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Get the object's drag (i.e., how fast it will stop when there's no acc)
 * 
 * @param  pDy  The vertical drag
 * @param  pCtx The object
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmObject_getVerticalDrag(double *pDy, gfmObject *pCtx) {
    gfmRV rv;
    
    // Sanitize arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    ASSERT(pDy, GFMRV_ARGUMENTS_BAD);
    
    // Get the object's drag
    *pDy = pCtx->dragY;
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Get the object's child and type; ppChild mustn't be NULL, even if the object
 * has no "sub-class"
 * 
 * @param  ppChild The object's "sub-class"
 * @param  pType   The object's type
 * @param  pCtx The object
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmObject_getChild(void **ppChild, int *pType, gfmObject *pCtx) {
    gfmRV rv;
    
    // Sanitize arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    ASSERT(ppChild, GFMRV_ARGUMENTS_BAD);
    ASSERT(pType, GFMRV_ARGUMENTS_BAD);
    
    // Get the object's child and type
    *ppChild = pCtx->pChild;
    *pType = pCtx->type;
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

gfmRV gfmObject_isPointInside(gfmObject *pCtx, int x, int y) {
    return GFMRV_FUNCTION_NOT_IMPLEMENTED;
}

