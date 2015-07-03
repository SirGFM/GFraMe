/**
 * @file src/include/GFraMe_int/gfmVirtualKey.h
 * 
 * Virtual key used by gfmInput
 */
#ifndef __GFMVIRTUALKEY_STRUCT__
#define __GFMVIRTUALKEY_STRUCT__

#include <GFraMe/gfmInput.h>

/** Virtual key structure  */
struct enGFMVirtualKey {
    /** Current state of the pointer device */
    gfmInputState state;
    /** How many times the buttons has been pressed consecutively */
    int num;
    /** Last time, since a common time frame, that the state changed */
    unsigned int lastTime;
};
typedef struct enGFMVirtualKey gfmVirtualKey;

#endif /* __GFMVIRTUALKEY_STRUCT__ */

#ifndef __GFMVIRTUALKEY_H_
#define __GFMVIRTUALKEY_H_

#include <GFraMe/gfmError.h>

/**
 * Alloc a new virtual key
 * 
 * @param  ppCtx The virtual key
 * @return       GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_ALLOC_FAILED
 */
gfmRV gfmVirtualKey_getNew(gfmVirtualKey **ppCtx);

/**
 * Releases a virtual key
 * 
 * @param  ppCtx The virtual key
 * @return       GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmVirtualKey_free(gfmVirtualKey **ppCtx);

/**
 * Initialize a virtual key (with all zeros)
 * 
 * @param  ppCtx The virtual key
 * @return       GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmVirtualKey_init(gfmVirtualKey *pCtx);

#endif /* __GFMVIRTUALKEY_H_ */

