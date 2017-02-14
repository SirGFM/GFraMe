/**
 * @file include/GFraMe/gfmObject.h
 * 
 * Basic physical object; It has an AABB, position, velocity, acceleration and
 * current (and previous frame) collision info;
 * Since this is the base type to be passed to the quadtree for
 * overlaping/collision, it also has info about it's "child type" (e.g., a
 * gfmSprite pointer and the type T_GFMSPRITE)
 */
#include <GFraMe/gframe.h>
#include <GFraMe/gfmAssert.h>
#include <GFraMe/gfmError.h>
#include <GFraMe/gfmHitbox.h>
#include <GFraMe/gfmObject.h>
#include <GFraMe/gfmTypes.h>

#include <GFraMe_int/gfmFixedPoint.h>
#include <GFraMe_int/gfmGeometry.h>
#include <GFraMe_int/gfmHitbox.h>

#include <stdlib.h>
#include <string.h>

enum {
    gfmFlags_isFixed    = 0x10000
  , gfmFlags_currentBit = 0
  , gfmFlags_lastBit    = 4
  , gfmFlags_instBit    = 8
};

/** The gfmObject structure */
struct stGFMObject {
    /** The object's hitbox (i.e., its transform) */
    gfmHitbox t;
    /** Collision and fixed flags */
    uint32_t flags;
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
};

/** Size of gfmObject */
const int sizeofGFMObject = (int)sizeof(gfmObject);

/**
 * Set a object's horizontal position (as double)
 * 
 * NOTE: The anchor is the upper-left corner!
 * 
 * @param  pCtx The object
 * @param  x    The horizontal position
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
static gfmRV _int_gfmObject_setHorizontalPosition(gfmObject *pCtx, double x) {
    gfmRV rv;
    
    // Sanitize arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    
    // Set both the position and the previous position
    pCtx->t.x = (int)x;
    pCtx->dx = x;
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Set a object's vertical position (as double)
 * 
 * NOTE: The anchor is the upper-left corner!
 * 
 * @param  pCtx The object
 * @param  y    The vertical position
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
static gfmRV _int_gfmObject_setVerticalPosition(gfmObject *pCtx, double y) {
    gfmRV rv;
    
    // Sanitize arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    
    // Set both the position and the previous position
    pCtx->t.y = (int)y;
    pCtx->dy = y;
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

#if 0
/**
 * Set a object's position (as doubles)
 * 
 * NOTE: The anchor is the upper-left corner!
 * 
 * @param  pCtx The object
 * @param  x    The horizontal position
 * @param  y    The vertical position
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
static gfmRV _int_gfmObject_setPosition(gfmObject *pCtx, double x, double y) {
    gfmRV rv;
    
    // Sanitize arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    
    // Set the position
    rv = _int_gfmObject_setHorizontalPosition(pCtx, x);
    ASSERT_NR(rv == GFMRV_OK);
    rv = _int_gfmObject_setVerticalPosition(pCtx, y);
    ASSERT_NR(rv == GFMRV_OK);
    
    rv = GFMRV_OK;
__ret:
    return rv;
}
#endif

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
    ASSERT(*ppCtx, GFMRV_ARGUMENTS_BAD);
    
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
    pCtx->t.x = x;
    pCtx->t.y = y;
    // Must also set the previous position, to avoid collision errors
    pCtx->dx = x;
    pCtx->dy = y;
    pCtx->ldx = x;
    pCtx->ldy = y;
    
    // Set the object's dimensions
    pCtx->t.hw = width / 2;
    pCtx->t.hh = height / 2;
    
    // Set the object's child
    pCtx->t.pContext = pChild;
    pCtx->t.type = type;
    pCtx->t.innerType = gfmType_object;
    
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
    pCtx->t.hw = width / 2;
    
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
    pCtx->t.hh = height / 2;
    
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
    
    *pWidth = pCtx->t.hw * 2;
    
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
    
    *pHeight = pCtx->t.hh * 2;
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

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
 * NOTE: The anchor is the upper-left corner!
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
    pCtx->t.x = x;
    pCtx->dx = (double)x;
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Set a object's vertical position
 * 
 * NOTE: The anchor is the upper-left corner!
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
    pCtx->t.y = y;
    pCtx->dy = (double)y;
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

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
 * NOTE: The anchor is the upper-left corner!
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
    *pX = pCtx->t.x;
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Get the object's vertical position
 * 
 * NOTE: The anchor is the upper-left corner!
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
    *pY = pCtx->t.y;
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Set the object's central position
 *
 * @param  [ in]pCtx The object
 * @param  [ in]x    The horizontal position
 * @param  [ in]y    The vertical position
 * @return           GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_OBJECT_NOT_INITIALIZED
 */
gfmRV gfmObject_setCenter(gfmObject *pCtx, int x, int y) {
    gfmRV rv;

    /* Sanitize arguments */
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    /* Check that the object was initialized */
    ASSERT(pCtx->t.hw > 0, GFMRV_OBJECT_NOT_INITIALIZED);
    ASSERT(pCtx->t.hh > 0, GFMRV_OBJECT_NOT_INITIALIZED);

    /* Adjust the position */
    x -= pCtx->t.hw;
    y -= pCtx->t.hh;

    rv = gfmObject_setPosition(pCtx, x, y);
__ret:
    return rv;
}

/**
 * Get the object's central position
 * 
 * @param  pX   The horizontal position
 * @param  pY   The vertical position
 * @param  pCtx The object
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_OBJECT_NOT_INITIALIZED
 */
gfmRV gfmObject_getCenter(int *pX, int *pY, gfmObject *pCtx) {
    gfmRV rv;
    
    // Sanitize arguments
    ASSERT(pX, GFMRV_ARGUMENTS_BAD);
    ASSERT(pY, GFMRV_ARGUMENTS_BAD);
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    // Check that the object was initialized
    ASSERT(pCtx->t.hw > 0, GFMRV_OBJECT_NOT_INITIALIZED);
    ASSERT(pCtx->t.hh > 0, GFMRV_OBJECT_NOT_INITIALIZED);
    
    // Get the position
    rv = gfmObject_getHorizontalPosition(pX, pCtx);
    ASSERT_NR(rv == GFMRV_OK);
    rv = gfmObject_getVerticalPosition(pY, pCtx);
    ASSERT_NR(rv == GFMRV_OK);
    // Offset it to the center
    *pX += pCtx->t.hw;
    *pY += pCtx->t.hh;
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Get the object's central position on the previous frame
 * 
 * @param  pX   The horizontal position
 * @param  pY   The vertical position
 * @param  pCtx The object
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_OBJECT_NOT_INITIALIZED
 */
gfmRV gfmObject_getLastCenter(int *pX, int *pY, gfmObject *pCtx) {
    gfmRV rv;
    
    // Sanitize arguments
    ASSERT(pX, GFMRV_ARGUMENTS_BAD);
    ASSERT(pY, GFMRV_ARGUMENTS_BAD);
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    ASSERT(pCtx->t.innerType == gfmType_object, GFMRV_INVALID_TYPE);
    // Check that the object was initialized
    ASSERT(pCtx->t.hw > 0, GFMRV_OBJECT_NOT_INITIALIZED);
    ASSERT(pCtx->t.hh > 0, GFMRV_OBJECT_NOT_INITIALIZED);
    
    // Get the position
    *pX = (int)pCtx->ldx;
    *pY = (int)pCtx->ldy;
    // Offset it to the center
    *pX += pCtx->t.hw;
    *pY += pCtx->t.hh;
    
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
    ASSERT(pCtx->t.innerType == gfmType_object, GFMRV_INVALID_TYPE);
    
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
    ASSERT(pCtx->t.innerType == gfmType_object, GFMRV_INVALID_TYPE);
    
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
    ASSERT(pCtx->t.innerType == gfmType_object, GFMRV_INVALID_TYPE);
    
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
    ASSERT(pCtx->t.innerType == gfmType_object, GFMRV_INVALID_TYPE);
    
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
    ASSERT(pCtx->t.innerType == gfmType_object, GFMRV_INVALID_TYPE);
    
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
    ASSERT(pCtx->t.innerType == gfmType_object, GFMRV_INVALID_TYPE);
    
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
    ASSERT(pCtx->t.innerType == gfmType_object, GFMRV_INVALID_TYPE);
    
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
    ASSERT(pCtx->t.innerType == gfmType_object, GFMRV_INVALID_TYPE);
    
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
    ASSERT(pCtx->t.innerType == gfmType_object, GFMRV_INVALID_TYPE);
    
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
    ASSERT(pCtx->t.innerType == gfmType_object, GFMRV_INVALID_TYPE);
    
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
    ASSERT(pCtx->t.innerType == gfmType_object, GFMRV_INVALID_TYPE);
    
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
    ASSERT(pCtx->t.innerType == gfmType_object, GFMRV_INVALID_TYPE);
    
    // Get the object's acceleration
    *pAy = pCtx->ay;
    
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
    *ppChild = pCtx->t.pContext;
    *pType = pCtx->t.type;
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Force this object to stand immovable on collision
 * 
 * NOTE: An object can move through its physics even if fixed!
 * 
 * @param  pCtx The object
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmObject_setFixed(gfmObject *pCtx) {
    gfmRV rv;
    
    // Sanitize arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    ASSERT(pCtx->t.innerType == gfmType_object, GFMRV_INVALID_TYPE);
    
    pCtx->flags |= gfmFlags_isFixed;
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Allow this object to move on collision
 * 
 * @param  pCtx The object
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmObject_setMovable(gfmObject *pCtx) {
    gfmRV rv;
    
    // Sanitize arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    ASSERT(pCtx->t.innerType == gfmType_object, GFMRV_INVALID_TYPE);
    
    pCtx->flags &= ~gfmFlags_isFixed;
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Integrate an object's components.
 * 
 * Currently, it's using verlet integration, i.e.:
 *  x1 = x0 + v0*dt + 0.5*a0*t*t
 *
 * @param  [i/o]x       The object's position
 * @param  [i/o]vx      The object's velocity
 * @param  [in ]ax      The object's acceleration
 * @param  [in ]elapsed Time elapsed from the previous frame
 */
static inline void _gfmObject_integrate(double *x, double *vx, double ax,
        double elapsed) {
    *x += (*vx) * elapsed;
    if (ax != 0.0) {
        *x += 0.5 * ax * elapsed * elapsed;

        *vx += ax * elapsed;
    }
}

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
gfmRV gfmObject_applyDelta(gfmObject *pCtx, gfmObject *pOther) {
    gfmRV rv;

    /* Sanitize everything */
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    ASSERT(pOther, GFMRV_ARGUMENTS_BAD);
    ASSERT(pCtx->t.innerType == gfmType_object, GFMRV_INVALID_TYPE);
    ASSERT(pOther->t.innerType == gfmType_object, GFMRV_INVALID_TYPE);
    ASSERT(pCtx->t.hw > 0, GFMRV_OBJECT_NOT_INITIALIZED);
    ASSERT(pCtx->t.hh > 0, GFMRV_OBJECT_NOT_INITIALIZED);
    ASSERT(pOther->t.hw > 0, GFMRV_OBJECT_NOT_INITIALIZED);
    ASSERT(pOther->t.hh > 0, GFMRV_OBJECT_NOT_INITIALIZED);

    /* Update its position with the other's translation */
    pCtx->dx += pOther->dx - pOther->ldx;
    pCtx->dy += pOther->dy - pOther->ldy;
    pCtx->t.x = (int)pCtx->dx;
    pCtx->t.y = (int)pCtx->dy;

    rv = GFMRV_OK;
__ret:
    return rv;
}

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
gfmRV gfmObject_applyDeltaX(gfmObject *pCtx, gfmObject *pOther) {
    gfmRV rv;

    /* Sanitize everything */
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    ASSERT(pOther, GFMRV_ARGUMENTS_BAD);
    ASSERT(pCtx->t.innerType == gfmType_object, GFMRV_INVALID_TYPE);
    ASSERT(pOther->t.innerType == gfmType_object, GFMRV_INVALID_TYPE);
    ASSERT(pCtx->t.hw > 0, GFMRV_OBJECT_NOT_INITIALIZED);
    ASSERT(pCtx->t.hh > 0, GFMRV_OBJECT_NOT_INITIALIZED);
    ASSERT(pOther->t.hw > 0, GFMRV_OBJECT_NOT_INITIALIZED);
    ASSERT(pOther->t.hh > 0, GFMRV_OBJECT_NOT_INITIALIZED);

    /* Update its position with the other's translation */
    pCtx->dx += pOther->dx - pOther->ldx;
    pCtx->t.x = (int)pCtx->dx;

    rv = GFMRV_OK;
__ret:
    return rv;
}

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
gfmRV gfmObject_applyDeltaY(gfmObject *pCtx, gfmObject *pOther) {
    gfmRV rv;

    /* Sanitize everything */
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    ASSERT(pOther, GFMRV_ARGUMENTS_BAD);
    ASSERT(pCtx->t.innerType == gfmType_object, GFMRV_INVALID_TYPE);
    ASSERT(pOther->t.innerType == gfmType_object, GFMRV_INVALID_TYPE);
    ASSERT(pCtx->t.hw > 0, GFMRV_OBJECT_NOT_INITIALIZED);
    ASSERT(pCtx->t.hh > 0, GFMRV_OBJECT_NOT_INITIALIZED);
    ASSERT(pOther->t.hw > 0, GFMRV_OBJECT_NOT_INITIALIZED);
    ASSERT(pOther->t.hh > 0, GFMRV_OBJECT_NOT_INITIALIZED);

    /* Update its position with the other's translation */
    pCtx->dy += pOther->dy - pOther->ldy;
    pCtx->t.y = (int)pCtx->dy;

    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Update the object; Its last collision status is cleared and the object's
 * properties are integrated using the Euler method
 * 
 * @param  pObj The object
 * @param  pCtx The game's context
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_OBJECT_NOT_INITIALIZED
 */
gfmRV gfmObject_update(gfmObject *pObj, gfmCtx *pCtx) {
    gfmRV rv;
    double elapsed;
    
    // Sanitize arguments
    ASSERT(pObj, GFMRV_ARGUMENTS_BAD);
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    ASSERT(pObj->t.innerType == gfmType_object, GFMRV_INVALID_TYPE);
    // Check that the object was initialized
    ASSERT(pObj->t.hw > 0, GFMRV_OBJECT_NOT_INITIALIZED);
    ASSERT(pObj->t.hh > 0, GFMRV_OBJECT_NOT_INITIALIZED);
    
    // Get the delta time
    rv = gfm_getElapsedTimed(&elapsed, pCtx);
    ASSERT_NR(rv == GFMRV_OK);
    
    // Store the previous position
    pObj->ldx = pObj->dx;
    pObj->ldy = pObj->dy;
    
    // Integrate the position and velocity
    _gfmObject_integrate(&pObj->dx, &pObj->vx, pObj->ax, elapsed);
    _gfmObject_integrate(&pObj->dy, &pObj->vy, pObj->ay, elapsed);
    
    // Set the actual (integer) position
    pObj->t.x = (int)pObj->dx;
    pObj->t.y = (int)pObj->dy;
    
    // Clear this frame's collisions and set the previous one
    pObj->flags &= ~gfmCollision_last;
    pObj->flags |= (pObj->flags & gfmCollision_cur) << gfmFlags_lastBit;
    pObj->flags &= ~gfmCollision_cur;
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

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
        gfmObject *pOther) {
    gfmRV rv;
    
    // Sanitize arguments
    ASSERT(pDx, GFMRV_ARGUMENTS_BAD);
    ASSERT(pDy, GFMRV_ARGUMENTS_BAD);
    ASSERT(pSelf, GFMRV_ARGUMENTS_BAD);
    ASSERT(pOther, GFMRV_ARGUMENTS_BAD);
    
    // Get the distance
    rv = gfmObject_getHorizontalDistance(pDx, pSelf, pOther);
    ASSERT_NR(rv == GFMRV_OK);
    rv = gfmObject_getVerticalDistance(pDy, pSelf, pOther);
    ASSERT_NR(rv == GFMRV_OK);
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Get the horizontal distance between two objects' centers
 *
 * @param  pSelf  An object
 * @param  pOther An object
 */
static int32_t _getHorizontalDistance(gfmObject *pSelf, gfmObject *pOther) {
    int32_t dist;

    dist = pSelf->t.x + pSelf->t.hw;
    dist -= pOther->t.x + pOther->t.hw;

    if (dist < 0) {
        return -dist;
    }
    return dist;
}

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
        gfmObject *pOther) {
    gfmRV rv;
    
    // Sanitize arguments
    ASSERT(pDx, GFMRV_ARGUMENTS_BAD);
    ASSERT(pSelf, GFMRV_ARGUMENTS_BAD);
    ASSERT(pOther, GFMRV_ARGUMENTS_BAD);
    
    // Get the distance
    *pDx = (int)_getHorizontalDistance(pSelf, pOther);
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Get the vertical distance between two objects' centers
 *
 * @param  pSelf  An object
 * @param  pOther An object
 */
static int32_t _getVerticalDistance(gfmObject *pSelf, gfmObject *pOther) {
    int32_t dist;

    dist = pSelf->t.y + pSelf->t.hh;
    dist -= pOther->t.y + pOther->t.hh;

    if (dist < 0) {
        return -dist;
    }
    return dist;
}

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
        gfmObject *pOther) {
    gfmRV rv;
    
    // Sanitize arguments
    ASSERT(pDy, GFMRV_ARGUMENTS_BAD);
    ASSERT(pSelf, GFMRV_ARGUMENTS_BAD);
    ASSERT(pOther, GFMRV_ARGUMENTS_BAD);
    
    // Get the distance
    *pDy = (pSelf->t.y + pSelf->t.hh) - (pOther->t.y + pOther->t.hh);
    if (*pDy < 0)
        *pDy = -(*pDy);
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

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
        gfmObject *pOther) {
    gfmRV rv;
    
    // Sanitize arguments
    ASSERT(pDx, GFMRV_ARGUMENTS_BAD);
    ASSERT(pSelf, GFMRV_ARGUMENTS_BAD);
    ASSERT(pOther, GFMRV_ARGUMENTS_BAD);
    ASSERT(pSelf->t.innerType == gfmType_object, GFMRV_INVALID_TYPE);
    ASSERT(pOther->t.innerType == gfmType_object, GFMRV_INVALID_TYPE);
    
    // Get the distance
    *pDx = (pSelf->dx + pSelf->t.hw) - (pOther->dx + pOther->t.hw);
    if (*pDx < 0)
        *pDx = -(*pDx);
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

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
        gfmObject *pOther) {
    gfmRV rv;
    
    // Sanitize arguments
    ASSERT(pDy, GFMRV_ARGUMENTS_BAD);
    ASSERT(pSelf, GFMRV_ARGUMENTS_BAD);
    ASSERT(pOther, GFMRV_ARGUMENTS_BAD);
    ASSERT(pSelf->t.innerType == gfmType_object, GFMRV_INVALID_TYPE);
    ASSERT(pOther->t.innerType == gfmType_object, GFMRV_INVALID_TYPE);
    
    // Get the distance
    *pDy = (pSelf->dy + pSelf->t.hh) - (pOther->dy + pOther->t.hh);
    if (*pDy < 0)
        *pDy = -(*pDy);
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Check if a give point is inside the object
 * 
 * @param  pCtx The object
 * @param  x    The point's horizontal position
 * @param  y    The point's vertical position
 * @return      GFMRV_TRUE, GFMRV_FALSE, GFMRV_ARGUMENTS_BAD,
 *              GFMRV_OBJECT_NOT_INITIALIZED
 */
gfmRV gfmObject_isPointInside(gfmObject *pCtx, int x, int y) {
    gfmRV rv;
    int cx, cy, delta;
    
    // Sanitize arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    // Check that the object was initialized
    ASSERT(pCtx->t.hw > 0, GFMRV_OBJECT_NOT_INITIALIZED);
    ASSERT(pCtx->t.hh > 0, GFMRV_OBJECT_NOT_INITIALIZED);
    
    // Get the object's central position
    rv = gfmObject_getCenter(&cx, &cy, pCtx);
    ASSERT_NR(rv == GFMRV_OK);
    
    // Check that it's horizontally inside the object
    delta = cx - x;
    if (delta > pCtx->t.hw  || delta < -pCtx->t.hw) {
        rv = GFMRV_FALSE;
        goto __ret;
    }
    // Check that it's vertically inside the object
    delta = cy - y;
    if (delta > pCtx->t.hh  || delta < -pCtx->t.hh) {
        rv = GFMRV_FALSE;
        goto __ret;
    }
    
    rv = GFMRV_TRUE;
__ret:
    return rv;
}

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
gfmRV gfmObject_isOverlaping(gfmObject *pSelf, gfmObject *pOther) {
    gfmRV rv;
    int delta, maxWidth, maxHeight, ox, oy, sx, sy;
    
    // Sanitize arguments
    ASSERT(pSelf, GFMRV_ARGUMENTS_BAD);
    ASSERT(pOther, GFMRV_ARGUMENTS_BAD);
    // Check that the object was initialized
    ASSERT(pSelf->t.hw > 0, GFMRV_OBJECT_NOT_INITIALIZED);
    ASSERT(pSelf->t.hh > 0, GFMRV_OBJECT_NOT_INITIALIZED);
    ASSERT(pOther->t.hw > 0, GFMRV_OBJECT_NOT_INITIALIZED);
    ASSERT(pOther->t.hh > 0, GFMRV_OBJECT_NOT_INITIALIZED);
    
    // Get 'this' object's center
    rv = gfmObject_getCenter(&sx, &sy, pSelf);
    ASSERT_NR(rv == GFMRV_OK);
    // Get 'the other' object's center
    rv = gfmObject_getCenter(&ox, &oy, pOther);
    ASSERT_NR(rv == GFMRV_OK);
    
    // Get both max distances for an overlap to happen
    maxWidth = pSelf->t.hw + pOther->t.hw;
    maxHeight = pSelf->t.hh + pOther->t.hh;
    
    // Check if a horizontal overlap happened
    delta = sx - ox;
    if (delta > maxWidth || delta < -maxWidth) {
        return GFMRV_FALSE;
    }
    //ASSERT(delta <= maxWidth && delta >= -maxWidth, GFMRV_FALSE);
    
    // Check if a vertical overlap happened
    delta = sy - oy;
    if (delta > maxHeight || delta < -maxHeight) {
        return GFMRV_FALSE;
    }
    //ASSERT(delta <= maxHeight && delta >= -maxHeight, GFMRV_FALSE);
    
    rv = GFMRV_TRUE;
__ret:
    return rv;
}

/**
 * Check if an object just overlaped a hitbox
 *
 * @param  [ in]pObj    The object
 * @param  [ in]pHitbox The hitbox
 */
gfmRV gfmObject_justOverlapedHitbox(gfmObject *pObj, gfmHitbox *pHitbox) {
    gfmRV rv;
    int32_t dist, lastDist, maxDist;

    /* Sanitize arguments */
    ASSERT(pObj, GFMRV_ARGUMENTS_BAD);
    ASSERT(pHitbox, GFMRV_ARGUMENTS_BAD);
    ASSERT(pObj->t.innerType == gfmType_object, GFMRV_INVALID_TYPE);
    ASSERT(pHitbox->innerType == gfmType_hitbox, GFMRV_INVALID_TYPE);
    /* Check that the object was initialized */
    ASSERT(pObj->t.hw > 0, GFMRV_OBJECT_NOT_INITIALIZED);
    ASSERT(pObj->t.hh > 0, GFMRV_OBJECT_NOT_INITIALIZED);
    ASSERT(pHitbox->hw > 0, GFMRV_OBJECT_NOT_INITIALIZED);
    ASSERT(pHitbox->hh > 0, GFMRV_OBJECT_NOT_INITIALIZED);

    /* Clear the instantaneous flag */
    pObj->flags &= ~gfmCollision_inst;

    /* Check horizontal collision */
    dist = _getHorizontalDistance(pObj, (gfmObject*)pHitbox);
    maxDist = pObj->t.hw + pHitbox->hw;
    if (dist > maxDist) {
        return GFMRV_FALSE;
    }
    lastDist = (int32_t)pObj->ldx + pObj->t.hw;
    lastDist -= pHitbox->x + pHitbox->hw;
    /* Overlap was only triggered this frame if they weren't overlaping on the
     * previous one */
    if (lastDist >= maxDist || lastDist <= -maxDist) {
        if (dist + pObj->t.hw <= pHitbox->hw
                || dist + pHitbox->hw <= pObj->t.hw) {
            /* One of the entities was placed inside the other. Simply ignore */
        }
        else if (pObj->t.x < pHitbox->x) {
            pObj->flags |= gfmCollision_instRight;
        }
        else {
            pObj->flags |= gfmCollision_instLeft;
        }
    }

    /* Check vertical collision */
    dist = _getVerticalDistance(pObj, (gfmObject*)pHitbox);
    maxDist = pObj->t.hh + pHitbox->hh;
    if (dist > maxDist) {
        return GFMRV_FALSE;
    }
    lastDist = (int32_t)pObj->ldy + pObj->t.hh;
    lastDist -= pHitbox->y + pHitbox->hh;
    /* Overlap was only triggered this frame if they weren't overlaping on the
     * previous one */
    if (lastDist >= maxDist || lastDist <= -maxDist) {
#if 0
        if (dist + pObj->t.hh <= pHitbox->hh
                || dist + pHitbox->hh <= pObj->t.hh) {
            /* One of the entities was placed inside the other. Simply ignore */
        }
        else
#endif
        if (pObj->t.y < pHitbox->y) {
            pObj->flags |= gfmCollision_instDown;
        }
        else {
            pObj->flags |= gfmCollision_instUp;
        }
    }

    /* Check that they collided in any direction */
    if ((pObj->flags & (gfmCollision_instHor | gfmCollision_instVer)) == 0 ) {
        return GFMRV_FALSE;
    }

    /* Set overlap position definitivelly */
    pObj->flags |= (pObj->flags & gfmCollision_inst) >> gfmFlags_instBit;
    rv = GFMRV_TRUE;
__ret:
    return rv;
}

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
gfmRV gfmObject_justOverlaped(gfmObject *pSelf, gfmObject *pOther) {
    gfmRV rv;
    int delta, lox, loy, lsx, lsy, maxWidth, maxHeight, ox, oy, sx, sy;
    
    // Sanitize arguments
    ASSERT(pSelf, GFMRV_ARGUMENTS_BAD);
    ASSERT(pOther, GFMRV_ARGUMENTS_BAD);
    // Check that the object was initialized
    ASSERT(pSelf->t.hw > 0, GFMRV_OBJECT_NOT_INITIALIZED);
    ASSERT(pSelf->t.hh > 0, GFMRV_OBJECT_NOT_INITIALIZED);
    ASSERT(pOther->t.hw > 0, GFMRV_OBJECT_NOT_INITIALIZED);
    ASSERT(pOther->t.hh > 0, GFMRV_OBJECT_NOT_INITIALIZED);

    /* If necessary, override object collision with object X hitbox */
    if (pSelf->t.innerType == gfmType_object
            && pOther->t.innerType == gfmType_hitbox) {
        return gfmObject_justOverlapedHitbox(pSelf, (gfmHitbox*)pOther);
    }
    else if (pOther->t.innerType == gfmType_object
            && pSelf->t.innerType == gfmType_hitbox) {
        return gfmObject_justOverlapedHitbox(pOther, (gfmHitbox*)pSelf);
    }
    else if (pSelf->t.innerType == gfmType_hitbox) {
        /* Both objects are hitboxes */
        return GFMRV_INVALID_TYPE;
    }
    
    // Clear the instantaneous flag
    pSelf->flags &= ~gfmCollision_inst;
    pOther->flags &= ~gfmCollision_inst;
    
    // Get 'this' object's center
    rv = gfmObject_getCenter(&sx, &sy, pSelf);
    ASSERT_NR(rv == GFMRV_OK);
    // Get 'the other' object's center
    rv = gfmObject_getCenter(&ox, &oy, pOther);
    ASSERT_NR(rv == GFMRV_OK);
    // Get 'this' object's center on last frame
    rv = gfmObject_getLastCenter(&lsx, &lsy, pSelf);
    ASSERT_NR(rv == GFMRV_OK);
    // Get 'the other' object's center on last frame
    rv = gfmObject_getLastCenter(&lox, &loy, pOther);
    ASSERT_NR(rv == GFMRV_OK);
    
    // Get both max distances for an overlap to happen
    maxWidth = pSelf->t.hw + pOther->t.hw;
    maxHeight = pSelf->t.hh + pOther->t.hh;
    
    // Check if a horizontal overlap may have happened
    delta = sx - ox;
    ASSERT(delta <= maxWidth && delta >= -maxWidth, GFMRV_FALSE);
    // Check that they weren't overlaping previous frame; or they just moved
    delta = lsx - lox;
    if (delta >= maxWidth || delta <= -maxWidth) {
        // Check their relative position and set the collision results
        delta = abs(sx - ox);
        if (delta + pSelf->t.hw <= pOther->t.hw
                || delta + pOther->t.hw <= pSelf->t.hw) {
            /* One of the entities was placed inside the other. Simply ignore
             * collision */
        }
        else if (pSelf->t.x < pOther->t.x) {
            // pSelf is to the left, so it collided on its right
            pSelf->flags |= gfmCollision_instRight;
            pOther->flags |= gfmCollision_instLeft;
        }
        else {
            // pSelf is to the right, so it collided on its left
            pSelf->flags |= gfmCollision_instLeft;
            pOther->flags |= gfmCollision_instRight;
        }
    }
    
    // Check if a vertical overlap may have happened
    delta = sy - oy;
    ASSERT(delta <= maxHeight && delta >= -maxHeight, GFMRV_FALSE);
    // Check that they weren't overlaping previous frame; or they just moved
    delta = lsy - loy;
    if (delta >= maxHeight || delta <= -maxHeight) {
        // Check their relative position and set the collision results
        if (pSelf->t.y < pOther->t.y) {
            // pSelf is above, so it collided bellow
            pSelf->flags |= gfmCollision_instDown;
            pOther->flags |= gfmCollision_instUp;
        }
        else {
            // pSelf is bellow, so it collided above
            pSelf->flags |= gfmCollision_instUp;
            pOther->flags |= gfmCollision_instDown;
        }
    }
    
    // Check that they collided in any directions
    if ((pSelf->flags & (gfmCollision_instHor | gfmCollision_instVer)) == 0 ) {
        rv = GFMRV_FALSE;
        goto __ret;
    }
    
    // Set overlap position definitivelly 
    pSelf->flags |= (pSelf->flags & gfmCollision_inst) >> gfmFlags_instBit;
    pOther->flags |= (pOther->flags & gfmCollision_inst) >> gfmFlags_instBit;
    
    rv = GFMRV_TRUE;
__ret:
    return rv;
}

/**
 * Separate an object and a hitbox in the Y axis
 * 
 * @param  [ in]pObj    The object
 * @param  [ in]pHitbox The hitbox
 */
gfmRV gfmObject_separateHorizontalHitbox(gfmObject *pObj, gfmHitbox *pHitbox) {
    gfmRV rv;

    /* Sanitize arguments */
    ASSERT(pObj, GFMRV_ARGUMENTS_BAD);
    ASSERT(pHitbox, GFMRV_ARGUMENTS_BAD);
    ASSERT(pObj->t.innerType == gfmType_object, GFMRV_INVALID_TYPE);
    ASSERT(pHitbox->innerType == gfmType_hitbox, GFMRV_INVALID_TYPE);
    /* Check that the object was initialized */
    ASSERT(pObj->t.hw > 0, GFMRV_OBJECT_NOT_INITIALIZED);
    ASSERT(pObj->t.hh > 0, GFMRV_OBJECT_NOT_INITIALIZED);
    ASSERT(pHitbox->hw > 0, GFMRV_OBJECT_NOT_INITIALIZED);
    ASSERT(pHitbox->hh > 0, GFMRV_OBJECT_NOT_INITIALIZED);
    /* Check that the object isn't fixed */
    ASSERT(!(pObj->flags & gfmFlags_isFixed), GFMRV_OBJECTS_CANT_COLLIDE);
    /* Check that collision happened in the X axis */
    ASSERT(pObj->flags & gfmCollision_instHor, GFMRV_COLLISION_NOT_TRIGGERED);

    if (pObj->flags & gfmCollision_instLeft) {
        /* pMovable collided to the left, place it at static's right */
        pObj->dx = pHitbox->x + 2 * pHitbox->hw + 1;
    }
    else if (pObj->flags & gfmCollision_instRight) {
        /* pMovable collided to the right, place it at static's left */
        pObj->dx = pHitbox->x - 2 * pObj->t.hw - 1;
    }
    pObj->t.x = (int)pObj->dx;

    rv = GFMRV_TRUE;
__ret:
    return rv;
}

/**
 * Separate an object and a hitbox in the Y axis
 * 
 * @param  [ in]pObj    The object
 * @param  [ in]pHitbox The hitbox
 */
gfmRV gfmObject_separateVerticalHitbox(gfmObject *pObj, gfmHitbox *pHitbox) {
    gfmRV rv;

    /* Sanitize arguments */
    ASSERT(pObj, GFMRV_ARGUMENTS_BAD);
    ASSERT(pHitbox, GFMRV_ARGUMENTS_BAD);
    ASSERT(pObj->t.innerType == gfmType_object, GFMRV_INVALID_TYPE);
    ASSERT(pHitbox->innerType == gfmType_hitbox, GFMRV_INVALID_TYPE);
    /* Check that the object was initialized */
    ASSERT(pObj->t.hw > 0, GFMRV_OBJECT_NOT_INITIALIZED);
    ASSERT(pObj->t.hh > 0, GFMRV_OBJECT_NOT_INITIALIZED);
    ASSERT(pHitbox->hw > 0, GFMRV_OBJECT_NOT_INITIALIZED);
    ASSERT(pHitbox->hh > 0, GFMRV_OBJECT_NOT_INITIALIZED);
    /* Check that the object isn't fixed */
    ASSERT(!(pObj->flags & gfmFlags_isFixed), GFMRV_OBJECTS_CANT_COLLIDE);
    /* Check that collision happened in the Y axis */
    ASSERT(pObj->flags & gfmCollision_instVer, GFMRV_COLLISION_NOT_TRIGGERED);

    if (pObj->flags & gfmCollision_instUp) {
        /* pMovable collided above, place it bellow static */
        pObj->dy = pHitbox->y + 2 * pHitbox->hh;
    }
    else if (pObj->flags & gfmCollision_instDown) {
        /* pMovable collided bellow, place it above static */
        pObj->dy = pHitbox->y - 2 * pObj->t.hh;
    }
    pObj->t.y = (int)pObj->dy;

    rv = GFMRV_TRUE;
__ret:
    return rv;
}

/**
 * Collide two objects
 * 
 * NOTE: It fails to detect if an object was inside another one and is leaving
 * 
 * @param  [ in]pObj    The object
 * @param  [ in]pHitbox The hitbox
 */
gfmRV gfmObject_collideHitbox(gfmObject *pObj, gfmHitbox *pHitbox) {
    gfmRV rv;

    /* Sanitize arguments */
    ASSERT(pObj, GFMRV_ARGUMENTS_BAD);
    ASSERT(pHitbox, GFMRV_ARGUMENTS_BAD);
    ASSERT(pObj->t.innerType == gfmType_object, GFMRV_INVALID_TYPE);
    ASSERT(pHitbox->innerType == gfmType_hitbox, GFMRV_INVALID_TYPE);
    /* Check that the object was initialized */
    ASSERT(pObj->t.hw > 0, GFMRV_OBJECT_NOT_INITIALIZED);
    ASSERT(pObj->t.hh > 0, GFMRV_OBJECT_NOT_INITIALIZED);
    ASSERT(pHitbox->hw > 0, GFMRV_OBJECT_NOT_INITIALIZED);
    ASSERT(pHitbox->hh > 0, GFMRV_OBJECT_NOT_INITIALIZED);
    /* Check that the object isn't fixed */
    ASSERT(!(pObj->flags & gfmFlags_isFixed), GFMRV_OBJECTS_CANT_COLLIDE);

    if (gfmObject_justOverlapedHitbox(pObj, pHitbox) != GFMRV_TRUE) {
        return GFMRV_FALSE;
    }
    if (pObj->flags & gfmCollision_instHor) {
        gfmObject_separateHorizontalHitbox(pObj, pHitbox);
    }
    if (pObj->flags & gfmCollision_instVer) {
        gfmObject_separateVerticalHitbox(pObj, pHitbox);
    }

    rv = GFMRV_TRUE;
__ret:
    return rv;
}

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
gfmRV gfmObject_collide(gfmObject *pSelf, gfmObject *pOther) {
    gfmRV rv;
    
    // Sanitize arguments
    ASSERT(pSelf, GFMRV_ARGUMENTS_BAD);
    ASSERT(pOther, GFMRV_ARGUMENTS_BAD);
    // Check that the object was initialized
    ASSERT(pSelf->t.hw > 0, GFMRV_OBJECT_NOT_INITIALIZED);
    ASSERT(pSelf->t.hh > 0, GFMRV_OBJECT_NOT_INITIALIZED);
    ASSERT(pOther->t.hw > 0, GFMRV_OBJECT_NOT_INITIALIZED);
    ASSERT(pOther->t.hh > 0, GFMRV_OBJECT_NOT_INITIALIZED);

    /* If necessary, override object collision with object X hitbox */
    if (pSelf->t.innerType == gfmType_object
            && pOther->t.innerType == gfmType_hitbox) {
        return gfmObject_collideHitbox(pSelf, (gfmHitbox*)pOther);
    }
    else if (pOther->t.innerType == gfmType_object
            && pSelf->t.innerType == gfmType_hitbox) {
        return gfmObject_collideHitbox(pOther, (gfmHitbox*)pSelf);
    }
    else if (pSelf->t.innerType == gfmType_hitbox) {
        /* Both objects are hitboxes */
        return GFMRV_INVALID_TYPE;
    }

    // Check that at least one isn't fixed
    ASSERT(!(pSelf->flags & gfmFlags_isFixed)
            || !(pOther->flags & gfmFlags_isFixed), GFMRV_OBJECTS_CANT_COLLIDE);
    
    // Overlap both objects
    rv = gfmObject_justOverlaped(pSelf, pOther);
    ASSERT(rv == GFMRV_TRUE || rv == GFMRV_FALSE, rv);
    if (rv == GFMRV_FALSE) {
        goto __ret;
    }
    
    // If they overlapped horizontally, separate'em
    if (pSelf->flags & gfmCollision_instHor) {
        rv = gfmObject_separateHorizontal(pSelf, pOther);
        ASSERT(rv == GFMRV_OK, rv);
    }
    // If they overlapped vertically, separate'em
    if (pSelf->flags & gfmCollision_instVer) {
        rv = gfmObject_separateVertical(pSelf, pOther);
        ASSERT(rv == GFMRV_OK, rv);
    }
    
    rv = GFMRV_TRUE;
__ret:
    return rv;
}

/**
 * Separate two object in the X axis
 * 
 * @param  pSelf  An object
 * @param  pOther An object
 * @return        GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_OBJECT_NOT_INITIALIZED,
 *                GFMRV_OBJECTS_CANT_COLLIDE, GFMRV_COLLISION_NOT_TRIGGERED
 */
gfmRV gfmObject_separateHorizontal(gfmObject *pSelf, gfmObject *pOther) {
    gfmObject *pStatic, *pMovable;
    gfmRV rv;
    
    // Sanitize arguments
    ASSERT(pSelf, GFMRV_ARGUMENTS_BAD);
    ASSERT(pOther, GFMRV_ARGUMENTS_BAD);
    ASSERT(pSelf->t.innerType == gfmType_object, GFMRV_INVALID_TYPE);
    ASSERT(pOther->t.innerType == gfmType_object, GFMRV_INVALID_TYPE);
    // Check that the object was initialized
    ASSERT(pSelf->t.hw > 0, GFMRV_OBJECT_NOT_INITIALIZED);
    ASSERT(pSelf->t.hh > 0, GFMRV_OBJECT_NOT_INITIALIZED);
    ASSERT(pOther->t.hw > 0, GFMRV_OBJECT_NOT_INITIALIZED);
    ASSERT(pOther->t.hh > 0, GFMRV_OBJECT_NOT_INITIALIZED);

    /* If necessary, override object collision with object X hitbox */
    if (pSelf->t.innerType == gfmType_object
            && pOther->t.innerType == gfmType_hitbox) {
        return gfmObject_separateHorizontalHitbox(pSelf, (gfmHitbox*)pOther);
    }
    else if (pOther->t.innerType == gfmType_object
            && pSelf->t.innerType == gfmType_hitbox) {
        return gfmObject_separateHorizontalHitbox(pOther, (gfmHitbox*)pSelf);
    }
    else if (pSelf->t.innerType == gfmType_hitbox) {
        /* Both objects are hitboxes */
        return GFMRV_INVALID_TYPE;
    }

    // Check that at least one isn't fixed
    ASSERT(!(pSelf->flags & gfmFlags_isFixed)
            || !(pOther->flags & gfmFlags_isFixed), GFMRV_OBJECTS_CANT_COLLIDE);
    // Check that collision happened in the X axis
    ASSERT(pSelf->flags & gfmCollision_instHor, GFMRV_COLLISION_NOT_TRIGGERED);
    
    // If an object is static, get the other one
    if (pOther->flags & gfmFlags_isFixed) {
        pStatic = pOther;
        pMovable = pSelf;
    }
    else if (pSelf->flags & gfmFlags_isFixed) {
        pStatic = pSelf;
        pMovable = pOther;
    }
    else
        pStatic = 0;
    
    if (pStatic && pMovable) {
        double newX;
        // If one object is static
        
        if (pMovable->flags & gfmCollision_instLeft) {
            // pMovable collided to the left, place it at static's right
            newX = pStatic->dx + 2 * pStatic->t.hw + 1;
        }
        else if (pMovable->flags & gfmCollision_instRight) {
            // pMovable collided to the right, place it at static's left
            newX = pStatic->dx - 2 * pMovable->t.hw - 1;
        }
        else {
            // Never gonna happen, but avoids warning (stupid compiler!)
            ASSERT(0, GFMRV_FUNCTION_FAILED);
        }
        rv = _int_gfmObject_setHorizontalPosition(pMovable, newX);
        ASSERT_NR(rv == GFMRV_OK);
    }
    else {
        double dist;
        
        // Get the object's distance
        rv = gfmObject_getHorizontalDistanced(&dist, pSelf, pOther);
        ASSERT_NR(rv == GFMRV_OK);
        dist *= 0.5;
        // Push both objects
        if (pSelf->flags & gfmCollision_instLeft) {
            // pSelf collided left, so it must be pushed to the right
            rv = _int_gfmObject_setHorizontalPosition(pSelf, pSelf->dx + dist);
            ASSERT_NR(rv == GFMRV_OK);
            rv = _int_gfmObject_setHorizontalPosition(pOther, pOther->dx - dist);
            ASSERT_NR(rv == GFMRV_OK);
        }
        if (pSelf->flags & gfmCollision_instRight) {
            // pSelf collided right, so it must be pushed to the left
            rv = _int_gfmObject_setHorizontalPosition(pSelf, pSelf->dx - dist);
            ASSERT_NR(rv == GFMRV_OK);
            rv = _int_gfmObject_setHorizontalPosition(pOther, pOther->dx + dist);
            ASSERT_NR(rv == GFMRV_OK);
        }
    }
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Separate two object in the Y axis
 * 
 * @param  pSelf  An object
 * @param  pOther An object
 * @return        GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_OBJECT_NOT_INITIALIZED,
 *                GFMRV_OBJECTS_CANT_COLLIDE, GFMRV_COLLISION_NOT_TRIGGERED
 */
gfmRV gfmObject_separateVertical(gfmObject *pSelf, gfmObject *pOther) {
    gfmObject *pStatic, *pMovable;
    gfmRV rv;
    
    // Sanitize arguments
    ASSERT(pSelf, GFMRV_ARGUMENTS_BAD);
    ASSERT(pOther, GFMRV_ARGUMENTS_BAD);
    ASSERT(pSelf->t.innerType == gfmType_object, GFMRV_INVALID_TYPE);
    ASSERT(pOther->t.innerType == gfmType_object, GFMRV_INVALID_TYPE);
    // Check that the object was initialized
    ASSERT(pSelf->t.hw > 0, GFMRV_OBJECT_NOT_INITIALIZED);
    ASSERT(pSelf->t.hh > 0, GFMRV_OBJECT_NOT_INITIALIZED);
    ASSERT(pOther->t.hw > 0, GFMRV_OBJECT_NOT_INITIALIZED);
    ASSERT(pOther->t.hh > 0, GFMRV_OBJECT_NOT_INITIALIZED);

    /* If necessary, override object collision with object X hitbox */
    if (pSelf->t.innerType == gfmType_object
            && pOther->t.innerType == gfmType_hitbox) {
        return gfmObject_separateVerticalHitbox(pSelf, (gfmHitbox*)pOther);
    }
    else if (pOther->t.innerType == gfmType_object
            && pSelf->t.innerType == gfmType_hitbox) {
        return gfmObject_separateVerticalHitbox(pOther, (gfmHitbox*)pSelf);
    }
    else if (pSelf->t.innerType == gfmType_hitbox) {
        /* Both objects are hitboxes */
        return GFMRV_INVALID_TYPE;
    }

    // Check that at least one isn't fixed
    ASSERT(!(pSelf->flags & gfmFlags_isFixed)
            || !(pOther->flags & gfmFlags_isFixed), GFMRV_OBJECTS_CANT_COLLIDE);
    // Check that collision happened in the X axis
    ASSERT(pSelf->flags & gfmCollision_instVer, GFMRV_COLLISION_NOT_TRIGGERED);
    
    // If an object is static, get the other one
    if (pOther->flags & gfmFlags_isFixed) {
        pStatic = pOther;
        pMovable = pSelf;
    }
    else if (pSelf->flags & gfmFlags_isFixed) {
        pStatic = pSelf;
        pMovable = pOther;
    }
    else
        pStatic = 0;
    
    if (pStatic && pMovable) {
        double newY;
        // If one object is static
        
        // If the object was just placed inside another object, push it out
        if (pMovable->flags & gfmCollision_instUp) {
            // pMovable collided above, place it bellow static
            newY = pStatic->dy + 2 * pStatic->t.hh;
        }
        else if (pMovable->flags & gfmCollision_instDown) {
            // pMovable collided bellow, place it above static
            newY = pStatic->dy - 2 * pMovable->t.hh;
        }
        else {
            // Never gonna happen, but avoids warning (stupid compiler!)
            ASSERT(0, GFMRV_FUNCTION_FAILED);
        }
        rv = _int_gfmObject_setVerticalPosition(pMovable, newY);
        ASSERT_NR(rv == GFMRV_OK);
    }
    else {
        double dist;
        
        // Get the object's distance
        rv = gfmObject_getVerticalDistanced(&dist, pSelf, pOther);
        ASSERT_NR(rv == GFMRV_OK);
        dist *= 0.5;
        // Push both objects
        if (pSelf->flags & gfmCollision_instUp) {
            // pSelf collided above so it must be pushed downward
            rv = _int_gfmObject_setVerticalPosition(pSelf, pSelf->dy + dist);
            ASSERT_NR(rv == GFMRV_OK);
            rv = _int_gfmObject_setVerticalPosition(pOther, pOther->dy - dist);
            ASSERT_NR(rv == GFMRV_OK);
        }
        if (pSelf->flags & gfmCollision_instDown) {
            // pSelf collided bellow, so it must be pushed upward
            rv = _int_gfmObject_setVerticalPosition(pSelf, pSelf->dy - dist);
            ASSERT_NR(rv == GFMRV_OK);
            rv = _int_gfmObject_setVerticalPosition(pOther, pOther->dy + dist);
            ASSERT_NR(rv == GFMRV_OK);
        }
    }
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Get the current collision/overlap flags
 * 
 * @param  pDir The collision direction
 * @param  pCtx The object
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmObject_getCollision(gfmCollision *pDir, gfmObject *pCtx) {
    gfmRV rv;
    
    // Sanitize arguments
    ASSERT(pDir, GFMRV_ARGUMENTS_BAD);
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    ASSERT(pCtx->t.innerType == gfmType_object, GFMRV_INVALID_TYPE);
    
    // Get the flags
    *pDir = (pCtx->flags & gfmCollision_cur);
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Get the last frame's collision/overlap flags
 * 
 * @param  pDir The collision direction
 * @param  pCtx The object
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmObject_getLastCollision(gfmCollision *pDir, gfmObject *pCtx) {
    gfmRV rv;
    
    // Sanitize arguments
    ASSERT(pDir, GFMRV_ARGUMENTS_BAD);
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    ASSERT(pCtx->t.innerType == gfmType_object, GFMRV_INVALID_TYPE);
    
    // Get the flags
    *pDir = (pCtx->flags & gfmCollision_last) >> gfmFlags_lastBit;
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Get the collision/overlap flags from the last 'gfmObject_isOverlaping' call
 * 
 * @param  pDir The collision direction
 * @param  pCtx The object
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmObject_getCurrentCollision(gfmCollision *pDir, gfmObject *pCtx) {
    gfmRV rv;
    
    // Sanitize arguments
    ASSERT(pDir, GFMRV_ARGUMENTS_BAD);
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    ASSERT(pCtx->t.innerType == gfmType_object, GFMRV_INVALID_TYPE);
    
    // Get the flags
    *pDir = (pCtx->flags & gfmCollision_inst) >> gfmFlags_instBit;
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

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
gfmRV gfmObject_overlapLine(gfmObject *pCtx, int x0, int y0, int x1, int y1) {
    gfmRect object;
    gfmLine line;
    gfmPoint point0, point1;
    gfmRV rv;
    int ox, oy, minX, minY;

    /* Sanitize arguments */
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);

    /* Normalize */
    rv = gfmObject_getCenter(&ox, &oy, pCtx);
    ASSERT_NR(rv == GFMRV_OK);
    if ((abs(ox - x0) > GFM_FRACTION_MAX_INT)
            || (abs(ox - x1) > GFM_FRACTION_MAX_INT)
            || (abs(x0 - x1) > GFM_FRACTION_MAX_INT)
            || (abs(oy - y0) > GFM_FRACTION_MAX_INT)
            || (abs(oy - y1) > GFM_FRACTION_MAX_INT)
            || (abs(y0 - y1) > GFM_FRACTION_MAX_INT)) {
        return GFMRV_FIXED_POINT_TOO_BIG;
    }

#define min(X, Y) ((X) < (Y) ? (X) : (Y))
    minX = min(ox, x0);
    minX = min(minX, x1);
    minY = min(oy, y0);
    minY = min(minY, y1);
#undef min

    object.centerX = gfmFixedPoint_fromInt(ox - minX);
    object.centerY = gfmFixedPoint_fromInt(oy - minY);
    object.halfWidth = gfmFixedPoint_fromInt(pCtx->t.hw);
    object.halfHeight = gfmFixedPoint_fromInt(pCtx->t.hh);

    point0.x = gfmFixedPoint_fromInt(x0 - minX);
    point0.y = gfmFixedPoint_fromInt(y0 - minY);
    point1.x = gfmFixedPoint_fromInt(x1 - minX);
    point1.y = gfmFixedPoint_fromInt(y1 - minY);

    if (x0 == x1) {
        gfmRect rect;

        /* Vertical lines must be handled in a special case, as a "thin" (i.e.,
         * 0 width) rectangle */
        rect.centerX = point0.x;
        rect.halfWidth = 0;
        rect.halfHeight = gfmFixedPoint_fromInt(abs(y1 - y0) >> 1);
        if (y0 < y1) {
            rect.centerY = point0.y + (point1.y >> 1);
        }
        else {
            rect.centerY = point1.y + (point0.y >> 1);
        }

        if (gfmGeometry_doesRectsIntersect(&rect, &object)) {
            return GFMRV_TRUE;
        }
        return GFMRV_FALSE;
    }

    line.x.lt = point0.x;
    line.x.gt = point1.x;
    line.a = gfmFixedPoint_div(gfmFixedPoint_fromInt(y1 - y0)
            , gfmFixedPoint_fromInt(x1 - x0));
    line.b = point0.y - gfmFixedPoint_mul(point0.x, line.a);

    if (gfmGeometry_doesLineIntersectRect(&line, &object)) {
        return GFMRV_TRUE;
    }
    return GFMRV_FALSE;
__ret:
    return rv;
}

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
gfmRV gfmObject_setType(gfmObject *pCtx, int type) {
    if (pCtx == 0) {
        return GFMRV_ARGUMENTS_BAD;
    }

    pCtx->t.type = type;

    return GFMRV_OK;
}

