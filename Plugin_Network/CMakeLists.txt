cmake_minimum_required(VERSION 3.10)

project(Plugin_Network)

set(SOURCES
    Plugin_Network.cpp
    Downloader.cpp
)

set(INCLUDE_DIRS
    ../Library_Bullet_Physics/include
    ../Library_GLFW/include
    ../Library_MMDFiles/include
    ../Library_MMDAgent/include
)

add_library(Plugin_Network SHARED ${SOURCES})

target_include_directories(Plugin_Network PRIVATE ${INCLUDE_DIRS})

target_compile_definitions(Plugin_Network PRIVATE MMDAGENT)

if(APPLE)
   target_compile_options(Plugin_Network PRIVATE -std=c++11)
endif()

target_link_libraries(Plugin_Network
    ${MMDAGENT_LINK_OPTIONS}
)

add_custom_command(
    TARGET Plugin_Network
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:Plugin_Network> ${CMAKE_SOURCE_DIR}/Release/Plugins/Plugin_Network.so
    COMMENT "Copying Plugin_Network to Release/Plugins directory"
)
