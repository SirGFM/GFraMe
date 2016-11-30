/**
 * @file include/GFraMe/gframe.h
 * 
 * The game's main context
 */
#ifndef __GFRAME_STRUCT__
#define __GFRAME_STRUCT__

/** 'Exports' the gfmCtx structure */
typedef struct stGFMCtx gfmCtx;

/** 'Exports' the backends enum */
enum enGFMVideoBackend {
    GFM_VIDEO_SDL2 = 0,
    GFM_VIDEO_GL3,
    GFM_VIDEO_GLES2,
    GFM_VIDEO_GLES3,
    GFM_VIDEO_WGL,
    GFM_VIDEO_SWSDL2,
    GFM_VIDEO_MAX
};
typedef enum enGFMVideoBackend gfmVideoBackend;

/** 'Exports' assets types */
enum enGFMAssetType {
    ASSET_TEXTURE = 0,
    ASSET_AUDIO
};
typedef enum enGFMAssetType gfmAssetType;

#endif /* __GFRAME_STRUCT__ */

#ifndef __GFMTEXTURE_STRUCT__
#define __GFMTEXTURE_STRUCT__
/** "Export" the texture structure's type */
typedef struct stGFMTexture gfmTexture;
#endif /* __GFMTEXTURE_STRUCT__ */

#ifndef __GFRAME_H_
#define __GFRAME_H_

#include <GFraMe/gfmCamera.h>
#include <GFraMe/gfmError.h>
#include <GFraMe/gfmInput.h>
#include <GFraMe/gfmLog.h>
#include <GFraMe/gfmSprite.h>
#include <GFraMe/gfmSpriteset.h>
#include <GFraMe/gfmString.h>
#include <GFraMe/core/gfmAudio_bkend.h>
#include <GFraMe/core/gfmEvent_bkend.h>

#define GFraMe_major_version	0
#define GFraMe_minor_version	1
#define GFraMe_fix_version		0
#define GFraMe_version	"0.1.0"
#define gfmVersion "v1.0.0"

/** 'Exportable' size of gfmCtx */
extern const int sizeofGFMCtx;

/**
 * Alloc a new gfmContext
 * 
 * @param  ppCtx The allocated context
 * @return       GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_ALLOC_FAILED
 */
gfmRV gfm_getNew(gfmCtx **ppCtx);

/**
 * Dealloc and clean up a gfmContext
 * 
 * @param  ppCtx The allocated context
 * @return       GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfm_free(gfmCtx **ppCtx);

/**
 * Select the video backend to be used; MUST be called before gfm_init
 * 
 * @param  pCtx  The allocated context
 * @param  bkend The backend
 * @return       GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_ALREADY_INITIALIZED
 */
gfmRV gfm_setVideoBackend(gfmCtx *pCtx, gfmVideoBackend bkend);

/**
 * Initialize and alloc every one of this object's members; Also sets the
 * game's title
 * 
 * @param  pCtx    The allocated context
 * @param  pOrg    Organization's name (used by log and save file)
 * @param  orgLen  Organization's name's length
 * @param  pName   Game's title (also used as window's title)
 * @param  nameLen Game's title's length
 * @return         GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfm_init(gfmCtx *pCtx, char *pOrg, int orgLen, char *pName, int nameLen);

/**
 * Initialize and alloc every one of this object's members; Also sets the
 * game's title from static buffers
 * 
 * @param  pCtx    The allocated context
 * @param  pOrg    Organization's name (used by log and save file)
 * @param  pName   Game's title (also used as window's title)
 * @return         GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
#define gfm_initStatic(pCtx, pOrg, pName) \
    gfm_init(pCtx, pOrg, sizeof(pOrg) - 1, pName, sizeof(pName) - 1)


/**
 * Get the binary's running path
 * 
 * @param  ppStr The running path as a gfmString
 * @param  pCtx  The game's context
 * @return       GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_FUNCTION_NOT_SUPPORTED
 */
gfmRV gfm_getBinaryPath(gfmString **ppStr, gfmCtx *pCtx);

/**
 * Set the game's title and organization from static buffers
 * 
 * @param  pCtx    The game's context
 * @param  org     Statically allocated organization's name
 * @param  name    Statically allocated game's title (i.e., char var[] = "...")
 * @return         GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_TITLE_ALREADY_SET,
 *                 GFMRV_ALLOC_FAILED
 */
#define gfm_setTitleStatic(pCtx, pOrg, pName) \
    gfm_setTitle(pCtx, pOrg, sizeof(pOrg)-1, pName, sizeof(pName)-1)

/**
 * Set the game's title and organization
 * 
 * @param  pCtx     The game's context
 * @param  pOrg     Organization's name (used by log and save file)
 * @param  orgLen   Organization's name's length
 * @param  pName    Game's title (also used as window's title)
 * @param  nameLen  Game's title's length
 * @return          GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_TITLE_ALREADY_SET,
 *                  GFMRV_ALLOC_FAILED
 */
gfmRV gfm_setTitle(gfmCtx *pCtx, char *pOrg, int orgLen, char *pName,
        int nameLen);

/**
 * Get the game's title and organization
 * 
 * @param  ppOrg      Organization's name (used by log and save file)
 * @param  ppTitle    Game's title (also used as window's title)
 * @param  pCtx       The game's context
 * @return            GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_TITLE_NOT_SET
 */
gfmRV gfm_getTitle(char **ppOrg, char **ppTitle, gfmCtx *pCtx);

/**
 * Get the game's local path
 * 
 * @param  ppLocalPath Local path (a new gfmString is alloc'ed!)
 * @param  pCtx        The game's context
 * @return             GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfm_getLocalPath(gfmString **ppLocalPath, gfmCtx *pCtx);

/**
 * Query the resolutions and add them to a internal buffer
 * 
 * @param  pCount How many resolutions were found
 * @param  pCtx   The game's context
 * @return        GFMRV_OK, ...
 */
gfmRV gfm_queryResolutions(int *pCount, gfmCtx *pCtx);

/**
 * Get a resolution; if gfmWindow_queryResolutions wasn't previously called, it
 * will be automatically called
 * 
 * @param  pWidth   A possible window's width
 * @param  pHeight  A possible window's height
 * @param  pRefRate A possible window's refresh rate
 * @param  pCtx     The game's context
 * @param  index    Resolution to be read (0 is the default resolution)
 * @return          GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_INTERNAL_ERROR,
 *                  GFMRV_ALLOC_FAILED, GFMRV_INVALID_INDEX
 */
gfmRV gfm_getResolution(int *pWidth, int *pHeight, int *pRefRate,
        gfmCtx *pCtx, int index);

/**
 * Initialize the game's window and backbuffer
 * 
 * *NOTE*: The game window may be later resized, but not the backbuffer!
 * 
 * @param  pCtx            The game's context
 * @param  bufWidth        Backbuffer's width
 * @param  bufHeight       Backbuffer's height
 * @param  wndWidth        Window's width
 * @param  wndHeight       Window's height
 * @param  isUserResizable Whether the user can resize the window through the OS
 * @param  useVsync        Whether vsync should be enabled or not
 * @return                 GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_TITLE_NOT_SET,
 *                         GFMRV_INVALID_WIDTH, GFMRV_INVALID_HEIGHT
 */
gfmRV gfm_initGameWindow(gfmCtx *pCtx, int bufWidth, int bufHeight,
        int wndWidth, int wndHeight, int isUserResizable, int useVsync);

/**
 * Initialize the game's window (in fullscreen) and backbuffer
 * 
 * *NOTE*: The game window may be later resized, but not the backbuffer!
 * 
 * @param  pCtx            The game's context
 * @param  bufWidth        Backbuffer's width
 * @param  bufHeight       Backbuffer's height
 * @param  resIndex        Resolution to be used (0 is the default resolution)
 * @param  isUserResizable Whether the user can resize the window through the OS
 * @param  useVsync        Whether vsync should be enabled or not
 * @return                 GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_TITLE_NOT_SET,
 *                         GFMRV_INVALID_WIDTH, GFMRV_INVALID_HEIGHT
 */
gfmRV gfm_initGameFullScreen(gfmCtx *pCtx, int bufWidth, int bufHeight,
        int resIndex, int isUserResizable, int useVsync);

/**
 * Disable the audio subsystem; Any further call to any audio function will be
 * ignored
 * NOTE: It must be called before gfm_initAudio!
 * 
 * @param  pCtx The game's context
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_NOT_INITIALIZED,
 *              GFMRV_AUDIO_ALREADY_INITIALIZED
 */
gfmRV gfm_disableAudio(gfmCtx *pCtx);

/**
 * Initialize the audio sub-system; This function must be called before loading
 * any song
 * 
 * @param  pCtx     The game's context
 * @param  settings Audio quality (sampling rate, etc)
 * @return          GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_ALLOC_FAILED
 */
gfmRV gfm_initAudio(gfmCtx *pCtx, gfmAudioQuality settings);

/**
 * Set the game's fps resolution; This defines when will the game automatically
 * wake to update its timers and check if a new frame should be issued
 * (therefore, it's somewhat different from the stateFramerate)
 * 
 * This can be used to ease the game's resource (CPU) consuption, when focus is
 * lost
 * 
 * NOTE: This function will round the time to its nearest multiple of ten
 * 
 * @param  pCtx The game's context
 * @param  fps  The game's fps
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_ALLOC_FAILED,
 *              GFMRV_INTERNAL_ERROR, GFMRV_FPS_TOO_HIGH
 */
gfmRV gfm_setFPS(gfmCtx *pCtx, int fps);

/**
 * Set the game's fps resolution; This defines when will the game automatically
 * wake to update its timers and check if a new frame should be issued
 * (therefore, it's somewhat different from the stateFramerate)
 * 
 * This can be used to ease the game's resource (CPU) consuption, when focus is
 * lost
 * 
 * @param  pCtx The game's context
 * @param  fps  The game's fps
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_ALLOC_FAILED,
 *              GFMRV_INTERNAL_ERROR, GFMRV_FPS_TOO_HIGH
 */
gfmRV gfm_setRawFPS(gfmCtx *pCtx, int fps);

/**
 * Signal the game's context that it should quit
 * 
 * @param  pCtx The game's context
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfm_setQuitFlag(gfmCtx *pCtx);

/**
 * Check whether the quit flag was received or not
 * 
 * @param  pCtx The game's context
 * @return      GFMRV_ARGUMENTS_BAD, GFMRV_FALSE, GFMRV_TRUE
 */
gfmRV gfm_didGetQuitFlag(gfmCtx *pCtx);

/**
 * Get the event context
 * 
 * @param ppEvent The event context
 * @param pCtx    The game's context
 * @return        GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfm_getEventCtx(gfmEvent **ppEvent, gfmCtx *pCtx);

/**
 * Convert a point in window-space to backbuffer-space
 * 
 * NOTE: Both pX and pY must be initialized with the window-space point
 * 
 * @param  [out]pX   The horizontal position, in backbuffer-space
 * @param  [out]pY   The vertical position, in backbuffer-space
 * @param  [ in]pCtx The game's context
 * @return           GFMRV_OK, GFMRV_ARGUMENTS_BAD,
 *                   GFMRV_BACKBUFFER_NOT_INITIALIZED
 */
gfmRV gfm_windowToBackbuffer(int *pX, int *pY, gfmCtx *pCtx);

/**
 * Get the backbuffer's dimension
 * 
 * @param  pWidth  The backbuffer's width
 * @param  pHeigth The backbuffer's height
 * @param  pCtx    The game's contex
 * @return         GFMRV_OK, GFMRV_ARGUMENTS_BAD,
 *                 GFMRV_BACKBUFFER_NOT_INITIALIZED
 */
gfmRV gfm_getBackbufferDimensions(int *pWidth, int *pHeight, gfmCtx *pCtx);

/**
 * Resize the window to the desired dimensions
 * 
 * @param  pCtx   The window context
 * @param  width  The desired width
 * @param  height The desired height
 * @return        GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_INTERNAL_ERROR,
 *                GFMRV_WINDOW_NOT_INITIALIZED, GFMRV_WINDOW_IS_FULLSCREEN
 */
gfmRV gfm_setDimensions(gfmCtx *pCtx, int width, int height);

/**
 * Make the game go full-screen
 * 
 * @param  pCtx   The game's context
 * @return        GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_INTERNAL_ERROR,
 *                GFMRV_WINDOW_NOT_INITIALIZED, GFMRV_WINDOW_MODE_UNCHANGED
 */
gfmRV gfm_setFullscreen(gfmCtx *pCtx);

/**
 * Make the game go windowed;
 * The window's dimensions will be kept!
 * 
 * @param  pCtx   The game's context
 * @return        GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_INTERNAL_ERROR,
 *                GFMRV_WINDOW_NOT_INITIALIZED, GFMRV_WINDOW_MODE_UNCHANGED
 */
gfmRV gfm_setWindowed(gfmCtx *pCtx);

/**
 * Set the window's resolution;
 * If the window is in full screen mode, it's resolution and refresh rate will
 * be modified; Otherwise, only it's dimension's will be modified
 * 
 * @param  pCtx  The game's context
 * @param  index Resolution to be used (0 is the default resolution)
 * @return       GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_INTERNAL_ERROR,
 *               GFMRV_INVALID_INDEX
 */
gfmRV gfm_setResolution(gfmCtx *pCtx, int resIndex);

/**
 * Set the background color
 * 
 * @param  pCtx  The game's context
 * @param  color The background color (in ARGB, 32 bits, format)
 * @return       GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfm_setBackground(gfmCtx *pCtx, int color);

/**
 * Create and load a texture; the lib will keep track of it and release its
 * memory, on exit
 * 
 * @param  pIndex      The texture's index
 * @param  pCtx        The game's contex
 * @param  pFilename   The image's filename in a static buf (must be a '.bmp')
 * @param  colorKey    Color to be treat as transparent (in RGB, 24 bits)
 * @return             GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_TEXTURE_NOT_BITMAP,
 *                     GFMRV_TEXTURE_FILE_NOT_FOUND,
 *                     GFMRV_TEXTURE_INVALID_WIDTH,
 *                     GFMRV_TEXTURE_INVALID_HEIGHT, GFMRV_ALLOC_FAILED,
 *                     GFMRV_INTERNAL_ERROR
 */
#define gfm_loadTextureStatic(pIndex, pCtx, pFilename, colorKey) \
    gfm_loadTexture(pIndex, pCtx, pFilename, sizeof(pFilename)-1, colorKey)

/**
 * Create and load a texture; the lib will keep track of it and release its
 * memory, on exit
 *
 * NOTE: This function is still dumb and forces the keycolor to 0xff00ff
 *       (magenta)
 * 
 * @param  pIndex      The texture's index
 * @param  pCtx        The game's contex
 * @param  pFilename   The image's filename (must be a '.bmp')
 * @param  filenameLen The filename's length
 * @param  colorKey    Color to be treat as transparent (in RGB, 24 bits)
 * @return             GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_TEXTURE_NOT_BITMAP,
 *                     GFMRV_TEXTURE_FILE_NOT_FOUND,
 *                     GFMRV_TEXTURE_INVALID_WIDTH,
 *                     GFMRV_TEXTURE_INVALID_HEIGHT, GFMRV_ALLOC_FAILED,
 *                     GFMRV_INTERNAL_ERROR
 */
gfmRV gfm_loadTexture(int *pIndex, gfmCtx *pCtx, char *pFilename,
        int filenameLen, int colorKey);

/**
 * Get a texture
 * 
 * @param  ppTex The texture
 * @param  pCtx  The game's context
 * @param  index The texture's index
 * @return       GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_INVALID_INDEX
 */
gfmRV gfm_getTexture(gfmTexture **ppTex, gfmCtx *pCtx, int index);

/**
 * Get a texture's dimesions
 * 
 * @param  [out]pWidth  The texture's width
 * @param  [out]pHeight The texture's height
 * @param  [ in]pCtx    The game's context
 * @param  [ in]pTex    The texture
 * @return             GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfm_getTextureDimensions(int *pWidth, int *pHeight, gfmCtx *pCtx,
        gfmTexture *pTex);

/**
 * Create a new (automatically managed) spriteset
 * 
 * @param  ppSset     The spriteset
 * @param  pCtx       The game's context
 * @param  index      The texture's index
 * @param  tileWidth  The width of each tile
 * @param  tileHeight The height of each tile
 * @return            GFMRV_OK, GFMRV_ARGUMENTS_BAD,
 *                    GFMRV_SPRITESET_INVALID_WIDTH,
 *                    GFMRV_SPRITESET_INVALID_HEIGHT,
 *                    GFMRV_TEXTURE_NOT_INITIALIZED
 */
gfmRV gfm_createSpritesetCached(gfmSpriteset **ppSset, gfmCtx *pCtx, int index,
        int tileWidth, int tileHeight);

/**
 * OBSOLETE FUCTION!!
 * 
 * Set a texture as default; this texture will always be loaded before drawing
 * anything
 * 
 * @param  pCtx  The game's context
 * @param  index The texture index
 * @return       GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_INVALID_INDEX
 */
gfmRV gfm_setDefaultTexture(gfmCtx *pCtx, int index);

/**
 * Loads an audio
 * 
 * @param  pHandle     Handle of the loaded audio
 * @param  pCtx        The game's context
 * @param  pFilename   The filename
 * @param  filenameLen Length of the filename
 * @return             GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_COULDNT_OPEN_FILE,
 *                     GFMRV_READ_ERROR, GFMRV_AUDIO_FILE_NOT_SUPPORTED,
 *                     GFMRV_ALLOC_FAILED, GFMRV_INTERNAL_ERROR
 */
gfmRV gfm_loadAudio(int *pHandle, gfmCtx *pCtx, char *pFilename, int filenameLen);

/**
 * Set to which sample the song must loop
 * 
 * @param  pCtx   The game's context
 * @param  handle The handle of the looped audio
 * @param  pos    Sample to which the song should go back when looping
 * @return        GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_INVALID_INDEX,
 *                GFMRV_INVALID_BUFFER_LEN
 */
gfmRV gfm_setRepeat(gfmCtx *pCtx, int handle, int pos);

/**
 * Plays an audio and returns its instance
 * 
 * @param  ppHnd  The audio instance (may be NULL, if one simply doesn't care)
 * @param  pCtx The game's context
 * @param  handle The handle of the audio to be played
 * @param  volume How loud should the audio be played (in the range (0.0, 1.0])
 * @return        GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_INVALID_INDEX,
 *                GFMRV_AUDIO_NOT_INITIALIZED, GFMRV_ALLOC_FAILED, 
 */
gfmRV gfm_playAudio(gfmAudioHandle **ppHnd, gfmCtx *pCtx, int handle,
        double volume);

/**
 * Stops a currently playing audio
 *
 * @param  [ in]pHnd The audio instance
 * @param  [ in]pCtx The game's context
 * @return           GFMRV_OK, ...
 */
gfmRV gfm_stopAudio(gfmAudioHandle *pHnd, gfmCtx *pCtx);

/**
 * Queue an audio. If the audio system is paused, this function won't forcefully
 * start it (in contrast to gfm_playAudio)
 *
 * @param  ppHnd  The audio instance (may be NULL, if one simply doesn't care)
 * @param  pCtx The game's context
 * @param  handle The handle of the audio to be played
 * @param  volume How loud should the audio be played (in the range (0.0, 1.0])
 * @return        GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_INVALID_INDEX,
 *                GFMRV_AUDIO_NOT_INITIALIZED, GFMRV_ALLOC_FAILED, 
 */
gfmRV gfm_queueAudio(gfmAudioHandle **ppHnd, gfmCtx *pCtx, int handle,
        double volume);

/**
 * Pause any playing audio. It will restart as soon as any audio is played or
 * gfm_resumeAudio is called
 *
 * NOTE: Queueing an audio won't restart the audio system!
 *
 * @param  [ in]pCtx The game's context
 * @return        GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_AUDIO_NOT_INITIALIZED
 */
gfmRV gfm_pauseAudio(gfmCtx *pCtx);

/**
 * Resume playing audios. If there are no audios currently playing, nothing will
 * happen
 *
 * @param  [ in]pCtx The game's context
 * @return        GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_AUDIO_NOT_INITIALIZED
 */
gfmRV gfm_resumeAudio(gfmCtx *pCtx);

/**
 * Load assets in a separated thread
 *
 * @param  [out]pProgress Updated with how many assets have been loaded
 * @param  [ in]pCtx      The lib's main context
 * @param  [ in]pType     List of assets types to be loaded
 * @param  [ in]ppPath    List of paths to the assets
 * @param  [ in]ppHandles List of pointers where the loaded handles shall be
 *                        stored
 * @param  [ in]numAssets How many assets are there to be loaded
 * @return                GFraMe return value
 */
gfmRV gfm_loadAssetsAsync(int *pProgress, gfmCtx *pCtx, gfmAssetType *pType,
        char **ppPath, int **ppHandles, int numAssets);

/**
 * Retrieve the current camera
 * 
 * @param  ppCam The camera
 * @param  pCtx  The game's context
 * @return       GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_CAMERA_NOT_INITIALIZED
 */
gfmRV gfm_getCamera(gfmCamera **ppCam, gfmCtx *pCtx);

/**
 * Get the default camera's current position
 * 
 * @param  pX    The horizontal position
 * @param  pY    The vertical position
 * @param  pCtx  The game's context
 * @return       GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_CAMERA_NOT_INITIALIZED
 */
gfmRV gfm_getCameraPosition(int *pX, int *pY, gfmCtx *pCtx);

/**
 * Get the default camera's current position
 * 
 * @param  pWidth  The camera's width
 * @param  pHeight The camera's height
 * @param  pCtx    The game's context
 * @return         GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_CAMERA_NOT_INITIALIZED
 */
gfmRV gfm_getCameraDimensions(int *pWidth, int *pHeight, gfmCtx *pCtx);

/**
 * Check if a sprite is inside the camera
 * 
 * @param  pCtx The game's context
 * @param  pSpr The sprite
 * @return      GFMRV_TRUE, GFMRV_FALSE, GFMRV_ARGUMENTS_BAD,
 *              GFMRV_CAMERA_NOT_INITIALIZED
 */
gfmRV gfm_isSpriteInsideCamera(gfmCtx *pCtx, gfmSprite *pSpr);

/**
 * Set the state's framerate
 * 
 * @param  pCtx The game's context
 * @param  ups  Number of updates per seconds
 * @param  dps  Number of draws per seconds
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_ACC_FPS_TOO_HIGH
 */
gfmRV gfm_setStateFrameRate(gfmCtx *pCtx, int ups, int dps);

/**
 * Set the state's framerate
 * 
 * @param  pUps  Number of updates per seconds
 * @param  pDps  Number of draws per seconds
 * @param  pCtx  The game's context
 * @return       GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_ACC_NOT_INITIALIZED
 */
gfmRV gfm_getStateFrameRate(int *pUps, int *pDps, gfmCtx *pCtx);

/**
 * Check if there are any frames left and updates the inputs
 * 
 * @param  pCtx The game's context
 * @return      GFMRV_ARGUMENTS_BAD, GFMRV_FALSE, GFMRV_TRUE
 */
gfmRV gfm_isUpdating(gfmCtx *pCtx);

/**
 * Check if there are any frames left to be drawn
 * 
 * @param  pCtx The game's context
 * @return      GFMRV_ARGUMENTS_BAD, GFMRV_FALSE, GFMRV_TRUE
 */
gfmRV gfm_isDrawing(gfmCtx *pCtx);

/**
 * Get how many time elapsed on each frame, in milliseconds; If static time loop
 * is used, this number will always be the same, for variable time loop, this
 * time will be the mean of how many frames were elapsed
 * 
 * NOTE: Only static time loop is implemented, as of now!
 * 
 * @param  pElapsed The elapsed time, in milliseconds
 * @param  pCtx     The game's context
 * @return          GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_ACC_NOT_INITIALIZED
 */
gfmRV gfm_getElapsedTime(int *pElapsed, gfmCtx *pCtx);

/**
 * Get how many time elapsed on each frame, in seconds; If static time loop is
 * used, this number will always be the same, for variable time loop, this time
 * will be the mean of how many frames were elapsed
 * 
 * NOTE: Only static time loop is implemented, as of now!
 * 
 * @param  pElapsed The elapsed time, in seconds
 * @param  pCtx     The game's context
 * @return          GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_ACC_NOT_INITIALIZED
 */
gfmRV gfm_getElapsedTimef(float *pElapsed, gfmCtx *pCtx);

/**
 * Get how many time elapsed on each frame, in seconds; If static time loop is
 * used, this number will always be the same, for variable time loop, this time
 * will be the mean of how many frames were elapsed
 * 
 * NOTE: Only static time loop is implemented, as of now!
 * 
 * @param  pElapsed The elapsed time, in seconds
 * @param  pCtx     The game's context
 * @return          GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_ACC_NOT_INITIALIZED
 */
gfmRV gfm_getElapsedTimed(double *pElapsed, gfmCtx *pCtx);

/**
 * Update both accumulators
 * 
 * @param  pCtx The game's context
 * @param  ms   Time elapsed (in milliseconds)
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_ACC_NOT_INITIALIZED
 */
gfmRV gfm_updateAccumulators(gfmCtx *pCtx, int ms);

/**
 * Sleep until any event is received and handle everything
 * 
 * @param  pCtx The game's context
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfm_handleEvents(gfmCtx *pCtx);

/**
 * Initialize the FPS counter; On the release version, this function does
 * nothing but returns GFMRV_OK
 *
 * The FPS counter uses a internal bitmap font, which is only added on debug
 * mode. Therefore, it's disable on release mode and both pSset and firstTile
 * are ignored.
 * 
 * @param  pCtx      The game's context
 * @param  pSset     The spriteset
 * @param  firstTile The first ASCII character's tile ('!') on the spriteset
 * @return           GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfm_initFPSCounter(gfmCtx *pCtx, gfmSpriteset *pSset, int firstTile);

/**
 * Set the position where the FPS counter is to be rendered
 *
 * @param  [ in]pCtx The game's context
 * @param  [ in]x    The horizontal position
 * @param  [ in]y    The vertical position
 * @return           GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfm_setFPSCounterPos(gfmCtx *pCtx, int x, int y);

/**
 * Make the FPS counter visible
 * 
 * @param  pCtx      The game's context
 * @return           GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfm_showFPSCounter(gfmCtx *pCtx);

/**
 * Hide the FPS counter
 * 
 * @param  pCtx      The game's context
 * @return           GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfm_hideFPSCounter(gfmCtx *pCtx);

/**
 * Signal the counter that an update will happen; On the release version, this
 * function does nothing but returns GFMRV_OK
 * 
 * @param  pCtx      The game's context
 * @return           GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfm_fpsCounterUpdateBegin(gfmCtx *pCtx);

/**
 * Signal the counter that an update happened; On the release version, this
 * function does nothing but returns GFMRV_OK
 * 
 * @param  pCtx      The game's context
 * @return           GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfm_fpsCounterUpdateEnd(gfmCtx *pCtx);

/**
 * Adds a new virtual key to the game's context
 * 
 * @param  pHandle Handle to the action
 * @param  pCtx    The game's context
 * @return         GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_ALLOC_FAILED
 */
gfmRV gfm_addVirtualKey(int *pHandle, gfmCtx *pCtx);

/**
 * Bind a key to an action
 * 
 * @param  pCtx   The game's context
 * @param  handle The action's handle
 * @param  key    The key
 * @return        GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_INPUT_INVALID_HANDLE,
 *                GFMRV_INPUT_ALREADY_BOUND
 */
gfmRV gfm_bindInput(gfmCtx *pCtx, int handle, gfmInputIface key);

/**
 * Bind a gamepad's button to an action
 * 
 * @param  pCtx   The game's context
 * @param  handle The action's handle
 * @param  button The button
 * @param  port   Port (i.e., index) of the gamepad
 * @return        GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_INPUT_INVALID_HANDLE,
 *                GFMRV_INPUT_ALREADY_BOUND
 */
gfmRV gfm_bindGamepadInput(gfmCtx *pCtx, int handle, gfmInputIface button,
        int port);

/**
 * Reset all bindings from the input
 * 
 * @param  pCtx   The game's context
 * @return        GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfm_resetInput(gfmCtx *pCtx);

/**
 * Retrieves a virtual key state
 * 
 * @param  pState The current state
 * @param  pNum   How many consecutive times the key has been pressed
 * @param  pCtx   The game's context
 * @param  handle The action's handle
 * @param        GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_INPUT_INVALID_HANDLE
 */
gfmRV gfm_getKeyState(gfmInputState *pState, int *pNum, gfmCtx *pCtx,
        int handle);

/**
 * Get the port of the last pressed button; If the last input didn't come from
 * a gamepad, the port will be -1
 * NOTE: This function must be called before getLastPressed!!!
 * 
 * @param  pPort The port
 * @param  pCtx   The game's context
 * @return       GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_OPERATION_NOT_ACTIVE,
 *               GFMRV_WAITING
 */
gfmRV gfm_getLastPort(int *pPort, gfmCtx *pCtx);

/**
 * Get the last key/button/whatever pressed; This function doesn't block but,
 * unless it's ready, it will return GFMRV_WAITING; The 'key' pressed is only
 * valid when the function return GFMRV_OK
 * 
 * @param  pIface The last 'iface' pressed
 * @param  pCtx   The game's context
 * @return        GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_WAITING
 */
gfmRV gfm_getLastPressed(gfmInputIface *pIface, gfmCtx *pCtx);

/**
 * Retrieve the current input context
 * 
 * @param  ppInput The input context
 * @param  pCtx    The game's context
 * @return         GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfm_getInput(gfmInput **ppInput, gfmCtx *pCtx);

/**
 * Get the logger instance, so we can log elsewhere
 * 
 * @param  ppLog The logger
 * @param  pCtx  The game's context
 * @return       GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_NOT_INITIALIZED
 */
gfmRV gfm_getLogger(gfmLog **ppLog, gfmCtx *pCtx);

/**
 * Takes a snapshot as soon as the frame finishes rendering and saves it as a
 * GIF image; If this function is called more than once in a frame, it will
 * ignore the second call and save according to the first call
 * 
 * @param  pCtx         The game's context
 * @param  pFilepath    Path (and filename) where it will be saved (depends on
 *                      useLocalPath); The extension isn't required, but, if
 *                      present, must be .gif!
 * @param  len          Filename's length
 * @param  useLocalPath Whether the path should be appended to the local path
 *                      (e.g., %APPDATA%\concat(organization, title)\, on
 *                      windows); or "as-is" (relative or absolute, depending on
 *                      the actual path)
 * @return              GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_OPERATION_ACTIVE,
 *                      GFMRV_ALLOC_FAILED, ...
 */
gfmRV gfm_snapshot(gfmCtx *pCtx, char *pFilepath, int len, int useLocalPath);

/**
 * Takes a snapshot as soon as the frame finishes rendering and saves it as a
 * GIF image; If this function is called more than once in a frame, it will
 * ignore the second call and save according to the first call
 * 
 * @param  pCtx         The game's context
 * @param  pFilepath    Path (and filename) where it will be saved (depends on
 *                      useLocalPath); The extension isn't required, but, if
 *                      present, must be .gif!
 * @param  useLocalPath Whether the path should be appended to the local path
 *                      (e.g., %APPDATA%\concat(organization, title)\, on
 *                      windows); or "as-is" (relative or absolute, depending on
 *                      the actual path)
 * @return              GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_OPERATION_ACTIVE,
 *                      GFMRV_ALLOC_FAILED, ...
 */
#define gfm_snapshotStatic(pCtx, pFilepath, useLocalPath) \
    gfm_snapshot(pCtx, pFilepath, sizeof(pFilepath)-1, useLocalPath)

/**
 * Record a few milliseconds as a animated GIF
 * 
 * @param  pCtx         The game's context
 * @param  ms           How long should be recorded, in milliseconds
 * @param  pFilepath    Path (and filename) where it will be saved (depends on
 *                      useLocalPath); The extension isn't required, but, if
 *                      present, must be .gif!
 * @param  len          Filename's length
 * @param  useLocalPath Whether the path should be appended to the local path
 *                      (e.g., %APPDATA%\concat(organization, title)\, on
 *                      windows); or "as-is" (relative or absolute, depending on
 *                      the actual path)
 * @return              GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_OPERATION_ACTIVE,
 *                      GFMRV_ALLOC_FAILED, ...
 */
gfmRV gfm_recordGif(gfmCtx *pCtx, int ms, char *pFilepath, int len,
        int useLocalPath);

/**
 * Whether a previous 'gfm_recordGif' has finished; Must be called before
 * recording another gif
 * 
 * @param  pCtx The game's context
 * @return      GFMRV_TRUE, GFMRV_FALSE, GFMRV_GIF_OPERATION_NOT_ACTIVE,
 *              GFMRV_ARGUMENTS_BAD
 */
gfmRV gfm_didExportGif(gfmCtx *pCtx);

/**
 * Initialize a rendering operation
 * 
 * @param  pCtx  The game's context
 * @return       GFMRV_OK, GFMRV_ARGUMENTS_BAD, ...
 */
gfmRV gfm_drawBegin(gfmCtx *pCtx);

/**
 * OBSOLETE FUNCTION!!
 * 
 * Loads a texture into the backbuffer; The texture must be managed by the
 * framework
 * 
 * @param  pCtx  The game's context
 * @param  iTex  Texture index (the value returned when created)
 * @return       GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_INVALID_INDEX
 */
gfmRV gfm_drawLoadCachedTexture(gfmCtx *pCtx, int iTex);

/**
 * OBSOLETE FUNCTION!!
 * 
 * Loads a texture into the backbuffer
 * 
 * @param  pCtx  The game's context
 * @param  pTex  The texture
 * @return       GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_TEXTURE_NOT_INITIALIZED
 */
gfmRV gfm_drawLoadTexture(gfmCtx *pCtx, gfmTexture *pTex);

/**
 * Renders a tile into the backbuffer
 * 
 * @param  pCtx      The game's context
 * @param  pSSet     The spriteset containing the tile
 * @param  x         Horizontal position in screen space
 * @param  y         Vertical position in screen space
 * @param  tile      Tile to be rendered
 * @param  isFlipped Whether the tile should be drawn flipped
 * @return           GFMRV_OK, GFMRV_ARGUMENTS_BAD, ...
 */
gfmRV gfm_drawTile(gfmCtx *pCtx, gfmSpriteset *pSset, int x, int y, int tile,
        int isFlipped);

/**
 * Renders a number at the desired position; The spriteset's texture must have
 * a bitmap font (in the ASCII sequence)
 * 
 * @param  pCtx      The game's context
 * @param  pSSet     The spriteset containing the tile
 * @param  x         Horizontal position in screen space
 * @param  y         Vertical position in screen space
 * @param  num       Number to be rendered
 * @param  res       Number of digits
 * @param  firstTile First ASCII tile in the spriteset
 * @return           GFMRV_OK, GFMRV_ARGUMENTS_BAD,
 *                   GFMRV_BACKBUFFER_NOT_INITIALIZED,
 */
gfmRV gfm_drawNumber(gfmCtx *pCtx, gfmSpriteset *pSset, int x, int y, int num,
        int res, int firstTile);

/**
 * Renders a sprite into the backbuffer
 * 
 * @param  pCtx  The game's context
 * @param  pSpr  The sprite
 * @return       GFMRV_OK, GFMRV_ARGUMENTS_BAD, ...
 */
gfmRV gfm_drawSprite(gfmCtx *pCtx, gfmSprite *pSpr);

/**
 * Renders a rectangle (only its vertices);
 * NOTE: This function isn't guaranteed to be fast, so use it wisely
 * 
 * @param  pCtx   The game's context
 * @param  x      Top-left position, in world-space
 * @param  y      Top-left position, in world-space
 * @param  width  Rectangle's width
 * @param  height Rectangle's height
 * @param  red    Color's red component
 * @param  green  Color's green component
 * @param  blue   Color's blue component
 * @return        GFMRV_OK, GFMRV_ARGUMENTS_BAD,
 *                GFMRV_BACKBUFFER_NOT_INITIALIZED
 */
gfmRV gfm_drawRect(gfmCtx *pCtx, int x, int y, int width, int height,
        unsigned char red, unsigned char green, unsigned char blue);

/**
 * Render last frame's render info
 * 
 * The displayed info is the number of batched draws and the number of drawn
 * sprites
 *
 * This function uses an internal bitmap font, only available on debug mode.
 * Therefore, it's disable on release mode and both pSset and firstTile are
 * ignored.
 * 
 * @param  [ in]pCtx      The game's conext
 * @param  [ in]pSset     The spriteset
 * @param  [ in]x         Horizontal position
 * @param  [ in]y         Vertical position
 * @param  [ in]firstTile First ASCII tile in the spriteset
 * @return                GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_NOT_INITIALIZED,
 *                        GFMRV_BACKBUFFER_NOT_INITIALIZED
 */
gfmRV gfm_drawRenderInfo(gfmCtx *pCtx, gfmSpriteset *pSset, int x, int y,
        int firstTile);

/**
 * Finalize a rendering operation
 * 
 * @param  pCtx  The game's context
 * @return       GFMRV_OK, GFMRV_ARGUMENTS_BAD, ...
 */
gfmRV gfm_drawEnd(gfmCtx *pCtx);

/**
 * Issue a new frame; Should only be used on singled threaded environments
 * 
 * @param  pCtx  The game's context
 * @return       GFMRV_OK, GFMRV_ARGUMENTS_BAD, ...
 */
gfmRV gfm_issueFrame(gfmCtx *pCtx);

/**
 * Wait for a new frame; Should only be used on singled threaded environments
 * 
 * @param  pCtx  The game's context
 * @return       GFMRV_OK, GFMRV_ARGUMENTS_BAD, ...
 */
gfmRV gfm_waitFrame(gfmCtx *pCtx);

/**
 * Reset the FPS accumulators
 *
 * This function should be called after sections that may lag (and therefore,
 * messes with accumulated frames and whatnot).
 * One example is before switching from a menu to a game state, after loading
 * assets in background.
 *
 * @param  [ in]pCtx The game's context
 * @reutnr           GFraMe return value
 */
gfmRV gfm_resetFPS(gfmCtx *pCtx);

/**
 * Clean up a context
 * 
 * @param  pCtx The context
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfm_clean(gfmCtx *pCtx);

#endif /* __GFRAME_H_ */

