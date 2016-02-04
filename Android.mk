LOCAL_PATH := $(call my-dir)

###########################
#
# GFraMe shared library
#
###########################

include $(CLEAR_VARS)

LOCAL_MODULE := GFraMe

LOCAL_C_INCLUDES := $(LOCAL_PATH)/../include $(LOCAL_PATH)/include

LOCAL_EXPORT_C_INCLUDES := $(LOCAL_C_INCLUDES)

LOCAL_SRC_FILES := \
        $(LOCAL_PATH)/gfmAccumulator.c \
        $(LOCAL_PATH)/gfmAnimation.c \
        $(LOCAL_PATH)/gfmCamera.c \
        $(LOCAL_PATH)/gfmError.c \
        $(LOCAL_PATH)/gfmGroup.c \
        $(LOCAL_PATH)/gfmInput.c \
        $(LOCAL_PATH)/gfmLog.c \
        $(LOCAL_PATH)/gfmObject.c \
        $(LOCAL_PATH)/gfmParser.c \
        $(LOCAL_PATH)/gfmQuadtree.c \
        $(LOCAL_PATH)/gfmSave.c \
        $(LOCAL_PATH)/gfmSprite.c \
        $(LOCAL_PATH)/gfmSpriteset.c \
        $(LOCAL_PATH)/gfmString.c \
        $(LOCAL_PATH)/gfmText.c \
        $(LOCAL_PATH)/gfmTilemap.c \
        $(LOCAL_PATH)/gfmUtils.c \
        $(LOCAL_PATH)/gframe.c \
        $(LOCAL_PATH)/core/noip/gfmGifExporter.c \
        $(LOCAL_PATH)/core/event/android/gfmEvent_android.c \
        $(LOCAL_PATH)/core/video/sdl2/gfmVideo_sdl2.c \
        $(LOCAL_PATH)/core/sdl2/gfmAudio.c \
        $(LOCAL_PATH)/core/sdl2/gfmBackend.c \
        $(LOCAL_PATH)/core/sdl2/gfmFile.c \
        $(LOCAL_PATH)/core/sdl2/gfmPath.c \
        $(LOCAL_PATH)/core/sdl2/gfmTimer.c \
        $(LOCAL_PATH)/util/gfmAudio_mml.c \
        $(LOCAL_PATH)/util/gfmAudio_vorbis.c \
        $(LOCAL_PATH)/util/gfmAudio_wave.c \
        $(LOCAL_PATH)/util/gfmFPSCounter.c \
        $(LOCAL_PATH)/util/gfmGroupHelpers.c \
        $(LOCAL_PATH)/util/gfmKeyNode.c \
        $(LOCAL_PATH)/util/gfmParserCommon.c \
        $(LOCAL_PATH)/util/gfmTileAnimation.c \
        $(LOCAL_PATH)/util/gfmTileType.c \
        $(LOCAL_PATH)/util/gfmTrie.c \
        $(LOCAL_PATH)/util/gfmVideo_bmp.c \
        $(LOCAL_PATH)/util/gfmVirtualKey.c

LOCAL_SHARED_LIBRARIES := SDL2 CSynth
LOCAL_CFLAGS += -DGL_GLEXT_PROTOTYPES -DALIGN=4 -DFORCE_FPS
LOCAL_LDLIBS := -ldl -lGLESv1_CM -lGLESv2 -llog -landroid

include $(BUILD_SHARED_LIBRARY)

