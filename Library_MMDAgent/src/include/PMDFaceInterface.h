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
