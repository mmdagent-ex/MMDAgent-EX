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

// Bone control information
class BoneControl
{
private:
   PMDBone *m_targetBone;
   btVector3 m_basePos;
   btVector3 m_targetPos;
   btVector3 m_currentPos;
   btQuaternion m_baseRot;
   btQuaternion m_targetRot;
   btQuaternion m_currentRot;
   int m_lastNum;
   bool m_enableCalibration;

   bool m_calibrationResetting;
   btVector3 m_basePosLast;
   btQuaternion m_baseRotLast;

   // initialize: initialize instance
   void initialize();

public:
   // constructor
   BoneControl();

   // constructor
   BoneControl(PMDBone *b, bool needsCalibration);

   // destructor
   ~BoneControl();

   // clear: clear instance
   void clear();

   // setup: setup
   void setup(PMDBone *b, bool needsCalibration);

   // resetTarget: reset target pos/rot
   void resetTarget();

   // fetchCurrentPosition: fetch current position of the bone to this controller
   void fetchCurrentPosition();

   // fetchCurrentRotation: fetch current rotation of the bone to this controller
   void fetchCurrentRotation();

   // setTargetPosition: set target position
   void setTargetPosition(btVector3 pos);

   // setTargetRotation: set target rotation
   void setTargetRotation(btQuaternion rot);

   // getTargetPosition: get target position
   void getTargetPosition(btVector3 *pos);

   // getTargetRotation: get target rotation
   void getTargetRotation(btQuaternion *rot);

   // resetCalibration: reset calibration
   void resetCalibration();

   // doCalibrateTarget: perform calibration using current context
   void doCalibrateTarget();

   // doMirror: swap left and right
   void doMirror();

   // applyPosition: apply the current position to bone
   void applyPosition(float smearingCoef, float brendRate);

   // applyRotation: apply the current rotation to bone
   void applyRotation(float smearingCoef, float brendRate);

   // addPosition: add the current position to bone
   void addPosition(float smearingCoef);

   // update: update the bone
   void update();
};

// Bone controller set
class BoneControlSet
{
private:
   PTree *m_index;

   // initialize: initialize instance
   void initialize();

   // clear: clear instance
   void clear();

public:
   // constructor
   BoneControlSet();

   // destructor
   ~BoneControlSet();

   // setup: setup
   void setup();

   // set: set control for the shape name on the model
   BoneControl *set(const char *shapeName, PMDBone *bone, btVector3 pos, btQuaternion rot, bool needsCalibration);

   /* find: find morphinfo in the index */
   BoneControl *find(const char *shapeName);

   // resetTargets: reset all targets
   void resetTargets();

   // fetchCurrentPositions: fetch all current position of the bone to this controller
   void fetchCurrentPositions();

   // fetchCurrentRotations: fetch all current rotation of the bone to this controller
   void fetchCurrentRotations();

   // resetCalibrations: reset all calibrations
   void resetCalibrations();

   // doCalibrateTargets: perform all calibration using current context
   void doCalibrateTargets();

   // doMirrors: swap left and right for all bones
   void doMirrors();

   // applyPositions: apply the current position to all bones
   void applyPositions(float smearingCoef, float brendRate);

   // applyRotations: apply the current rotation to all bones
   void applyRotations(float smearingCoef, float brendRate);

   // updates: update all bones
   void updates();

};
