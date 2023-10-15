#
#  Copyright 2022-2023  Nagoya Institute of Technology
#
#  Licensed under the Apache License, Version 2.0 (the "License");
#  you may not use this file except in compliance with the License.
#  You may obtain a copy of the License at
#
#    http://www.apache.org/licenses/LICENSE-2.0
#
#  Unless required by applicable law or agreed to in writing, software
#  distributed under the License is distributed on an "AS IS" BASIS,
#  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#  See the License for the specific language governing permissions and
#  limitations under the License.
#

cmake_minimum_required(VERSION 3.10)

project(MMDFILES)

set(LIB_SOURCES
    src/lib/BulletPhysics.cpp
    src/lib/CameraController.cpp
    src/lib/KinematicMotionState.cpp
    src/lib/MMDFiles_utils.cpp
    src/lib/MotionController.cpp
    src/lib/MotionManager.cpp
    src/lib/PMDBone.cpp
    src/lib/PMDBoneMorph.cpp
    src/lib/PMDConstraint.cpp
    src/lib/PMDFace.cpp
    src/lib/PMDGroupMorph.cpp
    src/lib/PMDIK.cpp
    src/lib/PMDMaterial.cpp
    src/lib/PMDMaterialMorph.cpp
    src/lib/PMDModel.cpp
    src/lib/PMDModel_parse.cpp
    src/lib/PMDModel_render.cpp
    src/lib/PMDModel_update.cpp
    src/lib/PMDRigidBody.cpp
    src/lib/PMDTexture.cpp
    src/lib/PMDTextureLoader.cpp
    src/lib/PMDUVMorph.cpp
    src/lib/PMDVertexMorph.cpp
    src/lib/PTree.cpp
    src/lib/SystemTexture.cpp
    src/lib/VMD.cpp
    src/lib/ZFile.cpp
)

set(INCLUDE_DIRS
    ../Library_Bullet_Physics/include
    ../Library_libpng/include
    src/include
)

set(PRIVATE_COMPILER_OPTIONS
    -DGL_GLEXT_PROTOTYPES
    -DMMDFILES_DONTUPDATEMATRICESFORIK
    -DMMDFILES_DONTSORTORDERFORALPHARENDERING
)

add_library(MMDFILES STATIC ${LIB_SOURCES})

target_include_directories(MMDFILES BEFORE PRIVATE ${INCLUDE_DIRS})

target_compile_definitions(MMDFILES PRIVATE GL_GLEXT_PROTOTYPES MMDFILES_DONTUPDATEMATRICESFORIK MMDFILES_DONTSORTORDERFORALPHARENDERING)

if(APPLE)
    target_compile_options(MMDFILES PRIVATE -Xpreprocessor -fopenmp)
    target_link_libraries(MMDFILES PUBLIC BulletPhysics GLFW LIBPNG -ljpeg -lminizip -lomp)
else()
    target_compile_options(MMDFILES PRIVATE -fopenmp)
    target_link_libraries(MMDFILES PUBLIC BulletPhysics GLFW LIBPNG -ljpeg -lminizip -lgomp -ldl -lpthread)
endif()

