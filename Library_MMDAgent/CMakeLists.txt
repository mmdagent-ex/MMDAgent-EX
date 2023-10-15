#
#  Copyright 2022-2023  Nagoya Institute of Technology
#
#  Licensed under the Apache License, Version 2.0 (the "License");
#  you may not use this file except in compliance with the License.
#  You may obtain a copy of the License at
#
#    http://www.apache.org/licenses/LICENSE-2.0
#
#  Unless required by applicable law or agreed to in writing, software
#  distributed under the License is distributed on an "AS IS" BASIS,
#  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#  See the License for the specific language governing permissions and
#  limitations under the License.
#

cmake_minimum_required(VERSION 3.10)

project(MMDAGENT)

set(LIB_SOURCES
    src/lib/BoneController.cpp
    src/lib/BoneFaceControl.cpp
    src/lib/Button.cpp
    src/lib/Caption.cpp
    src/lib/ContentManager.cpp
    src/lib/ContentManagerThreadWeb.cpp
    src/lib/ContentManagerThreadZip.cpp
    src/lib/ContentUpload.cpp
    src/lib/FileBrowser.cpp
    src/lib/FreeTypeGL.cpp
    src/lib/InfoText.cpp
    src/lib/KeyHandler.cpp
    src/lib/KeyValue.cpp
    src/lib/LipSync.cpp
    src/lib/LogText.cpp
    src/lib/Menu.cpp
    src/lib/Message.cpp
    src/lib/MMDAgent.cpp
    src/lib/MMDAgent_utils.cpp
    src/lib/MotionStocker.cpp
    src/lib/Option.cpp
    src/lib/Plugin.cpp
    src/lib/PMDFaceInterface.cpp
    src/lib/PMDObject.cpp
    src/lib/Prompt.cpp
    src/lib/Render.cpp
    src/lib/RenderOffScreen.cpp
    src/lib/ScreenWindow.cpp
    src/lib/ShapeMap.cpp
    src/lib/Slider.cpp
    src/lib/Stage.cpp
    src/lib/Tabbar.cpp
    src/lib/ThreadedLoading.cpp
    src/lib/TileTexture.cpp
    src/lib/Timer.cpp
)

set(INCLUDE_DIRS
    ../Library_Bullet_Physics/include
    ../Library_FreeType/include
    ../Library_UTF8-CPP/include
    ../Library_MMDFiles/include
    ../Library_GLFW/include
    ../Library_libpng/include
    src/include
    /usr/X11/include
)

set(COMPILER_DEFS
    GL_GLEXT_PROTOTYPES
    MMDAGENT
    MMDAGENT_DONTPICKMODEL
)

add_library(MMDAGENT STATIC ${LIB_SOURCES})

if(APPLE)
   target_compile_options(MMDAGENT PRIVATE -std=c++11)
endif()

target_include_directories(MMDAGENT BEFORE PRIVATE ${INCLUDE_DIRS})

target_compile_definitions(MMDAGENT PRIVATE ${COMPILER_DEFS})

target_link_libraries(MMDAGENT PUBLIC MMDFILES FreeType)
