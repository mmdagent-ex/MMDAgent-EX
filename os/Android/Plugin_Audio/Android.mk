LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE           := Plugin_Audio
LOCAL_SRC_FILES        := Plugin_Audio.cpp \
                          Audio_Manager.cpp \
                          Audio_Thread.cpp
LOCAL_STATIC_LIBRARIES := GLFW PortAudio
LOCAL_C_INCLUDES       := $(LOCAL_PATH)/../Library_MMDAgent/include \
                          $(LOCAL_PATH)/../Library_JPEG/include \
                          $(LOCAL_PATH)/../Library_Bullet_Physics/include \
                          $(LOCAL_PATH)/../Library_GLee/include \
                          $(LOCAL_PATH)/../Library_libpng/include \
                          $(LOCAL_PATH)/../Library_zlib/include \
                          $(LOCAL_PATH)/../Library_MMDFiles/include \
                          $(LOCAL_PATH)/../Library_GLFW/include \
                          $(LOCAL_PATH)/../Library_PortAudio/include
LOCAL_CFLAGS           += -DMMDAGENT

include $(BUILD_STATIC_LIBRARY)
