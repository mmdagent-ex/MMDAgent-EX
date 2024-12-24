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

#include "MouseQueue.h"
#include "MorphControl.h"

// mouse shape queue size in seconds
#define MOUSE_SHAPE_QUEUE_SIZE_SEC  10.0

// morph controller index for lip sync
enum LIPLIST {
   LIP_A,
   LIP_I,
   LIP_U,
   LIP_O,
   LIP_NUM
};

class Julius_Thread;

class AudioLipSync
{
private:

   MMDAgent *m_mmdagent;      // MMDAgent class
   int m_id;                  // Module ID for messaging/logging

   char *m_name;
   PMDObject *m_obj;
   PMDModel *m_pmd;  /* PMD model */
   bool m_enable;

   char **m_messageBuf;
   int m_messageBufMaxNum;

   ShapeMap *m_shapemap;
   MorphControl m_lipControl[LIP_NUM];

   float m_mouseAgingFrameLeft;
   float m_mouseTargetRate[4];

   MouthQueue *m_mq;

   bool m_hasLipSyncData;

   Julius_Thread *m_julius_thread;

   // initialize: initialize instance
   void initialize();

   // clear: clear instance
   void clear();

   // resetFaceTarget: reset face target
   void resetFaceTarget();

   // startJulius: start Julius thread
   void startJulius(const char *conffile, bool wantLocal, bool wantPassthrough);

public:

   // constructor
   AudioLipSync();

   // destructor
   ~AudioLipSync();

   // setup: setup instance
   bool setup(MMDAgent *mmdagent, int id, bool wantLocal, bool wantPassthrough);

   // assignModel: assign model
   bool assignModel(const char *alias);

   // start: start
   void start();

   // stop: stop
   void stop();

   // update: apply controlled motions to bones and morphs
   void update(float deltaFrame);

   // storeMouthShape: store mouth shape
   void storeMouthShape(const char *phonestr);

   // setCurrentMouthShape: set current mouth shape
   void setCurrentMouthShape(double ellapsedFrame);

   // processSoundData: process sound data
   void processSoundData(const char *data, int len, bool requestSegmentAfterPlayed = false);

   // getStreamingSoundDataFlag: get streaming sound data flag
   bool getStreamingSoundDataFlag();

   // setStreamingSoundDataFlag: set streaming sound data flag
   void setStreamingSoundDataFlag(bool flag);

   // segmentSoundData: segment the current sound data
   void segmentSoundData();

   // waitAudioThreadStart: wait audio thread start
   void waitAudioThreadStart();
};
