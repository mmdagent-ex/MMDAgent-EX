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

/* BoneFaceControl: bone/face control */
class BoneFaceControl
{
private:

   KeyValue *m_keyValue;
   char *m_keyName;
   char *m_targetName;
   float m_min;
   float m_max;
   PMDBone *m_bone;
   btVector3 m_pos0;
   btVector3 m_pos1;
   btQuaternion m_rot0;
   btQuaternion m_rot1;
   PMDFace *m_face;
   PMDBoneMorph *m_boneMorph;
   PMDVertexMorph *m_vertexMorph;
   PMDUVMorph *m_uvMorph;
   PMDMaterialMorph *m_materialMorph;
   PMDGroupMorph *m_groupMorph;
   float m_val0;
   float m_val1;
   bool m_isBone;

   double m_durationFrame;
   double m_restFrame;

   int m_numChildBone;
   PMDBone **m_childBoneList;

   /* initialize: initialize */
   void initialize();

   /* clear: free */
   void clear();

public:

   /* BoneFaceControl: constructor */
   BoneFaceControl();

   /* ~BoneFaceControl: destructor */
   ~BoneFaceControl();

   /* setupBone: setup bone control */
   void setupBone(KeyValue *keyValue, const char *keyName, float kvmin, float kvmax, const char *boneName, btVector3 *pos0, btVector3 *pos1, btQuaternion *rot0, btQuaternion *rot1);

   /* setupMorph: setup morph control */
   void setupMorph(KeyValue *keyValue, const char *keyName, float kvmin, float kvmax, const char *morphName, float value0, float value1);

   /* setModel: reset model reference */
   bool setModel(PMDModel *pmd);

   /* setupSkeleton: set up as skeleton for matching */
   void setupSkeleton(const char *name, bool isBone);

   /* match: entry matching function */
   bool match(BoneFaceControl *d);
   
   /* update: update control */
   bool update(double ellapsedFrame);
};
