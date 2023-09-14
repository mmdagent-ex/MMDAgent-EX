LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE     := hts_engine_API
LOCAL_SRC_FILES  := src/lib/HTS_audio.c \
                    src/lib/HTS_engine.c \
                    src/lib/HTS_gstream.c \
                    src/lib/HTS_label.c \
                    src/lib/HTS_misc.c \
                    src/lib/HTS_model.c \
                    src/lib/HTS_pstream.c \
                    src/lib/HTS_sstream.c \
                    src/lib/HTS_vocoder.c
LOCAL_STATIC_LIBRARIES := PortAudio
LOCAL_C_INCLUDES := $(LOCAL_PATH)/include \
                    $(LOCAL_PATH)/../Library_PortAudio/include
LOCAL_CFLAGS     += -DAUDIO_PLAY_PORTAUDIO

include $(BUILD_STATIC_LIBRARY)
