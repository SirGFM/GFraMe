/**
 * @src/gframe_mobile.h
 */
#include <GFraMe/GFraMe_error.h>
#include <GFraMe/GFraMe_mobile.h>
#include <SDL2/SDL_system.h>
#if defined(__ANDROID__) && __ANDROID__
#include <jni.h>	
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

