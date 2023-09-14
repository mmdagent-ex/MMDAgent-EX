LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE           := PortAudio
LOCAL_SRC_FILES        := src/src/hostapi/aaudio/pa_android_oboe.cpp src/src/hostapi/aaudio/pa_android_opensles_sub.c
LOCAL_STATIC_LIBRARIES := oboe
LOCAL_C_INCLUDES       := $(LOCAL_PATH)/include \
                          $(LOCAL_PATH)/src/src/common \
                          $(LOCAL_PATH)/../oboe/include

include $(BUILD_STATIC_LIBRARY)
