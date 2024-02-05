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

/* TileTexture::initialize: initialize texture */
void TileTexture::initialize()
{
   int i;

   m_isLoaded = false;

   for (i = 0; i < 12; i++)
      m_vertices[i] = 0.0f;
   m_numx = 1.0f;
   m_numy = 1.0f;
}

/* TileTexture::clear: free texture */
void TileTexture::clear()
{
   if (m_isLoaded)
      m_texture.release();

   initialize();
}

/* TileTexture::TileTexture: constructor */
TileTexture::TileTexture()
{
   initialize();
}

/* TileTexture: destructor */
TileTexture::~TileTexture()
{
   clear();
}

/* TileTexture::load: load a texture from file name (wide char) */
bool TileTexture::load(const char *file)
{
   bool ret;

   if (MMDAgent_strlen(file) <= 0)
      return false;

   glActiveTexture(GL_TEXTURE0);
   glClientActiveTexture(GL_TEXTURE0);
   glEnable(GL_TEXTURE_2D);
   ret = m_texture.loadImage(file);
   glDisable(GL_TEXTURE_2D);

   if (ret == false)
      return false;

   m_texture.setAnimationSpeedRate(1.0);

   m_isLoaded = true;

   return true;
}

/* TileTexture::render: render the textures */
void TileTexture::render(bool cullFace, const float *normal)
{
   GLfloat color[] = {1.0f, 1.0f, 1.0f, 1.0f};
   GLfloat spec[] = {0.0f, 0.0f, 0.0f, 0.0f};
   GLfloat coords[] = {0.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f};
   GLfloat vers[] = { m_vertices[0], m_vertices[1], m_vertices[2],
                      m_vertices[3], m_vertices[4], m_vertices[5],
                      m_vertices[9], m_vertices[10], m_vertices[11],
                      m_vertices[6], m_vertices[7], m_vertices[8]
                    };

   if (m_isLoaded == false) return;

   if (!cullFace)
      glDisable(GL_CULL_FACE);

   glPushMatrix();
   glEnable(GL_TEXTURE_2D);
   glActiveTexture(GL_TEXTURE0);
   glClientActiveTexture(GL_TEXTURE0);
   glVertexPointer(3, GL_FLOAT, 0, vers);
   glEnableClientState(GL_VERTEX_ARRAY);
   glBindTexture(GL_TEXTURE_2D, m_texture.getID());
   glTexCoordPointer(2, GL_FLOAT, 0, coords);
   glEnableClientState(GL_TEXTURE_COORD_ARRAY);
   glNormal3f(normal[0], normal[1], normal[2]);
   glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, color);
   glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, spec);
   glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
   glDisableClientState(GL_TEXTURE_COORD_ARRAY);
   glDisableClientState(GL_VERTEX_ARRAY);
   glDisable(GL_TEXTURE_2D);
   glPopMatrix();

   if (!cullFace)
      glEnable(GL_CULL_FACE);
}

/* TileTexture::getSize: get texture size */
GLfloat TileTexture::getSize(int i, int j)
{
   return m_vertices[i * 3 + j];
}

/* TileTexture::setSize: set texture size */
void TileTexture::setSize(float v00, float v01, float v02,
                          float v10, float v11, float v12,
                          float v20, float v21, float v22,
                          float v30, float v31, float v32,
                          float numx, float numy)
{
   m_vertices[0] = v00;
   m_vertices[1] = v01;
   m_vertices[2] = v02;
   m_vertices[3] = v10;
   m_vertices[4] = v11;
   m_vertices[5] = v12;
   m_vertices[6] = v20;
   m_vertices[7] = v21;
   m_vertices[8] = v22;
   m_vertices[9] = v30;
   m_vertices[10] = v31;
   m_vertices[11] = v32;

   m_numx = numx;
   m_numy = numy;
}

/* TileTexture::update: update */
void TileTexture::update(double ellapsedFrame)
{
   if (m_isLoaded == false) return;

   m_texture.setNextFrame(ellapsedFrame);
}
