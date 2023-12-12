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

/* definitions */

/* size, location, linespace, margins */
#define MENU_WIDTH                8     /* menu width */
#define MENU_HEIGHT_RATIO_DEFAULT 0.45f  /* default height of the menu, in ratio of screen height */
#define MENU_SCALEMIN             0.1f  /* minimum scale */
#define MENU_SCALEMAX             1.0f  /* maximum scale */
#define MENU_MARGIN_X             0.1f  /* X margin */
#define MENU_MARGIN_Y             0.1f  /* Y margin */
#define MENU_PADDING_X            0.2f  /* X padding space */
#define MENU_PADDING_Y            0.43f  /* Y padding space */
#define MENU_LINESPACE            0.2f  /* extra line space between items */
#define MENU_BAR_HEIGHT           0.1f  /* menu bar height */
#define MENU_CURSOR_WIDTH         0.1f  /* menu cursor width */
#define MENU_ICON_WIDTH           0.5f  /* menu icon width */
#define MENU_ICON_X1              0.01f /* menu icon left location in ratio */
#define MENU_ICON_X2              0.98f /* menu icon right location in ratio */
#define MENU_ICON_Y               0.15f /* menu icon y location in ratio */
#define MENU_LABELSCALE           0.8f  /* menu text scale factor for main text */
#define MENU_LABELSCALEFORSUB     0.3f  /* menu text scale factor for sub text */
#define MENU_LABELSCALEFORTITLE   0.8f  /* menu text scale factor for stem title text */

/* animation parameters */
#define MENU_DURATION_SHOWHIDE   6.0f  /* animation duration for show/hide */
#define MENU_DURATION_EXEC       7.0f  /* animation duration for item execution */
#define MENU_DURATION_FORWARD    7.0f  /* animation duration for moving forward */
#define MENU_DURATION_BACKWARD   7.0f  /* animation duration for moving backward */
#define MENU_DURATION_JUMP       7.0f  /* animation duration for jump */
#define MENU_DURATION_REGIST     6.0f  /* animation duration for regist */
#define MENU_ROTATION_AXIS       0.0f, 1.0f, -0.3f   /* rotation axis for moving */
#define MENU_DURATION_VSCROLL    4.0f  /* animation duration for vertical scroll */
#define MENU_DURATION_POPUP_SHOWHIDE  6.0f  /* animation duration for popup show/hide */

/* colors */
#define MENU_COLOR_TITLE_CONTENT    0.3f, 0.7f, 0.1f, 0.7f  /* title background color of content stem */
#define MENU_COLOR_TITLE_SYSTEM     0.0f, 0.5f, 0.8f, 0.7f  /* title background color of system stem */
#define MENU_COLOR_TITLE_DEVELOP    0.0f, 0.2f, 0.9f, 0.7f  /* title background color of develop stem */
#define MENU_COLOR_TITLE_TEMPORAL   0.0f, 0.9f, 0.1f, 0.7f  /* title background color of develop stem */
#define MENU_COLOR_BAR              0.9f, 0.9f, 1.0f, 0.8f  /* bar background color */
#define MENU_COLOR_ITEM_EXEC        0.9f, 0.0f, 0.1f, 0.7f  /* background color of execiting item */
#define MENU_COLOR_ITEM_EXEC_IMG    0.9f, 0.0f, 0.1f, 1.0f  /* background color of execiting item on image */
#define MENU_COLOR_ITEM_CURSOR      1.0f, 0.0f, 0.0f, 1.0f  /* background color of item on cursor */
#define MENU_COLOR_STATE_NONE       0.0f, 0.0f, 0.6f, 0.6f  /* background color of non-configured item */
#define MENU_COLOR_STATE_NONE_C     0.0f, 0.2f, 0.5f, 0.6f  /* background color of non-configured item for content */
#define MENU_COLOR_STATE_NORMAL     0.0f, 0.0f, 0.7f, 0.6f  /* background color of item in normal state */
#define MENU_COLOR_STATE_NORMAL_C   0.0f, 0.3f, 0.5f, 0.6f  /* background color of item in normal state for content */
#define MENU_COLOR_STATE_PRESSED    0.7f, 0.4f, 0.0f, 0.8f  /* background color of item in pressed state */
#define MENU_COLOR_STATE_UNKNOWN    1.0f, 0.0f, 0.0f, 0.8f  /* background color of item in unknown state */
#define MENU_COLOR_TEXT             0.8f, 0.8f, 0.8f, 1.0f  /* color of text */
#define MENU_COLOR_SUBTEXT          0.7f, 0.7f, 0.8f, 1.0f  /* color of subtext */
#define MENU_COLOR_TEXT_DISABLED    0.3f, 0.3f, 0.5f, 1.0f  /* color of text in disabled state  */
#define MENU_COLOR_SCROLLINDICATOR  0.8f, 0.9f, 0.5f, 1.0f  /* color of scroll indicator */

#define RENDERING_Z_OFFSET 0.3f


/* Popup::makeBox: make box vertices */
void Popup::makeBox(GLfloat *v, float x, float width)
{
   float h = 1.0f + MENU_LINESPACE;

   v[0] = x;
   v[1] = h;
   v[2] = 0.0f;
   v[3] = x;
   v[4] = 0.0f;
   v[5] = 0.0f;
   v[6] = width;
   v[7] = 0.0f;
   v[8] = 0.0f;
   v[9] = width;
   v[10] = h;
   v[11] = 0.0f;
   v[12] = v[0] + MENU_CURSOR_WIDTH;
   v[13] = v[1] - MENU_CURSOR_WIDTH;
   v[14] = v[2];
   v[15] = v[3] + MENU_CURSOR_WIDTH;
   v[16] = v[4] + MENU_CURSOR_WIDTH;
   v[17] = v[5];
   v[18] = v[6] - MENU_CURSOR_WIDTH;
   v[19] = v[7] + MENU_CURSOR_WIDTH;
   v[20] = v[8];
   v[21] = v[9] - MENU_CURSOR_WIDTH;
   v[22] = v[10] - MENU_CURSOR_WIDTH;
   v[23] = v[11];
}

/* Popup::initialize: initialize */
void Popup::initialize()
{
   m_choices = NULL;
   m_num = 0;
   m_func = NULL;
   m_data = NULL;
   m_font = NULL;
   memset(&m_elem, 0, sizeof(FTGLTextDrawElements));
   m_width = 0.0f;
   m_step = 0.0f;
   m_active = false;
   m_cursorPos = 0;
   m_showHideAnimationFrameLeft = MENU_DURATION_POPUP_SHOWHIDE;
   m_showHideAnimationFrameStep = 0.0f;
}

/* Popup::clear: clear */
void Popup::clear()
{
   int i;

   for (i = 0; i < m_num; i++) {
      if (m_choices[i])
         free(m_choices[i]);
   }
   free(m_choices);
   if (m_elem.vertices) free(m_elem.vertices);
   if (m_elem.texcoords) free(m_elem.texcoords);
   if (m_elem.indices) free(m_elem.indices);

   initialize();
}

/* Popup::Popup: constructor */
Popup::Popup(const char **choices, int num, void(*func)(int id, int row, int chosen, void *data), void *data, FTGLTextureFont *font, float width)
{
   int i;

   initialize();

   m_num = num;
   if (m_num > MENUPOPUPMAXCHOICES)
      m_num = MENUPOPUPMAXCHOICES;
   m_func = func;
   m_data = data;
   m_font = font;
   m_width = width;

   m_step = m_width / (float)m_num;

   m_choices = (char **)malloc(sizeof(char *) * m_num);
   for (i = 0; i < m_num; i++) {
      m_choices[i] = MMDAgent_strdup(choices[i]);
      if (m_choices[i] != NULL) {
         if (font->getTextDrawElementsFixed(m_choices[i], &m_elem, m_elem.textLen, m_step * i + MENU_PADDING_X, MENU_PADDING_Y, 0.0f, m_step - MENU_PADDING_X * 2.0f, MENU_LABELSCALE) == false) {
            free(m_choices[i]);
            m_choices[i] = NULL;
         }
      }
   }

   makeBox(m_verticesBoxFull, 0.0f, m_width);
   makeBox(m_verticesBoxCursor, 0.0f, m_step);

}

/* Popup::~Popup: destructor */
Popup::~Popup()
{
   clear();
}

/* Popup::activate: activate */
void Popup::activate()
{
   if (m_active == false) {
      m_active = true;
      m_cursorPos = 0;
      m_showHideAnimationFrameStep = -1.0f;
   }
}

/* Popup::deactivate: deactivate */
void Popup::deactivate()
{
   if (m_active == true) {
      m_active = false;
      m_showHideAnimationFrameStep = 1.0f;
   }
}

/* Popup::forceActivate: force activate */
void Popup::forceActivate()
{
   activate();
   m_showHideAnimationFrameLeft = 0.0f;
}

/* Popup::forceDeactivate: force deactivate */
void Popup::forceDeactivate()
{
   deactivate();
   m_showHideAnimationFrameLeft = MENU_DURATION_POPUP_SHOWHIDE;
}

/* Popup::isActive: return true when active */
bool Popup::isActive()
{
   return m_active;
}

/* Popup::update: update status */
void Popup::update(double ellapsedFrame)
{
   m_showHideAnimationFrameLeft += (float)ellapsedFrame * m_showHideAnimationFrameStep;
   if (m_showHideAnimationFrameLeft > MENU_DURATION_POPUP_SHOWHIDE)
      m_showHideAnimationFrameLeft = MENU_DURATION_POPUP_SHOWHIDE;
   if (m_showHideAnimationFrameLeft < 0.0f)
      m_showHideAnimationFrameLeft = 0.0f;
}

/* Popup::renderBegin: render at beginning */
void Popup::renderBegin()
{
   if (m_active == false && m_showHideAnimationFrameLeft == MENU_DURATION_POPUP_SHOWHIDE)
      return;

   if (m_num == 0)
      return;

   if (m_elem.numIndices <= 0)
      return;

   glTranslatef(0.0f, 0.5f, -0.5f);
   glRotatef(-90.0f * (1.0f - m_showHideAnimationFrameLeft / MENU_DURATION_POPUP_SHOWHIDE), 1.0f, 0.0f, 0.0f);
   glTranslatef(0.0f, -0.5f, 0.5f);

}

/* Popup::renderEnd: render at end */
void Popup::renderEnd()
{
   GLindices bgIndices[6] = { 0, 1, 2, 0, 2, 3 };
   GLindices cursorIndices[24] = { 0, 1, 5, 0, 5, 4, 0, 4, 7, 0, 7, 3, 5, 1, 2, 5, 2, 6, 6, 2, 3, 6, 3, 7 };

   if (m_active == false && m_showHideAnimationFrameLeft == MENU_DURATION_POPUP_SHOWHIDE)
      return;

   if (m_num == 0)
      return;

   if (m_elem.numIndices <= 0)
      return;

   glPushMatrix();
   if (m_showHideAnimationFrameLeft > 0.0f) {
      glTranslatef(0.0f, 0.5f, -0.5f);
      glRotatef(90.0f * m_showHideAnimationFrameLeft / MENU_DURATION_POPUP_SHOWHIDE, 1.0f, 0.0f, 0.0f);
      glTranslatef(0.0f, -0.5f, 0.5f);
   }
   glColor4f(0.0f, 0.0f, 0.0f, 0.7f);
   glVertexPointer(3, GL_FLOAT, 0, m_verticesBoxFull);
   glDrawElements(GL_TRIANGLES, 6, GL_INDICES, (const GLvoid *)bgIndices);
   glTranslatef(0.0f, 0.0f, RENDERING_Z_OFFSET + 0.1f);
   glEnable(GL_TEXTURE_2D);
   glBindTexture(GL_TEXTURE_2D, m_font->getTextureID());
   glEnableClientState(GL_TEXTURE_COORD_ARRAY);
   glColor4f(MENU_COLOR_TEXT);
   glVertexPointer(3, GL_FLOAT, 0, m_elem.vertices);
   glTexCoordPointer(2, GL_FLOAT, 0, m_elem.texcoords);
   glDrawElements(GL_TRIANGLES, m_elem.numIndices, GL_INDICES, (const GLvoid *)m_elem.indices);
   glDisableClientState(GL_TEXTURE_COORD_ARRAY);
   glDisable(GL_TEXTURE_2D);

   glTranslatef(m_cursorPos * m_step, 0.0f, 0.0f);
   glColor4f(1.0f, 1.0f, 0.0f, 1.0f);
   glVertexPointer(3, GL_FLOAT, 0, m_verticesBoxCursor);
   glDrawElements(GL_TRIANGLES, 24, GL_INDICES, (const GLvoid *)cursorIndices);

   glPopMatrix();
}

/* Popup::forward: move forward */
void Popup::forward()
{
   m_cursorPos++;
   if (m_cursorPos >= m_num)
      m_cursorPos -= m_num;
}

/* Popup::backward: move backward */
void Popup::backward()
{
   m_cursorPos--;
   if (m_cursorPos < 0)
      m_cursorPos += m_num;
}

/* Popup::execCurrent: execute current item */
void Popup::execCurrent(int id, int row)
{
   m_func(id, row, m_cursorPos, m_data);
}

/* Popup::execByPosition: execute by position */
void Popup::execByPosition(int id, int row, float rpos)
{
   int pos;

   if (rpos < 0.0f)
      pos = 0;
   else if (rpos > 1.0f)
     pos = m_num - 1;
   else
     pos = (int)(rpos * m_num);
   m_cursorPos = pos;
   execCurrent(id, row);
}


/* Menu::initialize: initialize menu */
void Menu::initialize()
{
   int i;
   float h;

   for (i = 0; i < MENUMAXNUM; i++) {
      initializeStem(i);
      m_currentCursor[i] = m_prevCursor[i] = -1;
      m_currentTopItem[i] = m_prevTopItem[i] = 0;
   }

   m_mmdagent = NULL;
   m_id = 0;
   m_font = NULL;
   m_icon = NULL;

   m_currentCountId = 0;
   m_stemMaxNum = 0;

   m_needsUpdate = false;
   m_orderNum = 0;
   m_currentId = 0;
   m_currentPos = 0;
   m_showing = false;
   m_poppingRow = -1;
   m_popAnimatingRow = -1;
   m_popAnimatingId = -1;
   m_inhibitFlip = false;

   m_orientation = MENU_ORIENTATION;
   m_cWidth = (float)MENU_WIDTH + MENU_PADDING_X;
   m_cHeight = (1.0f + MENU_LINESPACE) * (MENUHEIGHT + 1) + MENU_PADDING_Y;
   m_size = MENU_HEIGHT_RATIO_DEFAULT;
   m_savedSize = m_size;

   m_viewWidth = 0;
   m_viewHeight = 0;
   m_paddingY = 0.0f;

   m_cursorShow = false;
   m_showHideAnimationFrameLeft = MENU_DURATION_SHOWHIDE;
   m_execItemAnimationFrameLeft = 0.0f;
   m_forwardAnimationFrameLeft = 0.0f;
   m_backwardAnimationFrameLeft = 0.0f;
   m_jumpAnimationFrameLeft = 0.0f;
   m_forwardRegistAnimationFrameLeft = 0.0f;
   m_backwardRegistAnimationFrameLeft = 0.0f;
   m_isRegisting = false;
   m_forwardFrameForced = false;
   m_backwardFrameForced = false;
   m_vscrollAnimationFrameLeft = 0.0f;
   m_forceShowAnimationFlag = false;

   h = 1.0f + MENU_LINESPACE;
   m_verticesBox[0] = 0;
   m_verticesBox[1] = h;
   m_verticesBox[2] = -0.1f;
   m_verticesBox[3] = 0;
   m_verticesBox[4] = 0;
   m_verticesBox[5] = -0.1f;
   m_verticesBox[6] = m_cWidth;
   m_verticesBox[7] = 0;
   m_verticesBox[8] = -0.1f;
   m_verticesBox[9] = m_cWidth;
   m_verticesBox[10] = h;
   m_verticesBox[11] = -0.1f;
   m_verticesBox[12] = m_verticesBox[0] + MENU_CURSOR_WIDTH;
   m_verticesBox[13] = m_verticesBox[1] - MENU_CURSOR_WIDTH;
   m_verticesBox[14] = m_verticesBox[2];
   m_verticesBox[15] = m_verticesBox[3] + MENU_CURSOR_WIDTH;
   m_verticesBox[16] = m_verticesBox[4] + MENU_CURSOR_WIDTH;
   m_verticesBox[17] = m_verticesBox[5];
   m_verticesBox[18] = m_verticesBox[6] - MENU_CURSOR_WIDTH;
   m_verticesBox[19] = m_verticesBox[7] + MENU_CURSOR_WIDTH;
   m_verticesBox[20] = m_verticesBox[8];
   m_verticesBox[21] = m_verticesBox[9] - MENU_CURSOR_WIDTH;
   m_verticesBox[22] = m_verticesBox[10] - MENU_CURSOR_WIDTH;
   m_verticesBox[23] = m_verticesBox[11];
   h *= (float)(MENUHEIGHT + 1);
   m_verticesAll[0] = 0;
   m_verticesAll[1] = 0;
   m_verticesAll[2] = -0.2f;
   m_verticesAll[3] = 0;
   m_verticesAll[4] = -h;
   m_verticesAll[5] = -0.2f;
   m_verticesAll[6] = m_cWidth;
   m_verticesAll[7] = -h;
   m_verticesAll[8] = -0.2f;
   m_verticesAll[9] = m_cWidth;
   m_verticesAll[10] = 0;
   m_verticesAll[11] = -0.2f;
   m_verticesIcon[0] = 0;
   m_verticesIcon[1] = MENU_ICON_WIDTH;
   m_verticesIcon[2] = 0.0f;
   m_verticesIcon[3] = 0;
   m_verticesIcon[4] = 0;
   m_verticesIcon[5] = 0.0f;
   m_verticesIcon[6] = MENU_ICON_WIDTH;
   m_verticesIcon[7] = 0;
   m_verticesIcon[8] = 0.0f;
   m_verticesIcon[9] = MENU_ICON_WIDTH;
   m_verticesIcon[10] = MENU_ICON_WIDTH;
   m_verticesIcon[11] = 0.0f;

}

/* Menu::clear: free menu */
void Menu::clear()
{
   int i;

   for (i = 0; i < MENUMAXNUM; i++)
      clearStem(i);

   if (m_icon)
      delete m_icon;

   initialize();
}

/* Menu::loadIcon: load Icon */
bool Menu::loadIcon(const char *appDirName)
{
   char buff[MMDAGENT_MAXBUFLEN];
   PMDTexture *tex;

   MMDAgent_snprintf(buff, MMDAGENT_MAXBUFLEN, "%s%c%s", appDirName, MMDAGENT_DIRSEPARATOR, MENU_ICONPATH);

   tex = new PMDTexture;
   if (tex->loadImage(buff)) {
      m_icon = tex;
   } else {
      delete tex;
      return false;
   }

   return true;
}

/* Menu::initializeStem: initialize a stem */
void Menu::initializeStem(int id)
{
   int i, j;

   m_stem[id].active = false;
   m_stem[id].priority = MENUPRIORITY_CONTENT;
   m_stem[id].countId = 0;
   m_stem[id].sortedPos = 0;
   m_stem[id].name = NULL;
   for (i = 0; i < MENUMAXITEM; i++) {
      m_stem[id].itemName[i] = NULL;
      m_stem[id].subtext[i] = NULL;
      m_stem[id].messageType[i] = NULL;
      m_stem[id].messageArg[i] = NULL;
      m_stem[id].image[i] = NULL;
      m_stem[id].status[i] = MENUITEM_STATUS_NONE;
      for (j = 0; j < MENU_ICONNUM; j++)
         m_stem[id].iconStatus[i][j] = false;
      m_stem[id].popup[i] = NULL;
   }
   m_stem[id].func = NULL;
   m_stem[id].data = NULL;
   for (i = 0; i < MENUMAXITEM + 1; i++) {
      memset(&(m_stem[id].elem[i]), 0, sizeof(FTGLTextDrawElements));
      memset(&(m_stem[id].subelem[i]), 0, sizeof(FTGLTextDrawElements));
      memset(&(m_stem[id].elemOut[i]), 0, sizeof(FTGLTextDrawElements));
      memset(&(m_stem[id].subelemOut[i]), 0, sizeof(FTGLTextDrawElements));
   }
   m_stem[id].needsUpdate = false;
   m_stem[id].maxValidItemId = 0;
   m_stem[id].bgImage = NULL;
   m_stem[id].skip = false;
   m_stem[id].hasCustomTitleColor = false;
}

/* Menu::clearStem: clear a stem */
void Menu::clearStem(int id)
{
   int i;

   if (m_stem[id].name)
      free(m_stem[id].name);
   for (i = 0; i < MENUMAXITEM; i++) {
      if (m_stem[id].itemName[i])
         free(m_stem[id].itemName[i]);
      if (m_stem[id].subtext[i])
         free(m_stem[id].subtext[i]);
      if (m_stem[id].messageType[i])
         free(m_stem[id].messageType[i]);
      if (m_stem[id].messageArg[i])
         free(m_stem[id].messageArg[i]);
      if (m_stem[id].image[i])
         delete m_stem[id].image[i];
      if (m_stem[id].popup[i])
         delete m_stem[id].popup[i];
   }
   for (i = 0; i < MENUMAXITEM; i++) {
      if (m_stem[id].elem[i].vertices) free(m_stem[id].elem[i].vertices);
      if (m_stem[id].elem[i].texcoords) free(m_stem[id].elem[i].texcoords);
      if (m_stem[id].elem[i].indices) free(m_stem[id].elem[i].indices);
      if (m_stem[id].subelem[i].vertices) free(m_stem[id].subelem[i].vertices);
      if (m_stem[id].subelem[i].texcoords) free(m_stem[id].subelem[i].texcoords);
      if (m_stem[id].subelem[i].indices) free(m_stem[id].subelem[i].indices);
      if (m_stem[id].elemOut[i].vertices) free(m_stem[id].elemOut[i].vertices);
      if (m_stem[id].elemOut[i].texcoords) free(m_stem[id].elemOut[i].texcoords);
      if (m_stem[id].elemOut[i].indices) free(m_stem[id].elemOut[i].indices);
      if (m_stem[id].subelemOut[i].vertices) free(m_stem[id].subelemOut[i].vertices);
      if (m_stem[id].subelemOut[i].texcoords) free(m_stem[id].subelemOut[i].texcoords);
      if (m_stem[id].subelemOut[i].indices) free(m_stem[id].subelemOut[i].indices);
   }
   if (m_stem[id].bgImage)
      delete m_stem[id].bgImage;
}

/* Menu::updatePosition: update menu position */
void Menu::updateMenuPosition()
{
   float scale, rate;

   /* compute relative height from aspect */
   rate = sqrtf((float)m_viewWidth / (float)m_viewHeight) * 1.333f * m_size;
   if (rate < 0.5f)
      rate = 0.5f;
   if (rate > 1.0f)
      rate = 1.0f;
   scale = rate * m_viewHeight / m_cHeight;
   m_width = (float)m_viewWidth / scale;
   m_height = (float)m_viewHeight / scale;

   /* compute position and range */
   if (m_orientation == MENU_ORIENTATION_TOP_LEFT || m_orientation == MENU_ORIENTATION_BOTTOM_LEFT) {
      /* aligned to left */
      m_posX = MENU_MARGIN_X;
      m_rx1 = 0.0f;
      m_rx2 = m_cWidth / m_width;
   } else {
      /* aligned to right */
      m_posX = m_width - MENU_MARGIN_X;
      m_rx1 = 1.0f - m_cWidth / m_width;
      m_rx2 = 1.0f;
   }
   if (m_orientation == MENU_ORIENTATION_TOP_LEFT || m_orientation == MENU_ORIENTATION_TOP_RIGHT) {
      /* aligned to top */
      m_posY = m_height - MENU_MARGIN_Y;
      m_ry1 = 1.0f - m_cHeight / m_height;
      m_ry2 = 1.0f;
   } else {
      /* aligned to bottom */
      m_posY = MENU_MARGIN_Y;
      m_ry1 = m_paddingY / m_height;
      m_ry2 = (m_cHeight + m_paddingY) / m_height;
   }
}

/* Menu::updateStem: update stem for rendering */
void Menu::updateStem(int id)
{
   int i;
   float xpad;
   float bx1, bx2;

   if (m_font == NULL)
      return;

   if (m_stem[id].active == false)
      return;

   m_stem[id].maxValidItemId = 0;
   for (i = 0; i < MENUMAXITEM; i++) {
      if (m_stem[id].status[i] != MENUITEM_STATUS_NONE)
         m_stem[id].maxValidItemId = i;
   }

   for (i = 0; i < MENUMAXITEM + 1; i++) {
      m_stem[id].elem[i].textLen = 0;
      m_stem[id].elem[i].numIndices = 0;
      m_stem[id].subelem[i].textLen = 0;
      m_stem[id].subelem[i].numIndices = 0;
      m_stem[id].elemOut[i].textLen = 0;
      m_stem[id].elemOut[i].numIndices = 0;
      m_stem[id].subelemOut[i].textLen = 0;
      m_stem[id].subelemOut[i].numIndices = 0;
   }

   if (m_font->getTextDrawElementsWithScale(m_stem[id].name, &(m_stem[id].elem[0]), 0, MENU_PADDING_X, MENU_PADDING_Y, 0.0f, MENU_LABELSCALEFORTITLE) == false) {
      m_stem[id].elem[0].textLen = 0; /* reset */
      m_stem[id].elem[0].numIndices = 0;
      return;
   }
   m_font->setZ(&(m_stem[id].elem[0]), 0.05f);
   m_font->enableOutlineMode();
   if (m_font->getTextDrawElementsWithScale(m_stem[id].name, &(m_stem[id].elemOut[0]), 0, MENU_PADDING_X, MENU_PADDING_Y, 0.0f, MENU_LABELSCALEFORTITLE) == false) {
      m_stem[id].elemOut[0].textLen = 0; /* reset */
      m_stem[id].elemOut[0].numIndices = 0;
   }
   m_font->disableOutlineMode();

   /* vertices for items */
   for (i = 0; i < MENUMAXITEM; i++) {
      if (m_stem[id].itemName[i] != NULL) {
         if (m_font->getTextDrawElementsFixed(m_stem[id].itemName[i], &(m_stem[id].elem[i + 1]), m_stem[id].elem[i + 1].textLen, MENU_PADDING_X, MENU_PADDING_Y, 0.0f, MENU_WIDTH - MENU_PADDING_X, MENU_LABELSCALE) == false) {
            m_stem[id].elem[i + 1].textLen = 0; /* reset */
            m_stem[id].elem[i + 1].numIndices = 0;
            free(m_stem[id].itemName[i]);
            m_stem[id].itemName[i] = NULL;
         } else {
            m_font->setZ(&(m_stem[id].elem[i + 1]), 0.05f);
            m_font->enableOutlineMode();
            if (m_font->getTextDrawElementsFixed(m_stem[id].itemName[i], &(m_stem[id].elemOut[i + 1]), m_stem[id].elemOut[i + 1].textLen, MENU_PADDING_X, MENU_PADDING_Y, 0.0f, MENU_WIDTH - MENU_PADDING_X, MENU_LABELSCALE) == false) {
               m_stem[id].elemOut[i + 1].textLen = 0; /* reset */
               m_stem[id].elemOut[i + 1].numIndices = 0;
            }
            m_font->disableOutlineMode();
         }
      }
      if (m_stem[id].subtext[i] != NULL) {
         if (m_font->getTextDrawElementsFixed(m_stem[id].subtext[i], &(m_stem[id].subelem[i + 1]), m_stem[id].subelem[i + 1].textLen, MENU_PADDING_X, MENU_PADDING_Y * 0.3f, 0.0f, MENU_WIDTH - MENU_PADDING_X, MENU_LABELSCALEFORSUB) == false) {
            m_stem[id].subelem[i + 1].textLen = 0; /* reset */
            m_stem[id].subelem[i + 1].numIndices = 0;
            free(m_stem[id].subtext[i]);
            m_stem[id].subtext[i] = NULL;
         } else {
            xpad = (MENU_WIDTH - MENU_PADDING_X * 2) - m_stem[id].subelem[i + 1].width;
            m_font->addOffset(&(m_stem[id].subelem[i + 1]), xpad, 0.0f, 0.15f);
            m_font->enableOutlineMode();
            if (m_font->getTextDrawElementsFixed(m_stem[id].subtext[i], &(m_stem[id].subelemOut[i + 1]), m_stem[id].subelemOut[i + 1].textLen, MENU_PADDING_X, MENU_PADDING_Y * 0.3f, 0.0f, MENU_WIDTH - MENU_PADDING_X, MENU_LABELSCALEFORSUB) == false) {
               m_stem[id].subelemOut[i + 1].textLen = 0; /* reset */
               m_stem[id].subelemOut[i + 1].numIndices = 0;
            }
            m_font->disableOutlineMode();
            m_font->addOffset(&(m_stem[id].subelemOut[i + 1]), xpad, 0.0f, 0.1f);
         }
      }
   }

   /* vertices for bar */
   bx1 = (m_stem[id].sortedPos / (float)m_orderNum) * m_cWidth;
   bx2 = ((m_stem[id].sortedPos + 1) / (float)m_orderNum) * m_cWidth;
   m_stem[id].vertices_bar[0] = bx1;
   m_stem[id].vertices_bar[1] = 0;
   m_stem[id].vertices_bar[2] = -0.1f;
   m_stem[id].vertices_bar[3] = bx1;
   m_stem[id].vertices_bar[4] = -MENU_BAR_HEIGHT;
   m_stem[id].vertices_bar[5] = -0.1f;
   m_stem[id].vertices_bar[6] = bx2;
   m_stem[id].vertices_bar[7] = -MENU_BAR_HEIGHT;
   m_stem[id].vertices_bar[8] = -0.1f;
   m_stem[id].vertices_bar[9] = bx2;
   m_stem[id].vertices_bar[10] = 0;
   m_stem[id].vertices_bar[11] = -0.1f;

}

/* Menu::sortStem: sort stem */
void Menu::sortStem()
{
   int i, k;
   bool swapped;

   k = 0;
   for (i = 0; i < m_stemMaxNum; i++) {
      if (m_stem[i].active == false)
         continue;
      if (m_stem[i].skip)
         continue;
      m_order[k] = i;
      k++;
   }
   m_orderNum = k;

   /* sort by priority and countId, store order in m_order[] */
   do {
      swapped = false;
      for (i = 0; i < m_orderNum - 1; i++) {
         if (m_stem[m_order[i]].priority > m_stem[m_order[i + 1]].priority
               || (m_stem[m_order[i]].priority == m_stem[m_order[i + 1]].priority && m_stem[m_order[i]].countId < m_stem[m_order[i + 1]].countId)) {
            k = m_order[i];
            m_order[i] = m_order[i + 1];
            m_order[i + 1] = k;
            swapped = true;
         }
      }
   } while (swapped == true);

   for (i = 0; i < m_orderNum; i++)
      m_stem[m_order[i]].sortedPos = i;

}

/* Menu::resetTemporalStatus: reset temporal status */
void Menu::resetTemporalStatus()
{
   int i;
   bool removed;

   /* remove temporal stems */
   removed = false;
   for (i = 0; i < m_stemMaxNum; i++) {
      if (m_stem[i].active == false)
         continue;
      if (m_stem[i].priority == MENUPRIORITY_TEMPORAL) {
         remove(i);
         removed = true;
      }
   }
   /* reset menu position if any removal was occured */
   if (removed)
      jump(0);
   /* re-enable flipping */
   enableForwardBackward();
   /* reset size */
   setSize(m_savedSize);
}


/* Menu::renderBegin: render begin */
void Menu::renderBegin()
{
   int w, h;

   m_mmdagent->getWindowSize(&w, &h);
   if (m_viewWidth != w || m_viewHeight != h) {
      m_viewWidth = w;
      m_viewHeight = h;
      updateMenuPosition();
   }

   glDisable(GL_CULL_FACE);
   glDisable(GL_LIGHTING);
   glMatrixMode(GL_PROJECTION);
   glPushMatrix();
   glLoadIdentity();
   MMDAgent_setOrtho(0, m_width, 0, m_height, -1.0f, 1.0f);
   glMatrixMode(GL_MODELVIEW);
   glPushMatrix();
   glLoadIdentity();
#ifdef MMDAGENT_DEPTHFUNC_DEFAULT_LESS
   glDepthFunc(GL_LEQUAL);
#endif /* MMDAGENT_DEPTHFUNC_DEFAULT_LESS */
   glActiveTexture(GL_TEXTURE0);
   glClientActiveTexture(GL_TEXTURE0);
   glEnableClientState(GL_VERTEX_ARRAY);
   glTranslatef(m_posX, m_posY + m_paddingY, RENDERING_Z_OFFSET);
}

/* Menu::renderStem: render a stem */
void Menu::renderStem(int id)
{
   int i, j;
   GLindices bgIndices[6] = {0, 1, 2, 0, 2, 3};
   GLindices cursorIndices[24] = { 0, 1, 5, 0, 5, 4, 0, 4, 7, 0, 7, 3, 5, 1, 2, 5, 2, 6, 6, 2, 3, 6, 3, 7 };
   GLfloat bgTexcoords[8] = { 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f };
   GLfloat iconTexcoords[8] = { 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f };
   float r, x1, y, h;
   GLfloat triVerticesUpper[9] = { MENU_WIDTH - 0.5f, -0.53f, 0.2f, MENU_WIDTH, -0.53f, 0.2f, MENU_WIDTH - 0.25f, -0.1f, 0.2f };
   GLfloat triVerticesLower[9] = { MENU_WIDTH - 0.5f, MENU_BAR_HEIGHT + 0.6f, 0.2f, MENU_WIDTH - 0.25f, MENU_BAR_HEIGHT + 0.17f, 0.2f, MENU_WIDTH, MENU_BAR_HEIGHT + 0.6f, 0.2f };
   bool scrolling;
   float yoffset;
   int start_row, end_row;
   bool hasIcon;

   /* check if we are in smooth scrolling duration */
   scrolling = false;
   if (m_currentTopItem[id] != m_prevTopItem[id] && m_vscrollAnimationFrameLeft > 0.0f)
      scrolling = true;

   /* set initial position */
   h = 1.0f + MENU_LINESPACE;
   if (m_orientation == MENU_ORIENTATION_TOP_LEFT || m_orientation == MENU_ORIENTATION_BOTTOM_LEFT) {
      /* aligned to left */
      x1 = 0.0f;
   } else {
      /* aligned to right */
      x1 = -m_cWidth;
   }
   if (m_orientation == MENU_ORIENTATION_TOP_LEFT || m_orientation == MENU_ORIENTATION_TOP_RIGHT) {
      /* aligned to top */
      y = 0.0f;
   } else {
      /* aligned to bottom */
      y = h * (float)(MENUHEIGHT + 1);
   }

   /* move on top */
   glTranslatef(x1, y, 0.0f);
   /* draw background image if given */
   if (m_stem[id].bgImage) {
      glColor4f(1.0f, 1.0f, 1.0f, 0.85f);
      glVertexPointer(3, GL_FLOAT, 0, m_verticesAll);
      glEnable(GL_TEXTURE_2D);
      glEnableClientState(GL_TEXTURE_COORD_ARRAY);
      glTexCoordPointer(2, GL_FLOAT, 0, bgTexcoords);
      glBindTexture(GL_TEXTURE_2D, m_stem[id].bgImage->getID());
      glDrawElements(GL_TRIANGLES, 6, GL_INDICES, (const GLvoid *)bgIndices);
      glDisableClientState(GL_TEXTURE_COORD_ARRAY);
      glDisable(GL_TEXTURE_2D);
   }

   glTranslatef(0.0f, -h, 0.0f);

   /* draw title */
   if (m_stem[id].hasCustomTitleColor) {
      glColor4f(m_stem[id].customTitleColor[0], m_stem[id].customTitleColor[1], m_stem[id].customTitleColor[2], m_stem[id].customTitleColor[3]);
   } else {
      switch (m_stem[id].priority) {
      case MENUPRIORITY_CONTENT:
         glColor4f(MENU_COLOR_TITLE_CONTENT);
         break;
      case MENUPRIORITY_SYSTEM:
         glColor4f(MENU_COLOR_TITLE_SYSTEM);
         break;
      case MENUPRIORITY_DEVELOP:
         glColor4f(MENU_COLOR_TITLE_DEVELOP);
         break;
      case MENUPRIORITY_TEMPORAL:
         glColor4f(MENU_COLOR_TITLE_TEMPORAL);
         break;
      }
   }
   glVertexPointer(3, GL_FLOAT, 0, m_verticesBox);
   glDrawElements(GL_TRIANGLES, 6, GL_INDICES, (const GLvoid *)bgIndices);
   if (m_stem[id].elem[0].numIndices > 0) {
      glEnable(GL_TEXTURE_2D);
      glBindTexture(GL_TEXTURE_2D, m_font->getTextureID());
      glEnableClientState(GL_TEXTURE_COORD_ARRAY);
      if (m_stem[id].elemOut[0].numIndices > 0) {
         glColor4f(0.0f, 0.0f, 0.0f, 1.0f);
         glVertexPointer(3, GL_FLOAT, 0, m_stem[id].elemOut[0].vertices);
         glTexCoordPointer(2, GL_FLOAT, 0, m_stem[id].elemOut[0].texcoords);
         glDrawElements(GL_TRIANGLES, m_stem[id].elemOut[0].numIndices, GL_INDICES, (const GLvoid *)m_stem[id].elemOut[0].indices);
      }
      glColor4f(MENU_COLOR_TEXT);
      glVertexPointer(3, GL_FLOAT, 0, m_stem[id].elem[0].vertices);
      glTexCoordPointer(2, GL_FLOAT, 0, m_stem[id].elem[0].texcoords);
      glDrawElements(GL_TRIANGLES, m_stem[id].elem[0].numIndices, GL_INDICES, (const GLvoid *)m_stem[id].elem[0].indices);
      glDisableClientState(GL_TEXTURE_COORD_ARRAY);
      glDisable(GL_TEXTURE_2D);
   }

   /* store the top position */
   glPushMatrix();

   if (scrolling) {
      /* prepare additional rendering while scrolling */
      float r = m_vscrollAnimationFrameLeft / MENU_DURATION_VSCROLL;
      /* set marginal offset */
      if (m_prevTopItem[id] < m_currentTopItem[id])
         yoffset = (1.0f - r) * (m_currentTopItem[id] - m_prevTopItem[id]) * h;
      else
         yoffset = r * (m_prevTopItem[id] - m_currentTopItem[id]) * h;
      /* expand rendering region */
      start_row = (m_currentTopItem[id] < m_prevTopItem[id]) ? m_currentTopItem[id] : m_prevTopItem[id];
      end_row = ((m_currentTopItem[id] > m_prevTopItem[id]) ? m_currentTopItem[id] : m_prevTopItem[id]) + MENUHEIGHT;
      /* use stencil buffer to draw only the visible area of items */
      glStencilMask(0xFFFF);
      glClear(GL_STENCIL_BUFFER_BIT);
      glEnable(GL_STENCIL_TEST);
      glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
      glDepthMask(GL_FALSE);
      glStencilFunc(GL_ALWAYS, 1, ~0);
      glStencilOp(GL_REPLACE, GL_REPLACE, GL_REPLACE);
      glPushMatrix();
      for (i = 0; i < MENUHEIGHT; i++) {
         glTranslatef(0.0f, -h, 0.0f);
         glVertexPointer(3, GL_FLOAT, 0, m_verticesBox);
         glDrawElements(GL_TRIANGLES, 6, GL_INDICES, (const GLvoid *)bgIndices);
      }
      glPopMatrix();
      glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
      glStencilFunc(GL_EQUAL, 1, ~0);
      glStencilMask(0x0);
      glDepthMask(GL_TRUE);
      /* apply the marginal offset */
      glTranslatef(0.0f, yoffset, 0.0f);
   } else {
      start_row = m_currentTopItem[id];
      end_row = m_currentTopItem[id] + MENUHEIGHT;
   }

   /* draw items */
   for (i = start_row; (i < end_row && i < MENUMAXITEM); i++) {
      hasIcon = false;
      if (m_icon) {
         for (j = 0; j < MENU_ICONNUM; j++) {
            if (m_stem[id].iconStatus[i][j]) {
               hasIcon = true;
               break;
            }
         }
      }
      glTranslatef(0.0f, -h, 0.0f);
      if (m_stem[id].popup[i]) {
         glPushMatrix();
         m_stem[id].popup[i]->renderBegin();
      }
      if (m_execItemAnimationFrameLeft > 0.0f &&  id == m_execPos && i == m_execItemId) {
         /* in exec animation */
         glPushMatrix();
         r = m_execItemAnimationFrameLeft / MENU_DURATION_EXEC;
         r = - 2.0f * r * (1.0f - r);
         glTranslatef(r, 0.0f, 0.0f);
         if (m_stem[id].image[i])
            glColor4f(MENU_COLOR_ITEM_EXEC_IMG);
         else
            glColor4f(MENU_COLOR_ITEM_EXEC);
      } else if (m_stem[id].image[i]) {
         /* has image */
         glColor4f(1.0f, 1.0f, 1.0f, 0.85f);
      } else if (m_stem[id].bgImage) {
         /* has bg image */
         glColor4f(0.0f, 0.0f, 0.0f, 0.0f);
      } else {
         switch (m_stem[id].status[i]) {
         case MENUITEM_STATUS_NONE:
            if (m_stem[id].priority == MENUPRIORITY_CONTENT)
               glColor4f(MENU_COLOR_STATE_NONE_C);
            else
               glColor4f(MENU_COLOR_STATE_NONE);
            break;
         case MENUITEM_STATUS_NORMAL:
         case MENUITEM_STATUS_DISABLED:
            if (m_stem[id].priority == MENUPRIORITY_CONTENT)
               glColor4f(MENU_COLOR_STATE_NORMAL_C);
            else
               glColor4f(MENU_COLOR_STATE_NORMAL);
            break;
         case MENUITEM_STATUS_PRESSED:
            glColor4f(MENU_COLOR_STATE_PRESSED);
            break;
         default:
            glColor4f(MENU_COLOR_STATE_UNKNOWN);
         }
      }
      glVertexPointer(3, GL_FLOAT, 0, m_verticesBox);
      if (m_stem[id].image[i]) {
         /* draw image */
         glEnable(GL_TEXTURE_2D);
         glEnableClientState(GL_TEXTURE_COORD_ARRAY);
         glTexCoordPointer(2, GL_FLOAT, 0, bgTexcoords);
         glBindTexture(GL_TEXTURE_2D, m_stem[id].image[i]->getID());
      }
      glDrawElements(GL_TRIANGLES, 6, GL_INDICES, (const GLvoid *)bgIndices);
      if (m_stem[id].elem[i + 1].numIndices > 0) {
         /* draw text */
         if (m_stem[id].image[i] == NULL) {
            glEnable(GL_TEXTURE_2D);
            glEnableClientState(GL_TEXTURE_COORD_ARRAY);
         }
         glBindTexture(GL_TEXTURE_2D, m_font->getTextureID());
         if (m_stem[id].elemOut[i + 1].numIndices > 0) {
            glColor4f(0.0f, 0.0f, 0.0f, 1.0f);
            glVertexPointer(3, GL_FLOAT, 0, m_stem[id].elemOut[i + 1].vertices);
            glTexCoordPointer(2, GL_FLOAT, 0, m_stem[id].elemOut[i + 1].texcoords);
            glDrawElements(GL_TRIANGLES, m_stem[id].elemOut[i + 1].numIndices, GL_INDICES, (const GLvoid *)m_stem[id].elemOut[i + 1].indices);
         }
         if (m_stem[id].status[i] == MENUITEM_STATUS_DISABLED)
            glColor4f(MENU_COLOR_TEXT_DISABLED);
         else
            glColor4f(MENU_COLOR_TEXT);
         glVertexPointer(3, GL_FLOAT, 0, m_stem[id].elem[i + 1].vertices);
         glTexCoordPointer(2, GL_FLOAT, 0, m_stem[id].elem[i + 1].texcoords);
         glDrawElements(GL_TRIANGLES, m_stem[id].elem[i + 1].numIndices, GL_INDICES, (const GLvoid *)m_stem[id].elem[i + 1].indices);
         if (m_stem[id].subelem[i + 1].numIndices > 0) {
            if (m_stem[id].subelemOut[i + 1].numIndices > 0) {
               glColor4f(0.0f, 0.0f, 0.0f, 1.0f);
               glVertexPointer(3, GL_FLOAT, 0, m_stem[id].subelemOut[i + 1].vertices);
               glTexCoordPointer(2, GL_FLOAT, 0, m_stem[id].subelemOut[i + 1].texcoords);
               glDrawElements(GL_TRIANGLES, m_stem[id].subelemOut[i + 1].numIndices, GL_INDICES, (const GLvoid *)m_stem[id].subelemOut[i + 1].indices);
            }
            if (m_stem[id].status[i] == MENUITEM_STATUS_DISABLED)
               glColor4f(MENU_COLOR_TEXT_DISABLED);
            else
               glColor4f(MENU_COLOR_SUBTEXT);
            glVertexPointer(3, GL_FLOAT, 0, m_stem[id].subelem[i + 1].vertices);
            glTexCoordPointer(2, GL_FLOAT, 0, m_stem[id].subelem[i + 1].texcoords);
            glDrawElements(GL_TRIANGLES, m_stem[id].subelem[i + 1].numIndices, GL_INDICES, (const GLvoid *)m_stem[id].subelem[i + 1].indices);
         }
      } else if (m_stem[id].subelem[i + 1].numIndices > 0) {
         /* draw text */
         if (m_stem[id].image[i] == NULL) {
            glEnable(GL_TEXTURE_2D);
            glEnableClientState(GL_TEXTURE_COORD_ARRAY);
         }
         glBindTexture(GL_TEXTURE_2D, m_font->getTextureID());
         if (m_stem[id].subelemOut[i + 1].numIndices > 0) {
            glColor4f(0.0f, 0.0f, 0.0f, 1.0f);
            glVertexPointer(3, GL_FLOAT, 0, m_stem[id].subelemOut[i + 1].vertices);
            glTexCoordPointer(2, GL_FLOAT, 0, m_stem[id].subelemOut[i + 1].texcoords);
            glDrawElements(GL_TRIANGLES, m_stem[id].subelemOut[i + 1].numIndices, GL_INDICES, (const GLvoid *)m_stem[id].subelemOut[i + 1].indices);
         }
         if (m_stem[id].status[i] == MENUITEM_STATUS_DISABLED)
            glColor4f(MENU_COLOR_TEXT_DISABLED);
         else
            glColor4f(MENU_COLOR_SUBTEXT);
         glVertexPointer(3, GL_FLOAT, 0, m_stem[id].subelem[i + 1].vertices);
         glTexCoordPointer(2, GL_FLOAT, 0, m_stem[id].subelem[i + 1].texcoords);
         glDrawElements(GL_TRIANGLES, m_stem[id].subelem[i + 1].numIndices, GL_INDICES, (const GLvoid *)m_stem[id].subelem[i + 1].indices);
      }
      /* icon */
      if (hasIcon) {
         if (m_stem[id].image[i] == NULL && m_stem[id].elem[i + 1].numIndices == 0 && m_stem[id].subelem[i + 1].numIndices == 0) {
            glEnable(GL_TEXTURE_2D);
            glEnableClientState(GL_TEXTURE_COORD_ARRAY);
         }
         glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
         glVertexPointer(3, GL_FLOAT, 0, m_verticesIcon);
         glBindTexture(GL_TEXTURE_2D, m_icon->getID());
         for (j = 0; j < MENU_ICONNUM; j++) {
            if (m_stem[id].iconStatus[i][j]) {
               glPushMatrix();
               switch (j) {
               case MENU_ICON_DOWNLOAD:
                  glTranslatef(m_cWidth * MENU_ICON_X1, MENU_ICON_Y + MENU_ICON_WIDTH, 0.0f);
                  break;
               case MENU_ICON_HOME:
                  glTranslatef(m_cWidth * MENU_ICON_X2 - MENU_ICON_WIDTH * 2.0f, MENU_ICON_Y, 0.0f);
                  break;
               case MENU_ICON_LOCK:
                  glTranslatef(m_cWidth * MENU_ICON_X2 - MENU_ICON_WIDTH, MENU_ICON_Y, 0.0f);
                  break;
               case MENU_ICON_PLAY:
                  glTranslatef(m_cWidth * MENU_ICON_X1, MENU_ICON_Y, 0.0f);
                  break;
               case MENU_ICON_EXPORT:
                  glTranslatef(m_cWidth * MENU_ICON_X2 - MENU_ICON_WIDTH, MENU_ICON_Y + MENU_ICON_WIDTH, 0.0f);
                  break;
               case MENU_ICON_MIC:
                  glTranslatef(m_cWidth * MENU_ICON_X2 - MENU_ICON_WIDTH * 2.0f, MENU_ICON_Y + MENU_ICON_WIDTH, 0.0f);
                  break;
               }
               float ht = 1.0f / MENU_ICONNUM;
               float y = j * ht;
               iconTexcoords[1] = iconTexcoords[7] = y;
               iconTexcoords[3] = iconTexcoords[5] = y + ht;
               glTexCoordPointer(2, GL_FLOAT, 0, iconTexcoords);
               glDrawElements(GL_TRIANGLES, 6, GL_INDICES, (const GLvoid *)bgIndices);
               glPopMatrix();
            }
         }
      }
      if (m_stem[id].image[i] || m_stem[id].elem[i + 1].numIndices > 0 || m_stem[id].subelem[i + 1].numIndices > 0 || hasIcon) {
         glDisableClientState(GL_TEXTURE_COORD_ARRAY);
         glDisable(GL_TEXTURE_2D);
      }
      if (m_cursorShow == true && id == m_currentId && i == m_currentCursor[id]) {
         /* draw cursor */
         float current_y = (m_currentCursor[id] - m_currentTopItem[id]) * h;
         float prev_y = (m_prevCursor[id] - m_prevTopItem[id]) * h;
         float r = m_vscrollAnimationFrameLeft / MENU_DURATION_VSCROLL;
         float yoffset = (current_y - prev_y) * r;
         glPushMatrix();
         glTranslatef(0.0f, yoffset, 0.2f);
         glColor4f(MENU_COLOR_ITEM_CURSOR);
         glVertexPointer(3, GL_FLOAT, 0, m_verticesBox);
         glDrawElements(GL_TRIANGLES, 24, GL_INDICES, (const GLvoid *)cursorIndices);
         glPopMatrix();
      }
      if (m_execItemAnimationFrameLeft > 0.0f &&  id == m_execPos && i == m_execItemId)
         glPopMatrix();
      if (m_stem[id].popup[i]) {
         glPopMatrix();
         m_stem[id].popup[i]->renderEnd();
      }
   }

   /* restore position to the top */
   glPopMatrix();
   if (m_currentTopItem[id] > 0) {
      /* draw upper scroll indicator */
      glColor4f(MENU_COLOR_SCROLLINDICATOR);
      glVertexPointer(3, GL_FLOAT, 0, triVerticesUpper);
      glDrawElements(GL_TRIANGLES, 3, GL_INDICES, (const GLvoid *)bgIndices);
   }
   /* move down to the bottom */
   glTranslatef(0.0f, - h * MENUHEIGHT, 0.0f);
   if (m_currentTopItem[id] + MENUHEIGHT <= m_stem[id].maxValidItemId) {
      /* draw lower scroll indicator */
      glColor4f(MENU_COLOR_SCROLLINDICATOR);
      glVertexPointer(3, GL_FLOAT, 0, triVerticesLower);
      glDrawElements(GL_TRIANGLES, 3, GL_INDICES, (const GLvoid *)bgIndices);
   }
   /* draw bar */
   if (m_inhibitFlip == false) {
      glColor4f(MENU_COLOR_BAR);
      glVertexPointer(3, GL_FLOAT, 0, m_stem[id].vertices_bar);
      glDrawElements(GL_TRIANGLES, 6, GL_INDICES, (const GLvoid *)bgIndices);
   }

   if (scrolling) {
      /* disable drawing with stencil buffer */
      glDisable(GL_STENCIL_TEST);
   }
}

/* Menu::renderEnd: render end */
void Menu::renderEnd()
{
#ifdef MMDAGENT_DEPTHFUNC_DEFAULT_LESS
   glDepthFunc(GL_LESS);
#endif /* MMDAGENT_DEPTHFUNC_DEFAULT_LESS */
   glDisableClientState(GL_VERTEX_ARRAY);
   glPopMatrix();
   glMatrixMode(GL_PROJECTION);
   glPopMatrix();
   glMatrixMode(GL_MODELVIEW);
   glEnable(GL_LIGHTING);
   glEnable(GL_CULL_FACE);
}

/* Menu: constructor */
Menu::Menu()
{
   initialize();
}

/* Menu::~Menu: destructor */
Menu::~Menu()
{
   clear();
}

/* Menu::setup: initialize and setup menu */
void Menu::setup(MMDAgent *mmdagent, int id, FTGLTextureFont *font, const char *appDirName)
{
   clear();
   m_mmdagent = mmdagent;
   m_id = id;
   m_font = font;
   loadIcon(appDirName);
}

/* Menu::add: add a new stem and return its id */
int Menu::add(const char *name, int priority, void(*func)(int id, int row, void *data), void *data, const char *imageFile)
{
   int i;
   PMDTexture *tex;

   for (i = 0; i < MENUMAXNUM; i++) {
      if (m_stem[i].active == false) {
         /* found inactive stem, use it */
         /* make sure it's clean */
         clearStem(i);
         initializeStem(i);
         /* set initial values */
         m_stem[i].name = MMDAgent_strdup(name);
         m_stem[i].func = func;
         m_stem[i].data = data;
         m_stem[i].priority = priority;
         if (imageFile) {
            tex = new PMDTexture;
            if (tex->loadImage(imageFile))
               m_stem[i].bgImage = tex;
            else
               delete tex;
         }
         /* set this as active */
         m_stem[i].active = true;
         /* update stem max num */
         if (m_stemMaxNum < i + 1)
            m_stemMaxNum = i + 1;
         /* update count id */
         m_stem[i].countId = m_currentCountId++;
         /* mark as needs update, this stem and whole */
         m_stem[i].needsUpdate = true;
         m_needsUpdate = true;
         return i;
      }
   }

   return -1;
}

/* Menu::find: find stem by name, and return the id */
int Menu::find(const char *name)
{
   int i;

   for (i = 0; i < m_stemMaxNum; i++) {
      if (m_stem[i].active == false)
         continue;
      if (MMDAgent_strequal(m_stem[i].name, name))
         return i;
   }

   return -1;
}

/* Menu::setName: set name of the stem */
bool Menu::setName(int id, const char *name)
{
   if (id < 0 || id >= m_stemMaxNum || m_stem[id].active == false)
      return false;

   if (m_stem[id].name)
      free(m_stem[id].name);
   m_stem[id].name = MMDAgent_strdup(name);

   m_stem[id].needsUpdate = true;

   return true;
}

/* Menu::setItem: set an item to the stem */
bool Menu::setItem(int id, int row, const char *label, const char *imagefile, const char *messageType, const char *messageArg, const char *subText)
{
   PMDTexture *tex;
   int i;

   if (id < 0 || id >= m_stemMaxNum || m_stem[id].active == false)
      return false;

   if (row < 0 || row >= MENUMAXITEM)
      return false;

   if (m_stem[id].itemName[row])
      free(m_stem[id].itemName[row]);
   m_stem[id].itemName[row] = MMDAgent_strdup(label);
   if (m_stem[id].subtext[row])
      free(m_stem[id].subtext[row]);
   m_stem[id].subtext[row] = MMDAgent_strdup(subText);
   if (m_stem[id].messageType[row])
      free(m_stem[id].messageType[row]);
   m_stem[id].messageType[row] = MMDAgent_strdup(messageType);
   if (m_stem[id].messageArg[row])
      free(m_stem[id].messageArg[row]);
   m_stem[id].messageArg[row] = MMDAgent_strdup(messageArg);
   if (m_stem[id].status[row] == MENUITEM_STATUS_NONE)
      m_stem[id].status[row] = MENUITEM_STATUS_NORMAL;
   for (i = 0; i < MENU_ICONNUM; i++)
      m_stem[id].iconStatus[row][i] = false;
   if (m_stem[id].popup[row])
      delete m_stem[id].popup[row];
   m_stem[id].popup[row] = NULL;
   if (m_stem[id].image[row])
      delete m_stem[id].image[row];
   m_stem[id].image[row] = NULL;
   if (imagefile) {
      tex = new PMDTexture;
      if (tex->loadImage(imagefile))
         m_stem[id].image[row] = tex;
      else
         delete tex;
   }

   m_stem[id].needsUpdate = true;

   return true;
}

/* Menu::findItem: find item of the stem by label, and return the row */
int Menu::findItem(int id, const char *label)
{
   int i;

   if (id < 0 || id >= m_stemMaxNum || m_stem[id].active == false)
      return -1;

   for (i = 0; i < MENUMAXITEM; i++) {
      if (m_stem[id].itemName[i] != NULL && MMDAgent_strequal(m_stem[id].itemName[i], label))
         return i;
   }

   return -1;
}

/* Menu::setItemLabel: set item label in the stem */
bool Menu::setItemLabel(int id, int row, const char *label)
{
   if (id < 0 || id >= m_stemMaxNum || m_stem[id].active == false)
      return false;

   if (row < 0 || row >= MENUMAXITEM)
      return false;

   if (m_stem[id].status[row] != MENUITEM_STATUS_NONE) {
      if (m_stem[id].itemName[row])
         free(m_stem[id].itemName[row]);
      m_stem[id].itemName[row] = MMDAgent_strdup(label);
   }

   m_stem[id].needsUpdate = true;

   return true;
}

/* Menu::setItemSubLabel: set item sub label in the stem */
bool Menu::setItemSubLabel(int id, int row, const char *sublabel)
{
   if (id < 0 || id >= m_stemMaxNum || m_stem[id].active == false)
      return false;

   if (row < 0 || row >= MENUMAXITEM)
      return false;

   if (m_stem[id].status[row] != MENUITEM_STATUS_NONE) {
      if (m_stem[id].subtext[row])
         free(m_stem[id].subtext[row]);
      m_stem[id].subtext[row] = MMDAgent_strdup(sublabel);
   }

   m_stem[id].needsUpdate = true;

   return true;
}

/* Menu::setItemStatus: set item status of the stem */
bool Menu::setItemStatus(int id, int row, int status)
{
   if (id < 0 || id >= m_stemMaxNum || m_stem[id].active == false)
      return false;

   if (row < 0 || row >= MENUMAXITEM) {
      return false;
   }

   m_stem[id].status[row] = status;

   return true;
}

/* Menu::removeItem: remove item of the stem */
bool Menu::removeItem(int id, int row)
{
   if (id < 0 || id >= m_stemMaxNum || m_stem[id].active == false)
      return false;

   if (row < 0 || row >= MENUMAXITEM)
      return false;

   if (row == m_currentCursor[id])
      m_currentCursor[id] = m_prevCursor[id] = - 1;

   if (m_stem[id].itemName[row])
      free(m_stem[id].itemName[row]);
   m_stem[id].itemName[row] = NULL;
   if (m_stem[id].subtext[row])
      free(m_stem[id].subtext[row]);
   m_stem[id].subtext[row] = NULL;
   if (m_stem[id].messageType[row])
      free(m_stem[id].messageType[row]);
   m_stem[id].messageType[row] = NULL;
   if (m_stem[id].messageArg[row])
      free(m_stem[id].messageArg[row]);
   m_stem[id].messageArg[row] = NULL;
   m_stem[id].status[row] = MENUITEM_STATUS_NONE;
   if (m_stem[id].image[row])
      delete m_stem[id].image[row];
   m_stem[id].image[row] = NULL;
   if (m_stem[id].popup[row])
      delete m_stem[id].popup[row];
   m_stem[id].popup[row] = NULL;

   m_stem[id].needsUpdate = true;

   return true;
}

/* Menu::remove: remove the stem */
bool Menu::remove(int id)
{
   if (id < 0 || id >= MENUMAXNUM)
      return false;

   clearStem(id);
   initializeStem(id);
   m_currentCursor[id] = m_prevCursor[id] = -1;
   m_currentTopItem[id] = m_prevTopItem[id] = 0;

   m_stem[id].active = false;

   m_needsUpdate = true;

   return true;
}

/* Menu::setIconFlag: set icon flag */
bool Menu::setIconFlag(int id, int row, int iconId, bool flag)
{
   if (id < 0 || id >= m_stemMaxNum || m_stem[id].active == false)
      return false;

   if (row < 0 || row >= MENUMAXITEM)
      return false;

   if (iconId < 0 || iconId >= MENU_ICONNUM)
      return false;

   m_stem[id].iconStatus[row][iconId] = flag;

   return true;
}

/* Menu::setPopup: set popup */
bool Menu::setPopup(int id, int row, const char **choices, int num, void(*func)(int id, int row, int chosen, void *data), void *data)
{
   if (id < 0 || id >= m_stemMaxNum || m_stem[id].active == false)
      return false;

   if (row < 0 || row >= MENUMAXITEM)
      return false;

   if (m_stem[id].popup[row])
      delete m_stem[id].popup[row];
   m_stem[id].popup[row] = new Popup(choices, num, func, data, m_font, m_cWidth);

   return true;
}

/* Menu::setPopupFlag: set popup flag */
void Menu::setPopupFlag(int id, int row, bool flag)
{
   if (id < 0 || id >= m_stemMaxNum || m_stem[id].active == false)
      return;

   if (row < 0 || row >= MENUMAXITEM)
      return;

   if (m_stem[id].popup[row] == NULL)
      return;

   if (flag == true) {
      m_stem[id].popup[row]->forceActivate();
      m_poppingRow = row;
      m_popAnimatingRow = m_poppingRow;
      m_popAnimatingId = id;
   } else {
      m_stem[id].popup[row]->forceDeactivate();
      m_poppingRow = -1;
   }
}

/* Menu::setTitleColor: set title color */
void Menu::setTitleColor(int id, const float *cols)
{
   if (id < 0 || id >= m_stemMaxNum || m_stem[id].active == false)
      return;

   if (cols) {
      m_stem[id].hasCustomTitleColor = true;
      for (int i = 0; i < 4; i++)
         m_stem[id].customTitleColor[i] = cols[i];
   } else {
      m_stem[id].hasCustomTitleColor = false;
   }
}

/* Menu::togglePopupCurrent: toggle popup flag on current item */
void Menu::togglePopupCurrent()
{
   int id;

   id = m_currentId;

   if (m_currentCursor[id] == -1)
      return;

   if (m_stem[id].popup[m_currentCursor[id]] == NULL)
      return;

   if (m_stem[id].popup[m_currentCursor[id]]->isActive()) {
      m_stem[id].popup[m_currentCursor[id]]->deactivate();
      m_poppingRow = -1;
   } else {
      m_stem[id].popup[m_currentCursor[id]]->activate();
      m_poppingRow = m_currentCursor[id];
      m_popAnimatingRow = m_poppingRow;
      m_popAnimatingId = id;
   }
}

/* Menu::releasePopup: release current popup */
void Menu::releasePopup()
{
   int id;

   id = m_currentId;

   if (m_poppingRow == -1)
      return;

   if (m_stem[id].popup[m_poppingRow] == NULL)
      return;

   if (m_stem[id].popup[m_poppingRow]->isActive()) {
      m_stem[id].popup[m_poppingRow]->deactivate();
      m_poppingRow = -1;
   }
}

/* Menu::isPopping: return true when popup is shown */
bool Menu::isPopping()
{
   return (m_poppingRow != -1) ? true : false;
}

/* Menu::isShowing: return true when showing */
bool Menu::isShowing()
{
   return m_showing;
}

/* Menu::getPoppingRow: return current popping row */
int Menu::getPoppingRow()
{
   return m_poppingRow;
}

/* Menu::getSize: return menu size */
float Menu::getSize()
{
   return m_size;
}

/* Menu::setSize: set menu size */
void Menu::setSize(float size)
{
   m_size = size;
   if (m_size < MENU_SCALEMIN)
      m_size = MENU_SCALEMIN;
   if (m_size > MENU_SCALEMAX)
      m_size = MENU_SCALEMAX;
   updateMenuPosition();
   m_savedSize = m_size;
}

/* Menu::setSizeTillHide: set menu size till hide */
void Menu::setSizeTillHide(float size)
{
   m_size = size;
   updateMenuPosition();
}

/* Menu::setOrientation: set orientation */
void Menu::setOrientation(int orientation)
{
   int i;

   m_orientation = orientation;
   updateMenuPosition();
   for (i = 0; i < m_stemMaxNum; i++) {
      if (m_stem[i].active == false)
         continue;
      m_stem[i].needsUpdate = true;
   }
}

/* Menu::show: turn on this menu*/
void Menu::show()
{
   if (m_showing == false)
      m_showing = true;
}

/* Menu::hide: rurn off this menu */
void Menu::hide()
{
   if (m_showing == true)
      m_showing = false;
}

/* Menu::forward: move the menus forward */
void Menu::forward()
{
   if (m_inhibitFlip)
      return;

   if (m_poppingRow != -1) {
      if (m_stem[m_currentId].popup[m_poppingRow])
         m_stem[m_currentId].popup[m_poppingRow]->forward();
      else
         m_poppingRow = -1;
      return;
   }

   if (m_currentPos > m_orderNum - 1) {
      m_currentPos = m_orderNum - 1;
   } else if (m_currentPos == m_orderNum - 1) {
      m_forwardAnimationFrameLeft = 0.0f;
      m_backwardAnimationFrameLeft = 0.0f;
      m_jumpAnimationFrameLeft = 0.0f;
      m_forwardRegistAnimationFrameLeft = MENU_DURATION_REGIST;
      m_backwardRegistAnimationFrameLeft = 0.0f;
      m_isRegisting = true;
   } else {
      m_forwardAnimationFrameLeft = MENU_DURATION_FORWARD;
      m_backwardAnimationFrameLeft = 0.0f;
      m_jumpAnimationFrameLeft = 0.0f;
      m_forwardRegistAnimationFrameLeft = 0.0f;
      m_backwardRegistAnimationFrameLeft = 0.0f;
      m_currentPos++;
      m_isRegisting = false;
   }
   m_currentId = m_order[m_currentPos];
}

/* Menu::backward: move the menus backward */
void Menu::backward()
{
   if (m_inhibitFlip)
      return;

   if (m_poppingRow != -1) {
      if (m_stem[m_currentId].popup[m_poppingRow])
         m_stem[m_currentId].popup[m_poppingRow]->backward();
      else
         m_poppingRow = -1;
      return;
   }

   if (m_currentPos < 0) {
      m_currentPos = 0;
   } else if (m_currentPos == 0) {
      m_forwardAnimationFrameLeft = 0.0f;
      m_backwardAnimationFrameLeft = 0.0f;
      m_jumpAnimationFrameLeft = 0.0f;
      m_forwardRegistAnimationFrameLeft = 0.0f;
      m_backwardRegistAnimationFrameLeft = MENU_DURATION_REGIST;;
      m_isRegisting = true;
   } else {
      m_forwardAnimationFrameLeft = 0.0f;
      m_backwardAnimationFrameLeft = MENU_DURATION_BACKWARD;
      m_jumpAnimationFrameLeft = 0.0f;
      m_forwardRegistAnimationFrameLeft = 0.0f;
      m_backwardRegistAnimationFrameLeft = 0.0f;
      m_currentPos--;
      m_isRegisting = false;
   }
   m_currentId = m_order[m_currentPos];
}

/* Menu::jump: bring the specified stem to front */
void Menu::jump(int id)
{
   if (m_poppingRow != -1) {
      if (m_stem[m_currentId].popup[m_poppingRow])
         m_stem[m_currentId].popup[m_poppingRow]->deactivate();
      m_poppingRow = -1;
   }
   m_currentId = id;
}

/* Menu::jumpByPos: bring the specified stem to front */
void Menu::jumpByPos(int pos)
{
   if (m_poppingRow != -1) {
      if (m_stem[m_currentId].popup[m_poppingRow])
         m_stem[m_currentId].popup[m_poppingRow]->deactivate();
      m_poppingRow = -1;
   }
   m_currentPos = pos;
   if (m_currentPos < 0) {
      m_currentPos = 0;
   } else if (m_currentPos > m_orderNum - 1) {
      m_currentPos = m_orderNum - 1;
   }
   m_currentId = m_order[m_currentPos];
}

/* Menu::moveCursorUp: move cursor up */
void Menu::moveCursorUp()
{
   int id;
   int n;

   id = m_currentId;

   if (m_poppingRow != -1) {
      if (m_stem[id].popup[m_poppingRow])
         m_stem[id].popup[m_poppingRow]->deactivate();
      m_poppingRow = -1;
   }

   m_prevCursor[id] = m_currentCursor[id];
   n = m_currentCursor[id] - 1;
   /* move to the first valid items */
   while (n >= 0) {
      if (m_stem[id].status[n] != MENUITEM_STATUS_NONE) {
         m_currentCursor[id] = n;
         break;
      }
      n--;
   }

   m_prevTopItem[id] = m_currentTopItem[id];
   if (m_currentTopItem[id] > m_currentCursor[id])
      m_currentTopItem[id] = m_currentCursor[id];
   if (m_currentTopItem[id] < 0)
      m_currentTopItem[id] = 0;
   if (m_currentTopItem[id] > MENUMAXITEM - MENUHEIGHT)
      m_currentTopItem[id] = MENUMAXITEM - MENUHEIGHT;

   m_vscrollAnimationFrameLeft = MENU_DURATION_VSCROLL;
   m_cursorShow = true;
}

/* Menu::moveCursorDown: move cursor down */
void Menu::moveCursorDown()
{
   int id;
   int n;

   id = m_currentId;

   if (m_poppingRow != -1) {
      if (m_stem[id].popup[m_poppingRow])
         m_stem[id].popup[m_poppingRow]->deactivate();
      m_poppingRow = -1;
   }

   m_prevCursor[id] = m_currentCursor[id];
   n = m_currentCursor[id] + 1;
   /* move to the first valid items */
   while (n < MENUMAXITEM) {
      if (m_stem[id].status[n] != MENUITEM_STATUS_NONE) {
         m_currentCursor[id] = n;
         break;
      }
      n++;
   }

   m_prevTopItem[id] = m_currentTopItem[id];
   if (m_currentTopItem[id] < m_currentCursor[id] - MENUHEIGHT + 1)
      m_currentTopItem[id] = m_currentCursor[id] - MENUHEIGHT + 1;
   if (m_currentTopItem[id] < 0)
      m_currentTopItem[id] = 0;
   if (m_currentTopItem[id] > MENUMAXITEM - MENUHEIGHT)
      m_currentTopItem[id] = MENUMAXITEM - MENUHEIGHT;

   m_vscrollAnimationFrameLeft = MENU_DURATION_VSCROLL;
   m_cursorShow = true;
}

/* Menu::moveCursorAt: move cursor of a stem to the specified position */
bool Menu::moveCursorAt(int id, int row)
{
   if (id < 0 || id >= m_stemMaxNum || m_stem[id].active == false)
      return false;

   if (row < 0 || row >= MENUMAXITEM) {
      return false;
   }

   if (m_poppingRow != -1) {
      if (m_stem[m_currentId].popup[m_poppingRow])
         m_stem[m_currentId].popup[m_poppingRow]->deactivate();
      m_poppingRow = -1;
   }

   m_currentCursor[id] = row;

   if (m_currentTopItem[id] < m_currentCursor[id] - MENUHEIGHT + 1) {
      m_currentTopItem[id] = m_currentCursor[id] - MENUHEIGHT + 1;
      if (m_currentTopItem[id] < 0)
         m_currentTopItem[id] = 0;
      if (m_currentTopItem[id] > MENUMAXITEM - MENUHEIGHT)
         m_currentTopItem[id] = MENUMAXITEM - MENUHEIGHT;
   }

   m_prevCursor[id] = m_currentCursor[id];
   m_prevTopItem[id] = m_currentTopItem[id];

   m_cursorShow = true;

   return true;
}

/* Menu::scroll: scroll */
void Menu::scroll(int step)
{
   int id;

   id = m_currentId;

   if (m_poppingRow != -1) {
      if (m_stem[id].popup[m_poppingRow])
         m_stem[id].popup[m_poppingRow]->deactivate();
      m_poppingRow = -1;
   }

   m_prevTopItem[id] = m_currentTopItem[id];
   m_currentTopItem[id] += step;
   if (m_currentTopItem[id] > m_stem[id].maxValidItemId + 1 - MENUHEIGHT)
      m_currentTopItem[id] = m_stem[id].maxValidItemId + 1 - MENUHEIGHT;
   if (m_currentTopItem[id] < 0)
      m_currentTopItem[id] = 0;
   if (m_currentTopItem[id] > MENUMAXITEM - MENUHEIGHT)
      m_currentTopItem[id] = MENUMAXITEM - MENUHEIGHT;

   m_vscrollAnimationFrameLeft = MENU_DURATION_VSCROLL;
}

/* Menu::execItem: execute the item at the cursor */
void Menu::execCurrentItem()
{
   int id;

   id = m_currentId;

   if (m_currentCursor[id] == -1)
      return;

   if (m_poppingRow != -1) {
      if (m_stem[id].popup[m_poppingRow]) {
         m_stem[id].popup[m_poppingRow]->deactivate();
         m_stem[id].popup[m_poppingRow]->execCurrent(id, m_currentCursor[id]);
      }
      m_poppingRow = -1;
   } else {
      execItem(m_currentCursor[id]);
   }
}

/* Menu::execItem: execute the item of the stem */
void Menu::execItem(int choice)
{
   int id;

   id = m_currentId;

   if (choice < 0 || choice > MENUMAXITEM - 1)
      return;

   if (m_stem[id].status[choice] == MENUITEM_STATUS_NONE || m_stem[id].status[choice] == MENUITEM_STATUS_DISABLED)
      return;

   /* set animation status to show executed animation */
   m_execItemAnimationFrameLeft = MENU_DURATION_EXEC;
   m_execPos = id;
   m_execItemId = choice;

   if (m_stem[id].messageType[choice] && m_stem[id].messageType[choice][0] == '@') {
      if (m_mmdagent->getKeyValue()) {
         m_mmdagent->getKeyValue()->loadBuf(m_stem[id].messageType[choice]);
      }
   } else if (m_stem[id].messageType[choice] && m_stem[id].messageArg[choice]) {
      m_mmdagent->sendMessage(m_id, m_stem[id].messageType[choice], "%s", m_stem[id].messageArg[choice]);
   }

   if (m_stem[id].func)
      m_stem[id].func(id, choice, m_stem[id].data);
}

/* Menu::execByTap: execute the item of the stem at tapped point */
int Menu::execByTap(int x, int y, int screenWidth, int screenHeight)
{
   int id;
   float rx, ry;
   int n;
   int row;

   id = m_currentId;

   rx = x / (float)screenWidth;
   ry = 1.0f - y / (float)screenHeight;

   if (rx < m_rx1 || rx > m_rx2 || ry < m_ry1 || ry > m_ry2)
      return -1;

   n = (int)((MENUHEIGHT + 1) * (m_ry2 - ry) / (m_ry2 - m_ry1));
   if (n > MENUHEIGHT)
      n = MENUHEIGHT;

   if (n == 0) {
      /* tapped menu title */
   } else {
      /* tapped menu item */
      row = m_currentTopItem[id] + n - 1;
      if (m_poppingRow == row) {
      	/* tapped on popup menu */
         if (m_stem[id].popup[row]) {
            m_stem[id].popup[row]->execByPosition(id, row, (rx - m_rx1) / (m_rx2 - m_rx1));
            m_stem[id].popup[row]->deactivate();
         }
         m_poppingRow = -1;
      } else if (m_poppingRow != -1) {
	      /* tapped on other menu while popup is on */
         if (m_stem[id].popup[m_poppingRow])
            m_stem[id].popup[m_poppingRow]->deactivate();
	      m_poppingRow = -1;
      } else {
	      /* tapped on item */
	      execItem(row);
      }
   }

   return n - 1;
}

/* Menu::togglePopupByTap: toggle popup of the item of the stem at tapped point */
int Menu::togglePopupByTap(int x, int y, int screenWidth, int screenHeight)
{
   int id;
   float rx, ry;
   int n;
   int row;

   id = m_currentId;

   rx = x / (float)screenWidth;
   ry = 1.0f - y / (float)screenHeight;

   if (rx < m_rx1 || rx > m_rx2 || ry < m_ry1 || ry > m_ry2)
      return -1;

   n = (int)((MENUHEIGHT + 1) * (m_ry2 - ry) / (m_ry2 - m_ry1));
   if (n > MENUHEIGHT)
      n = MENUHEIGHT;

   if (n == 0) {
      /* tapped menu title */
   } else {
      /* tapped menu item */
      row = m_currentTopItem[id] + n - 1;

      if (m_stem[id].popup[row] == NULL)
         return -1;

      if (m_stem[id].popup[row]->isActive()) {
         m_stem[id].popup[row]->deactivate();
         m_poppingRow = -1;
      } else {
         m_stem[id].popup[row]->activate();
         m_poppingRow = row;
         m_popAnimatingRow = m_poppingRow;
	      m_popAnimatingId = id;
      }
   }

   return n - 1;
}

/* Menu::isPointed: return true when pointed */
bool Menu::isPointed(int x, int y, int screenWidth, int screenHeight)
{
   float rx, ry;

   rx = x / (float)screenWidth;
   ry = 1.0f - y / (float)screenHeight;

   if (rx < m_rx1 || rx > m_rx2 || ry < m_ry1 || ry > m_ry2)
      return false;

   return true;
}

/* Menu::forceForwardAnimationRate: force forward animation rate */
void Menu::forceForwardAnimationRate(float rate)
{
   if (m_inhibitFlip)
      return;
   if (rate < 0.0f) {
      /* release force mode */
      m_forwardFrameForced = false;
      return;
   }
   if (rate > 1.0f)
      rate = 1.0;
   if (m_isRegisting)
      m_forwardRegistAnimationFrameLeft = MENU_DURATION_REGIST * (1.0f - rate);
   else
      m_forwardAnimationFrameLeft = MENU_DURATION_FORWARD * (1.0f - rate);
   m_forwardFrameForced = true;
}

/* Menu::forceBackwardAnimationRate: force backward animation rate */
void Menu::forceBackwardAnimationRate(float rate)
{
   if (m_inhibitFlip)
      return;
   if (rate < 0.0f) {
      /* release force mode */
      m_backwardFrameForced = false;
      return;
   }
   if (rate > 1.0f)
      rate = 1.0;
   if (m_isRegisting)
      m_backwardRegistAnimationFrameLeft = MENU_DURATION_REGIST * (1.0f - rate);
   else
      m_backwardAnimationFrameLeft = MENU_DURATION_BACKWARD * (1.0f - rate);
   m_backwardFrameForced = true;
}

/* Menu::forceShowHideAnimationRate: force show/hide animation rate */
void Menu::forceShowHideAnimationRate(float rate)
{
   if (rate < 0.0f)
      rate = 0.0f;
   if (rate > 1.0f)
      rate = 1.0;
   m_showHideAnimationFrameLeft = MENU_DURATION_SHOWHIDE * (1.0f - rate);
}

/* Menu::update: if needs update, sort by priority and update menu */
void Menu::update(double ellapsedFrame)
{
   int i;

   if (m_needsUpdate) {
      /* update whole */
      sortStem();
      m_needsUpdate = false;
   }
   /* re-construct vertex array for stems with needsUpdate flags */
   for (i = 0; i < m_stemMaxNum; i++) {
      if (m_stem[i].active == false)
         continue;
      if (m_stem[i].needsUpdate == true) {
         updateStem(i);
         m_stem[i].needsUpdate = false;
      }
   }
   /* decrement animation durations */
   if (m_forceShowAnimationFlag == false) {
      if (m_showing) {
         m_showHideAnimationFrameLeft -= (float)ellapsedFrame;
         if (m_showHideAnimationFrameLeft < 0.0f)
            m_showHideAnimationFrameLeft = 0.0f;
      } else {
         m_showHideAnimationFrameLeft += (float)ellapsedFrame;
         if (m_showHideAnimationFrameLeft > MENU_DURATION_SHOWHIDE) {
            m_showHideAnimationFrameLeft = MENU_DURATION_SHOWHIDE;
            /* after disappered from screen, remove temporal stems */
            resetTemporalStatus();
         }
      }
   }
   if (m_forwardFrameForced == false) {
      m_forwardAnimationFrameLeft -= (float)ellapsedFrame;
      if (m_forwardAnimationFrameLeft < 0.0f)
         m_forwardAnimationFrameLeft = 0.0f;
      m_forwardRegistAnimationFrameLeft -= (float)ellapsedFrame;
      if (m_forwardRegistAnimationFrameLeft < 0.0f)
         m_forwardRegistAnimationFrameLeft = 0.0f;
   }
   if (m_backwardFrameForced == false) {
      m_backwardAnimationFrameLeft -= (float)ellapsedFrame;
      if (m_backwardAnimationFrameLeft < 0.0f)
         m_backwardAnimationFrameLeft = 0.0f;
      m_backwardRegistAnimationFrameLeft -= (float)ellapsedFrame;
      if (m_backwardRegistAnimationFrameLeft < 0.0f)
         m_backwardRegistAnimationFrameLeft = 0.0f;
   }
   m_execItemAnimationFrameLeft -= (float)ellapsedFrame;
   if (m_execItemAnimationFrameLeft < 0.0f)
      m_execItemAnimationFrameLeft = 0.0f;
   m_jumpAnimationFrameLeft -= (float)ellapsedFrame;
   if (m_jumpAnimationFrameLeft < 0.0f)
      m_jumpAnimationFrameLeft = 0.0f;
   m_vscrollAnimationFrameLeft -= (float)ellapsedFrame;
   if (m_vscrollAnimationFrameLeft < 0.0f)
      m_vscrollAnimationFrameLeft = 0.0f;

   /* update popup */
   if (m_popAnimatingRow != -1 && m_stem[m_popAnimatingId].popup[m_popAnimatingRow])
      m_stem[m_popAnimatingId].popup[m_popAnimatingRow]->update(ellapsedFrame);

   /* update padding */
   if (m_mmdagent && m_mmdagent->getTabbar()) {
      m_paddingY = m_mmdagent->getTabbar()->getBarHeight() * m_height;
   } else {
      m_paddingY = 0.0f;
   }
}

/* Menu::render: render the menu structure */
void Menu::render()
{
   int id;
   float r;

   if (m_font == NULL)
      return;

   if (m_orderNum == 0)
      return;

   if (m_showing == false && m_showHideAnimationFrameLeft >= MENU_DURATION_SHOWHIDE)
      return;

   if (m_currentPos < 0) {
      m_currentPos = 0;
   } else if (m_currentPos > m_orderNum - 1) {
      m_currentPos = m_orderNum - 1;
   }

   /* show menu according to animation frame lefts */
   id = m_currentId;

   glPushMatrix();

   renderBegin();
   if (m_showHideAnimationFrameLeft > 0.0f) {
      /* sliding animation at show/hide */
      r = m_showHideAnimationFrameLeft / MENU_DURATION_SHOWHIDE;
      glTranslatef(0.0f, - m_cHeight * r, 0.0f);
   }
   if (m_forwardAnimationFrameLeft > 0.0f) {
      /* moving forward rotation */
      r = m_forwardAnimationFrameLeft / MENU_DURATION_FORWARD;
      glPushMatrix();
      glRotatef(-90.0f + 90.0f * r, MENU_ROTATION_AXIS);
      renderStem(m_order[m_currentPos - 1]);
      glPopMatrix();
      glRotatef(90.0f * r, MENU_ROTATION_AXIS);
   }
   if (m_forwardRegistAnimationFrameLeft > 0.0f) {
      /* forward regist rotation, go and back */
      r = m_forwardRegistAnimationFrameLeft / MENU_DURATION_REGIST;
      r = r * (1.0f - r);
      glRotatef(-30.0f * r, MENU_ROTATION_AXIS);
   }
   if (m_backwardAnimationFrameLeft > 0.0f) {
      /* moving backward rotation */
      r = m_backwardAnimationFrameLeft / MENU_DURATION_BACKWARD;
      glPushMatrix();
      glRotatef(90.0f - 90.0f * r, MENU_ROTATION_AXIS);
      renderStem(m_order[m_currentPos + 1]);
      glPopMatrix();
      glRotatef(-90.0f * r, MENU_ROTATION_AXIS);
   }
   if (m_backwardRegistAnimationFrameLeft > 0.0f) {
      /* backward regist rotation, go and back */
      r = m_backwardRegistAnimationFrameLeft / MENU_DURATION_REGIST;
      r = r * (1.0f - r);
      glRotatef(30.0f * r, MENU_ROTATION_AXIS);
   }
   renderStem(id);
   renderEnd();

   glPopMatrix();
}

/* Menu::procMessage: process message */
bool Menu::processMessage(const char *type, const char *args)
{
   char buff[MMDAGENT_MAXBUFLEN];
   char *argv[6];
   int num = 0;
   char *str1, *str2;
   int id, n;

   for (n = 0; n < 6; n++)
      argv[n] = NULL;
   /* divide string into arguments */
   if (MMDAgent_strlen(args) > 0) {
      strncpy(buff, args, MMDAGENT_MAXBUFLEN - 1);
      buff[MMDAGENT_MAXBUFLEN - 1] = '\0';
      for (str1 = MMDAgent_strtok(buff, "|", &str2); str1; str1 = MMDAgent_strtok(NULL, "|", &str2)) {
         argv[num] = str1;
         num++;
         if (num >= 5) {
            argv[num] = MMDAgent_strtok(NULL, "\r\n", &str2);
            num++;
            break;
         }
      }
   }
   if (num < 1) {
      m_mmdagent->sendLogString(m_id, MLOG_ERROR, "Error: %s: no arguments", type);
      return false;
   }

   if (MMDAgent_strequal(argv[0], MENU_COMMAND_ADD)) {
      /* MENU|ADD|alias(|backgroundimagepath */
      if (num < 2) {
         m_mmdagent->sendLogString(m_id, MLOG_ERROR, "Error: %s|%s: too few arguments", type, argv[0]);
         return false;
      } else if (num > 3) {
         m_mmdagent->sendLogString(m_id, MLOG_ERROR, "Error: %s|%s: too many arguments", type, argv[0]);
         return false;
      }
      if (find(argv[1]) != -1) {
         m_mmdagent->sendLogString(m_id, MLOG_WARNING, "Warning: %s|%s: alias %s already exists", type, argv[0], argv[1]);
         return false;
      }
      if (add(argv[1], MENUPRIORITY_CONTENT, NULL, NULL, (num >= 3) ? argv[2] : NULL) == -1) {
         m_mmdagent->sendLogString(m_id, MLOG_ERROR, "Error: %s|%s: failed to add alias %s", type, argv[0], argv[1]);
         return false;
      }
      m_mmdagent->sendMessage(m_id, MENU_EVENT_TYPE, "%s|%s", MENU_COMMAND_ADD, argv[1]);
   } else if (MMDAgent_strequal(argv[0], MENU_COMMAND_DELETE)) {
      /* MENU|DELETE|alias */
      if (num != 2) {
         m_mmdagent->sendLogString(m_id, MLOG_ERROR, "Error: %s|%s: too few arguments", type, argv[0]);
         return false;
      }
      id = find(argv[1]);
      if (id == -1) {
         m_mmdagent->sendLogString(m_id, MLOG_WARNING, "Warning: %s|%s: alias %s not found", type, argv[0], argv[1]);
         return false;
      }
      if (remove(id) == false) {
         m_mmdagent->sendLogString(m_id, MLOG_ERROR, "Error: %s|%s: failed to remove alias %s", type, argv[0], argv[1]);
         return false;
      }
      m_mmdagent->sendMessage(m_id, MENU_EVENT_TYPE, "%s|%s", MENU_COMMAND_DELETE, argv[1]);
   } else if (MMDAgent_strequal(argv[0], MENU_COMMAND_SETITEM)) {
      /* MENU|SETITEM|alias|n|label|MESSAGE|.. */
      if (num < 5) {
         m_mmdagent->sendLogString(m_id, MLOG_ERROR, "Error: %s|%s: too few arguments", type, argv[0]);
         return false;
      }
      id = find(argv[1]);
      if (id == -1) {
         m_mmdagent->sendLogString(m_id, MLOG_ERROR, "Error: %s|%s: alias %s not found", type, argv[0], argv[1]);
         return false;
      }
      n = MMDAgent_str2int(argv[2]);
      if (setItem(id, n, argv[3], NULL, argv[4], argv[5]) == false) {
         m_mmdagent->sendLogString(m_id, MLOG_ERROR, "Error: %s|%s: failed to add item to alias %s", type, argv[0], argv[1]);
         return false;
      }
      m_mmdagent->sendMessage(m_id, MENU_EVENT_TYPE, "%s|%s|%s", MENU_COMMAND_SETITEM, argv[1], argv[2]);
   } else if (MMDAgent_strequal(argv[0], MENU_COMMAND_DELETEITEM)) {
      /* MENU|DELITEM|alias|n */
      if (num != 3) {
         m_mmdagent->sendLogString(m_id, MLOG_ERROR, "Error: %s|%s: too few arguments", type, argv[0]);
         return false;
      }
      id = find(argv[1]);
      if (id == -1) {
         m_mmdagent->sendLogString(m_id, MLOG_ERROR, "Error: %s|%s: alias %s not found", type, argv[0], argv[1]);
         return false;

      }
      n = MMDAgent_str2int(argv[2]);
      if (removeItem(id, n) == false) {
         m_mmdagent->sendLogString(m_id, MLOG_ERROR, "Error: %s|%s: failed to remove item from alias %s", type, argv[0], argv[1]);
         return false;
      }
      m_mmdagent->sendMessage(m_id, MENU_EVENT_TYPE, "%s|%s|%s", MENU_COMMAND_DELETEITEM, argv[1], argv[2]);
   }

   return true;
}


/* Menu::disableForwardBackwardTillHide: disable forward-backward till hide */
void Menu::disableForwardBackwardTillHide()
{
   m_inhibitFlip = true;
}

/* Menu::enableForwardBackward: enable forward-backward */
void Menu::enableForwardBackward()
{
   m_inhibitFlip = false;
   /* restore current Id to position */
   m_currentId = m_order[m_currentPos];
}

/* Menu::setSkipFlag: set skip flag */
void Menu::setSkipFlag(int id, bool flag)
{
   if (id < 0 || id >= m_stemMaxNum || m_stem[id].active == false)
      return;

   m_stem[id].skip = flag;

   m_needsUpdate = true;
}
