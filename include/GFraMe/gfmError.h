/**
 * @file include/GFraMe/gfmError.h
 * 
 * Define error codes
 */
#ifndef __GFMERROR__
#define __GFMERROR__

enum enGFMError {
    GFMRV_OK = 0,
    GFMRV_ARGUMENTS_BAD,
    GFMRV_ALLOC_FAILED,
    GFMRV_MAX
}; /* enum enGFMError */
typedef enum enGFMError gfmRV;

#endif /* __GFMERROR__ */

