cmake_minimum_required(VERSION 3.10)

project(Plugin_AnyScript)

set(SOURCES
    ChildProcess.cpp
    Plugin_AnyScript.cpp
)

set(INCLUDE_DIRS
    ../Library_Bullet_Physics/include
    ../Library_GLFW/include
    ../Library_MMDFiles/include
    ../Library_MMDAgent/include
)

add_library(Plugin_AnyScript SHARED ${SOURCES})

target_include_directories(Plugin_AnyScript PRIVATE ${INCLUDE_DIRS})

target_compile_definitions(Plugin_AnyScript PRIVATE MMDAGENT)

target_link_libraries(Plugin_AnyScript ${MMDAGENT_LINK_OPTIONS})

add_custom_command(
    TARGET Plugin_AnyScript
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:Plugin_AnyScript> ${CMAKE_SOURCE_DIR}/Release/Plugins/Plugin_AnyScript.so
    COMMENT "Copying Plugin_AnyScript to Release/Plugins directory"
)
