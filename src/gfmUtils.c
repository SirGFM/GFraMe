/**
 * @file src/gfmUtils.c
 * 
 * Module with diverse utilities
 */
#include <GFraMe/gfmAssert.h>
#include <GFraMe/gfmError.h>
#include <GFraMe/gfmUtils.h>

/**
 * Check if a given value is a power of two
 * 
 * @param  n The number
 * @return   GFMRV_TRUE, GFMRV_FALSE
 */
gfmRV gfmUtils_isPow2(int n) {
    gfmRV rv;
    int foundBit;
    
    foundBit = 0;
    while (n) {
        if (n & 1) {
            // Check that this was the first bit found
            ASSERT(!foundBit, GFMRV_FALSE);
            foundBit = 1;
        }
        
        // Got to the next bit
        n >>= 1;
    }
    // Assert that a bit was found
    ASSERT(foundBit, GFMRV_FALSE);
    
    rv = GFMRV_TRUE;
__ret:
    return rv;
}

