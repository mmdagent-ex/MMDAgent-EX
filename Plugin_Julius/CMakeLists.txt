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

project(Plugin_Julius)

set(SOURCES
    Julius_Logger.cpp
    Julius_Thread.cpp
    Julius_Record.cpp
    Plugin_Julius.cpp
)

set(INCLUDE_DIRS
    ../Library_Bullet_Physics/include
    ../Library_GLFW/include
    ../Library_MMDFiles/include
    ../Library_MMDAgent/include
    ../Library_Julius/include
)

add_library(Plugin_Julius SHARED ${SOURCES})

target_include_directories(Plugin_Julius PRIVATE ${INCLUDE_DIRS})

target_compile_definitions(Plugin_Julius PRIVATE MMDAGENT HAVE_STRCASECMP)

target_link_libraries(Plugin_Julius
    ${MMDAGENT_LINK_OPTIONS}
    JULIUS
)

add_custom_command(
    TARGET Plugin_Julius
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:Plugin_Julius> ${CMAKE_SOURCE_DIR}/Release/Plugins/Plugin_Julius.so
    COMMENT "Copying Plugin_Julius to Release/Plugins directory"
)
