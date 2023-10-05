cmake_minimum_required(VERSION 3.10)

project(Plugin_Remote)

find_package(OpenCV REQUIRED)

set(SOURCES
    Plugin_Remote.cpp
    Avatar.cpp
    Thread.cpp
    ServerClient.cpp
    MouseQueue.cpp
    MorphControl.cpp
    Julius_Thread.cpp
    Julius_Record.cpp
    Speak.cpp
)

set(INCLUDE_DIRS
    ../Library_Bullet_Physics/include
    ../Library_GLFW/include
    ../Library_MMDFiles/include
    ../Library_MMDAgent/include
    ../Library_Julius/include
    ${OpenCV_INCLUDE_DIRS}
)

add_library(Plugin_Remote SHARED ${SOURCES})

target_include_directories(Plugin_Remote PRIVATE ${INCLUDE_DIRS})

target_compile_definitions(Plugin_Remote PRIVATE MMDAGENT HAVE_SOCKLEN_T)

target_compile_options(Plugin_Remote PRIVATE -std=c++17)

target_link_libraries(Plugin_Remote
    ${MMDAGENT_LINK_OPTIONS}
    JULIUS
    ${OpenCV_LIBRARIES}
    -lsndfile -lsamplerate
    -lavcodec -lavutil -lswscale
)

add_custom_command(
    TARGET Plugin_Remote
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:Plugin_Remote> ${CMAKE_SOURCE_DIR}/Release/Plugins/Plugin_Remote.so
    COMMENT "Copying Plugin_Remote to Release/Plugins directory"
)
