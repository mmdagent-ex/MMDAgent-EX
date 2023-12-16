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
/* ----------------------------------------------------------------- */
/*           The Toolkit for Building Voice Interaction Systems      */
/*           "MMDAgent" developed by MMDAgent Project Team           */
/*           http://www.mmdagent.jp/                                 */
/* ----------------------------------------------------------------- */
/*                                                                   */
/*  Copyright (c) 2009-2016  Nagoya Institute of Technology          */
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

#include "MMDFiles.h"

/* PMDBone::initialize: initialize bone */
void PMDBone::initialize()
{
   m_id = 0;
   m_name = NULL;
   m_parentBone = NULL;
   m_childBone = NULL;
   m_type = UNKNOWN;
   m_targetBone = NULL;
   m_originPosition.setZero();
   m_followCoef = 0.0f;
   m_processAfterPhysics = false;
   m_processLayer = 0;
   m_ik = NULL;
   m_iklink = NULL;

   m_offset.setZero();
   m_parentIsRoot = false;
   m_limitAngleX = false;
   m_motionIndependent = false;

   m_trans.setIdentity();
   m_trans.setOrigin(m_originPosition);
   m_transMoveToOrigin.setIdentity();
   m_transMoveToOrigin.setOrigin(-m_originPosition);
   m_simulated = false;
   m_IKSwitchFlag = true;
   m_hasTransBySimulation = false;
   m_transBySimulation.setIdentity();

   m_pos.setZero();
   m_rot = btQuaternion(btScalar(0.0f), btScalar(0.0f), btScalar(0.0f), btScalar(1.0f));
   m_morphPos.setZero();
   m_morphRot = btQuaternion(btScalar(0.0f), btScalar(0.0f), btScalar(0.0f), btScalar(1.0f));

   m_saveFirstCall = false;
   m_saveLastSkipped = false;
}

/* PMDBone::clear: free bone */
void PMDBone::clear()
{
   if(m_name)
      free(m_name);

   initialize();
}

/* PMDBone::PMDBone: constructor */
PMDBone::PMDBone()
{
   initialize();
}

/* PMDBone::~PMDBone: destructor */
PMDBone::~PMDBone()
{
   clear();
}

/* PMDBone::setup: initialize and setup bone */
bool PMDBone::setup(PMDFile_Bone *b, PMDBone *boneList, unsigned short maxBones, PMDBone *rootBone)
{
   bool ret = true;
   char sjisBuff[21];

   clear();

   /* name */
   strncpy(sjisBuff, b->name, 20);
   sjisBuff[20] = '\0';
   m_name = MMDFiles_strdup_from_sjis_to_utf8(sjisBuff);

   /* mark if this bone should be treated as angle-constrained bone in IK process */
   if (m_name && strstr(m_name, PMDBONE_KNEENAME))
      m_limitAngleX = true;
   else
      m_limitAngleX = false;

   /* parent bone */
   if (b->parentBoneID != -1) {
      /* has parent bone */
      if (b->parentBoneID >= maxBones) {
         ret = false;
      } else {
         m_parentBone = &(boneList[b->parentBoneID]);
         m_parentIsRoot = false;
      }
   } else {
      /* no parent bone */
      if (rootBone) {
         /* set model root bone as parent */
         m_parentBone = rootBone;
         m_parentIsRoot = true;
      } else {
         /* no parent, just use it */
         m_parentIsRoot = false;
      }
   }

   /* child bone */
   if (b->childBoneID >= 0) {
      if (b->childBoneID >= maxBones)
         ret = false;
      else
         m_childBone = &(boneList[b->childBoneID]);
   }

   /* type */
   m_type = b->type;

   /* target bone to which this bone is subject to */
   if (m_type == UNDER_IK || m_type == UNDER_ROTATE) {
      if (b->targetBoneID < 0 || b->targetBoneID >= maxBones)
         ret = false;
      else
         m_targetBone = &(boneList[b->targetBoneID]);
   }

   /* store the value of targetBoneID as co-rotate coef if kind == FOLLOW_ROTATE */
   if (m_type == FOLLOW_ROTATE) {
      if (m_childBone == NULL)
         ret = false;
      m_followCoef = (float) b->targetBoneID * 0.01f;
   }

   /* store absolute bone positions */
   /* reverse Z value on bone position */
#ifdef MMDFILES_CONVERTCOORDINATESYSTEM
   m_originPosition = btVector3(btScalar(b->pos[0]), btScalar(b->pos[1]), btScalar(-b->pos[2]));
#else
   m_originPosition = btVector3(btScalar(b->pos[0]), btScalar(b->pos[1]), btScalar(b->pos[2]));
#endif /* MMDFILES_CONVERTCOORDINATESYSTEM */

   /* reset current transform values */
   m_trans.setOrigin(m_originPosition);

   /* set absolute position->origin transform matrix for skinning */
   m_transMoveToOrigin.setOrigin(-m_originPosition);

   return ret;
}

/* PMDBone::setFollowRotate: set follow-rotate bone */
void PMDBone::setFollowRotate(float rate, PMDBone *targetBone)
{
   m_childBone = targetBone;
   m_targetBone = NULL;
   m_type = FOLLOW_ROTATE;
   m_followCoef = rate;
}

/* PMDBone::setFollowMove: set follow-move bone */
void PMDBone::setFollowMove(float rate, PMDBone *targetBone)
{
   m_childBone = targetBone;
   m_targetBone = NULL;
   m_type = FOLLOW_MOVE;
   m_followCoef = rate;
}

/* PMDBone::computeOffset: compute offset position */
void PMDBone::computeOffset()
{
   if (m_parentBone)
      m_offset = m_originPosition - m_parentBone->m_originPosition;
   else
      m_offset = m_originPosition;
}

/* PMDBone::reset: reset working pos and rot */
void PMDBone::reset()
{
   m_pos.setZero();
   m_rot = btQuaternion(btScalar(0.0f), btScalar(0.0f), btScalar(0.0f), btScalar(1.0f));
   /* default transform will be referred while loading rigid bodies in PMD */
   m_trans.setIdentity();
   m_trans.setOrigin(m_originPosition);
}

/* PMDBone::setMotionIndependency: check if this bone does not be affected by other controller bones */
void PMDBone::setMotionIndependency()
{
   int i;
   const char *names[] = {PMDBONE_ADDITIONALROOTNAME};

   if (! m_parentBone || m_parentIsRoot) {
      /* if no parent bone in the model, return true */
      m_motionIndependent = true;
      return;
   }

   /* some models has additional model root bone or offset bones, they should be treated specially */
   for (i = 0; i < PMDBONE_NADDITIONALROOTNAME; i++) {
      if (MMDFiles_strequal(m_parentBone->m_name, names[i]) == true) {
         m_motionIndependent = true;
         return;
      }
   }

   m_motionIndependent = false;
}

/* PMDBone::update: update internal transform for current position/rotation */
void PMDBone::update()
{
   btVector3 p;
   btQuaternion r;
   const btQuaternion norot(btScalar(0.0f), btScalar(0.0f), btScalar(0.0f), btScalar(1.0f));

   p = m_pos;
   r = m_rot;

   p += m_morphPos;
   r = m_morphRot * r;

   if (m_type == FOLLOW_MOVE) {
      /* for co-move bone, further apply the movementn of child bone scaled by the weight */
      if (m_followCoef != 0.0f)
         p += (m_childBone->m_pos + m_childBone->m_morphPos) * m_followCoef;
   }

   m_trans.setOrigin(p + m_offset);

   if (m_type == UNDER_ROTATE) {
      /* for under-rotate bone, overwrite rotation by the target bone */
      m_trans.setRotation(m_targetBone->m_rot);
   } else if (m_type == FOLLOW_ROTATE) {
      /* for co-rotate bone, further apply the rotation of child bone scaled by the weight */
      if (m_followCoef >= 0.0f)
         r = r * norot.slerp(m_childBone->m_rot, btScalar(m_followCoef));
      else
         r = r * norot.slerp(m_childBone->m_rot.inverse(), btScalar(-m_followCoef));
      m_trans.setRotation(r);
   } else {
      m_trans.setRotation(r);
   }
   if (m_parentBone)
      m_trans = m_parentBone->m_trans * m_trans;
}

/* PMDBone::calcSkinningTrans: get internal transform for skinning */
void PMDBone::calcSkinningTrans(btTransform *b)
{
   *b = m_trans * m_transMoveToOrigin;
}

/* PMDBone;:getName: get bone name */
char *PMDBone::getName()
{
   return m_name;
}

/* PMDBone;:setName: set bone name */
void PMDBone::setName(const char *name)
{
   if (m_name)
      free(m_name);
   m_name = MMDFiles_strdup(name);
}

/* PMDBone::getType: get bone type */
unsigned char PMDBone::getType()
{
   return m_type;
}

/* PMDBone::getTransform: get transform */
btTransform *PMDBone::getTransform()
{
   return &m_trans;
}

/* PMDBone::setTransform: set transform */
void PMDBone::setTransform(btTransform *tr)
{
   m_trans = *tr;
}

/* PMDBone::saveTrans: save current transform */
void PMDBone::saveTrans()
{
   m_savedTrans = m_trans;
}

/* PMDBone::getSavedTrans: get saved transform */
void PMDBone::getSavedTrans(btTransform *tr)
{
   *tr = m_savedTrans;
}

/* PMDBone::getOriginPosition: get position */
void PMDBone::getOriginPosition(btVector3 *v)
{
   (*v) = m_originPosition;
}

/* PMDBone::isLimitAngleX: return true if this bone can be bended for X axis only at IK process */
bool PMDBone::isLimitAngleX()
{
   return m_limitAngleX;
}

/* PMDBone::hasMotionIndependency: return true if this bone is not affected by other controller bones */
bool PMDBone::hasMotionIndependency()
{
   return m_motionIndependent;
}

/* PMDBone::setSimlatedFlag: set flag whether bone is controlled under phsics or not */
void PMDBone::setSimulatedFlag(bool flag)
{
   m_simulated = flag;
}
/* PMDBone::isSimulated: return true if this bone is controlled under physics */
bool PMDBone::isSimulated()
{
   return m_simulated;
}

/* PMDBone::getOffset: get offset */
void PMDBone::getOffset(btVector3 *v)
{
   (*v) = m_offset;
}

/* PMDBone::setOffset: set offset */
void PMDBone::setOffset(btVector3 *v)
{
   m_offset = *v;
}

/* PMDBone::getParentBone: get parent bone */
PMDBone *PMDBone::getParentBone()
{
   return m_parentBone;
}

/* PMDBone::getChildBone: get child bone */
PMDBone *PMDBone::getChildBone()
{
   return m_childBone;
}

/* PMDBone::getTargetBone: get target bone */
PMDBone *PMDBone::getTargetBone()
{
   return m_targetBone;
}

/* PMDBone::getCurrentPosition: get current position */
void PMDBone::getCurrentPosition(btVector3 *v)
{
   (*v) = m_pos;
}

/* PMDBone::setCurrentPosition: set current position */
void PMDBone::setCurrentPosition(btVector3 *v)
{
   m_pos = (*v);
}

/* PMDBone::getCurrentRotation: get current rotation */
void PMDBone::getCurrentRotation(btQuaternion *q)
{
   (*q) = m_rot;
}

/* PMDBone::setCurrentRotation: set current rotation */
void PMDBone::setCurrentRotation(btQuaternion *q)
{
   m_rot = (*q);
}

/* PMDBone::resetMorph: reset morph position and rotation */
void PMDBone::resetMorph()
{
   m_morphPos.setZero();
   m_morphRot = btQuaternion(btScalar(0.0f), btScalar(0.0f), btScalar(0.0f), btScalar(1.0f));
}

/* PMDBone::addMorph: add morph position and rotation */
void PMDBone::addMorph(btVector3 *pos, btQuaternion *rot)
{
   const btQuaternion norot(btScalar(0.0f), btScalar(0.0f), btScalar(0.0f), btScalar(1.0f));

   m_morphPos += *pos;
   m_morphRot = *rot * m_morphRot;
}

/* PMDBone::setIKSwitchFlag: set IK switching flag */
void PMDBone::setIKSwitchFlag(bool flag)
{
   m_IKSwitchFlag = flag;
}

/* PMDBone::getIKSwitchFlag: get IK switching flag */
bool PMDBone::getIKSwitchFlag()
{
   return m_IKSwitchFlag;
}

/* PMDBone::renderDebug: render bones for debug */
void PMDBone::renderDebug()
{
#ifndef MMDFILES_DONTRENDERDEBUG
   btScalar m[16];
   btVector3 a;
   btVector3 b;
   GLfloat v[6];

   /* do not draw IK target bones if the IK chain is under simulation */
   if (m_type == IK_TARGET && m_parentBone && m_parentBone->m_simulated) return;

   m_trans.getOpenGLMatrix(m);

   /* draw node */
   glEnableClientState(GL_VERTEX_ARRAY);
   glPushMatrix();
   glMultMatrixf(m);
   if (m_type != NO_DISP) { /* do not draw invisible bone nodes */
      if (m_simulated) {
         /* under physics simulation */
         glColor4f(0.8f, 0.8f, 0.0f, 1.0f);
         glScalef(0.1f, 0.1f, 0.1f);
      } else {
         switch (m_type) {
         case IK_DESTINATION:
            glColor4f(0.7f, 0.2f, 0.2f, 1.0f);
            glScalef(0.25f, 0.25f, 0.25f);
            break;
         case UNDER_IK:
            glColor4f(0.8f, 0.5f, 0.0f, 1.0f);
            glScalef(0.15f, 0.15f, 0.15f);
            break;
         case IK_TARGET:
            glColor4f(1.0f, 0.0f, 0.0f, 1.0f);
            glScalef(0.15f, 0.15f, 0.15f);
            break;
         case UNDER_ROTATE:
         case TWIST:
         case FOLLOW_ROTATE:
         case FOLLOW_MOVE:
            glColor4f(0.0f, 0.8f, 0.2f, 1.0f);
            glScalef(0.15f, 0.15f, 0.15f);
            break;
         default:
            if (m_motionIndependent) {
               glColor4f(0.0f, 1.0f, 1.0f, 1.0f);
               glScalef(0.25f, 0.25f, 0.25f);
            } else {
               glColor4f(0.0f, 0.5f, 1.0f, 1.0f);
               glScalef(0.15f, 0.15f, 0.15f);
            }
            break;
         }
      }
      MMDFiles_drawcube();
   }
   glPopMatrix();

   if (!m_parentBone) {
      glDisableClientState(GL_VERTEX_ARRAY);
      return;
   }

   if (m_type == IK_DESTINATION) {
      glDisableClientState(GL_VERTEX_ARRAY);
      return;
   }

   /* draw line from parent */
   glPushMatrix();
   if (m_type == NO_DISP) {
      glColor4f(0.5f, 0.4f, 0.5f, 1.0f);
   } else if (m_simulated) {
      glColor4f(0.7f, 0.7f, 0.0f, 1.0f);
   } else if (m_type == UNDER_IK || m_type == IK_TARGET) {
      glColor4f(0.8f, 0.5f, 0.3f, 1.0f);
   } else {
      glColor4f(0.5f, 0.6f, 1.0f, 1.0f);
   }
   a = m_parentBone->m_trans.getOrigin();
   b = m_trans.getOrigin();
   v[0] = a.x();
   v[1] = a.y();
   v[2] = a.z();
   v[3] = b.x();
   v[4] = b.y();
   v[5] = b.z();
   glVertexPointer(3, GL_FLOAT, 0, v);
   glDrawArrays(GL_LINES, 0, 2);

   glPopMatrix();
   glDisableClientState(GL_VERTEX_ARRAY);
#endif /* !MMDFILES_DONTRENDERDEBUG */
}

/* PMDBone::setId: set id */
void PMDBone::setId(short id)
{
   m_id = id;
}

/* PMDBone::getId: get id */
short PMDBone::getId()
{
   return m_id;
}

/* PMDBone::enableProcessingAfterPhysics: enable processing after physics */
void PMDBone::enableProcessingAfterPhysics()
{
   m_processAfterPhysics = true;
}

/* PMDBone::disableProcessingAfterPhysics: disable processing after physics */
void PMDBone::disableProcessingAfterPhysics()
{
   m_processAfterPhysics = false;
}

/* PMDBone::processAfterPhysics: return true when should be processed after physics */
bool PMDBone::processAfterPhysics()
{
   return m_processAfterPhysics;
}

/* PMDBone::setProcessLayer: set process layer */
void PMDBone::setProcessLayer(short id)
{
   m_processLayer = id;
}

/* PMDBone::getProcessLayer: get process layer */
short PMDBone::getProcessLayer()
{
   return m_processLayer;
}

/* PMDBone::setIK: set IK */
void PMDBone::setIK(PMDIK *ik)
{
   m_ik = ik;
}

/* PMDBone::getIK: get IK */
PMDIK *PMDBone::getIK()
{
   return m_ik;
}


/* PMDBone::clearTransBySimulationFlag: clear flag for transform by physics simulation */
void PMDBone::clearTransBySimulationFlag()
{
   m_hasTransBySimulation = false;
}

/* PMDBone::setTransBySimulation: set transform by physics simulation */
void PMDBone::setTransBySimulation(btTransform *tr)
{
   m_transBySimulation = *tr;
   m_hasTransBySimulation = true;
}


/* PMDBone::updateAfterSimulation: update after simulation */
void PMDBone::updateAfterSimulation()
{
   btVector3 p;
   btQuaternion r;
   const btQuaternion norot(btScalar(0.0f), btScalar(0.0f), btScalar(0.0f), btScalar(1.0f));

   p = m_pos;
   r = m_rot;

   p += m_morphPos;
   r = m_morphRot * r;

   if (m_type == FOLLOW_MOVE) {
      /* for co-move bone, further apply the movementn of child bone scaled by the weight */
      if (m_followCoef != 0.0f)
         p += (m_childBone->m_pos + m_childBone->m_morphPos) * m_followCoef;
   }

   m_trans.setOrigin(p + m_offset);
   if (m_type == UNDER_ROTATE) {
      /* for under-rotate bone, overwrite rotation by the target bone */
      m_trans.setRotation(m_targetBone->m_rot);
   } else if (m_type == FOLLOW_ROTATE) {
      /* for co-rotate bone, further apply the rotation of child bone scaled by the rotation weight */
      if (m_followCoef >= 0.0f)
         r = r * norot.slerp(m_childBone->m_rot, btScalar(m_followCoef));
      else
         r = r * norot.slerp(m_childBone->m_rot.inverse(), btScalar(-m_followCoef));
      m_trans.setRotation(r);
   } else {
      m_trans.setRotation(r);
   }

   if (m_hasTransBySimulation) {
      m_trans = m_transBySimulation * m_trans;
   } else {
      if (m_parentBone)
         m_trans = m_parentBone->m_trans * m_trans;
   }
}

/* PMDBone::setIKLink: set IK link */
void PMDBone::setIKLink(PMDIK *ik)
{
   m_iklink = ik;
}

/* PMDBone::getIKLink: get IK link */
PMDIK *PMDBone::getIKLink()
{
   return m_iklink;
}

/* setBoneFrame: set bone frame to the data */
static void setBoneFrame(const unsigned char *data, const char *name, unsigned int keyFrame, btVector3 pos, btQuaternion rot)
{
   char *sjisBuff = MMDFiles_strdup_from_utf8_to_sjis(name);
   VMDFile_BoneFrame *bone = (VMDFile_BoneFrame *)data;
   strncpy(bone->name, sjisBuff, 15);
   free(sjisBuff);
   bone->keyFrame = keyFrame ;
#ifdef MMDFILES_CONVERTCOORDINATESYSTEM
   bone->pos[0] = pos.getX();
   bone->pos[1] = pos.getY();
   bone->pos[2] = -pos.getZ();
   bone->rot[0] = -rot.getX();
   bone->rot[1] = -rot.getY();
   bone->rot[2] = rot.getZ();
   bone->rot[3] = rot.getW();
#else
   bone->pos[0] = pos.getX();
   bone->pos[1] = pos.getY();
   bone->pos[2] = pos.getZ();
   bone->rot[0] = rot.getX();
   bone->rot[1] = rot.getY();
   bone->rot[2] = rot.getZ();
   bone->rot[3] = rot.getW();
#endif /* MMDFILES_CONVERTCOORDINATESYSTEM */
   memset(bone->interpolation, 0, 64);
}

/* PMDBone::saveAsBoneFrame: save as bone frame */
int PMDBone::saveAsBoneFrame(unsigned char **data, unsigned int keyFrame)
{
   bool saveLast = false;
   btVector3 pos;
   btQuaternion rot;
   const btQuaternion norot(btScalar(0.0f), btScalar(0.0f), btScalar(0.0f), btScalar(1.0f));

   if (data == NULL) {
      /* reset */
      m_saveFirstCall = true;
      return 0;
   }

   /* do not save when this bone is controlled by physics simulation */
   if (m_hasTransBySimulation == true)
      return 0;
   /* do not save when this is UNDER_ROTATE bone */
   if (m_type == UNDER_ROTATE)
      return 0;

   /* adjust pos/rot to be saved */
   pos = m_pos + m_morphPos;
   rot = m_morphRot * m_rot;
   if (m_type == FOLLOW_MOVE) {
      if (m_followCoef != 0.0f)
         pos -= (m_childBone->m_pos + m_childBone->m_morphPos) * m_followCoef;
   }
   if (m_type == FOLLOW_ROTATE) {
      if (m_followCoef >= 0.0f)
         rot = rot * norot.slerp(m_childBone->m_rot, btScalar(m_followCoef)).inverse();
      else
         rot = rot * norot.slerp(m_childBone->m_rot.inverse(), btScalar(-m_followCoef)).inverse();
   }

   /* check if the value is the same with the previous call */
   if (m_saveFirstCall) {
      m_saveFirstCall = false;
      m_saveLastSkipped = false;
      saveLast = false;
   } else {
      if (pos == m_posLastSaved && rot == m_rotLastSaved) {
         m_saveLastSkipped = true;
         return 0;
      } else {
         if (m_saveLastSkipped)
            saveLast = true;
         else
            saveLast = false;
         m_saveLastSkipped = false;
      }
   }

   if (saveLast && keyFrame > 0) {
      setBoneFrame(*data, m_name, keyFrame - 1, m_posLastSaved, m_rotLastSaved);
      *data += sizeof(VMDFile_BoneFrame);
   }
   setBoneFrame(*data, m_name, keyFrame, pos, rot);
   *data += sizeof(VMDFile_BoneFrame);

   m_posLastSaved = pos;
   m_rotLastSaved = rot;

   return (saveLast && keyFrame > 0) ? 2 : 1;
}
