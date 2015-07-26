/**
 * @file src/core/sdl2/gfmAudio.c
 * 
 * Backend to load and play audios; Audio playback is expect to run
 * asynchronously, so it's the backend responsability to handle when its
 * processing thread should awake/sleep;
 * Before playing/loading any sound, it's necessary to manually initialize this
 * subsystem; During this initialization, one can define the system's quality;
 * At highers qualities, the system must respond more quickly and have more bits
 * per samples;
 * Then, audio files may be loaded into "audio structs"; Those should be managed
 * by the backend itself, only returning a handle (i.e., int) to the user; Each
 * audio may be set as repeating, with a custom repeat point (i.e., start at
 * position 0 but, when looping, goes back to position X);
 * When an audio is requested to be played, it must return a gfmAudioHandle;
 * This structure represents a instance of the played audio and must be used to
 * enabled modifing its volume, as way as making it stop playing;
 */
#include <GFraMe/gfmAssert.h>
#include <GFraMe/gfmError.h>
#include <GFraMe/gfmGenericArray.h>
#include <GFraMe/gfmString.h>
#include <GFraMe/gframe.h>
#include <GFraMe/core/gfmAudio_bkend.h>
#include <GFraMe_int/gfmAudio_mml.h>
#include <GFraMe_int/gfmAudio_vorbis.h>
#include <GFraMe_int/gfmAudio_wave.h>

// Required because of SDL_InitSubSystem... >_<
#include <SDL2/SDL.h>

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/** An audio representation */
typedef struct stGFMAudio gfmAudio;

/** Define an array of nodes */
gfmGenArr_define(gfmAudioHandle);
/** Define an array of audios */
gfmGenArr_define(gfmAudio);

/** Enumeration of systems already initialized */
enum enGFMAudioState {
    gfmAudio_SDLSystem = 0x01,
    gfmAudio_mutex     = 0x02,
    gfmAudio_device    = 0x04,
};
typedef enum enGFMAudioState gfmAudioState;

/** Audio sub-system context */
struct stGFMAudioCtx {
    /** List of available audio handles (for recycling) */
    gfmAudioHandle *pAudioHndAvailable;
    /** List of playing audio handles */
    gfmAudioHandle *pAudioHndPlaying;
    /** Which subsystems has been initialized */
    gfmAudioState init;
    /** Pool of audios */
    gfmGenArr_var(gfmAudio, pAudioPool);
    /** Pool of audio handles */
    gfmGenArr_var(gfmAudioHandle, pAudioHndPool);
    /** RV from the last time the callback was executed */
    gfmRV callbackRV;
    /** Whether the subsystem is playing or was paused */
    int isPlaying;
    /** Mutext to lock access to the context's resources */
    pthread_mutex_t mutex;
    /** ID of the opened audio device */
    SDL_AudioDeviceID dev;
    /** Specs of the opened audio device */
    SDL_AudioSpec spec;
};

/** An audio representation */
struct stGFMAudio {
    /** Audio samples buffer */
    unsigned char *pBuf;
    /** Number of bytes in pBuf */
    int len;
    /** Whether should repeat */
    int doRepeat;
    /** Position to which should jump to, on repeat */
    int repeatPosition;
};

/** Custom audio handle; Is returned when an audio is played */
struct stGFMAudioHandle {
    /** Audio played by this node */
    gfmAudio *pSelf;
    /** Next audio being played/available for recycling */
    gfmAudioHandle *pNext;
    /** Volume at which this should be played */
    double volume;
    /** Position in the current audio */
    int pos;
    /** Whether this instance is playing or not */
    int isPlaying;
};

/******************************************************************************/
/*                                                                            */
/* Static functions                                                           */
/*                                                                            */
/******************************************************************************/

/**
 * Callback called by SDL2 to fill the audio buffer
 * 
 * @param  pArg    The audio context (set on SDL_OpenAudioDevice)
 * @param  pStream The buffer to be filled
 * @param  len     How many bytes there are in the buffer
 */
static void gfmAudio_callback(void *pArg, Uint8 *pStream, int len) {
    gfmAudioCtx *pCtx;
    gfmAudioHandle *pTmp;
    gfmRV rv;
    int isLocked;
    int irv;
    
    // Set default values
    isLocked = 0;
    
    // Sanitize arguments
    ASSERT(pArg, GFMRV_ARGUMENTS_BAD);
    ASSERT(pStream, GFMRV_ARGUMENTS_BAD);
    ASSERT(len > 0, GFMRV_ARGUMENTS_BAD);
    
    // Retrieve the audio context
    pCtx = (gfmAudioCtx*)pArg;
    
    // Lock the mutex
    irv = pthread_mutex_lock(&(pCtx->mutex));
    ASSERT(irv == 0, GFMRV_INTERNAL_ERROR);
    isLocked = 1;
    
    // Clean the output buffer (so there's no noise)
    memset(pStream, 0x0, len);
    
    // Loop through every node
    pTmp = pCtx->pAudioHndPlaying;
    while (pTmp) {
        if (pTmp->isPlaying) {
            // TODO Play the buffer
        }
        // Go to the next node
        pTmp = pTmp->pNext;
    }
    // TODO Pause the device if there're no more audios
__ret:
    if (isLocked) {
        // Set the return value on the context
        pCtx->callbackRV = rv;
        // Unlock the mutex
        pthread_mutex_unlock(&(pCtx->mutex));
    }
}

/**
 * Alloc a new gfmAudioHandle
 * 
 * @param  ppCtx The audio handle
 * @return       GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_ALLOC_FAILED
 */
static gfmRV gfmAudio_getNewHandle(gfmAudioHandle **ppCtx) {
    gfmRV rv;
    
    // Sanitize arguments
    ASSERT(ppCtx, GFMRV_ARGUMENTS_BAD);
    ASSERT(!(*ppCtx), GFMRV_ARGUMENTS_BAD);
    
    // Alloc the object
    *ppCtx = (gfmAudioHandle*)malloc(sizeof(gfmAudioHandle));
    ASSERT(*ppCtx, GFMRV_ALLOC_FAILED);
    // Clean it
    memset(*ppCtx, 0x0, sizeof(gfmAudioHandle));
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Frees a gfmAudioHandle
 * 
 * @param  ppCtx The audio handle
 * @return       GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
static gfmRV gfmAudio_freeHandle(gfmAudioHandle **ppCtx) {
    gfmRV rv;
    
    // Sanitize arguments
    ASSERT(ppCtx, GFMRV_ARGUMENTS_BAD);
    ASSERT(*ppCtx, GFMRV_ARGUMENTS_BAD);
    
    // Since there's no memory alloc'ed, simply free the memory
    free(*ppCtx);
    *ppCtx = 0;
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Alloc a new gfmAudio
 * 
 * @param  ppCtx The audio
 * @return       GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_ALLOC_FAILED
 */
static gfmRV gfmAudio_getNewAudio(gfmAudio **ppCtx) {
    gfmRV rv;
    
    // Sanitize arguments
    ASSERT(ppCtx, GFMRV_ARGUMENTS_BAD);
    ASSERT(!(*ppCtx), GFMRV_ARGUMENTS_BAD);
    
    // Alloc the object
    *ppCtx = (gfmAudio*)malloc(sizeof(gfmAudio));
    ASSERT(*ppCtx, GFMRV_ALLOC_FAILED);
    // Clean it
    memset(*ppCtx, 0x0, sizeof(gfmAudio));
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Frees a gfmAudio
 * 
 * @param  ppCtx The audio handle
 * @return       GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
static gfmRV gfmAudio_freeAudio(gfmAudio **ppCtx) {
    gfmRV rv;
    
    // Sanitize arguments
    ASSERT(ppCtx, GFMRV_ARGUMENTS_BAD);
    ASSERT(*ppCtx, GFMRV_ARGUMENTS_BAD);
    
    // If it has a buffer, free it
    if ((*ppCtx)->pBuf) {
        free((*ppCtx)->pBuf);
    }
    // Free its memory
    free(*ppCtx);
    *ppCtx = 0;
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/******************************************************************************/
/*                                                                            */
/* Public functions                                                           */
/*                                                                            */
/******************************************************************************/

/**
 * Alloc a new gfmAudioCtx
 * 
 * @param  ppCtx The audio context
 * @return       GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_ALLOC_FAILED
 */
gfmRV gfmAudio_getNew(gfmAudioCtx **ppCtx) {
    gfmRV rv;
    
    // Sanitize arguments
    ASSERT(ppCtx, GFMRV_ARGUMENTS_BAD);
    ASSERT(!(*ppCtx), GFMRV_ARGUMENTS_BAD);
    
    // Alloc the struct
    *ppCtx = (gfmAudioCtx*)malloc(sizeof(gfmAudioCtx));
    ASSERT(ppCtx, GFMRV_ALLOC_FAILED);
    // Clean it
    memset(*ppCtx, 0x0, sizeof(gfmAudioCtx));
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Frees (and close, if it was initialized) the audio context
 * 
 * @param  ppCtx The audio context
 * @return       GFMRV_OK, GFMRV_ARGUMENTS_BAD
 */
gfmRV gfmAudio_Free(gfmAudioCtx **ppCtx) {
    gfmRV rv;
    
    // Sanitize arguments
    ASSERT(ppCtx, GFMRV_ARGUMENTS_BAD);
    ASSERT(*ppCtx, GFMRV_ARGUMENTS_BAD);
    
    // Close the subsystem
    gfmAudio_closeSubSystem(*ppCtx);
    
    // Free the memory
    free(*ppCtx);
    *ppCtx = 0;
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Initialize the audio subsystem
 * 
 * @param  pCtx    The audio context
 * @param  setting The desired audio settings
 * @return         GFMRV_OK, GFMRV_ARGUMENTS_BAD,
 *                 GFMRV_AUDIO_ALREADY_INITIALIZED, GFMRV_INTERNAL_ERROR
 */
gfmRV gfmAudio_initSubsystem(gfmAudioCtx *pCtx, gfmAudioQuality settings) {
    gfmRV rv;
    int irv;
    SDL_AudioSpec wanted;
    
    // Sanitize arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    // Check that it still wasn't initialized
    ASSERT(!pCtx->init, GFMRV_AUDIO_ALREADY_INITIALIZED);
    // TODO Add support for others settings
    ASSERT(settings == gfmAudio_defQuality, GFMRV_FUNCTION_NOT_SUPPORTED);
    
    // Initialize the audio subsystem
    irv = SDL_InitSubSystem(SDL_INIT_AUDIO);
    ASSERT(irv == 0, GFMRV_INTERNAL_ERROR);
    // Set the system as initialized
    pCtx->init |= gfmAudio_SDLSystem;
    
    // Initialize mutexes
    irv = pthread_mutex_init(&(pCtx->mutex), 0);
    ASSERT(irv == 0, GFMRV_INTERNAL_ERROR);
    // Set the mutex as initialized
    pCtx->init |= gfmAudio_mutex;
    
    // Set the desired audio format
    // Samples per second
    wanted.freq = 44100;
    // Sample format (i.e., signedness, endianess, number of bits per samples)
    wanted.format = AUDIO_S16LSB;
    // Number of channels (defaults to stereo)
    wanted.channels = 2;
    // Callback used to fill the buffer
    wanted.callback = gfmAudio_callback;
    // Send the audio context itself to the callback
    wanted.userdata = (void*)pCtx;
    
    // Open the audio device with the requested format (NOTE: SDL2 already
    // starts the device paused!!)
    pCtx->dev = SDL_OpenAudioDevice(NULL, 0, &wanted, &(pCtx->spec), 0);
    ASSERT(pCtx->dev > 0, GFMRV_INTERNAL_ERROR);
    // Set the device as initialized
    pCtx->init |= gfmAudio_device;
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Clear all alloc'ed memory and closed the subsystem
 * 
 * @param  pCtx The audio context
 * @return      GFMRV_OK, GFMRV_ARUMGENTS_BAD
 */
gfmRV gfmAudio_closeSubSystem(gfmAudioCtx *pCtx) {
    gfmRV rv;
    
    // Sanitize arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    
    // Try to pause the subsystem
    gfmAudio_pauseSubsystem(pCtx);
    // Close the device
    if (pCtx->init & gfmAudio_device) {
        SDL_CloseAudioDevice(pCtx->dev);
    }
    // Close the SDL subsystem
    if (pCtx->init & gfmAudio_SDLSystem) {
        SDL_QuitSubSystem(SDL_INIT_AUDIO);
    }
    // Destroy the mutex
    if (pCtx->init & gfmAudio_mutex) {
        pthread_mutex_destroy(&(pCtx->mutex));
    }
    
    // Free the generic arrays
    gfmGenArr_clean(pCtx->pAudioPool, gfmAudio_freeAudio);
    gfmGenArr_clean(pCtx->pAudioHndPool, gfmAudio_freeHandle);
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Stop the audio system, if it was playing
 * 
 * @param  pCtx The audio context
 * @return      GFMRV_OK, GFMRV_ARUMGENTS_BAD, GFMRV_AUDIO_NOT_INITIALIZED
 */
gfmRV gfmAudio_resumeSubsystem(gfmAudioCtx *pCtx) {
    gfmRV rv;
    int irv;
    
    // Sanitize arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    // Check that it was initialized
    ASSERT(pCtx->dev > 0, GFMRV_AUDIO_NOT_INITIALIZED);
    // Check that it was playing
    ASSERT(pCtx->isPlaying, GFMRV_OK);
    
    // Lock the mutex
    irv = pthread_mutex_lock(&(pCtx->mutex));
    ASSERT(irv == 0, GFMRV_INTERNAL_ERROR);
    // Only unpause if there's anything to play
    if (pCtx->pAudioHndPlaying) {
        SDL_PauseAudioDevice(pCtx->dev, 1);
    }
    // Unlock the mutex
    pthread_mutex_unlock(&(pCtx->mutex));
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Pauses the audio system; It will restart playing as soon as a new audio is
 * played or gfmAudio_resumeSubSystem is called
 * 
 * @param  pCtx The audio context
 * @return      GFMRV_OK, GFMRV_ARUMGENTS_BAD, GFMRV_AUDIO_NOT_INITIALIZED
 */
gfmRV gfmAudio_pauseSubsystem(gfmAudioCtx *pCtx) {
    gfmRV rv;
    int irv;
    
    // Sanitize arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    // Check that it was initialized
    ASSERT(pCtx->dev > 0, GFMRV_AUDIO_NOT_INITIALIZED);
    // Check that it was paused
    ASSERT(!(pCtx->isPlaying), GFMRV_OK);
    
    // Lock the mutex
    irv = pthread_mutex_lock(&(pCtx->mutex));
    ASSERT(irv == 0, GFMRV_INTERNAL_ERROR);
    // Pause the sub-system
    SDL_PauseAudioDevice(pCtx->dev, 0);
    // Unlock the mutex
    pthread_mutex_unlock(&(pCtx->mutex));
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Loads an audio from a file, converting it (if possible) to the current
 * format and sample rate; The file must be in the assets folder, but only the
 * relative path to assets should be sent
 * 
 * @param  pHandle     
 * @param  pAud        
 * @param  pCtx        
 * @param  pFilename   
 * @param  filenameLen 
 * @return             GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_COULDNT_OPEN_FILE,
 *                     GFMRV_READ_ERROR, GFMRV_AUDIO_FILE_NOT_SUPPORTED,
 GFMRV_ALLOC_FAILED, GFMRV_INTERNAL_ERROR
 */
gfmRV gfmAudio_loadAudio(int *pHandle, gfmAudioCtx *pAud, gfmCtx *pCtx,
        char *pFilename, int filenameLen) {
    char *pPath;
    FILE *pFp;
    gfmRV rv;
    gfmString *pStr;
    int didParse;
    
    // Set default values
    pFp = 0;
    didParse = 0;
    
    // Sanitize arguments
    ASSERT(pHandle, GFMRV_ARGUMENTS_BAD);
    ASSERT(pAud, GFMRV_ARGUMENTS_BAD);
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    ASSERT(pFilename, GFMRV_ARGUMENTS_BAD);
    ASSERT(filenameLen > 0, GFMRV_ARGUMENTS_BAD);
    
    // Append 'assets/' to the file path (note that 'pStr' is managed by the
    // game's context)
    rv = gfm_getBinaryPath(&pStr, pCtx);
    ASSERT_NR(rv == GFMRV_OK);
    rv = gfmString_concatStatic(pStr, "assets/");
    ASSERT_NR(rv == GFMRV_OK);
    rv = gfmString_concat(pStr, pFilename, filenameLen);
    ASSERT_NR(rv == GFMRV_OK);
    // Get the pointer to the string managed by 'pStr'
    rv = gfmString_getString(&pPath, pStr);
    ASSERT_NR(rv == GFMRV_OK);
    
    // Open the file
    pFp = fopen(pPath, "rb");
    ASSERT(pFp, GFMRV_COULDNT_OPEN_FILE);
    
    // Try to parse the file as WAVE
    if (didParse == 0) {
        rv = gfmAudio_isWave(pFp);
        if (rv == GFMRV_TRUE) {
            // TODO Parse the wave file
            didParse = 1;
        }
    }
    // Try to parse the file as MML
    if (didParse == 0) {
        rv = gfmAudio_isMml(pFp);
        if (rv == GFMRV_TRUE) {
            // TODO Add support for MML
            ASSERT(0, GFMRV_AUDIO_FILE_NOT_SUPPORTED);
        }
        didParse = 1;
    }
    // Try to parse the file as vorbis
    if (didParse == 0) {
        rv = gfmAudio_isVorbis(pFp);
        if (rv == GFMRV_TRUE) {
            // TODO Add support for MML
            ASSERT(0, GFMRV_AUDIO_FILE_NOT_SUPPORTED);
        }
        didParse = 1;
    }
    // Check that the file was parsed
    ASSERT(didParse == 1, GFMRV_AUDIO_FILE_NOT_SUPPORTED);
    
    rv = GFMRV_OK;
__ret:
    // Close the file, if it was opened
    if (pFp) {
        fclose(pFp);
    }
    
    return rv;
}

gfmRV gfmAudio_setRepeat(gfmAudioCtx *pCtx, int handle, int pos);

/**
 * Play an audio and return its instance's handle (so you can pause/play/stop it
 * and change its volume)
 * 
 * @param  ppHnd  The audio instance (may be NULL, if one simply doesn't care)
 * @param  pCtx   The audio context
 * @param  handle The handle of the audio to be played
 * @param  volume How loud should the audio be played (in the range (0.0, 1.0])
 * @return        GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_INVALID_INDEX,
 *                GFMRV_AUDIO_NOT_INITIALIZED, GFMRV_ALLOC_FAILED, 
 */
gfmRV gfmAudio_playAudio(gfmAudioHandle **ppHnd, gfmAudioCtx *pCtx, int handle
        , double volume) {
    gfmAudio *pAudio;
    gfmAudioHandle *pAudioHnd;
    gfmRV rv;
    int isLocked;
    int irv;
    
    // Set default values
    isLocked = 0;
    
    // Sanitize arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    ASSERT(volume > 0.0, GFMRV_ARGUMENTS_BAD);
    ASSERT(volume <= 1.0, GFMRV_ARGUMENTS_BAD);
    // Check that the handle is valid
    ASSERT(handle >= 0, GFMRV_INVALID_INDEX);
    ASSERT(handle < gfmGenArr_getUsed(pCtx->pAudioPool), GFMRV_INVALID_INDEX);
    // Check that it was initialized
    ASSERT(pCtx->dev > 0, GFMRV_AUDIO_NOT_INITIALIZED);
    
    // Retrieve the audio
    pAudio = gfmGenArr_getObject(pCtx->pAudioPool, handle);
    
    // Lock the mutex
    irv = pthread_mutex_lock(&(pCtx->mutex));
    ASSERT(irv == 0, GFMRV_INTERNAL_ERROR);
    isLocked = 1;
    
    // Check if there's any handle to be recycled
    if (pCtx->pAudioHndAvailable) {
        // Recycle a handle
        pAudioHnd = pCtx->pAudioHndAvailable;
        // Remove that handle from the list
        pCtx->pAudioHndAvailable = pCtx->pAudioHndAvailable->pNext;
    }
    else {
        // Otherwise, alloc a new handle
        gfmGenArr_getNextRef(gfmAudioHandle, pCtx->pAudioHndPool, 1, pAudioHnd,
                gfmAudio_getNewHandle);
        gfmGenArr_push(pCtx->pAudioHndPool);
    }
    // Set the audio to be played
    pAudioHnd->pSelf = pAudio;
    // Set the audio's volume
    pAudioHnd->volume = volume;
    // Set the at the audio's start
    pAudioHnd->pos = 0;
    // Set the instance as playing
    pAudioHnd->isPlaying = 1;
    // Prepend it to the playing list
    pAudioHnd->pNext = pCtx->pAudioHndPlaying;
    pCtx->pAudioHndPlaying = pAudioHnd->pNext;
    
    rv = GFMRV_OK;
__ret:
    if (isLocked) {
        // Unlock the mutex
        pthread_mutex_unlock(&(pCtx->mutex));
    }
    return rv;
}

/**
 * Stops an audio instance
 * 
 * @param  pCtx  The audio context
 * @param  ppHnd The instance to be stopped
 * @return       GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_AUDIO_NOT_INITIALIZED
 */
gfmRV gfmAudio_stopAudio(gfmAudioCtx *pCtx, gfmAudioHandle **ppHnd) {
    gfmRV rv;
    int irv;
    
    // Sanitize arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    ASSERT(ppHnd, GFMRV_ARGUMENTS_BAD);
    ASSERT(*ppHnd, GFMRV_ARGUMENTS_BAD);
    // Check that it was initialized
    ASSERT(pCtx->dev > 0, GFMRV_AUDIO_NOT_INITIALIZED);
    
    // Lock the mutex
    irv = pthread_mutex_lock(&(pCtx->mutex));
    ASSERT(irv == 0, GFMRV_INTERNAL_ERROR);
    // Instead of actually removing it, put it at the buffer's end, so it will
    // be removed next time the callback is called
    (*ppHnd)->pos = (*ppHnd)->pSelf->len;
    // "Clean" the returned pointer
    *ppHnd = 0;
    // Unlock the mutex
    pthread_mutex_unlock(&(pCtx->mutex));
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Pause a currently playing audio
 * 
 * @param  pCtx The audio context
 * @param  pHnd The instance handle
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_AUDIO_NOT_INITIALIZED
 */
gfmRV gfmAudio_pauseAudio(gfmAudioCtx *pCtx, gfmAudioHandle *pHnd) {
    gfmRV rv;
    int irv;
    
    // Sanitize arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    ASSERT(pHnd, GFMRV_ARGUMENTS_BAD);
    // Check that it was initialized
    ASSERT(pCtx->dev > 0, GFMRV_AUDIO_NOT_INITIALIZED);
    
    // Lock the mutex
    irv = pthread_mutex_lock(&(pCtx->mutex));
    ASSERT(irv == 0, GFMRV_INTERNAL_ERROR);
    pHnd->isPlaying = 0;
    // Unlock the mutex
    pthread_mutex_unlock(&(pCtx->mutex));
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Resume a paused audio
 * 
 * @param  pCtx The audio context
 * @param  pHnd The instance handle
 * @return      GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_AUDIO_NOT_INITIALIZED
 */
gfmRV gfmAudio_resumeAudio(gfmAudioCtx *pCtx, gfmAudioHandle *pHnd) {
    gfmRV rv;
    int irv;
    
    // Sanitize arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    ASSERT(pHnd, GFMRV_ARGUMENTS_BAD);
    // Check that it was initialized
    ASSERT(pCtx->dev > 0, GFMRV_AUDIO_NOT_INITIALIZED);
    
    // Lock the mutex
    irv = pthread_mutex_lock(&(pCtx->mutex));
    ASSERT(irv == 0, GFMRV_INTERNAL_ERROR);
    pHnd->isPlaying = 1;
    // Unlock the mutex
    pthread_mutex_unlock(&(pCtx->mutex));
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Set an audio's volume
 * 
 * @param  pCtx   The audio context
 * @param  pHnd   The instance handle
 * @param  volume The new volume (in the range (0.0, 1.0])
 * @return        GFMRV_OK, GFMRV_ARGUMENTS_BAD, GFMRV_AUDIO_NOT_INITIALIZED
 */
gfmRV gfmAudio_setHandleVolume(gfmAudioCtx *pCtx, gfmAudioHandle *pHnd,
        double volume) {
    gfmRV rv;
    int irv;
    
    // Sanitize arguments
    ASSERT(pCtx, GFMRV_ARGUMENTS_BAD);
    ASSERT(pHnd, GFMRV_ARGUMENTS_BAD);
    ASSERT(volume > 0.0, GFMRV_ARGUMENTS_BAD);
    ASSERT(volume <= 1.0, GFMRV_ARGUMENTS_BAD);
    // Check that it was initialized
    ASSERT(pCtx->dev > 0, GFMRV_AUDIO_NOT_INITIALIZED);
    
    // Lock the mutex
    irv = pthread_mutex_lock(&(pCtx->mutex));
    ASSERT(irv == 0, GFMRV_INTERNAL_ERROR);
    pHnd->volume = volume;
    // Unlock the mutex
    pthread_mutex_unlock(&(pCtx->mutex));
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

gfmRV gfmAudio_isTrackSupported(gfmAudioCtx *pCtx) {
    return GFMRV_FALSE;
}

gfmRV gfmAudio_getNumTracks(int *pNum, gfmAudioCtx *pCtx, int handle) {
    return GFMRV_FUNCTION_NOT_SUPPORTED;
}

gfmRV gfmAudio_setTrackVolume(gfmAudioCtx *pCtx, int handle, double volume) {
    return GFMRV_FUNCTION_NOT_SUPPORTED;
}


