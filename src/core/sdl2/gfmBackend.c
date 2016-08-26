/**
 * @file src/core/sdl2/gfmBackend.c
 * 
 * Initializes and finalize a backend
 */
#include <GFraMe/gfmAssert.h>
#include <GFraMe/gfmError.h>
#include <GFraMe/core/gfmBackend_bkend.h>

#include <SDL2/SDL.h>

#include <string.h>

#if defined(GFRAME_MOBILE)
#  include <SDL2/SDL_platform.h>
#  include <SDL2/SDL_system.h>

/** Version of the device running this */
int androidVersion;
#endif

#if defined(GFRAME_ANDROID)
#  include <jni.h>
#endif

/** Whether SDL2 was already initialized */
static int isSDLInit = 0;

/**
 * Initialize a backend
 * 
 * @return GFMRV_OK, GFMRV_INTERNAL_ERROR, GFMRV_BACKEND_ALREADY_INITIALIZED
 */
gfmRV gfmBackend_init() {
    gfmRV rv;
    int irv;
    
    ASSERT(isSDLInit == 0, GFMRV_BACKEND_ALREADY_INITIALIZED);
    irv = SDL_Init(0);
    ASSERT(irv >= 0, GFMRV_INTERNAL_ERROR);

#if defined(GFRAME_MOBILE)
#  if defined(GFRAME_ANDROID)
    if (strcmp(SDL_GetPlatform(), "Android") == 0) {
        JNIEnv *env;
        jclass versionClass;
        jfieldID sdkIntFieldID;
        jint sdkInt;

        env = (JNIEnv*)SDL_AndroidGetJNIEnv();
        ASSERT(env, GFMRV_INTERNAL_ERROR);
        versionClass = (*env)->FindClass(env, "android/os/Build$VERSION");
        ASSERT(versionClass != 0, GFMRV_INTERNAL_ERROR);
        sdkIntFieldID = (*env)->GetStaticFieldID(env, versionClass, "SDK_INT", "I");
        ASSERT(sdkIntFieldID != 0, GFMRV_INTERNAL_ERROR);
        sdkInt = (*env)->GetStaticIntField(env, versionClass, sdkIntFieldID);
        androidVersion = (int)sdkInt;
    }
    else {
        androidVersion = -1;
    }
#  else
    androidVersion = -1;
#  endif
#endif
    
    isSDLInit = 1;
    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Finalize a backend
 * 
 * @return GFMRV_OK, GFMRV_BACKEND_NOT_INITIALIZED
 */
gfmRV gfmBackend_finalize() {
    gfmRV rv;
    
    ASSERT(isSDLInit == 1, GFMRV_BACKEND_NOT_INITIALIZED);
    SDL_Quit();
    isSDLInit = 0;
    
    rv = GFMRV_OK;
__ret:
    return rv;
}

