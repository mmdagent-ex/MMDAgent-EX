cmake_minimum_required(VERSION 3.10)

project(Plugin_TextArea)

find_package(OpenCV REQUIRED)

set(SOURCES
    TextArea.cpp
    Plugin_TextArea.cpp
)

set(INCLUDE_DIRS
    ../Library_Bullet_Physics/include
    ../Library_GLFW/include
    ../Library_MMDFiles/include
    ../Library_MMDAgent/include
    ${OpenCV_INCLUDE_DIRS}
)

add_library(Plugin_TextArea SHARED ${SOURCES})

target_include_directories(Plugin_TextArea PRIVATE ${INCLUDE_DIRS})

target_compile_definitions(Plugin_TextArea PRIVATE MMDAGENT)

target_compile_options(Plugin_TextArea PRIVATE -std=c++17)

target_link_libraries(Plugin_TextArea
    ${MMDAGENT_LINK_OPTIONS}
    ${OpenCV_LIBRARIES}
)

add_custom_command(
    TARGET Plugin_TextArea
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:Plugin_TextArea> ${CMAKE_SOURCE_DIR}/Release/Plugins/Plugin_TextArea.so
    COMMENT "Copying Plugin_TextArea to Release/Plugins directory"
)
