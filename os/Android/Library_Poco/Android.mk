LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := Poco
LOCAL_SRC_FILES := Android/$(TARGET_ARCH_ABI)/lib/libPoco.a
include $(PREBUILT_STATIC_LIBRARY)
