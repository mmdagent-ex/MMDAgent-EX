LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE     := libpng
LOCAL_SRC_FILES  := src/png.c \
                    src/pngerror.c \
                    src/pngget.c \
                    src/pngmem.c \
                    src/pngpread.c \
                    src/pngread.c \
                    src/pngrio.c \
                    src/pngrtran.c \
                    src/pngrutil.c \
                    src/pngset.c \
                    src/pngtrans.c \
                    src/pngwio.c \
                    src/pngwrite.c \
                    src/pngwtran.c \
                    src/pngwutil.c
LOCAL_C_INCLUDES := $(LOCAL_PATH)/include \
                    $(LOCAL_PATH)/../Library_zlib/include
ifeq ($(TARGET_ARCH_ABI),armeabi-v7a)
  LOCAL_SRC_FILES += src/arm/arm_init.c \
                     src/arm/filter_neon.S
endif

LOCAL_CFLAGS += -fno-integrated-as

LOCAL_ARM_NEON := true

include $(BUILD_STATIC_LIBRARY)
