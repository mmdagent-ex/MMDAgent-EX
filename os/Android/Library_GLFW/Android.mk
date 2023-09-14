LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE           := GLFW
LOCAL_SRC_FILES        := src/lib/enable.c \
                          src/lib/glext.c \
                          src/lib/init.c \
                          src/lib/input.c \
                          src/lib/thread.c \
                          src/lib/time.c \
                          src/lib/window.c \
                          src/lib/android/android_enable.c \
                          src/lib/android/android_glext.c \
                          src/lib/android/android_init.c \
                          src/lib/android/android_thread.c \
                          src/lib/android/android_time.c \
                          src/lib/android/android_window.c
LOCAL_STATIC_LIBRARIES := android_native_app_glue
LOCAL_C_INCLUDES       := $(LOCAL_PATH)/include \
                          $(LOCAL_PATH)/src/lib \
                          $(LOCAL_PATH)/src/lib/android
LOCAL_CFLAGS           += -D_GLFW_HAS_PTHREAD \
                          -D_GLFW_HAS_SYSCONF \
                          -DMMDAGENT

include $(BUILD_STATIC_LIBRARY)

