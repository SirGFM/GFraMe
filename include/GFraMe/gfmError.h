/**
 * @file include/GFraMe/gfmError.h
 * 
 * Define error codes
 */
#ifndef __GFMERROR__
#define __GFMERROR__

enum enGFMError {
    // Generic errors
    GFMRV_OK = 0,
    GFMRV_ARGUMENTS_BAD,
    GFMRV_ALLOC_FAILED,
    GFMRV_INTERNAL_ERROR,
    // Main module errors
    GFMRV_BACKEND_ALREADY_INITIALIZED,
    GFMRV_BACKEND_NOT_INITIALIZED,
    GFMRV_TITLE_ALREADY_SET,
    GFMRV_TITLE_NOT_SET,
    // String errors
    GFMRV_STRING_WASNT_COPIED,
    GFMRV_STRING_NOT_INITIALIZED,
    // Timer errors
    GFMRV_FPS_TOO_HIGH,
    GFMRV_FAILED_TO_INIT_TIMER,
    GFMRV_TIMER_NOT_INITIALIZED,
    GFMRV_FAILED_TO_STOP_TIMER,
    GFMRV_MAX
}; /* enum enGFMError */
typedef enum enGFMError gfmRV;

#endif /* __GFMERROR__ */

