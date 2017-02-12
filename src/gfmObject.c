/**
 * @file include/GFraMe/gfmObject.h
 * 
 * Basic physical object; It has an AABB, position, velocity, acceleration, drag
 * and current (and previous frame) collision info;
 * Since this is the base type to be passed to the quadtree for
 * overlaping/collision, it also has info about it's "child type" (e.g., a
 * gfmSprite pointer and the type T_GFMSPRITE)
 */
#include <GFraMe/gframe.h>
#include <GFraMe/gfmAssert.h>
#include <GFraMe/gfmError.h>
#include <GFraMe/gfmObject.h>

#include <GFraMe_int/gfmFixedPoint.h>
#include <GFraMe_int/gfmGeometry.h>

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

enum enPhysicBodyFlags {
    gfmInstantCollision_none  = 0x0000
  , gfmInstantCollision_left  = 0x0100
  , gfmInstantCollision_right = 0x0200
  , gfmInstantCollision_up    = 0x0400
  , gfmInstantCollision_down  = 0x0800
  , gfmInstantCollision_hor   = 0x0300
  , gfmInstantCollision_ver   = 0x0C00
  , gfmInstantCollision_mask  = 0x0F00
  , EPBF_FIXED                = 0x8000

  , EPBF_INSTANT_SHIFT        = 8
};

struct stGFMTransform {
    /** Object's real type */
    uint32_t innerType;
    /** Position as a fixed point number: 16 bits for the integer part and 7
     * bits for the decimal part */
    int32_t x;
    int32_t y;
    /** Half dimension as a fixed point number: 14 bits for the integer part and
     * 1 bit for the decimal part. It's signed to ease some calculations */
    int32_t hw;
    int32_t hh;
    /** Object that currently 'own' this transform */
    uint32_t type;
    void *pParent;
};
struct stGFMPhysicBody {
    /** Object's type */
    uint16_t type;
    /** Stores instant, current and last collision state. Also stores whether
     * this body may be pushed by others on collision  */
    uint16_t flags;
    /** Previous position as a fixed point number: 16 bits for the integer part
     * and 7 bits for the decimal part */
    int32_t lx;
    int32_t ly;
    /** Velocity as a fixed point number: 16 bits for the integer part and 7
     * bits for the decimal part */
    int32_t vx;
    int32_t vy;
    /** Acceleration as a fixed point number: 16 bits for the integer part and 7
     * bits for the decimal part */
    int32_t ax;
    int32_t ay;
};
/** The gfmObject structure */
struct stGFMObject {
    struct stGFMTransform t;
    struct stGFMPhysicBody pb;
};

#define FP_INTEGER_BIT  16
#define FP_DECIMAL_BIT  7

/** Return the absolute value of a fixed point number */
inline static int32_t _fp_abs(int32_t a) {
    if (a < 0) {
        return -a;
    }
    return a;
}

/** Multiply two fixed point numbers */
inline static int32_t _fp_mul(int32_t a, int32_t b) {
    return (a * b) >> FP_DECIMAL_BIT;
}

/** Divide two fixed point numbers */
inline static int32_t _fp_div(int32_t a, int32_t b) {
    return (a << FP_DECIMAL_BIT) / b;
}

/** Convert a fixed point number to integer */
inline static int _fp2i(int32_t val) {
    /* From ISO/IEC 9899:TC3
     *  - 6.5.7 $5: If E1 has a signed type and a negative value, the resulting
     *              value is implementation-defined. */
    if (val > 0) {
        return (int)(val >> FP_DECIMAL_BIT);
    }
    else {
        val = -val;
        return -(int)(val >> FP_DECIMAL_BIT);
    }
}

/** Convert an integer to a fixed point number */
inline static int32_t _i2fp(int val) {
    if (val < 0) {
        val = -val;
        /* Ensure that it's within bounds */
        assert(val < (1 << FP_INTEGER_BIT));
        return -(int32_t)(val << FP_DECIMAL_BIT);
    }
    else {
        /* Ensure that it's within bounds */
        assert(val < (1 << FP_INTEGER_BIT));
        return (int32_t)(val << FP_DECIMAL_BIT);
    }
}

/** Convert a fixed point number to double */
inline static double _fp2d(int32_t val) {
    return (double)val / (double)(1 << FP_DECIMAL_BIT);
}

/** Convert a double to a fixed point number */
inline static int32_t _d2fp(double val) {
    int32_t integer, fraction;
    int signal;

    if (val < 0.0f ) {
        val *= -1.0f;
        signal = 1;
    }
    else {
        signal = 0;
    }

    /* Ensure that it's within bounds */
    assert((int)val < (1 << FP_INTEGER_BIT));

    integer = (int32_t)val;
    val -= (double)integer;
    fraction = (int32_t)(val * (double)(1 << FP_DECIMAL_BIT));

    if (signal) {
        return -((integer << FP_DECIMAL_BIT) | fraction);
    }
    return (integer << FP_DECIMAL_BIT) | fraction;
}

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
    /* Sanitize arguments */
    if (pCtx == 0 || width <= 0 || height <= 0) {
        return GFMRV_ARGUMENTS_BAD;
    }
    assert(width < (1 << 14));
    assert(height < (1 << 14));

    /* Clear it up */
    gfmObject_clean(pCtx);

    /* Set the object's position */
    pCtx->t.x = _i2fp(x);
    pCtx->pb.lx = _i2fp(x);
    pCtx->t.y = _i2fp(y);
    pCtx->pb.ly = _i2fp(y);

    pCtx->t.hw = _i2fp(width) >> 1;
    pCtx->t.hh = _i2fp(height) >> 1;

    /* Set the object's parent */
    pCtx->t.pParent = pChild;
    pCtx->t.type = (uint32_t)type;
    pCtx->t.innerType = gfmType_object;

    return GFMRV_OK;
}

/**
 * Clear the object
 * 
 * @param  pCtx   The object
 * @return        GFMRV_OK, GFMRV_ARGUMENTS_BAD, ...
 */
gfmRV gfmObject_clean(gfmObject *pCtx) {
    if (pCtx == 0) {
        return GFMRV_ARGUMENTS_BAD;
    }

    /* Clean it up */
    memset(pCtx, 0x0, sizeof(gfmObject));

    return GFMRV_OK;
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
    assert(pCtx != 0);
    assert(width > 0);
    assert(height > 0);
    assert(width < (1 << 14));
    assert(height < (1 << 14));

    pCtx->t.hw = _i2fp(width) >> 1;
    pCtx->t.hh = _i2fp(height) >> 1;

    return GFMRV_OK;
}

/**
 * Set the object's dimension
 * 
 * @param  pCtx  The object
 * @param  width The object's width
 * @return       GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmObject_setHorizontalDimension(gfmObject *pCtx, int width) {
    assert(pCtx != 0);
    assert(width > 0);
    assert(width < (1 << 14));

    pCtx->t.hw = _i2fp(width) >> 1;

    return GFMRV_OK;
}

/**
 * Set the object's dimension
 * 
 * @param  pCtx   The object
 * @param  height The object's height
 * @return        GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmObject_setVerticalDimension(gfmObject *pCtx, int height) {
    assert(pCtx != 0);
    assert(height > 0);
    assert(height < (1 << 14));

    pCtx->t.hh = _i2fp(height) >> 1;

    return GFMRV_OK;
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
    assert(pCtx != 0);
    assert(pWidth != 0);
    assert(pHeight != 0);

    *pWidth = _fp2i(pCtx->t.hw << 1);
    *pHeight = _fp2i(pCtx->t.hh << 1);

    return GFMRV_OK;
}

/**
 * Get the object's width
 * 
 * @param  pWidth The object's width
 * @param  pCtx   The object
 * @return        GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmObject_getWidth(int *pWidth, gfmObject *pCtx) {
    assert(pCtx != 0);
    assert(pWidth != 0);

    *pWidth = _fp2i(pCtx->t.hw << 1);

    return GFMRV_OK;
}

/**
 * Get the object's height
 * 
 * @param  pHeight The object's height
 * @param  pCtx    The object
 * @return         GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmObject_getHeight(int *pHeight, gfmObject *pCtx) {
    assert(pCtx != 0);
    assert(pHeight != 0);

    *pHeight = _fp2i(pCtx->t.hh << 1);

    return GFMRV_OK;
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
    assert(pCtx != 0);

    pCtx->t.x = _i2fp(x);
    pCtx->t.y = _i2fp(y);

    return GFMRV_OK;
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
    assert(pCtx != 0);

    pCtx->t.x = _i2fp(x);

    return GFMRV_OK;
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
    assert(pCtx != 0);

    pCtx->t.y = _i2fp(y);

    return GFMRV_OK;
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
    assert(pCtx != 0);
    assert(pX != 0);
    assert(pY != 0);

    *pX = _fp2i(pCtx->t.x);
    *pY = _fp2i(pCtx->t.y);

    return GFMRV_OK;
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
    assert(pCtx != 0);
    assert(pX != 0);

    *pX = _fp2i(pCtx->t.x);

    return GFMRV_OK;
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
    assert(pCtx != 0);
    assert(pY != 0);

    *pY = _fp2i(pCtx->t.y);

    return GFMRV_OK;
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
    assert(pCtx != 0);
    if (pCtx->t.hw == 0 || pCtx->t.hh == 0) {
        return GFMRV_OBJECT_NOT_INITIALIZED;
    }

    pCtx->t.x = _i2fp(x) - pCtx->t.hw;
    pCtx->t.y = _i2fp(y) - pCtx->t.hh;

    return GFMRV_OK;
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
    assert(pCtx != 0);
    assert(pX != 0);
    assert(pY != 0);
    if (pCtx->t.hw == 0 || pCtx->t.hh == 0) {
        return GFMRV_OBJECT_NOT_INITIALIZED;
    }

    *pX = _fp2i(pCtx->t.x + pCtx->t.hw);
    *pY = _fp2i(pCtx->t.y + pCtx->t.hh);

    return GFMRV_OK;
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
    assert(pCtx != 0);
    assert(pX != 0);
    assert(pY != 0);
    if (pCtx->t.hw == 0 || pCtx->t.hh == 0) {
        return GFMRV_OBJECT_NOT_INITIALIZED;
    }

    *pX = _fp2i(pCtx->pb.lx + pCtx->t.hw);
    *pY = _fp2i(pCtx->pb.ly + pCtx->t.hh);

    return GFMRV_OK;
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
    assert(pCtx != 0);

    pCtx->pb.vx = _d2fp(vx);
    pCtx->pb.vy = _d2fp(vy);

    return GFMRV_OK;
}

/**
 * Set the object's velocity
 * 
 * @param  pCtx The object
 * @param  vx   The horizontal velocity
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmObject_setHorizontalVelocity(gfmObject *pCtx, double vx) {
    assert(pCtx != 0);

    pCtx->pb.vx = _d2fp(vx);

    return GFMRV_OK;
}

/**
 * Set the object's velocity
 * 
 * @param  pCtx The object
 * @param  vy   The vertical velocity
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmObject_setVerticalVelocity(gfmObject *pCtx, double vy) {
    assert(pCtx != 0);

    pCtx->pb.vy = _d2fp(vy);

    return GFMRV_OK;
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
    assert(pCtx != 0);
    assert(pVx != 0);
    assert(pVy != 0);

    *pVx = _fp2d(pCtx->pb.vx);
    *pVy = _fp2d(pCtx->pb.vy);

    return GFMRV_OK;
}

/**
 * Get the object's velocity
 * 
 * @param  pVx  The horizontal velocity
 * @param  pCtx The object
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmObject_getHorizontalVelocity(double *pVx, gfmObject *pCtx) {
    assert(pCtx != 0);
    assert(pVx != 0);

    *pVx = _fp2d(pCtx->pb.vx);

    return GFMRV_OK;
}

/**
 * Get the object's velocity
 * 
 * @param  pVy  The vertical velocity
 * @param  pCtx The object
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmObject_getVerticalVelocity(double *pVy, gfmObject *pCtx) {
    assert(pCtx != 0);
    assert(pVy != 0);

    *pVy = _fp2d(pCtx->pb.vy);

    return GFMRV_OK;
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
    assert(pCtx != 0);

    pCtx->pb.ax = _d2fp(ax);
    pCtx->pb.ay = _d2fp(ay);

    return GFMRV_OK;
}

/**
 * Set the object's acceleration
 * 
 * @param  pCtx The object
 * @param  ax   The object's horizontal acceleration
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmObject_setHorizontalAcceleration(gfmObject *pCtx, double ax) {
    assert(pCtx != 0);

    pCtx->pb.ax = _d2fp(ax);

    return GFMRV_OK;
}

/**
 * Set the object's acceleration
 * 
 * @param  pCtx The object
 * @param  ay   The object's vertical acceleration
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmObject_setVerticalAcceleration(gfmObject *pCtx, double ay) {
    assert(pCtx != 0);

    pCtx->pb.ay = _d2fp(ay);

    return GFMRV_OK;
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
    assert(pCtx != 0);
    assert(pAx != 0);
    assert(pAy != 0);

    *pAx = _fp2d(pCtx->pb.ax);
    *pAy = _fp2d(pCtx->pb.ay);

    return GFMRV_OK;
}

/**
 * Get the object's acceleration
 * 
 * @param  pAx  The object's horizontal acceleration
 * @param  pCtx The object
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmObject_getHorizontalAcceleration(double *pAx, gfmObject *pCtx) {
    assert(pCtx != 0);
    assert(pAx != 0);

    *pAx = _fp2d(pCtx->pb.ax);

    return GFMRV_OK;
}

/**
 * Get the object's acceleration
 * 
 * @param  pAy  The object's vertical acceleration
 * @param  pCtx The object
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmObject_getVerticalAcceleration(double *pAy, gfmObject *pCtx) {
    assert(pCtx != 0);
    assert(pAy != 0);

    *pAy = _fp2d(pCtx->pb.ay);

    return GFMRV_OK;
}

/**
 * Set the object's drag (i.e., how fast it will stop when there's no acc)
 * 
 * @param  pCtx The object
 * @param  dx   The horizontal drag
 * @param  dy   The vertical drag
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_NEGATIVE_DRAG
 */
gfmRV gfmObject_setDrag(gfmObject *pCtx, double dx, double dy) {
    return GFMRV_OK;
}

/**
 * Set the object's drag (i.e., how fast it will stop when there's no acc)
 * 
 * @param  pCtx The object
 * @param  dx   The horizontal drag
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_NEGATIVE_DRAG
 */
gfmRV gfmObject_setHorizontalDrag(gfmObject *pCtx, double dx) {
    return GFMRV_OK;
}

/**
 * Set the object's drag (i.e., how fast it will stop when there's no acc)
 * 
 * @param  pCtx The object
 * @param  dy   The vertical drag
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_NEGATIVE_DRAG
 */
gfmRV gfmObject_setVerticalDrag(gfmObject *pCtx, double dy) {
    return GFMRV_OK;
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
    if (pDx != 0) { *pDx = 0; }
    if (pDy != 0) { *pDy = 0; }
    return GFMRV_OK;
}

/**
 * Get the object's drag (i.e., how fast it will stop when there's no acc)
 * 
 * @param  pDx  The horizontal drag
 * @param  pCtx The object
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmObject_getHorizontalDrag(double *pDx, gfmObject *pCtx) {
    if (pDx != 0) { *pDx = 0; }
    return GFMRV_OK;
}

/**
 * Get the object's drag (i.e., how fast it will stop when there's no acc)
 * 
 * @param  pDy  The vertical drag
 * @param  pCtx The object
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmObject_getVerticalDrag(double *pDy, gfmObject *pCtx) {
    if (pDy != 0) { *pDy = 0; }
    return GFMRV_OK;
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
    assert(pCtx != 0);
    assert(pType != 0);

    if (ppChild != 0) {
        *ppChild = pCtx->t.pParent;
    }
    *pType = (int)pCtx->t.type;

    return GFMRV_OK;
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
    assert(pCtx != 0);

    pCtx->pb.flags |= EPBF_FIXED;

    return GFMRV_OK;
}

/**
 * Allow this object to move on collision
 * 
 * @param  pCtx The object
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmObject_setMovable(gfmObject *pCtx) {
    assert(pCtx != 0);

    pCtx->pb.flags &= ~EPBF_FIXED;

    return GFMRV_OK;
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
static inline void _gfmObject_integrate(int32_t *x, int32_t *vx, int32_t ax
        , int32_t elapsed) {
    *x += _fp_mul(*vx, elapsed);

    if (ax != 0.0) {
        int32_t tmp;

        tmp = _fp_mul(ax, elapsed);
        *vx += tmp;
        tmp = _fp_mul(tmp, elapsed);
        *x += tmp >> 1;
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
    assert(pCtx != 0);
    assert(pOther != 0);

    if (pCtx->t.hw == 0 || pCtx->t.hh == 0 || pOther->t.hw == 0
            || pOther->t.hh == 0) {
        return GFMRV_OBJECT_NOT_INITIALIZED;
    }

    /* Update its position with the other's translation */
    pCtx->t.x += pOther->t.x - pOther->pb.lx;
    pCtx->t.y += pOther->t.y - pOther->pb.ly;

    return GFMRV_OK;
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
    assert(pCtx != 0);
    assert(pOther != 0);

    if (pCtx->t.hw == 0 || pCtx->t.hh == 0 || pOther->t.hw == 0
            || pOther->t.hh == 0) {
        return GFMRV_OBJECT_NOT_INITIALIZED;
    }

    /* Update its position with the other's translation */
    pCtx->t.x += pOther->t.x - pOther->pb.lx;

    return GFMRV_OK;
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
    assert(pCtx != 0);
    assert(pOther != 0);

    if (pCtx->t.hw == 0 || pCtx->t.hh == 0 || pOther->t.hw == 0
            || pOther->t.hh == 0) {
        return GFMRV_OBJECT_NOT_INITIALIZED;
    }

    /* Update its position with the other's translation */
    pCtx->t.y += pOther->t.y - pOther->pb.ly;

    return GFMRV_OK;
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
    int32_t elapsed;

    assert(pObj != 0);
    assert(pCtx != 0);

    if (pObj->t.hw == 0 || pObj->t.hh == 0) {
        return GFMRV_OBJECT_NOT_INITIALIZED;
    }

    /* Retrieve the delta time */
    do {
        int tmp;

        /* Retrieve time in milliseconds and convert it to fp seconds */
        gfm_getElapsedTime(&tmp, pCtx);
        tmp <<= FP_DECIMAL_BIT;
        elapsed = tmp /= 1000;
    } while(0);

    /* Store the previous position */
    pObj->pb.lx = pObj->t.x;
    pObj->pb.ly = pObj->t.y;

    /* Integrate the position and velocity */
    _gfmObject_integrate(&pObj->t.x, &pObj->pb.vx, pObj->pb.ax, elapsed);
    _gfmObject_integrate(&pObj->t.y, &pObj->pb.vy, pObj->pb.ay, elapsed);

    /* Clear this frame's collisions and set the previous one */
    pObj->pb.flags &= ~gfmCollision_last;
    pObj->pb.flags |= (pObj->pb.flags & gfmCollision_cur) << 4;
    pObj->pb.flags &= ~gfmCollision_cur;

    return GFMRV_OK;
}

/** Get the horizontal distance between two objects as a fixed point number */
static int32_t _fp_getHorizontalDistance(gfmObject *pSelf, gfmObject *pOther) {
    int32_t a, b;

    assert(pSelf != 0);
    assert(pOther != 0);

    a = pSelf->t.x + pSelf->t.hw;
    b = pOther->t.x + pOther->t.hw;
    return _fp_abs(a - b);
}

/** Get the vertical distance between two objects as a fixed point number */
static int32_t _fp_getVerticalDistance(gfmObject *pSelf, gfmObject *pOther) {
    int32_t a, b;

    assert(pSelf != 0);
    assert(pOther != 0);

    a = pSelf->t.y + pSelf->t.hh;
    b = pOther->t.y + pOther->t.hh;
    return _fp_abs(a - b);
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
    assert(pDx != 0);
    assert(pDy != 0);

    *pDx = _fp2i(_fp_getHorizontalDistance(pSelf, pOther));
    *pDy = _fp2i(_fp_getVerticalDistance(pSelf, pOther));

    return GFMRV_OK;
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
    assert(pDx != 0);

    *pDx = _fp2i(_fp_getHorizontalDistance(pSelf, pOther));

    return GFMRV_OK;
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
    assert(pDy != 0);

    *pDy = _fp2i(_fp_getVerticalDistance(pSelf, pOther));

    return GFMRV_OK;
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
    assert(pDx != 0);

    *pDx = _fp2d(_fp_getHorizontalDistance(pSelf, pOther));

    return GFMRV_OK;
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
    assert(pDy != 0);

    *pDy = _fp2d(_fp_getVerticalDistance(pSelf, pOther));

    return GFMRV_OK;
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
    int32_t tmp;

    assert(pCtx != 0);
    if (pCtx->t.hw == 0 || pCtx->t.hh == 0) {
        return GFMRV_OBJECT_NOT_INITIALIZED;
    }

    tmp = pCtx->t.x + pCtx->t.hw;
    tmp -= _i2fp(x);
    if (tmp > pCtx->t.hw || tmp < -pCtx->t.hw) {
        return GFMRV_FALSE;
    }
    tmp = pCtx->t.y + pCtx->t.hh;
    tmp -= _i2fp(y);
    if (tmp > pCtx->t.hh || tmp < -pCtx->t.hh) {
        return GFMRV_FALSE;
    }

    return GFMRV_TRUE;
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
    int32_t dist, maxDist;

    assert(pSelf != 0);
    assert(pOther != 0);

    if (pSelf->t.hw == 0 || pSelf->t.hh == 0 || pOther->t.hw == 0
            || pOther->t.hh == 0) {
        return GFMRV_OBJECT_NOT_INITIALIZED;
    }

    dist = _fp_getHorizontalDistance(pSelf, pOther);
    maxDist = pSelf->t.hw + pOther->t.hw;
    if (dist > maxDist) {
        return GFMRV_FALSE;
    }

    dist = _fp_getVerticalDistance(pSelf, pOther);
    maxDist = pSelf->t.hh + pOther->t.hh;
    if (dist > maxDist) {
        return GFMRV_FALSE;
    }

    return GFMRV_TRUE;
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
    int32_t dist, lastDist, maxDist;

    assert(pSelf != 0);
    assert(pOther != 0);

    if (pSelf->t.hw == 0 || pSelf->t.hh == 0 || pOther->t.hw == 0
            || pOther->t.hh == 0) {
        return GFMRV_OBJECT_NOT_INITIALIZED;
    }

    /* Clear instantaneous hit */
    pSelf->pb.flags &= ~gfmInstantCollision_mask;
    pOther->pb.flags &= ~gfmInstantCollision_mask;

    /* Check if just collided horizontally */
    dist = _fp_getHorizontalDistance(pSelf, pOther);
    maxDist = pSelf->t.hw + pOther->t.hw;
    if (dist > maxDist) {
        return GFMRV_FALSE;
    }
    lastDist = pSelf->pb.lx + pSelf->t.hw;
    lastDist -= pOther->pb.lx + pOther->t.hw;
    if (lastDist >= maxDist || lastDist <= -maxDist) {
        if (dist + pSelf->t.hw <= pOther->t.hw
                || dist + pOther->t.hw <= pSelf->t.hw) {
            /* One of the entities was placed inside the other. Simply ignore */
        }
        else if (pSelf->t.x < pOther->t.x) {
            pSelf->pb.flags |= gfmInstantCollision_right;
            pOther->pb.flags |= gfmInstantCollision_left;
        }
        else {
            pSelf->pb.flags |= gfmInstantCollision_left;
            pOther->pb.flags |= gfmInstantCollision_right;
        }
    }

    /* Check if just collided vertically */
    dist = _fp_getVerticalDistance(pSelf, pOther);
    maxDist = pSelf->t.hh + pOther->t.hh;
    if (dist > maxDist) {
        return GFMRV_FALSE;
    }
    lastDist = pSelf->pb.ly + pSelf->t.hh;
    lastDist -= pOther->pb.ly + pOther->t.hh;
    if (lastDist >= maxDist || lastDist <= -maxDist) {
#if 0
        if (dist + pSelf->t.hh <= pOther->t.hh
                || dist + pOther->t.hh <= pSelf->t.hh) {
            /* One of the entities was placed inside the other. Simply ignore */
        }
        else 
#endif
        if (pSelf->t.y < pOther->t.y) {
            pSelf->pb.flags |= gfmInstantCollision_down;
            pOther->pb.flags |= gfmInstantCollision_up;
        }
        else {
            pSelf->pb.flags |= gfmInstantCollision_up;
            pOther->pb.flags |= gfmInstantCollision_down;
        }
    }

    /* Check if any collision actually happened */
    if ((pSelf->pb.flags & gfmInstantCollision_mask) == 0) {
        return GFMRV_FALSE;
    }

    /* Update the current collision */
    pSelf->pb.flags |= (pSelf->pb.flags & gfmInstantCollision_mask)
            >> EPBF_INSTANT_SHIFT;
    pOther->pb.flags |= (pOther->pb.flags & gfmInstantCollision_mask)
            >> EPBF_INSTANT_SHIFT;

    return GFMRV_TRUE;
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
    assert(pSelf != 0);
    assert(pOther != 0);

    if (pSelf->t.hw == 0 || pSelf->t.hh == 0 || pOther->t.hw == 0
            || pOther->t.hh == 0) {
        return GFMRV_OBJECT_NOT_INITIALIZED;
    }
    else if ((pSelf->pb.flags & EPBF_FIXED)
            && (pOther->pb.flags & EPBF_FIXED)) {
        return GFMRV_OBJECTS_CANT_COLLIDE;
    }

    if (gfmObject_justOverlaped(pSelf, pOther) != GFMRV_TRUE) {
        return GFMRV_FALSE;
    }
    if (pSelf->pb.flags & gfmInstantCollision_hor) {
        gfmObject_separateHorizontal(pSelf, pOther);
    }
    if (pSelf->pb.flags & gfmInstantCollision_ver) {
        gfmObject_separateVertical(pSelf, pOther);
    }

    return GFMRV_TRUE;
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

    assert(pSelf != 0);
    assert(pOther != 0);

    if (pSelf->t.hw == 0 || pSelf->t.hh == 0 || pOther->t.hw == 0
            || pOther->t.hh == 0) {
        return GFMRV_OBJECT_NOT_INITIALIZED;
    }
    else if ((pSelf->pb.flags & EPBF_FIXED)
            && (pOther->pb.flags & EPBF_FIXED)) {
        return GFMRV_OBJECTS_CANT_COLLIDE;
    }
    else if ((pSelf->pb.flags & gfmInstantCollision_hor) == 0) {
        return GFMRV_COLLISION_NOT_TRIGGERED;
    }

    /* Store which object is static (if any) and which may move */
    if (pOther->pb.flags & EPBF_FIXED) {
        pStatic = pOther;
        pMovable = pSelf;
    }
    else if (pSelf->pb.flags & EPBF_FIXED) {
        pStatic = pSelf;
        pMovable = pOther;
    }
    else {
        pStatic = 0;
    }

    if (pStatic && pMovable) {
        if (pMovable->pb.flags & gfmInstantCollision_left) {
            /* pMovable collided to the left, place it at static's right */
            pMovable->t.x = pStatic->t.x + (pStatic->t.hw << 1) + _i2fp(1);
        }
        else if (pMovable->pb.flags & gfmInstantCollision_right) {
            /* pMovable collided to the right, place it at static's left */
            pMovable->t.x = pStatic->t.x - (pMovable->t.hw << 1) - _i2fp(1);
        }
    }
    else {
        int32_t dist;

        dist = _fp_getHorizontalDistance(pSelf, pOther) >> 1;

        /* Push both objects */
        if (pSelf->pb.flags & gfmInstantCollision_left) {
            /* pSelf collided left, so it must be pushed to the right */
            pSelf->t.x += dist;
            pOther->t.x -= dist;
        }
        if (pSelf->pb.flags & gfmInstantCollision_right) {
            /* pSelf collided right, so it must be pushed to the left */
            pSelf->t.x -= dist;
            pOther->t.x += dist;
        }
    }

    return GFMRV_OK;
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

    assert(pSelf != 0);
    assert(pOther != 0);

    if (pSelf->t.hw == 0 || pSelf->t.hh == 0 || pOther->t.hw == 0
            || pOther->t.hh == 0) {
        return GFMRV_OBJECT_NOT_INITIALIZED;
    }
    else if ((pSelf->pb.flags & EPBF_FIXED)
            && (pOther->pb.flags & EPBF_FIXED)) {
        return GFMRV_OBJECTS_CANT_COLLIDE;
    }
    else if ((pSelf->pb.flags & gfmInstantCollision_ver) == 0) {
        return GFMRV_COLLISION_NOT_TRIGGERED;
    }

    /* Store which object is static (if any) and which may move */
    if (pOther->pb.flags & EPBF_FIXED) {
        pStatic = pOther;
        pMovable = pSelf;
    }
    else if (pSelf->pb.flags & EPBF_FIXED) {
        pStatic = pSelf;
        pMovable = pOther;
    }
    else {
        pStatic = 0;
    }

    if (pStatic && pMovable) {
        if (pMovable->pb.flags & gfmInstantCollision_up) {
            /* pMovable collided above, place it bellow static */
            pMovable->t.y = pStatic->t.y + (pStatic->t.hh << 1);
        }
        else if (pMovable->pb.flags & gfmInstantCollision_down) {
            /* pMovable collided bellow, place it above static */
            pMovable->t.y = pStatic->t.y - (pMovable->t.hh << 1);
        }
    }
    else {
        int32_t dist;

        dist = _fp_getVerticalDistance(pSelf, pOther) >> 1;

        /* Push both objects */
        if (pSelf->pb.flags & gfmInstantCollision_up) {
            /* pSelf collided above so it must be pushed downward */
            pSelf->t.y += dist;
            pOther->t.y -= dist;
        }
        if (pSelf->pb.flags & gfmInstantCollision_down) {
            /* pSelf collided bellow, so it must be pushed upward */
            pSelf->t.y -= dist;
            pOther->t.y += dist;
        }
    }

    return GFMRV_OK;
}

/**
 * Get the current collision/overlap flags
 * 
 * @param  pDir The collision direction
 * @param  pCtx The object
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmObject_getCollision(gfmCollision *pDir, gfmObject *pCtx) {
    assert(pDir != 0);
    assert(pCtx != 0);

    *pDir = (pCtx->pb.flags & gfmCollision_cur);

    return GFMRV_OK;
}

/**
 * Get the last frame's collision/overlap flags
 * 
 * @param  pDir The collision direction
 * @param  pCtx The object
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmObject_getLastCollision(gfmCollision *pDir, gfmObject *pCtx) {
    assert(pDir != 0);
    assert(pCtx != 0);

    *pDir = (pCtx->pb.flags & gfmCollision_last) >> 4;

    return GFMRV_OK;
}

/**
 * Get the collision/overlap flags from the last 'gfmObject_isOverlaping' call
 * 
 * @param  pDir The collision direction
 * @param  pCtx The object
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmObject_getCurrentCollision(gfmCollision *pDir, gfmObject *pCtx) {
    assert(pDir != 0);
    assert(pCtx != 0);

    *pDir = (pCtx->pb.flags & gfmInstantCollision_mask) >> EPBF_INSTANT_SHIFT;

    return GFMRV_OK;
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
    object.halfWidth = gfmFixedPoint_fromFloat((float)_fp2d(pCtx->t.hw));
    object.halfHeight = gfmFixedPoint_fromFloat((float)_fp2d(pCtx->t.hh));

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
 * @param  [ in]pCtx The object
 * @param  [ in]type The object's child new type
 * @return           GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmObject_setType(gfmObject *pCtx, int type) {
    assert(pCtx != 0);

    pCtx->t.type = (uint32_t)type;

    return GFMRV_OK;
}

