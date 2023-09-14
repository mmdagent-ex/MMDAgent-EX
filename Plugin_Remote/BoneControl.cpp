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
#include "BoneControl.h"

// number of arrived frames to be used for head rotation adjustment
#define FIRSTHEADADJUSTFRAMES 10

// BoneControl::initialize: initialize instance
void BoneControl::initialize()
{
   m_targetBone = NULL;
   m_basePos.setZero();
   m_targetPos.setZero();
   m_currentPos.setZero();
   m_baseRot.setValue(btScalar(0.0f), btScalar(0.0f), btScalar(0.0f), btScalar(1.0f));
   m_targetRot.setValue(btScalar(0.0f), btScalar(0.0f), btScalar(0.0f), btScalar(1.0f));
   m_currentRot.setValue(btScalar(0.0f), btScalar(0.0f), btScalar(0.0f), btScalar(1.0f));
   m_lastNum = 0;
   m_enableCalibration = false;

   m_calibrationResetting = false;
   m_basePosLast.setZero();
   m_baseRotLast.setValue(btScalar(0.0f), btScalar(0.0f), btScalar(0.0f), btScalar(1.0f));
}

// BoneControl::clear: clear instance
void BoneControl::clear()
{
   initialize();
}

// constructor
BoneControl::BoneControl()
{
   initialize();
}

// constructor
BoneControl::BoneControl(PMDBone *b, bool needsCalibration)
{
   setup(b, needsCalibration);
}

// destructor
BoneControl::~BoneControl()
{
   clear();
}

// BoneControl::setup: setup
void BoneControl::setup(PMDBone *b, bool needsCalibration)
{
   initialize();
   m_targetBone = b;
   m_enableCalibration = needsCalibration;
}

// BoneControl::resetTarget: reset target pos/rot
void BoneControl::resetTarget()
{
   m_targetPos.setZero();
   m_targetRot.setValue(btScalar(0.0f), btScalar(0.0f), btScalar(0.0f), btScalar(1.0f));
}

// BoneControl::fetchCurrentPosition: fetch current position of the bone to this controller
void BoneControl::fetchCurrentPosition()
{
   if (m_targetBone)
      m_targetBone->getCurrentPosition(&m_currentPos);
}

// BoneControl::fetchCurrentRotation: fetch current rotation of the bone to this controller
void BoneControl::fetchCurrentRotation()
{
   if (m_targetBone)
      m_targetBone->getCurrentRotation(&m_currentRot);
}

// BoneControl::setTargetPosition: set target position
void BoneControl::setTargetPosition(btVector3 pos)
{
   m_targetPos = pos;
}

// BoneControl::setTargetRotation: set target rotation
void BoneControl::setTargetRotation(btQuaternion rot)
{
   m_targetRot = rot;
}

// BoneControl::getTargetPosition: get target position
void BoneControl::getTargetPosition(btVector3 *pos)
{
   *pos = m_targetPos;
}

// BoneControl::getTargetRotation: get target rotation
void BoneControl::getTargetRotation(btQuaternion *rot)
{
   *rot = m_targetRot;
}

// BoneControl::resetCalibration: reset calibration
void BoneControl::resetCalibration()
{
   m_lastNum = 0;
   m_calibrationResetting = true;
   m_basePosLast = m_basePos;
   m_baseRotLast = m_baseRot;
}

// BoneControl::doCalibrateTarget: perform calibration using current context
void BoneControl::doCalibrateTarget()
{
   if (m_lastNum == 0) {
      // start of calibration
      m_basePos = m_targetPos;
      m_baseRot = m_targetRot;
      m_lastNum++;
   } else if (m_lastNum < FIRSTHEADADJUSTFRAMES) {
      // calibration in progress
      m_lastNum++;
      btScalar rate = 1.0f / ((float)m_lastNum);
      m_basePos = m_basePos.lerp(m_targetPos, rate);
      m_baseRot = m_baseRot.slerp(m_targetRot, rate);
      if (m_lastNum >= FIRSTHEADADJUSTFRAMES) {
         // just reached end of calibration
         m_baseRot = m_baseRot.inverse();
         fetchCurrentRotation();
         m_calibrationResetting = false;
      }
   }
   if (m_lastNum >= FIRSTHEADADJUSTFRAMES) {
      // perform calibration
      m_targetPos -= m_basePos;
      m_targetRot = m_targetRot * m_baseRot;
   } else if (m_calibrationResetting) {
      // perform calibration with last base while updating
      m_targetPos -= m_basePosLast;
      m_targetRot = m_targetRot * m_baseRotLast;
   }
}

// BoneControl::doMirror: swap left and right
void BoneControl::doMirror()
{
   m_targetPos.setX(-m_targetPos.getX());
   m_targetRot.setW(-m_targetRot.getW());
}

// BoneControl::applyPosition: apply the current position to bone
void BoneControl::applyPosition(float smearingCoef, float brendRate)
{
   if (m_enableCalibration && m_lastNum < FIRSTHEADADJUSTFRAMES && m_calibrationResetting == false) return;
   m_currentPos = m_currentPos.lerp(m_targetPos, btScalar(smearingCoef));
   if (m_targetBone) {
      if (brendRate == 1.0f) {
         m_targetBone->setCurrentPosition(&m_currentPos);
      } else {
         btVector3 pos;
         m_targetBone->getCurrentPosition(&pos);
         pos = pos.lerp(m_currentPos, brendRate);
         m_targetBone->setCurrentPosition(&pos);
      }
   }
}

// BoneControl::applyRotation: apply the current rotation to bone
void BoneControl::applyRotation(float smearingCoef, float brendRate)
{
   if (m_enableCalibration && m_lastNum < FIRSTHEADADJUSTFRAMES && m_calibrationResetting == false) return;
   m_currentRot = m_currentRot.slerp(m_targetRot, btScalar(smearingCoef));
   if (m_targetBone) {
      if (brendRate == 1.0f) {
         m_targetBone->setCurrentRotation(&m_currentRot);
      } else {
         btQuaternion rot;
         m_targetBone->getCurrentRotation(&rot);
         rot = rot.slerp(m_currentRot, brendRate);
         m_targetBone->setCurrentRotation(&rot);
      }
   }
}

// BoneControl::add: add the current position to bone
void BoneControl::addPosition(float smearingCoef)
{
   if (m_enableCalibration && m_lastNum < FIRSTHEADADJUSTFRAMES && m_calibrationResetting == false) return;
   m_currentPos = m_currentPos.lerp(m_targetPos, btScalar(smearingCoef));
   if (m_targetBone) {
      btVector3 v;
      m_targetBone->getCurrentPosition(&v);
      v += m_currentPos;
      m_targetBone->setCurrentPosition(&v);
   }
}

// BoneControl::update: update the bone
void BoneControl::update()
{
   if (m_targetBone)
      m_targetBone->update();
}

// BoneControlSet::initialize: initialize instance
void BoneControlSet::initialize()
{
   m_index = NULL;
}

// BoneControlSet::clear: clear instance
void BoneControlSet::clear()
{
   void *save;
   BoneControl *b;

   if (m_index) {
      for (b = (BoneControl *)m_index->firstData(&save); b; b = (BoneControl *)m_index->nextData(&save)) {
         delete b;
      }
      delete m_index;
      m_index = NULL;
   }
   initialize();
}

// constructor
BoneControlSet::BoneControlSet()
{
   initialize();
}

// destructor
BoneControlSet::~BoneControlSet()
{
   clear();
}

// BoneControlSet::setup: setup
void BoneControlSet::setup()
{
   clear();
   m_index = new PTree();
}

// BoneControlSet::set: set control for the shape name on the model
BoneControl *BoneControlSet::set(const char *shapeName, PMDBone *bone, btVector3 pos, btQuaternion rot, bool needsCalibration)
{
   BoneControl *b;
   int len;

   if (m_index == NULL || bone == NULL)
      return NULL;

   len = MMDAgent_strlen(shapeName);
   if (m_index->search(shapeName, len, (void **)&b) == true) {
      b->setTargetPosition(pos);
      b->setTargetRotation(rot);
   } else {
      b = new BoneControl(bone, needsCalibration);
      b->setTargetPosition(pos);
      b->setTargetRotation(rot);
      m_index->add(shapeName, MMDAgent_strlen(shapeName), (void *)b);
      b->fetchCurrentPosition();
      b->fetchCurrentRotation();
   }
   return b;
}

/* BoneControlSet::find: find BoneControl in the index */
BoneControl *BoneControlSet::find(const char *shapeName)
{
   BoneControl *b;

   if (m_index == NULL)
      return NULL;

   if (m_index->search(shapeName, MMDAgent_strlen(shapeName), (void **)&b) == true)
      return b;

   return NULL;
}

// BoneControlSet::resetTargets: reset all targets
void BoneControlSet::resetTargets()
{
   BoneControl *b;
   void *save;

   if (m_index == NULL)
      return;

   for (b = (BoneControl *)m_index->firstData(&save); b; b = (BoneControl *)m_index->nextData(&save))
      b->resetTarget();
}

// BoneControlSet::fetchCurrentPositions: fetch all current position of the bone to this controller
void BoneControlSet::fetchCurrentPositions()
{
   BoneControl *b;
   void *save;

   if (m_index == NULL)
      return;

   for (b = (BoneControl *)m_index->firstData(&save); b; b = (BoneControl *)m_index->nextData(&save))
      b->fetchCurrentPosition();
}

// BoneControlSet::fetchCurrentRotations: fetch all current rotation of the bone to this controller
void BoneControlSet::fetchCurrentRotations()
{
   BoneControl *b;
   void *save;

   if (m_index == NULL)
      return;

   for (b = (BoneControl *)m_index->firstData(&save); b; b = (BoneControl *)m_index->nextData(&save))
      b->fetchCurrentRotation();
}

// BoneControlSet::resetCalibrations: reset all calibration
void BoneControlSet::resetCalibrations()
{
   BoneControl *b;
   void *save;

   if (m_index == NULL)
      return;

   for (b = (BoneControl *)m_index->firstData(&save); b; b = (BoneControl *)m_index->nextData(&save))
      b->resetCalibration();
}

// BoneControlSet::doCalibrateTargets: perform all calibration using current context
void BoneControlSet::doCalibrateTargets()
{
   BoneControl *b;
   void *save;

   if (m_index == NULL)
      return;

   for (b = (BoneControl *)m_index->firstData(&save); b; b = (BoneControl *)m_index->nextData(&save))
      b->doCalibrateTarget();
}

// BoneControlSet::doMirrors: swap left and right for all bones
void BoneControlSet::doMirrors()
{
   BoneControl *b;
   void *save;

   if (m_index == NULL)
      return;

   for (b = (BoneControl *)m_index->firstData(&save); b; b = (BoneControl *)m_index->nextData(&save))
      b->doMirror();
}

// BoneControlSet::applyPositions: apply the current position to all bones
void BoneControlSet::applyPositions(float smearingCoef, float brendRate)
{
   BoneControl *b;
   void *save;

   if (m_index == NULL)
      return;

   for (b = (BoneControl *)m_index->firstData(&save); b; b = (BoneControl *)m_index->nextData(&save))
      b->applyPosition(smearingCoef, brendRate);
}

// BoneControlSet::applyRotations: apply the current rotation to all bones
void BoneControlSet::applyRotations(float smearingCoef, float brendRate)
{
   BoneControl *b;
   void *save;

   if (m_index == NULL)
      return;

   for (b = (BoneControl *)m_index->firstData(&save); b; b = (BoneControl *)m_index->nextData(&save))
      b->applyRotation(smearingCoef, brendRate);
}

// BoneControlSet::updates: update all bones
void BoneControlSet::updates()
{
   BoneControl *b;
   void *save;

   if (m_index == NULL)
      return;

   for (b = (BoneControl *)m_index->firstData(&save); b; b = (BoneControl *)m_index->nextData(&save))
      b->update();
}
