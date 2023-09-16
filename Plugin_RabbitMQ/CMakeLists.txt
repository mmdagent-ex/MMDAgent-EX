cmake_minimum_required(VERSION 3.10)

project(Plugin_RabbitMQ)

set(SOURCES
    Thread.cpp
    RabbitMQ.cpp
    Plugin_RabbitMQ.cpp
)

set(INCLUDE_DIRS
    ../Library_Bullet_Physics/include
    ../Library_GLFW/include
    ../Library_MMDFiles/include
    ../Library_MMDAgent/include
)

add_library(Plugin_RabbitMQ SHARED ${SOURCES})

target_include_directories(Plugin_RabbitMQ PRIVATE ${INCLUDE_DIRS})

target_compile_definitions(Plugin_RabbitMQ PRIVATE MMDAGENT)

target_compile_options(Plugin_RabbitMQ PRIVATE -std=c++17)

target_link_libraries(Plugin_RabbitMQ
    ${MMDAGENT_LINK_OPTIONS}
    -lrabbitmq
)

add_custom_command(
    TARGET Plugin_RabbitMQ
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:Plugin_RabbitMQ> ${CMAKE_SOURCE_DIR}/Release/Plugins/Plugin_RabbitMQ.so
    COMMENT "Copying Plugin_RabbitMQ to Release/Plugins directory"
)
