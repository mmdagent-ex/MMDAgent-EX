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

project(Plugin_Flite_plus_hts_engine)

set(SOURCES
    Flite_plus_hts_engine.cpp
    Flite_plus_hts_engine_Manager.cpp
    Flite_plus_hts_engine_Thread.cpp
    Plugin_Flite_plus_hts_engine.cpp
)

set(INCLUDE_DIRS
    ../Library_Bullet_Physics/include
    ../Library_GLFW/include
    ../Library_MMDFiles/include
    ../Library_MMDAgent/include
    ../Library_hts_engine_API/include
    ../Library_Flite_plus_hts_engine/include
)

add_library(Plugin_Flite_plus_hts_engine SHARED ${SOURCES})

target_include_directories(Plugin_Flite_plus_hts_engine PRIVATE ${INCLUDE_DIRS})

target_compile_definitions(Plugin_Flite_plus_hts_engine PRIVATE MMDAGENT)

target_link_libraries(Plugin_Flite_plus_hts_engine
    ${MMDAGENT_LINK_OPTIONS}
    HTSENGINEAPI
)

add_custom_command(
    TARGET Plugin_Flite_plus_hts_engine
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:Plugin_Flite_plus_hts_engine> ${CMAKE_SOURCE_DIR}/Release/Plugins/Plugin_Flite_plus_hts_engine.so
    COMMENT "Copying Plugin_Flite_plus_hts_engine to Release/Plugins directory"
)
