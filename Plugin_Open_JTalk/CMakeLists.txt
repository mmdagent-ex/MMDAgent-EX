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
project(Plugin_Open_JTalk)

# packages required to build this project
#find_package()

# list of source files
set(SOURCES
    Open_JTalk.cpp
    Open_JTalk_Manager.cpp
    Open_JTalk_Thread.cpp
    Plugin_Open_JTalk.cpp
)

# add this project as shared library
add_library(Plugin_Open_JTalk SHARED ${SOURCES})

# include dirs to build this project
target_include_directories(Plugin_Open_JTalk PRIVATE
    ../Library_Bullet_Physics/include
    ../Library_GLFW/include
    ../Library_MMDFiles/include
    ../Library_MMDAgent/include
    ../Library_hts_engine_API/include
    ../Library_Open_JTalk/include
)

# compiler definitions (-D) for build
#target_compile_definitions(Plugin_Open_JTalk PRIVATE
#)

# compiler options for build
target_compile_options(Plugin_Open_JTalk PRIVATE
   -Wno-deprecated-declarations
)

# library linking
target_link_libraries(Plugin_Open_JTalk
    ${MMDAGENT_LINK_OPTIONS}
    OPEN_JTALK
)

# custom command to copy binaries
add_custom_command(
    TARGET Plugin_Open_JTalk
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:Plugin_Open_JTalk> ${CMAKE_SOURCE_DIR}/Release/Plugins/Plugin_Open_JTalk.so
    COMMENT "Copying Plugin_Open_JTalk to Release/Plugins directory"
)
