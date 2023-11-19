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

# project name
project(Plugin_Audio)

# packages required to build this project
#find_package()

# list of source files
set(SOURCES
    Audio_Manager.cpp
    Audio_Thread.cpp
    Plugin_Audio.cpp
)

# add this project as shared library
add_library(Plugin_Audio SHARED ${SOURCES})

# include dirs to build this project
target_include_directories(Plugin_Audio PRIVATE
    ../Library_Bullet_Physics/include
    ../Library_GLFW/include
    ../Library_MMDFiles/include
    ../Library_MMDAgent/include
)

# compiler definitions (-D) for build
target_compile_definitions(Plugin_Audio PRIVATE
    MMDAGENT
)

target_compile_options(Plugin_Audio PRIVATE
   -Wno-deprecated-declarations
   -Wno-format-truncation
)

#library linking
if(APPLE)
    target_link_libraries(Plugin_Audio
        ${MMDAGENT_LINK_OPTIONS}
        "-framework AudioUnit"
        "-framework AudioToolbox"
        "-framework CoreAudio"
    )
else()
    target_link_libraries(Plugin_Audio
    	${MMDAGENT_LINK_OPTIONS}
    )
endif()

# custom command to copy binaries
add_custom_command(
    TARGET Plugin_Audio
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:Plugin_Audio> ${CMAKE_SOURCE_DIR}/Release/Plugins/Plugin_Audio.so
    COMMENT "Copying Plugin_Audio to Release/Plugins directory"
)
