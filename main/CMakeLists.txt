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

#project(MMDAgent-EX)

set(SOURCES
    main.cpp
)

set(INCLUDE_DIRS
    ../Library_Bullet_Physics/include
    ../Library_GLFW/include
    ../Library_MMDFiles/include
    ../Library_MMDAgent/include
)

add_executable(MMDAgent-EX ${SOURCES})

target_compile_definitions(MMDAgent-EX PRIVATE MMDAGENT)

target_include_directories(MMDAgent-EX PRIVATE ${INCLUDE_DIRS})

target_link_libraries(MMDAgent-EX ${MMDAGENT_LINK_OPTIONS})

add_custom_command(
   TARGET MMDAgent-EX
   POST_BUILD
   COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:MMDAgent-EX> ${CMAKE_SOURCE_DIR}/Release
   COMMENT "Copying MMDAgent-EX binary to Release directory"
)
