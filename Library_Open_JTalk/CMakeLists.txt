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
project(OPEN_JTALK)

# packages required to build this project
#find_package(portaudio)

# list of source files
set(SOURCES
    src/jpcommon/jpcommon.c
    src/jpcommon/jpcommon_label.c
    src/jpcommon/jpcommon_node.c
    src/mecab2njd/mecab2njd.c
    src/njd/njd.c
    src/njd/njd_node.c
    src/njd2jpcommon/njd2jpcommon.c
    src/njd_set_accent_phrase/njd_set_accent_phrase.c
    src/njd_set_accent_type/njd_set_accent_type.c
    src/njd_set_digit/njd_set_digit.c
    src/njd_set_long_vowel/njd_set_long_vowel.c
    src/njd_set_pronunciation/njd_set_pronunciation.c
    src/njd_set_unvoiced_vowel/njd_set_unvoiced_vowel.c
    src/text2mecab/text2mecab.c
    src/mecab/src/char_property.cpp
    src/mecab/src/connector.cpp
    src/mecab/src/context_id.cpp
    src/mecab/src/dictionary.cpp
    src/mecab/src/dictionary_compiler.cpp
    src/mecab/src/dictionary_generator.cpp
    src/mecab/src/dictionary_rewriter.cpp
    src/mecab/src/eval.cpp
    src/mecab/src/feature_index.cpp
    src/mecab/src/iconv_utils.cpp
    src/mecab/src/lbfgs.cpp
    src/mecab/src/learner.cpp
    src/mecab/src/learner_tagger.cpp
    src/mecab/src/libmecab.cpp
    src/mecab/src/mecab.cpp
    src/mecab/src/nbest_generator.cpp
    src/mecab/src/param.cpp
    src/mecab/src/string_buffer.cpp
    src/mecab/src/tagger.cpp
    src/mecab/src/tokenizer.cpp
    src/mecab/src/utils.cpp
    src/mecab/src/viterbi.cpp
    src/mecab/src/writer.cpp
)

# add this project as static library
add_library(OPEN_JTALK STATIC ${SOURCES})

# private include dirs to build this project
target_include_directories(OPEN_JTALK PRIVATE
    src/jpcommon
    src/mecab/src
    src/mecab2njd
    src/njd
    src/njd2jpcommon
    src/njd_set_accent_phrase
    src/njd_set_accent_type
    src/njd_set_digit
    src/njd_set_long_vowel
    src/njd_set_pronunciation
    src/njd_set_unvoiced_vowel
    src/text2mecab
    ${PORTAUDIO_INCLUDE_DIR}
)

# compiler definitions (-D) for private build
target_compile_definitions(OPEN_JTALK PRIVATE
    DIC_VERSION=102
    MECAB_DEFAULT_RC="dummy"
    PACKAGE="open_jtalk"
    VERSION="1.09"
    MECAB_WITHOUT_SHARE_DIC
    CHARSET_UTF_8
    HAVE_CTYPE_H=1
    HAVE_DIRENT_H=1
    HAVE_FCNTL_H=1
    HAVE_GETENV=1
    HAVE_GETPAGESIZE=1
    HAVE_INTTYPES_H=1
    HAVE_LIBM=1
    HAVE_MEMORY_H=1
    HAVE_MMAP=1
    HAVE_OPENDIR=1
    HAVE_SETJMP=1
    HAVE_SETJMP_H=1
    HAVE_SQRT=1
    HAVE_STDINT_H=1
    HAVE_STDLIB_H=1
    HAVE_STRINGS_H=1
    HAVE_STRING_H=1
    HAVE_STRSTR=1
    HAVE_SYS_MMAN_H=1
    HAVE_SYS_STAT_H=1
    HAVE_SYS_TIMES_H=1
    HAVE_SYS_TYPES_H=1
    HAVE_UNISTD_H=1
    PACKAGE_BUGREPORT="open-jtalk-users@lists.sourceforge.net"
    PACKAGE_NAME="open_jtalk"
    PACKAGE_STRING="open_jtalk 1.09"
    PACKAGE_TARNAME="open_jtalk"
    PACKAGE_VERSION="1.09"
    SIZEOF_CHAR=1
    SIZEOF_INT=4
    SIZEOF_LONG=4
    SIZEOF_LONG_LONG=8
    SIZEOF_SHORT=2
    SIZEOF_SIZE_T=4
    STDC_HEADERS=1
    MECAB_DEFAULT_CHARSET="UTF-8"
    ASCII_HEADER=1
)

# compiler options for private build
target_compile_options(OPEN_JTALK PRIVATE
    -Wno-deprecated-declarations
)

if(APPLE)
else()
    # set only for C++ compiler
    set_target_properties(OPEN_JTALK PROPERTIES
        COMPILE_OPTIONS "$<$<COMPILE_LANGUAGE:CXX>:-Wno-register>"
    )
endif()

##############
# public compiler definitions (-D) required to link with this project
#target_compile_definitions(OPEN_JTALK PUBLIC
#)

# public compiler options required to link with this project
#target_compile_options(OPEN_JTALK PUBLIC
#)

# public link libraries to be linked with this project
target_link_libraries(OPEN_JTALK PUBLIC
    HTSENGINEAPI
    ${PORTAUDIO_LIBRARIES}
)
