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
    if(IS_DIRECTORY /opt/homebrew)
        set(MINIZIP_INCLUDE_DIR /opt/homebrew/include)
        set(MINIZIP_LIBRARIES /opt/homebrew/lib/libminizip.dylib)
        set(PORTAUDIO_INCLUDE_DIR /opt/homebrew/include)
        set(PORTAUDIO_LIBRARIES /opt/homebrew/lib/libportaudio.dylib)
        set(LIBRDKAFKA_INCLUDE_DIR /opt/homebrew/include)
        set(LIBRDKAFKA_LIBRARIES /opt/homebrew/lib/librdkafka.dylib)
    else()
        if(IS_DIRECTORY /usr/local)
            set(MINIZIP_INCLUDE_DIR /usr/local/include)
            set(MINIZIP_LIBRARIES /usr/local/lib/libminizip.dylib)
            set(PORTAUDIO_INCLUDE_DIR /usr/local/include)
            set(PORTAUDIO_LIBRARIES /usr/local/lib/libportaudio.dylib)
            set(LIBRDKAFKA_INCLUDE_DIR /usr/local/include)
            set(LIBRDKAFKA_LIBRARIES /usr/local/lib/librdkafka.dylib)
            endif()
    endif()
else()
    set(MINIZIP_LIBRARIES -lminizip)
    set(PORTAUDIO_LIBRARIES -lportaudio)
    set(LIBRDKAFKA_LIBRARIES -lrdkafka)
endif()

if(APPLE)
    # brew path of Apple Silicon Mac
    list(APPEND CMAKE_PREFIX_PATH "/opt/homebrew")
    # brew path of Intel Mac
    list(APPEND CMAKE_PREFIX_PATH "/usr/local")
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

SET(CMAKE_C_FLAGS_RELEASE "-O3 -fomit-frame-pointer -march=native -ffast-math")
SET(CMAKE_CXX_FLAGS_RELEASE "-O3 -fomit-frame-pointer -march=native -ffast-math")
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
