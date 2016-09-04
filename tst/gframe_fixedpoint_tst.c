/**
 * @file tst/gframe_fixedpoint_tst.c
 *
 * Test if fixedpoint primitives works.
 */
#include <GFraMe_int/gfmFixedPoint.h>

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

int main(int argc, char *argv[]) {
    int success = 0, failures = 0;

    printf("--== RUNNING ==--\n");

    TEST(gfmFixedPoint_fromInt(1) == 0x40);
    TEST(gfmFixedPoint_fromInt(-1) == 0xFFFFFFC0);
    TEST(gfmFixedPoint_fromFloat(0.5) == 0x20);
    TEST(gfmFixedPoint_fromFloat(0.25) == 0x10);
    TEST(gfmFixedPoint_fromFloat(-0.5) == 0xFFFFFFE0);

    printf("--== RESULTS ==--\n");
    printf("Succeeded tests: %i\n", success);
    printf("Failed tests: %i\n", failures);
    printf("--=============--\n");
    /* Return 0 if there was no failure */
    return !(failures == 0);
}

