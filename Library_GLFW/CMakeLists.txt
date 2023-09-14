cmake_minimum_required(VERSION 3.10)

project(GLFW)

set(COMPILER_DEFS
    MMDAGENT
    _GLFW_USE_LINUX_JOYSTICKS
    _GLFW_HAS_XRANDR
    _GLFW_HAS_PTHREAD
    _GLFW_HAS_SCHED_YIELD
    _GLFW_HAS_GLXGETPROCADDRESS
    _GLFW_HAS_SYSCONF
)

set(LIB_SOURCES
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
    list(APPEND LIB_SOURCES
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
    list(APPEND COMPILER_DEFS
        TARGET_OS_IPHONE=0
    )
else()
    list(APPEND LIB_SOURCES
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

set(PRIVATE_COMPILER_OPTIONS
    -pthread
    -Wno-deprecated-declarations
)

add_library(GLFW STATIC ${LIB_SOURCES})

target_include_directories(GLFW PRIVATE ${INCLUDE_DIRS})

target_compile_options(GLFW PRIVATE ${PRIVATE_COMPILER_OPTIONS})

target_compile_definitions(GLFW PRIVATE ${COMPILER_DEFS})

if(APPLE)
else()
    target_link_libraries(GLFW PUBLIC -lXrandr -lX11 -lGL -lGLU)
endif()
