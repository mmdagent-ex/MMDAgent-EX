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
#include "julius/juliuslib.h"
#include "Julius_Record.h"
#include "Julius_Thread.h"
#include "Avatar.h"

// speed for bone changing, set larger value for faster transition
#define SMEARINGRATEBONE 0.20f

// speed for morph changing, set larger value for faster transition
#define SMEARINGRATEFACE 0.40f

// leaving frame when avatar control is disabled
#define LEAVINGFRAMES 25.0f

// leaving frame after the last face tracking message
#define FACETRACKINGLEAVINGFRAMES 30.0f

// Julius jconf file for lip sync
#define JCONF_NAME "jconf_phone.txt"

// millimeter to MMD scale coef (MMD scale; 1 unit = 8cm)
#define MM2MMD_COEF 0.0125f

// leaning coef: Z pos to arctan(Z/2.4) (rad) linear approx coef
// 0.4f most fits, but we want to have extra move
#define POS2TAN_COEF 0.6f

// leadning coef: back leaning maximum (10 degree)
#define BACK_LEANING_MAXIMUM_RAD 0.17f

// mouth shape last aging frame length
#define MOUTH_SHAPE_LAST_AGING_FRAME_LENGTH 10.0f

// message argument number malloc step
#define MESSAGE_ARGUMENT_MALLOC_STEP 60

// maximum length of a message argument
#define MESSAGE_ARGUMENT_MAXLEN MMDAGENT_MAXBUFLEN

// motion alias
#define DIALOGUE_ACTION_MOTION_ALIASNAME "__action"
#define DIALOGUE_ACTION_MOTION_MAXNUM 10

// idle time: if no message comes for this period, issue event
#define IDLE_TIME_FRAME 450.0f

// shapemap entry name for lip sync
static const char *lipNames[] = {
   "LIP_A",
   "LIP_I",
   "LIP_U",
   "LIP_O"
};

// shapemap entry name for tracking bones
static const char *boneNames[] = {
   "TRACK_HEAD",
   "TRACK_LEFTEYE",
   "TRACK_RIGHTEYE",
   "TRACK_BOTHEYE",
   "TRACK_NECK",
   "TRACK_BODY"
};

// Avatar::initialize: initialize instance
void Avatar::initialize()
{
   m_name = NULL;
   m_obj = NULL;
   m_pmd = NULL;
   m_enable = false;

   m_messageBuf = NULL;
   m_messageBufMaxNum = 0;

   m_mirror = false;
   m_both_eye_mode = false;
   m_allow_far_close_move = true;
   m_rotation_rate_body = BODY_ROTATION_COEF;
   m_rotation_rate_neck = NECK_ROTATION_COEF;
   m_rotation_rate_head = HEAD_ROTATION_COEF;
   m_move_scale_up = CENTERBONE_ADDITIONALMOVECOEF_SCALE;
   m_move_scale_relative_down = CENTERBONE_ADDITIONALMOVECOEF_SCALE_RELATIVE_X;
   m_leavingFrameLeft = 0.0f;
   m_issueLeaveEvent = false;
   m_mouseAgingFrameLeft = 0.0f;
   m_disableAutoLipFlag = DAL_NO;
   m_autoLipFlag = true;
   m_idleFrames = 0.0f;
   m_idle = false;
   m_noControlFromAudioLipSync = false;

#ifdef LIPSYNC_JULIUS
   m_julius_thread = NULL;
#endif /* LIPSYNC_JULIUS */

   m_shapemap = NULL;
   m_mq = NULL;

   m_faceTrackingFrameLeft = 0.0f;

   m_arkit.setup();
   m_exMorph.setup();
   m_exBone.setup();

   for (int i = 0; i < TRACK_NUM; i++)
      m_boneControl[i].clear();
   for (int i = 0; i < LIP_NUM; i++)
      m_lipControl[i].clear();
   for (int i = 0; i < NUMACTIONUNITS; i++)
      m_auControl[i].clear();

}

// Avatar::clear: clear instance
void Avatar::clear()
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

// Avatar::resetBoneTarget: reset bone target
void Avatar::resetBoneTarget()
{
   for (int i = 0; i < TRACK_NUM; i++)
      m_boneControl[i].resetTarget();
   m_centerAddControl.resetTarget();
   m_exBone.resetTargets();
}

// Avatar::resetFaceTarget: reset face target
void Avatar::resetFaceTarget()
{
   for (int i = 0; i < LIP_NUM; i++)
      m_lipControl[i].resetTarget();
   for (int i = 0; i < NUMACTIONUNITS; i++)
      m_auControl[i].resetTarget();
   m_arkit.resetTargets();
   m_exMorph.resetTargets();
}

// Avatar::getCurrent: get current
void Avatar::getCurrent()
{
   for (int i = 0; i < TRACK_NUM; i++)
      m_boneControl[i].fetchCurrentRotation();
}

/* Avatar::Avatar: constructor */
Avatar::Avatar()
{
   initialize();
}

/* Avatar::~Avatar: destructor */
Avatar::~Avatar()
{
#ifdef LIPSYNC_JULIUS
   /* stop Julius */
   if (m_julius_thread)
      m_julius_thread->stopAndRelease();
#endif /* LIPSYNC_JULIUS */
   clear();
}

// Avatar::assignModel: assign model
bool Avatar::assignModel(const char *alias)
{
   PMDObject *obj;
   PMDModel *pmd;
   char buf[MMDAGENT_MAXBUFLEN];
   int id;
   PMDObject *modellist;

   if (m_mmdagent == NULL)
      return false;

   modellist = m_mmdagent->getModelList();

   resetBoneTarget();
   resetFaceTarget();

   m_arkit.setup();
   m_exMorph.setup();
   m_exBone.setup();

   obj = NULL;
   pmd = NULL;

   // find model of the given alias
   id = m_mmdagent->findModelAlias(alias);
   if (id >= 0) {
      obj = &(modellist[id]);
      pmd = modellist[id].getPMDModel();
   }

   // if not specified or not found, return with error
   if (pmd == NULL) {
      m_obj = NULL;
      m_pmd = NULL;
      if (m_name)
         free(m_name);
      m_name = NULL;
      return FALSE;
   }

   if (pmd->getNumBone() <= 0)
      return false;

   m_name = MMDAgent_strdup(alias);

   m_shapemap = obj->getShapeMap();

   for (int i = 0; i < TRACK_NUM; i++)
      m_boneControl[i].clear();
   m_centerAddControl.clear();

   if (m_shapemap) {
      // pick bones to be controlled
      PMDBone *b;
      for (int i = 0; i < TRACK_NUM; i++) {
         b = m_shapemap->getTrackBone(boneNames[i]);
         if (b)
            m_boneControl[i].setup(b, true);
      }
      b = m_shapemap->getTrackBone("TRACK_CENTER");
      if (b)
         m_centerAddControl.setup(b, true);
   }

   for (int i = 0; i < LIP_NUM; i++)
      m_lipControl[i].clear();
   for (int i = 0; i < NUMACTIONUNITS; i++)
      m_auControl[i].clear();
   if (m_shapemap) {
      // set up lip controller
      for (int i = 0; i < LIP_NUM; i++) {
         PMDFaceInterface *f = m_shapemap->getLipMorph(lipNames[i]);
         if (f)
            m_lipControl[i].set(f, 0.0f);
      }
      // set up AU controller
      for (int i = 0; i < NUMACTIONUNITS; i++) {
         MMDAgent_snprintf(buf, MMDAGENT_MAXBUFLEN, "AU%d", i + 1);
         PMDFaceInterface *f = m_shapemap->getAUMorph(buf);
         if (f)
            m_auControl[i].set(f, 0.0f);
      }
   }

   m_obj = obj;
   m_pmd = pmd;

   getCurrent();

   return true;
}

// Avatar::setup: setup avatar control for a model
bool Avatar::setup(MMDAgent *mmdagent, int id, bool mirror_mode, bool wantLocal, bool wantPassthrough)
{
   m_mmdagent = mmdagent;
   m_id = id;

   clear();

#ifdef LIPSYNC_JULIUS
   /* start Julius */
   startJulius(JCONF_NAME, wantLocal, wantPassthrough);
#endif /* LIPSYNC_JULIUS */

   /* send udp Port num to remote end */

   m_mirror = mirror_mode;

   if (m_mq)
      delete m_mq;
   m_mq = new MouthQueue();

   return true;
}

// Avatar::processMessage: process message from openface
bool Avatar::processMessage(const char *AVString)
{
   char buff[MMDAGENT_MAXBUFLEN];
   char buff2[MMDAGENT_MAXBUFLEN];
   char buff3[MMDAGENT_MAXBUFLEN];
   char *p, *save = NULL;
   int n = 0;
   float rx, ry, rz;
   float rx2, ry2, rz2;
   btVector3 pos;
   btQuaternion rot, rotSave, rot2;
   const btQuaternion norot(btScalar(0.0f), btScalar(0.0f), btScalar(0.0f), btScalar(1.0f));

   if (AVString == NULL)
      return false;

   // detect model change and re-assign if model change has occured
   if (m_obj) {
      m_obj->lock();
      if (m_pmd != m_obj->getPMDModel())
         assignModel(m_name);
   }

   // store arguments
   if (m_messageBuf == NULL) {
      m_messageBufMaxNum = MESSAGE_ARGUMENT_MALLOC_STEP;
      m_messageBuf = (char **)malloc(sizeof(char *) * m_messageBufMaxNum);
      for (int i = 0; i < m_messageBufMaxNum; i++)
         m_messageBuf[i] = NULL;
   }
   MMDAgent_snprintf(buff, MMDAGENT_MAXBUFLEN, "%s", AVString);
   for (p = MMDAgent_strtok(buff, ",\r\n", &save); p; p = MMDAgent_strtok(NULL, ",\r\n", &save)) {
      if (n >= m_messageBufMaxNum) {
         // expand
         char **tmp = (char **)malloc(sizeof(char *) * (m_messageBufMaxNum + MESSAGE_ARGUMENT_MALLOC_STEP));
         for (int i = 0; i < m_messageBufMaxNum; i++)
            tmp[i] = m_messageBuf[i];
         for (int i = m_messageBufMaxNum; i < m_messageBufMaxNum + MESSAGE_ARGUMENT_MALLOC_STEP; i++)
            tmp[i] = NULL;
         m_messageBufMaxNum += MESSAGE_ARGUMENT_MALLOC_STEP;
         free(m_messageBuf);
         m_messageBuf = tmp;
      }
      if (m_messageBuf[n] == NULL)
         m_messageBuf[n] = (char *)malloc(sizeof(char) * MESSAGE_ARGUMENT_MAXLEN);
      strncpy(m_messageBuf[n], p, MESSAGE_ARGUMENT_MAXLEN - 1);
      m_messageBuf[n][MESSAGE_ARGUMENT_MAXLEN - 1] = '\0';
      n++;
   }

   if (MMDAgent_strequal(m_messageBuf[0], "__AV_START")) {
      setEnableFlagInternal(true);
      if (m_obj) m_obj->unlock();
      return true;
   } else if (MMDAgent_strequal(m_messageBuf[0], "__AV_END")) {
      setEnableFlagInternal(false);
      if (m_obj) m_obj->unlock();
      return true;
   } else if (MMDAgent_strequal(m_messageBuf[0], "__AV_RECALIBRATE")) {
      // trigger adjustment
      for (int i = 0; i < TRACK_NUM; i++)
         m_boneControl[i].resetCalibration();
      m_centerAddControl.resetCalibration();
      if (m_obj) m_obj->unlock();
      return true;
   } else if (MMDAgent_strequal(m_messageBuf[0], "__AV_SETMODEL")) {
      if (n != 2) {
         if (m_obj) m_obj->unlock();
         return false;
      }
      bool ret = assignModel(m_messageBuf[1]);
      if (m_obj) m_obj->unlock();
      return ret;
   } else if (MMDAgent_strequal(m_messageBuf[0], "__AVCONF_ALLOWFARCLOSEMOVE")) {
      if (n != 2) {
         if (m_obj) m_obj->unlock();
         return false;
      }
      if (MMDAgent_strequal(m_messageBuf[1], "true"))
         m_allow_far_close_move = true;
      else
         m_allow_far_close_move = false;
      return true;
   } else if (MMDAgent_strequal(m_messageBuf[0], "__AV_TRACK") && m_enable == true) {
      if (n != 14) {
         if (m_obj) m_obj->unlock();
         return false;
      }
      // head
      pos.setX(MMDAgent_str2float(m_messageBuf[1]) * MM2MMD_COEF);
      pos.setY(MMDAgent_str2float(m_messageBuf[2]) * MM2MMD_COEF);
      pos.setZ(-MMDAgent_str2float(m_messageBuf[3]) * MM2MMD_COEF);
      m_boneControl[TRACK_HEAD].setTargetPosition(pos);
      rx = MMDAgent_str2float(m_messageBuf[4]);
      ry = MMDAgent_str2float(m_messageBuf[5]);
      rz = MMDAgent_str2float(m_messageBuf[6]);
      /* OpenFace = left-handed, rotation around x-axis is inverted  Bulletphysics = right handed */
      rot.setEuler(btScalar(-ry), btScalar(rx), btScalar(-rz));
      m_boneControl[TRACK_HEAD].setTargetRotation(rot);
      /* save ogirinal rotation for later eye gaze computation */
      rotSave.setEulerZYX(btScalar(rz), btScalar(ry), btScalar(rx));
      // eyes
      rx = MMDAgent_str2float(m_messageBuf[7]);
      ry = MMDAgent_str2float(m_messageBuf[8]);
      rz = MMDAgent_str2float(m_messageBuf[9]);
      rx2 = MMDAgent_str2float(m_messageBuf[10]);
      ry2 = MMDAgent_str2float(m_messageBuf[11]);
      rz2 = MMDAgent_str2float(m_messageBuf[12]);
      if (MMDAgent_str2int(m_messageBuf[13]) == 1) {
         // perform OpenFace-Webcam specific conversion
         // 1. eye rotation is global, convert to local
         // 2. use only both-eye bone: apply mean
         btVector3 axis;
         btVector3 front = btTransform(rotSave) * btVector3(0, 0, -1);
         btVector3 target = btVector3(rx, ry, rz);
         if (target.fuzzyZero() == false) {
            target.normalize();
            axis = target.cross(front);
            axis.normalize();
            rot.setRotation(axis, front.angle(target));
         } else {
            rot.setValue(btScalar(0.0f), btScalar(0.0f), btScalar(0.0f), btScalar(1.0f));
         }
         target = btVector3(rx2, ry2, rz2);
         if (target.fuzzyZero() == false) {
            target.normalize();
            axis = target.cross(front);
            axis.normalize();
            rot2.setRotation(axis, front.angle(target));
         } else {
            rot2.setValue(btScalar(0.0f), btScalar(0.0f), btScalar(0.0f), btScalar(1.0f));
         }
         // convert to right-handed
         rot.setX(-rot.getX());
         rot.setZ(-rot.getZ());
         rot2.setX(-rot2.getX());
         rot2.setZ(-rot2.getZ());
         // make both-eye rotation as average of left eye and right eye
         m_boneControl[TRACK_BOTHEYE].setTargetRotation(rot.slerp(rot2, btScalar(0.5f)));
         // reset individual rotation for both eyes
         m_boneControl[TRACK_LEFTEYE].setTargetRotation(norot);
         m_boneControl[TRACK_RIGHTEYE].setTargetRotation(norot);
         m_both_eye_mode = true;
      } else {
         if (m_obj && m_shapemap) {
            float p = m_shapemap->getEyeRotationCoef();
            ry *= p; rx *= p; rz *= p;
            ry2 *= p; rx2 *= p; rz2 *= p;
         }
         rot.setEuler(btScalar(-ry), btScalar(rx), btScalar(-rz));
         rot2.setEuler(btScalar(-ry2), btScalar(rx2), btScalar(-rz2));
         m_boneControl[TRACK_LEFTEYE].setTargetRotation(rot);
         m_boneControl[TRACK_RIGHTEYE].setTargetRotation(rot2);
         m_both_eye_mode = false;
      }

      // set neck and body rotation from head rotation
      m_boneControl[TRACK_HEAD].getTargetRotation(&rot);
      m_boneControl[TRACK_NECK].setTargetRotation(norot.slerp(rot, btScalar(m_rotation_rate_neck)));
      m_boneControl[TRACK_BODY].setTargetRotation(norot.slerp(rot, btScalar(m_rotation_rate_body)));
      m_boneControl[TRACK_HEAD].setTargetRotation(norot.slerp(rot, btScalar(m_rotation_rate_head)));

      // additional XYZ-axis move on center bone form head rotation for dynamic leaning
      {
         m_boneControl[TRACK_HEAD].getTargetPosition(&pos);
         m_boneControl[TRACK_HEAD].getTargetRotation(&rot);
         btVector3 v = btTransform(rot) * btVector3(0.0f, 0.0f, m_move_scale_up);
         m_centerAddControl.setTargetPosition(btVector3(v.getX()*  m_move_scale_relative_down, v.getY(), m_allow_far_close_move ? pos.getZ() : 0.0f));
      }

      // adjust and add rotation from relative head position
      {
         m_boneControl[TRACK_HEAD].getTargetPosition(&pos);
         float r = pos.getZ() * POS2TAN_COEF;
         float r2 = 0.0f;
         if (m_allow_far_close_move) {
            /* allow more back leaning */
            if (r < -BACK_LEANING_MAXIMUM_RAD * 2.0f)
               r = -BACK_LEANING_MAXIMUM_RAD * 2.0f;
            r2 = pos.getX() * 0.2f;
         } else {
            if (r < -BACK_LEANING_MAXIMUM_RAD)
               r = -BACK_LEANING_MAXIMUM_RAD;
         }
         m_boneControl[TRACK_BODY].getTargetRotation(&rot);
         m_boneControl[TRACK_BODY].setTargetRotation(btQuaternion(0.0f, r, -r2) * rot);
         m_boneControl[TRACK_NECK].getTargetRotation(&rot);
         m_boneControl[TRACK_NECK].setTargetRotation(btQuaternion(0.0f, -r, 0.0f) * rot);
      }
      if (m_mirror) {
         for (int i = 0; i < TRACK_NUM; i++)
            m_boneControl[i].doMirror();
         m_centerAddControl.doMirror();
      }
      // calibration
      for (int i = 0; i < TRACK_NUM; i++)
         m_boneControl[i].doCalibrateTarget();
      m_centerAddControl.doCalibrateTarget();
      // reset face tracking last frame count
      m_faceTrackingFrameLeft = FACETRACKINGLEAVINGFRAMES + LEAVINGFRAMES;
   } else if (MMDAgent_strequal(m_messageBuf[0], "__AV_ARKIT") && m_enable == true) {
      char *p2, *p3, *save2;
      for (int i = 1; i < n; i++) {
         strcpy(buff2, m_messageBuf[i]);
         p2 = MMDAgent_strtok(buff2, "=", &save2);
         p3 = MMDAgent_strtok(NULL, "=", &save2);
         if (p2 != NULL && p3 != NULL) {
            if (m_obj && m_shapemap) {
               PMDFaceInterface *f = m_shapemap->getARKitMorph(p2);
               m_arkit.set(p2, f, MMDAgent_str2float(p3));
            }
         }
      }
      // set autolip flag
      if (m_disableAutoLipFlag == DAL_ARKIT || m_disableAutoLipFlag == DAL_ARKITANDAU) {
         m_autoLipFlag = false;
      }
      // reset face tracking last frame count
      m_faceTrackingFrameLeft = FACETRACKINGLEAVINGFRAMES + LEAVINGFRAMES;
   } else if (MMDAgent_strequal(m_messageBuf[0], "__AV_AU") && m_enable == true) {
      char *p2, *p3, *save2;
      for (int i = 1; i < n; i++) {
         strcpy(buff2, m_messageBuf[i]);
         p2 = MMDAgent_strtok(buff2, "=", &save2);
         p3 = MMDAgent_strtok(NULL, "=", &save2);
         if (p2 != NULL && p3 != NULL) {
            int auidx = MMDAgent_str2int(p2) - 1;
            if (0 <= auidx && auidx < NUMACTIONUNITS) {
               float v = MMDAgent_str2float(p3);
               if (v < 0.0f) v = 0.0f;
               if (v > 1.0f) v = 1.0f;
               m_auControl[auidx].setTarget(v);
            }
         }
      }
      // set autolip flag
      if (m_disableAutoLipFlag == DAL_AU || m_disableAutoLipFlag == DAL_ARKITANDAU) {
         m_autoLipFlag = false;
      }
      // reset face tracking last frame count
      m_faceTrackingFrameLeft = FACETRACKINGLEAVINGFRAMES + LEAVINGFRAMES;
   } else if (MMDAgent_strequal(m_messageBuf[0], "__AV_EXBONE") && m_enable == true) {
      PMDBone *b;
      if (m_obj && m_shapemap) {
         for (int i = 1; i + 7 < n; i += 8) {
            strcpy(buff2, "EXBONE_");
            strcat(buff2, m_messageBuf[i]);
            b = m_shapemap->getExBone(buff2);
            if (b == NULL) continue;
            pos.setX(MMDAgent_str2float(m_messageBuf[i + 1]) * MM2MMD_COEF);
            pos.setY(MMDAgent_str2float(m_messageBuf[i + 2]) * MM2MMD_COEF);
            pos.setZ(-MMDAgent_str2float(m_messageBuf[i + 3]) * MM2MMD_COEF);
            rot = btQuaternion(btScalar(-MMDAgent_str2float(m_messageBuf[i + 4])), btScalar(MMDAgent_str2float(m_messageBuf[i + 5])), btScalar(-MMDAgent_str2float(m_messageBuf[i + 6])), btScalar(MMDAgent_str2float(m_messageBuf[i + 7])));
            m_exBone.set(m_messageBuf[i], b, pos, rot, false);
         }
         if (m_mirror)
            m_exBone.doMirrors();
      }
      // reset face tracking last frame count
      m_faceTrackingFrameLeft = FACETRACKINGLEAVINGFRAMES + LEAVINGFRAMES;
   } else if (MMDAgent_strequal(m_messageBuf[0], "__AV_EXMORPH") && m_enable == true) {
      char *p2, *p3, *save2;
      for (int i = 1; i < n; i++) {
         strcpy(buff2, m_messageBuf[i]);
         p2 = MMDAgent_strtok(buff2, "=", &save2);
         p3 = MMDAgent_strtok(NULL, "=", &save2);
         if (p2 != NULL && p3 != NULL) {
            if (m_obj && m_shapemap) {
               strcpy(buff3, "EXMORPH_");
               strcat(buff3, p2);
               PMDFaceInterface *f = m_shapemap->getExMorph(buff3);
               m_exMorph.set(p2, f, MMDAgent_str2float(p3));
            }
         }
      }
      // reset face tracking last frame count
      m_faceTrackingFrameLeft = FACETRACKINGLEAVINGFRAMES + LEAVINGFRAMES;
   } else if (MMDAgent_strequal(m_messageBuf[0], "__AV_ACTION")) {
      if (n >= 2 && m_obj && m_shapemap && m_name) {
         int act_id = MMDAgent_str2int(m_messageBuf[1]);
         MMDAgent_snprintf(buff2, MMDAGENT_MAXBUFLEN, "ACT%d", act_id);
         const char *fileName = m_shapemap->getActionMotionFileName(buff2);
         if (fileName) {
            int id = m_mmdagent->findModelAlias(m_name);
            if (id >= 0) {
               int i;
               for (i = 0; i < DIALOGUE_ACTION_MOTION_MAXNUM; i++) {
                  MMDAgent_snprintf(buff3, MMDAGENT_MAXBUFLEN, "%s%d", DIALOGUE_ACTION_MOTION_ALIASNAME, i);
                  if (m_mmdagent->getModelList()[id].getMotionManager()->getRunning(buff3) == NULL)
                     break;
               }
               if (i < DIALOGUE_ACTION_MOTION_MAXNUM) {
                  m_mmdagent->sendMessage(m_id, MMDAGENT_COMMAND_MOTIONADD, "%s|%s|%s|PART|ONCE|ON|OFF", m_name, buff3, fileName);
                  for (int k = 0; k < DIALOGUE_ACTION_MOTION_MAXNUM; k++) {
                     MMDAgent_snprintf(buff3, MMDAGENT_MAXBUFLEN, "%s%d", DIALOGUE_ACTION_MOTION_ALIASNAME, k);
                     if (m_mmdagent->getModelList()[id].getMotionManager()->getRunning(buff3))
                        m_mmdagent->sendMessage(m_id, MMDAGENT_COMMAND_MOTIONDELETE, "%s|%s", m_name, buff3);
                  }
               }
            }
         }
      }
   } else if (MMDAgent_strequal(m_messageBuf[0], "__AVCONF_DISABLEAUTOLIP")) {
      if (n >= 2) {
         if (MMDAgent_strequal(m_messageBuf[1], "NO")) {
            m_disableAutoLipFlag = DAL_NO;
            m_autoLipFlag = true;
         } else if (MMDAgent_strequal(m_messageBuf[1], "ARKIT")) {
            m_disableAutoLipFlag = DAL_ARKIT;
            m_autoLipFlag = true;
         } else if (MMDAgent_strequal(m_messageBuf[1], "AU")) {
            m_disableAutoLipFlag = DAL_AU;
            m_autoLipFlag = true;
         } else if (MMDAgent_strequal(m_messageBuf[1], "ARKIT+AU") || MMDAgent_strequal(m_messageBuf[1], "AU+ARKIT")) {
            m_disableAutoLipFlag = DAL_ARKITANDAU;
            m_autoLipFlag = true;
         } else if (MMDAgent_strequal(m_messageBuf[1], "ALWAYS")) {
            m_disableAutoLipFlag = DAL_ALWAYS;
            m_autoLipFlag = false;
         }
      }
   }

   /* reset all faces except lip (?) */

   if (m_obj) m_obj->unlock();

   return true;
}

// Avatar::update: apply controlled motions to bones and morphs
void Avatar::update(float deltaFrame)
{
   float rate;
   float blendRate;

   // detect model change and re-assign if model change has occured
   if (m_obj) {
      m_obj->lock();
      if (m_pmd != m_obj->getPMDModel())
         assignModel(m_name);
   }

   // update idle status
   m_idleFrames += deltaFrame;
   if (m_idleFrames < IDLE_TIME_FRAME && m_idle == true) {
      m_idle = false;
      m_mmdagent->sendMessage(m_id, "AVATAR_EVENT_IDLE", "STOP");
   }
   if (m_idleFrames >= IDLE_TIME_FRAME && m_idle == false) {
      m_idle = true;
      m_mmdagent->sendMessage(m_id, "AVATAR_EVENT_IDLE", "START");
   }

   if (m_faceTrackingFrameLeft > LEAVINGFRAMES) {
      m_faceTrackingFrameLeft -= deltaFrame;
      if (m_faceTrackingFrameLeft <= LEAVINGFRAMES) {
         // FACETRACKINGLEAVINGFRAMES frames has been passed since last ARKIT or AU message, leaving calmly
         m_faceTrackingFrameLeft = LEAVINGFRAMES;
         resetBoneTarget();
         resetFaceTarget();
      }
   } else if (m_faceTrackingFrameLeft > 0.0f) {
      m_faceTrackingFrameLeft -= deltaFrame;
      if (m_faceTrackingFrameLeft <= 0.0f) {
         m_faceTrackingFrameLeft = 0.0f;
         if (m_disableAutoLipFlag == DAL_ARKIT || m_disableAutoLipFlag == DAL_AU || m_disableAutoLipFlag == DAL_ARKITANDAU) {
            m_autoLipFlag = true;
         }
      }
   }

   // get face changing rate
   rate = SMEARINGRATEFACE * deltaFrame;
   if (rate > 1.0f) rate = 1.0f;
   if (rate < 0.0f) rate = 0.0f;

   // set mouth shape from auto lipsync queue 
   setCurrentMouthShape(deltaFrame);

   // leaving avatar control
   blendRate = 1.0f;
   if (m_enable == false) {
      m_leavingFrameLeft -= deltaFrame;
      if (m_leavingFrameLeft <= 0.0f) {
         m_leavingFrameLeft = 0.0f;
      }
      if (m_leavingFrameLeft == 0.0f) {
         // LEAVINGFRAMES frames has been passed since last disable message, now completely stop controlling bone/faces by avatar plugin
         if (m_issueLeaveEvent == true) {
            m_mmdagent->sendMessage(m_id, PLUGIN_EVENT_AVATARCONTROL, "DISABLED");
            m_issueLeaveEvent = false;
         }
         if (m_obj) m_obj->unlock();
         return;
      }
      blendRate = m_leavingFrameLeft / LEAVINGFRAMES;
   }

   if (m_pmd == NULL) {
      if (m_obj) m_obj->unlock();
      return;
   }

   if (m_faceTrackingFrameLeft == 0.0f && m_noControlFromAudioLipSync == false) {
      // face tracking parameter does not arrive for a moment, just do for lip sync
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

   if (m_faceTrackingFrameLeft > 0.0f) {
      // control morphs
      // 1. clear weights on the target morph
      for (int i = 0; i < LIP_NUM; i++)
         m_lipControl[i].resetWeight();
      for (int i = 0; i < NUMACTIONUNITS; i++)
         m_auControl[i].resetWeight();
      m_arkit.resetWeights();
      m_exMorph.resetWeights();
      // 2. sum current weights to be applied to the target morph
      for (int i = 0; i < LIP_NUM; i++)
         m_lipControl[i].addWeight(rate);
      for (int i = 0; i < NUMACTIONUNITS; i++)
         m_auControl[i].addWeight(rate);
      m_arkit.addWeights(rate);
      m_exMorph.addWeights(rate);
      // 3. apply the summed weights to the target morph
      for (int i = 0; i < LIP_NUM; i++)
         m_lipControl[i].apply();
      for (int i = 0; i < NUMACTIONUNITS; i++)
         m_auControl[i].apply();
      m_arkit.apply();
      m_exMorph.apply();
      // 4. tune morph
      if (m_shapemap)
         m_shapemap->doMorphTune();
   }

   // get bone changing rate
   rate = SMEARINGRATEBONE * deltaFrame;
   if (rate > 1.0f) rate = 1.0f;
   if (rate < 0.0f) rate = 0.0f;

   // control bone rotations to the target rotation at the rate
   for (int i = 0; i < TRACK_NUM; i++) {
      if (i == TRACK_BOTHEYE && m_both_eye_mode == false)
         continue;
      m_boneControl[i].applyRotation(rate, blendRate);
   }
   m_exBone.applyPositions(rate, blendRate);
   m_exBone.applyRotations(rate, blendRate);

   // control bone movement to the target at the rate
   m_centerAddControl.addPosition(rate);

   // update bone
   m_centerAddControl.update();
   for (int i = 0; i < TRACK_NUM; i++)
      m_boneControl[i].update();
   m_exBone.updates();

   // update while bone for dependent bones
   m_pmd->updateBone(false);

   // re-update object root positions that are mounted
   PMDObject* modellist = m_mmdagent->getModelList();
   for (int i = 0; i < m_mmdagent->getNumModel(); i++) {
      if (modellist[i].isEnable() && modellist[i].getAssignedModel()) {
         modellist[i].updateRootBone();
         modellist[i].updateMotion(0.0);
      }
   }

   if (m_obj) m_obj->unlock();
}

// Avatar::setEnableFlagInternal: set enable flag internally
void Avatar::setEnableFlagInternal(bool flag)
{
   m_enable = flag;
   if (m_enable) {
      // trigger adjustment
      for (int i = 0; i < TRACK_NUM; i++)
         m_boneControl[i].resetCalibration();
      m_centerAddControl.resetCalibration();
      m_leavingFrameLeft = 0.0f;
      // send "AVATAR|START" message
      m_mmdagent->sendMessage(m_id, AVATAR_MESSAGE, "START");
      // set "Avatar_mode" KeyValue to 1.0
      m_mmdagent->getKeyValue()->setString(AVATAR_SW_KEYNAME, "%f", 1.0f);
   } else {
      // leaving
      m_leavingFrameLeft = LEAVINGFRAMES;
      resetBoneTarget();
      resetFaceTarget();
      // send "AVATAR|END" message
      m_mmdagent->sendMessage(m_id, AVATAR_MESSAGE, "END");
      // set "Avatar_mode" KeyValue to 0.0
      m_mmdagent->getKeyValue()->setString(AVATAR_SW_KEYNAME, "%f", 0.0f);
   }
   m_issueLeaveEvent = false;
}

// Avatar::setEnableFlag: set enable flag
void Avatar::setEnableFlag(bool flag)
{
   if (flag == true) {
      if (m_enable == false) {
         setEnableFlagInternal(flag);
      }
      m_mmdagent->sendMessage(m_id, PLUGIN_EVENT_AVATARCONTROL, "ENABLED");
   } else {
      if (m_enable == true) {
         setEnableFlagInternal(flag);
      }
      m_issueLeaveEvent = true;
   }
}

// Avatar::getEnableFlag: get enable flag
bool Avatar::getEnableFlag()
{
   return m_enable;
}

// Avatar::storeMouthShape: store mouth shape
void Avatar::storeMouthShape(const char *phonestr)
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

// Avatar::setCurrentMouthShape: set current mouth shape
void Avatar::setCurrentMouthShape(double ellapsedFrame)
{
   float rate[4];

   m_noControlFromAudioLipSync = true;

   if (m_shapemap == NULL)
      return;

   if (m_mq == NULL)
      return;

   if (m_autoLipFlag == false) {
      /* flush queue */
      m_mq->clearqueue();
      return;
   }

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
         m_noControlFromAudioLipSync = true;
      }
      else {
         m_noControlFromAudioLipSync = false;
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
         m_noControlFromAudioLipSync = true;
      }
      else {
         m_noControlFromAudioLipSync = false;
      }
   }



}

// Avatar::setRates: set motion rates
void Avatar::setRates(float body_rotate, float neck_rotate, float head_rotate, float move_up, float move_down)
{
   m_rotation_rate_body = body_rotate;
   m_rotation_rate_neck = neck_rotate;
   m_rotation_rate_head = head_rotate;
   m_move_scale_up = move_up;
   m_move_scale_relative_down = move_down;
}

#ifdef LIPSYNC_JULIUS

// Avatar::startJulius: start Julius thread
void Avatar::startJulius(const char *conffile, bool wantLocal, bool wantPassthrough)
{
   char configFile[MMDAGENT_MAXBUFLEN];

   /* config file */
   MMDAgent_snprintf(configFile, MMDAGENT_MAXBUFLEN, "%s%c%s%c%s", m_mmdagent->getAppDirName(), MMDAGENT_DIRSEPARATOR, "Julius", MMDAGENT_DIRSEPARATOR, conffile);

   /* load models and start thread */
   if (m_julius_thread == NULL)
      m_julius_thread = new Julius_Thread();
   m_julius_thread->loadAndStart(m_mmdagent, m_id, configFile, this, wantLocal, wantPassthrough);
}

#endif /* LIPSYNC_JULIUS */


// Avatar::processSoundData: process sound data
void Avatar::processSoundData(const char *data, int len)
{
   /* send the internally given audio chunk to Julius adin thread */
   m_julius_thread->processAudio(data, len);
}

// Avatar::getStreamingSoundDataFlag: get streaming sound data flag
bool Avatar::getStreamingSoundDataFlag()
{
   return m_julius_thread->getStreamingFlag();
}

// Avatar::setStreamingSoundDataFlag: set streaming sound data flag
void Avatar::setStreamingSoundDataFlag(bool flag) {
   m_julius_thread->setStreamingFlag(flag);
}

// Avatar::segmentSoundData: segment the current sound data
void Avatar::segmentSoundData()
{
   m_julius_thread->segmentAudio();
}

// Avatar::waitAudioThreadStart: wait audio thread start
void Avatar::waitAudioThreadStart()
{
   m_julius_thread->waitRunning();
}

// Avatar::getMaxVol: get max volume of avatar's speaking since last call
int Avatar::getMaxVol()
{
   return m_julius_thread->getMaxVol();
}

// Avatar::resetIdleTime: reset idle time
void Avatar::resetIdleTime()
{
   m_idleFrames = 0.0f;
}
