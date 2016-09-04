/**
 * @file tst/gframe_fixedpoint_tst.c
 *
 * Test if fixedpoint primitives works.
 */
#include <GFraMe_int/gfmFixedPoint.h>

#include <stdio.h>

static inline float _fabs(float value) {
    return (value >= 0.0) ? value : -value;
}

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

/** Ignore parameters 2, 4 and 6 so they may be used to print names */
#define gfmFixedPoint_test(op, _1, a, _2, b, _3, res) test_ ## op (a, b, res)

int test_fsub(float a, float b, float res) {
    gfmFixedPoint fpA, fpB, fpRes;

    fpA = gfmFixedPoint_fromFloat(a);
    fpB = gfmFixedPoint_fromFloat(b);
    fpRes = fpA - fpB;

    return _fabs(res - (fpRes * GFM_FIXED_POINT_ERROR))
            <= GFM_FIXED_POINT_ERROR;
}

int test_imul(int a, int b, int res) {
    gfmFixedPoint fpA, fpB, fpRes;

    fpA = gfmFixedPoint_fromInt(a);
    fpB = gfmFixedPoint_fromInt(b);
    fpRes = gfmFixedPoint_fromInt(res);

    return fpRes == gfmFixedPoint_mul(fpA, fpB);
}

int test_fmul(float a, float b, float res) {
    gfmFixedPoint fpA, fpB, fpRes;

    fpA = gfmFixedPoint_fromFloat(a);
    fpB = gfmFixedPoint_fromFloat(b);
    fpRes = gfmFixedPoint_mul(fpA, fpB);

    return _fabs(res - (fpRes * GFM_FIXED_POINT_ERROR))
            <= GFM_FIXED_POINT_ERROR;
}

int test_idiv(int a, int b, int res) {
    gfmFixedPoint fpA, fpB, fpRes;

    fpA = gfmFixedPoint_fromInt(a);
    fpB = gfmFixedPoint_fromInt(b);
    fpRes = gfmFixedPoint_fromInt(res);

    return fpRes == gfmFixedPoint_div(fpA, fpB);
}

int test_fdiv(float a, float b, float res) {
    gfmFixedPoint fpA, fpB, fpRes;

    fpA = gfmFixedPoint_fromFloat(a);
    fpB = gfmFixedPoint_fromFloat(b);
    fpRes = gfmFixedPoint_div(fpA, fpB);

    return _fabs(res - (fpRes * GFM_FIXED_POINT_ERROR))
            <= GFM_FIXED_POINT_ERROR;
}

int main(int argc, char *argv[]) {
    int success = 0, failures = 0;

    printf("--== RUNNING ==--\n");

    TEST(gfmFixedPoint_fromInt(1) == 0x40);
    TEST(gfmFixedPoint_fromInt(-1) == 0xFFFFFFC0);
    TEST(gfmFixedPoint_fromInt(3) == 0xC0);
    TEST(gfmFixedPoint_fromInt(7) == 0x1C0);
    TEST(gfmFixedPoint_fromInt(-25) == 0xFFFFF9C0);
    TEST(gfmFixedPoint_fromFloat(0.5f) == 0x20);
    TEST(gfmFixedPoint_fromFloat(0.25f) == 0x10);
    TEST(gfmFixedPoint_fromFloat(-0.5f) == 0xFFFFFFE0);
    TEST(gfmFixedPoint_fromFloat(0.3f) == 0x13);

    TEST(gfmFixedPoint_test(fsub, a=, 3.0f, b=, 2.7f, res=, 0.3f));
    TEST(gfmFixedPoint_test(fsub, a=, 27.53f, b=, 6.7f, res=, 20.83f));

    TEST(gfmFixedPoint_test(imul, a=, 1, b=, 2, res=, 2));
    TEST(gfmFixedPoint_test(imul, a=, -1, b=, 2, res=, -2));
    TEST(gfmFixedPoint_test(imul, a=, 3, b=, 7, res=, 21));
    TEST(gfmFixedPoint_test(imul, a=, -4, b=, 15, res=, -60));

    TEST(gfmFixedPoint_test(fmul, a=, -0.5f, b=, 4.31f, res=, -2.155f));
    TEST(gfmFixedPoint_test(fmul, a=, 13.45f, b=, 34.21f, res=, 460.1245f));

    TEST(gfmFixedPoint_test(idiv, a=, 6, b=, 2, res=, 3));
    TEST(gfmFixedPoint_test(idiv, a=, -52, b=, 13, res=, -4));

    TEST(gfmFixedPoint_test(fdiv, a=, 0.5f, b=, 0.25f, res=, 2.0f));
    TEST(gfmFixedPoint_test(fdiv, a=, 0.37f, b=, 11.0f, res=, 0.033636f));
    TEST(gfmFixedPoint_test(fdiv, a=, 13.0f, b=, 3.1f, res=, 4.193548387f));

    printf("--== RESULTS ==--\n");
    printf("Succeeded tests: %i\n", success);
    printf("Failed tests: %i\n", failures);
    printf("--=============--\n");
    /* Return 0 if there was no failure */
    return !(failures == 0);
}

