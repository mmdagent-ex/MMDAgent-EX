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
#include "VIManager.h"
#include "VIManager_Logger.h"
#include "VIManager_Thread.h"

/* VIManager_Logger::initialize: initialize logger */
void VIManager_Logger::initialize()
{
   m_mmdagent = NULL;
   memset(&m_elem, 0, sizeof(FTGLTextDrawElements));
}

/* VIManager_Logger::clear: free logger */
void VIManager_Logger::clear()
{
   if(m_elem.vertices)
      free(m_elem.vertices);
   if(m_elem.texcoords)
      free(m_elem.texcoords);
   if(m_elem.indices)
      free(m_elem.indices);

   initialize();
}

/* VIManager_Logger::VIManager_Logger: constructor */
VIManager_Logger::VIManager_Logger()
{
   initialize();
}

/* VIManager_Logger::~VIManager_Logger: destructor */
VIManager_Logger::~VIManager_Logger()
{
   clear();
}

/* VIManager_Logger::setup: setup logger */
void VIManager_Logger::setup(MMDAgent *mmdagent)
{
   m_mmdagent = mmdagent;
}

static void dispNum(char *buff, VIManager_State *s1, VIManager_State *s2)
{
   if (s1 == NULL)
      if (s2->virtual_fromState)
         sprintf(buff, "(initial state) [-> %s]", s2->virtual_toState->label);
      else
         sprintf(buff, "(initial state) %s", s2->label);
   else if (s1->virtual_fromState && s2->virtual_fromState)
      sprintf(buff, "[%s -> %s]", s1->virtual_fromState->label, s2->virtual_toState->label);
   else if (s1->virtual_fromState)
      sprintf(buff, "[%s ->] %s", s1->virtual_fromState->label, s2->label);
   else if (s2->virtual_fromState)
      sprintf(buff, "%s [-> %s]", s1->label, s2->virtual_toState->label);
   else
      sprintf(buff, "%s %s", s1->label, s2->label);
}

/* VIManager_Logger::addArcToElement: add arc to element */
void VIManager_Logger::addArcToElement(VIManager_Arc *arc, float x, float y)
{
   int i, j;
   char buf1[MMDAGENT_MAXBUFLEN];
   char buf2[MMDAGENT_MAXBUFLEN];
   char buf3[MMDAGENT_MAXBUFLEN];

   strcpy(buf1, arc->input_event_type);
   for(i = 0; i < arc->input_event_args.size; i++) {
      strcat(buf1, "|");
      for(j = 0; j < arc->input_event_args.argc[i]; j++) {
         if(j != 0)
            strcat(buf1, ",");
         strcat(buf1, arc->input_event_args.args[i][j]);
      }
   }

   if (arc->label)
      MMDAgent_snprintf(buf3, MMDAGENT_MAXBUFLEN, "[%s %03u]", arc->label, arc->line_number);
   else
      MMDAgent_snprintf(buf3, MMDAGENT_MAXBUFLEN, "[%03u]", arc->line_number);
   if (MMDAgent_strlen(arc->output_command_args) > 0)
      MMDAgent_snprintf(buf2, MMDAGENT_MAXBUFLEN, "%s  %s %s|%s", buf3, buf1, arc->output_command_type, arc->output_command_args);
   else
      MMDAgent_snprintf(buf2, MMDAGENT_MAXBUFLEN, "%s  %s %s", buf3, buf1, arc->output_command_type);
   if (arc->variable_action) {
      strcat(buf2, " ");
      strcat(buf2, arc->variable_action);
   }

   if (m_mmdagent->getTextureFont())
      m_mmdagent->getTextureFont()->getTextDrawElements(buf2, &m_elem, m_elem.textLen, x, y, 0.0f);
}

/* VIManager_Logger::addVariableToElement: draw variable string */
void VIManager_Logger::addVariableToElement(VIManager_Variable *v, float x, float y)
{
   char buf[MMDAGENT_MAXBUFLEN];

   sprintf(buf, "$%s=%s", v->name, v->value);
   if (m_mmdagent->getTextureFont())
      m_mmdagent->getTextureFont()->getTextDrawElements(buf, &m_elem, m_elem.textLen, x, y, 0.0f);
}

/* VIManager_Logger::addRenderElement: add render element */
void VIManager_Logger::addRenderElement(VIManager *vim, float base, float lineHeight, int numItems)
{
#ifndef VIMANAGER_DONTRENDERDEBUG
   char buff[MMDAGENT_MAXBUFLEN];
   char buff2[MMDAGENT_MAXBUFLEN];
   char buf1[MMDAGENT_MAXBUFLEN];

   if (vim == NULL)
      return;

   VIManager_Arc *arcs[VIMANAGER_HISTORY_LEN];
   int len = vim->getTransitionHistory(arcs, VIMANAGER_HISTORY_LEN);
   
   for (int i = 1; i < numItems - 1; i++) {
      int idx = len - i;
      if (idx < 0)
         break;
      addArcToElement(arcs[idx], 0.0f, base + lineHeight * i);
   }

   if (vim->getFileName()) {
      MMDAgent_snprintf(buff, MMDAGENT_MAXBUFLEN, "---- %s: %s ----", vim->getName(), vim->getFileName());
      if (m_mmdagent->getTextureFont())
         m_mmdagent->getTextureFont()->getTextDrawElements(buff, &m_elem, m_elem.textLen, 0.0f, base + lineHeight * (numItems - 1), 0.0f);
   }
   if (len > 0) {
      VIManager_Arc *arc = vim->getCurrentState()->arc_list.head;
      if (arc == NULL) {
         strcpy(buff2, " (no arc)");
      } else {
         bool first = true;
         strcpy(buff2, ", waiting");
         while (arc) {
            strcpy(buf1, arc->input_event_type);
            for (int i = 0; i < arc->input_event_args.size; i++) {
               strcat(buf1, "|");
               for (int j = 0; j < arc->input_event_args.argc[i]; j++) {
                  if (j != 0)
                     strcat(buf1, ",");
                  strcat(buf1, arc->input_event_args.args[i][j]);
               }
            }
            if (MMDAgent_strlen(buff2) + MMDAgent_strlen(buf1) + 2 >= MMDAGENT_MAXBUFLEN)
               break;
            if (first == true) {
               strcat(buff2, " ");
               first = false;
            } else {
               strcat(buff2, " or ");
            }
            strcat(buff2, buf1);
            if (MMDAgent_strlen(buff2) > 80) {
               strcat(buff2, " etc...");
               break;
            }
            arc = arc->next;
         }
      }
      if (arcs[len - 1]->next_state->virtual_fromState) {
         MMDAgent_snprintf(buff, MMDAGENT_MAXBUFLEN, "-> %03u %s", arcs[len - 1]->line_number + 1, buff2);
      } else {
         MMDAgent_snprintf(buff, MMDAGENT_MAXBUFLEN, "-> state \"%s\" %s", arcs[len - 1]->next_state->label, buff2);
      }
      if (m_mmdagent->getTextureFont())
         m_mmdagent->getTextureFont()->getTextDrawElements(buff, &m_elem, m_elem.textLen, 0.0f, base, 0.0f);
   }
#endif /* !VIMANAGER_DONTRENDERDEBUG */
}

/* renderMain: render main */
void renderMain()
{

}

/* VIManager_Logger::renderMain: render main function */
void VIManager_Logger::renderMain(float width, float height)
{
#ifndef VIMANAGER_DONTRENDERDEBUG
   GLfloat vertices[12];
   GLindices indices[] = { 0, 1, 2, 0, 2, 3 };

   /* show the history */
   vertices[0] = 0;
   vertices[1] = 0;
   vertices[2] = 0;
   vertices[3] = width;
   vertices[4] = 0;
   vertices[5] = 0;
   vertices[6] = width;
   vertices[7] = height;
   vertices[8] = 0;
   vertices[9] = 0;
   vertices[10] = height;
   vertices[11] = 0;
   glPushMatrix();
   glTranslatef(0.0f, VIMANAGERLOGGER_POSITION_Y_OFFSET, 0.0f);
   glColor4f(VIMANAGERLOGGER_BGCOLOR1);
   glDisable(GL_TEXTURE_2D);
   glVertexPointer(3, GL_FLOAT, 0, vertices);
   glDrawElements(GL_TRIANGLES, 6, GL_INDICES, indices);
   if (m_elem.textLen > 0) {
      glScalef(VIMANAGERLOGGER_TEXTSCALE_2D, VIMANAGERLOGGER_TEXTSCALE_2D, VIMANAGERLOGGER_TEXTSCALE_2D);
      glTranslatef(VIMANAGERLOGGER_POSITION_X_OFFSET, 0.0f, 0.0f);
      glTranslatef(0.5f, VIMANAGERLOGGER_LINESTEP * 0.7f, 0.01f);
      glEnable(GL_TEXTURE_2D);
      glActiveTexture(GL_TEXTURE0);
      glClientActiveTexture(GL_TEXTURE0);
      if (m_mmdagent->getTextureFont())
         glBindTexture(GL_TEXTURE_2D, m_mmdagent->getTextureFont()->getTextureID());
      glEnableClientState(GL_TEXTURE_COORD_ARRAY);
      glVertexPointer(3, GL_FLOAT, 0, m_elem.vertices);
      glTexCoordPointer(2, GL_FLOAT, 0, m_elem.texcoords);
      glColor4f(VIMANAGERLOGGER_TEXTCOLOR1);
      glDrawElements(GL_TRIANGLES, m_elem.numIndices, GL_INDICES, (const GLvoid *)m_elem.indices);
      glDisableClientState(GL_TEXTURE_COORD_ARRAY);
   }
   glPopMatrix();

   /* end of draw */
   glDisable(GL_TEXTURE_2D);
#endif /* !VIMANAGER_DONTRENDERDEBUG */
}

/* VIManager_Logger::render: render log */
void VIManager_Logger::render(VIManager **list, int len, float screenWidth, float screenHeight)
{
#ifndef VIMANAGER_DONTRENDERDEBUG
   if (m_mmdagent == NULL || list == NULL || len == 0)
      return;

   float allHeight = m_mmdagent->getLogBaseHeight() - VIMANAGERLOGGER_POSITION_Y_OFFSET - 0.2f;
   float partHeight = allHeight / (float)len;
   int n = (int)((partHeight - 0.2f) / (VIMANAGERLOGGER_LINESTEP * VIMANAGERLOGGER_TEXTSCALE_2D));

   m_elem.textLen = 0;
   m_elem.numIndices = 0;
   for (int i = 0; i < len; i++)
      addRenderElement(list[i], (partHeight * (len - 1 - i)) / VIMANAGERLOGGER_TEXTSCALE_2D, VIMANAGERLOGGER_LINESTEP, n);

   renderMain(screenWidth, allHeight + 0.1f);

#endif /* !VIMANAGER_DONTRENDERDEBUG */
}
