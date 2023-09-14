cmake_minimum_required(VERSION 3.10)

project(Plugin_Audio)

set(SOURCES
    Audio_Manager.cpp
    Audio_Thread.cpp
    Plugin_Audio.cpp
)

set(INCLUDE_DIRS
    ../Library_Bullet_Physics/include
    ../Library_GLFW/include
    ../Library_MMDFiles/include
    ../Library_MMDAgent/include
)

add_library(Plugin_Audio SHARED ${SOURCES})

target_include_directories(Plugin_Audio PRIVATE ${INCLUDE_DIRS})

target_compile_definitions(Plugin_Audio PRIVATE MMDAGENT)

if(APPLE)
    target_link_libraries(Plugin_Audio
        ${MMDAGENT_LINK_OPTIONS}
        "-framework AudioUnit"
        "-framework AudioToolbox"
        "-framework CoreAudio"
    )
else()
    target_link_libraries(Plugin_Audio
	${MMDAGENT_LINK_OPTIONS}
    )
endif()

add_custom_command(
    TARGET Plugin_Audio
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:Plugin_Audio> ${CMAKE_SOURCE_DIR}/Release/Plugins/Plugin_Audio.so
    COMMENT "Copying Plugin_Audio to Release/Plugins directory"
)
