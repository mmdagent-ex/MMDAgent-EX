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

project(HTSENGINEAPI)

set(LIB_SOURCES
    src/lib/HTS_audio.c
    src/lib/HTS_engine.c
    src/lib/HTS_gstream.c
    src/lib/HTS_label.c
    src/lib/HTS_misc.c
    src/lib/HTS_model.c
    src/lib/HTS_pstream.c
    src/lib/HTS_sstream.c
    src/lib/HTS_vocoder.c
)

set(INCLUDE_DIRS
    ../Library_PortAudio/include
    src/include
)

add_library(HTSENGINEAPI STATIC ${LIB_SOURCES})

target_include_directories(HTSENGINEAPI PRIVATE ${INCLUDE_DIRS})

target_compile_definitions(HTSENGINEAPI PUBLIC MMDAGENT AUDIO_PLAY_PORTAUDIO)

target_compile_options(HTSENGINEAPI PRIVATE
   -Wno-tautological-compare
   -Wno-empty-body
)

target_link_libraries(HTSENGINEAPI PUBLIC -lportaudio)
