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
#include "BoneController.h"

/* BoneController::initialize: initialize bone controller */
void BoneController::initialize()
{
   m_numBone = 0;
   m_boneList = NULL;
   m_rotList = NULL;

   m_rateOn = 1.0f;
   m_rateOff = 1.0f;
   m_baseVector = btVector3(btScalar(0.0f), btScalar(0.0f), btScalar(0.0f));
   m_upperAngLimit = btVector3(btScalar(0.0f), btScalar(0.0f), btScalar(0.0f));
   m_lowerAngLimit = btVector3(btScalar(0.0f), btScalar(0.0f), btScalar(0.0f));
   m_adjustPos = btVector3(btScalar(0.0f), btScalar(0.0f), btScalar(0.0f));

   m_numChildBone = 0;
   m_childBoneList = NULL;

   m_enable = false;
   m_fadingRate = 0.0f;
}

/* BoneController::clear: free bone controller */
void BoneController::clear()
{
   if(m_boneList)
      free(m_boneList);
   if (m_rotList)
      MMDFiles_alignedfree(m_rotList);
   if(m_childBoneList)
      free(m_childBoneList);

   initialize();
}

/* BoneController::BoneController: constructor */
BoneController::BoneController()
{
   initialize();
}

/* BoneController::~BoneController: destructor */
BoneController::~BoneController()
{
   clear();
}

/* BoneController::setup: initialize and setup bone controller */
void BoneController::setup(PMDModel *model, const char **boneName, int numBone, float rateOn, float rateOff,
                           float baseVectorX, float baseVectorY, float baseVectorZ,
                           float upperAngLimitX, float upperAngLimitY, float upperAngLimitZ,
                           float lowerAngLimitX, float lowerAngLimitY, float lowerAngLimitZ,
                           float adjustPosX, float adjustPosY, float adjustPosZ)
{
   int i, j;
   PMDBone **tmpBoneList;

   /* check */
   if(model == NULL || boneName == NULL || numBone <= 0) return;

   /* initialize */
   clear();

   /* set bones */
   tmpBoneList = (PMDBone **) malloc(sizeof(PMDBone *) * numBone);
   for(i = 0, j = 0; i < numBone; i++) {
      tmpBoneList[i] = model->getBone(boneName[i]);
      if(tmpBoneList[i] != NULL)
         j++;
   }
   if(j <= 0) {
      free(tmpBoneList);
      return;
   }
   m_numBone = j;
   m_boneList = (PMDBone **) malloc(sizeof(PMDBone *) * m_numBone);
   for(i = 0, j = 0; i < numBone; i++)
      if(tmpBoneList[i] != NULL)
         m_boneList[j++] = tmpBoneList[i];
   free(tmpBoneList);
   m_rotList = (btQuaternion*)MMDFiles_alignedmalloc(sizeof(btQuaternion) * m_numBone, 16);
   for (int i = 0; i < m_numBone; i++)
      new (m_rotList + i) btQuaternion();

   /* set parameters */
   m_rateOn = rateOn;
   m_rateOff = rateOff;
   m_baseVector = btVector3(btScalar(baseVectorX), btScalar(baseVectorY), btScalar(baseVectorZ));
   m_upperAngLimit = btVector3(btScalar(MMDFILES_RAD(upperAngLimitX)), btScalar(MMDFILES_RAD(upperAngLimitY)), btScalar(MMDFILES_RAD(upperAngLimitZ)));
   m_lowerAngLimit = btVector3(btScalar(MMDFILES_RAD(lowerAngLimitX)), btScalar(MMDFILES_RAD(lowerAngLimitY)), btScalar(MMDFILES_RAD(lowerAngLimitZ)));
   m_adjustPos = btVector3(btScalar(adjustPosX), btScalar(adjustPosY), btScalar(adjustPosZ));

   /* set child bones */
   if(model->getNumBone() > 0) {
      tmpBoneList = (PMDBone **) malloc(sizeof(PMDBone *) * model->getNumBone());
      m_numChildBone = model->getChildBoneList(m_boneList, m_numBone, tmpBoneList);
      if(m_numChildBone > 0) {
         m_childBoneList = (PMDBone **) malloc(sizeof(PMDBone *) * m_numChildBone);
         for(i = 0; i < m_numChildBone; i++)
            m_childBoneList[i] = tmpBoneList[i];
      }
      free(tmpBoneList);
   }
}

/* BoneController::setEnableFlag: set enable flag */
void BoneController::setEnableFlag(bool b)
{
   int i;

   if(b == true) {
      for(i = 0; i < m_numBone; i++)
         m_boneList[i]->getCurrentRotation(&m_rotList[i]);
   } else if(m_enable == true) {
      m_fadingRate = 1.0f;
   }
   m_enable = b;
}

/* BoneController::update: update motions */
bool BoneController::update(btVector3 *pos, float deltaFrame)
{
   int i;
   float rate, dot;
   btVector3 v, localDest, axis;

   btQuaternion targetRot;
   btScalar x, y, z;
   btMatrix3x3 mat;

   if(m_numBone <= 0) return false;

   if(m_enable) {
      /* rotate bone to target */
      /* increasement rate */
      rate = m_rateOn * deltaFrame;
      if(rate > 1.0f) rate = 1.0f;
      if(rate < 0.0f) rate = 0.0f;
      /* set offset to target position */
      v = (*pos) + m_adjustPos;
      for(i = 0; i < m_numBone; i++) {
         /* calculate rotation to target position */
         localDest = m_boneList[i]->getTransform()->inverse() * v;
         localDest.normalize();
         dot = m_baseVector.dot(localDest);
         if (dot <= 1.0f) {
            axis = m_baseVector.cross(localDest);
            if (axis.length2() >= BONECONTROLLER_MINLENGTH) {
               axis.normalize();
               targetRot = btQuaternion(axis, btScalar(acosf(dot)));
               /* set limit of rotation */
               mat.setRotation(targetRot);
               mat.getEulerZYX(z, y, x);
               if (x > m_upperAngLimit.x()) x = m_upperAngLimit.x();
               if (y > m_upperAngLimit.y()) y = m_upperAngLimit.y();
               if (z > m_upperAngLimit.z()) z = m_upperAngLimit.z();
               if (x < m_lowerAngLimit.x()) x = m_lowerAngLimit.x();
               if (y < m_lowerAngLimit.y()) y = m_lowerAngLimit.y();
               if (z < m_lowerAngLimit.z()) z = m_lowerAngLimit.z();
               targetRot.setEulerZYX(z, y, x);
               /* slerp from current rotation to target rotation */
               m_rotList[i] = m_rotList[i].slerp(targetRot, btScalar(rate));
               /* set result to current rotation */
               m_boneList[i]->setCurrentRotation(&m_rotList[i]);
            }
         }
      }
      /* slerp from current rotation to target rotation */
      for(i = 0; i < m_numBone; i++)
         m_boneList[i]->update();
      /* set result to current rotation */
      for(i = 0; i < m_numChildBone; i++)
         m_childBoneList[i]->update();
   } else {
      /* loose bone to its original rotation */
      if (m_fadingRate > 0.0f) {
         /* decrement rate */
         m_fadingRate -= m_rateOff * deltaFrame;
         if (m_fadingRate < 0.0f)
            m_fadingRate = 0.0f;
         /* rate multiplication for bone rotation */
         for (i = 0; i < m_numBone; i++) {
            m_boneList[i]->getCurrentRotation(&targetRot);
            m_rotList[i] = targetRot.slerp(m_rotList[i], btScalar(m_fadingRate));
            m_boneList[i]->setCurrentRotation(&m_rotList[i]);
         }
         /* update bone transform matrices */
         for (i = 0; i < m_numBone; i++)
            m_boneList[i]->update();
         for (i = 0; i < m_numChildBone; i++)
            m_childBoneList[i]->update();
      } else {
         /* now bone has returned to its original rotation */
         return false;
      }
   }

   return true;
}

