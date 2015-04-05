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
    GFMRV_FPS_TOO_HIGH,
    GFMRV_FAILED_TO_INIT_TIMER,
    GFMRV_TIMER_NOT_INITIALIZED,
    GFMRV_FAILED_TO_STOP_TIMER,
    GFMRV_MAX
}; /* enum enGFMError */
typedef enum enGFMError gfmRV;

#endif /* __GFMERROR__ */

