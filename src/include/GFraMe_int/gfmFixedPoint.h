/**
 * @file src/include/GFraMe_int/gfmFixedPoint.h
 *
 * Define a fixed point number with its ranges and operations.
 */
#ifndef __GFMFIXEDPOINT_TYPE__
#define __GFMFIXEDPOINT_TYPE__
#include <stdint.h>

/** Default fixed point */
typedef int16_t gfmFixedPoint;
/** Promoted fixed point. Required on multiplication and division */
typedef int32_t gfmPromotedFixedPoint;
/** Number of bits used to represent the fractional part */
#define GFM_FRACTION_BITS 6
/** Maximum error when using fixed point, as a float */
#define GFM_FIXED_POINT_ERROR ((float)(1.0f / (1 << GFM_FRACTION_BITS)))

#endif /* __GFMFIXEDPOINT_TYPE__ */

#ifndef __GFMFIXEDPOINT_H__
#define __GFMFIXEDPOINT_H__

/**
 * Retrieve a fixed point absolute value
 *
 * @param  [ in]value A value
 * @return            The value without signal
 */
inline static gfmFixedPoint gfmFixedPoint_abs(gfmFixedPoint value) {
    if (value < 0) {
        return -value;
    }
    return value;
}

/**
 * Multiply two fixed point numbers
 *
 * @param  [ in]a A factor
 * @param  [ in]b Another factor
 * @return        The product
 */
inline static gfmFixedPoint gfmFixedPoint_mul(gfmFixedPoint a,
        gfmFixedPoint b) {
    return (gfmFixedPoint)((a * (gfmPromotedFixedPoint)b) >> GFM_FRACTION_BITS);
}

/**
 * Divide a fixed point number
 *
 * @param  [ in]a The dividend
 * @param  [ in]b The divisor
 * @return        The quotient
 */
inline static gfmFixedPoint gfmFixedPoint_div(gfmFixedPoint a,
        gfmFixedPoint b) {
    gfmPromotedFixedPoint div = ((gfmPromotedFixedPoint)a) << GFM_FRACTION_BITS;
    return (gfmFixedPoint)(div / b);
}

/**
 * Convert an integer to a fixed point number
 *
 * @param  [ in]val The value
 * @return          The value represented in fixed point notation
 */
inline static gfmFixedPoint gfmFixedPoint_fromInt(int val) {
    return (gfmFixedPoint)(val << GFM_FRACTION_BITS);
}

/**
 * Convert float point number to a fixed point number
 *
 * @param  [ in]val The value
 * @return          The value represented in fixed point notation
 */
inline static gfmFixedPoint gfmFixedPoint_fromFloat(float val) {
    gfmFixedPoint integer, fraction;
    int signal;

    if (val < 0.0f ) {
        val *= -1.0f;
        signal = 1;
    }
    else {
        signal = 0;
    }

    integer = (gfmFixedPoint)val;
    val -= (float)integer;
    fraction = 0;
    while (fraction < GFM_FRACTION_BITS) {
        val *= 2.0f;
        fraction++;
    }
    fraction = (gfmFixedPoint)val;

    if (signal) {
        return -(gfmFixedPoint)((integer << GFM_FRACTION_BITS) | fraction);
    }
    return (gfmFixedPoint)((integer << GFM_FRACTION_BITS) | fraction);
}

#endif /* __GFMFIXEDPOINT_H__ */

