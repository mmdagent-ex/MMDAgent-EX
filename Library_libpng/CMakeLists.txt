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

project(LIBPNG)

set(LIB_SOURCES
    src/png.c
    src/pngerror.c
    src/pngget.c
    src/pngmem.c
    src/pngpread.c
    src/pngread.c
    src/pngrio.c
    src/pngrtran.c
    src/pngrutil.c
    src/pngset.c
    src/pngtrans.c
    src/pngwio.c
    src/pngwrite.c
    src/pngwtran.c
    src/pngwutil.c
)
if(APPLE)
    list(APPEND LIB_SOURCES
        src/arm/arm_init.c
        src/arm/filter_neon.S
        src/arm/filter_neon_intrinsics.c
        src/arm/palette_neon_intrinsics.c
    )
endif()

set(INCLUDE_DIRS
    src
    include
)

add_library(LIBPNG STATIC ${LIB_SOURCES})

target_include_directories(LIBPNG PRIVATE ${INCLUDE_DIRS})

target_link_libraries(LIBPNG PUBLIC -lz)

