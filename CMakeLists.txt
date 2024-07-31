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

project(MMDAgent-EX)

#if(NOT CMAKE_BUILD_TYPE)
#  set(CMAKE_BUILD_TYPE "Release")
#endif()

# deal with libraries that has no cmake config
if(APPLE)
    if(DEFINED ${HOMEBREW_PREFIX})
        list(APPEND CMAKE_PREFIX_PATH ${HOMEBREW_PREFIX})
    else()
        execute_process(
            COMMAND brew --prefix
            RESULT_VARIABLE HOMEBREW_RESULT
            OUTPUT_VARIABLE HOMEBREW_PREFIX
            OUTPUT_STRIP_TRAILING_WHITESPACE
        )
        if(${HOMEBREW_RESULT} EQUAL 0 AND EXISTS "${HOMEBREW_PREFIX}")
            list(APPEND CMAKE_PREFIX_PATH ${HOMEBREW_PREFIX})
        else()
            if(IS_DIRECTORY /opt/homebrew)
                set(HOMEBREW_PREFIX /opt/homebrew)
            else()
                set(HOMEBREW_PREFIX /usr/local)
            endif()
            list(APPEND CMAKE_PREFIX_PATH ${HOMEBREW_PREFIX})
        endif()
    endif()
    message(STATUS "Detected homebrew prefix: ${HOMEBREW_PREFIX}")
    set(MINIZIP_INCLUDE_DIR ${HOMEBREW_PREFIX}/include)
    set(MINIZIP_LIBRARIES ${HOMEBREW_PREFIX}/lib/libminizip.dylib)
    set(PORTAUDIO_INCLUDE_DIR ${HOMEBREW_PREFIX}/include)
    set(PORTAUDIO_LIBRARIES ${HOMEBREW_PREFIX}/lib/libportaudio.dylib)
    set(LIBRDKAFKA_INCLUDE_DIR ${HOMEBREW_PREFIX}/include)
    set(LIBRDKAFKA_LIBRARIES ${HOMEBREW_PREFIX}/lib/librdkafka.dylib)
    list(APPEND CMAKE_PREFIX_PATH ${HOMEBREW_PREFIX}/opt/libomp)
else()
    set(MINIZIP_LIBRARIES -lminizip)
    set(PORTAUDIO_LIBRARIES -lportaudio)
    set(LIBRDKAFKA_LIBRARIES -lrdkafka)
endif()

if(APPLE)
    # global definitions
    add_compile_definitions(
      TARGET_OS_IPHONE=0
      GL_SILENCE_DEPRECATION=1
    )
    set(MMDAGENT_LINK_OPTIONS
        MMDAGENT
        -lstdc++
        "-framework AGL"
        "-framework Cocoa"
        "-framework OpenGL"
        "-framework IOKit"
    )
else()
    set(MMDAGENT_LINK_OPTIONS
        MMDAGENT
    )
endif()

if(CMAKE_SYSTEM_PROCESSOR MATCHES "^arm" OR CMAKE_SYSTEM_PROCESSOR MATCHES "^aarch64")
    if(APPLE)
        # assume gcc is clang.  Apple M1 mac and later.
        SET(OPT_ARCH_FLAG "-mcpu=apple-m1")
    else()
        # assume gcc is gcc.  Ubuntu on arm. 
        SET(OPT_ARCH_FLAG "-march=armv8.5-a")
    endif()
elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "^i?86" OR CMAKE_SYSTEM_PROCESSOR MATCHES "^x86_64*")
    # gcc for x86_64
    SET(OPT_ARCH_FLAG "-march=native")
else()
    # could not determine target architecture, disable architecture-dependent flag
    SET(OPT_ARCH_FLAG "")
endif()

SET(CMAKE_C_FLAGS_RELEASE "-O3 -fomit-frame-pointer ${OPT_ARCH_FLAG} -ffast-math")
SET(CMAKE_CXX_FLAGS_RELEASE "-O3 -fomit-frame-pointer ${OPT_ARCH_FLAG} -ffast-math")
SET(CMAKE_POSITION_INDEPENDENT_CODE "On")

add_subdirectory(Library_Bullet_Physics)
add_subdirectory(Library_Flite_plus_hts_engine)
add_subdirectory(Library_FreeType)
add_subdirectory(Library_GLFW)
add_subdirectory(Library_libpng)
add_subdirectory(Library_hts_engine_API)
#add_subdirectory(Library_librdkafka)
add_subdirectory(Library_Open_JTalk)
add_subdirectory(Library_Julius)
add_subdirectory(Library_MMDFiles)
add_subdirectory(Library_MMDAgent)
add_subdirectory(main)
add_subdirectory(Plugin_VIManager)
add_subdirectory(Plugin_Audio)
add_subdirectory(Plugin_Variables)
add_subdirectory(Plugin_TextArea)
add_subdirectory(Plugin_LookAt)
add_subdirectory(Plugin_Flite_plus_hts_engine)
add_subdirectory(Plugin_Julius)
add_subdirectory(Plugin_Kafka)
add_subdirectory(Plugin_Network)
add_subdirectory(Plugin_Open_JTalk)
add_subdirectory(Plugin_Remote)
add_subdirectory(Plugin_RabbitMQ)
add_subdirectory(Plugin_AnyScript)
