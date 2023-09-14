LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE     := MMDAgent
LOCAL_SRC_FILES  := src/lib/BoneController.cpp \
                    src/lib/BoneFaceControl.cpp \
                    src/lib/ContentManager.cpp \
                    src/lib/ContentManagerThreadWeb.cpp \
                    src/lib/ContentManagerThreadZip.cpp \
                    src/lib/ContentUpload.cpp \
                    src/lib/FreeTypeGL.cpp \
                    src/lib/KeyValue.cpp \
                    src/lib/LipSync.cpp \
                    src/lib/LogText.cpp \
                    src/lib/Message.cpp \
                    src/lib/MMDAgent.cpp \
                    src/lib/MMDAgent_utils.cpp \
                    src/lib/MotionStocker.cpp \
                    src/lib/Option.cpp \
                    src/lib/PMDObject.cpp \
                    src/lib/Plugin.cpp \
                    src/lib/Render.cpp \
                    src/lib/ScreenWindow.cpp \
                    src/lib/Slider.cpp \
                    src/lib/Stage.cpp \
                    src/lib/TileTexture.cpp \
                    src/lib/Timer.cpp \
                    src/lib/Menu.cpp \
                    src/lib/Tabbar.cpp \
                    src/lib/Button.cpp \
                    src/lib/InfoText.cpp \
                    src/lib/FileBrowser.cpp \
                    src/lib/Prompt.cpp
LOCAL_STATIC_LIBRARIES := MMDFiles GLFW Poco Plugin_Audio Plugin_Flite_plus_hts_engine Plugin_Julius Plugin_Open_JTalk Plugin_TextArea Plugin_VIManager Plugin_Variables Plugin_Kafka
LOCAL_C_INCLUDES := $(LOCAL_PATH)/include \
                    $(LOCAL_PATH)/../Library_JPEG/include \
                    $(LOCAL_PATH)/../Library_Bullet_Physics/include \
                    $(LOCAL_PATH)/../Library_GLee/include \
                    $(LOCAL_PATH)/../Library_libpng/include \
                    $(LOCAL_PATH)/../Library_zlib/include \
                    $(LOCAL_PATH)/../Library_MMDFiles/include \
                    $(LOCAL_PATH)/../Library_GLFW/include \
                    $(LOCAL_PATH)/../Library_FreeType/include \
                    $(LOCAL_PATH)/../Library_Poco/include \
                    $(LOCAL_PATH)/../Library_Poco/Android/$(TARGET_ARCH_ABI)/include \
                    $(LOCAL_PATH)/../Library_UTF8-CPP/include
LOCAL_CFLAGS     += -DMMDAGENT_DONTRENDERDEBUG \
                    -DMMDAGENT_DONTUSESHADOWMAP \
                    -DMMDAGENT_DONTPICKMODEL \
                    -DMMDAGENT_DONTUSEMOUSE \
                    -DMMDAGENT_DONTUSEWINDOW \
                    -DMMDAGENT_DONTUSELUMINOUS \
                    -DMMDAGENT_DONTUSEACCUM \
                    -DMMDAGENT

LOCAL_ARM_NEON := true

include $(BUILD_STATIC_LIBRARY)
