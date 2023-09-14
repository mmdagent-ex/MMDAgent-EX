#!/bin/sh
cp src/src/*.h include
cp src/src/BulletCollision/BroadphaseCollision/*.h include/BulletCollision/BroadphaseCollision
cp src/src/BulletCollision/CollisionDispatch/*.h include/BulletCollision/CollisionDispatch
cp src/src/BulletCollision/CollisionShapes/*.h include/BulletCollision/CollisionShapes
cp src/src/BulletCollision/Gimpact/*.h include/BulletCollision/Gimpact
cp src/src/BulletCollision/NarrowPhaseCollision/*.h include/BulletCollision/NarrowPhaseCollision
cp src/src/BulletDynamics/Character/*.h include/BulletDynamics/Character
cp src/src/BulletDynamics/ConstraintSolver/*.h include/BulletDynamics/ConstraintSolver
cp src/src/BulletDynamics/Dynamics/*.h include/BulletDynamics/Dynamics
cp src/src/BulletDynamics/Vehicle/*.h include/BulletDynamics/Vehicle
cp src/src/BulletSoftBody/*.h include/BulletSoftBody
cp src/src/LinearMath/*.h include/LinearMath
