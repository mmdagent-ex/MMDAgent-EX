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
#include "julius/juliuslib.h"
#include "Julius_Thread.h"
#include "AudioLipSync.h"

// speed for morph changing, set larger value for faster transition
#define SMEARINGRATEFACE 0.40f

// Julius jconf file for lip sync
#define JCONF_NAME "jconf_phone.txt"

// mouth shape last aging frame length
#define MOUTH_SHAPE_LAST_AGING_FRAME_LENGTH 10.0f

// shapemap entry name for lip sync
static const char *lipNames[] = {
   "LIP_A",
   "LIP_I",
   "LIP_U",
   "LIP_O"
};

// AudioLipSync::initialize: initialize instance
void AudioLipSync::initialize()
{
   m_name = NULL;
   m_obj = NULL;
   m_pmd = NULL;
   m_enable = false;

   m_messageBuf = NULL;
   m_messageBufMaxNum = 0;

   m_mouseAgingFrameLeft = 0.0f;
   m_hasLipSyncData = true;

   m_julius_thread = NULL;

   m_shapemap = NULL;
   m_mq = NULL;

   for (int i = 0; i < LIP_NUM; i++)
      m_lipControl[i].clear();
}

// AudioLipSync::clear: clear instance
void AudioLipSync::clear()
{
   if (m_messageBuf) {
      for (int i = 0; i < m_messageBufMaxNum; i++)
         if (m_messageBuf[i])
            free(m_messageBuf[i]);
      free(m_messageBuf);
   }

   if (m_name)
      free(m_name);

   if (m_mq)
      delete m_mq;

   initialize();
}

// AudioLipSync::resetFaceTarget: reset face target
void AudioLipSync::resetFaceTarget()
{
   for (int i = 0; i < LIP_NUM; i++)
      m_lipControl[i].resetTarget();
}

/* AudioLipSync::AudioLipSync: constructor */
AudioLipSync::AudioLipSync()
{
   initialize();
}

/* AudioLipSync::~AudioLipSync: destructor */
AudioLipSync::~AudioLipSync()
{
   /* stop Julius */
   if (m_julius_thread)
      m_julius_thread->stopAndRelease();
   clear();
}

// AudioLipSync::assignModel: assign model
bool AudioLipSync::assignModel(const char *alias)
{
   PMDObject *obj;
   PMDModel *pmd;
   int id;
   PMDObject *modellist;

   if (m_mmdagent == NULL)
      return false;

   /* store specified model alias even if failed */
   if (alias) {
      if (m_name)
         free(m_name);
      m_name = MMDAgent_strdup(alias);
   }

   modellist = m_mmdagent->getModelList();

   obj = NULL;
   pmd = NULL;

   // find model of the given alias
   id = m_mmdagent->findModelAlias(m_name);
   if (id >= 0) {
      obj = &(modellist[id]);
      pmd = modellist[id].getPMDModel();
   }

   // if not specified or not found, return with error
   if (pmd == NULL) {
      m_obj = NULL;
      m_pmd = NULL;
      return FALSE;
   }

   if (pmd->getNumBone() <= 0)
      return false;

   resetFaceTarget();

   m_shapemap = obj->getShapeMap();

   for (int i = 0; i < LIP_NUM; i++)
      m_lipControl[i].clear();
   if (m_shapemap) {
      // set up lip controller
      for (int i = 0; i < LIP_NUM; i++) {
         PMDFaceInterface *f = m_shapemap->getLipMorph(lipNames[i]);
         if (f)
            m_lipControl[i].set(f, 0.0f);
      }
   }

   m_obj = obj;
   m_pmd = pmd;

   return true;
}

// AudioLipSync::setup: setup control for a model
bool AudioLipSync::setup(MMDAgent *mmdagent, int id, bool wantLocal, bool wantPassthrough)
{
   m_mmdagent = mmdagent;
   m_id = id;

   clear();

   /* start Julius */
   startJulius(JCONF_NAME, wantLocal, wantPassthrough);

   /* send udp Port num to remote end */
   if (m_mq)
      delete m_mq;
   m_mq = new MouthQueue();

   return true;
}

// AudioLipSync::start: start
void AudioLipSync::start()
{
   m_enable = true;
   if (m_obj) m_obj->unlock();
}

// AudioLipSync::stop: stop
void AudioLipSync::stop()
{
   m_enable = false;
   resetFaceTarget();
   if (m_obj) m_obj->unlock();
}

// AudioLipSync::update: apply controlled motions to bones and morphs
void AudioLipSync::update(float deltaFrame)
{
   float rate;

   if (m_obj) {
      // detect model change and re-assign if model change has occured
      m_obj->lock();
      if (m_pmd != m_obj->getPMDModel())
         assignModel(NULL);
   } else {
      // model was set but not loaded yet, try assigning
      if (m_name)
         assignModel(NULL);
   }

   // get face changing rate
   rate = SMEARINGRATEFACE * deltaFrame;
   if (rate > 1.0f) rate = 1.0f;
   if (rate < 0.0f) rate = 0.0f;

   // set mouth shape from auto lipsync queue
   setCurrentMouthShape(deltaFrame);

   if (m_pmd == NULL) {
      if (m_obj) m_obj->unlock();
      return;
   }

   if (m_hasLipSyncData == true) {
      m_lipControl[LIP_A].resetWeight();
      m_lipControl[LIP_I].resetWeight();
      m_lipControl[LIP_U].resetWeight();
      m_lipControl[LIP_O].resetWeight();
      m_lipControl[LIP_A].addWeight(rate);
      m_lipControl[LIP_I].addWeight(rate);
      m_lipControl[LIP_U].addWeight(rate);
      m_lipControl[LIP_O].addWeight(rate);
      m_lipControl[LIP_A].apply();
      m_lipControl[LIP_I].apply();
      m_lipControl[LIP_U].apply();
      m_lipControl[LIP_O].apply();
   }

   if (m_obj) m_obj->unlock();
}

// AudioLipSync::storeMouthShape: store mouth shape
void AudioLipSync::storeMouthShape(const char *phonestr)
{
   float rate[4];

   if (m_pmd == NULL)
      return;

   if (m_shapemap == NULL)
      return;

   // O = FACE_A, 1 = FACE_I, 2 = FACE_U, 3 = FACE_O
   rate[0] = 0.0f;
   rate[1] = 0.0f;
   rate[2] = 0.0f;
   rate[3] = 0.0f;

   if (m_mq->getFramesPerIndex() == 0.0) {
      m_mq->setup(m_mmdagent, m_id, AUDIO_PLAY_BUFFER_SIZE_IN_SEC * 1000, m_julius_thread->getFrameIntervalMSec());
   }

   if (phonestr == NULL) {
      m_mq->enqueue(rate);
      return;
   }

   switch (phonestr[0]) {
   case 'a':
      rate[0] = 0.5f;
      rate[1] = 0.0f;
      rate[2] = 0.0f;
      rate[3] = 0.0f;
      break;
   case 'i':
      rate[0] = 0.0f;
      rate[1] = 0.4f;
      rate[2] = 0.0f;
      rate[3] = 0.0f;
      break;
   case 'u':
      rate[0] = 0.0f;
      rate[1] = 0.0f;
      rate[2] = 1.0f;
      rate[3] = 0.0f;
      break;
   case 'e':
      rate[0] = 0.1f;
      rate[1] = 0.6f;
      rate[2] = 0.2f;
      rate[3] = 0.0f;
      break;
   case 'o':
      rate[0] = 0.0f;
      rate[1] = 0.0f;
      rate[2] = 0.0f;
      rate[3] = 0.8f;
      break;
   case 'f':
   case 'w':
      rate[0] = 0.0f;
      rate[1] = 0.0f;
      rate[2] = 0.5f;
      rate[3] = 0.0f;
      break;
   case 'h':
   case 'j':
      rate[0] = 0.0f;
      rate[1] = 0.0f;
      rate[2] = 0.3f;
      rate[3] = 0.0f;
      break;
   case 'y':
   case 'b':
   case 'd':
   case 'g':
   case 'k':
   case 'r':
   case 'z':
      rate[0] = 0.1f;
      rate[1] = 0.0f;
      rate[2] = 0.0f;
      rate[3] = 0.0f;
      break;
   case 't':
      if (phonestr[1] == 'h') {
         rate[0] = 0.0f;
         rate[1] = 0.3f;
         rate[2] = 0.0f;
         rate[3] = 0.0f;
      } else {
         rate[0] = 0.1f;
         rate[1] = 0.0f;
         rate[2] = 0.0f;
         rate[3] = 0.0f;
      }
      break;
   }

   rate[0] *= 1.4f; if (rate[0] > 1.0f) rate[0] = 1.0f;
   rate[1] *= 1.4f; if (rate[1] > 1.0f) rate[1] = 1.0f;
   rate[2] *= 1.4f; if (rate[2] > 1.0f) rate[2] = 1.0f;
   rate[3] *= 1.4f; if (rate[3] > 1.0f) rate[3] = 1.0f;

   if (m_mq)
      m_mq->enqueue(rate);
}

// AudioLipSync::setCurrentMouthShape: set current mouth shape
void AudioLipSync::setCurrentMouthShape(double ellapsedFrame)
{
   float rate[4];

   m_hasLipSyncData = false;

   if (m_shapemap == NULL)
      return;

   if (m_mq == NULL)
      return;

   if (m_mq->getFramesPerIndex() == 0.0)
      return;

   if (m_mq->dequeue(rate, ellapsedFrame) == false) {
      /* when no more mouth shape is given, slowly close the mouth */
      m_mouseAgingFrameLeft -= (float)ellapsedFrame;
      if (m_mouseAgingFrameLeft < 0.0f)
         m_mouseAgingFrameLeft = 0.0f;
      float r = m_mouseAgingFrameLeft / MOUTH_SHAPE_LAST_AGING_FRAME_LENGTH;
      m_lipControl[LIP_A].setTarget(m_mouseTargetRate[0] * r);
      m_lipControl[LIP_I].setTarget(m_mouseTargetRate[1] * r);
      m_lipControl[LIP_U].setTarget(m_mouseTargetRate[2] * r);
      m_lipControl[LIP_O].setTarget(m_mouseTargetRate[3] * r);
      if (
         m_mouseTargetRate[0] * r < 0.1f &&
         m_mouseTargetRate[1] * r < 0.1f &&
         m_mouseTargetRate[2] * r < 0.1f &&
         m_mouseTargetRate[3] * r < 0.1f) {
         m_hasLipSyncData = false;
      }
      else {
         m_hasLipSyncData = true;
      }
   } else {
      /* set the rate */
      m_mouseTargetRate[0] = rate[0];
      m_mouseTargetRate[1] = rate[1];
      m_mouseTargetRate[2] = rate[2];
      m_mouseTargetRate[3] = rate[3];
      m_mouseAgingFrameLeft = MOUTH_SHAPE_LAST_AGING_FRAME_LENGTH;
      m_lipControl[LIP_A].setTarget(m_mouseTargetRate[0]);
      m_lipControl[LIP_I].setTarget(m_mouseTargetRate[1]);
      m_lipControl[LIP_U].setTarget(m_mouseTargetRate[2]);
      m_lipControl[LIP_O].setTarget(m_mouseTargetRate[3]);
      if (
         m_mouseTargetRate[0] < 0.1f &&
         m_mouseTargetRate[1] < 0.1f &&
         m_mouseTargetRate[2] < 0.1f &&
         m_mouseTargetRate[3] < 0.1f) {
         m_hasLipSyncData = false;
      }
      else {
         m_hasLipSyncData = true;
      }
   }
}

// AudioLipSync::startJulius: start Julius thread
void AudioLipSync::startJulius(const char *conffile, bool wantLocal, bool wantPassthrough)
{
   char configFile[MMDAGENT_MAXBUFLEN];

   /* config file */
   MMDAgent_snprintf(configFile, MMDAGENT_MAXBUFLEN, "%s%c%s%c%s", m_mmdagent->getAppDirName(), MMDAGENT_DIRSEPARATOR, "Julius", MMDAGENT_DIRSEPARATOR, conffile);

   /* load models and start thread */
   if (m_julius_thread == NULL)
      m_julius_thread = new Julius_Thread();
   m_julius_thread->loadAndStart(m_mmdagent, m_id, configFile, this, wantLocal, wantPassthrough);
}

// AudioLipSync::processSoundData: process sound data
void AudioLipSync::processSoundData(const char *data, int len)
{
   /* would not play&lipsync sent audio when no model is set up */
   if (m_obj == NULL)
      return;
   /* send the internally given audio chunk to Julius adin thread */
   m_julius_thread->processAudio(data, len);
}

// AudioLipSync::getStreamingSoundDataFlag: get streaming sound data flag
bool AudioLipSync::getStreamingSoundDataFlag()
{
   return m_julius_thread->getStreamingFlag();
}

// AudioLipSync::setStreamingSoundDataFlag: set streaming sound data flag
void AudioLipSync::setStreamingSoundDataFlag(bool flag) {
   m_julius_thread->setStreamingFlag(flag);
}

// AudioLipSync::segmentSoundData: segment the current sound data
void AudioLipSync::segmentSoundData()
{
   m_julius_thread->segmentAudio();
}

// AudioLipSync::waitAudioThreadStart: wait audio thread start
void AudioLipSync::waitAudioThreadStart()
{
   m_julius_thread->waitRunning();
}
