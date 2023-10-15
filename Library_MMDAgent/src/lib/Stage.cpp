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

#include "MMDAgent.h"

/* Stage::initialize: initialize stage */
void Stage::initialize()
{
   int i, j;

   m_hasPMD = false;
   for (i = 0; i < 4 ; i++)
      for (j = 0; j < 4; j++)
         m_floorShadow[i][j] = 0.0f;
   m_range = 0.0f;

   m_frameTexture = NULL;
   m_width = -1.0f;
   m_height = -1.0f;
}

/* Stage::clear: free stage */
void Stage::clear()
{
   if (m_frameTexture)
      delete m_frameTexture;
   initialize();
}

/* Stage::Stage: constructor */
Stage::Stage()
{
   initialize();
}

/* Stage::~Stage: destructor */
Stage::~Stage()
{
   clear();
}

/* Stage::setSize: set size of floor and background */
void Stage::setSize(const float *size, float numx, float numy)
{
   m_floor.setSize(-size[0], 0.0f, size[1],
                   size[0], 0.0f, size[1],
                   size[0], 0.0f, -size[1],
                   -size[0], 0.0f, -size[1],
                   numx, numy);
   m_background.setSize(-size[0], 0.0f, -size[1],
                        size[0], 0.0f, -size[1],
                        size[0], size[2], -size[1],
                        -size[0], size[2], -size[1],
                        numx, numy);
   if (m_range < size[0])
      m_range = size[0];
   if (m_range < size[1])
      m_range = size[1];
}

/* Stage::loadFloor: load floor image */
bool Stage::loadFloor(const char *file)
{
   if(m_floor.load(file) == false)
      return false;

   if (m_hasPMD) {
      m_pmd.release();
      m_hasPMD = false;
   }

   return true;
}

/* Stage::loadBackground: load background image */
bool Stage::loadBackground(const char *file)
{
   if(m_background.load(file) == false)
      return false;

   if (m_hasPMD) {
      m_pmd.release();
      m_hasPMD = false;
   }

   return true;
}

/* Stage::loadStagePMD: load stage pmd */
bool Stage::loadStagePMD(const char *file, BulletPhysics *bullet, SystemTexture *systex)
{
   if(m_pmd.load(file, bullet, systex) == false)
      return false;

   m_pmd.setToonFlag(MMDAgent_strtailmatch(file, ".xpmd") ? false : true);
   btVector3 v;
   m_range = m_pmd.calculateBoundingSphereRange(&v);
   m_hasPMD = true;

   return true;
}

/* Stage::renderFloor: render the floor */
void Stage::renderFloor()
{
   const float normal[3] = {0.0f, 1.0f, 0.0f};

   if (m_hasPMD)
      renderPMD();
   else
      m_floor.render(false, normal);
}

/* Stage::renderBackground: render the background */
void Stage::renderBackground()
{
   const float normal[3] = {0.0f, 0.0f, 1.0f};

   if (!m_hasPMD)
      m_background.render(true, normal);
}

/* Stage::renderPMD: render the stage pmd */
void Stage::renderPMD()
{
   glPushMatrix();
   m_pmd.renderModel(false);
   glPopMatrix();
}

/* Stage::getPMD: get pmd */
PMDModel *Stage::getPMD()
{
   if (m_hasPMD)
      return &m_pmd;
   return NULL;
}

/* Stage::updateShadowMatrix: update shadow projection matrix */
void Stage::updateShadowMatrix(const float *lightDirection)
{
   GLfloat dot;
   GLfloat floorPlane[4];
   GLfloat vec0x, vec0y, vec0z, vec1x, vec1y, vec1z;

   /* need 2 vectors to find cross product */
   vec0x = m_floor.getSize(2, 0) - m_floor.getSize(1, 0);
   vec0y = m_floor.getSize(2, 1) - m_floor.getSize(1, 1);
   vec0z = m_floor.getSize(2, 2) - m_floor.getSize(1, 2);

   vec1x = m_floor.getSize(3, 0) - m_floor.getSize(1, 0);
   vec1y = m_floor.getSize(3, 1) - m_floor.getSize(1, 1);
   vec1z = m_floor.getSize(3, 2) - m_floor.getSize(1, 2);

   /* find cross product to get A, B, and C of plane equation */
   floorPlane[0] =   vec0y * vec1z - vec0z * vec1y;
   floorPlane[1] = -(vec0x * vec1z - vec0z * vec1x);
   floorPlane[2] =   vec0x * vec1y - vec0y * vec1x;
   floorPlane[3] = -(floorPlane[0] * m_floor.getSize(1, 0) + floorPlane[1] * m_floor.getSize(1, 1) + floorPlane[2] * m_floor.getSize(1, 2));

   /* find dot product between light position vector and ground plane normal */
   dot = floorPlane[0] * lightDirection[0] +
         floorPlane[1] * lightDirection[1] +
         floorPlane[2] * lightDirection[2] +
         floorPlane[3] * lightDirection[3];

   m_floorShadow[0][0] = dot - lightDirection[0] * floorPlane[0];
   m_floorShadow[1][0] = 0.f - lightDirection[0] * floorPlane[1];
   m_floorShadow[2][0] = 0.f - lightDirection[0] * floorPlane[2];
   m_floorShadow[3][0] = 0.f - lightDirection[0] * floorPlane[3];

   m_floorShadow[0][1] = 0.f - lightDirection[1] * floorPlane[0];
   m_floorShadow[1][1] = dot - lightDirection[1] * floorPlane[1];
   m_floorShadow[2][1] = 0.f - lightDirection[1] * floorPlane[2];
   m_floorShadow[3][1] = 0.f - lightDirection[1] * floorPlane[3];

   m_floorShadow[0][2] = 0.f - lightDirection[2] * floorPlane[0];
   m_floorShadow[1][2] = 0.f - lightDirection[2] * floorPlane[1];
   m_floorShadow[2][2] = dot - lightDirection[2] * floorPlane[2];
   m_floorShadow[3][2] = 0.f - lightDirection[2] * floorPlane[3];

   m_floorShadow[0][3] = 0.f - lightDirection[3] * floorPlane[0];
   m_floorShadow[1][3] = 0.f - lightDirection[3] * floorPlane[1];
   m_floorShadow[2][3] = 0.f - lightDirection[3] * floorPlane[2];
   m_floorShadow[3][3] = dot - lightDirection[3] * floorPlane[3];
}

/* Stage::getShadowMatrix: get shadow projection matrix */
GLfloat *Stage::getShadowMatrix()
{
   return (GLfloat *) m_floorShadow;
}

/* Stage::getRange: return maximum stage range from origin */
float Stage::getRange()
{
   return m_range;
}

/* Stage::update: update */
void Stage::update(double ellapsedFrame)
{
   if (m_hasPMD) {
      m_pmd.getTextureLoader()->update(ellapsedFrame);
   } else {
      m_floor.update(ellapsedFrame);
      m_background.update(ellapsedFrame);
   }
}

/* Stage::loadFrameTexture: load frame texture */
bool Stage::loadFrameTexture(const char *file)
{
   PMDTexture *tex;
   bool ret;

   if (file == NULL) {
      if (m_frameTexture)
         delete m_frameTexture;
      m_frameTexture = NULL;
      m_width = -1.0f;
      m_height = -1.0f;
      return true;
   }
   tex = new PMDTexture;
   glActiveTexture(GL_TEXTURE0);
   glClientActiveTexture(GL_TEXTURE0);
   glEnable(GL_TEXTURE_2D);
   ret = tex->load(file);
   glDisable(GL_TEXTURE_2D);
   if (ret == false) {
      delete tex;
      return false;
   }
   if (m_frameTexture)
      delete m_frameTexture;
   m_frameTexture = tex;

   m_frameIndices[0] = 0;
   m_frameIndices[1] = 1;
   m_frameIndices[2] = 2;
   m_frameIndices[3] = 0;
   m_frameIndices[4] = 2;
   m_frameIndices[5] = 3;
   m_frameTexcoords[0] = 0.0f;
   m_frameTexcoords[1] = 0.0f;
   m_frameTexcoords[2] = 0.0f;
   m_frameTexcoords[3] = 1.0f;
   m_frameTexcoords[4] = 1.0f;
   m_frameTexcoords[5] = 1.0f;
   m_frameTexcoords[6] = 1.0f;
   m_frameTexcoords[7] = 0.0f;
   m_width = -1.0f;
   m_height = -1.0f;

   return true;
}

/* Stage::hasFrameTexture: return TRUE if has frame texture */
bool Stage::hasFrameTexture()
{
   if (m_frameTexture)
      return true;
   return false;
}

/* Stage::renderFrameTexture2D: render frame texture */
void Stage::renderFrameTexture2D(float screenWidth, float screenHeight)
{
   if (m_frameTexture == NULL)
      return;
   if (m_width != screenWidth || m_height != screenHeight) {
      m_width = screenWidth;
      m_height = screenHeight;
      float w = m_width;
      float h = m_height;
      m_frameVertices[0] = 0;
      m_frameVertices[1] = h;
      m_frameVertices[2] = 0;
      m_frameVertices[3] = 0;
      m_frameVertices[4] = 0;
      m_frameVertices[5] = 0;
      m_frameVertices[6] = w;
      m_frameVertices[7] = 0;
      m_frameVertices[8] = 0;
      m_frameVertices[9] = w;
      m_frameVertices[10] = h;
      m_frameVertices[11] = 0;
   }
   glEnable(GL_TEXTURE_2D);
   glVertexPointer(3, GL_FLOAT, 0, m_frameVertices);
   glEnableClientState(GL_TEXTURE_COORD_ARRAY);
   glTexCoordPointer(2, GL_FLOAT, 0, m_frameTexcoords);
   glBindTexture(GL_TEXTURE_2D, m_frameTexture->getID());
   glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
   glDrawElements(GL_TRIANGLES, 6, GL_INDICES, (const GLvoid *)m_frameIndices);
   glDisableClientState(GL_TEXTURE_COORD_ARRAY);
   glDisable(GL_TEXTURE_2D);
}
