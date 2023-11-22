/*
  Copyright 2022-2023  Nagoya Institute of Technology

  Licensed under the Apache License, Version 2.0 (the "License");
  you may not use this file except in compliance with the License.
  You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.
*/
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

/* PMDIK::initialize: initialize IK */
void PMDIK::initialize()
{
   m_destBone = NULL;
   m_targetBone = NULL;
   m_boneList = NULL;
   m_numBone = 0;
   m_iteration = 0;
   m_angleConstraint = 0.0f;
}

/* PMDIK::clear: free IK */
void PMDIK::clear()
{
   if (m_boneList)
      free(m_boneList);
   initialize();
}

/* PMDIK::PMDIK: constructor */
PMDIK::PMDIK()
{
   initialize();
}

/* PMDIK::~PMDIK: destructor */
PMDIK::~PMDIK()
{
   clear();
}

/* PMDIK::setup: initialize and setup IK  */
void PMDIK::setup(PMDFile_IK *ik, const unsigned char *data, PMDBone *boneList)
{
   unsigned char i;
   short ikBoneId;

   clear();

   m_destBone = &(boneList[ik->destBoneID]);
   m_targetBone = &(boneList[ik->targetBoneID]);
   m_numBone = ik->numLink;
   if (m_numBone) {
      m_boneList = (PMDBone **) malloc(sizeof(PMDBone *) * m_numBone);
      for (i = 0; i < m_numBone; i++) {
         memcpy(&ikBoneId, data + sizeof(short) * i, sizeof(short));
         m_boneList[i] = &(boneList[ikBoneId]);
      }
   }
   m_iteration = ik->numIteration;
   m_angleConstraint = ik->angleConstraint * 4.0f;
}

/* PMDIK::solve: try to move targetBone toward destBone, solving constraint among bones in boneList[] and the targetBone */
void PMDIK::solve()
{
   short i;
   unsigned char j;
   unsigned short ite;
   btQuaternion tmpRot;

   btQuaternion origTargetRot;

   btVector3 destPos; /* destination position */
   btVector3 targetPos;
   btVector3 currentBonePos;

   btTransform tr;
   btVector3 localDestVec;
   btVector3 localTargetVec;

   float angle, dot;

   btVector3 axis;
   btQuaternion rot;

   btScalar x, y, z;
   btScalar cx, cy, cz;
   btMatrix3x3 mat;

   if (m_boneList == NULL)
      return;

   if (m_destBone->getIKSwitchFlag() == false)
      return;

   /* get the global destination point */
   destPos = m_destBone->getTransform()->getOrigin();

   /* before begin IK iteration, make sure all the child bones and target bone are up to date update from root to child */
#ifndef MMDFILES_DONTUPDATEMATRICESFORIK
   /* this can be disabled for compatibility with MikuMikuDance */
   updateLinkBones();
#endif /* !MMDFILES_DONTUPDATEMATRICESFORIK */

   /* save the current rotation of the target bone */
   /* it will be restored at the end of this function */
   m_targetBone->getCurrentRotation(&origTargetRot);

   /* begin IK iteration */
   for (ite = 0; ite < m_iteration; ite++) {
      /* solve each step from leaf bone to root bone */
      for (j = 0; j < m_numBone; j++) {
         /* get current global target bone location */
         targetPos = m_targetBone->getTransform()->getOrigin();
         /* skip if target or destination is idential to current bone */
         currentBonePos = m_boneList[j]->getTransform()->getOrigin();
         if (currentBonePos == targetPos || currentBonePos == destPos)
            continue;
         /* calculate local positions of destination position and target position at current bone */
         tr = m_boneList[j]->getTransform()->inverse();
         localDestVec = tr * destPos;
         localTargetVec = tr * targetPos;
         /* exit if they are close enough */
         if (localDestVec.distance2(localTargetVec) < PMDIK_MINDISTANCE) {
            ite = m_iteration;
            break;
         }
         /* normalize vectors */
         localDestVec.normalize();
         localTargetVec.normalize();
         /* get angle */
         dot = localDestVec.dot(localTargetVec);
         if (dot > 1.0f) /* assume angle = 0.0f, skip to next bone */
            continue;
         angle = acosf(dot);
         /* if angle is small enough, skip to next bone */
         if (fabsf(angle) < PMDIK_MINANGLE)
            continue;
         /* limit angle per step */
         if (angle < - m_angleConstraint)
            angle = -m_angleConstraint;
         else if (angle > m_angleConstraint)
            angle = m_angleConstraint;
         /* get rotation axis */
         axis = localTargetVec.cross(localDestVec);
         /* if the axis is too small (= direction of destination and target is so close) and this is not a first iteration, skip to next bone */
         x = axis.length2();
         if (x < PMDIK_MINAXIS && ite > 0)
            continue;
         if (x < SIMD_EPSILON * SIMD_EPSILON) {
            continue;
         }
         /* normalize rotation axis */
         axis.normalize();
         /* create quaternion for this step, to rotate the target point to the goal point, from the axis and angle */
         rot = btQuaternion(axis, btScalar(angle));
         /* if this bone has limitation for rotation, consult the limitation */
         if (m_boneList[j]->isLimitAngleX()) {
            /* get euler angles of this rotation */
            mat.setRotation(rot);
            mat.getEulerZYX(z, y, x);
            /* get euler angles of current bone rotation (specified by the motion) */
            m_boneList[j]->getCurrentRotation(&tmpRot);
            mat.setRotation(tmpRot);
            mat.getEulerZYX(cz, cy, cx);
            if (ite == 0 && cx < m_angleConstraint) {
               /* when this is the first iteration, we force rotating to the maximum angle toward limited direction */
               /* this will help convergence the whole IK step earlier for most of models, especially for legs */
               if (angle < 0.0f)
                  angle = -angle;
               rot = btQuaternion(btVector3(btScalar(1.0f), btScalar(0.0f), btScalar(0.0f)), btScalar(angle));
            } else {
               /* y and z should be zero, x should be from 0 to PI */
               if (cx < -PMDIK_PI * 0.5f)
                  cx += PMDIK_PI * 2.0f;
               if (x + cx > PMDIK_PI)
                  x = PMDIK_PI - cx;
               if (PMDIK_MINROTSUM > x + cx)
                  x = PMDIK_MINROTSUM - cx;
               /* if rotation becomes minimal by the limitation, skip to next bone */
               if (fabsf(x) < PMDIK_MINROTATION)
                  continue;
               /* get rotation quaternion from the limited euler angles */
               rot.setEulerZYX(btScalar(0.0f), btScalar(0.0f), btScalar(x));
            }
            /* apply the limited rotation to current bone */
            m_boneList[j]->getCurrentRotation(&tmpRot);
            tmpRot = rot * tmpRot;
            m_boneList[j]->setCurrentRotation(&tmpRot);
         } else {
            /* apply the rotation to current bone */
            m_boneList[j]->getCurrentRotation(&tmpRot);
            tmpRot *= rot;
            m_boneList[j]->setCurrentRotation(&tmpRot);
         }

         /* update transform matrices for relevant (child) bones */
         for (i = j; i >= 0; i--) m_boneList[i]->update();
         m_targetBone->update();
      }
   }

   /* restore the original rotation of the target bone */
   m_targetBone->setCurrentRotation(&origTargetRot);
   m_targetBone->update();
}

/* PMDIK::updateLinkBones: update link bones */
void PMDIK::updateLinkBones()
{
   short i;

   for (i = m_numBone - 1; i >= 0; i--)
      m_boneList[i]->update();
   m_targetBone->update();
}

/* PMDIK::isSimulated: check if this IK is under simulation, in case no need to calculate this IK */
bool PMDIK::isSimulated()
{
   return m_boneList[0]->isSimulated();
}

/* PMDIK::updateInfo: update information */
void PMDIK::updateInfo()
{
   if (m_destBone) m_destBone->setIK(this);
   for (short i = 0; i < m_numBone; i++) {
      m_boneList[i]->setIKLink(this);
   }
}
