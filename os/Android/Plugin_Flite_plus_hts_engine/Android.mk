LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE           := Plugin_Flite_plus_hts_engine
LOCAL_SRC_FILES        := Plugin_Flite_plus_hts_engine.cpp \
                          Flite_plus_hts_engine.cpp \
                          Flite_plus_hts_engine_Manager.cpp \
                          Flite_plus_hts_engine_Thread.cpp
LOCAL_STATIC_LIBRARIES := hts_engine_API Flite_plus_hts_engine
LOCAL_C_INCLUDES       := $(LOCAL_PATH)/../Library_MMDAgent/include \
                          $(LOCAL_PATH)/../Library_JPEG/include \
                          $(LOCAL_PATH)/../Library_Bullet_Physics/include \
                          $(LOCAL_PATH)/../Library_GLee/include \
                          $(LOCAL_PATH)/../Library_libpng/include \
                          $(LOCAL_PATH)/../Library_zlib/include \
                          $(LOCAL_PATH)/../Library_MMDFiles/include \
                          $(LOCAL_PATH)/../Library_hts_engine_API/include \
                          $(LOCAL_PATH)/../Library_Flite_plus_hts_engine/include \
                          $(LOCAL_PATH)/../Library_GLFW/include
LOCAL_CFLAGS           += -DMMDAGENT

include $(BUILD_STATIC_LIBRARY)
