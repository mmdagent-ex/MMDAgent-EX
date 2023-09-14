cmake_minimum_required(VERSION 3.10)

project(HTSENGINEAPI)

set(LIB_SOURCES
    src/lib/HTS_audio.c
    src/lib/HTS_engine.c
    src/lib/HTS_gstream.c
    src/lib/HTS_label.c
    src/lib/HTS_misc.c
    src/lib/HTS_model.c
    src/lib/HTS_pstream.c
    src/lib/HTS_sstream.c
    src/lib/HTS_vocoder.c
)

set(INCLUDE_DIRS
    ../Library_PortAudio/include
    src/include
)

add_library(HTSENGINEAPI STATIC ${LIB_SOURCES})

target_include_directories(HTSENGINEAPI PRIVATE ${INCLUDE_DIRS})

target_compile_definitions(HTSENGINEAPI PUBLIC MMDAGENT AUDIO_PLAY_PORTAUDIO)

target_compile_options(HTSENGINEAPI PRIVATE
   -Wno-tautological-compare
   -Wno-empty-body
)

target_link_libraries(HTSENGINEAPI PUBLIC -lportaudio)
