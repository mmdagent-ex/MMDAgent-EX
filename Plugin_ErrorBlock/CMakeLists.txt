cmake_minimum_required(VERSION 3.10)

project(Plugin_ErrorBlock)

set(SOURCES
    Plugin_ErrorBlock.cpp
)

set(INCLUDE_DIRS
    ../Library_Bullet_Physics/include
    ../Library_GLFW/include
    ../Library_MMDFiles/include
    ../Library_MMDAgent/include
)

add_library(Plugin_ErrorBlock SHARED ${SOURCES})

target_include_directories(Plugin_ErrorBlock PRIVATE ${INCLUDE_DIRS})

target_compile_definitions(Plugin_ErrorBlock PRIVATE MMDAGENT)

target_link_libraries(Plugin_ErrorBlock ${MMDAGENT_LINK_OPTIONS})

add_custom_command(
    TARGET Plugin_ErrorBlock
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:Plugin_ErrorBlock> ${CMAKE_SOURCE_DIR}/Release/Plugins/Plugin_ErrorBlock.so
    COMMENT "Copying Plugin_ErrorBlock to Release/Plugins directory"
)
