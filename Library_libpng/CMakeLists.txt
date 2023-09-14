cmake_minimum_required(VERSION 3.10)

project(LIBPNG)

set(LIB_SOURCES
    src/png.c
    src/pngerror.c
    src/pngget.c
    src/pngmem.c
    src/pngpread.c
    src/pngread.c
    src/pngrio.c
    src/pngrtran.c
    src/pngrutil.c
    src/pngset.c
    src/pngtrans.c
    src/pngwio.c
    src/pngwrite.c
    src/pngwtran.c
    src/pngwutil.c
)
if(APPLE)
    list(APPEND LIB_SOURCES
        src/arm/arm_init.c
        src/arm/filter_neon.S
        src/arm/filter_neon_intrinsics.c
        src/arm/palette_neon_intrinsics.c
    )
endif()

set(INCLUDE_DIRS
    src
    include
)

add_library(LIBPNG STATIC ${LIB_SOURCES})

target_include_directories(LIBPNG PRIVATE ${INCLUDE_DIRS})

target_link_libraries(LIBPNG PUBLIC -lz)

