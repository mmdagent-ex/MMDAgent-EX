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
project(Plugin_Variables)

# packages required to build this project
#find_package()

# list of source files
set(SOURCES
    CountDown_Thread.cpp
    Variables.cpp
    Plugin_Variables.cpp
)

# add this project as shared library
add_library(Plugin_Variables SHARED ${SOURCES})

# include dirs to build this project
target_include_directories(Plugin_Variables PRIVATE
    ../Library_Bullet_Physics/include
    ../Library_GLFW/include
    ../Library_MMDFiles/include
    ../Library_MMDAgent/include
)

# compiler definitions (-D) for buildtarget_compile_definitions(Plugin_Variables PRIVATE MMDAGENT)
#target_compile_definitions(Plugin_Variables PRIVATE
#)

# compiler options for build
target_compile_options(Plugin_Variables PRIVATE
   -Wno-deprecated-declarations
)

# library linking
target_link_libraries(Plugin_Variables
    ${MMDAGENT_LINK_OPTIONS}
)

# custom command to copy binaries
add_custom_command(
    TARGET Plugin_Variables
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:Plugin_Variables> ${CMAKE_SOURCE_DIR}/Release/Plugins/Plugin_Variables.so
    COMMENT "Copying Plugin_Variables to Release/Plugins directory"
)
