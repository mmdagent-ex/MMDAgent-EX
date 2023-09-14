LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE     := JPEG
LOCAL_SRC_FILES  := src/jaricom.c \
                    src/jcapimin.c \
                    src/jcapistd.c \
                    src/jcarith.c \
                    src/jccoefct.c \
                    src/jccolor.c \
                    src/jcdctmgr.c \
                    src/jchuff.c \
                    src/jcinit.c \
                    src/jcmainct.c \
                    src/jcmarker.c \
                    src/jcmaster.c \
                    src/jcomapi.c \
                    src/jcparam.c \
                    src/jcprepct.c \
                    src/jcsample.c \
                    src/jctrans.c \
                    src/jdapimin.c \
                    src/jdapistd.c \
                    src/jdarith.c \
                    src/jdatadst.c \
                    src/jdatasrc.c \
                    src/jdcoefct.c \
                    src/jdcolor.c \
                    src/jddctmgr.c \
                    src/jdhuff.c \
                    src/jdinput.c \
                    src/jdmainct.c \
                    src/jdmarker.c \
                    src/jdmaster.c \
                    src/jdmerge.c \
                    src/jdpostct.c \
                    src/jdsample.c \
                    src/jdtrans.c \
                    src/jerror.c \
                    src/jfdctflt.c \
                    src/jfdctfst.c \
                    src/jfdctint.c \
                    src/jidctflt.c \
                    src/jidctfst.c \
                    src/jidctint.c \
                    src/jquant1.c \
                    src/jquant2.c \
                    src/jutils.c \
                    src/jmemmgr.c \
                    src/jmemnobs.c
LOCAL_C_INCLUDES := $(LOCAL_PATH)/include

LOCAL_ARM_NEON := true

include $(BUILD_STATIC_LIBRARY)
