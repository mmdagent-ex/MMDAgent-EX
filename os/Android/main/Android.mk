LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE           := main
LOCAL_SRC_FILES        := main.cpp
LOCAL_LDLIBS           := -lmediandk -llog -landroid -lEGL -lGLESv1_CM -lOpenSLES -ldl
LOCAL_STATIC_LIBRARIES := MMDAgent android_native_app_glue
LOCAL_C_INCLUDES       := $(LOCAL_PATH)/../Library_JPEG/include \
                          $(LOCAL_PATH)/../Library_Bullet_Physics/include \
                          $(LOCAL_PATH)/../Library_GLee/include \
                          $(LOCAL_PATH)/../Library_libpng/include \
                          $(LOCAL_PATH)/../Library_zlib/include \
                          $(LOCAL_PATH)/../Library_MMDFiles/include \
                          $(LOCAL_PATH)/../Library_MMDAgent/include \
                          $(LOCAL_PATH)/../Library_GLFW/include \
                          $(LOCAL_PATH)/../Library_FreeType/include
LOCAL_CFLAGS           += -DMMDAGENT

include $(BUILD_SHARED_LIBRARY)

$(call import-module,android/native_app_glue)
