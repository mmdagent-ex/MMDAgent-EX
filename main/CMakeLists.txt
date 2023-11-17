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
#project(MMDAgent-EX)

# packages required to build this project
#find_package()

# list of source files
set(SOURCES
    main.cpp
)

# add this project as executable
add_executable(MMDAgent-EX ${SOURCES})

# private include dirs to build this project
target_include_directories(MMDAgent-EX PRIVATE
    ../Library_Bullet_Physics/include
    ../Library_GLFW/include
    ../Library_MMDFiles/include
    ../Library_MMDAgent/include
)

# compiler definitions (-D) for private build
target_compile_definitions(MMDAgent-EX PRIVATE
    MMDAGENT
)

# compiler options for private build
target_compile_options(MMDAgent-EX PRIVATE
    -Wno-deprecated-declarations
)

# link libraries to build executable
target_link_libraries(MMDAgent-EX ${MMDAGENT_LINK_OPTIONS})

# custom command to copy built binaries
add_custom_command(
   TARGET MMDAgent-EX
   POST_BUILD
   COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:MMDAgent-EX> ${CMAKE_SOURCE_DIR}/Release
   COMMENT "Copying MMDAgent-EX binary to Release directory"
)
