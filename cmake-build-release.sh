#!/bin/sh
# set up build system: source code top directory = here, output should be under "build" directory
# "Debug" can be "Release"
cmake -S. -Bbuild -DCMAKE_BUILD_TYPE=Release
# do build
cmake --build build

