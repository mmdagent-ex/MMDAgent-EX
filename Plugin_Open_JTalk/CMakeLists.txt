cmake_minimum_required(VERSION 3.10)

project(Plugin_Open_JTalk)

set(SOURCES
    Open_JTalk.cpp
    Open_JTalk_Manager.cpp
    Open_JTalk_Thread.cpp
    Plugin_Open_JTalk.cpp
)

set(INCLUDE_DIRS
    ../Library_Bullet_Physics/include
    ../Library_GLFW/include
    ../Library_MMDFiles/include
    ../Library_MMDAgent/include
    ../Library_hts_engine_API/include
    ../Library_Open_JTalk/include
)

add_library(Plugin_Open_JTalk SHARED ${SOURCES})

target_include_directories(Plugin_Open_JTalk PRIVATE ${INCLUDE_DIRS})

target_link_libraries(Plugin_Open_JTalk
    ${MMDAGENT_LINK_OPTIONS}
    HTSENGINEAPI
    OPEN_JTALK
)

add_custom_command(
    TARGET Plugin_Open_JTalk
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:Plugin_Open_JTalk> ${CMAKE_SOURCE_DIR}/Release/Plugins/Plugin_Open_JTalk.so
    COMMENT "Copying Plugin_Open_JTalk to Release/Plugins directory"
)
