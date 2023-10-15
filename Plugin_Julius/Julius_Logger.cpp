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

/* definitions */
#define TEXT_MARGIN 0.1f
#define TEXT_LASTPAD 1.5f
#define CAPTION_LOCATION         1.0f, 2.0f, 0.2f
#define CAPTION_TEXTCOLOR_USER   1.0f, 1.0f, 0.5f, 1.0f
#define CAPTION_TEXTCOLOR_SYSTEM 0.5f, 1.0f, 1.0f, 1.0f
#define CAPTION_BACKGROUNDCOLOR  0.0f, 0.0f, 0.0f, 0.3f

/* headers */
#include "MMDAgent.h"
#include "julius/juliuslib.h"
#include "Julius_Logger.h"

/* callbackRecogBegin: callback for beginning of recognition */
static void callbackRecogBegin(Recog *recog, void *data)
{
   Julius_Logger *j = (Julius_Logger *) data;

   j->setRecognitionFlag(true);
}

/* callbackRecogEnd: callback for end of recognition */
static void callbackRecogEnd(Recog *recog, void *data)
{
   Julius_Logger *j = (Julius_Logger *) data;

   j->setRecognitionFlag(false);
   j->checkRecogProgress(recog, false);
}

/* callbackRcogAdin: callback for input audio segment */
static void callbackRecogAdin(Recog *recog, SP16 *buf, int len, void *data)
{
   Julius_Logger *j = (Julius_Logger *) data;

   j->updateMaxVol(buf, len);
   j->updateLevelThres(recog->jconf->detect.level_thres);
}

/* callbackReccogPass1Frame: callback for pass1 frame */
static void callbackRecogPass1Frame(Recog *recog, void *data)
{
   Julius_Logger *j = (Julius_Logger *)data;

   j->checkRecogProgress(recog, true);
}

/* Julius_Logger::initialize: initialize data */
void Julius_Logger::initialize()
{
   int i, j;

   m_mmdagent = NULL;
   m_id = 0;

   m_active = false;
   m_size = JULIUSLOGGER_SIZE;
   m_recognizing = false;
   m_hasResult = false;
   m_currentMaxAdIn = 0;
   m_maxAdIn = 0.0f;
   m_adInFrameStep = 0.0;
   m_levelThres = 0;
   m_recognizingFrame = 0.0;
   m_recogTransFrame = 0.0;
   m_lastMaxAdin = 0.0f;
   m_currentSize = 0.0f;
   m_width = 0.0f;
   m_height = 0.0f;
   m_levelSize = 0.0f;
   m_adinUpdated = false;
   m_hasCaption = false;
   for (i = 0; i < 2; i++) {
      m_ccFrame[i] = 0.0;
      for (j = 0; j < 2; j++)
         m_ccPos[i][j] = 0.0f;
      memset(&(m_ccElem[i]), 0, sizeof(FTGLTextDrawElements));
   }
   m_logtable = NULL;
}

/* Julius_Logger::clear: clear data */
void Julius_Logger::clear()
{
   int i;

   if (m_logtable != NULL)
      free(m_logtable);
   for (i = 0; i < 2; i++) {
      if (m_ccElem[i].vertices) free(m_ccElem[i].vertices);
      if (m_ccElem[i].texcoords) free(m_ccElem[i].texcoords);
      if (m_ccElem[i].indices) free(m_ccElem[i].indices);
   }
   initialize();
}

/* Julius_Logger::Julius_Logger: constructor */
Julius_Logger::Julius_Logger()
{
   initialize();
}

/* Julius_Logger::~Julius_Logger: destructor */
Julius_Logger::~Julius_Logger()
{
   clear();
}

/* Julius_Logger::mklogtable: make log table*/
void Julius_Logger::mklogtable()
{
   int i;
   int step;
   int len;

   if (m_logtable != NULL)
      free(m_logtable);
   step = 1 << JULIUSLOGGER_LOGTABLE_STEP_SHIFT;
   len = 32768 >> JULIUSLOGGER_LOGTABLE_STEP_SHIFT;
   m_logtable = (float *)malloc(sizeof(float) * len);
   for (i = 0; i < len; i++) {
      if (i * step <= JULIUSLOGGER_ADINUNDERFLOWTHRES)
         m_logtable[i] = m_levelMin;
      else if (i * step >= JULIUSLOGGER_ADINOVERFLOWTHRES)
         m_logtable[i] = m_levelMax;
      else
         m_logtable[i] = logf((float)(i * step));
   }
}

/* Julius_Logger::logval: return log value */
float Julius_Logger::logval(int v)
{
   if (m_logtable == NULL)
      return m_levelMin;
   return m_logtable[v >> JULIUSLOGGER_LOGTABLE_STEP_SHIFT];
}

/* Julius_Logger::updateVers: update vertices */
void Julius_Logger::updateVers()
{
   int j;

   m_triggerLevelVers[0] = m_triggerLevelVers[1] = m_triggerLevelVers[3] = m_triggerLevelVers[10] = -m_size;
   m_triggerLevelVers[4] = m_triggerLevelVers[6] = m_triggerLevelVers[7] = m_triggerLevelVers[9] = m_size;
   m_triggerLevelVers[2] = m_triggerLevelVers[5] = m_triggerLevelVers[8] = m_triggerLevelVers[11] = 0.0f;

   j = 0;
   m_circleVers[j++] = 0.0f;
   m_circleVers[j++] = 0.0f;
   m_circleVers[j++] = 0.0f;
   for (int i = 0; i < 30; i++) {
      float rad = MMDFILES_RAD(12.0f * (float)i);
      m_circleVers[j++] = cosf(rad) * m_size;
      m_circleVers[j++] = sinf(rad) * m_size;
      m_circleVers[j++] = 0.0f;
   }
   m_circleVers[j++] = m_circleVers[3];
   m_circleVers[j++] = m_circleVers[4];
   m_circleVers[j++] = m_circleVers[5];
}

/* Julius_Logger::setup: setup for logging */
void Julius_Logger::setup(MMDAgent *mmdagent, int id, Recog *recog)
{
   /* reset */
   clear();

   m_mmdagent = mmdagent;
   m_id = id;

   if (recog) {
      /* set callback */
      callback_add(recog, CALLBACK_EVENT_RECOGNITION_BEGIN, callbackRecogBegin, this);
      callback_add(recog, CALLBACK_EVENT_RECOGNITION_END, callbackRecogEnd, this);
      callback_add(recog, CALLBACK_RESULT_PASS1_INTERIM, callbackRecogPass1Frame, this);
      callback_add_adin(recog, CALLBACK_ADIN_CAPTURED, callbackRecogAdin, this);
      m_levelThres = recog->jconf->detect.level_thres;
   }

   m_levelMin = logf((float)JULIUSLOGGER_ADINUNDERFLOWTHRES);
   m_levelMax = logf((float)JULIUSLOGGER_ADINOVERFLOWTHRES);
   m_levelSize = (logf((float)m_levelThres) - m_levelMin) / (m_levelMax - m_levelMin);

   mklogtable();

   updateVers();
}

/* checkRecogProgress: check recognition progress */
void Julius_Logger::checkRecogProgress(Recog *recog, bool flag)
{
   int i;
   RecogProcess *process;
   char buf[MAX_HMMNAME_LEN];

   if (flag == false) {
      m_hasResult = false;
      return;
   }

   if (m_hasResult)
      return;

   for (process = recog->process_list; process; process = process->next) {
      if (!process->live)
         continue;
      if (!process->have_interim)
         continue;
      for (i = 0; i < process->result.pass1.word_num; i++) {
         if (MMDAgent_strlen(process->lm->winfo->woutput[process->result.pass1.word[i]]) > 0) {
            center_name(process->lm->winfo->wseq[process->result.pass1.word[i]][0]->name, buf);
            if (MMDAgent_strequal(buf, "sp") == false) {
               m_hasResult = true;
               return;
            }
         }
      }
   }
}

/* setRecognitionFlag: mark recognition start and end */
void Julius_Logger::setRecognitionFlag(bool flag)
{
   if (m_recognizing != flag)
      m_recogTransFrame = JULIUSLOGGER_TRIGGERFACETRANSFRAME;
   m_recognizing = flag;
}

/* updateMaxVol: update maximum volume */
void Julius_Logger::updateMaxVol(SP16 *buf, int len)
{
   int i;

   for (i = 0; i < len; i++) {
      if (m_currentMaxAdIn < abs(buf[i]))
         m_currentMaxAdIn = abs(buf[i]);
   }
   m_adinUpdated = true;
}

/* updateLevelThres: update level threshold */
void Julius_Logger::updateLevelThres(int thres)
{
   if (m_levelThres != thres) {
      m_levelThres = thres;
      m_levelSize = (logval(m_levelThres) - m_levelMin) / (m_levelMax - m_levelMin);
   }
}

/* setActiveFlag: set active flag */
void Julius_Logger::setActiveFlag(bool flag)
{
   m_active = flag;
}

/* getActiveFlag: get active flag */
bool Julius_Logger::getActiveFlag()
{
   return m_active;
}

/* setSize: set size */
void Julius_Logger::setSize(float size)
{
   m_size = size;
   updateVers();
}


/* setCaption: set caption */
void Julius_Logger::setCaption(const char *s, int id, float x, float y)
{
   int i;
   float x1, x2, y1, y2;
   float maxwidth;

   if (m_active == false)
      return;

   if (id < 0 || id > 2)
      return;

   if (m_ccElem[id].vertices) free(m_ccElem[id].vertices);
   if (m_ccElem[id].texcoords) free(m_ccElem[id].texcoords);
   if (m_ccElem[id].indices) free(m_ccElem[id].indices);
   memset(&(m_ccElem[id]), 0, sizeof(FTGLTextDrawElements));
   m_ccFrame[id] = 0.0f;

   if (s != NULL && MMDAgent_strlen(s) > 0) {
      if (m_mmdagent->getTextureFont()->getTextDrawElementsWithScale(s, &(m_ccElem[id]), 0, x, y, 0.1f, JULIUSLOGGER_CCFONTSCALE) == false) {
         m_ccElem[id].textLen = 0;
         m_ccElem[id].numIndices = 0;
         m_ccFrame[id] = 0.0f;
      } else {
         if (x >= 0.0f) {
            maxwidth = m_width - x - TEXT_LASTPAD;
            if (m_ccElem[id].width > maxwidth)
               m_mmdagent->getTextureFont()->getTextDrawElementsWithScale(s, &(m_ccElem[id]), 0, x, y, 0.1f, JULIUSLOGGER_CCFONTSCALE * maxwidth / m_ccElem[id].width);
            x1 = -TEXT_MARGIN + x;
            x2 = m_ccElem[id].width + TEXT_MARGIN + x;
         } else {
            maxwidth = m_width + x - TEXT_LASTPAD;
            if (m_ccElem[id].width > maxwidth) {
               x1 = TEXT_LASTPAD;
               m_mmdagent->getTextureFont()->getTextDrawElementsWithScale(s, &(m_ccElem[id]), 0, x1, y, 0.1f, JULIUSLOGGER_CCFONTSCALE * maxwidth / m_ccElem[id].width);
            } else {
               x1 = m_width + x - m_ccElem[id].width;
               m_mmdagent->getTextureFont()->getTextDrawElementsWithScale(s, &(m_ccElem[id]), 0, x1, y, 0.1f, JULIUSLOGGER_CCFONTSCALE);
            }
            x1 -= TEXT_MARGIN;
            x2 = x1 + m_ccElem[id].width + TEXT_MARGIN * 2.0f;
         }
         y1 = m_ccElem[id].upheight - m_ccElem[id].height - TEXT_MARGIN + y;
         y2 = m_ccElem[id].upheight + TEXT_MARGIN + y;
         m_ccVertices[id][0] = x1;
         m_ccVertices[id][1] = y1;
         m_ccVertices[id][2] = 0;
         m_ccVertices[id][3] = x2;
         m_ccVertices[id][4] = y1;
         m_ccVertices[id][5] = 0;
         m_ccVertices[id][6] = x2;
         m_ccVertices[id][7] = y2;
         m_ccVertices[id][8] = 0;
         m_ccVertices[id][9] = x1;
         m_ccVertices[id][10] = y2;
         m_ccVertices[id][11] = 0;
         m_ccFrame[id] = JULIUSLOGGER_CCDURATIONFRAME;
      }
   }

   for (i = 0; i < 2; i++) {
      if (m_ccFrame[i] > 0.0f)
         break;
   }
   m_hasCaption = (i < 2) ? true : false;
}

/* setCaptionDuration: set caption duration */
void Julius_Logger::setCaptionDuration(int id, double frame)
{
   int i;

   if (m_active == false)
      return;

   if (id < 0 || id > 2)
      return;

   m_ccFrame[id] = frame;

   for (i = 0; i < 2; i++) {
      if (m_ccFrame[i] > 0.0f)
         break;
   }
   m_hasCaption = (i < 2) ? true : false;
}

/* update: update log view per step */
void Julius_Logger::update(double frame)
{
   double d;
   int i;
   PMDObject *objs;
   PMDFace *face;

   if (m_active == false)
      return;

   /* update volume */
   m_adInFrameStep += frame;
   if (m_adInFrameStep >= JULIUSLOGGER_ADINMAXVOLUMEUPDATEFRAME) {
      if (m_adinUpdated == true) {
         m_adInFrameStep = 0.0;
         m_lastMaxAdin = m_maxAdIn;
         if (m_currentMaxAdIn >= JULIUSLOGGER_ADINOVERFLOWTHRES)
            m_maxAdIn = 1.0f;
         else if (m_currentMaxAdIn <= JULIUSLOGGER_ADINUNDERFLOWTHRES)
            m_maxAdIn = 0.0f;
         else
            m_maxAdIn = (logval(m_currentMaxAdIn) - m_levelMin) / (m_levelMax - m_levelMin);
         m_currentMaxAdIn = 0;
         m_adinUpdated = false;
      }
   }
   /* output message when overflow */
   if (m_maxAdIn == 1.0f)
      m_mmdagent->sendMessage(m_id, JULIUSLOGGER_EVENTOVERFLOW, "");


   /* set current indicator size */
   float r = (float)(m_adInFrameStep / JULIUSLOGGER_ADINMAXVOLUMEUPDATEFRAME);
   if (r > 1.0f) r = 1.0f;
   m_currentSize = (m_lastMaxAdin * (1.0f - r) + m_maxAdIn * r);

   /* store it to keyvalue storage */
   m_mmdagent->getKeyValue()->setString(JULIUSLOGGER_MAXVOLUMEKEYNAME, "%f", m_currentSize);

   /* update result animation frame */
   if (m_hasResult == true) {
      m_recognizingFrame += frame;
      if (m_recognizingFrame > JULIUSLOGGER_HASRESULTFRAME)
         m_recognizingFrame = JULIUSLOGGER_HASRESULTFRAME;
   } else {
      m_recognizingFrame -= frame;
      if (m_recognizingFrame < 0.0)
         m_recognizingFrame = 0.0;
   }

   /* update volume morph */
   objs = m_mmdagent->getModelList();
   for (i = 0; i < m_mmdagent->getNumModel(); i++) {
      if (objs[i].isEnable() == false) continue;
      face = objs[i].getPMDModel()->getFace(JULIUSLOGGER_VOLUMEFACENAME);
      if (face != NULL)
         face->setWeight(m_currentSize);
   }
   if (m_recogTransFrame > 0.0) {
      m_recogTransFrame -= frame;
      if (m_recogTransFrame < 0.0)
         m_recogTransFrame = 0.0;
   }
   d = m_recogTransFrame / JULIUSLOGGER_TRIGGERFACETRANSFRAME;
   if (m_recognizing == true)
      d = 1.0 - d;
   for (i = 0; i < m_mmdagent->getNumModel(); i++) {
      if (objs[i].isEnable() == false) continue;
      face = objs[i].getPMDModel()->getFace(JULIUSLOGGER_TRIGGERFACENAME);
      if (face != NULL)
         face->setWeight((float)d);
   }

   /* update caption */
   for (i = 0; i < 2; i++) {
      if (m_ccFrame[i] != 0.0) {
         m_ccFrame[i] -= frame;
         if (m_ccFrame[i] <= 0.0)
            m_ccFrame[i] = 0.0;
      }
   }
   for (i = 0; i < 2; i++) {
      if (m_ccFrame[i] > 0.0f)
         break;
   }
   m_hasCaption = (i < 2) ? true : false;
}

/* render: render log view */
void Julius_Logger::render2D(float width, float height)
{
   static GLindices indices[] = { 0, 1, 2, 0, 2, 3 };
   int i;
   float r;

   if (m_active == false)
      return;

   m_width = width;
   m_height = height;

   glPushMatrix();
   glTranslatef(1.2f, 1.2f + m_mmdagent->getTabbar()->getBarHeight() * m_height * m_mmdagent->getTabbar()->getCurrentShowRate(), 0.0f);

   glVertexPointer(3, GL_FLOAT, 0, m_circleVers);

   /* circle 1 (base) */
   if (m_recognizingFrame > 0.0f) {
      glPushMatrix();
      r = (float)(m_recognizingFrame / JULIUSLOGGER_HASRESULTFRAME);
      glScalef(r, r, 1.0);
      glColor4f(0.6f * r, 0.2f * r, 0.0f, 0.7f);
      glDrawArrays(GL_TRIANGLE_FAN, 0, 32);
      glPopMatrix();
   }
   glTranslatef(0.0f, 0.0f, 0.01f);

   /* circle 2 (scaling by volume) */
   glPushMatrix();
   glScalef(m_currentSize, m_currentSize, m_currentSize);
   if (m_maxAdIn == 1.0f)
      glColor4f(1.0f, 0.0f, 0.0f, 0.8f);
   else if (m_recognizing)
      glColor4f(1.0f, 0.6f, 0.0f, 0.8f);
   else
      glColor4f(0.0f, 0.3f, 0.7f, 0.8f);
   glDrawArrays(GL_TRIANGLE_FAN, 0, 32);
   glPopMatrix();

   /* square 4 (trigger level) */
   glPushMatrix();
   glScalef(m_levelSize, m_levelSize, m_levelSize);
   glColor4f(1.0f, 1.0f, 0.7f, 0.7f);
   glVertexPointer(3, GL_FLOAT, 0, m_triggerLevelVers);
   glDrawArrays(GL_LINES, 0, 4);
   glPopMatrix();

   glPopMatrix();
   if (m_hasCaption) {
      for (i = 0; i < 2; i++) {
         if (m_ccFrame[i] == 0.0f)
            continue;
         /* draw background */
         glVertexPointer(3, GL_FLOAT, 0, m_ccVertices[i]);
         glBindTexture(GL_TEXTURE_2D, 0);
         glColor4f(CAPTION_BACKGROUNDCOLOR);
         glDrawElements(GL_TRIANGLES, 6, GL_INDICES, (const GLvoid *)indices);
         /* draw text */
         glVertexPointer(3, GL_FLOAT, 0, m_ccElem[i].vertices);
         glEnable(GL_TEXTURE_2D);
         glBindTexture(GL_TEXTURE_2D, m_ccElem[i].textureId);
         glEnableClientState(GL_TEXTURE_COORD_ARRAY);
         glTexCoordPointer(2, GL_FLOAT, 0, m_ccElem[i].texcoords);
         switch (i) {
         case JULIUSLOGGER_CCUSER:
            glColor4f(CAPTION_TEXTCOLOR_USER);
            break;
         case JULIUSLOGGER_CCSYSTEM:
            glColor4f(CAPTION_TEXTCOLOR_SYSTEM);
            break;
         }
         glDrawElements(GL_TRIANGLES, m_ccElem[i].numIndices, GL_INDICES, (const GLvoid *)m_ccElem[i].indices);
         glDisableClientState(GL_TEXTURE_COORD_ARRAY);
         glDisable(GL_TEXTURE_2D);
      }
   }
}

