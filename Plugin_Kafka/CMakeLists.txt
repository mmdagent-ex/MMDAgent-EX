cmake_minimum_required(VERSION 3.10)

project(Plugin_Kafka)

set(SOURCES
    Kafka.cpp
    Thread.cpp
    Plugin_Kafka.cpp
)

set(INCLUDE_DIRS
    ../Library_Bullet_Physics/include
    ../Library_GLFW/include
    ../Library_MMDFiles/include
    ../Library_MMDAgent/include
    ../Library_librdkafka/include
)

add_library(Plugin_Kafka SHARED ${SOURCES})

target_include_directories(Plugin_Kafka PRIVATE ${INCLUDE_DIRS})

target_compile_definitions(Plugin_Kafka PRIVATE MMDAGENT HAVE_SOCKLEN_T)

target_link_libraries(Plugin_Kafka
    ${MMDAGENT_LINK_OPTIONS}
    LIBRDKAFKA
)

add_custom_command(
    TARGET Plugin_Kafka
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:Plugin_Kafka> ${CMAKE_SOURCE_DIR}/Release/Plugins/Plugin_Kafka.so
    COMMENT "Copying Plugin_Kafka to Release/Plugins directory"
)
