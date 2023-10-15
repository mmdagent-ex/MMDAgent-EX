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

#define PMDBONE_KNEENAME "\xe3\x81\xb2\xe3\x81\x96" /* knee */

#define PMDBONE_ADDITIONALROOTNAME  "\xe5\x85\xa8\xe3\x81\xa6\xe3\x81\xae\xe8\xa6\xaa", "\xe4\xb8\xa1\xe8\xb6\xb3\xe3\x82\xaa\xe3\x83\x95\xe3\x82\xbb", "\xe5\x8f\xb3\xe8\xb6\xb3\xe3\x82\xaa\xe3\x83\x95\xe3\x82\xbb", "\xe5\xb7\xa6\xe8\xb6\xb3\xe3\x82\xaa\xe3\x83\x95\xe3\x82\xbb" /* parent of all, both legs offset, right leg offset, left leg offset */
#define PMDBONE_NADDITIONALROOTNAME 4

class PMDIK;

/* PMDBone: bone of PMD */
class PMDBone
{
private:

   /* defined data */
   short m_id;
   char *m_name;               /* bone name */
   PMDBone *m_parentBone;      /* parent bone (NULL = none) */
   PMDBone *m_childBone;       /* child bone (NULL = none) or co-rotate bone if type == 9 */
   unsigned char m_type;       /* bone type (PMD_BONE_TYPE) */
   PMDBone *m_targetBone;      /* bone ID by which this bone if affected: IK bone (type 4), under_rotate bone (type 5) */
   btVector3 m_originPosition; /* position from origin, defined in model (absolute) */
   float m_followCoef;         /* effect coefficient if type == corotate / comove */

   /* extra defined data */
   bool m_processAfterPhysics; /* true if process after physics simulation */
   short m_processLayer;       /* process layer */
   PMDIK *m_ik;                /* corresponding IK (this is destination bone) */
   PMDIK *m_iklink;            /* links to IK when this bone is under effect */

   /* definitions extracted at startup */
   btVector3 m_offset;       /* offset position from parent bone */
   bool m_parentIsRoot;      /* true if parent is root bone, otherwise false */
   bool m_limitAngleX;       /* true if this bone can be bended for X axis only at IK process */
   bool m_motionIndependent; /* true if this bone is not affected by other controller bones */

   /* work area */
   btTransform m_trans;             /* current transform matrix, computed from m_pos and m_rot */
   btTransform m_savedTrans;        /* saved transform matrix for physics */
   btTransform m_transMoveToOrigin; /* transform to move position to origin, for skinning */
   bool m_simulated;                /* true if this bone is controlled under physics */
   btVector3 m_pos;                 /* current position from parent bone, given by motion */
   btQuaternion m_rot;              /* current rotation, given by motion */
   btVector3 m_morphPos;            /* current additional position by bone morph */
   btQuaternion m_morphRot;         /* current additional rotation by bone morph */
   bool m_IKSwitchFlag;             /* whether to perform IK solving when this is IK destination bone */
   bool m_hasTransBySimulation;     /* true when has valid m_transBySimulation */
   btTransform m_transBySimulation; /* transform given by physics simulation */

   /* motion capture work area */
   btVector3 m_posLastSaved;     /* last saved position */
   btQuaternion m_rotLastSaved;  /* last saved rotation */
   bool m_saveFirstCall;         /* true when called at the first time */
   bool m_saveLastSkipped;       /* true when the last save was skipped */

   /* initialize: initialize bone */
   void initialize();

   /* clear: free bone */
   void clear();

public:

   /* PMDBone: constructor */
   PMDBone();

   /* ~PMDBone: destructor */
   ~PMDBone();

   /* setup: initialize and setup bone */
   bool setup(PMDFile_Bone *b, PMDBone *boneList, unsigned short maxBones, PMDBone *rootBone);

   /* setFollowRotate: set follow-rotate bone */
   void setFollowRotate(float rate, PMDBone *targetBone);

   /* setFollowMove: set follow-move bone */
   void setFollowMove(float rate, PMDBone *targetBone);

   /* computeOffset: compute offset position */
   void computeOffset();

   /* reset: reset working pos and rot */
   void reset();

   /* setMotionIndependency: check if this bone does not be affected by other controller bones */
   void setMotionIndependency();

   /* update: update internal transform for current position/rotation */
   void update();

   /* calcSkinningTrans: get internal transform for skinning */
   void calcSkinningTrans(btTransform *b);

   /* getName: get bone name */
   char *getName();

   /* getType: get bone type */
   unsigned char getType();

   /* getTransform: get transform */
   btTransform *getTransform();

   /* setTransform: set transform */
   void setTransform(btTransform *tr);

   /* saveTrans: save current transform */
   void saveTrans();

   /* getSavedTrans: get saved transform */
   void getSavedTrans(btTransform *tr);

   /* getOriginPosition: get origin position */
   void getOriginPosition(btVector3 *v);

   /* isLimitAngleX: return true if this bone can be bended for X axis only at IK process */
   bool isLimitAngleX();

   /* hasMotionIndependency: return true if this bone is not affected by other controller bones */
   bool hasMotionIndependency();

   /* setSimlatedFlag: set flag whether bone is controlled under phsics or not */
   void setSimulatedFlag(bool flag);

   /* isSimulated: return true if this bone is controlled under physics */
   bool isSimulated();

   /* getOffset: get offset */
   void getOffset(btVector3 *v);

   /* setOffset: set offset */
   void setOffset(btVector3 *v);

   /* getParentBone: get parent bone */
   PMDBone *getParentBone();

   /* getChildBone: get child bone */
   PMDBone *getChildBone();

   /* getTargetBone: get target bone */
   PMDBone *getTargetBone();

   /* getCurrentPosition: get current position */
   void getCurrentPosition(btVector3 *v);

   /* setCurrentPosition: set current position */
   void setCurrentPosition(btVector3 *v);

   /* getCurrentRotation: get current rotation */
   void getCurrentRotation(btQuaternion *q);

   /* setCurrentRotation: set current rotation */
   void setCurrentRotation(btQuaternion *q);

   /* resetMorph: reset morph position and rotation */
   void resetMorph();

   /* addMorph: add morph position and rotation */
   void addMorph(btVector3 *pos, btQuaternion *rot);

   /* setIKSwitchFlag: set IK switching flag */
   void setIKSwitchFlag(bool flag);

   /* getIKSwitchFlag: get IK switching flag */
   bool getIKSwitchFlag();

   /* renderDebug: render bones for debug */
   void renderDebug();

   /* setId: set id */
   void setId(short id);

   /* getId: get id */
   short getId();

   /* enableProcessingAfterPhysics: enable processing after physics */
   void enableProcessingAfterPhysics();

   /* disableProcessingAfterPhysics: disable processing after physics */
   void disableProcessingAfterPhysics();

   /* processAfterPhysics: return true when should be processed after physics */
   bool processAfterPhysics();

   /* setProcessLayer: set process layer */
   void setProcessLayer(short id);

   /* getProcessLayer: get process layer */
   short getProcessLayer();

   /* setIK: set IK */
   void setIK(PMDIK *ik);

   /* getIK: get IK */
   PMDIK *getIK();

   /* clearTransBySimulationFlag: clear flag for transform by physics simulation */
   void clearTransBySimulationFlag();

   /* setTransBySimulation: set transform by physics simulation */
   void setTransBySimulation(btTransform *tr);

   /* updateAfterSimulation: update after simulation */
   void updateAfterSimulation();

   /* setIKLink: set IK link */
   void setIKLink(PMDIK *ik);

   /* getIKLink: get IK link */
   PMDIK *getIKLink();

   /* saveAsBoneFrame: save as bone frame */
   int saveAsBoneFrame(unsigned char **data, unsigned int keyFrame);
};
