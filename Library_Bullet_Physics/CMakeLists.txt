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

# project name
project(BulletPhysics)

# packages required to build this project
#find_package()

# list of source files
set(SOURCES
    src/src/BulletCollision/BroadphaseCollision/btAxisSweep3.cpp
    src/src/BulletCollision/BroadphaseCollision/btBroadphaseProxy.cpp
    src/src/BulletCollision/BroadphaseCollision/btCollisionAlgorithm.cpp
    src/src/BulletCollision/BroadphaseCollision/btDbvt.cpp
    src/src/BulletCollision/BroadphaseCollision/btDbvtBroadphase.cpp
    src/src/BulletCollision/BroadphaseCollision/btDispatcher.cpp
    src/src/BulletCollision/BroadphaseCollision/btMultiSapBroadphase.cpp
    src/src/BulletCollision/BroadphaseCollision/btOverlappingPairCache.cpp
    src/src/BulletCollision/BroadphaseCollision/btQuantizedBvh.cpp
    src/src/BulletCollision/BroadphaseCollision/btSimpleBroadphase.cpp
    src/src/BulletCollision/CollisionDispatch/SphereTriangleDetector.cpp
    src/src/BulletCollision/CollisionDispatch/btActivatingCollisionAlgorithm.cpp
    src/src/BulletCollision/CollisionDispatch/btBox2dBox2dCollisionAlgorithm.cpp
    src/src/BulletCollision/CollisionDispatch/btBoxBoxCollisionAlgorithm.cpp
    src/src/BulletCollision/CollisionDispatch/btBoxBoxDetector.cpp
    src/src/BulletCollision/CollisionDispatch/btCollisionDispatcher.cpp
    src/src/BulletCollision/CollisionDispatch/btCollisionObject.cpp
    src/src/BulletCollision/CollisionDispatch/btCollisionWorld.cpp
    src/src/BulletCollision/CollisionDispatch/btCompoundCollisionAlgorithm.cpp
    src/src/BulletCollision/CollisionDispatch/btCompoundCompoundCollisionAlgorithm.cpp
    src/src/BulletCollision/CollisionDispatch/btConvex2dConvex2dAlgorithm.cpp
    src/src/BulletCollision/CollisionDispatch/btConvexConcaveCollisionAlgorithm.cpp
    src/src/BulletCollision/CollisionDispatch/btConvexConvexAlgorithm.cpp
    src/src/BulletCollision/CollisionDispatch/btConvexPlaneCollisionAlgorithm.cpp
    src/src/BulletCollision/CollisionDispatch/btDefaultCollisionConfiguration.cpp
    src/src/BulletCollision/CollisionDispatch/btEmptyCollisionAlgorithm.cpp
    src/src/BulletCollision/CollisionDispatch/btGhostObject.cpp
    src/src/BulletCollision/CollisionDispatch/btHashedSimplePairCache.cpp
    src/src/BulletCollision/CollisionDispatch/btInternalEdgeUtility.cpp
    src/src/BulletCollision/CollisionDispatch/btManifoldResult.cpp
    src/src/BulletCollision/CollisionDispatch/btSimulationIslandManager.cpp
    src/src/BulletCollision/CollisionDispatch/btSphereBoxCollisionAlgorithm.cpp
    src/src/BulletCollision/CollisionDispatch/btSphereSphereCollisionAlgorithm.cpp
    src/src/BulletCollision/CollisionDispatch/btSphereTriangleCollisionAlgorithm.cpp
    src/src/BulletCollision/CollisionDispatch/btUnionFind.cpp
    src/src/BulletCollision/CollisionShapes/btBox2dShape.cpp
    src/src/BulletCollision/CollisionShapes/btBoxShape.cpp
    src/src/BulletCollision/CollisionShapes/btBvhTriangleMeshShape.cpp
    src/src/BulletCollision/CollisionShapes/btCapsuleShape.cpp
    src/src/BulletCollision/CollisionShapes/btCollisionShape.cpp
    src/src/BulletCollision/CollisionShapes/btCompoundShape.cpp
    src/src/BulletCollision/CollisionShapes/btConcaveShape.cpp
    src/src/BulletCollision/CollisionShapes/btConeShape.cpp
    src/src/BulletCollision/CollisionShapes/btConvex2dShape.cpp
    src/src/BulletCollision/CollisionShapes/btConvexHullShape.cpp
    src/src/BulletCollision/CollisionShapes/btConvexInternalShape.cpp
    src/src/BulletCollision/CollisionShapes/btConvexPointCloudShape.cpp
    src/src/BulletCollision/CollisionShapes/btConvexPolyhedron.cpp
    src/src/BulletCollision/CollisionShapes/btConvexShape.cpp
    src/src/BulletCollision/CollisionShapes/btConvexTriangleMeshShape.cpp
    src/src/BulletCollision/CollisionShapes/btCylinderShape.cpp
    src/src/BulletCollision/CollisionShapes/btEmptyShape.cpp
    src/src/BulletCollision/CollisionShapes/btHeightfieldTerrainShape.cpp
    src/src/BulletCollision/CollisionShapes/btMinkowskiSumShape.cpp
    src/src/BulletCollision/CollisionShapes/btMultiSphereShape.cpp
    src/src/BulletCollision/CollisionShapes/btMultimaterialTriangleMeshShape.cpp
    src/src/BulletCollision/CollisionShapes/btOptimizedBvh.cpp
    src/src/BulletCollision/CollisionShapes/btPolyhedralConvexShape.cpp
    src/src/BulletCollision/CollisionShapes/btScaledBvhTriangleMeshShape.cpp
    src/src/BulletCollision/CollisionShapes/btShapeHull.cpp
    src/src/BulletCollision/CollisionShapes/btSphereShape.cpp
    src/src/BulletCollision/CollisionShapes/btStaticPlaneShape.cpp
    src/src/BulletCollision/CollisionShapes/btStridingMeshInterface.cpp
    src/src/BulletCollision/CollisionShapes/btTetrahedronShape.cpp
    src/src/BulletCollision/CollisionShapes/btTriangleBuffer.cpp
    src/src/BulletCollision/CollisionShapes/btTriangleCallback.cpp
    src/src/BulletCollision/CollisionShapes/btTriangleIndexVertexArray.cpp
    src/src/BulletCollision/CollisionShapes/btTriangleIndexVertexMaterialArray.cpp
    src/src/BulletCollision/CollisionShapes/btTriangleMesh.cpp
    src/src/BulletCollision/CollisionShapes/btTriangleMeshShape.cpp
    src/src/BulletCollision/CollisionShapes/btUniformScalingShape.cpp
    src/src/BulletCollision/Gimpact/btContactProcessing.cpp
    src/src/BulletCollision/Gimpact/btGImpactBvh.cpp
    src/src/BulletCollision/Gimpact/btGImpactCollisionAlgorithm.cpp
    src/src/BulletCollision/Gimpact/btGImpactQuantizedBvh.cpp
    src/src/BulletCollision/Gimpact/btGImpactShape.cpp
    src/src/BulletCollision/Gimpact/btGenericPoolAllocator.cpp
    src/src/BulletCollision/Gimpact/btTriangleShapeEx.cpp
    src/src/BulletCollision/Gimpact/gim_box_set.cpp
    src/src/BulletCollision/Gimpact/gim_contact.cpp
    src/src/BulletCollision/Gimpact/gim_memory.cpp
    src/src/BulletCollision/Gimpact/gim_tri_collision.cpp
    src/src/BulletCollision/NarrowPhaseCollision/btContinuousConvexCollision.cpp
    src/src/BulletCollision/NarrowPhaseCollision/btConvexCast.cpp
    src/src/BulletCollision/NarrowPhaseCollision/btGjkConvexCast.cpp
    src/src/BulletCollision/NarrowPhaseCollision/btGjkEpa2.cpp
    src/src/BulletCollision/NarrowPhaseCollision/btGjkEpaPenetrationDepthSolver.cpp
    src/src/BulletCollision/NarrowPhaseCollision/btGjkPairDetector.cpp
    src/src/BulletCollision/NarrowPhaseCollision/btMinkowskiPenetrationDepthSolver.cpp
    src/src/BulletCollision/NarrowPhaseCollision/btPersistentManifold.cpp
    src/src/BulletCollision/NarrowPhaseCollision/btPolyhedralContactClipping.cpp
    src/src/BulletCollision/NarrowPhaseCollision/btRaycastCallback.cpp
    src/src/BulletCollision/NarrowPhaseCollision/btSubSimplexConvexCast.cpp
    src/src/BulletCollision/NarrowPhaseCollision/btVoronoiSimplexSolver.cpp
    src/src/BulletDynamics/Character/btKinematicCharacterController.cpp
    src/src/BulletDynamics/ConstraintSolver/btConeTwistConstraint.cpp
    src/src/BulletDynamics/ConstraintSolver/btContactConstraint.cpp
    src/src/BulletDynamics/ConstraintSolver/btFixedConstraint.cpp
    src/src/BulletDynamics/ConstraintSolver/btGearConstraint.cpp
    src/src/BulletDynamics/ConstraintSolver/btGeneric6DofConstraint.cpp
    src/src/BulletDynamics/ConstraintSolver/btGeneric6DofSpring2Constraint.cpp
    src/src/BulletDynamics/ConstraintSolver/btGeneric6DofSpringConstraint.cpp
    src/src/BulletDynamics/ConstraintSolver/btHinge2Constraint.cpp
    src/src/BulletDynamics/ConstraintSolver/btHingeConstraint.cpp
    src/src/BulletDynamics/ConstraintSolver/btNNCGConstraintSolver.cpp
    src/src/BulletDynamics/ConstraintSolver/btPoint2PointConstraint.cpp
    src/src/BulletDynamics/ConstraintSolver/btSequentialImpulseConstraintSolver.cpp
    src/src/BulletDynamics/ConstraintSolver/btSliderConstraint.cpp
    src/src/BulletDynamics/ConstraintSolver/btSolve2LinearConstraint.cpp
    src/src/BulletDynamics/ConstraintSolver/btTypedConstraint.cpp
    src/src/BulletDynamics/ConstraintSolver/btUniversalConstraint.cpp
    src/src/BulletDynamics/Dynamics/btDiscreteDynamicsWorld.cpp
    src/src/BulletDynamics/Dynamics/btRigidBody.cpp
    src/src/BulletDynamics/Dynamics/btSimpleDynamicsWorld.cpp
    src/src/BulletDynamics/Vehicle/btRaycastVehicle.cpp
    src/src/BulletDynamics/Vehicle/btWheelInfo.cpp
    src/src/BulletSoftBody/btDefaultSoftBodySolver.cpp
    src/src/BulletSoftBody/btSoftBody.cpp
    src/src/BulletSoftBody/btSoftBodyConcaveCollisionAlgorithm.cpp
    src/src/BulletSoftBody/btSoftBodyHelpers.cpp
    src/src/BulletSoftBody/btSoftBodyRigidBodyCollisionConfiguration.cpp
    src/src/BulletSoftBody/btSoftRigidCollisionAlgorithm.cpp
    src/src/BulletSoftBody/btSoftRigidDynamicsWorld.cpp
    src/src/BulletSoftBody/btSoftSoftCollisionAlgorithm.cpp
    src/src/LinearMath/btAlignedAllocator.cpp
    src/src/LinearMath/btConvexHull.cpp
    src/src/LinearMath/btConvexHullComputer.cpp
    src/src/LinearMath/btGeometryUtil.cpp
    src/src/LinearMath/btPolarDecomposition.cpp
    src/src/LinearMath/btQuickprof.cpp
    src/src/LinearMath/btSerializer.cpp
    src/src/LinearMath/btVector3.cpp
)

# add this project as static library
add_library(BulletPhysics STATIC ${SOURCES})

# private include dirs to build this project
target_include_directories(BulletPhysics PRIVATE
    src/src
)

# compiler definitions (-D) for private build
#target_compile_definitions(BulletPhysics PRIVATE
#    XXXX=1
#    XXXX
#)

# compiler options for private build
if(APPLE)
    target_compile_options(BulletPhysics PRIVATE
        ${OPT_ARCH_FLAG}
        -ffast-math
    )
else()
    target_compile_options(BulletPhysics PRIVATE
        ${OPT_ARCH_FLAG}
        -ffast-math
        -Wno-stringop-overflow
    )
endif()

##############
# public compiler definitions (-D) required to link with this project
#target_compile_definitions(BulletPhysics PUBLIC
#)

# public compiler options required to link with this project
#target_compile_options(BulletPhysics PUBLIC
#)

# public link libraries to be linked with this project
#target_link_libraries(BulletPhysics PUBLIC
#)
