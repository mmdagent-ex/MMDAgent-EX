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
#define PROMPT_DEFAULT_LINES     21  /* default number of lines */
#define PROMPT_MIN_WIDTH          7  /* minumum width */
#define PROMPT_MAX_WIDTH_RATE  0.9f  /* maximum width as ratio of screen */
#define PROMPT_LINESPACE       0.2f  /* line spacing */
#define PROMPT_PADDING         0.3f  /* space around text element */
#define PROMPT_MARGIN          0.3f  /* common margin between text, labels and around */
#define PROMPT_EXTRAMARGIN     0.5f  /* extra Y margin between text and label area */

/* animation parameters */
#define PROMPT_DURATION_SHOWHIDE  15.0f  /* duration length in frame for show/hide animation */
#define PROMPT_DURATION_SHOWSHOW  10.0f  /* duration length in frame for show - show animation */
#define PROMPT_DURATION_EXEC       7.0f  /* duratino length in frame for label execute animation */
#define PROMPT_FACTOR_SHOWHIDE    10.0f  /* degree factor of show/hide pop animation */
#define PROMPT_FACTOR_EXEC         2.0f  /* degree factor of exec pop animation */

/* colors */
#define PROMPT_COLOR_BACKGROUND       0.3f, 0.2f, 0.6f, 0.8f  /* whole background color */
#define PROMPT_COLOR_TEXT             1.0f, 1.0f, 1.0f, 1.0f  /* text color */
#define PROMPT_COLOR_LABELBACKGROUND  1.0f, 1.0f, 1.0f, 0.8f  /* label background color */
#define PROMPT_COLOR_LABELTEXT        0.0f, 0.0f, 0.0f, 1.0f  /* label text color */
#define PROMPT_COLOR_CURSOR           1.0f, 0.8f, 0.3f, 0.8f  /* cursor background color */
#define PROMPT_COLOR_EXEC             1.0f, 0.8f, 0.3f, 0.8f  /* exec background color */

#define RENDERING_Z_OFFSET -0.4f

/* Prompt::initialize: initialize prompt */
void Prompt::initialize()
{
   m_mmdagent = NULL;
   m_id = 0;
   m_font = NULL;

   initializeElem();
   m_textScale = 1.0f;
   m_textOffsetX = 0.0f;
   m_textOffsetY = 0.0f;
   m_labelNum = 0;

   m_currentCursor = -1;
   m_showing = false;

   m_lines = PROMPT_DEFAULT_LINES;
   m_viewWidth = 0;
   m_viewHeight = 0;

   m_showHideAnimationFrameLeft = PROMPT_DURATION_SHOWHIDE;
   m_execLabelAnimationFrameLeft = 0.0f;
   m_execLabelId = -1;
}

/* Prompt::clear: free prompt */
void Prompt::clear()
{
   clearElem();
   initialize();
}

/* Prompt::initializeElem: initialize text elements */
void Prompt::initializeElem()
{
   int i;

   memset(&m_elem_text, 0, sizeof(FTGLTextDrawElements));
   for (i = 0; i < PROMPT_LABEL_MAXNUM; i++) {
      memset(&(m_elem_label[i]), 0, sizeof(FTGLTextDrawElements));
      m_labelScale[i] = 1.0f;
      m_labelOffsetX[i] = 0.0f;
      m_labelOffsetY[i] = 0.0f;
      m_labelTransX[i] = 0.0f;
      m_labelTransY[i] = 0.0f;
   }
}

/* Prompt::clearElem: clear text elements */
void Prompt::clearElem()
{
   int i;

   if (m_elem_text.vertices) free(m_elem_text.vertices);
   if (m_elem_text.texcoords) free(m_elem_text.texcoords);
   if (m_elem_text.indices) free(m_elem_text.indices);
   for (i = 0; i < PROMPT_LABEL_MAXNUM; i++) {
      if (m_elem_label[i].vertices) free(m_elem_label[i].vertices);
      if (m_elem_label[i].texcoords) free(m_elem_label[i].texcoords);
      if (m_elem_label[i].indices) free(m_elem_label[i].indices);
   }
}

/* Prompt::setVertices: set vertices */
void Prompt::setVertices(int id, float x1, float y1, float x2, float y2, float z)
{
   /* first 12 values are for background drawing */
   m_vertices[id][0] = x1;
   m_vertices[id][1] = y2;
   m_vertices[id][2] = z;
   m_vertices[id][3] = x1;
   m_vertices[id][4] = y1;
   m_vertices[id][5] = z;
   m_vertices[id][6] = x2;
   m_vertices[id][7] = y1;
   m_vertices[id][8] = z;
   m_vertices[id][9] = x2;
   m_vertices[id][10] = y2;
   m_vertices[id][11] = z;
   /* next 3 values are center position, used for exec animation translation */
   m_vertices[id][12] = (x1 + x2) * 0.5f;
   m_vertices[id][13] = (y1 + y2) * 0.5f;
}

/* Prompt::updatePosition: update positions */
void Prompt::updatePosition()
{
   float screenWidth, screenHeight;
   float y, y2;
   float xmargin;
   int i;

   /* set screen-based parameters */
   m_scale = m_viewHeight / m_lines;
   m_maxWidth = (float)m_viewWidth * PROMPT_MAX_WIDTH_RATE / m_scale;
   screenWidth = (float)m_viewWidth / m_scale;
   screenHeight = (float)m_viewHeight / m_scale;
   xmargin = PROMPT_MARGIN * 2.0f + PROMPT_PADDING * 2.0f;

   /* obtain whole width */
   m_width = PROMPT_MIN_WIDTH;
   if (m_width < m_elem_text.width)
      m_width = m_elem_text.width;
   for (i = 0; i < m_labelNum; i++) {
      if (m_width < m_elem_label[i].width)
         m_width = m_elem_label[i].width;
   }
   m_width += xmargin;
   if (m_width > m_maxWidth)
      m_width = m_maxWidth;

   /* set scale factors and x offsets for centering */
   if (m_elem_text.width > m_width - xmargin) {
      m_textScale = (m_width - xmargin) / m_elem_text.width;
      m_textOffsetX = xmargin * 0.5f;
   } else {
      m_textScale = 1.0f;
      m_textOffsetX = (m_width - m_elem_text.width) * 0.5f;
   }
   for (i = 0; i < m_labelNum; i++) {
      if (m_elem_label[i].width > m_width - xmargin) {
         m_labelScale[i] = (m_width - xmargin) / m_elem_label[i].width;
         m_labelOffsetX[i] = xmargin * 0.5f;
      } else {
         m_labelScale[i] = 1.0f;
         m_labelOffsetX[i] = (m_width - m_elem_label[i].width) * 0.5f;
      }
   }

   /* obtain whole height */
   m_height = m_elem_text.height + (PROMPT_MARGIN + PROMPT_PADDING) * 2.0f + PROMPT_EXTRAMARGIN;
   for (i = 0; i < m_labelNum; i++)
      m_height += m_elem_label[i].height + PROMPT_MARGIN + PROMPT_PADDING * 2.0f;

   /* set position coordinates from obtained width, height and screen parameters */
   m_posX = (screenWidth - m_width) * 0.5f;
   m_rx1 = m_posX / screenWidth;
   m_rx2 = 1.0f - m_rx1;
   m_posY = (screenHeight - m_height) * 0.5f;
   m_ry1 = m_posY / screenHeight;
   m_ry2 = 1.0f - m_ry1;

   /* set y offsets for each element */
   y = m_height - PROMPT_MARGIN - PROMPT_PADDING;
   m_textOffsetY = y - m_elem_text.upheight;
   y -= m_elem_text.height + PROMPT_PADDING + PROMPT_EXTRAMARGIN;
   /* preserve top-of-selection ratio for exec-by-tap function */
   m_rsy = m_ry1 + (m_ry2 - m_ry1) * y / m_height;
   for (i = 0; i < m_labelNum; i++) {
      m_labelOffsetY[i] = y - PROMPT_MARGIN - PROMPT_PADDING - m_elem_label[i].upheight;
      y -= m_elem_label[i].height + PROMPT_MARGIN + PROMPT_PADDING * 2.0f;
   }

   /* update vertices for background plate rendering */
   setVertices(0, 0.0f, 0.0f, m_width, m_height, -0.2f);
   for (i = 0; i < m_labelNum; i++) {
      y = m_labelOffsetY[i] + m_elem_label[i].upheight + PROMPT_PADDING;
      y2 = y - m_elem_label[i].height - PROMPT_PADDING * 2.0f;
      setVertices(i + 1, PROMPT_MARGIN, y2, m_width - PROMPT_MARGIN, y, -0.1f);
   }

   /* update transition for each label for exec animation */
   for (i = 0; i < m_labelNum; i++) {
      m_labelTransX[i] = m_elem_label[i].width * 0.5f;
      m_labelTransY[i] = m_elem_label[i].upheight - m_elem_label[i].height * 0.5f;
   }
}

/* Prompt::compose: compose prompt */
bool Prompt::compose(const char *text, char **labels, int num)
{
   size_t i, size, inLen;
   int j;
   bool mode;
   char *str;

   if (m_font == NULL)
      return false;

   if (num > PROMPT_LABEL_MAXNUM)
      return false;

   /* clear text elements */
   clearElem();
   initializeElem();

   /* convert "\n" as newline in the given text */
   str = MMDAgent_strdup(text);
   inLen = MMDAgent_strlen(str);
   mode = false;
   for (i = 0; i < inLen; i += size) {
      size = MMDFiles_getcharsize(&(str[i]));
      if (size == 1 && str[i] == '\\') {
         mode = true;
      } else if (mode == true && size == 1 && str[i] == 'n') {
         str[i - 1] = '\r';
         str[i] = '\n';
         mode = false;
      }
   }

   /* build text element for text */
   m_elem_text.textLen = 0;
   m_elem_text.numIndices = 0;
   if (m_font->getTextDrawElements(str, &m_elem_text, m_elem_text.textLen, 0.0f, 0.0f, 0.1f) == false) {
      m_elem_text.textLen = 0; /* reset */
      m_elem_text.numIndices = 0;
      free(str);
      return false;
   }
   free(str);

   /* build text elements for labels */
   for (j = 0; j < num; j++) {
      m_elem_label[j].textLen = 0;
      m_elem_label[j].numIndices = 0;
      if (m_font->getTextDrawElements(labels[j], &(m_elem_label[j]), m_elem_label[j].textLen, 0.0f, 0.0f, 0.1f) == false) {
         m_elem_label[j].textLen = 0; /* reset */
         m_elem_label[j].numIndices = 0;
         return false;
      }
   }

   m_labelNum = num;

   /* update position */
   updatePosition();

   /* reset cursor position */
   m_currentCursor = -1;

   /* set minimal animation */
   if (m_showHideAnimationFrameLeft < PROMPT_DURATION_SHOWSHOW)
      m_showHideAnimationFrameLeft = PROMPT_DURATION_SHOWSHOW;

   /* start showing */
   m_showing = true;

   return true;
}

/* Prompt: constructor */
Prompt::Prompt()
{
   initialize();
}

/* Prompt::~Prompt: destructor */
Prompt::~Prompt()
{
   clear();
}

/* Prompt::setup: initialize and setup Prompt */
void Prompt::setup(MMDAgent *mmdagent, int id, FTGLTextureFont *font)
{
   clear();
   m_mmdagent = mmdagent;
   m_id = id;
   m_font = font;
}

/* Prompt::isShowing: return true when showing */
bool Prompt::isShowing()
{
   return m_showing;
}

/* Prompt::cancel: cancel this Prompt */
void Prompt::cancel()
{
   if (m_showing == true) {
      /* start closing */
      m_showing = false;
      /* output message with selection number -1 */
      m_mmdagent->sendMessage(m_id, PROMPT_EVENT_SELECTED, "%d", -1);
   }
}


/* Prompt::moveCursorUp: move cursor up */
void Prompt::moveCursorUp()
{
   if (m_showing == false)
      return;

   m_currentCursor--;
   if (m_currentCursor < 0)
      m_currentCursor = m_labelNum - 1;
}

/* Prompt::moveCursorDown: move cursor down */
void Prompt::moveCursorDown()
{
   if (m_showing == false)
      return;

   m_currentCursor++;
   if (m_currentCursor > m_labelNum - 1)
      m_currentCursor = 0;
}

/* Prompt::execItem: execute the item at the cursor */
void Prompt::execCursorItem()
{
   if (m_currentCursor == -1)
      return;

   execItem(m_currentCursor);
}

/* Prompt::execItem: execute the specified item  */
void Prompt::execItem(int choice)
{
   if (m_showing == false)
      return;

   if (choice < 0 || choice > m_labelNum - 1)
      return;

   m_execLabelAnimationFrameLeft = PROMPT_DURATION_EXEC;
   m_execLabelId = choice;
}

/* Prompt::execByTap: execute the item at tapped point */
int Prompt::execByTap(int x, int y, int screenWidth, int screenHeight)
{
   float rx, ry;
   int n;

   rx = x / (float)screenWidth;
   ry = 1.0f - y / (float)screenHeight;

   if (rx < m_rx1 || rx > m_rx2 || ry < m_ry1 || ry > m_ry2)
      return -1;

   n = (int)(m_labelNum * (m_rsy - ry) / (m_rsy - m_ry1));
   if (n > m_labelNum - 1)
      return -1;

   execItem(n);

   return n;
}

/* Prompt::isPointed: return true when pointed */
bool Prompt::isPointed(int x, int y, int screenWidth, int screenHeight)
{
   float rx, ry;

   rx = x / (float)screenWidth;
   ry = 1.0f - y / (float)screenHeight;

   if (rx < m_rx1 || rx > m_rx2 || ry < m_ry1 || ry > m_ry2)
      return false;

   return true;
}

/* Prompt::update: if needs update, sort by priority and update menu */
void Prompt::update(double ellapsedFrame)
{
   if (m_execLabelAnimationFrameLeft > 0.0f) {
      m_execLabelAnimationFrameLeft -= (float)ellapsedFrame;
      if (m_execLabelAnimationFrameLeft <= 0.0f) {
         m_execLabelAnimationFrameLeft = 0.0f;
         /* when exec animation was expired, send message and start hiding this prompt */
         if (m_execLabelId != -1) {
            m_mmdagent->sendMessage(m_id, PROMPT_EVENT_SELECTED, "%d", m_execLabelId);
            m_execLabelId = -1;
         }
         if (m_showing == true)
            m_showing = false;
      }
   } else {
      if (m_showing) {
         m_showHideAnimationFrameLeft -= (float)ellapsedFrame;
         if (m_showHideAnimationFrameLeft < 0.0f)
            m_showHideAnimationFrameLeft = 0.0f;
      } else {
         m_showHideAnimationFrameLeft += (float)ellapsedFrame;
         if (m_showHideAnimationFrameLeft > PROMPT_DURATION_SHOWHIDE)
            m_showHideAnimationFrameLeft = PROMPT_DURATION_SHOWHIDE;
      }
   }
}

/* Prompt::render: render the menu structure */
void Prompt::render()
{
   static GLindices indices[6] = { 0, 1, 2, 0, 2, 3 };
   int i;
   float f;
   float alphaCoef;
   float execFactor;

   if (m_font == NULL)
      return;

   if (m_labelNum == 0)
      return;

   if (m_showing == false && m_showHideAnimationFrameLeft >= PROMPT_DURATION_SHOWHIDE)
      return;

   /* check if screen property has changed */
   int w, h;
   m_mmdagent->getWindowSize(&w, &h);
   if (m_viewWidth != w || m_viewHeight != h) {
      m_viewWidth = w;
      m_viewHeight = h;
      /* rebuild positions and rendering coordinates */
      updatePosition();
   }

   if (m_width <= 0.0f || m_height <= 0.0f)
      return;

   glDisable(GL_CULL_FACE);
   glDisable(GL_LIGHTING);
   glMatrixMode(GL_PROJECTION);
   glPushMatrix();
   glLoadIdentity();
   MMDAgent_setOrtho(0, (float)m_viewWidth / m_scale, 0, m_viewHeight / m_scale, -1.0f, 1.0f);
   glMatrixMode(GL_MODELVIEW);
   glPushMatrix();
   glLoadIdentity();
#ifdef MMDAGENT_DEPTHFUNC_DEFAULT_LESS
   glDepthFunc(GL_LEQUAL);
#endif /* MMDAGENT_DEPTHFUNC_DEFAULT_LESS */
   glEnableClientState(GL_VERTEX_ARRAY);
   glTranslatef(m_posX, m_posY, RENDERING_Z_OFFSET);

   f = 0.0f;
   if (m_showHideAnimationFrameLeft > 0.0f) {
      /* pop up/down animation in progress */
      f = m_showHideAnimationFrameLeft / PROMPT_DURATION_SHOWHIDE;
      glTranslatef(0.0f, f * (0.6f - f) * PROMPT_FACTOR_SHOWHIDE, 0.0f);
   }
   /* set alpha coef for pop/hide animation */
   alphaCoef = 1.0f - f;

   /* set exec factor */
   execFactor = 0.0f;
   if (m_execLabelAnimationFrameLeft > 0.0f) {
      f = m_execLabelAnimationFrameLeft / PROMPT_DURATION_EXEC;
      execFactor = f * (1.0f - f) * PROMPT_FACTOR_EXEC;
   }

   /* draw background */
   glVertexPointer(3, GL_FLOAT, 0, m_vertices[0]);
   glBindTexture(GL_TEXTURE_2D, 0);
   glColor4f(PROMPT_COLOR_BACKGROUND * alphaCoef);
   glDrawElements(GL_TRIANGLES, 6, GL_INDICES, (const GLvoid *)indices);
   for (i = 0; i < m_labelNum; i++) {
      if (m_execLabelId == i && m_execLabelAnimationFrameLeft > 0.0f) {
         glPushMatrix();
         glVertexPointer(3, GL_FLOAT, 0, &(m_vertices[i + 1][0]));
         glTranslatef(m_vertices[i + 1][12], m_vertices[i + 1][13], 0.0f);
         glScalef(1.0f + execFactor, 1.0f + execFactor, 1.0f);
         glTranslatef(-m_vertices[i + 1][12], -m_vertices[i + 1][13], 0.0f);
         glColor4f(PROMPT_COLOR_EXEC * alphaCoef);
         glDrawElements(GL_TRIANGLES, 6, GL_INDICES, (const GLvoid *)indices);
         glPopMatrix();
      } else {
         if (m_currentCursor == i)
            glColor4f(PROMPT_COLOR_CURSOR * alphaCoef);
         else
            glColor4f(PROMPT_COLOR_LABELBACKGROUND * alphaCoef);
         glVertexPointer(3, GL_FLOAT, 0, &(m_vertices[i + 1][0]));
         glDrawElements(GL_TRIANGLES, 6, GL_INDICES, (const GLvoid *)indices);
      }
   }
   /* draw text */
   glEnable(GL_TEXTURE_2D);
   glEnableClientState(GL_TEXTURE_COORD_ARRAY);
   glBindTexture(GL_TEXTURE_2D, m_font->getTextureID());
   glColor4f(PROMPT_COLOR_TEXT * alphaCoef);
   glPushMatrix();
   glTranslatef(m_textOffsetX, m_textOffsetY, 0.0f);
   if (m_textScale != 1.0f)
      glScalef(m_textScale, m_textScale, 0.0f);
   glVertexPointer(3, GL_FLOAT, 0, m_elem_text.vertices);
   glTexCoordPointer(2, GL_FLOAT, 0, m_elem_text.texcoords);
   glDrawElements(GL_TRIANGLES, m_elem_text.numIndices, GL_INDICES, (const GLvoid *)m_elem_text.indices);
   glPopMatrix();
   for (i = 0; i < m_labelNum; i++) {
      glPushMatrix();
      glColor4f(PROMPT_COLOR_LABELTEXT * alphaCoef);
      if (m_execLabelId == i && m_execLabelAnimationFrameLeft > 0.0f) {
         glTranslatef(m_labelOffsetX[i] + m_labelTransX[i] * m_labelScale[i], m_labelOffsetY[i] + m_labelTransY[i] * m_labelScale[i], 0.0f);
         glScalef(m_labelScale[i] + execFactor, m_labelScale[i] + execFactor, 1.0f);
         glTranslatef(-m_labelTransX[i], -m_labelTransY[i], 0.0f);
      } else if (m_labelScale[i] != 1.0f) {
         glTranslatef(m_labelOffsetX[i], m_labelOffsetY[i], 0.0f);
         glScalef(m_labelScale[i], m_labelScale[i], 1.0f);
      } else {
         glTranslatef(m_labelOffsetX[i], m_labelOffsetY[i], 0.0f);
      }
      glVertexPointer(3, GL_FLOAT, 0, m_elem_label[i].vertices);
      glTexCoordPointer(2, GL_FLOAT, 0, m_elem_label[i].texcoords);
      glDrawElements(GL_TRIANGLES, m_elem_label[i].numIndices, GL_INDICES, (const GLvoid *)m_elem_label[i].indices);
      glPopMatrix();
   }
   glDisableClientState(GL_TEXTURE_COORD_ARRAY);

   /* ending part */
#ifdef MMDAGENT_DEPTHFUNC_DEFAULT_LESS
   glDepthFunc(GL_LESS);
#endif /* MMDAGENT_DEPTHFUNC_DEFAULT_LESS */
   glDisableClientState(GL_VERTEX_ARRAY);
   glDisable(GL_TEXTURE_2D);
   glPopMatrix();
   glMatrixMode(GL_PROJECTION);
   glPopMatrix();
   glMatrixMode(GL_MODELVIEW);
   glEnable(GL_LIGHTING);
   glEnable(GL_CULL_FACE);
}

/* Prompt::procMessage: process message */
bool Prompt::processMessage(const char *type, const char *args)
{
   char buff[MMDAGENT_MAXBUFLEN];
   char *argv[MMDAGENT_MAXNCOMMAND];
   int num = 0;
   char *str1, *str2;

   /* divide string into arguments */
   if (MMDAgent_strlen(args) > 0) {
      strncpy(buff, args, MMDAGENT_MAXBUFLEN - 1);
      buff[MMDAGENT_MAXBUFLEN - 1] = '\0';
      for (str1 = MMDAgent_strtok(buff, "|", &str2); str1; str1 = MMDAgent_strtok(NULL, "|", &str2)) {
         if (num >= MMDAGENT_MAXNCOMMAND) {
            m_mmdagent->sendLogString(m_id, MLOG_ERROR, "Error: %s: number of arguments exceed the limit.", type);
            break;
         }
         argv[num] = str1;
         num++;
      }
   }

   if (MMDAgent_strequal(type, PROMPT_COMMAND_SHOW)) {
      if (num < 2) {
         m_mmdagent->sendLogString(m_id, MLOG_ERROR, "Error: %s: too few arguments", type);
         return false;
      }
      /* compose a new prompt and start showing */
      if (compose(argv[0], &(argv[1]), num - 1) == false)
         m_mmdagent->sendLogString(m_id, MLOG_ERROR, "Error: %s: failed to compose prompt", type);
   }

   return true;
}

