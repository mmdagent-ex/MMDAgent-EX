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

/* PMDMaterial::initialize: initialize material */
void PMDMaterial::initialize()
{
   int i;

   m_name = NULL;

   for (i = 0; i < 3; i++) {
      m_diffuse[i] = 0.0f;
      m_ambient[i] = 0.0f;
      m_avgcol[i] = 0.0f;
      m_specular[i] = 0.0f;
   }
   m_alpha = 0.0f;
   m_shiness = 0.0f;
   m_numSurface = 0;
   m_toonID = 0;
   m_edgeFlag = false;
   m_edgeWidth = 1.0f;
   m_extEdgeColor = NULL;
   m_faceFlag = false;
   m_shadowFlag = false;
   m_shadowMapDropFlag = false;
   m_shadowMapRenderFlag = false;
#ifdef MY_LUMINOUS
   m_luminousFlag = false;
#endif
   m_textureFile = NULL;
   m_texture = NULL;
   m_additionalTexture = NULL;
   m_surfaceList = 0;
   m_centerVertexIndex = 0;
   m_centerVertexRadius = 0.0f;

   resetMorphParam();
}

/* PMDMaterial::clear: free material */
void PMDMaterial::clear()
{
   if (m_name)
      free(m_name);
   if (m_extEdgeColor)
      free(m_extEdgeColor);
   if (m_textureFile)
      free(m_textureFile);
   /* actual texture data will be released inside textureLoader, so just reset pointer here */
   initialize();
}

/* PMDMaterial:: constructor */
PMDMaterial::PMDMaterial()
{
   initialize();
}

/* ~PMDMaterial:: destructor */
PMDMaterial::~PMDMaterial()
{
   clear();
}

/* PMDMaterial::setup: initialize and setup material */
bool PMDMaterial::setup(PMDFile_Material *m, PMDTextureLoader *textureLoader, const char *dir, unsigned int indices)
{
   int i;
   bool ret = true;
   char sjisBuff[21];

   clear();

   /* colors */
   for (i = 0; i < 3; i++) {
      m_diffuse[i] = m->diffuse[i];
      m_ambient[i] = m->ambient[i];
      /* calculate color for toon rendering */
      m_avgcol[i] = m_diffuse[i] * 0.5f + m_ambient[i];
      if (m_avgcol[i] > 1.0f)
         m_avgcol[i] = 1.0f;
      m_specular[i] = m->specular[i];
   }
   m_alpha = m->alpha;
   m_shiness = m->shiness;

   /* number of surface indices whose material should be assigned by this */
   m_numSurface = m->numSurfaceIndex;

   /* toon texture ID */
   if (m->toonID == 0xff)
      m_toonID = 0;
   else
      m_toonID = m->toonID + 1;
   /* edge drawing flag */
   m_edgeFlag = m->edgeFlag ? true : false;
   m_edgeWidth = 1.0f;
   /* face drawing flag */
   m_faceFlag = (m_alpha < 1.0f) ? true : false;
   /* shadow drawing flag */
   m_shadowFlag = m_edgeFlag;
   /* shadow map flag */
   m_shadowMapDropFlag = (m_alpha >= 0.97999 && m_alpha <= 0.98001) ? m_shadowFlag : false;
   m_shadowMapRenderFlag = m_shadowMapDropFlag;
#ifdef MY_LUMINOUS
   /* luminous flag */
   m_luminousFlag = (m_specular[0] == 0.0f && m_specular[1] == 0.0f && m_specular[2] == 0.0f && m_shiness >= 100.0f) ? true : false;
#endif

   /* store model texture filename */
   strncpy(sjisBuff, m->textureFile, 20);
   sjisBuff[20] = '\0';
   m_textureFile = MMDFiles_strdup_from_sjis_to_utf8(sjisBuff);
   if (textureLoader) {
      /* load model texture */
      ret = loadTexture(m_textureFile, textureLoader, dir);
   }

   /* store pointer to surface */
   m_surfaceList = indices;

   return ret;
}

/* PMDMaterial::computeCenterVertex: compute center vertex */
void PMDMaterial::computeCenterVertex(btVector3 *vertices, INDICES *surfaces)
{
   int i;
   unsigned int j;
   float f[3], d, tmp = 0.0f;
   INDICES *surface;

   /* calculate for center vertex */
   surface = &(surfaces[m_surfaceList]);
   for (i = 0; i < 3; i++)
      f[i] = 0.0f;
   for (j = 0; j < m_numSurface; j++) {
      f[0] += vertices[surface[j]].getX();
      f[1] += vertices[surface[j]].getY();
      f[2] += vertices[surface[j]].getZ();
   }
   for (i = 0; i < 3; i++)
      f[i] /= (float)m_numSurface;
   for (j = 0; j < m_numSurface; j++) {
      d = (f[0] - vertices[surface[j]].getX()) * (f[0] - vertices[surface[j]].getX())
         + (f[1] - vertices[surface[j]].getY()) * (f[1] - vertices[surface[j]].getY())
         + (f[2] - vertices[surface[j]].getZ()) * (f[2] - vertices[surface[j]].getZ());
      if (j == 0 || tmp > d) {
         tmp = d;
         m_centerVertexIndex = surface[j];
      }
   }
   /* get maximum radius from the center vertex */
   for (j = 0; j < m_numSurface; j++) {
      d = vertices[m_centerVertexIndex].distance2(vertices[surface[j]]);
      if (j == 0 || m_centerVertexRadius < d) {
         m_centerVertexRadius = d;
      }
   }
   m_centerVertexRadius = sqrtf(m_centerVertexRadius);
}

/* PMDMaterial::loadTexture: load texture from file */
bool PMDMaterial::loadTexture(char *textureFileString, PMDTextureLoader *textureLoader, const char *dir)
{
   int len;
   char *p;
   char buf[MMDFILES_MAXBUFLEN];
   bool ret = true;

   if (MMDFiles_strlen(textureFileString) > 0) {
      p = strchr(textureFileString, '*');
      if (p) {
         /* has extra sphere map */
         len = p - &(textureFileString[0]);
         MMDFiles_snprintf(buf, MMDFILES_MAXBUFLEN, "%s%c", dir, MMDFILES_DIRSEPARATOR);
         strncat(buf, textureFileString, len);
         m_texture = textureLoader->load(buf, textureFileString);
         if (!m_texture)
            ret = false;
         MMDFiles_snprintf(buf, MMDFILES_MAXBUFLEN, "%s%c%s", dir, MMDFILES_DIRSEPARATOR, p + 1);
         m_additionalTexture = textureLoader->load(buf, textureFileString);
         if (!m_additionalTexture)
            ret = false;
      } else {
         MMDFiles_snprintf(buf, MMDFILES_MAXBUFLEN, "%s%c%s", dir, MMDFILES_DIRSEPARATOR, textureFileString);
         m_texture = textureLoader->load(buf, textureFileString);
         if (!m_texture)
            ret = false;
      }
   }

   return ret;
}

/* PMDMaterial::hasSingleSphereMap: return if it has single sphere maps */
bool PMDMaterial::hasSingleSphereMap()
{
   if (m_texture && m_texture->isSphereMap() && m_additionalTexture == NULL)
      return true;
   else
      return false;
}

/* PMDMaterial::hasMultipleSphereMap: return if it has multiple sphere map */
bool PMDMaterial::hasMultipleSphereMap()
{
   if (m_additionalTexture)
      return true;
   else
      return false;
}

/* PMDMaterial::copyDiffuse: get diffuse colors */
void PMDMaterial::copyDiffuse(float *c)
{
   int i;

   for (i = 0; i < 3; i++)
      c[i] = m_diffuse[i] * m_morphParamMul.diffuse[i] + m_morphParamAdd.diffuse[i];
}

/* PMDMaterial::copyAvgcol: get average colors of diffuse and ambient */
void PMDMaterial::copyAvgcol(float *c)
{
   int i;

   for (i = 0; i < 3; i++) {
      c[i] = m_diffuse[i] * m_morphParamMul.diffuse[i] + m_morphParamAdd.diffuse[i];
      c[i] = c[i] * 0.5f + m_ambient[i] * m_morphParamMul.ambient[i] + m_morphParamAdd.ambient[i];
   }
}

/* PMDMaterial::copyAmbient: get ambient colors */
void PMDMaterial::copyAmbient(float *c)
{
   int i;

   for (i = 0; i < 3; i++)
      c[i] = m_ambient[i] * m_morphParamMul.ambient[i] + m_morphParamAdd.ambient[i];
}

/* PMDMaterial::copySpecular: get specular colors */
void PMDMaterial::copySpecular(float *c)
{
   int i;

   for (i = 0; i < 3; i++)
      c[i] = m_specular[i] * m_morphParamMul.specular[i] + m_morphParamAdd.specular[i];
}

/* PMDMaterial::getAlpha: get alpha */
float PMDMaterial::getAlpha()
{
   return m_alpha * m_morphParamMul.diffuse[3] + m_morphParamAdd.diffuse[3];
}

/* PMDMaterial::getShiness: get shiness */
float PMDMaterial::getShiness()
{
   return m_shiness * m_morphParamMul.shiness + m_morphParamAdd.shiness;
}

/* PMDMaterial::copyTextureBase: get texture base colors */
void PMDMaterial::copyTextureBase(float *c)
{
   int i;

   for (i = 0; i < 3; i++)
      c[i] = m_avgcol[i] * m_morphParamMul.tex[i] + m_morphParamAdd.tex[i];
   c[3] = m_alpha * m_morphParamMul.tex[3] + m_morphParamAdd.tex[3];
}

/* PMDMaterial::getNumSurface: get number of surface */
unsigned int PMDMaterial::getNumSurface()
{
   return m_numSurface;
}

/* PMDMaterial::getToonID: get toon index */
unsigned char PMDMaterial::getToonID()
{
   return m_toonID;
}

/* PMDMaterial::getEdgeFlag: get edge flag */
bool PMDMaterial::getEdgeFlag()
{
   return m_edgeFlag;
}

/* PMDMaterial::getTexture: get texture */
PMDTexture *PMDMaterial::getTexture()
{
   return m_texture;
}

/* PMDMaterial::getAdditionalTexture: get additional sphere map */
PMDTexture *PMDMaterial::getAdditionalTexture()
{
   return m_additionalTexture;
}

/* PMDMaterial::getCenterPositionIndex: get center position index */
unsigned int PMDMaterial::getCenterPositionIndex()
{
   return m_centerVertexIndex;
}

/* PMDMaterial::getCenterVertexRadius: get maximum radius from center position index */
float PMDMaterial::getCenterVertexRadius()
{
   return m_centerVertexRadius;
}

/* PMDMaterial::getSurfaceListIndex: get surface list index */
unsigned int PMDMaterial::getSurfaceListIndex()
{
   return m_surfaceList;
}

/* PMDMaterial::setExtParam: set EXT parameters */
void PMDMaterial::setExtParam(bool edge, float edgeSize, float *col, float alpha, bool face, bool shadow, bool shadowMapDrop, bool shadowMapRender, char *texFile, char *sphereFile, unsigned short sphereMode, const char *dir, PMDTextureLoader *textureLoader)
{
   unsigned long i;
   char buf[MMDFILES_MAXBUFLEN];
   bool ret;

   if (col == NULL) {
      if (m_extEdgeColor)
         free(m_extEdgeColor);
      m_extEdgeColor = NULL;
   } else {
      if (m_extEdgeColor == NULL)
         m_extEdgeColor = (float *)malloc(sizeof(float) * 4);
      for (i = 0; i < 4; i++)
         m_extEdgeColor[i] = col[i];

   }
   m_alpha = alpha;
   m_edgeFlag = (alpha == 0.0f) ? false : edge;
   m_edgeWidth = edgeSize;
   m_faceFlag = (alpha == 0.0f) ? false : face;
   m_shadowFlag = (alpha == 0.0f) ? false : shadow;
   m_shadowMapDropFlag = (alpha == 0.0f) ? false : shadowMapDrop;
   m_shadowMapRenderFlag = (alpha == 0.0f) ? false : shadowMapRender;

   /* texture: assume
   xxx  -> m_texture, isSphereMap=false, isSphereMapAdd=N/A
   xxx  -> m_additionalTexture, isSphereMap=true, isSphereMapAdd=true(spa)/false(sph)

   xxx   -> m_texture, isSphereMap=false, isSphereMapAdd=N/A
   ---   m_additionalTexture = NULL

   ---
   xxx   -> m_texture, isSphereMap=true, isSphereMapAdd=true(spa)/false(sph)
         m_additionalTexture = NULL
   */

   ret = true;
   if (texFile != NULL) {
      MMDFiles_snprintf(buf, MMDFILES_MAXBUFLEN, "%s%c%s", dir, MMDFILES_DIRSEPARATOR, texFile);
      m_texture = textureLoader->load(buf, texFile, false, false);
      if (!m_texture) {
         ret = false;
      }
   }
   if (sphereFile != NULL && (sphereMode == 1 || sphereMode == 2)) {
      MMDFiles_snprintf(buf, MMDFILES_MAXBUFLEN, "%s%c%s", dir, MMDFILES_DIRSEPARATOR, sphereFile);
      if (texFile != NULL) {
         m_additionalTexture = textureLoader->load(buf, sphereFile, true, sphereMode == 2 ? true : false);
         if (!m_additionalTexture) {
            ret = false;
         }
      } else {
         m_texture = textureLoader->load(buf, sphereFile, true, sphereMode == 2 ? true : false);
         if (!m_texture) {
            ret = false;
         }
      }
   }
   if (ret == false && m_textureFile != NULL) {
      /* if failed to load textures in ExtCsv, try default */
      loadTexture(m_textureFile, textureLoader, dir);
   }
}

/* PMDMaterial::getExtEdgeColor: get EXT edge color */
float *PMDMaterial::getExtEdgeColor()
{
   int i;

   if (m_extEdgeColor == NULL)
      return NULL;

   for (i = 0; i < 4; i++) {
      m_morphedEdgeColor[i] = m_extEdgeColor[i] * m_morphParamMul.edgecol[i] + m_morphParamAdd.edgecol[i];
   }

   return m_morphedEdgeColor;
}

/* PMDMaterial::getFaceFlag: get face flag */
bool PMDMaterial::getFaceFlag()
{
   return m_faceFlag;
}

/* PMDMaterial::getShadowFlag: get shadow flag */
bool PMDMaterial::getShadowFlag()
{
   return m_shadowFlag;
}

/* PMDMaterial::getShadowMapDropFlag: get shadow map drop flag */
bool PMDMaterial::getShadowMapDropFlag()
{
   return m_shadowMapDropFlag;
}

/* PMDMaterial::getShadowMapRenderFlag: get shadow map render flag */
bool PMDMaterial::getShadowMapRenderFlag()
{
   return m_shadowMapRenderFlag;
}


#ifdef MY_LUMINOUS
/* PMDMaterial::getLuminousFlag: get luminous flag */
bool PMDMaterial::getLimunousFlag()
{
   return m_luminousFlag;
}
#endif

/* PMDMaterial::setName: set name */
void PMDMaterial::setName(const char *name)
{
   if (m_name)
      free(m_name);
   m_name = MMDFiles_strdup(name);
}


/* PMDMaterial::getName: get name */
const char *PMDMaterial::getName()
{
   return m_name;
}

/* PMDMaterial::resetMorphParam: reset morph param */
void PMDMaterial::resetMorphParam()
{
   int i;

   for (i = 0; i < 4; i++) {
      m_morphParamMul.diffuse[i] = 1.0f;
      m_morphParamMul.edgecol[i] = 1.0f;
      m_morphParamMul.tex[i] = 1.0f;
      m_morphParamMul.sphere[i] = 1.0f;
      m_morphParamMul.toon[i] = 1.0f;
      m_morphParamAdd.diffuse[i] = 0.0f;
      m_morphParamAdd.edgecol[i] = 0.0f;
      m_morphParamAdd.tex[i] = 0.0f;
      m_morphParamAdd.sphere[i] = 0.0f;
      m_morphParamAdd.toon[i] = 0.0f;
   }
   for (i = 0; i < 3; i++) {
      m_morphParamMul.specular[i] = 1.0f;
      m_morphParamMul.ambient[i] = 1.0f;
      m_morphParamAdd.specular[i] = 0.0f;
      m_morphParamAdd.ambient[i] = 0.0f;
   }
   m_morphParamMul.shiness = 1.0f;
   m_morphParamMul.edgesize = 1.0f;
   m_morphParamAdd.shiness = 0.0f;
   m_morphParamAdd.edgesize = 0.0f;
}

/* PMDMaterial::addMorphParam: add morph param */
void PMDMaterial::addMorphParam(PMDMaterialMorphElem *param, float weight)
{
   int i;

   if (weight <= PMDMODEL_MINFACEWEIGHT)
      return;

   if (param->addflag) {
      for (i = 0; i < 4; i++) {
         m_morphParamAdd.diffuse[i] += param->diffuse[i] * weight;
         m_morphParamAdd.edgecol[i] += param->edgecol[i] * weight;
         m_morphParamAdd.tex[i] += param->tex[i] * weight;
         m_morphParamAdd.sphere[i] += param->sphere[i] * weight;
         m_morphParamAdd.toon[i] += param->toon[i] * weight;
      }
      for (i = 0; i < 3; i++) {
         m_morphParamAdd.specular[i] += param->specular[i] * weight;
         m_morphParamAdd.ambient[i] += param->ambient[i] * weight;
      }
      m_morphParamAdd.shiness += param->shiness * weight;
      m_morphParamAdd.edgesize += param->edgesize * weight;
   } else {
      for (i = 0; i < 4; i++) {
         m_morphParamMul.diffuse[i] *= (param->diffuse[i] - 1.0f) * weight + 1.0f;
         m_morphParamMul.edgecol[i] *= (param->edgecol[i] - 1.0f) * weight + 1.0f;
         m_morphParamMul.tex[i] *= (param->tex[i] - 1.0f) * weight + 1.0f;
         m_morphParamMul.sphere[i] *= (param->sphere[i] - 1.0f) * weight + 1.0f;
         m_morphParamMul.toon[i] *= (param->toon[i] - 1.0f) * weight + 1.0f;
      }
      for (i = 0; i < 3; i++) {
         m_morphParamMul.specular[i] *= (param->specular[i] - 1.0f) * weight + 1.0f;
         m_morphParamMul.ambient[i] *= (param->ambient[i] - 1.0f) * weight + 1.0f;
      }
      m_morphParamMul.shiness *= (param->shiness - 1.0f) * weight + 1.0f;
      m_morphParamMul.edgesize *= (param->edgesize - 1.0f) * weight + 1.0f;
   }
}

/* PMDMaterial::updateMorphedEdge: update morphed edge */
void PMDMaterial::updateMorphedEdge(INDICES *surfaceList, float *src, float *dst)
{
   unsigned int i;
   INDICES idx;

   if (m_morphParamMul.edgesize == 1.0f && m_morphParamAdd.edgesize == 0.0f)
      return;

   for (i = 0; i < m_numSurface; i++) {
      idx = surfaceList[m_surfaceList + i];
      dst[idx] = src[idx] * m_morphParamMul.edgesize + m_morphParamAdd.edgesize;
   }
}

/* PMDMaterial::getEdgeWidth: get edge width */
float PMDMaterial::getEdgeWidth()
{
   return m_edgeWidth;
}
