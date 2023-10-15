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

#define PMDFACEINTERFACE_MAXMORPHASSIGNNUM 10 /* maximum number of morphs assigned to a shape name */

/* PMDFaceInterface: PMD Face interface */
class PMDFaceInterface
{
private:
   PMDModel *m_pmd;
   PMDFace *m_face[PMDFACEINTERFACE_MAXMORPHASSIGNNUM];         /* face to be managed */
   float m_faceRate[PMDFACEINTERFACE_MAXMORPHASSIGNNUM];
   int m_faceNum;
   PMDBoneMorph *m_boneMorph[PMDFACEINTERFACE_MAXMORPHASSIGNNUM];         /* bone morph to be managed */
   float m_boneMorphRate[PMDFACEINTERFACE_MAXMORPHASSIGNNUM];
   int m_boneMorphNum;
   PMDVertexMorph *m_vertexMorph[PMDFACEINTERFACE_MAXMORPHASSIGNNUM];     /* vertex morph to be managed */
   float m_vertexMorphRate[PMDFACEINTERFACE_MAXMORPHASSIGNNUM];
   int m_vertexMorphNum;
   PMDUVMorph *m_uvMorph[PMDFACEINTERFACE_MAXMORPHASSIGNNUM];             /* uv morph to be managed */
   float m_uvMorphRate[PMDFACEINTERFACE_MAXMORPHASSIGNNUM];
   int m_uvMorphNum;
   PMDMaterialMorph *m_materialMorph[PMDFACEINTERFACE_MAXMORPHASSIGNNUM]; /* material morph to be managed */
   float m_materialMorphRate[PMDFACEINTERFACE_MAXMORPHASSIGNNUM];
   int m_materialMorphNum;
   PMDGroupMorph *m_groupMorph[PMDFACEINTERFACE_MAXMORPHASSIGNNUM];       /* group morph to be managed */
   float m_groupMorphRate[PMDFACEINTERFACE_MAXMORPHASSIGNNUM];
   int m_groupMorphNum;
   bool m_valid;
   float m_weight;

   float *m_sigmoidTable;

   /* initialize: initialize PMDFaceInterface */
   void initialize();

   /* clear: free PMDFaceInterface */
   void clear();

   /* makeSigmoidTable: make sigmoid table */
   void makeSigmoidTable();

   /* getSigmoidValue: get sigmoid value */
   float getSigmoidValue(float value, float thres);

   /* computeWeight: compute weight */
   float computeWeight(float value, float rate);

public:

   /* PMDFaceInterface: constructor */
   PMDFaceInterface();

   /* PMDFaceInterface: constructor */
   PMDFaceInterface(PMDModel *pmd, const char *name, float rate, bool thres);

   /* PMDFaceInterface: destructor */
   ~PMDFaceInterface();

   /* set: set */
   bool set(PMDModel *pmd, const char *name, float rate, bool thres);

   /* add: add */
   bool add(PMDModel *pmd, const char *name, float rate, bool thres);

   /* isValid: true when valid */
   bool isValid();

   /* resetWeight: reset weight */
   void resetWeight();

   /* addWeight: add weight */
   void addWeight(float value);

   /* apply: apply added weight */
   void apply();

   /* getAssignedWeight: get assigned weight */
   float getAssignedWeight();

   /* forceAssignedWeight: force assigned weight */
   void forceAssignedWeight(float value);

};
