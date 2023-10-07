cmake_minimum_required(VERSION 3.10)

project(Plugin_Julius)

set(SOURCES
    Julius_Logger.cpp
    Julius_Thread.cpp
    Julius_Record.cpp
    Plugin_Julius.cpp
)

set(INCLUDE_DIRS
    ../Library_Bullet_Physics/include
    ../Library_GLFW/include
    ../Library_MMDFiles/include
    ../Library_MMDAgent/include
    ../Library_Julius/include
)

add_library(Plugin_Julius SHARED ${SOURCES})

target_include_directories(Plugin_Julius PRIVATE ${INCLUDE_DIRS})

target_compile_definitions(Plugin_Julius PRIVATE MMDAGENT HAVE_STRCASECMP)

target_link_libraries(Plugin_Julius
    ${MMDAGENT_LINK_OPTIONS}
    JULIUS
)

add_custom_command(
    TARGET Plugin_Julius
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:Plugin_Julius> ${CMAKE_SOURCE_DIR}/Release/Plugins/Plugin_Julius.so
    COMMENT "Copying Plugin_Julius to Release/Plugins directory"
)
