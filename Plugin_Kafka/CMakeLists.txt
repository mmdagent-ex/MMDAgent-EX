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
project(Plugin_Kafka)

# packages required to build this project
#find_package()

# list of source files
set(SOURCES
    Kafka.cpp
    Thread.cpp
    Plugin_Kafka.cpp
)

# add this project as shared library
add_library(Plugin_Kafka SHARED ${SOURCES})

# nclude dirs to build this project
target_include_directories(Plugin_Kafka PRIVATE
    ../Library_Bullet_Physics/include
    ../Library_GLFW/include
    ../Library_MMDFiles/include
    ../Library_MMDAgent/include
    ../Library_librdkafka/include
    ${LIBRDKAFKA_INCLUDE_DIR}
)

# compiler definitions (-D) for build
target_compile_definitions(Plugin_Kafka PRIVATE
    MMDAGENT
    HAVE_SOCKLEN_T
)

# compiler options for build
#target_compile_options(MYPROJECT PRIVATE
#)

# library linking
target_link_libraries(Plugin_Kafka
    ${MMDAGENT_LINK_OPTIONS}
    ${LIBRDKAFKA_LIBRARIES}
)

# custom command to copy binaries
add_custom_command(
    TARGET Plugin_Kafka
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:Plugin_Kafka> ${CMAKE_SOURCE_DIR}/Release/Plugins/Plugin_Kafka.so
    COMMENT "Copying Plugin_Kafka to Release/Plugins directory"
)
