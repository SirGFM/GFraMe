/**
 * @src/gframe_mobile.h
 */
#include <GFraMe/GFraMe_error.h>
#include <GFraMe/GFraMe_mobile.h>
#include <SDL2/SDL_system.h>
#if defined(__ANDROID__) && __ANDROID__
#include <jni.h>	
#endif

#if defined(__ANDROID__) && __ANDROID__
jint JNI_OnLoad(JavaVM* vm, void* reserved)
{
/*
    JNIEnv *env;
    mJavaVM = vm;
    LOGI("JNI_OnLoad called");
    if ((*mJavaVM)->GetEnv(mJavaVM, (void**) &env, JNI_VERSION_1_4) != JNI_OK) {
        LOGE("Failed to get the environment using GetEnv()");
        return -1;
    }
    
    //
    // Create mThreadKey so we can keep track of the JNIEnv assigned to each thread
    // Refer to http://developer.android.com/guide/practices/design/jni.html for the rationale behind this
    //
    if (pthread_key_create(&mThreadKey, Android_JNI_ThreadDestroyed) != 0) {
        __android_log_print(ANDROID_LOG_ERROR, "SDL", "Error initializing pthread key");
    }
    Android_JNI_SetupThread();
*/
    return JNI_VERSION_1_4;
}
#endif

GFraMe_mobile_err GFraMe_mobile_call_void_function(char *functionName) {
	GFraMe_mobile_err rv = GFraMe_mobile_ok;
#if defined(__ANDROID__) && __ANDROID__
	JNIEnv *env = (JNIEnv*)SDL_AndroidGetJNIEnv();
	jobject actv = (jobject)SDL_AndroidGetActivity();
	jclass clazz = (*env)->GetObjectClass(env, actv);
	jmethodID method = (*env)->GetMethodID(env, clazz, functionName, "()V");
	GFraMe_assertRV(method != NULL, "Failed to get method",
				rv = GFraMe_mobile_function_not_found, _ret);
	(*env)->CallVoidMethod(env, actv, method);
#endif
_ret:
	return rv;
}

