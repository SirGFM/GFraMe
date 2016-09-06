/**
 * @file tst/gframe_geometry_tst.c
 *
 * Test if geometry primitives works.
 */
#include <GFraMe_int/gfmGeometry.h>

#include <stdio.h>

/** AXIS: lt = minimum value, gt = maximum value */
#define AXIS(lt, gt) lt, gt
/** LINE: lt = minimum value, gt = maximum value, a = angular coefficient,
 * b = offset */
#define LINE(lt, gt, a, b) lt, gt, a, b
/** POINT: x = horizontal coordinate, y = vertical coordinate */
#define POINT(x, y) x, y
/** RECT: cx = horizontal coordinate, cy = vertical coordinate, hw = half width,
 * hh = half height */
#define RECT(cx, cy, hw, hh) cx, cy, hw, hh

#define TEST(stmt) \
    do { \
        if (!(stmt)) { \
            printf("Failure (@%i): " #stmt "\n", __LINE__); \
            failures++; \
        } \
        else { \
            success++; \
        } \
    } while (0)

int test_valueInAxis(int lt, int gt, int val) {
    gfmAxis axis = {.lt = gfmFixedPoint_fromInt(lt)
            , .gt = gfmFixedPoint_fromInt(gt)};
    return gfmGeometry_isValueInAxis(&axis, gfmFixedPoint_fromInt(val));
}

int test_axesIntersect(int lt1, int gt1, int lt2, int gt2) {
    gfmAxis axis1 = {.lt = gfmFixedPoint_fromInt(lt1)
            , .gt = gfmFixedPoint_fromInt(gt1)};
    gfmAxis axis2 = {.lt = gfmFixedPoint_fromInt(lt2)
            , .gt = gfmFixedPoint_fromInt(gt2)};
    return gfmGeometry_doesAxesIntersect(&axis1, &axis2);
}

int test_linePoint(int lt, int gt, float a, int b, float x, float y) {
    gfmLine line = {.a = gfmFixedPoint_fromFloat(a)
            , .b = gfmFixedPoint_fromInt(b)
            , .x = {.lt = gfmFixedPoint_fromInt(lt)
                    , .gt = gfmFixedPoint_fromInt(gt)}};
    return gfmFixedPoint_fromFloat(y)
            == gfmGeometry_getLineY(&line, gfmFixedPoint_fromFloat(x));
}

int test_lineY(int lt, int gt, float a, int b, float y) {
    gfmLine line = {.a = gfmFixedPoint_fromFloat(a)
            , .b = gfmFixedPoint_fromInt(b)
            , .x = {.lt = gfmFixedPoint_fromInt(lt)
                    , .gt = gfmFixedPoint_fromInt(gt)}};
    return gfmGeometry_isYInLineImage(&line, gfmFixedPoint_fromFloat(y));
}

int test_lineIntersect(int lt1, int gt1, float a1, int b1, int lt2, int gt2
        , float a2, int b2) {
    gfmLine line1 = {.a = gfmFixedPoint_fromFloat(a1)
            , .b = gfmFixedPoint_fromInt(b1)
            , .x = {.lt = gfmFixedPoint_fromInt(lt1)
                    , .gt = gfmFixedPoint_fromInt(gt1)}};
    gfmLine line2 = {.a = gfmFixedPoint_fromFloat(a2)
            , .b = gfmFixedPoint_fromInt(b2)
            , .x = {.lt = gfmFixedPoint_fromInt(lt2)
                    , .gt = gfmFixedPoint_fromInt(gt2)}};
    return gfmGeometry_doesLinesIntersect(&line1, &line2);
}

int test_pointRect(float cx, float cy, float hw, float hh, float x, float y) {
    gfmPoint point = {.x = gfmFixedPoint_fromFloat(x)
            , .y = gfmFixedPoint_fromFloat(y)};
    gfmRect rect = {.centerX = gfmFixedPoint_fromFloat(cx),
            .centerY = gfmFixedPoint_fromFloat(cy),
            .halfWidth = gfmFixedPoint_fromFloat(hw),
            .halfHeight = gfmFixedPoint_fromFloat(hh)};
    return gfmGeometry_isPointInsideRect(&rect, &point);
}

int main(int argc, char *argv[]) {
    int success = 0, failures = 0;

    printf("--== RUNNING ==--\n");

    TEST(test_valueInAxis(AXIS( 0, 10),   5) == 1);
    TEST(test_valueInAxis(AXIS(-8,  5),  -4) == 1);
    TEST(test_valueInAxis(AXIS( 0, 10),  -2) == 0);
    TEST(test_valueInAxis(AXIS(-8,  5), -40) == 0);
    TEST(test_valueInAxis(AXIS(-8,  5),   6) == 0);

    TEST(test_axesIntersect(AXIS( 0, 10), AXIS( -2, 11)) == 1);
    TEST(test_axesIntersect(AXIS( 0, 10), AXIS(  4, 20)) == 1);
    TEST(test_axesIntersect(AXIS( 8, 12), AXIS(  4, 10)) == 1);
    TEST(test_axesIntersect(AXIS(-5,  0), AXIS(  1,  4)) == 0);
    TEST(test_axesIntersect(AXIS( 5, 30), AXIS(-21,  3)) == 0);

    TEST(test_linePoint(LINE(0, 5, 1, 0), POINT(0, 0)) == 1);
    TEST(test_linePoint(LINE(0, 5, 1, 0), POINT(2.5, 2.5)) == 1);
    TEST(test_linePoint(LINE(0, 5, 1, 0), POINT(5, 5)) == 1);
    /* The two following tests must pass since the function
     * 'gfmGeometry_getLineY' simply calculates the vertical coordinate of a
     * point, but ignores whether or not it's within the line segment */
    TEST(test_linePoint(LINE(0, 5, 1, 0), POINT(-1, -1)) == 1);
    TEST(test_linePoint(LINE(0, 5, 1, 0), POINT(6, 6)) == 1);

    TEST(test_linePoint(LINE(0, 5, 1, 0), POINT(6, 5)) == 0);
    /* TODO Write tests for non trivial angular coefficient */

    TEST(test_lineY(LINE(0, 5, 1, 0),  0) == 1);
    TEST(test_lineY(LINE(0, 5, 1, 0),  5) == 1);
    TEST(test_lineY(LINE(0, 5, 1, 0), -1) == 0);
    TEST(test_lineY(LINE(0, 5, 1, 0),  6) == 0);
    /* TODO Write tests for non trivial angular coefficient */

    TEST(test_lineIntersect(LINE(0, 5, 1, 0), LINE(0, 5, -1, 0)) == 1);
    TEST(test_lineIntersect(LINE(0, 5, 1, 0), LINE(0, 5,  1, 1)) == 0);
    TEST(test_lineIntersect(LINE(-2, 3, 0.75f, 1), LINE(-2, -1,  1.333f, 2)) == 1);
    TEST(test_lineIntersect(LINE(-2, 3, 0.75f, 1), LINE(0, 4,  1.333f, 2)) == 0);
    TEST(test_lineIntersect(LINE(-2, 3, 0.75f, 1), LINE(-4, -2,  1.333f, 2)) == 0);

    TEST(test_pointRect(RECT(0, 0, 1, 1), POINT(0, 0)) == 1);
    TEST(test_pointRect(RECT(5, 3, 1, 1), POINT(0, 0)) == 0);
    /* TODO Write tests for non trivial rects/points */

#if 0
int gfmGeometry_doesLineIntersectRect(gfmLine *pLine, gfmRect *pRect);
int gfmGeometry_doesRectsIntersect(gfmRect *pRect1, gfmRect *pRect2);
#endif

    printf("--== RESULTS ==--\n");
    printf("Succeeded tests: %i\n", success);
    printf("Failed tests: %i\n", failures);
    printf("--=============--\n");
    /* Return 0 if there was no failure */
    return !(failures == 0);
}

