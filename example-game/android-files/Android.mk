LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := main

SDL_PATH := ../SDL2-2.0.3

LOCAL_C_INCLUDES := $(LOCAL_PATH)/../include

# Add your application source files here...
LOCAL_SRC_FILES := $(SDL_PATH)/src/main/android/SDL_android_main.c \
       ../GFraMe/gframe_accumulator.c ../GFraMe/gframe_animation.c \
       ../GFraMe/gframe_assets.c ../GFraMe/gframe_object.c \
       ../GFraMe/gframe_screen.c ../GFraMe/gframe_sprite.c \
       ../GFraMe/gframe_spriteset.c ../GFraMe/gframe_texture.c \
       ../GFraMe/gframe_timer.c ../GFraMe/gframe_util.c \
       ../GFraMe/gframe_tilemap.c ../GFraMe/gframe_audio.c \
       ../GFraMe/gframe_audio_player.c ../GFraMe/gframe_messagebox.c \
       ../GFraMe/gframe_save.c ../GFraMe/gframe_hitbox.c \
	   ../GFraMe/gframe_tween.c ../GFraMe/gframe_pointer.c \
	   ../GFraMe/gframe_mobile.c \
       ../GFraMe/wavtodata/chunk.c ../GFraMe/wavtodata/fmt.c \
       ../GFraMe/wavtodata/wavtodata.c \
       main.c global.c playstate.c enemies.c player.c \
       score.c multiplier.c background.c menustate.c \
       button.c gameover.c

LOCAL_SHARED_LIBRARIES := SDL2

LOCAL_CFLAGS := -DMOBILE
LOCAL_LDLIBS := -lGLESv1_CM -lGLESv2 -llog

include $(BUILD_SHARED_LIBRARY)

