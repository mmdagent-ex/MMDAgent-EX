cmake_minimum_required(VERSION 3.10)

project(Plugin_LookAt)

set(SOURCES
    Plugin_LookAt.cpp
)

set(INCLUDE_DIRS
    ../Library_Bullet_Physics/include
    ../Library_GLFW/include
    ../Library_MMDFiles/include
    ../Library_MMDAgent/include
)

add_library(Plugin_LookAt SHARED ${SOURCES})

target_include_directories(Plugin_LookAt PRIVATE ${INCLUDE_DIRS})

target_compile_definitions(Plugin_LookAt PRIVATE MMDAGENT)

target_link_libraries(Plugin_LookAt ${MMDAGENT_LINK_OPTIONS})

add_custom_command(
    TARGET Plugin_LookAt
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:Plugin_LookAt> ${CMAKE_SOURCE_DIR}/Release/Plugins/Plugin_LookAt.so
    COMMENT "Copying Plugin_LookAt to Release/Plugins directory"
)
