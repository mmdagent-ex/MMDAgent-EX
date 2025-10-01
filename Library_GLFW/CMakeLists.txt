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

# tell system to prefer GLVND than legacy OpenGL library
if(POLICY CMP0072)
  cmake_policy(SET CMP0072 NEW)
endif()

# project name
project(GLFW)

# packages required to build this project
find_package(OpenGL REQUIRED)

# list of source files
set(SOURCES
    src/lib/enable.c
    src/lib/fullscreen.c
    src/lib/glext.c
    src/lib/image.c
    src/lib/init.c
    src/lib/input.c
    src/lib/joystick.c
    src/lib/stream.c
    src/lib/tga.c
    src/lib/thread.c
    src/lib/time.c
    src/lib/window.c
)

set(INCLUDE_DIRS
    src/include
    src/lib
)

if(APPLE)
    list(APPEND SOURCES
        src/lib/cocoa/cocoa_thread.c
        src/lib/cocoa/cocoa_enable.m
        src/lib/cocoa/cocoa_fullscreen.m
        src/lib/cocoa/cocoa_glext.m
        src/lib/cocoa/cocoa_joystick.m
        src/lib/cocoa/cocoa_time.m
        src/lib/cocoa/cocoa_init.m
        src/lib/cocoa/cocoa_window.m
    )
    list(APPEND INCLUDE_DIRS
        src/lib/cocoa
    )
else()
    list(APPEND SOURCES
        src/lib/x11/x11_enable.c
        src/lib/x11/x11_fullscreen.c
        src/lib/x11/x11_glext.c
        src/lib/x11/x11_init.c
        src/lib/x11/x11_joystick.c
        src/lib/x11/x11_keysym2unicode.c
        src/lib/x11/x11_thread.c
        src/lib/x11/x11_time.c
        src/lib/x11/x11_window.c
    )
    list(APPEND INCLUDE_DIRS
        src/lib/x11
    )
endif()

# add this project as static library
add_library(GLFW STATIC ${SOURCES})

# private include dirs to build this project
target_include_directories(GLFW PRIVATE
    ${INCLUDE_DIRS}
)

# compiler definitions (-D) for private build
target_compile_definitions(GLFW PRIVATE
    MMDAGENT
    _GLFW_USE_LINUX_JOYSTICKS
    _GLFW_HAS_XRANDR
    _GLFW_HAS_PTHREAD
    _GLFW_HAS_SCHED_YIELD
    _GLFW_HAS_GLXGETPROCADDRESS
    _GLFW_HAS_SYSCONF
)

# compiler options for private build
target_compile_options(GLFW PRIVATE
    -pthread
    -Wno-deprecated-declarations
)

##############
# public compiler definitions (-D) required to link with this project
target_compile_definitions(GLFW PUBLIC
)

# public compiler options required to link with this project
target_compile_options(GLFW PUBLIC
)

# public link libraries to be linked with this project
if(APPLE)
    target_link_libraries(GLFW PUBLIC
        ${OPENGL_LIBRARIES}
    )
else()
    target_link_libraries(GLFW PUBLIC
        ${OPENGL_LIBRARIES}
        -lXrandr -lX11 -lGL -lGLU
    )
endif()

