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
