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
 * 
 * @param  pCtx   The object
 * @param  x      The object's horizontal position
 * @param  y      The object's vertical position
 * @param  width  The object's width
 * @param  height The object's height
 * @return        GFMRV_OK, GFMRV_ARGUMENTS_BAD, ...
 */
gfmRV gfmObject_init(gfmObject *pCtx, int x, int y, int width, int height) {
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
 * @param  pCtx   The object
 * @param  width  The object's width
 * @return        GFMRV_OK, GFMRV_ARGUMENTS_BAD
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

gfmRV gfmObject_isPointInside(gfmObject *pCtx, int x, int y) {
    return GFMRV_FUNCTION_NOT_IMPLEMENTED;
}

