/* ----------------------------------------------------------------- */
/*           The Toolkit for Building Voice Interaction Systems      */
/*           "MMDAgent" developed by MMDAgent Project Team           */
/*           http://www.mmdagent.jp/                                 */
/* ----------------------------------------------------------------- */
/*                                                                   */
/*  Copyright (c) 2009-2016  Nagoya Institute of Technology          */
/*                           Department of Computer Science          */
/*                                                                   */
/* All rights reserved.                                              */
/*                                                                   */
/* Redistribution and use in source and binary forms, with or        */
/* without modification, are permitted provided that the following   */
/* conditions are met:                                               */
/*                                                                   */
/* - Redistributions of source code must retain the above copyright  */
/*   notice, this list of conditions and the following disclaimer.   */
/* - Redistributions in binary form must reproduce the above         */
/*   copyright notice, this list of conditions and the following     */
/*   disclaimer in the documentation and/or other materials provided */
/*   with the distribution.                                          */
/* - Neither the name of the MMDAgent project team nor the names of  */
/*   its contributors may be used to endorse or promote products     */
/*   derived from this software without specific prior written       */
/*   permission.                                                     */
/*                                                                   */
/* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND            */
/* CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,       */
/* INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF          */
/* MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE          */
/* DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS */
/* BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,          */
/* EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED   */
/* TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,     */
/* DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON */
/* ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,   */
/* OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY    */
/* OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE           */
/* POSSIBILITY OF SUCH DAMAGE.                                       */
/* ----------------------------------------------------------------- */

/* headers */

#include "MMDFiles.h"
#include "KinematicMotionState.h"

/* PMDRigidBody::initialize: initialize PMDRigidBody */
void PMDRigidBody::initialize()
{
   m_shape = NULL;
   m_body = NULL;
   m_motionState = NULL;
   m_groupID = 0;
   m_groupMask = 0;

   m_type = 0;
   m_bone = NULL;
   m_noBone = false;
   m_kinematicMotionState = NULL;
   m_savedTrans.setIdentity();
   m_savedForce.setZero();
   m_savedLinearFactor.setZero();
   m_savedLinearVelocity.setZero();
   m_savedTorque.setZero();
   m_savedAngularFactor.setZero();
   m_savedAngularVelocity.setZero();

   m_world = NULL;
}

/* PMDRigidBody::clear: free PMDRigidBody */
void PMDRigidBody::clear()
{
   /* release motion state */
   if (m_motionState) {
      m_motionState->~btMotionState();
      btAlignedFree(m_motionState);
   }
   if (m_kinematicMotionState) {
      m_kinematicMotionState->~btMotionState();
      btAlignedFree(m_kinematicMotionState);
   }
   if (m_body) {
      m_world->removeCollisionObject(m_body); /* release body */
      delete m_body;
   }
   if (m_shape)
      delete m_shape;

   initialize();
}

/* PMDRigidBody::PMDRigidBody: constructor */
PMDRigidBody::PMDRigidBody()
{
   initialize();
}

/* PMDRigidBody::~PMDRigidBody: destructor */
PMDRigidBody::~PMDRigidBody()
{
   clear();
}

/* PMDRigidBody::setup: initialize and setup PMDRigidBody */
bool PMDRigidBody::setup(PMDFile_RigidBody *rb, PMDBone *bone)
{
   btScalar mass;
   btVector3 localInertia(btScalar(0.0f), btScalar(0.0f), btScalar(0.0f));
   btQuaternion rot;
   btTransform startTrans;

   clear();

   /* store bone */
   m_bone = bone;
   m_noBone = false;
   if (rb->boneID == 0xFFFF)
      m_noBone = true;

   /* store type ID */
   m_type = rb->type;

   /* create shape */
   if (rb->shapeType == 0) {
      /* sphere: radius == width */
      m_shape = new btSphereShape(btScalar(rb->width));
   } else if (rb->shapeType == 1) {
      /* box: half extent: width, height, depth */
      m_shape = new btBoxShape(btVector3(btScalar(rb->width), btScalar(rb->height), btScalar(rb->depth)));
   } else if (rb->shapeType == 2) {
      m_shape = new btCapsuleShape(btScalar(rb->width), btScalar(rb->height));
   } else {
      return false;
   }

   /* set mass and local inertial tensor */
   if (rb->type != 0)
      mass = rb->mass; /* dynamic (non-kinematic) bodies */
   else
      mass = 0.0f; /* the mass of static (kinematic) bodies should be always set to 0 */
   if (mass != 0.0f)
      m_shape->calculateLocalInertia(mass, localInertia);

   /* set position and rotation of the rigid body, local to the associated bone */
   m_trans.setIdentity();
#ifdef MMDFILES_CONVERTCOORDINATESYSTEM
   rot.setEuler(btScalar(-rb->rot[1]), btScalar(-rb->rot[0]), btScalar(rb->rot[2]));
#else
   rot.setEulerZYX(btScalar(rb->rot[2]), btScalar(rb->rot[1]), btScalar(rb->rot[0]));
#endif /* MMDFILES_CONVERTCOORDINATESYSTEM */
   m_trans.setRotation(rot);
#ifdef MMDFILES_CONVERTCOORDINATESYSTEM
   m_trans.setOrigin(btVector3(btScalar(rb->pos[0]), btScalar(rb->pos[1]), btScalar(-rb->pos[2])));
#else
   m_trans.setOrigin(btVector3(btScalar(rb->pos[0]), btScalar(rb->pos[1]), btScalar(rb->pos[2])));
#endif /* MMDFILES_CONVERTCOORDINATESYSTEM */

   /* calculate initial global transform */
   startTrans.setIdentity();
   startTrans.setOrigin(m_bone->getTransform()->getOrigin());
   startTrans *= m_trans;

   /* prepare motion state */
   if (rb->type == 0) {
      /* kinematic body, will be moved along the motion of corresponding bone */
      void* ptr = btAlignedAlloc(sizeof(KinematicMotionState), 16);
      m_motionState = new(ptr) KinematicMotionState(startTrans, m_trans, m_bone);
      m_kinematicMotionState = NULL;
   } else {
      /* simulated body, use default motion state */
      void* ptr = btAlignedAlloc(sizeof(btDefaultMotionState), 16);
      m_motionState = new(ptr) btDefaultMotionState(startTrans);
      ptr = btAlignedAlloc(sizeof(KinematicMotionState), 16);
      m_kinematicMotionState = new(ptr) KinematicMotionState(startTrans, m_trans, m_bone);
   }

   /* set rigid body parameters */
   btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, m_motionState, m_shape, localInertia);
   rbInfo.m_linearDamping = rb->linearDamping;
   rbInfo.m_angularDamping = rb->angularDamping;
   rbInfo.m_restitution = rb->restitution;
   rbInfo.m_friction = rb->friction;
   /* additional damping can help avoiding lowpass jitter motion, help stability for ragdolls etc. */
   rbInfo.m_additionalDamping = true;

   /* make rigid body for the shape */
   m_body = new btRigidBody(rbInfo);

   /* for knematic body, flag them as kinematic and disable the sleeping/deactivation */
   if (rb->type == 0)
      m_body->setCollisionFlags(m_body->getCollisionFlags() | btCollisionObject::CF_KINEMATIC_OBJECT);

   /* disable deactivation for all rigid bodies */
   m_body->setActivationState(DISABLE_DEACTIVATION);

   /* set collision group index and group mask */
   m_groupID = 0x0001 << rb->collisionGroupID;
   m_groupMask = rb->collisionMask;

   /* store inverse matrix of local transform */
   m_transInv = m_trans.inverse();

   /* store type ID in body flag for debug */
   switch(rb->type) {
   case 0:
      m_body->setFlags(m_body->getFlags() | BULLETPHYSICS_RIGIDBODYFLAGB);
      break;
   case 1:
      m_body->setFlags(m_body->getFlags() | BULLETPHYSICS_RIGIDBODYFLAGP);
      break;
   case 2:
      m_body->setFlags(m_body->getFlags() | BULLETPHYSICS_RIGIDBODYFLAGA);
      break;
   default:
      break;
   }
   m_body->setFlags(m_body->getFlags() | (rb->collisionGroupID << 8));

   return true;
}

/* PMDRigidBody::joinWorld: add the body to simulation world */
void PMDRigidBody::joinWorld(btDiscreteDynamicsWorld *btWorld)
{
   if (!m_body) return;

   /* add the body to the simulation world, with group id and group mask for collision */
   btWorld->addRigidBody(m_body, m_groupID, m_groupMask);
   m_world = btWorld;
}

/* PMDRigidBody::applyTransformToBone: apply the current rigid body transform to bone after simulation (for type 1 and 2) */
void PMDRigidBody::applyTransformToBone()
{
   btTransform tr;

   if (m_type == 0 || m_bone == NULL || m_noBone) return;

   tr = m_body->getCenterOfMassTransform();
   tr *= m_transInv;
   if (m_type == 2) {
      /* align the bone position to the non-simulated position */
      m_bone->update();
      tr.setOrigin(m_bone->getTransform()->getOrigin());
   }
   m_bone->setTransform(&tr);
   m_bone->setTransBySimulation(&tr);
}

/* PMDRigidBody::setKinematic: switch between Default and Kinematic body for non-simulated movement */
#ifdef MY_RESETPHYSICS
void PMDRigidBody::setKinematic(bool flag, bool keepStatus)
#else
void PMDRigidBody::setKinematic(bool flag)
#endif
{
   btTransform worldTrans, savedBaseTrans, tr;
   PMDBone *base;

   if (m_type == 0) return; /* always kinematic */

   if (flag) {
      /* default to kinematic */
#ifdef MY_RESETPHYSICS
      if (keepStatus) {
#endif
      /* save current world transform, force, and torque for resuming */
      m_savedTrans = m_body->getCenterOfMassTransform();
      m_savedForce = m_body->getTotalForce();
      m_savedLinearFactor = m_body->getLinearFactor();
      m_savedLinearVelocity = m_body->getLinearVelocity();
      m_savedTorque = m_body->getTotalTorque();
      m_savedAngularFactor = m_body->getAngularFactor();
      m_savedAngularVelocity = m_body->getAngularVelocity();
#ifdef MY_RESETPHYSICS
      }
#endif
      /* clear all forces and change motion state to kinematic */
      m_body->clearForces();
      m_body->setMotionState(m_kinematicMotionState);
      m_body->setCollisionFlags(m_body->getCollisionFlags() | btCollisionObject::CF_KINEMATIC_OBJECT);
   } else {
      /* kinematic to default */
#ifdef MY_RESETPHYSICS
      if (keepStatus) {
#endif
      /* find the first un-simulated bone in parents */
      for (base = m_bone; base; base = base->getParentBone())
         if (!base->isSimulated()) break;
      /* calculate initial transform */
      if (base) {
         /* compute transform of bone since the start of kinematic state */
         base->getSavedTrans(&savedBaseTrans);
         /* compute the difference matrix */
         tr = (*(base->getTransform())) * savedBaseTrans.inverse();
         /* apply the transform to the last saved transform to get the initial transform */
         worldTrans = tr * m_savedTrans;
         /* also apply the rotation to the saved velocities */
         tr.setOrigin(btVector3(btScalar(0.0f), btScalar(0.0f), btScalar(0.0f)));
         m_savedLinearVelocity = tr * m_savedLinearVelocity;
         m_savedAngularVelocity = tr * m_savedAngularVelocity;
      } else {
         m_kinematicMotionState->getWorldTransform(worldTrans);
      }
#ifdef MY_RESETPHYSICS
      } else {
         m_kinematicMotionState->getWorldTransform(worldTrans);
      }
#endif
      /* set the initial tranform to the motion state */
      /* it will be set to body by getWorldTransform() at next m_body->setMotionState() */
      m_motionState->setWorldTransform(worldTrans);
      /* change motion state to default */
      m_body->setMotionState(m_motionState);
      m_body->setCollisionFlags(m_body->getCollisionFlags() & ~btCollisionObject::CF_KINEMATIC_OBJECT);

      /* apply the saved force and torque */
#ifdef MY_RESETPHYSICS
      if (keepStatus) {
#endif
      m_body->clearForces();
      m_body->setLinearFactor(btVector3(btScalar(1.0f), btScalar(1.0f), btScalar(1.0f)));
      m_body->applyCentralForce(m_savedForce);
      m_body->setLinearFactor(m_savedLinearFactor);
      m_body->setLinearVelocity(m_savedLinearVelocity);
      m_body->setAngularFactor(btVector3(btScalar(1.0f), btScalar(1.0f), btScalar(1.0f)));
      m_body->applyTorque(m_savedTorque);
      m_body->setAngularFactor(m_savedAngularFactor);
      m_body->setAngularVelocity(m_savedAngularVelocity);
#ifdef MY_RESETPHYSICS
      }
#endif
   }
}

/* PMDRigidBody::getBody: get rigid body */
btRigidBody *PMDRigidBody::getBody()
{
   return m_body;
}
