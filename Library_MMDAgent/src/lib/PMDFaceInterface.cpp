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

/* headers */

#include "MMDAgent.h"

/* sigmoid table resolution */
#define SIGMOID_BINS 200
/* sigmoid table intensity (larger value makes sharpner gradient) */
#define SIGMOID_COEF 30.0f
/* floor value */
#define SIGMOID_EPSILON 0.00001

/* PMDFaceInterface::initialize: initialize PMDFaceInterface */
void PMDFaceInterface::initialize()
{
   m_pmd = NULL;
   for (int i = 0; i < PMDFACEINTERFACE_MAXMORPHASSIGNNUM; i++) {
      m_face[i] = NULL;
      m_boneMorph[i] = NULL;
      m_vertexMorph[i] = NULL;
      m_uvMorph[i] = NULL;
      m_materialMorph[i] = NULL;
      m_groupMorph[i] = NULL;
      m_faceRate[i] = 1.0f;
      m_boneMorphRate[i] = 1.0f;
      m_vertexMorphRate[i] = 1.0f;
      m_uvMorphRate[i] = 1.0f;
      m_materialMorphRate[i] = 1.0f;
      m_groupMorphRate[i] = 1.0f;
   }
   m_faceNum = m_boneMorphNum = m_vertexMorphNum = m_uvMorphNum = m_materialMorphNum = m_groupMorphNum = 0;
   m_valid = false;
   m_weight = 0.0f;
   m_sigmoidTable = NULL;
}

/* PMDFaceInterface::clear: free PMDFaceInterface */
void PMDFaceInterface::clear()
{
   if (m_sigmoidTable)
      free(m_sigmoidTable);
   initialize();
}

/* PMDFaceInterface::makeSigmoidTable: make sigmoid table */
void PMDFaceInterface::makeSigmoidTable()
{
   int i;
   float x;
   float f;

   if (m_sigmoidTable)
      return;

   m_sigmoidTable = (float *)malloc(sizeof(float) * SIGMOID_BINS);

   for (i = 0; i < SIGMOID_BINS; i++) {
      x = 1.0f * i / (float)SIGMOID_BINS - 0.5f;
      f = 1.0f / (1.0f + expf(-x * SIGMOID_COEF));
      if (f <= SIGMOID_EPSILON)
         f = 0.0f;
      if (f >= 1.0f - SIGMOID_EPSILON)
         f = 1.0f;
      m_sigmoidTable[i] = f;
   }
}

/* PMDFaceInterface::getSigmoidValue: get sigmoid value */
float PMDFaceInterface::getSigmoidValue(float value, float thres)
{
   int idx;
   float x;

   x = value - thres + 0.5f;

   if (x < 0.0f)
      return 0.0f;
   if (x > 1.0f)
      return 1.0f;

   idx = (int)(x * (float)SIGMOID_BINS);
   return m_sigmoidTable[idx];
}

/* PMDFaceInterface::PMDFaceInterface: constructor */
PMDFaceInterface::PMDFaceInterface()
{
   initialize();
}


/* PMDFaceInterface::PMDFaceInterface: constructor */
PMDFaceInterface::PMDFaceInterface(PMDModel *pmd, const char *name, float rate, bool thres)
{
   initialize();
   set(pmd, name, rate, thres);
}

/* PMDFaceInterface::PMDFaceInterface: destructor */
PMDFaceInterface::~PMDFaceInterface()
{
   clear();
}

/* PMDFaceInterface::set: set */
bool PMDFaceInterface::set(PMDModel *pmd, const char *name, float rate, bool thres)
{
   clear();

   if (pmd == NULL || name == NULL) {
      m_valid = false;
      return false;
   }
   m_valid = add(pmd, name, rate, thres);

   return m_valid;
}

/* PMDFaceInterface::add: add */
bool PMDFaceInterface::add(PMDModel *pmd, const char *name, float rate, bool thres)
{
   float val;

   if (pmd == NULL || name == NULL)
      return false;

   if (rate < 0.0f)
      return false;

   if (thres)
      val = -rate;
   else
      val = rate;

   if (thres)
      makeSigmoidTable();

   m_pmd = pmd;

   if (m_boneMorphNum < PMDFACEINTERFACE_MAXMORPHASSIGNNUM) {
      if ((m_boneMorph[m_boneMorphNum] = m_pmd->getBoneMorph(name))) {
         m_boneMorphRate[m_boneMorphNum] = val;
         m_boneMorphNum++;
         return true;
      }
   }
   if (m_vertexMorphNum < PMDFACEINTERFACE_MAXMORPHASSIGNNUM) {
      if ((m_vertexMorph[m_vertexMorphNum] = m_pmd->getVertexMorph(name))) {
         m_vertexMorphRate[m_vertexMorphNum] = val;
         m_vertexMorphNum++;
         return true;
      }
   }
   if (m_uvMorphNum < PMDFACEINTERFACE_MAXMORPHASSIGNNUM) {
      if ((m_uvMorph[m_uvMorphNum] = m_pmd->getUVMorph(name))) {
         m_uvMorphRate[m_uvMorphNum] = val;
         m_uvMorphNum++;
         return true;
      }
   }
   if (m_materialMorphNum < PMDFACEINTERFACE_MAXMORPHASSIGNNUM) {
      if ((m_materialMorph[m_materialMorphNum] = m_pmd->getMaterialMorph(name))) {
         m_materialMorphRate[m_materialMorphNum] = val;
         m_materialMorphNum++;
         return true;
      }
   }
   if (m_groupMorphNum < PMDFACEINTERFACE_MAXMORPHASSIGNNUM) {
      if ((m_groupMorph[m_groupMorphNum] = m_pmd->getGroupMorph(name))) {
         m_groupMorphRate[m_groupMorphNum] = val;
         m_groupMorphNum++;
         return true;
      }
   }
   if (m_faceNum < PMDFACEINTERFACE_MAXMORPHASSIGNNUM) {
      if ((m_face[m_faceNum] = m_pmd->getFace(name))) {
         m_faceRate[m_faceNum] = val;
         m_faceNum++;
         return true;
      }
   }
   return false;
}

/* PMDFaceInterface::isValid: true when valid */
bool PMDFaceInterface::isValid()
{
   return m_valid;
}

/* PMDFaceInterface::computeWeight: compute weight */

float PMDFaceInterface::computeWeight(float value, float rate)
{
   float f;

   if (rate < 0.0f) {
      f = getSigmoidValue(value, -rate);
   } else {
      f = value * rate;
      if (f > 1.0f)
         f = 1.0f;
      else if (f < 0.0f)
         f = 0.0f;
   }
   return f;
}

/* PMDFaceInterface::resetWeight: reset weight */
void PMDFaceInterface::resetWeight()
{
   int i;

   if (m_valid == false)
      return;

   // reset local weight sum
   m_weight = 0.0f;

   // reset model weights
   for (i = 0; i < m_boneMorphNum; i++)
      m_boneMorph[i]->reset();
   for (i = 0; i < m_groupMorphNum; i++) {
      if (m_groupMorph[i]->getHasBoneFlag() == true) {
         for (PMDGroupMorphElem *p = m_groupMorph[i]->getList(); p; p = p->next) {
            if (p->b)
               p->b->reset();
         }
      }
   }
}

/* PMDFaceInterface::addWeight: add weight */
void PMDFaceInterface::addWeight(float value)
{
   m_weight += value;
}

/* PMDFaceInterface::apply: apply added weight */
void PMDFaceInterface::apply()
{
   int i;

   for (i = 0; i < m_boneMorphNum; i++) {
      m_boneMorph[i]->setWeight(computeWeight(m_weight, m_boneMorphRate[i]));
      m_boneMorph[i]->apply();
   }
   for (i = 0; i < m_vertexMorphNum; i++)
      m_vertexMorph[i]->setWeight(computeWeight(m_weight, m_vertexMorphRate[i]));
   for (i = 0; i < m_uvMorphNum; i++)
      m_uvMorph[i]->setWeight(computeWeight(m_weight, m_uvMorphRate[i]));
   for (i = 0; i < m_materialMorphNum; i++)
      m_materialMorph[i]->setWeight(computeWeight(m_weight, m_materialMorphRate[i]));
   for (i = 0; i < m_groupMorphNum; i++) {
      m_groupMorph[i]->setWeight(computeWeight(m_weight, m_groupMorphRate[i]));
      for (PMDGroupMorphElem *p = m_groupMorph[i]->getList(); p; p = p->next) {
         if (p->b) {
            float w = p->b->getWeight();
            p->b->setWeight(m_groupMorph[i]->getWeight() * p->rate);
            p->b->apply();
            p->b->setWeight(w);
         }
      }
   }
   for (i = 0; i < m_faceNum; i++)
      m_face[i]->setWeight(computeWeight(m_weight, m_faceRate[i]));
}

/* PMDFaceInterface::getAssignedWeight: get assigned weight */
float PMDFaceInterface::getAssignedWeight()
{
   if (m_boneMorphNum > 0)
      return m_boneMorph[0]->getWeight();
   if (m_vertexMorphNum > 0)
      return m_vertexMorph[0]->getWeight();
   if (m_uvMorphNum > 0)
      return m_uvMorph[0]->getWeight();
   if (m_materialMorphNum > 0)
      return m_materialMorph[0]->getWeight();
   if (m_groupMorphNum > 0)
      return m_groupMorph[0]->getWeight();
   if (m_faceNum > 0)
      return m_face[0]->getWeight();
   return 0.0f;
}

/* PMDFaceInterface::forceAssignedWeight: force assigned weight */
void PMDFaceInterface::forceAssignedWeight(float value)
{
   if (m_boneMorphNum > 0)
      m_boneMorph[0]->setWeight(value);
   if (m_vertexMorphNum > 0)
      m_vertexMorph[0]->setWeight(value);
   if (m_uvMorphNum > 0)
      m_uvMorph[0]->setWeight(value);
   if (m_materialMorphNum > 0)
      m_materialMorph[0]->setWeight(value);
   if (m_groupMorphNum > 0)
      m_groupMorph[0]->setWeight(value);
   if (m_faceNum > 0)
      m_face[0]->setWeight(value);
}
