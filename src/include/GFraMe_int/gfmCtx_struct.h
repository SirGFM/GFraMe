/**
 * @file src/include/GFraMe_int/gfmCtx_struct.h
 *
 * Define the main context
 */
#ifndef __GFRAME_INT_GFMCTX_STRUCT_H__
#define __GFRAME_INT_GFMCTX_STRUCT_H__

#include <GFraMe/gframe.h>
#include <GFraMe/gfmAccumulator.h>
#include <GFraMe/gfmAssert.h>
#include <GFraMe/gfmCamera.h>
#include <GFraMe/gfmError.h>
#include <GFraMe/gfmGenericArray.h>
#include <GFraMe/gfmInput.h>
#include <GFraMe/gfmLog.h>
#include <GFraMe/gfmSprite.h>
#include <GFraMe/gfmSpriteset.h>
#include <GFraMe/gfmString.h>
#include <GFraMe/core/gfmAudio_bkend.h>
#include <GFraMe/core/gfmBackend_bkend.h>
#include <GFraMe/core/gfmEvent_bkend.h>
#include <GFraMe/core/gfmGifExporter_bkend.h>
#include <GFraMe/core/gfmPath_bkend.h>
#include <GFraMe/core/gfmTimer_bkend.h>

#include <GFraMe_int/gfmFPSCounter.h>
#include <GFraMe_int/gfmVideo_bmp.h>
#include <GFraMe_int/core/gfmVideo_bkend.h>
#include <GFraMe_int/core/gfmLoadAsync_bkend.h>

/** Define a texture array type */
gfmGenArr_define(gfmTexture);
/** Define a spriteset array type */
gfmGenArr_define(gfmSpriteset);

struct stGFMCtx {
    // TODO specify backend(?)
    /** "Organization" name; It's used as part of paths. */
    gfmString *pGameOrg;
    /** Game's title; It's used as part of paths. */
    gfmString *pGameTitle;
#ifndef GFRAME_MOBILE
    /** Directory where the game binary is being run from */
    gfmString *pBinPath;
    /** Length until the current directory (i.e., position to append stuff) */
    int binPathLen;
#endif
    /** Audio sub-system context */
    gfmAudioCtx *pAudio;
    /* The video context */
    gfmVideo *pVideo;
    /** Current video functions */
    gfmVideoFuncs videoFuncs;
    /** Default camera */
    gfmCamera *pCamera;
    /** Accumulate when new update frames should be issued */
    gfmAccumulator *pUpdateAcc;
    /** Accumulate when new draw frames should be issued */
    gfmAccumulator *pDrawAcc;
    /** Event context */
    gfmEvent *pEvent;
    /** Input context */
    gfmInput *pInput;
    /** The logger */
    gfmLog *pLog;
    /** The timer */
    gfmTimer *pTimer;
    /** The GIF exporter */
    gfmGifExporter *pGif;
    /** Asynchronous loader */
    gfmLoadAsyncCtx *pAsyncLoader;
    /** Path where the snapshot should be saved */
    gfmString *pSsPath;
    /** Stores the snapshot */
    unsigned char *pSsData;
#if defined(DEBUG)
    /** FPS Counter; only enabled on debug version */
    gfmFPSCounter *pCounter;
#endif
#if defined(DEBUG)
    /** Debug spriteset */
    gfmSpriteset *pDebugSset;
#endif
    /** Every cached spriteset */
    gfmGenArr_var(gfmSpriteset, pSpritesets);
#if defined(DEBUG)
    /** Whether the FPS counter should be displayed */
    int showFPS;
#endif
    /** Buffer for storing a save file's filename */
    gfmString *pSaveFilename;
    /** Length until the end of the save file's directory (i.e., position to
     * append stuff) */
    int saveFilenameLen;
    /** Whether the backend was initialized */
    int isBackendInit;
    /** Flag to easily disable the audio; must be set after initialized the
     * lib */
    int isAudioEnabled;
    /** Moment, in milisecond, when the last draw op finished */
    unsigned int lastDrawnTime;
    /** Time elapsed since the last update (great for fixed 60fps update, when
     * using vsync) */
    unsigned int lastDrawElapsed;
    /** Texture that should be loaded on every gfm_drawBegin */
    int defaultTexture;
    /** Whether a quit event was received */
    gfmRV doQuit;
    /** Whether a snapshot should be taken */
    int takeSnapshot;
    /** Whether is recording an animation or a single snapshot */
    int isAnimation;
    /** For how long the animation should be recorded, in milliseconds */
    int animationTime;
    /** Number of bytes on the snapshot data */
    int ssDataLen;
    /** How many update frames were accumulated */
    int updateFrames;
    /** How many draw frames were accumulated */
    int drawFrames;
};

#endif /* __GFRAME_INT_GFMCTX_STRUCT_H__ */

