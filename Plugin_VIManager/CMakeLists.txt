cmake_minimum_required(VERSION 3.10)

project(Plugin_VIManager)

set(SOURCES
    Plugin_VIManager.cpp
    VIManager.cpp
    VIManager_Logger.cpp
    VIManager_Thread.cpp
)

set(INCLUDE_DIRS
    ../Library_Bullet_Physics/include
    ../Library_GLFW/include
    ../Library_MMDFiles/include
    ../Library_MMDAgent/include
)

add_library(Plugin_VIManager SHARED ${SOURCES})

target_include_directories(Plugin_VIManager PRIVATE ${INCLUDE_DIRS})

target_compile_definitions(Plugin_VIManager PRIVATE MMDAGENT)

target_compile_options(Plugin_VIManager PRIVATE -std=c++17)

target_link_libraries(Plugin_VIManager ${MMDAGENT_LINK_OPTIONS} -lre2)

add_custom_command(
    TARGET Plugin_VIManager
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:Plugin_VIManager> ${CMAKE_SOURCE_DIR}/Release/Plugins/Plugin_VIManager.so
    COMMENT "Copying Plugin_VIManager to Release/Plugins directory"
)
