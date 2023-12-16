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

#define PMDFACE_MAXVERTEXID 65536

/* PMDFaceVertex: vertex of this model */
typedef struct _PMDFaceVertex {
   unsigned long id; /* vertex index of this model to be controlled */
   btVector3 pos;    /* position to be placed if this face rate is 1.0 */
} PMDFaceVertex;

/* PMDFace: face of PMD */
class PMDFace
{
private:

   char *m_name;              /* name of this face */
   unsigned char m_type;      /* face type (PMD_FACE_TYPE) */
   unsigned long m_numVertex; /* number of vertices controlled by this face */
   PMDFaceVertex *m_vertex;   /* vertices controlled by this face */
   float m_weight;            /* current weight of this face */

   /* capture work area */
   float m_weightLastSaved;  /* weight value saved at the last capture */
   bool m_saveFirstCall;     /* true when called at the first time */
   bool m_saveLastSkipped;   /* true when the last save was skipped */

   /* initialize: initialize face */
   void initialize();

   /* clear: free face */
   void clear();

public:

   /* PMDFace: constructor */
   PMDFace();

   /* ~PMDFace: destructor */
   ~PMDFace();

   /* setup: initialize and setup face */
   void setup(PMDFile_Face *face, const unsigned char *data);

   /* convertIndex: convert base-relative index to model vertex index */
   void convertIndex(PMDFace *base);

   /* apply: apply this face morph to model vertices */
   void apply(btVector3 *vertexList);

   /* add: add this face morph to model vertices with a certain rate */
   void add(btVector3 *vertexList, float rate);

   /* getName: get name */
   char *getName();

   /* setName: set face name */
   void setName(const char *name);

   /* getWeight: get weight */
   float getWeight();

   /* setWeight: set weight */
   void setWeight(float f);

   /* saveAsFaceFrame: save as face frame */
   int saveAsFaceFrame(unsigned char **data, unsigned int keyFrame);
};
