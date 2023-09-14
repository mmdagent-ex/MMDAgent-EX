cmake_minimum_required(VERSION 3.10)

project(Plugin_Variables)

set(SOURCES
    CountDown_Thread.cpp
    Variables.cpp
    Plugin_Variables.cpp
)

set(INCLUDE_DIRS
    ../Library_Bullet_Physics/include
    ../Library_GLFW/include
    ../Library_MMDFiles/include
    ../Library_MMDAgent/include
)

add_library(Plugin_Variables SHARED ${SOURCES})

target_include_directories(Plugin_Variables PRIVATE ${INCLUDE_DIRS})

target_compile_definitions(Plugin_Variables PRIVATE MMDAGENT)

target_link_libraries(Plugin_Variables ${MMDAGENT_LINK_OPTIONS})

add_custom_command(
    TARGET Plugin_Variables
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:Plugin_Variables> ${CMAKE_SOURCE_DIR}/Release/Plugins/Plugin_Variables.so
    COMMENT "Copying Plugin_Variables to Release/Plugins directory"
)
