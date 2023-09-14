cmake_minimum_required(VERSION 3.10)

project(FreeType)

set(LIB_SOURCES
    src/src/autofit/autofit.c
    src/src/base/ftbase.c
    src/src/base/ftbbox.c
    src/src/base/ftbdf.c
    src/src/base/ftbitmap.c
    src/src/base/ftcid.c
    src/src/base/ftdebug.c
    src/src/base/ftfstype.c
    src/src/base/ftgasp.c
    src/src/base/ftglyph.c
    src/src/base/ftgxval.c
    src/src/base/ftinit.c
    src/src/base/ftlcdfil.c
    src/src/base/ftmm.c
    src/src/base/ftotval.c
    src/src/base/ftpatent.c
    src/src/base/ftpfr.c
    src/src/base/ftstroke.c
    src/src/base/ftsynth.c
    src/src/base/ftsystem.c
    src/src/base/fttype1.c
    src/src/base/ftwinfnt.c
    src/src/base/ftxf86.c
    src/src/bdf/bdf.c
    src/src/cache/ftcache.c
    src/src/cff/cff.c
    src/src/cid/type1cid.c
    src/src/gxvalid/gxvalid.c
    src/src/gzip/ftgzip.c
    src/src/lzw/ftlzw.c
    src/src/otvalid/otvalid.c
    src/src/pcf/pcf.c
    src/src/pfr/pfr.c
    src/src/psaux/psaux.c
    src/src/pshinter/pshinter.c
    src/src/psnames/psnames.c
    src/src/raster/raster.c
    src/src/sfnt/sfnt.c
    src/src/smooth/smooth.c
    src/src/truetype/truetype.c
    src/src/type1/type1.c
    src/src/type42/type42.c
    src/src/winfonts/winfnt.c
)

set(INCLUDE_DIRS
    src/include
)

add_library(FreeType STATIC ${LIB_SOURCES})

target_include_directories(FreeType PRIVATE ${INCLUDE_DIRS})
target_compile_definitions(FreeType PRIVATE FT2_BUILD_LIBRARY)
