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

#include <stdarg.h>
#include "MMDAgent.h"

/* definitions */

#define LOGTEXT_COLOR          1.0f,0.7f,0.3f,0.7f /* default text color */
#define LOGTEXT_COLOR_ERROR    1.0f,0.0f,0.0f,0.8f /* text color for error text */
#define LOGTEXT_COLOR_WARNING  1.0f,0.6f,0.0f,0.8f /* text color for warning text */
#define LOGTEXT_COLOR_STATUS   0.0f,0.7f,0.4f,0.7f /* text color for status text*/
#define LOGTEXT_COLOR_SENT     1.0f,0.9f,0.0f,0.7f /* text color for sent text */
#define LOGTEXT_COLOR_CAPTURED 0.6f,0.4f,0.0f,0.7f /* text color for captured text */
#define LOGTEXT_COLOR_NARROW   0.7f,1.0f,0.2f,0.7f /* text color for narrowing */
#define LOGTEXT_BGCOLOR        0.0f,0.0f,0.0f,0.8f /* background color */
#define LOGTEXT_BGCOLOR_TYPING 0.3f,0.6f,0.1f,0.8f /* background color while typing narrowing */
#define LOGTEXT_BGCOLOR_UPDATE 0.7f,0.1f,0.0f      /* background color for update indicator */
#define LOGTEXT_MAXLINELEN     256
#define LOGTEXT_MAXNLINES      512
#define LOGTEXT_SCROLLBARWIDTH 0.4f                /* scroll bar width */

#define LOGTEXT_TYPINGDURATIONFRAME 90.0f
#define LOGTEXT_STATETRANSITIONFRAME 6.0f
#define LOGTEXT_TEXTTRANSITIONFRAME 30.0f

/* LogText::initialize: initialize logger */
void LogText::initialize()
{
   m_font = NULL;
   m_textWidth = 0;
   m_textHeight = 0;
   m_textX = 0.0;
   m_textY = 0.0;
   m_textZ = 0.0;
   m_textScale = 0.0;

   m_textList = NULL;
   m_flagList = NULL;
   m_transRateList = NULL;
   m_drawElements = NULL;
   m_elementErrorFlag = NULL;
   m_textIndex = 0;
   m_viewIndex = 0;

   m_typingFrame = 0.0;
   m_narrowString[0] = '\0';
   m_narrowStringHasCase = false;
   m_drawElementNarrow = NULL;
   m_noShowFlag = NULL;

   m_status = false;
   m_transRate = 0.0;

   m_heightBase = -1.0f;
   m_2d = false;
}

/* LogText::clear: free logger */
void LogText::clear()
{
   int i;

   if (m_textList) {
      for (i = 0; i < LOGTEXT_MAXNLINES; i++)
         free(m_textList[i]);
      free(m_textList);
   }
   if (m_flagList)
      free(m_flagList);
   if (m_transRateList)
      free(m_transRateList);
   if (m_drawElements) {
      for(i = 0; i < LOGTEXT_MAXNLINES; i++) {
         if(m_drawElements[i].vertices) free(m_drawElements[i].vertices);
         if(m_drawElements[i].texcoords) free(m_drawElements[i].texcoords);
         if(m_drawElements[i].indices) free(m_drawElements[i].indices);
      }
      free(m_drawElements);
   }
   if (m_elementErrorFlag)
      free(m_elementErrorFlag);
   if (m_drawElementNarrow) {
      if (m_drawElementNarrow->vertices) free(m_drawElementNarrow->vertices);
      if (m_drawElementNarrow->texcoords) free(m_drawElementNarrow->texcoords);
      if (m_drawElementNarrow->indices) free(m_drawElementNarrow->indices);
      free(m_drawElementNarrow);
   }
   if (m_noShowFlag)
      free(m_noShowFlag);

   initialize();
}

/* LogText::LogText: constructor */
LogText::LogText()
{
   initialize();
}

/* LogText::~LogText: destructor */
LogText::~LogText()
{
   clear();
}

/* LogText::setup: initialize and setup logger with args */
bool LogText::setup(FTGLTextureFont *font, const int *size, const float *position, float scale)
{
   int i;

   if (font == NULL || size == NULL || position == NULL) return false;
   if (size[0] <= 0 || size[1] <= 0 || scale <= 0.0) return false;

   clear();

   m_font = font;

   m_textWidth = size[0];
   m_textHeight = size[1];
   m_textX = position[0];
   m_textY = position[1];
   m_textZ = position[2];
   m_textScale = scale;

   m_textList = (char **) malloc(sizeof(char *) * LOGTEXT_MAXNLINES);
   for (i = 0; i < LOGTEXT_MAXNLINES; i++) {
      m_textList[i] = (char *)malloc(sizeof(char) * LOGTEXT_MAXLINELEN);
      strcpy(m_textList[i], "");
   }
   m_flagList = (unsigned int *)malloc(sizeof(unsigned int) * LOGTEXT_MAXNLINES);
   m_transRateList = (float *)malloc(sizeof(float) * LOGTEXT_MAXNLINES);

   m_drawElements = (FTGLTextDrawElements *) malloc(sizeof(FTGLTextDrawElements) * LOGTEXT_MAXNLINES);
   for(i = 0; i < LOGTEXT_MAXNLINES; i++)
      memset(&m_drawElements[i], 0, sizeof(FTGLTextDrawElements));

   m_elementErrorFlag = (bool *)malloc(sizeof(bool) * LOGTEXT_MAXNLINES);
   for (i = 0; i < LOGTEXT_MAXNLINES; i++)
      m_elementErrorFlag[i] = false;

   m_drawElementNarrow = (FTGLTextDrawElements *)malloc(sizeof(FTGLTextDrawElements));
   memset(m_drawElementNarrow, 0, sizeof(FTGLTextDrawElements));

   m_noShowFlag = (bool *)malloc(sizeof(bool) * LOGTEXT_MAXNLINES);
   for (i = 0; i < LOGTEXT_MAXNLINES; i++)
      m_noShowFlag[i] = false;

   return true;
}

/* LogText::set2dflag: set 2d flag */
void LogText::set2dflag(bool flag)
{
   m_2d = flag;
}

/* LogText::get2dflag: get 2d flag */
bool LogText::get2dflag()
{
   return m_2d;
}

/* LogText::log: store log text */
void LogText::log(unsigned int flag, const char *format, ...)
{
   char *p, *save;
   char buff[MMDAGENT_MAXBUFLEN];
   va_list args;

   if (m_textList == NULL) return;

   va_start(args, format);
   vsnprintf(buff, MMDAGENT_MAXBUFLEN, format, args);
   va_end(args);
   for (p = MMDAgent_strtok(buff, "\n", &save); p; p = MMDAgent_strtok(NULL, "\n", &save)) {
      int i, len;
      char *c;
      unsigned char size;

      len = MMDAgent_strlen(p);
      if (len > 0) {
         c = p;
         for (i = 0; i < len; i += size) {
            size = MMDAgent_getcharsize(c);
            if (size == 0) break;
            if (i + size >= LOGTEXT_MAXLINELEN) {
               *c = '\0';
               break;
            }
            c += size;
         }
      }
      strcpy(m_textList[m_textIndex], p);
      m_flagList[m_textIndex] = flag;
      m_transRateList[m_textIndex] = 0.0f;
      m_drawElements[m_textIndex].textLen = 0;
      m_drawElements[m_textIndex].numIndices = 0;
      m_elementErrorFlag[m_textIndex] = false;
      m_noShowFlag[m_textIndex] = matchNarrow(m_textList[m_textIndex]);
      if (m_viewIndex != 0 && m_noShowFlag[m_textIndex] == false)
         scroll(1);
      m_textIndex++;
      if (m_textIndex >= LOGTEXT_MAXNLINES)
         m_textIndex = 0;
   }
}

/* LogText::scroll: scroll text area */
void LogText::scroll(int shift)
{
   if(LOGTEXT_MAXNLINES <= m_textHeight)
      return;

   m_viewIndex += shift;

   if(m_viewIndex < 0)
      m_viewIndex = 0;
   else if(m_viewIndex >= LOGTEXT_MAXNLINES - m_textHeight)
      m_viewIndex = LOGTEXT_MAXNLINES - m_textHeight;
}

/* LogText::updateTypingActiveTime: update typing active time */
void LogText::updateTypingActiveTime(double frame)
{
   if (m_typingFrame != 0.0) {
      m_typingFrame -= frame;
      if (m_typingFrame < 0.0)
         m_typingFrame = 0.0;
   }
}

/* LogText::startTyping: start typing */
void LogText::startTyping()
{
   m_typingFrame = LOGTEXT_TYPINGDURATIONFRAME;
   if (m_narrowString[0] == '\0') {
      m_narrowString[0] = '?';
      m_narrowString[1] = '\0';
   }
}

/* LogText::isTypingActive: return true when typing is active */
bool LogText::isTypingActive()
{
   return (m_typingFrame > 0.0 ? true : false);
}

/* LogText::endTyping: end typing */
void LogText::endTyping()
{
   m_typingFrame = 0.0;
}


/* LogText::resetNarrowString: reset narrow string */
void LogText::resetNarrowString()
{
   m_narrowString[0] = '\0';
   updateNarrow();
}

/* LogText::addCharToNarrowString: add char to narrow string */
void LogText::addCharToNarrowString(char c)
{
   int len = MMDAgent_strlen(m_narrowString);
   if (len < MMDAGENT_MAXBUFLEN - 1) {
      m_narrowString[len] = c;
      m_narrowString[len + 1] = '\0';
   }
   m_typingFrame = LOGTEXT_TYPINGDURATIONFRAME;
   updateNarrow();
}

/* LogText::backwardCharToNarrowString: backward char to narrow string */
void LogText::backwardCharToNarrowString()
{
   int len = MMDAgent_strlen(m_narrowString);
   if (len > 0)
      m_narrowString[len - 1] = '\0';
   if (m_narrowString[0] == '\0')
      endTyping();
   else
      m_typingFrame = LOGTEXT_TYPINGDURATIONFRAME;
   updateNarrow();
}

/* LogText::matchNarrow: check if string matches narrowing string */
bool LogText::matchNarrow(const char *str)
{
   bool ret;
   if (m_narrowString[0] == '\0') {
      return false;
   }
   if (m_narrowStringHasCase)
      ret = MMDAgent_strstr(str, &(m_narrowString[1])) == NULL ? true : false;
   else
      ret = MMDAgent_stristr(str, &(m_narrowString[1])) == NULL ? true : false;
   return ret;
}

/* LogText::updateNarrow: update Narrow */
void LogText::updateNarrow()
{
   int i;

   if (m_narrowString[0] == '\0') {
      for (i = 0; i < LOGTEXT_MAXNLINES; i++)
         m_noShowFlag[i] = false;
      m_drawElementNarrow->textLen = 0;
      m_drawElementNarrow->numIndices = 0;
   } else {
      m_narrowStringHasCase = false;
      for (i = 1; i < MMDAgent_strlen(m_narrowString); i++) {
         if (isupper(m_narrowString[i])) {
            m_narrowStringHasCase = true;
            break;
         }
      }
      for (i = 0; i < LOGTEXT_MAXNLINES; i++)
         m_noShowFlag[i] = matchNarrow(m_textList[i]);
      if (m_font->getTextDrawElements(m_narrowString, m_drawElementNarrow, 0, 0.0f, 0.0f, 0.0f) == false) {
         m_drawElementNarrow->textLen = 0; /* reset */
         m_drawElementNarrow->numIndices = 0;
      }
   }
}

/* LogText::renderMain: rendering main function */
void LogText::renderMain(float w, float h, float fullScale, float textScale, float x, float y, float z)
{
   int i, j, k, start, size;
   float rate;
   GLfloat vertices[12];

   glPushMatrix();
   glScalef(fullScale,fullScale, fullScale);
   glNormal3f(0.0f, 1.0f, 0.0f);

   /* effect according to transition rate */
   glTranslatef(0.0f, m_textHeight * (1.0f - m_transRate * m_transRate) * 0.5f, 0.0f);
   glScalef(1.0f, m_transRate * m_transRate, 1.0f);

   /* background */
   glColor4f(LOGTEXT_BGCOLOR);
   vertices[0] = x;
   vertices[1] = y;
   vertices[2] = z;
   vertices[3] = x + w;
   vertices[4] = y;
   vertices[5] = z;
   vertices[6] = x;
   vertices[7] = y + h;
   vertices[8] = z;
   vertices[9] = x + w;
   vertices[10] = y + h;
   vertices[11] = z;
   glVertexPointer(3, GL_FLOAT, 0, vertices);
   glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

   /* scroll bar */
   if(m_textHeight < LOGTEXT_MAXNLINES) {
      if (m_narrowString[0] != '\0')
         glColor4f(LOGTEXT_COLOR_NARROW);
      else
         glColor4f(LOGTEXT_COLOR);
      vertices[0] = x + w;
      vertices[1] = y;
      vertices[2] = z + 0.05f;
      vertices[3] = x + w + LOGTEXT_SCROLLBARWIDTH;
      vertices[4] = y;
      vertices[5] = z + 0.05f;
      vertices[6] = x + w + LOGTEXT_SCROLLBARWIDTH;
      vertices[7] = y + h;
      vertices[8] = z + 0.05f;
      vertices[9] = x + w;
      vertices[10] = y + h;
      vertices[11] = z + 0.05f;
      glVertexPointer(3, GL_FLOAT, 0, vertices);
      glDrawArrays(GL_LINE_LOOP, 0, 4);
      rate = (float)m_viewIndex / LOGTEXT_MAXNLINES;
      vertices[0] = x + w;
      vertices[1] = y + h * rate;
      vertices[2] = z + 0.05f;
      vertices[3] = x + w + LOGTEXT_SCROLLBARWIDTH;
      vertices[4] = y + h * rate;
      vertices[5] = z + 0.05f;
      rate = (float)(m_viewIndex + m_textHeight) / LOGTEXT_MAXNLINES;
      vertices[6] = x + w;
      vertices[7] = y + h * rate;
      vertices[8] = z + 0.05f;
      vertices[9] = x + w + LOGTEXT_SCROLLBARWIDTH;
      vertices[10] = y + h * rate;
      vertices[11] = z + 0.05f;
      glVertexPointer(3, GL_FLOAT, 0, vertices);
      glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
   }

   if (m_narrowString[0] != '\0' || isTypingActive()) {
      /* narrowing text bg */
      if (isTypingActive())
         glColor4f(LOGTEXT_BGCOLOR_TYPING);
      else
         glColor4f(LOGTEXT_BGCOLOR);
      vertices[0] = x;
      vertices[1] = y - 2.3f;
      vertices[2] = z + 0.04f;
      vertices[3] = x + w;
      vertices[4] = y - 2.3f;
      vertices[5] = z + 0.04f;
      vertices[6] = x;
      vertices[7] = y - 0.1f;
      vertices[8] = z + 0.04f;
      vertices[9] = x + w;
      vertices[10] = y - 0.1f;
      vertices[11] = z + 0.04f;
      glVertexPointer(3, GL_FLOAT, 0, vertices);
      glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
   }

   /* text */
   glEnable(GL_TEXTURE_2D);
   glActiveTexture(GL_TEXTURE0);
   glClientActiveTexture(GL_TEXTURE0);
   glBindTexture(GL_TEXTURE_2D, m_font->getTextureID());
   glEnableClientState(GL_TEXTURE_COORD_ARRAY);

   glTranslatef(x + 0.5f, y - 0.2f, z + 0.05f);
   glScalef(textScale, textScale, textScale);

   if (m_narrowString[0] != '\0' && m_drawElementNarrow && m_drawElementNarrow->numIndices > 0) {
      glPushMatrix();
      glTranslatef((w - m_drawElementNarrow->width) * 0.5f - 1.0f, -1.6f, 0.0f);
      glColor4f(LOGTEXT_COLOR_NARROW);
      glScalef(2.0f, 2.0f, 2.0f);
      glVertexPointer(3, GL_FLOAT, 0, m_drawElementNarrow->vertices);
      glTexCoordPointer(2, GL_FLOAT, 0, m_drawElementNarrow->texcoords);
      glDrawElements(GL_TRIANGLES, m_drawElementNarrow->numIndices, GL_INDICES, (const GLvoid *)m_drawElementNarrow->indices);
      glPopMatrix();
   }

   size = LOGTEXT_MAXNLINES < m_textHeight ? LOGTEXT_MAXNLINES : m_textHeight;
   i = 0;
   start = m_textIndex - 1;
   if (start < 0)
      start += LOGTEXT_MAXNLINES;
   j = start;
   k = m_viewIndex;
   while (i < size) {
      if (m_noShowFlag[j] == false && k-- <= 0) {
         glTranslatef(0.0f, 0.85f, 0.0f);
         if (m_drawElements[j].numIndices == 0 && m_elementErrorFlag[j] == false) {
            if (m_font->getTextDrawElements(m_textList[j], &(m_drawElements[j]), 0, 0.0f, 0.0f, 0.0f) == false) {
               m_drawElements[j].textLen = 0; /* reset */
               m_drawElements[j].numIndices = 0;
               m_elementErrorFlag[j] = true;
            }
         }
         if (m_drawElements[j].numIndices > 0) {
            glPushMatrix();
            if (m_transRateList[j] < 1.0f) {
               // hightlighting background for new logs
               glDisable(GL_TEXTURE_2D);
               glDisableClientState(GL_TEXTURE_COORD_ARRAY);
               glColor4f(LOGTEXT_BGCOLOR_UPDATE, 1.0f - m_transRateList[j]);
               vertices[0] = 0.0f;
               vertices[1] = -0.2f;
               vertices[2] = -0.03f;
               vertices[3] = w;
               vertices[4] = -0.2f;
               vertices[5] = -0.03f;
               vertices[6] = 0.0f;
               vertices[7] = 0.65f;
               vertices[8] = -0.03f;
               vertices[9] = w;
               vertices[10] = 0.65f;
               vertices[11] = -0.03f;
               glVertexPointer(3, GL_FLOAT, 0, vertices);
               glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
               glEnable(GL_TEXTURE_2D);
               glEnableClientState(GL_TEXTURE_COORD_ARRAY);
            }
            switch (m_flagList[j]) {
            case MLOG_ERROR:
               glColor4f(LOGTEXT_COLOR_ERROR); break;
            case MLOG_WARNING:
               glColor4f(LOGTEXT_COLOR_WARNING); break;
            case MLOG_STATUS:
               glColor4f(LOGTEXT_COLOR_STATUS); break;
            case MLOG_MESSAGE_SENT:
               glColor4f(LOGTEXT_COLOR_SENT); break;
            case MLOG_MESSAGE_CAPTURED:
               glColor4f(LOGTEXT_COLOR_CAPTURED); break;
            default:
               glColor4f(LOGTEXT_COLOR); break;
            }
            glScalef(0.9f, 0.9f, 0.9f);
            glVertexPointer(3, GL_FLOAT, 0, m_drawElements[j].vertices);
            glTexCoordPointer(2, GL_FLOAT, 0, m_drawElements[j].texcoords);
            glDrawElements(GL_TRIANGLES, m_drawElements[j].numIndices, GL_INDICES, (const GLvoid *)m_drawElements[j].indices);
            glPopMatrix();
         }
         i++;
      }
      j--;
      if (j < 0)
         j += LOGTEXT_MAXNLINES;
      if (j == start)
         break;
   }
   glDisableClientState(GL_TEXTURE_COORD_ARRAY);
   glDisable(GL_TEXTURE_2D);
   glPopMatrix();
}

/* LogText::render: render log text */
void LogText::render()
{
   float w, h;

   if (m_textList == NULL) return;
   if (m_status == false && m_transRate == 0.0f) return;
   if (m_2d) return;

   w = 0.5f * ((float)m_textWidth) * 0.85f + 1.0f;
   h = 1.0f * ((float)m_textHeight) * 0.85f + 1.0f;

   glDisable(GL_CULL_FACE);
   glDisable(GL_LIGHTING);
   glEnableClientState(GL_VERTEX_ARRAY);
   renderMain(w, h, m_textScale, 1.0f, m_textX, m_textY, m_textZ);
   glDisableClientState(GL_VERTEX_ARRAY);
   glEnable(GL_LIGHTING);
   glEnable(GL_CULL_FACE);
}

/* LogText::render2d: render text area for 2d screen */
void LogText::render2d(float screenWidth, float screenHeight, int idx)
{
   if (m_textList == NULL) return;
   if (m_2d == false) return;

   float textscale = 0.6f;
   float height = m_textHeight * 0.85f * textscale;
   m_heightBase = screenHeight - (height + 0.6f) * (idx + 1);

   renderMain(screenWidth - LOGTEXT_SCROLLBARWIDTH, height + 0.5f, 1.0f, textscale, 0.0f, m_heightBase, 0.0f);
}

/* LogText::setStatus: set open / close status */
void LogText::setStatus(bool sw)
{
   m_status = sw;
}

/* LogText::updateStatus: update open / close transition status */
void LogText::updateStatus(double ellapsedFrame)
{
   int i;
   bool enabled = false;

   if (m_status || m_2d)
      enabled = true;

   if (enabled && m_transRate < 1.0f) {
      m_transRate += (float)ellapsedFrame / LOGTEXT_STATETRANSITIONFRAME;
      if (m_transRate > 1.0f) {
         m_transRate = 1.0f;
      }
   } else if (!enabled && m_transRate > 0.0f) {
      m_transRate -= (float)ellapsedFrame / LOGTEXT_STATETRANSITIONFRAME;
      if (m_transRate < 0.0f) {
         m_transRate = 0.0f;
      }
   }

   for (i = 0; i < LOGTEXT_MAXNLINES; i++) {
      if (m_transRateList[i] < 1.0f) {
         m_transRateList[i] += (float)ellapsedFrame / LOGTEXT_TEXTTRANSITIONFRAME;
         if (m_transRateList[i] > 1.0f)
            m_transRateList[i] = 1.0f;
      }
   }
}

/* LogText::getHeightBase: get height base */
float LogText::getHeightBase()
{
   return m_heightBase;
}
