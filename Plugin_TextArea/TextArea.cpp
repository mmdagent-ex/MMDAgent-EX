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
#include "TextArea.h"

/* TextArea::initialize: initialize */
void TextArea::initialize()
{
   int i;

   m_mmdagent = NULL;
   m_id = 0;
   m_name = NULL;
   m_width_set = 0;
   m_height_set = 0;
   m_textsize_set = 1.0f;
   m_width = 0;
   m_height = 0;
   m_textsize = 1.0f;
   m_margin = 1.0f;
   m_linespace = 0.0f;
   m_outline = false;
   for (i = 0; i < 4; i++) {
      m_bgcolor[i] = 0.0f;
      m_textcolor[i] = 0.0f;
   }
   m_offset.setZero();
   m_rot.setEuler(0.0f, 0.0f, 0.0f);
   m_basebone = NULL;

   m_font = NULL;
   m_transFrame = 0.0;
   m_active = false;

   m_text[0] = m_text[1] = NULL;
   memset(&(m_elem[0]), 0, sizeof(FTGLTextDrawElements));
   memset(&(m_elem[1]), 0, sizeof(FTGLTextDrawElements));
   memset(&(m_elemOut[0]), 0, sizeof(FTGLTextDrawElements));
   memset(&(m_elemOut[1]), 0, sizeof(FTGLTextDrawElements));
   m_imageTexture[0] = m_imageTexture[1] = NULL;
   m_bid = 0;
   m_enableTrans = false;
}

/* TextArea::clear: free */
void TextArea::clear()
{
   if (m_name)
      free(m_name);
   if (m_text[0])
      free(m_text[0]);
   if (m_text[1])
      free(m_text[1]);
   if (m_elem[0].vertices) free(m_elem[0].vertices);
   if (m_elem[0].texcoords) free(m_elem[0].texcoords);
   if (m_elem[0].indices) free(m_elem[0].indices);
   if (m_elem[1].vertices) free(m_elem[1].vertices);
   if (m_elem[1].texcoords) free(m_elem[1].texcoords);
   if (m_elem[1].indices) free(m_elem[1].indices);
   if (m_elemOut[0].vertices) free(m_elemOut[0].vertices);
   if (m_elemOut[0].texcoords) free(m_elemOut[0].texcoords);
   if (m_elemOut[0].indices) free(m_elemOut[0].indices);
   if (m_elemOut[1].vertices) free(m_elemOut[1].vertices);
   if (m_elemOut[1].texcoords) free(m_elemOut[1].texcoords);
   if (m_elemOut[1].indices) free(m_elemOut[1].indices);
   if (m_imageTexture[0])
      delete m_imageTexture[0];
   if (m_imageTexture[1])
      delete m_imageTexture[1];
   m_active = false;

   initialize();
}

/* TextArea::TextArea: constructor */
TextArea::TextArea()
{
   initialize();
}

/* TextArea::~TextArea: destructor */
TextArea::~TextArea()
{
   clear();
}

/* TextArea::setup: setup variables */
bool TextArea::setup(MMDAgent *mmdagent, int id, const char *str)
{
   char *buff, *alias, *p, *save;
   float f[4];
   int modelId;
   btTransform t;
   int i;

   m_mmdagent = mmdagent;
   m_id = id;

   m_rot.setEuler(0.0f, 0.0f, 0.0f);
   m_basebone = NULL;

   buff = MMDAgent_strdup(str);

   alias = MMDAgent_strtok(buff, "|", &save);
   if (alias == NULL) {
      m_mmdagent->sendLogString(m_id, MLOG_ERROR, "arg 2: missing?");
      free(buff);
      return false;
   }
   m_name = MMDAgent_strdup(alias);

   if ((p = MMDAgent_strtok(NULL, "|", &save)) == NULL) {
      m_mmdagent->sendLogString(m_id, MLOG_ERROR, "arg 3: missing?");
      free(buff);
      return false;
   }
   if (MMDAgent_str2fvec(p, f, 2) == false) {
      m_mmdagent->sendLogString(m_id, MLOG_ERROR, "arg 3: should be \"width,height\"");
      free(buff);
      return false;
   }
   m_width_set = f[0];
   m_width = m_width_set;
   m_height_set = f[1];
   m_height = m_height_set;
   if ((p = MMDAgent_strtok(NULL, "|", &save)) == NULL) {
      m_mmdagent->sendLogString(m_id, MLOG_ERROR, "arg 4: missing?");
      free(buff);
      return false;
   }
   if (MMDAgent_str2fvec(p, f, 4) == false) {
      if (MMDAgent_str2fvec(p, f, 3) == false) {
         m_mmdagent->sendLogString(m_id, MLOG_ERROR, "arg 4: should be \"textsize,margin,linespace,outline\"");
         free(buff);
         return false;
      }
      f[3] = 0.0f;
   }
   m_textsize_set = f[0];
   m_margin = f[1];
   m_linespace = f[2];
   m_outline = (f[3] == 0.0f) ? false : true;
   if ((p = MMDAgent_strtok(NULL, "|", &save)) == NULL) {
      m_mmdagent->sendLogString(m_id, MLOG_ERROR, "arg 5: missing?");
      free(buff);
      return false;
   }
   if (MMDAgent_str2fvec(p, m_bgcolor, 4) == false) {
      m_mmdagent->sendLogString(m_id, MLOG_ERROR, "arg 5: should be background color in \"r,g,b,a\"");
      free(buff);
      return false;
   }
   if ((p = MMDAgent_strtok(NULL, "|", &save)) == NULL) {
      m_mmdagent->sendLogString(m_id, MLOG_ERROR, "arg 6: missing?");
      free(buff);
      return false;
   }
   if (MMDAgent_str2fvec(p, m_textcolor, 4) == false) {
      m_mmdagent->sendLogString(m_id, MLOG_ERROR, "arg 6: should be text color in \"r,g,b,a\"");
      free(buff);
      return false;
   }
   if ((p = MMDAgent_strtok(NULL, "|", &save)) == NULL) {
      m_mmdagent->sendLogString(m_id, MLOG_ERROR, "arg 7: missing?");
      free(buff);
      return false;
   }
   if (MMDAgent_str2pos(p, &m_offset) == false) {
      m_mmdagent->sendLogString(m_id, MLOG_ERROR, "arg 7: should be position of center in \"x,y,z\"");
      free(buff);
      return false;
   }
   if ((p = MMDAgent_strtok(NULL, "|", &save)) != NULL) {
      if (MMDAgent_str2rot(p, &m_rot) == false) {
         m_mmdagent->sendLogString(m_id, MLOG_ERROR, "arg 8: should be rotation degrees in \"rx,ry,rz\"");
         free(buff);
         return false;
      }
      if ((p = MMDAgent_strtok(NULL, "|", &save)) != NULL) {
         modelId = mmdagent->findModelAlias(p);
         if(modelId == -1) {
            m_mmdagent->sendLogString(m_id, MLOG_WARNING, "arg 9: model alias name %s not exist", p);
         } else {
            PMDObject *objs = mmdagent->getModelList();
            if ((p = MMDAgent_strtok(NULL, "|", &save)) != NULL) {
               m_basebone = objs[modelId].getPMDModel()->getBone(p);
               if (m_basebone == NULL)
                  m_mmdagent->sendLogString(m_id, MLOG_WARNING, "arg 10: bone name %s not exist", p);
            } else {
               m_basebone = objs[modelId].getPMDModel()->getCenterBone();
            }
         }
      }
   }

   t.setIdentity();
   t.setOrigin(m_offset);
   t.setRotation(m_rot);
   t.getOpenGLMatrix(m_matrix);

   m_font = mmdagent->getTextureFont();
   if (m_font == NULL) {
      free(buff);
      return false;
   }

   for (i = 0; i < 2; i++) {
      if (m_text[i]) {
         free(m_text[i]);
         m_text[i] = NULL;
      }
      if (m_imageTexture[i]) {
         delete m_imageTexture[i];
         m_imageTexture[i] = NULL;
      }
   }

   /* set active flag after setup */
   m_active = true;

   m_enableTrans = false;

   free(buff);
   return true;
}

/* TextArea::setActiveFlag: set active flag */
void TextArea::setActiveFlag(bool flag)
{
   m_active = flag;
}

/* TextArea::setText: set text */
void TextArea::setText(const char *text)
{
   char *buff, *p, *save;
   bool is_image;
   int i, j;
   char c;
   float old_width, old_height;
   float real_w, d;
   float real_h;
   char dummy[] = "x";

   if (m_active == false) return;

   /* is there is previous context, enable transition */
   if (m_imageTexture[m_bid] != NULL || m_elem[m_bid].textLen != 0) {
      m_enableTrans = true;
   } else {
      m_enableTrans = false;
   }
   old_width = m_width;
   old_height = m_height;

   /* store to another buffer */
   if (m_bid == 0)
      m_bid = 1;
   else
      m_bid = 0;

   if (m_text[m_bid]) {
      free(m_text[m_bid]);
      m_text[m_bid] = NULL;
   }
   if (m_imageTexture[m_bid]) {
      delete m_imageTexture[m_bid];
      m_imageTexture[m_bid] = NULL;
   }

   buff = MMDAgent_strdup(text);
   p = MMDAgent_strtok(buff, "|", &save);
   p = MMDAgent_strtok(NULL, "|", &save);

   is_image = false;

   /* first try as image path */
   if (MMDAgent_exist(p)) {
      /* image */
      m_imageTexture[m_bid] = new PMDTexture;
      if (m_imageTexture[m_bid]->loadImage(p) == true) {
         m_imageTexture[m_bid]->setAnimationSpeedRate(1.0);
         m_mmdagent->sendLogString(m_id, MLOG_STATUS, "image \"%s\", %d x %d", p, m_imageTexture[m_bid]->getWidth(), m_imageTexture[m_bid]->getHeight());
         is_image = true;
      } else {
         m_mmdagent->sendLogString(m_id, MLOG_ERROR, "image file \"%s\" exist but failed to load", p);
         delete m_imageTexture[m_bid];
         m_imageTexture[m_bid] = NULL;
      }
   }
   if (is_image == false) {
      /* text */
      /* text re-formatting */
      for (j = 0, i = 0; i < (int)MMDAgent_strlen(p); i++) {
         c = p[i];
         if (c == '_') {
            /* replace '_' to ' ' */
            c = ' ';
         }
         if (p[i] == '\\') {
            /* replace '\' + 'n' or 'r' to '\n' */
            if (i + 1 < (int)MMDAgent_strlen(p) && (p[i + 1] == 'r' || p[i + 1] == 'n')) {
               c = '\n';
               i++;
            }
         }
         p[j++] = c;
      }
      p[j] = '\0';

      m_text[m_bid] = MMDAgent_strdup(p);
   }
   free(buff);

   /* set up text drawing element */
   m_elem[m_bid].textLen = 0;
   m_elem[m_bid].numIndices = 0;
   m_elemOut[m_bid].textLen = 0;
   m_elemOut[m_bid].numIndices = 0;

   /* use 1 character space for background square at text mode or image rendering */
   m_font->getTextDrawElements(dummy, &(m_elem[m_bid]), m_elem[m_bid].textLen, 0.0f, 0.0f, 0.0f);

   if (is_image) {
      /* image */
      float aspect = (m_imageTexture[m_bid] != NULL) ? (float)m_imageTexture[m_bid]->getHeight() / (float)m_imageTexture[m_bid]->getWidth() : 1.0f;
      if (m_width_set <= 0.0f) {
         if (m_height_set <= 0.0f) {
            m_width = (m_imageTexture[m_bid] != NULL) ? (float)m_imageTexture[m_bid]->getWidth() : TEXTAREA_DEFAULT_WIDTH;
            m_height = m_width * aspect;
         } else {
            m_height = m_height_set;
            m_width = m_height / aspect;
         }
      } else {
         m_width = m_width_set;
         if (m_height_set <= 0.0f)
            m_height = m_width * aspect;
         else
            m_height = m_height_set;
      }
   } else {
      /* get text elements */
      m_font->getTextDrawElements(m_text[m_bid], &(m_elem[m_bid]), m_elem[m_bid].textLen, 0.0f, 0.0f, m_linespace);
      if (m_outline) {
         m_font->setZ(&(m_elem[m_bid]), 0.05f);
         m_font->enableOutlineMode(1.0f);
         m_font->getTextDrawElements(m_text[m_bid], &(m_elemOut[m_bid]), m_elemOut[m_bid].textLen, 0.0f, 0.0f, m_linespace);
         m_font->disableOutlineMode();
      }

      /* set width and height from the text */
      m_textsize = m_textsize_set;
      real_w = (m_outline ? m_elemOut[m_bid].width : m_elem[m_bid].width) * m_textsize + m_margin * 2.0f;
      if (m_width_set <= 0.0f) {
         /* set width required by the text */
         m_width = real_w;
      } else {
         m_width = m_width_set;
         if (real_w > m_width_set) {
            /* text is long and required width exceeds specified width, scale down the text size to fit the width */
            d = (real_w - m_margin * 2.0f) / m_textsize;
            m_textsize = (m_width - m_margin * 2.0f) / d;
         }
      }
      real_h = (m_outline ? m_elemOut[m_bid].height : m_elem[m_bid].height) * m_textsize + m_margin * 2.0f;
      if (m_height_set <= 0.0f)
         /* set the height required by the text */
         m_height = real_h;
      else
         m_height = m_height_set;
   }

   /* set background square */
   if (m_width > 0 || m_height > 0)
      updateVertices();

   if (m_enableTrans == true) {
      /* if has previous content but its size changes, disable transition */
      if (old_width != m_width || old_height != m_height)
         m_enableTrans = false;
   }

   m_transFrame = TEXTAREA_TRANSITION_FRAME_LEN;
}

/* TextArea::updateVertices: update vertices */
void TextArea::updateVertices()
{
   float w, h;

   w = m_width * 0.5f;
   h = m_height * 0.5f;
   if (m_imageTexture[m_bid] == NULL) {
      /* set elements for background box */
      m_elem[m_bid].vertices[0] = -w;
      m_elem[m_bid].vertices[1] = -h;
      m_elem[m_bid].vertices[2] = 0;
      m_elem[m_bid].vertices[3] = w;
      m_elem[m_bid].vertices[4] = -h;
      m_elem[m_bid].vertices[5] = 0;
      m_elem[m_bid].vertices[6] = w;
      m_elem[m_bid].vertices[7] = h;
      m_elem[m_bid].vertices[8] = 0;
      m_elem[m_bid].vertices[9] = -w;
      m_elem[m_bid].vertices[10] = h;
      m_elem[m_bid].vertices[11] = 0;
   } else {
      /* set elements for drawing image texture */
      m_elem[m_bid].vertices[0] = -w;
      m_elem[m_bid].vertices[1] = h;
      m_elem[m_bid].vertices[2] = 0;
      m_elem[m_bid].vertices[3] = -w;
      m_elem[m_bid].vertices[4] = -h;
      m_elem[m_bid].vertices[5] = 0;
      m_elem[m_bid].vertices[6] = w;
      m_elem[m_bid].vertices[7] = -h;
      m_elem[m_bid].vertices[8] = 0;
      m_elem[m_bid].vertices[9] = w;
      m_elem[m_bid].vertices[10] = h;
      m_elem[m_bid].vertices[11] = 0;
      m_elem[m_bid].indices[0] = 0;
      m_elem[m_bid].indices[1] = 1;
      m_elem[m_bid].indices[2] = 2;
      m_elem[m_bid].indices[3] = 0;
      m_elem[m_bid].indices[4] = 2;
      m_elem[m_bid].indices[5] = 3;
      m_elem[m_bid].texcoords[0] = 0.0f;
      m_elem[m_bid].texcoords[1] = 0.0f;
      m_elem[m_bid].texcoords[2] = 0.0f;
      m_elem[m_bid].texcoords[3] = 1.0f;
      m_elem[m_bid].texcoords[4] = 1.0f;
      m_elem[m_bid].texcoords[5] = 1.0f;
      m_elem[m_bid].texcoords[6] = 1.0f;
      m_elem[m_bid].texcoords[7] = 0.0f;
   }
}


/* TextArea::getName: get name */
char *TextArea::getName()
{
   return m_name;
}

/* TextArea::matchName: check if name matches*/
bool TextArea::matchName(const char *str)
{
   char *buff, *alias, *save;
   bool ret;

   buff = MMDAgent_strdup(str);
   alias = MMDAgent_strtok(buff, "|", &save);
   ret = MMDAgent_strequal(alias, m_name);
   free(buff);
   return ret;
}

/* TextArea::update: update */
void TextArea::update(double frame)
{
   /* update texture animation */
   for (int i = 0; i < 2; i++) {
      if (m_imageTexture[i] != NULL) {
         m_imageTexture[i]->setNextFrame(frame);
      }
   }

   if (m_transFrame == 0.0)
      return;

   m_transFrame -= frame;
   if (m_transFrame <= 0.0)
      m_transFrame = 0.0;
}

/* TextArea::render: render */
void TextArea::render()
{
   btScalar m[16];
   float w, h;
   float transRate;
   int bid;

   if (m_active == false) return;

   if (m_elem[0].textLen == 0 && m_elem[1].textLen == 0) return;

   glPushMatrix();

   /* apply base bone transform if exist */
   if (m_basebone) {
      m_basebone->getTransform()->getOpenGLMatrix(m);
      glMultMatrixf(m);
   }

   /* apply offset and rotation */
   glMultMatrixf(m_matrix);

   w = m_width * 0.5f;
   h = m_height * 0.5f;
   transRate = (float)(m_transFrame / TEXTAREA_TRANSITION_FRAME_LEN);

   bid = (m_bid + 1) % 2;

   if (m_imageTexture[m_bid] != NULL) {
      /* draw image */
      if (m_enableTrans && (m_imageTexture[bid] != NULL) && m_transFrame != 0.0) {
         /* draw old image as background while transition */
#ifdef MMDAGENT_DEPTHFUNC_DEFAULT_LESS
         glDepthFunc(GL_LEQUAL);
#endif /* MMDAGENT_DEPTHFUNC_DEFAULT_LESS */
         glVertexPointer(3, GL_FLOAT, 0, m_elem[bid].vertices);
         glEnableClientState(GL_TEXTURE_COORD_ARRAY);
         glTexCoordPointer(2, GL_FLOAT, 0, m_elem[bid].texcoords);
         if (m_imageTexture[bid])
            glBindTexture(GL_TEXTURE_2D, m_imageTexture[bid]->getID());
         glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
         glDrawElements(GL_TRIANGLES, 6, GL_INDICES, (const GLvoid *)m_elem[bid].indices);
      }
      glVertexPointer(3, GL_FLOAT, 0, m_elem[m_bid].vertices);
      glEnableClientState(GL_TEXTURE_COORD_ARRAY);
      glTexCoordPointer(2, GL_FLOAT, 0, m_elem[m_bid].texcoords);
      if (m_imageTexture[m_bid])
         glBindTexture(GL_TEXTURE_2D, m_imageTexture[m_bid]->getID());
      glColor4f(1.0f, 1.0f, 1.0f, 1.0f - transRate);
      glDrawElements(GL_TRIANGLES, 6, GL_INDICES, (const GLvoid *)m_elem[m_bid].indices);
#ifdef MMDAGENT_DEPTHFUNC_DEFAULT_LESS
      if (m_enableTrans && m_imageTexture[bid] != NULL && m_transFrame != 0.0)
         glDepthFunc(GL_LESS);
#endif /* MMDAGENT_DEPTHFUNC_DEFAULT_LESS */
   } else {
      /* draw background */
      if (m_width > 0 || m_height > 0) {
         glBindTexture(GL_TEXTURE_2D, 0);
         glVertexPointer(3, GL_FLOAT, 0, m_elem[m_bid].vertices);
         glColor4f(m_bgcolor[0], m_bgcolor[1], m_bgcolor[2], m_bgcolor[3]);
         glDrawElements(GL_TRIANGLES, 6, GL_INDICES, (const GLvoid *)m_elem[m_bid].indices);
      }
      glEnableClientState(GL_TEXTURE_COORD_ARRAY);
      glBindTexture(GL_TEXTURE_2D, m_font->getTextureID());

      if (m_enableTrans && m_elem[bid].textLen != 0 && m_transFrame != 0.0) {
         /* draw old elements while transition */
#ifdef MMDAGENT_DEPTHFUNC_DEFAULT_LESS
         glDepthFunc(GL_LEQUAL);
#endif /* MMDAGENT_DEPTHFUNC_DEFAULT_LESS */
         glPushMatrix();
         glTranslatef(-w + m_margin + TEXTAREA_TRANSITION_OFFSET * transRate, h - m_margin - ((m_outline ? m_elemOut[bid].upheight : m_elem[bid].upheight) * m_textsize), 0.02f);
         glScalef(m_textsize, m_textsize, m_textsize);
         if (m_outline) {
            glColor4f(0.0f, 0.0f, 0.0f, 1.0f - transRate);
            glVertexPointer(3, GL_FLOAT, 0, m_elemOut[m_bid].vertices);
            glTexCoordPointer(2, GL_FLOAT, 0, m_elemOut[bid].texcoords);
            glDrawElements(GL_TRIANGLES, m_elemOut[bid].numIndices, GL_INDICES, (const GLvoid *)m_elemOut[bid].indices);
         }
         glColor4f(m_textcolor[0], m_textcolor[1], m_textcolor[2], m_textcolor[3] * (1.0f - transRate));
         glVertexPointer(3, GL_FLOAT, 0, m_elem[m_bid].vertices);
         glTexCoordPointer(2, GL_FLOAT, 0, m_elem[bid].texcoords);
         glDrawElements(GL_TRIANGLES, m_elem[bid].numIndices - 6, GL_INDICES, (const GLvoid *)&(m_elem[bid].indices[6]));
         glPopMatrix();
      }

      /* draw text */
      glTranslatef(-w + m_margin + TEXTAREA_TRANSITION_OFFSET * transRate, h - m_margin - ((m_outline ? m_elemOut[m_bid].upheight : m_elem[m_bid].upheight) * m_textsize) , 0.02f);
      glScalef(m_textsize, m_textsize, m_textsize);
      if (m_outline) {
         glColor4f(0.0f, 0.0f, 0.0f, 1.0f - transRate);
         glVertexPointer(3, GL_FLOAT, 0, m_elemOut[m_bid].vertices);
         glTexCoordPointer(2, GL_FLOAT, 0, m_elemOut[m_bid].texcoords);
         glDrawElements(GL_TRIANGLES, m_elemOut[m_bid].numIndices, GL_INDICES, (const GLvoid *)m_elemOut[m_bid].indices);
      }
      glColor4f(m_textcolor[0], m_textcolor[1], m_textcolor[2], m_textcolor[3] * (1.0f - transRate));
      glVertexPointer(3, GL_FLOAT, 0, m_elem[m_bid].vertices);
      glTexCoordPointer(2, GL_FLOAT, 0, m_elem[m_bid].texcoords);
      glDrawElements(GL_TRIANGLES, m_elem[m_bid].numIndices - 6, GL_INDICES, (const GLvoid *)&(m_elem[m_bid].indices[6]));
#ifdef MMDAGENT_DEPTHFUNC_DEFAULT_LESS
      if (m_enableTrans && m_elem[bid].textLen != 0 && m_transFrame != 0.0)
         glDepthFunc(GL_LESS);
#endif /* MMDAGENT_DEPTHFUNC_DEFAULT_LESS */
   }
   glDisableClientState(GL_TEXTURE_COORD_ARRAY);

   glPopMatrix();
}
