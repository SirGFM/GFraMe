/**
 * @file tst/gframe_geometry_tst.c
 *
 * Test if geometry primitives works.
 */
#include <GFraMe_int/gfmGeometry.h>

#include <stdio.h>

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

int main(int argc, char *argv[]) {
    int success = 0, failures = 0;

    printf("--== RUNNING ==--\n");

    TEST(test_valueInAxis(0, 10, 5) == 1);
    TEST(test_valueInAxis(-8, 5, -4) == 1);
    TEST(test_valueInAxis(0, 10, -2) == 0);
    TEST(test_valueInAxis(-8, 5, -40) == 0);
    TEST(test_valueInAxis(-8, 5, 6) == 0);

    TEST(test_axesIntersect(0, 10, -2, 11) == 1);
    TEST(test_axesIntersect(0, 10, 4, 20) == 1);
    TEST(test_axesIntersect(8, 12, 4, 10) == 1);
    TEST(test_axesIntersect(-5, 0, 1, 4) == 0);
    TEST(test_axesIntersect(5, 30, -21, 3) == 0);

#if 0
int gfmGeometry_isYInLineImage(gfmLine *pLine, gfmFixedPoint y);
int gfmGeometry_doesLinesIntersect(gfmLine *pLine1, gfmLine *pLine2);
inline gfmFixedPoint gfmGeometry_getLineY(gfmLine *pLine, gfmFixedPoint x);
int gfmGeometry_doesLineIntersectRect(gfmLine *pLine, gfmRect *pRect);
int gfmGeometry_isPointInsideRect(gfmRect *pRect, gfmPoint *pPoint);
int gfmGeometry_doesRectsIntersect(gfmRect *pRect1, gfmRect *pRect2);
#endif

    printf("--== RESULTS ==--\n");
    printf("Succeeded tests: %i\n", success);
    printf("Failed tests: %i\n", failures);
    printf("--=============--\n");
    /* Return 0 if there was no failure */
    return !(failures == 0);
}

