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
#include "MorphControl.h"

// MorphControl::initialize: initialize instance
void MorphControl::initialize()
{
   m_targetFace = NULL;
   m_rateTarget = 0.0f;
   m_rateCurrent = 0.0f;
}

// MorphControl::clear: clear instance
void MorphControl::clear()
{
   initialize();
}

// constructor
MorphControl::MorphControl()
{
   initialize();
}

// constructor
MorphControl::MorphControl(PMDFaceInterface *f, float rateTarget)
{
   initialize();
   set(f, rateTarget);
}

// destructor
MorphControl::~MorphControl()
{
   clear();
}

// MorphControl::set: set
void MorphControl::set(PMDFaceInterface *f, float rateTarget)
{
   m_targetFace = f;
   m_rateTarget = rateTarget;
   m_rateCurrent = 0.0f;
}

// MorphControl::resetTarget: reset target weight
void MorphControl::resetTarget()
{
   m_rateTarget = 0.0f;
}

// MorphControl::setTarget: set target weight
void MorphControl::setTarget(float value)
{
   m_rateTarget = value;
}

// MorphControl::resetWeight: reset face weights before updates
void MorphControl::resetWeight()
{
   if (m_targetFace == NULL) return;
   m_targetFace->resetWeight();
}

// MorphControl::addWeight: add face weights before updates
void MorphControl::addWeight(float smearingCoef)
{
   if (m_targetFace == NULL) return;
   m_rateCurrent = m_rateCurrent * (1.0f - smearingCoef) + m_rateTarget * smearingCoef;
   m_targetFace->addWeight(m_rateCurrent);
}

// MorphControl::applyWeight: apply the added rate to faces
void MorphControl::apply()
{
   if (m_targetFace == NULL) return;
   m_targetFace->apply();
}

// MorphControlSet::initialize: initialize instance
void MorphControlSet::initialize()
{
   m_index = NULL;
}

// MorphControlSet::clear: clear instance
void MorphControlSet::clear()
{
   void *save;
   MorphControl *m;

   if (m_index) {
      for (m = (MorphControl *)m_index->firstData(&save); m; m = (MorphControl *)m_index->nextData(&save)) {
         delete m;
      }
      delete m_index;
      m_index = NULL;
   }
   initialize();
}

// constructor
MorphControlSet::MorphControlSet()
{
   initialize();
}

// destructor
MorphControlSet::~MorphControlSet()
{
   clear();
}

// MorphControlSet::setup: setup
void MorphControlSet::setup()
{
   clear();
   m_index = new PTree();
}

// MorphControlSet::set: set control for the shape name on the model
bool MorphControlSet::set(const char *shapeName, PMDFaceInterface *iface, float value)
{
   MorphControl *m;
   int len;

   if (m_index == NULL || iface == NULL)
      return false;

   len = MMDAgent_strlen(shapeName);
   if (m_index->search(shapeName, len, (void **)&m) == true) {
      m->setTarget(value);
   } else {
      m = new MorphControl(iface, value);
      m_index->add(shapeName, MMDAgent_strlen(shapeName), (void *)m);
   }
   return true;
}

/* MorphControlSet::find: find MorphControl in the index */
MorphControl *MorphControlSet::find(const char *shapeName)
{
   MorphControl *m;

   if (m_index == NULL)
      return NULL;

   if (m_index->search(shapeName, MMDAgent_strlen(shapeName), (void **)&m) == true)
      return m;

   return NULL;
}

// MorphControlSet::resetTargets: reset all targets
void MorphControlSet::resetTargets()
{
   MorphControl *m;
   void *save;

   if (m_index == NULL)
      return;

   for (m = (MorphControl *)m_index->firstData(&save); m; m = (MorphControl *)m_index->nextData(&save)) {
      m->resetTarget();
   }
}

// MorphControlSet::resetWeights: reset all weights
void MorphControlSet::resetWeights()
{
   MorphControl *m;
   void *save;

   if (m_index == NULL)
      return;

   for (m = (MorphControl *)m_index->firstData(&save); m; m = (MorphControl *)m_index->nextData(&save))
      m->resetWeight();
}

// MorphControlSet::addWeights: add face weights before updates
void MorphControlSet::addWeights(float smearingCoef)
{
   MorphControl *m;
   void *save;

   if (m_index == NULL)
      return;

   for (m = (MorphControl *)m_index->firstData(&save); m; m = (MorphControl *)m_index->nextData(&save))
      m->addWeight(smearingCoef);
}

// MorphControlSet::apply: apply the added rates to faces
void MorphControlSet::apply()
{
   MorphControl *m;
   void *save;

   if (m_index == NULL)
      return;

   for (m = (MorphControl *)m_index->firstData(&save); m; m = (MorphControl *)m_index->nextData(&save))
      m->apply();
}
