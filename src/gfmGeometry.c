/**
 * @file src/include/GFraMe_int/gfmGeometry.h
 *
 * Define geometric primitives and intersection check between them.
 *
 * This module's writing breaks most of the code organization used all through
 * the library. It's intended as a test for how GFraMe v3 is supposed to work.
 * It should ease manually managing memory usage (in contrast to the previous
 * policy of alloc'ing everything through the library).
 *
 * Four primitives are defined:
 *   - axis: 1d range, defined by its minimum and maximum values
 *   - point: 2d point defined by its horizontal and vertical coordinates
 *   - line: 2d line segment (can't represent vertical lines!)
 *   - rectangle: 2d rectangle, defined from its center
 *
 * Every primitive attribute is a fixed point number, as defined in
 * gfmFixedPoint. Given the limited range, primitives must be normalized (i.e.,
 * brought into the same space) before overlaping.
 */
#include <GFraMe_int/gfmFixedPoint.h>
#include <GFraMe_int/gfmGeometry.h>

/**
 * Check if a given value belongs to the defined axis
 *
 * @param  [ in]pAxis The axis
 * @param  [ in]value The value
 * @return            1 if it belongs, 0 otherwise
 */
inline int gfmGeometry_isValueInAxis(gfmAxis *pAxis, gfmFixedPoint value) {
    return (value >= pAxis->lt) && (value <= pAxis->gt);
}

/**
 * Check if two axes intersect
 *
 * @param  [ in]pAxis1 A axis
 * @param  [ in]pAxis2 Another axis
 * @return            1 if they intersect, 0 otherwise
 */
inline int gfmGeometry_doesAxesIntersect(gfmAxis *pAxis1, gfmAxis *pAxis2) {
    return (pAxis1->gt >= pAxis2->lt) && (pAxis2->gt >= pAxis1->lt);
}

/**
 * Check if a given value belongs to the image of a line segment
 *
 * @param  [ in]pLine The line
 * @param  [ in]y     The value
 * @return            1 if it belongs, 0 otherwise
 */
int gfmGeometry_isYInLineImage(gfmLine *pLine, gfmFixedPoint y) {
    gfmAxis vertical;

    vertical.lt = gfmGeometry_getLineY(pLine, pLine->x.lt);
    vertical.gt = gfmGeometry_getLineY(pLine, pLine->x.gt);
    if (vertical.lt > vertical.gt) {
        gfmFixedPoint tmp;

        tmp = vertical.lt;
        vertical.lt = vertical.gt;
        vertical.gt = tmp;
    }

    return gfmGeometry_isValueInAxis(&vertical, y);
}

/**
 * Check if two lines intersect
 *
 * @param  [ in]pLine1 A line
 * @param  [ in]pLine2 Another line
 * @return            1 if they intersect, 0 otherwise
 */
int gfmGeometry_doesLinesIntersect(gfmLine *pLine1, gfmLine *pLine2) {
    gfmFixedPoint x;

    /* a1 == a2 implies parallel lines; In that case, they shall only intersect
     * if they are equal (i.e., if b1 == b2 )*/
    if (pLine1->a == pLine2->a && pLine1->b == pLine2->b) {
        /* If the lines overlap, their domain must also intersect */
        return gfmGeometry_doesAxesIntersect(&pLine1->x, &pLine2->x);
    }
    else if (pLine1->a == pLine2->a && pLine1->b != pLine2->b) {
        return 0;
    }

    /* Check if the intesection point belongs to both lines' domains */
    x = gfmFixedPoint_div(pLine2->b - pLine1->b, pLine1->a - pLine2->a);
    return gfmGeometry_isValueInAxis(&pLine1->x, x) &&
            gfmGeometry_isValueInAxis(&pLine2->x, x);
}

/**
 * Calculate the vertical coordinate of a given horizontal one
 *
 * @param  [ in]pLine The line
 * @param  [ in]x     The horizontal coordinate of the point
 * @return            The vertical coordinate of the given horizontal one
 */
inline gfmFixedPoint gfmGeometry_getLineY(gfmLine *pLine, gfmFixedPoint x) {
    return gfmFixedPoint_mul(x, pLine->a) + pLine->b;
}

/**
 * Check if a line intersect with a rectangle
 *
 * @param  [ in]pLine The line
 * @param  [ in]pRect The rectangle
 * @return            1 if they intersect, 0 otherwise
 */
int gfmGeometry_doesLineIntersectRect(gfmLine *pLine, gfmRect *pRect) {
    gfmLine horizontal;
    gfmPoint point1, point2;
    gfmFixedPoint y;

    /* Horizontal line, with the same length as the rect's width (note: b
     * defines the y value, so it must be indepedently set) */
    horizontal.a = 0;
    horizontal.x.lt = pRect->centerX - pRect->halfWidth;
    horizontal.x.gt = pRect->centerX + pRect->halfWidth;

    /* Check it intersects with the upper edge */
    horizontal.b = pRect->centerY - pRect->halfHeight;
    if (gfmGeometry_doesLinesIntersect(pLine, &horizontal)) {
        return 1;
    }

    /* Check it intersects with the lower edge */
    horizontal.b = pRect->centerY + pRect->halfHeight;
    if (gfmGeometry_doesLinesIntersect(pLine, &horizontal)) {
        return 1;
    }

    /* Check if it intersects with the left edge */
    y = gfmGeometry_getLineY(pLine, horizontal.x.lt);
    if (gfmGeometry_isYInLineImage(pLine, y)) {
        return 1;
    }

    /* Check if it intersects with the right edge */
    y = gfmGeometry_getLineY(pLine, horizontal.x.gt);
    if (gfmGeometry_isYInLineImage(pLine, y)) {
        return 1;
    }

    /* Check if the line is inside the rect */
    point1.x = pLine->x.lt;
    point1.y = gfmGeometry_getLineY(pLine, point1.x);
    point2.x = pLine->x.gt;
    point2.y = gfmGeometry_getLineY(pLine, point2.x);
    return gfmGeometry_isPointInsideRect(pRect, &point1) &&
            gfmGeometry_isPointInsideRect(pRect, &point2);
}

/**
 * Check if a point is inside a rectangle
 *
 * @param  [ in]pRect  The rectangle
 * @param  [ in]pPoint The point
 * @return             1 if they intersect, 0 otherwise
 */
int gfmGeometry_isPointInsideRect(gfmRect *pRect, gfmPoint *pPoint) {
    gfmFixedPoint distanceX, distanceY;
    distanceX = gfmFixedPoint_abs(pRect->centerX - pPoint->x);
    distanceY = gfmFixedPoint_abs(pRect->centerY - pPoint->y);
    return (distanceX <= pRect->halfWidth) && (distanceY <= pRect->halfHeight);
}

/**
 * Check if two rectangles intersect
 *
 * @param  [ in]pRect1 The rectangle
 * @param  [ in]pRect2 The rectangle
 * @return             1 if they intersect, 0 otherwise
 */
int gfmGeometry_doesRectsIntersect(gfmRect *pRect1, gfmRect *pRect2) {
    gfmFixedPoint distanceX, distanceY;
    distanceX = gfmFixedPoint_abs(pRect1->centerX - pRect2->centerX);
    distanceY = gfmFixedPoint_abs(pRect1->centerY - pRect2->centerY);
    return (distanceX <= pRect1->halfWidth + pRect2->halfWidth) &&
            (distanceY <= pRect1->halfHeight + pRect2->halfHeight);
}

