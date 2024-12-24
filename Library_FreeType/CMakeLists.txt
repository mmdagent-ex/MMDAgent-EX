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
project(FreeType)

# packages required to build this project
#find_package()

# list of source files
set(SOURCES
    src/src/autofit/autofit.c
    src/src/base/ftbase.c
    src/src/base/ftbbox.c
    src/src/base/ftbdf.c
    src/src/base/ftbitmap.c
    src/src/base/ftcid.c
    src/src/base/ftdebug.c
    src/src/base/ftfstype.c
    src/src/base/ftgasp.c
    src/src/base/ftglyph.c
    src/src/base/ftgxval.c
    src/src/base/ftinit.c
    src/src/base/ftlcdfil.c
    src/src/base/ftmm.c
    src/src/base/ftotval.c
    src/src/base/ftpatent.c
    src/src/base/ftpfr.c
    src/src/base/ftstroke.c
    src/src/base/ftsynth.c
    src/src/base/ftsystem.c
    src/src/base/fttype1.c
    src/src/base/ftwinfnt.c
    src/src/base/ftxf86.c
    src/src/bdf/bdf.c
    src/src/cache/ftcache.c
    src/src/cff/cff.c
    src/src/cid/type1cid.c
    src/src/gxvalid/gxvalid.c
    src/src/gzip/ftgzip.c
    src/src/lzw/ftlzw.c
    src/src/otvalid/otvalid.c
    src/src/pcf/pcf.c
    src/src/pfr/pfr.c
    src/src/psaux/psaux.c
    src/src/pshinter/pshinter.c
    src/src/psnames/psnames.c
    src/src/raster/raster.c
    src/src/sfnt/sfnt.c
    src/src/smooth/smooth.c
    src/src/truetype/truetype.c
    src/src/type1/type1.c
    src/src/type42/type42.c
    src/src/winfonts/winfnt.c
)

# add this project as static library
add_library(FreeType STATIC ${SOURCES})

# private include dirs to build this project
target_include_directories(FreeType PRIVATE
    src/include
)


# compiler definitions (-D) for private build
if(APPLE)
    # workaround for build on macOS 15.2
    target_compile_definitions(FreeType PRIVATE
        FT2_BUILD_LIBRARY
        FT_CONFIG_OPTION_SYSTEM_ZLIB
    )
else()
    target_compile_definitions(FreeType PRIVATE
        FT2_BUILD_LIBRARY
    )
endif()

# compiler options for private build
#target_compile_options(FreeType PRIVATE
#    -Wno-deprecated-declarations
#)

##############
# public compiler definitions (-D) required to link with this project
#target_compile_definitions(FreeType PUBLIC
#)

# public compiler options required to link with this project
#target_compile_options(FreeType PUBLIC
#)

# public link libraries to be linked with this project
#target_link_libraries(FreeType PUBLIC
#)
