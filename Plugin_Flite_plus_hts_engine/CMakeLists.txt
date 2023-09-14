cmake_minimum_required(VERSION 3.10)

project(Plugin_Flite_plus_hts_engine)

set(SOURCES
    Flite_plus_hts_engine.cpp
    Flite_plus_hts_engine_Manager.cpp
    Flite_plus_hts_engine_Thread.cpp
    Plugin_Flite_plus_hts_engine.cpp
)

set(INCLUDE_DIRS
    ../Library_Bullet_Physics/include
    ../Library_GLFW/include
    ../Library_MMDFiles/include
    ../Library_MMDAgent/include
    ../Library_hts_engine_API/include
    ../Library_Flite_plus_hts_engine/include
)

add_library(Plugin_Flite_plus_hts_engine SHARED ${SOURCES})

target_include_directories(Plugin_Flite_plus_hts_engine PRIVATE ${INCLUDE_DIRS})

target_compile_definitions(Plugin_Flite_plus_hts_engine PRIVATE MMDAGENT)

target_link_libraries(Plugin_Flite_plus_hts_engine
    ${MMDAGENT_LINK_OPTIONS}
    HTSENGINEAPI
)

add_custom_command(
    TARGET Plugin_Flite_plus_hts_engine
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:Plugin_Flite_plus_hts_engine> ${CMAKE_SOURCE_DIR}/Release/Plugins/Plugin_Flite_plus_hts_engine.so
    COMMENT "Copying Plugin_Flite_plus_hts_engine to Release/Plugins directory"
)
