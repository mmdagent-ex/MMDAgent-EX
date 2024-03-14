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

/* PMDMaterialMorphElem: element of Material morph */
struct PMDMaterialMorphElem {
   int midx;            /* material index */
   bool  addflag;       /* true when add, false when multiply */
   float diffuse[4];    /* diffuse color coef */
   float specular[3];   /* specular color coef */
   float shiness;       /* shiness (specular) coef */
   float ambient[3];    /* ambient color coef */
   float edgecol[4];    /* edge color coef */
   float edgesize;      /* edge size coef */
   float tex[4];        /* texture coef */
   float sphere[4];     /* sphere texture coef */
   float toon[4];       /* toon texture coef */
   struct PMDMaterialMorphElem *next;
};

/* PMDMaterial: material of PMD */
class PMDMaterial
{
private:

   char *m_name;          /* name */

   float m_diffuse[3];  /* diffuse color */
   float m_ambient[3];  /* ambient color */
   float m_avgcol[3];   /* average of diffuse and ambient */
   float m_specular[3]; /* specular color */
   float m_alpha;       /* alpha color */
   float m_shiness;     /* shiness intensity */

   float *m_extEdgeColor; /* extra edge color per material (override model default) */
   bool m_edgeFlag;        /* true if edge should be drawn */
   float m_edgeWidth;      /* edge size */
   bool m_faceFlag;        /* true if both sides should be drawn */
   bool m_shadowFlag;      /* true if should drop shadow */
   bool m_shadowMapDropFlag;   /* true if should drop shadow on shadow map */
   bool m_shadowMapRenderFlag; /* true if should render dropped shadow on shadow map */
#ifdef MY_LUMINOUS
   bool m_luminousFlag;    /* true if should do auto-luminous rendering */
#endif
   char *m_textureFile;    /* texture file name */

   unsigned int m_numSurface; /* number of surface indices for this material */

   unsigned char m_toonID; /* toon index */

   PMDTexture *m_texture;           /* pointer to texture */
   PMDTexture *m_additionalTexture; /* pointer to additional sphere map */

   unsigned int m_surfaceList;       /* surface index of this material */
   unsigned int m_centerVertexIndex; /* center vertex index for this material */
   float m_centerVertexRadius;       /* maximum radius from center vertex */

   PMDMaterialMorphElem m_morphParamMul;  /* material morphing parameters (multiply) */
   PMDMaterialMorphElem m_morphParamAdd;  /* material morphing parameters (add) */
   float m_morphedEdgeColor[4];

   /* initialize: initialize material */
   void initialize();

   /* clear: free material */
   void clear();

public:

   /* PMDMaterial: constructor */
   PMDMaterial();

   /* ~PMDMaterial: destructor */
   ~PMDMaterial();

   /* setup: initialize and setup material */
   bool setup(PMDFile_Material *m, PMDTextureLoader *textureLoader, const char *dir, unsigned int indices);

   /* computeCenterVertex: compute center vertex */
   void computeCenterVertex(btVector3 *vertices, INDICES *surfaces);

   /* loadTexture: load texture from file */
   bool loadTexture(char *textureFileString, PMDTextureLoader *textureLoader, const char *dir);

   /* hasSingleSphereMap: return if it has single sphere maps */
   bool hasSingleSphereMap();

   /* hasMultipleSphereMap: return if it has multiple sphere map */
   bool hasMultipleSphereMap();

   /* copyDiffuse: get diffuse colors */
   void copyDiffuse(float *c);

   /* copyAvgcol: get average colors of diffuse and ambient */
   void copyAvgcol(float *c);

   /* copyAmbient: get ambient colors */
   void copyAmbient(float *c);

   /* copySpecular: get specular colors */
   void copySpecular(float *c);

   /* getAlpha: get alpha color */
   float getAlpha();

   /* getShiness: get shiness intensity */
   float getShiness();

   /* copyTextureBase: get texture base colors */
   void copyTextureBase(float *c);

   /* getNumSurface: get number of surface */
   unsigned int getNumSurface();

   /* getToonID: get toon index */
   unsigned char getToonID();

   /*getFaceFlag: get face flag */
   bool getFaceFlag();

   /* getEdgeFlag: get edge flag */
   bool getEdgeFlag();

   /* getShadowFlag: get shadow flag */
   bool getShadowFlag();

   /* getShadowMapDropFlag: get shadow map drop flag */
   bool getShadowMapDropFlag();

   /* getShadowMapRenderFlag: get shadow map render flag */
   bool getShadowMapRenderFlag();

   /* getTexture: get texture */
   PMDTexture *getTexture();

   /* getAdditionalTexture: get additional sphere map */
   PMDTexture *getAdditionalTexture();

   /* getCenterPositionIndex: get center position index */
   unsigned int getCenterPositionIndex();

   /* getCenterVertexRadius: get maximum radius from center position index */
   float getCenterVertexRadius();

   /* getSurfaceListIndex: get surface list index */
   unsigned int getSurfaceListIndex();

   /* setExtParam: set EXT parameters */
   void setExtParam(bool edge, float edgeSize, float *col, float alpha, bool face, bool shadow, bool shadowMapDrop, bool shadowMapRender, char *texFile, char *sphereFile, unsigned short sphereMode, const char *dir, PMDTextureLoader *textureLoader);

   /* getExtEdgeColor: get EXT edge color */
   float *getExtEdgeColor();

#ifdef MY_LUMINOUS
   /* getLuminousFlag: get luminous flag */
   bool getLimunousFlag();
#endif

   /* setName: set name */
   void setName(const char *name);

   /* getName: get name */
   const char *getName();

   /* resetMorphParam: reset morph param */
   void resetMorphParam();

   /* addMorphParam: add morph param */
   void addMorphParam(PMDMaterialMorphElem *param, float weight);

   /* updateMorphedEdge: update morphed edge */
   void updateMorphedEdge(INDICES *surfaceList, float *src, float *dst);

   /* getEdgeWidth: get edge width */
   float getEdgeWidth();
};
