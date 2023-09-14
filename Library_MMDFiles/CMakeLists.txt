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
    target_link_libraries(MMDFILES PUBLIC BulletPhysics GLFW LIBPNG -ljpeg -lminizip -lgomp)
endif()

