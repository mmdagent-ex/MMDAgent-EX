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

/* PMDConstraint::initialize: initialize constraint */
void PMDConstraint::initialize()
{
   m_constraint = NULL;
   m_world = NULL;
}

/* PMDConstraint::clear: free constraint */
void PMDConstraint::clear()
{
   if (m_constraint) {
      m_world->removeConstraint(m_constraint);
      delete m_constraint;
   }

   initialize();
}

/* PMDConstraint::PMDConstraint: constructor */
PMDConstraint::PMDConstraint()
{
   initialize();
}

/* PMDConstraint::~PMDConstraint: destructor */
PMDConstraint::~PMDConstraint()
{
   clear();
}

/* PMDConstraint::setup: initialize and setup constraint */
bool PMDConstraint::setup(PMDFile_Constraint *c, PMDRigidBody *bodyList, btVector3 *offset)
{
   int i;

   btRigidBody *rbA;
   btRigidBody *rbB;
   btTransform tr;
   btQuaternion rot;
   btTransform trA;
   btTransform trB;

   clear();

   /* get pointer to bodies at both end of this constraint */
   rbA = bodyList[c->bodyIDA].getBody();
   rbB = bodyList[c->bodyIDB].getBody();

   if (rbA == NULL || rbB == NULL) return false;

   /* make global transform of this constraint */
   tr.setIdentity();
#ifdef MMDFILES_CONVERTCOORDINATESYSTEM
   rot.setEuler(btScalar(-c->rot[1]), btScalar(-c->rot[0]), btScalar(c->rot[2]));
#else
   rot.setEulerZYX(btScalar(c->rot[2]), btScalar(c->rot[1]), btScalar(c->rot[0]));
#endif /* MMDFILES_CONVERTCOORDINATESYSTEM */
   tr.setRotation(rot);
#ifdef MMDFILES_CONVERTCOORDINATESYSTEM
   tr.setOrigin(btVector3(btScalar(c->pos[0]), btScalar(c->pos[1]), btScalar(-c->pos[2])) + *offset);
#else
   tr.setOrigin(btVector3(btScalar(c->pos[0]), btScalar(c->pos[1]), btScalar(c->pos[2])) + *offset);
#endif /* MMDFILES_CONVERTCOORDINATESYSTEM */
   /* make local transforms from both rigid bodies */
   trA = rbA->getWorldTransform().inverse() * tr;
   trB = rbB->getWorldTransform().inverse() * tr;

   /* create constraint */
   m_constraint = new btGeneric6DofSpringConstraint(*rbA, *rbB, trA, trB, true);

#ifdef MMDFILES_CONVERTCOORDINATESYSTEM
   /* set linear translation limits */
   m_constraint->setLinearLowerLimit(btVector3(btScalar(c->limitPosFrom[0]), btScalar(c->limitPosFrom[1]), btScalar(-c->limitPosTo[2])));
   m_constraint->setLinearUpperLimit(btVector3(btScalar(c->limitPosTo[0]), btScalar(c->limitPosTo[1]), btScalar(-c->limitPosFrom[2])));

   /* set rotation angle limits */
   m_constraint->setAngularLowerLimit(btVector3(btScalar(-c->limitRotTo[0]), btScalar(-c->limitRotTo[1]), btScalar(c->limitRotFrom[2])));
   m_constraint->setAngularUpperLimit(btVector3(btScalar(-c->limitRotFrom[0]), btScalar(-c->limitRotFrom[1]), btScalar(c->limitRotTo[2])));
#else
   /* set linear translation limits */
   m_constraint->setLinearLowerLimit(btVector3(btScalar(c->limitPosFrom[0]), btScalar(c->limitPosFrom[1]), btScalar(c->limitPosFrom[2])));
   m_constraint->setLinearUpperLimit(btVector3(btScalar(c->limitPosTo[0]), btScalar(c->limitPosTo[1]), btScalar(c->limitPosTo[2])));

   /* set rotation angle limits */
   m_constraint->setAngularLowerLimit(btVector3(btScalar(c->limitRotFrom[0]), btScalar(c->limitRotFrom[1]), btScalar(c->limitRotFrom[2])));
   m_constraint->setAngularUpperLimit(btVector3(btScalar(c->limitRotTo[0]), btScalar(c->limitRotTo[1]), btScalar(c->limitRotTo[2])));
#endif /* MMDFILES_CONVERTCOORDINATESYSTEM */

   /* set spring stiffnesses */
   for (i = 0; i < 6; i++) {
      if (i >= 3 || c->stiffness[i] != 0.0f) {
         /* allow spring always for rotation and only if stiffness is specified for translation */
         m_constraint->enableSpring(i, true);
         m_constraint->setStiffness(i, c->stiffness[i]);
      }
   }

   return true;
}

/* PMDConstraint::joinWorld: add the constraint to simulation world */
void PMDConstraint::joinWorld(btDiscreteDynamicsWorld *btWorld)
{
   if (!m_constraint)
      return;

   btWorld->addConstraint(m_constraint);
   m_world = btWorld;
}
