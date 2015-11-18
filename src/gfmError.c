/**
 * @file src/gfmError.c
 * 
 * Define error codes
 */
#include <GFraMe/gfmError.h>


/** Dictionary of error strings */
const char *gfmError_dict[GFMRV_MAX + 1] = {
    // Generic errors
    "Operation OK", /* GFMRV_OK */
    "true", /* GFMRV_TRUE */
    "false", /* GFMRV_FALSE */
    "Library not initialized", /* GFMRV_NOT_INITIALIZED */
    "Library already initialized", /* GFMRV_ALREADY_INITIALIZED */
    "Arguments bad", /* GFMRV_ARGUMENTS_BAD */
    "Alloc failed", /* GFMRV_ALLOC_FAILED */
    "Internal error", /* GFMRV_INTERNAL_ERROR */
    "Function failed", /* GFMRV_FUNCTION_FAILED */
    "Invalid index", /* GFMRV_INVALID_INDEX */
    "Invalid buffer len", /* GFMRV_INVALID_BUFFER_LEN */
    "Couldn't open file", /* GFMRV_COULDNT_OPEN_FILE */
    "Read error", /* GFMRV_READ_ERROR */
    "Buffer too small", /* GFMRV_BUFFER_TOO_SMALL */
    "Operation active", /* GFMRV_OPERATION_ACTIVE */
    "Operation not active", /* GFMRV_OPERATION_NOT_ACTIVE */
    "Waiting...", /* GFMRV_WAITING */
    "Function not supported", /* GFMRV_FUNCTION_NOT_SUPPORTED */
    "Function not implemented", /* GFMRV_FUNCTION_NOT_IMPLEMENTED */
    // Main module errors
    "Backend already initialized", /* GFMRV_BACKEND_ALREADY_INITIALIZED */
    "Backend not initialized", /* GFMRV_BACKEND_NOT_INITIALIZED */
    "Title already set", /* GFMRV_TITLE_ALREADY_SET */
    "Title not set", /* GFMRV_TITLE_NOT_SET */
    "Window already initialized", /* GFMRV_WINDOW_ALREADY_INITIALIZED */
    "Window not initialized", /* GFMRV_WINDOW_NOT_INITIALIZED */
    // Window errors
    "Window mode unchanged", /* GFMRV_WINDOW_MODE_UNCHANGED */
    "Invalid width", /* GFMRV_INVALID_WIDTH */
    "Invalid height", /* GFMRV_INVALID_HEIGHT */
    "Window is fullscreen", /* GFMRV_WINDOW_IS_FULLSCREEN */
    // Backbuffer errors
    "Backbuffer not initialized", /* GFMRV_BACKBUFFER_NOT_INITIALIZED */
    "Backbuffer already initialized", /* GFMRV_BACKBUFFER_ALREADY_INITIALIZED */
    "Backbuffer width invalid", /* GFMRV_BACKBUFFER_WIDTH_INVALID */
    "Backbuffer height invalid", /* GFMRV_BACKBUFFER_HEIGHT_INVALID */
    "Backbuffer window too small", /* GFMRV_BACKBUFFER_WINDOW_TOO_SMALL */
    "Backbuffer no texture loaded", /* GFMRV_BACKBUFFER_NO_TEXTURE_LOADED */
    // Texture errors
    "Texture already initialized", /* GFMRV_TEXTURE_ALREADY_INITIALIZED */
    "Texture invalid width", /* GFMRV_TEXTURE_INVALID_WIDTH */
    "Texture invalid height", /* GFMRV_TEXTURE_INVALID_HEIGHT */
    "Texture not a bitmap", /* GFMRV_TEXTURE_NOT_BITMAP */
    "Texture file not found", /* GFMRV_TEXTURE_FILE_NOT_FOUND */
    "Texture not initialized", /* GFMRV_TEXTURE_NOT_INITIALIZED */
    // Spriteset errors
    "Spriteset not initialized", /* GFMRV_SPRITESET_NOT_INITIALIZED */
    "Spriteset invalid width", /* GFMRV_SPRITESET_INVALID_WIDTH */
    "Spriteset invalid height", /* GFMRV_SPRITESET_INVALID_HEIGHT */
    // String errors
    "String wasn't copied", /* GFMRV_STRING_WASNT_COPIED */
    "String not initialized", /* GFMRV_STRING_NOT_INITIALIZED */
    "String too small", /* GFMRV_STRING_TOO_SMALL */
    // Timer errors
    "FPS too high", /* GFMRV_FPS_TOO_HIGH */
    "Timer not initialized", /* GFMRV_TIMER_NOT_INITIALIZED */
    "Timer already initialized", /* GFMRV_TIMER_ALREADY_INITIALIZED */
    "Failed to stop the timer", /* GFMRV_FAILED_TO_STOP_TIMER */
    // Camera errors
    "Camera not initialized", /* GFMRV_CAMERA_NOT_INITIALIZED */
    "Camera invalid width", /* GFMRV_CAMERA_INVALID_WIDTH */
    "Camera invalid height", /* GFMRV_CAMERA_INVALID_HEIGHT */
    "Camera moved", /* GFMRV_CAMERA_MOVED */
    "Camera didn't move", /* GFMRV_CAMERA_DIDNT_MOVE */
    "Pointer outside of camera", /* GFMRV_CAMERA_POINTER_OUT_OF_SCREEN */
    // Tilemap errors
    "Tilemap not initialized", /* GFMRV_TILEMAP_NOT_INITIALIZED */
    "Tilemap no tiletype", /* GFMRV_TILEMAP_NO_TILETYPE */
    "Tilemap no tileanim", /* GFMRV_TILEMAP_NO_TILEANIM */
    "Didn't parse any tilemap", /* GFMRV_TILEMAP_NO_TILEMAP_PARSED */
    "Tiletype extant", /* GFMRV_TILETYPE_EXTANT */
    "Tileanim extant", /* GFMRV_TILEANIM_EXTANT */
    // Accumulator errors
    "Accumulator FPS too high", /* GFMRV_ACC_FPS_TOO_HIGH */
    "Accumulator not initialized", /* GFMRV_ACC_NOT_INITIALIZED */
    // FPS Counter errors
    "FPS counter not initialized", /* GFMRV_FPSCOUNTER_NOT_INITIALIZED */
    // Object errors
    "Object not initialized", /* GFMRV_OBJECT_NOT_INITIALIZED */
    "Negative drag", /* GFMRV_NEGATIVE_DRAG */
    "Objects can't collide", /* GFMRV_OBJECTS_CANT_COLLIDE */
    "Collision not triggered", /* GFMRV_COLLISION_NOT_TRIGGERED */
    // Sprite errors
    "Sprite not initialized", /* GFMRV_SPRITE_NOT_INITIALIZED */
    "No animation playing", /* GFMRV_NO_ANIMATION_PLAYING */
    // Animation errors
    "Animation not initialized", /* GFMRV_ANIMATION_NOT_INITIALIZED */
    "Animation already initialized", /* GFMRV_ANIMATION_ALREADY_INITIALIZED */
    "Animation already finished", /* GFMRV_ANIMATION_ALREADY_FINISHED */
    // GIF exporter errors
    "GIF operation active", /* GFMRV_GIF_OPERATION_ACTIVE */
    "GIF operation not active", /* GFMRV_GIF_OPERATION_NOT_ACTIVE */
    "GIF too many frames", /* GFMRV_GIF_TOO_MANY_FRAMES */
    "GIF not enough frames", /* GFMRV_GIF_NOT_ENOUGH_FRAMES */
    "GIF path not set", /* GFMRV_GIF_PATH_NOT_SET */
    "GIF not initialized", /* GFMRV_GIF_NOT_INITIALIZED */
    "GIF too many colors", /* GFMRV_GIF_TOO_MANY_COLORS */
    "GIF image too large", /* GFMRV_GIF_IMAGE_TOO_LARGE */
    "GIF image too tall", /* GFMRV_GIF_IMAGE_TOO_TALL */
    "GIF failed to compress", /* GFMRV_GIF_FAILED_TO_COMPRESS */
    "GIF thread is running", /* GFMRV_GIF_THREAD_IS_RUNNING */
    // Trie errors
    "Trie is leaf", /* GFMRV_TRIE_IS_LEAF */
    "Trie key not found", /* GFMRV_TRIE_KEY_NOT_FOUND */
    // Group errors
    "Group max sprites", /* GFMRV_GROUP_MAX_SPRITES */
    "Group no last sprite", /* GFMRV_GROUP_NO_LAST_SPRITE */
    "Group spriteset not set", /* GFMRV_GROUP_SPRITESET_NOT_SET */
    "Group width not set", /* GFMRV_GROUP_WIDTH_NOT_SET */
    "Group height not set", /* GFMRV_GROUP_HEIGHT_NOT_SET */
    "Group invalid type", /* GFMRV_GROUP_INVALID_TYPE */
    "Group list empty", /* GFMRV_GROUP_LIST_EMPTY */
    // Input errors
    "Input not initialized", /* GFMRV_INPUT_NOT_INITIALIZED */
    "Input invalid handle", /* GFMRV_INPUT_INVALID_HANDLE */
    "Input already bound", /* GFMRV_INPUT_ALREADY_BOUND */
    "Input not bound", /* GFMRV_INPUT_NOT_BOUND */
    // Text errors
    "Text not set", /* GFMRV_TEXT_NOT_SET */
    "Text no char", /* GFMRV_TEXT_NO_CHAR */
    "Text no more lines", /* GFMRV_TEXT_NO_MORE_LINES */
    // Quadtree errors
    "Quadtree not initialized", /* GFMRV_QUADTREE_NOT_INITIALIZED */
    "Quadtree operation not active", /* GFMRV_QUADTREE_OPERATION_NOT_ACTIVE */
    "Quadtree overlaped", /* GFMRV_QUADTREE_OVERLAPED */
    "Quadtree no overlap", /* GFMRV_QUADTREE_NO_OVERLAP */
    "Quadtree done", /* GFMRV_QUADTREE_DONE */
    "Quadtree empty", /* GFMRV_QUADTREE_EMPTY */
    "Quadtree stack overflow", /* GFMRV_QUADTREE_STACK_OVERFLOW */
    // Audio errors
    "Audio already initialized", /* GFMRV_AUDIO_ALREADY_INITIALIZED */
    "Audio not initialized", /* GFMRV_AUDIO_NOT_INITIALIZED */
    "Audio file not supported", /* GFMRV_AUDIO_FILE_NOT_SUPPORTED */
    "Invalid audio type", /* GFMRV_AUDIO_INVALID_TYPE */
    // File errors
    "File already open", /* GFMRV_FILE_ALREADY_OPEN */
    "File not open", /* GFMRV_FILE_NOT_OPEN */
    "File not found", /* GFMRV_FILE_NOT_FOUND */
    "File eof reached", /* GFMRV_FILE_EOF_REACHED */
    "File can't unread", /* GFMRV_FILE_CANT_UNREAD */
    "File max stack pos", /* GFMRV_FILE_MAX_STACK_POS */
    "File stack empty", /* GFMRV_FILE_STACK_EMPTY */
    "File write error", /* GFMRV_FILE_WRITE_ERROR */
    // Log errors,
    "Log invalid level", /* GFMRV_LOG_INVALID_LEVEL */
    "Log already initialized", /* GFMRV_LOG_ALREADY_INITIALIZED */
    "Log not initialized", /* GFMRV_LOG_NOT_INITIALIZED */
    "Log unknown token", /* GFMRV_LOG_UNKNOWN_TOKEN */
    // Parser errors
    "Parser not initialized", /* GFMRV_PARSER_NOT_INITIALIZED */
    "Parser already initialized", /* GFMRV_PARSER_ALREADY_INITIALIZED */
    "Error while parsing", /* GFMRV_PARSER_ERROR */
    "Got a invalid token while parsing", /* GFMRV_PARSER_BAD_TOKEN */
    "Parser no object parsed", /* GFMRV_PARSER_NO_OBJECT */
    "Parsed object doesn't have the requested parameter", /* GFMRV_PARSER_INVALID_OBJECT */
    "Finished parsing the file", /* GFMRV_PARSER_FINISHED */
    // Event errors
    "Failed to bind controller", /* GFMRV_CONTROLLER_FAILED_TO_BIND */
    "Controller with invalid ID found", /* GFMRV_CONTROLLER_INVALID_ID */
    // GL Errors
    "Failed to compile the fragment shader", /* GFMRV_FRAGMENT_SHADER_ERROR */
    "Failed to compile the vertex shader", /* GFMRV_VERTEX_SHADER_ERROR */
    "Max error" /* GFMRV_MAX */
};

