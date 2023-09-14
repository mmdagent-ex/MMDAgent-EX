LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE           := oboe
LOCAL_SRC_FILES        := src/aaudio/AAudioLoader.cpp \
        src/aaudio/AudioStreamAAudio.cpp \
        src/common/LatencyTuner.cpp \
        src/common/AudioStream.cpp \
        src/common/AudioStreamBuilder.cpp \
        src/common/Utilities.cpp \
        src/fifo/FifoBuffer.cpp \
        src/fifo/FifoController.cpp \
        src/fifo/FifoControllerBase.cpp \
        src/fifo/FifoControllerIndirect.cpp \
        src/opensles/AudioInputStreamOpenSLES.cpp \
        src/opensles/AudioOutputStreamOpenSLES.cpp \
        src/opensles/AudioStreamBuffered.cpp \
        src/opensles/AudioStreamOpenSLES.cpp \
        src/opensles/EngineOpenSLES.cpp \
        src/opensles/OpenSLESUtilities.cpp \
        src/opensles/OutputMixerOpenSLES.cpp \
        src/common/StabilizedCallback.cpp \
        src/common/Trace.cpp \
        src/common/Version.cpp
LOCAL_C_INCLUDES       := $(LOCAL_PATH)/include $(LOCAL_PATH)/src
include $(BUILD_STATIC_LIBRARY)
