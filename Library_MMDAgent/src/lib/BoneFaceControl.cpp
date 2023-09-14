/* ----------------------------------------------------------------- */
/*           The Toolkit for Building Voice Interaction Systems      */
/*           "MMDAgent" developed by MMDAgent Project Team           */
/*           http://www.mmdagent.jp/                                 */
/* ----------------------------------------------------------------- */
/*                                                                   */
/*  Copyright (c) 2009-2015  Nagoya Institute of Technology          */
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

#include "MMDAgent.h"

/* BoneFaceControl::initialize: initialize */
void BoneFaceControl::initialize()
{
   m_keyValue = NULL;
   m_keyName = NULL;
   m_targetName = NULL;
   m_min = 0.0f;
   m_max = 0.0f;
   m_bone = NULL;
   m_pos0.setZero();
   m_pos1.setZero();
   m_rot0.setEulerZYX(btScalar(0.0f), btScalar(0.0f), btScalar(0.0f));
   m_rot1.setEulerZYX(btScalar(0.0f), btScalar(0.0f), btScalar(0.0f));
   m_face = NULL;
   m_boneMorph = NULL;
   m_vertexMorph = NULL;
   m_uvMorph = NULL;
   m_materialMorph = NULL;
   m_groupMorph = NULL;
   m_val0 = 0.0f;
   m_val1 = 0.0f;
   m_isBone = false;
   m_durationFrame = 0.0;
   m_restFrame = 0.0;
   m_numChildBone = 0;
   m_childBoneList = NULL;
}

/* BoneFaceControl::clear: free */
void BoneFaceControl::clear()
{
   if (m_keyName)
      free(m_keyName);
   if (m_targetName)
      free(m_targetName);
   if (m_childBoneList)
      free(m_childBoneList);
   initialize();
}

/* BoneFaceControl::BoneFaceControl: constructor */
BoneFaceControl::BoneFaceControl()
{
   initialize();
}

/* ~BoneFaceControl::BoneFaceControl: destructor */
BoneFaceControl::~BoneFaceControl()
{
   clear();
}

/* BoneFaceControl::setupBone: setup bone control */
void BoneFaceControl::setupBone(KeyValue *keyValue, const char *keyName, float kvmin, float kvmax, const char *boneName, btVector3 *pos0, btVector3 *pos1, btQuaternion *rot0, btQuaternion *rot1)
{
   clear();
   m_keyValue = keyValue;
   if (keyName == NULL)
      m_keyName = NULL;
   else
      m_keyName = MMDAgent_strdup(keyName);
   m_targetName = MMDAgent_strdup(boneName);
   m_min = kvmin;
   m_max = kvmax;
   m_pos0 = *pos0;
   m_pos1 = *pos1;
   m_rot0 = *rot0;
   m_rot1 = *rot1;
   m_isBone = true;
}

/* BoneFaceControl::setupMorph: setup morph control */
void BoneFaceControl::setupMorph(KeyValue *keyValue, const char *keyName, float kvmin, float kvmax, const char *morphName, float value0, float value1)
{
   clear();
   m_keyValue = keyValue;
   if (keyName == NULL)
      m_keyName = NULL;
   else
      m_keyName = MMDAgent_strdup(keyName);
   m_targetName = MMDAgent_strdup(morphName);
   m_min = kvmin;
   m_max = kvmax;
   m_val0 = value0;
   if (keyName == NULL) {
      /* value1 = duration */
      /* m_val1 = model morph value */
      m_val1 = 0.0;
      m_durationFrame = value1 * 30.0;
   } else {
      /* value1 = max value */
      /* m_val1 = max value */
      m_val1 = value1;
   }
   m_isBone = false;
   m_restFrame = m_durationFrame;
}

/* BoneFaceControl::setModel: reset model reference */
bool BoneFaceControl::setModel(PMDModel *pmd)
{
   if (m_targetName == NULL)
      return false;
   if (m_isBone) {
      m_bone = pmd->getBone(m_targetName);
      if (m_bone == NULL)
         return false;
      if (m_childBoneList)
         free(m_childBoneList);
      m_childBoneList = (PMDBone **)malloc(sizeof(PMDBone *) * pmd->getNumBone());
      m_numChildBone = pmd->getChildBoneList(&m_bone, 1, m_childBoneList);
   } else {
      m_boneMorph = pmd->getBoneMorph(m_targetName);
      if (m_boneMorph) {
         if (m_keyName == NULL) m_val1 = m_boneMorph->getWeight();
         return true;
      }
      m_vertexMorph = pmd->getVertexMorph(m_targetName);
      if (m_vertexMorph) {
         if (m_keyName == NULL) m_val1 = m_vertexMorph->getWeight();
         return true;
      }
      m_uvMorph = pmd->getUVMorph(m_targetName);
      if (m_uvMorph) {
         if (m_keyName == NULL) m_val1 = m_uvMorph->getWeight();
         return true;
      }
      m_materialMorph = pmd->getMaterialMorph(m_targetName);
      if (m_materialMorph) {
         if (m_keyName == NULL) m_val1 = m_materialMorph->getWeight();
         return true;
      }
      m_groupMorph = pmd->getGroupMorph(m_targetName);
      if (m_groupMorph) {
         if (m_keyName == NULL) m_val1 = m_groupMorph->getWeight();
         return true;
      }
      m_face = pmd->getFace(m_targetName);
      if (m_face == NULL) {
         if (m_keyName == NULL) m_val1 = m_face->getWeight();
         return false;
      }
   }
   return true;
}

/* BoneFaceControl::setupSkeleton: set up as skeleton for matching */
void BoneFaceControl::setupSkeleton(const char *name, bool isBone)
{
   clear();
   m_targetName = MMDAgent_strdup(name);
   m_isBone = isBone;
}

/* BoneFaceControl::match: entry matching function */
bool BoneFaceControl::match(BoneFaceControl *d)
{
   if (m_isBone == d->m_isBone && MMDAgent_strequal(m_targetName, d->m_targetName))
      return true;
   return false;
}

/* BoneFaceControl::update: update control */
bool BoneFaceControl::update(double ellapsedFrame)
{
   float value;
   float f, v;
   btVector3 pos;
   btQuaternion rot;

   if (m_keyValue == NULL)
      return false;

   // if no keyName is given, always force the value0 with change duration = value1 sec.
   if (m_keyName == NULL) {
      if (m_isBone) {
         pos = m_pos0;
         rot = m_rot0;
      } else {
         v = m_val0;
      }
      // smearing
      if (m_restFrame > 0.0 && m_durationFrame > 0.0) {
         double r = m_restFrame / m_durationFrame;
         v = m_val1 * (float)r + m_val0 * (float)(1.0 - r);
         m_restFrame -= ellapsedFrame;
         if (m_restFrame < 0.0)
            m_restFrame = 0.0;
      }
   } else {
      value = (float)atof(m_keyValue->getString(m_keyName, "0.0"));
      if (value > m_max)
         f = 1.0f;
      else if (value < m_min)
         f = 0.0f;
      else
         f = (value - m_min) / (m_max - m_min);
      if (m_isBone) {
         pos = m_pos0.lerp(m_pos1, btScalar(f));
         rot = m_rot0.slerp(m_rot1, btScalar(f));
      } else {
         v = (1.0f - f) * m_val0 + f * m_val1;
      }
   }
   if (m_isBone) {
      m_bone->setCurrentPosition(&pos);
      m_bone->setCurrentRotation(&rot);
      m_bone->update();
      for (int i = 0; i < m_numChildBone; i++)
         m_childBoneList[i]->update();
   } else {
      if (m_boneMorph)
         m_boneMorph->setWeight(v);
      if (m_vertexMorph)
         m_vertexMorph->setWeight(v);
      if (m_uvMorph)
         m_uvMorph->setWeight(v);
      if (m_materialMorph)
         m_materialMorph->setWeight(v);
      if (m_groupMorph)
         m_groupMorph->setWeight(v);
      if (m_face)
         m_face->setWeight(v);
   }

   return true;
}
