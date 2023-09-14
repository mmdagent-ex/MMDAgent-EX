LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE           := Plugin_VIManager
LOCAL_SRC_FILES        := Plugin_VIManager.cpp \
                          VIManager.cpp \
                          VIManager_Logger.cpp \
                          VIManager_Thread.cpp
LOCAL_STATIC_LIBRARIES := RE2
LOCAL_C_INCLUDES       := $(LOCAL_PATH)/../Library_MMDAgent/include \
                          $(LOCAL_PATH)/../Library_JPEG/include \
                          $(LOCAL_PATH)/../Library_Bullet_Physics/include \
                          $(LOCAL_PATH)/../Library_GLee/include \
                          $(LOCAL_PATH)/../Library_libpng/include \
                          $(LOCAL_PATH)/../Library_zlib/include \
                          $(LOCAL_PATH)/../Library_MMDFiles/include \
                          $(LOCAL_PATH)/../Library_GLFW/include \
                          $(LOCAL_PATH)/../Library_RE2/include
LOCAL_CFLAGS           += -DMMDAGENT \
                          -DVIMANAGER_DONTRENDERDEBUG

include $(BUILD_STATIC_LIBRARY)
