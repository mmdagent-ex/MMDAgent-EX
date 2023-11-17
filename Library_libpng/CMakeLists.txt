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
project(LIBPNG)

# packages required to build this project
#find_package()

# list of source files
set(SOURCES
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
    list(APPEND SOURCES
        src/arm/arm_init.c
        src/arm/filter_neon.S
        src/arm/filter_neon_intrinsics.c
        src/arm/palette_neon_intrinsics.c
    )
endif()

# add this project as static library
add_library(LIBPNG STATIC ${SOURCES})

# private include dirs to build this project
target_include_directories(LIBPNG PRIVATE
    src
    include
)

# compiler definitions (-D) for private build
#target_compile_definitions(LIBPNG PRIVATE
#    XXXX=1
#    XXXX
#)

# compiler options for private build
#target_compile_options(LIBPNG PRIVATE
#    -Wno-deprecated-declarations
#)

##############
# public compiler definitions (-D) required to link with this project
#target_compile_definitions(LIBPNG PUBLIC
#)

# public compiler options required to link with this project
#target_compile_options(LIBPNG PUBLIC
#)

# public link libraries to be linked with this project
#target_link_libraries(LIBPNG PUBLIC
#)
