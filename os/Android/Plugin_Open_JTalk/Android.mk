LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE           := Plugin_Open_JTalk
LOCAL_SRC_FILES        := Plugin_Open_JTalk.cpp \
                          Open_JTalk.cpp \
                          Open_JTalk_Manager.cpp \
                          Open_JTalk_Thread.cpp
LOCAL_STATIC_LIBRARIES := Open_JTalk
LOCAL_C_INCLUDES       := $(LOCAL_PATH)/../Library_MMDAgent/include \
                          $(LOCAL_PATH)/../Library_JPEG/include \
                          $(LOCAL_PATH)/../Library_Bullet_Physics/include \
                          $(LOCAL_PATH)/../Library_GLee/include \
                          $(LOCAL_PATH)/../Library_libpng/include \
                          $(LOCAL_PATH)/../Library_zlib/include \
                          $(LOCAL_PATH)/../Library_MMDFiles/include \
                          $(LOCAL_PATH)/../Library_hts_engine_API/include \
                          $(LOCAL_PATH)/../Library_Open_JTalk/include \
                          $(LOCAL_PATH)/../Library_GLFW/include
LOCAL_CFLAGS           += -DMMDAGENT

include $(BUILD_STATIC_LIBRARY)
