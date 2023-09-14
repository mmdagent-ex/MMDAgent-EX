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

/* definitions */

#define BONECONTROLLER_MINLENGTH 0.0000001f

/* BoneController: control bone */
class BoneController
{
private:

   int m_numBone;           /* number of target bone */
   PMDBone **m_boneList;    /* list of bones */
   btQuaternion *m_rotList; /* list of rotations */

   float m_rateOn;            /* speed rate when switch on */
   float m_rateOff;           /* speed rate when switch off */
   btVector3 m_baseVector;    /* normalized base vector */
   btVector3 m_upperAngLimit; /* upper angular limit */
   btVector3 m_lowerAngLimit; /* lower angular limit */
   btVector3 m_adjustPos;     /* offset to adjust target position */

   int m_numChildBone;        /* number of child bones */
   PMDBone **m_childBoneList; /* list of child bones */

   bool m_enable;             /* enable flag */
   float m_fadingRate;        /* fading rate */

   /* initialize: initialize bone controller */
   void initialize();

   /* clear: free bone controller */
   void clear();

public:

   /* BoneController: constructor */
   BoneController();

   /* ~BoneController: destructor */
   ~BoneController();

   /* setup: initialize and setup bone controller */
   void setup(PMDModel *model, const char **boneName, int numBone, float rateOn, float rateOff,
              float baseVectorX, float baseVectorY, float baseVectorZ,
              float upperAngLimitX, float upperAngLimitY, float upperAngLimitZ,
              float lowerAngLimitX, float lowerAngLimitY, float lowerAngLimitZ,
              float adjustPosX, float adjustPosY, float adjustPosZ);

   /* setEnableFlag: set enable flag */
   void setEnableFlag(bool b);

   /* update: update motions */
   bool update(btVector3 *pos, float deltaFrame);
};
