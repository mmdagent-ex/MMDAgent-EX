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

// Avatar mode handling client

#include "MouseQueue.h"
#include "MorphControl.h"

// avatar message type string
#define AVATAR_MESSAGE "AVATAR"

// avatar switch keyvalue key string
#define AVATAR_SW_KEYNAME "Avatar_mode"

#define LIPSYNC_JULIUS

#define PLUGIN_COMMAND_AVATARCONTROL "AVATAR_CONTROL"
#define PLUGIN_EVENT_AVATARCONTROL "AVATAR_EVENT_CONTROL"
#define PLUGIN_COMMAND_AVATARLOGSAVESTART "AVATAR_LOGSAVE_START"
#define PLUGIN_COMMAND_AVATARLOGSAVESTOP "AVATAR_LOGSAVE_STOP"

// mouse shape queue size in seconds
#define MOUSE_SHAPE_QUEUE_SIZE_SEC  10.0

// rates for head rotation: actual head move are summation of these
// (0.0f, 0.0f, 1.0f) for no additional rotation
// rate for applying body rotation according to given head rotation
#define BODY_ROTATION_COEF 0.5f
// rate for applying neck rotation according to given head rotation
#define NECK_ROTATION_COEF 0.5f
// rate for applying head rotation according to given head rotation
#define HEAD_ROTATION_COEF 0.6f

// coefficient for additional center bone Y transition according to head rotation
#define CENTERBONE_ADDITIONALMOVECOEF_SCALE 3.0f
// coefficient for additional center bone X transition according to head rotation
#define CENTERBONE_ADDITIONALMOVECOEF_SCALE_RELATIVE_X 0.7f


// morph controller index for lip sync
enum LIPLIST {
   LIP_A,
   LIP_I,
   LIP_U,
   LIP_O,
   LIP_NUM
};

// tracking bone controller index
enum BONELIST {
   TRACK_HEAD,
   TRACK_LEFTEYE,
   TRACK_RIGHTEYE,
   TRACK_BOTHEYE,
   TRACK_NECK,
   TRACK_BODY,
   TRACK_NUM
};

// number of action units
#define NUMACTIONUNITS 46

// auto lip disable index
enum DISABLEAUTOLIP {
   DAL_NO,
   DAL_ARKIT,
   DAL_AU,
   DAL_ARKITANDAU,
   DAL_ALWAYS,
   DAL_NUM
};

class Julius_Thread;

class Avatar
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
   MorphControl m_auControl[NUMACTIONUNITS];
   MorphControlSet m_arkit;
   MorphControlSet m_exMorph;

   float m_leavingFrameLeft;
   bool m_issueLeaveEvent;

   float m_mouseAgingFrameLeft;
   float m_mouseTargetRate[4];

   MouthQueue *m_mq;

   float m_faceTrackingFrameLeft;

   int m_disableAutoLipFlag;
   bool m_autoLipFlag;

   float m_idleFrames;
   bool m_idle;

   bool m_noControlFromAudioLipSync;

#ifdef LIPSYNC_JULIUS
   Julius_Thread *m_julius_thread;
#endif /* LIPSYNC_JULIUS */

   // initialize: initialize instance
   void initialize();

   // clear: clear instance
   void clear();

   // resetFaceTarget: reset face target
   void resetFaceTarget();

   // assignModel: assign model
   bool assignModel(const char *alias);

#ifdef LIPSYNC_JULIUS
   // startJulius: start Julius thread
   void startJulius(const char *conffile, bool wantLocal, bool wantPassthrough);
#endif /* LIPSYNC_JULIUS */

public:

   // constructor
   Avatar();

   // destructor
   ~Avatar();

   // setup: setup avatar instance
   bool setup(MMDAgent *mmdagent, int id, bool wantLocal, bool wantPassthrough);

   // processMessage: process message from openface
   bool processMessage(const char *AVString);

   // update: apply controlled motions to bones and morphs
   void update(float deltaFrame);

   // setEnableFlagInternal: set enable flag internal
   void setEnableFlagInternal(bool flag);

   // setEnableFlag: set enable flag
   void setEnableFlag(bool flag);

   // getEnableFlag: get enable flag
   bool getEnableFlag();

   // storeMouthShape: store mouth shape
   void storeMouthShape(const char *phonestr);

   // setCurrentMouthShape: set current mouth shape
   void setCurrentMouthShape(double ellapsedFrame);

   // processSoundData: process sound data
   void processSoundData(const char *data, int len);

   // getStreamingSoundDataFlag: get streaming sound data flag
   bool getStreamingSoundDataFlag();

   // setStreamingSoundDataFlag: set streaming sound data flag
   void setStreamingSoundDataFlag(bool flag);

   // segmentSoundData: segment the current sound data
   void segmentSoundData();

   // waitAudioThreadStart: wait audio thread start
   void waitAudioThreadStart();

   // getMaxVol: get max volume of avatar's speaking since last call
   int getMaxVol();

   // resetIdleTime: reset idle time
   void resetIdleTime();
};
