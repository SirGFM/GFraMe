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
#ifndef __GFMGEOMETRY_STRUCT_H__
#define __GFMGEOMETRY_STRUCT_H__

/** 1d range, defined by its minimum and maximum values */
typedef struct stGfmAxis gfmAxis;
/** 2d point defined by its horizontal and vertical coordinates */
typedef struct stGfmPoint gfmPoint;
/** 2d line segment (can't represent vertical lines!) */
typedef struct stGfmLine gfmLine;
/** 2d rectangle, defined from its center */
typedef struct stGfmRect gfmRect;

#endif /* __GFMGEOMETRY_STRUCT_H__ */

#ifndef __GFMGEOMETRY_H__
#define __GFMGEOMETRY_H__

#include <GFraMe/gfmError.h>
#include <GFraMe_int/gfmFixedPoint.h>

/** 1d range, defined by its minimum and maximum values */
struct stGfmAxis {
    gfmFixedPoint lt;
    gfmFixedPoint gt;
};

/** 2d point defined by its horizontal and vertical coordinates */
struct stGfmPoint {
    gfmFixedPoint x;
    gfmFixedPoint y;
};

/**
 * 2d line segment (can't represent vertical lines!)
 * It's defined in a 'reduced format', i.e., it's vertical limits must be
 * calculated whenever necessary
 */
struct stGfmLine {
    /** Line angular coefficient (slope) */
    gfmFixedPoint a;
    /** Vertical coordinate that intersects with the horizontal origin */
    gfmFixedPoint b;
    /** Domain range (limits for valid horizontal coordinate) */
    gfmAxis x;
};

/** 2d rectangle, defined from its center */
struct stGfmRect {
    gfmFixedPoint centerX;
    gfmFixedPoint centerY;
    gfmFixedPoint halfWidth;
    gfmFixedPoint halfHeight;
};


/**
 * Check if a given value belongs to the defined axis
 *
 * @param  [ in]pAxis The axis
 * @param  [ in]value The value
 * @return            1 if it belongs, 0 otherwise
 */
inline int gfmGeometry_isValueInAxis(gfmAxis *pAxis, gfmFixedPoint value);

/**
 * Check if two axes intersect
 *
 * @param  [ in]pAxis1 A axis
 * @param  [ in]pAxis2 Another axis
 * @return            1 if they intersect, 0 otherwise
 */
inline int gfmGeometry_doesAxesIntersect(gfmAxis *pAxis1, gfmAxis *pAxis2);

/**
 * Check if a given value belongs to the image of a line segment
 *
 * @param  [ in]pLine The line
 * @param  [ in]y     The value
 * @return            1 if it belongs, 0 otherwise
 */
int gfmGeometry_isYInLineImage(gfmLine *pLine, gfmFixedPoint y);

/**
 * Check if two lines intersect
 *
 * @param  [ in]pLine1 A line
 * @param  [ in]pLine2 Another line
 * @return            1 if they intersect, 0 otherwise
 */
int gfmGeometry_doesLinesIntersect(gfmLine *pLine1, gfmLine *pLine2);

/**
 * Calculate the vertical coordinate of a given horizontal one
 *
 * @param  [ in]pLine The line
 * @param  [ in]x     The horizontal coordinate of the point
 * @return            The vertical coordinate of the given horizontal one
 */
inline gfmFixedPoint gfmGeometry_getLineY(gfmLine *pLine, gfmFixedPoint x);

/**
 * Check if a line intersect with a rectangle
 *
 * @param  [ in]pLine The line
 * @param  [ in]pRect The rectangle
 * @return            1 if they intersect, 0 otherwise
 */
int gfmGeometry_doesLineIntersectRect(gfmLine *pLine, gfmRect *pRect);

/**
 * Check if a point is inside a rectangle
 *
 * @param  [ in]pRect  The rectangle
 * @param  [ in]pPoint The point
 * @return             1 if they intersect, 0 otherwise
 */
int gfmGeometry_isPointInsideRect(gfmRect *pRect, gfmPoint *pPoint);

/**
 * Check if two rectangles intersect
 *
 * @param  [ in]pRect1 The rectangle
 * @param  [ in]pRect2 The rectangle
 * @return             1 if they intersect, 0 otherwise
 */
int gfmGeometry_doesRectsIntersect(gfmRect *pRect1, gfmRect *pRect2);


#endif /* __GFMGEOMETRY_H__ */

