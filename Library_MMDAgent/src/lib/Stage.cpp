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

/* overlay transient frame */
#define OVERLAY_TRANSIENT_FRAME 3.0f

/* Stage::initialize: initialize stage */
void Stage::initialize()
{
   int i, j;

   m_hasPMD = false;
   for (i = 0; i < 4 ; i++)
      for (j = 0; j < 4; j++)
         m_floorShadow[i][j] = 0.0f;
   m_range = 0.0f;

   for (int i = 0; i < MMDAGENT_STAGE_FRAME_TEXTURE_MAX; i++) {
      m_frameTextures[i].texture = NULL;
      m_frameTextures[i].alias = NULL;
   }
   m_frameTextureNum = 0;

   for (int i = 0; i < MMDAGENT_STAGE_OVERLAY_TEXTURE_MAX; i++) {
      m_overlayTextures[i].texture = NULL;
      m_overlayTextures[i].alias = NULL;
      m_overlayTextures[i].view_rate = 0.0f;
      m_overlayTextures[i].visible = false;
      m_overlayTextures[i].delete_when_invisible = false;
   }
   m_overlayTextureNum = 0;

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
}

/* Stage::clear: free stage */
void Stage::clear()
{
   for (int i = 0; i < m_frameTextureNum; i++) {
      if (m_frameTextures[i].texture)
         delete m_frameTextures[i].texture;
      if (m_frameTextures[i].alias)
         free(m_frameTextures[i].alias);
   }
   for (int i = 0; i < m_overlayTextureNum; i++) {
      if (m_overlayTextures[i].texture)
         delete m_overlayTextures[i].texture;
      if (m_overlayTextures[i].alias)
         free(m_overlayTextures[i].alias);
   }
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
   overlayUpdate(ellapsedFrame);
}

/* Stage::addFrameTexture: add frame texture */
bool Stage::addFrameTexture(const char *alias, const char *file)
{
   PMDTexture *tex;
   bool ret;
   int i;

   if (alias == NULL || file == NULL)
      return false;

   /* check name */
   for (i = 0; i < m_frameTextureNum; i++) {
      if (MMDAgent_strequal(m_frameTextures[i].alias, alias))
         break;
   }
   if (i < m_frameTextureNum) {
      /* found, replace */
      delete m_frameTextures[i].texture;
      free(m_frameTextures[i].alias);
   } else {
      /* not found, assign new */
      if (m_frameTextureNum >= MMDAGENT_STAGE_FRAME_TEXTURE_MAX)
         return false;
      i = m_frameTextureNum++;
   }

   /* load texture from file */
   tex = new PMDTexture();
   glActiveTexture(GL_TEXTURE0);
   glClientActiveTexture(GL_TEXTURE0);
   glEnable(GL_TEXTURE_2D);
   ret = tex->loadImage(file);
   glDisable(GL_TEXTURE_2D);
   if (ret == false) {
      delete tex;
      return false;
   }

   /* assign */
   m_frameTextures[i].texture = tex;
   m_frameTextures[i].alias = MMDAgent_strdup(alias);

   return true;
}

/* Stage::deleteFrameTexture: delete frame texture */
bool Stage::deleteFrameTexture(const char *alias)
{
   int n;

   if (alias == NULL)
      return false;

   /* check name */
   for (n = 0; n < m_frameTextureNum; n++) {
      if (MMDAgent_strequal(m_frameTextures[n].alias, alias))
         break;
   }
   if (n >= m_frameTextureNum) {
      /* not found */
      return false;
   }

   /* delete */
   delete m_frameTextures[n].texture;
   free(m_frameTextures[n].alias);
   for (int i = n; i < m_frameTextureNum - 1; i++) {
      m_frameTextures[i].texture = m_frameTextures[i + 1].texture;
      m_frameTextures[i].alias = m_frameTextures[i + 1].alias;
   }
   m_frameTextureNum--;

   return true;
}

/* Stage::deleteAllFrameTexture: delete all frame texture */
bool Stage::deleteAllFrameTexture()
{
   for (int i = 0; i < m_frameTextureNum; i++) {
      delete m_frameTextures[i].texture;
      free(m_frameTextures[i].alias);
   }
   m_frameTextureNum = 0;

   return true;
}

/* Stage::addOverlayTexture: add overlay texture */
bool Stage::addOverlayTexture(const char *alias, const char *file, float width_rate, float height_rate, const char *orientation, float padding_rate)
{
   PMDTexture *tex;
   bool ret;
   int i;

   if (alias == NULL || file == NULL || orientation == NULL)
      return false;

   /* check name */
   for (i = 0; i < m_overlayTextureNum; i++) {
      if (MMDAgent_strequal(m_overlayTextures[i].alias, alias))
         break;
   }
   if (i < m_overlayTextureNum) {
      /* found, replace */
      delete m_overlayTextures[i].texture;
      free(m_overlayTextures[i].alias);
   } else {
      /* not found, assign new */
      if (m_overlayTextureNum >= MMDAGENT_STAGE_OVERLAY_TEXTURE_MAX)
         return false;
      i = m_overlayTextureNum++;
   }

   /* load texture from file */
   tex = new PMDTexture();
   glActiveTexture(GL_TEXTURE0);
   glClientActiveTexture(GL_TEXTURE0);
   glEnable(GL_TEXTURE_2D);
   ret = tex->loadImage(file);
   glDisable(GL_TEXTURE_2D);
   if (ret == false) {
      delete tex;
      return false;
   }

   /* assign */
   if (width_rate == 0.0f)
      width_rate = 1.0f;
   if (height_rate == 0.0f)
      height_rate = 1.0f;
   m_overlayTextures[i].texture = tex;
   m_overlayTextures[i].alias = MMDAgent_strdup(alias);
   m_overlayTextures[i].width_rate = width_rate;
   m_overlayTextures[i].height_rate = height_rate;
   m_overlayTextures[i].padding_rate = padding_rate;
   m_overlayTextures[i].orientation_left = (MMDAgent_strequal(orientation, "LEFT_TOP") || MMDAgent_strequal(orientation, "TOP_LEFT") || MMDAgent_strequal(orientation, "LEFT_BOTTOM") || MMDAgent_strequal(orientation, "BOTTOM_LEFT")) ? true : false;
   m_overlayTextures[i].orientation_bottom = (MMDAgent_strequal(orientation, "RIGHT_BOTTOM") || MMDAgent_strequal(orientation, "BOTTOM_RIGHT") || MMDAgent_strequal(orientation, "LEFT_BOTTOM") || MMDAgent_strequal(orientation, "BOTTOM_LEFT")) ? true : false;
   m_overlayTextures[i].orientation_center = MMDAgent_strequal(orientation, "CENTER");

   calculateOverlayPosition(&(m_overlayTextures[i]));

   m_overlayTextures[i].visible = true;
   m_overlayTextures[i].delete_when_invisible = false;

   return true;
}

/* Stage::deleteOverlayTexture: delete overlay texture */
bool Stage::deleteOverlayTexture(const char *alias)
{
   int n;

   if (alias == NULL)
      return false;

   /* check name */
   for (n = 0; n < m_overlayTextureNum; n++) {
      if (MMDAgent_strequal(m_overlayTextures[n].alias, alias))
         break;
   }
   if (n >= m_overlayTextureNum) {
      /* not found */
      return false;
   }

   /* mark as delete */
   m_overlayTextures[n].visible = false;
   m_overlayTextures[n].delete_when_invisible = true;

   return true;
}

/* Stage::deleteAllOverlayTexture: delete all overlay texture */
bool Stage::deleteAllOverlayTexture()
{
   for (int i = 0; i < m_overlayTextureNum; i++) {
      m_overlayTextures[i].visible = false;
      m_overlayTextures[i].delete_when_invisible = true;
   }

   return true;
}

/* Stage::hideOverlayTexture: hide overlay texture */
bool Stage::hideOverlayTexture(const char *alias)
{
   int n;

   if (alias == NULL)
      return false;

   /* check name */
   for (n = 0; n < m_overlayTextureNum; n++) {
      if (MMDAgent_strequal(m_overlayTextures[n].alias, alias))
         break;
   }
   if (n >= m_overlayTextureNum) {
      /* not found */
      return false;
   }

   /* mark as not visible */
   m_overlayTextures[n].visible = false;

   return true;
}

/* Stage::showOverlayTexture: show hided overlay texture */
bool Stage::showOverlayTexture(const char *alias)
{
   int n;

   if (alias == NULL)
      return false;

   /* check name */
   for (n = 0; n < m_overlayTextureNum; n++) {
      if (MMDAgent_strequal(m_overlayTextures[n].alias, alias))
         break;
   }
   if (n >= m_overlayTextureNum) {
      /* not found */
      return false;
   }

   /* when in the process of deleting, no effect */
   if (m_overlayTextures[n].delete_when_invisible)
      return false;

   /* mark as visible */
   m_overlayTextures[n].visible = true;

   return true;
}

/* Stage::calculateOverlayPosition; calculate overlay position */
void Stage::calculateOverlayPosition(OverlayTexture *ot)
{
   if (m_width < 0 || m_height < 0)
      return;

   float aspect = (float)ot->texture->getHeight() / (float)ot->texture->getWidth();
   float w1, h1, w2, h2, w, h;
   w1 = ot->width_rate * m_width;
   h1 = w1 * aspect;
   h2 = ot->height_rate * m_height;
   w2 = h2 / aspect;
   if (w1 < w2) {
      w = ot->width_rate;
      h = h1 / m_height;
   } else {
      h = ot->height_rate;
      w = w2 / m_width;
   }
   float x, y;
   if (ot->orientation_center) {
      x = (1.0f - w) * 0.5f;
      y = (1.0f - h) * 0.5f;
   } else {
      float pad = ot->padding_rate;
      x = ot->orientation_left ? pad : 1.0f - pad - w;
      y = ot->orientation_bottom ? pad : 1.0f - pad - h;
   }
   x *= m_width;
   y *= m_height;
   w *= m_width;
   h *= m_height;
   ot->vertices[0] = x;
   ot->vertices[1] = y + h;
   ot->vertices[2] = 0;
   ot->vertices[3] = x;
   ot->vertices[4] = y;
   ot->vertices[5] = 0;
   ot->vertices[6] = x + w;
   ot->vertices[7] = y;
   ot->vertices[8] = 0;
   ot->vertices[9] = x + w;
   ot->vertices[10] = y + h;
   ot->vertices[11] = 0;
}

/* Stage::overlayUpdate: update overlay */
void Stage::overlayUpdate(double ellapsedFrame)
{
   float step = (float)(ellapsedFrame / OVERLAY_TRANSIENT_FRAME);
   int i = 0;
   while (i < m_overlayTextureNum) {
      if (m_overlayTextures[i].visible) {
         if (m_overlayTextures[i].view_rate < 1.0f) {
            m_overlayTextures[i].view_rate += step;
            if (m_overlayTextures[i].view_rate >= 1.0f)
               m_overlayTextures[i].view_rate = 1.0f;
         }
      } else {
         if (m_overlayTextures[i].view_rate > 0.0f) {
            m_overlayTextures[i].view_rate -= step;
            if (m_overlayTextures[i].view_rate <= 0.0f)
               m_overlayTextures[i].view_rate = 0.0f;
         }
         if (m_overlayTextures[i].view_rate == 0.0f && m_overlayTextures[i].delete_when_invisible) {
            // purge
            delete m_overlayTextures[i].texture;
            free(m_overlayTextures[i].alias);
            for (int k = i; k < m_overlayTextureNum - 1; k++) {
               memcpy(&(m_overlayTextures[k]), &(m_overlayTextures[k + 1]), sizeof(OverlayTexture));
            }
            m_overlayTextures[m_overlayTextureNum - 1].view_rate = 0.0f;
            m_overlayTextureNum--;
            continue;
         }
      }
      i++;
   }
}

/* Stage::hasFrameTexture: return TRUE if has frame texture */
bool Stage::hasFrameTexture()
{
   if (m_frameTextureNum > 0 || m_overlayTextureNum > 0)
      return true;
   return false;
}

/* Stage::renderFrameTexture2D: render frame texture */
void Stage::renderFrameTexture2D(float screenWidth, float screenHeight)
{
   if (m_frameTextureNum == 0 && m_overlayTextureNum == 0)
      return;

   /* detect screen size change */
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
      for (int i = 0; i < m_overlayTextureNum; i++) {
         calculateOverlayPosition(&(m_overlayTextures[i]));
      }
   }

   /* render */
   glEnable(GL_TEXTURE_2D);
   glVertexPointer(3, GL_FLOAT, 0, m_frameVertices);
   glEnableClientState(GL_TEXTURE_COORD_ARRAY);
   glTexCoordPointer(2, GL_FLOAT, 0, m_frameTexcoords);
   glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
   for (int i = 0; i < m_frameTextureNum; i++) {
      glBindTexture(GL_TEXTURE_2D, m_frameTextures[i].texture->getID());
      glDrawElements(GL_TRIANGLES, 6, GL_INDICES, (const GLvoid *)m_frameIndices);
   }
   for (int i = 0; i < m_overlayTextureNum; i++) {
      if (m_overlayTextures[i].view_rate > 0.0f) {
         glColor4f(1.0f, 1.0f, 1.0f, m_overlayTextures[i].view_rate);
         glVertexPointer(3, GL_FLOAT, 0, m_overlayTextures[i].vertices);
         glBindTexture(GL_TEXTURE_2D, m_overlayTextures[i].texture->getID());
         glDrawElements(GL_TRIANGLES, 6, GL_INDICES, (const GLvoid *)m_frameIndices);
      }
   }
   glDisableClientState(GL_TEXTURE_COORD_ARRAY);
   glBindTexture(GL_TEXTURE_2D, 0);
   glDisable(GL_TEXTURE_2D);

}
