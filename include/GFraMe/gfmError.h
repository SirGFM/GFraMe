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
    GFMRV_TRUE,
    GFMRV_FALSE,
    GFMRV_ARGUMENTS_BAD,
    GFMRV_ALLOC_FAILED,
    GFMRV_INTERNAL_ERROR,
    GFMRV_FUNCTION_FAILED,
    GFMRV_INVALID_INDEX,
    GFMRV_FUNCTION_NOT_SUPPORTED,
    GFMRV_FUNCTION_NOT_IMPLEMENTED,
    // Main module errors
    GFMRV_BACKEND_ALREADY_INITIALIZED,
    GFMRV_BACKEND_NOT_INITIALIZED,
    GFMRV_TITLE_ALREADY_SET,
    GFMRV_TITLE_NOT_SET,
    GFMRV_WINDOW_ALREADY_INITIALIZED,
    GFMRV_WINDOW_NOT_INITIALIZED,
    // Window errors
    GFMRV_WINDOW_MODE_UNCHANGED,
    GFMRV_INVALID_WIDTH,
    GFMRV_INVALID_HEIGHT,
    GFMRV_WINDOW_IS_FULLSCREEN,
    // Backbuffer errors
    GFMRV_BACKBUFFER_NOT_INITIALIZED,
    GFMRV_BACKBUFFER_ALREADY_INITIALIZED,
    GFMRV_BACKBUFFER_WIDTH_INVALID,
    GFMRV_BACKBUFFER_HEIGHT_INVALID,
    GFMRV_BACKBUFFER_WINDOW_TOO_SMALL,
    GFMRV_BACKBUFFER_NO_TEXTURE_LOADED,
    // Texture errors
    GFMRV_TEXTURE_ALREADY_INITIALIZED,
    GFMRV_TEXTURE_INVALID_WIDTH,
    GFMRV_TEXTURE_INVALID_HEIGHT,
    GFMRV_TEXTURE_NOT_BITMAP,
    GFMRV_TEXTURE_FILE_NOT_FOUND,
    GFMRV_TEXTURE_NOT_INITIALIZED,
    // Spriteset errors
    GFMRV_SPRITESET_NOT_INITIALIZED,
    GFMRV_SPRITESET_INVALID_WIDTH,
    GFMRV_SPRITESET_INVALID_HEIGHT,
    // String errors
    GFMRV_STRING_WASNT_COPIED,
    GFMRV_STRING_NOT_INITIALIZED,
    GFMRV_STRING_TOO_SMALL,
    // Timer errors
    GFMRV_FPS_TOO_HIGH,
    GFMRV_TIMER_NOT_INITIALIZED,
    GFMRV_TIMER_ALREADY_INITIALIZED,
    GFMRV_FAILED_TO_STOP_TIMER,
    // Camera errors
    GFMRV_CAMERA_NOT_INITIALIZED,
    GFMRV_CAMERA_INVALID_WIDTH,
    GFMRV_CAMERA_INVALID_HEIGHT,
    GFMRV_CAMERA_MOVED,
    GFMRV_CAMERA_DIDNT_MOVE,
    // Tilemap errors
    GFMRV_TILEMAP_NOT_INITIALIZED,
    GFMRV_TILEMAP_NO_TILETYPE,
    GFMRV_TILEMAP_NO_TILEANIM,
    GFMRV_TILETYPE_EXTANT,
    GFMRV_TILEANIM_EXTANT,
    // Accumulator errors
    GFMRV_ACC_FPS_TOO_HIGH,
    GFMRV_ACC_NOT_INITIALIZED,
    // FPS Counter errors
    GFMRV_FPSCOUNTER_NOT_INITIALIZED,
    // Object errors
    GFMRV_OBJECT_NOT_INITIALIZED,
    GFMRV_NEGATIVE_DRAG,
    GFMRV_OBJECTS_CANT_COLLIDE,
    GFMRV_COLLISION_NOT_TRIGGERED,
    // Sprite errors
    GFMRV_SPRITE_NOT_INITIALIZED,
    GFMRV_NO_ANIMATION_PLAYING,
    // Animation errors
    GFMRV_ANIMATION_NOT_INITIALIZED,
    GFMRV_ANIMATION_ALREADY_INITIALIZED,
    GFMRV_ANIMATION_ALREADY_FINISHED,
    GFMRV_MAX
}; /* enum enGFMError */
typedef enum enGFMError gfmRV;

#endif /* __GFMERROR__ */

