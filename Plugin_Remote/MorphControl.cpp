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
bool MorphControlSet::set(const char *shapeName, PMDFaceInterface *interface, float value)
{
   MorphControl *m;
   int len;

   if (m_index == NULL || interface == NULL)
      return false;

   len = MMDAgent_strlen(shapeName);
   if (m_index->search(shapeName, len, (void **)&m) == true) {
      m->setTarget(value);
   } else {
      m = new MorphControl(interface, value);
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
