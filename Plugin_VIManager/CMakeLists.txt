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
project(Plugin_VIManager)

# packages required to build this project
if(APPLE)
    find_package(re2 REQUIRED)
    set(RE_LIBRARIES re2::re2)
else()
    set(RE_LIBRARIES -lre2)
endif()

# list of source files
set(SOURCES
    Plugin_VIManager.cpp
    VIManager.cpp
    VIManager_Logger.cpp
    VIManager_Thread.cpp
)

# add this project as shared library
add_library(Plugin_VIManager SHARED ${SOURCES})

# include dirs to build this project
target_include_directories(Plugin_VIManager PRIVATE
    ../Library_Bullet_Physics/include
    ../Library_GLFW/include
    ../Library_MMDFiles/include
    ../Library_MMDAgent/include
)

# compiler definitions (-D) for build
target_compile_definitions(Plugin_VIManager PRIVATE
    MMDAGENT
)

# compiler options for build
target_compile_options(Plugin_VIManager PRIVATE
    -std=c++17
    -Wno-deprecated-declarations
)

# library linking
target_link_libraries(Plugin_VIManager
    ${MMDAGENT_LINK_OPTIONS}
    ${RE_LIBRARIES}
)

# custom command to copy binaries
add_custom_command(
    TARGET Plugin_VIManager
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:Plugin_VIManager> ${CMAKE_SOURCE_DIR}/Release/Plugins/Plugin_VIManager.so
    COMMENT "Copying Plugin_VIManager to Release/Plugins directory"
)
