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

#include "HTS_engine.h"

#include "flite_hts_engine.h"

#include "Flite_plus_hts_engine.h"
#include "Flite_plus_hts_engine_Thread.h"
#include "Flite_plus_hts_engine_Manager.h"

/* Flite_plus_hts_engine_Event_initialize: initialize input message buffer */
static void Flite_plus_hts_engine_Event_initialize(Flite_plus_hts_engine_Event *e, const char *str)
{
   if (str != NULL)
      e->event = MMDAgent_strdup(str);
   else
      e->event = NULL;
   e->next = NULL;
}

/* Flite_plus_hts_engine_Event_clear: free input message buffer */
static void Flite_plus_hts_engine_Event_clear(Flite_plus_hts_engine_Event *e)
{
   if (e->event != NULL)
      free(e->event);
   Flite_plus_hts_engine_Event_initialize(e, NULL);
}

/* Flite_plus_hts_engine_EventQueue_initialize: initialize queue */
static void Flite_plus_hts_engine_EventQueue_initialize(Flite_plus_hts_engine_EventQueue *q)
{
   q->head = NULL;
   q->tail = NULL;
}

/* Flite_plus_hts_engine_EventQueue_clear: free queue */
static void Flite_plus_hts_engine_EventQueue_clear(Flite_plus_hts_engine_EventQueue *q)
{
   Flite_plus_hts_engine_Event *tmp1, *tmp2;

   for (tmp1 = q->head; tmp1 != NULL; tmp1 = tmp2) {
      tmp2 = tmp1->next;
      Flite_plus_hts_engine_Event_clear(tmp1);
      free(tmp1);
   }
   Flite_plus_hts_engine_EventQueue_initialize(q);
}

/* Flite_plus_hts_engine_EventQueue_enqueue: enqueue */
static void Flite_plus_hts_engine_EventQueue_enqueue(Flite_plus_hts_engine_EventQueue *q, const char *str)
{
   if(MMDAgent_strlen(str) <= 0)
      return;

   if (q->tail == NULL) {
      q->tail = (Flite_plus_hts_engine_Event *) calloc(1, sizeof (Flite_plus_hts_engine_Event));
      Flite_plus_hts_engine_Event_initialize(q->tail, str);
      q->head = q->tail;
   } else {
      q->tail->next = (Flite_plus_hts_engine_Event *) calloc(1, sizeof (Flite_plus_hts_engine_Event));
      Flite_plus_hts_engine_Event_initialize(q->tail->next, str);
      q->tail = q->tail->next;
   }
}

/* Flite_plus_hts_engine_EventQueue_dequeue: dequeue */
static void Flite_plus_hts_engine_EventQueue_dequeue(Flite_plus_hts_engine_EventQueue *q, char **str)
{
   Flite_plus_hts_engine_Event *tmp;

   if (q->head == NULL) {
      *str = NULL;
      return;
   }
   *str = MMDAgent_strdup(q->head->event);

   tmp = q->head->next;
   Flite_plus_hts_engine_Event_clear(q->head);
   free(q->head);
   q->head = tmp;
   if (tmp == NULL)
      q->tail = NULL;
}

/* mainThread: main thread */
static void mainThread(void *param)
{
   Flite_plus_hts_engine_Manager *flite_plus_hts_engine_manager = (Flite_plus_hts_engine_Manager *) param;
   flite_plus_hts_engine_manager->run();
}

/* Flite_plus_hts_engine_Manager::initialize: initialize */
void Flite_plus_hts_engine_Manager::initialize()
{
   m_mmdagent = NULL;
   m_id = 0;

   m_mutex = NULL;
   m_cond = NULL;
   m_thread = -1;

   m_count = 0;

   m_list = NULL;

   m_config = NULL;

   m_kill = false;

   Flite_plus_hts_engine_EventQueue_initialize(&m_bufferQueue);
}

/* Flite_plus_hts_engine_Manager::clear clear */
void Flite_plus_hts_engine_Manager::clear()
{
   Flite_plus_hts_engine_Link *tmp1, *tmp2;

   m_kill = true;

   /* stop and release all thread */
   for(tmp1 = m_list; tmp1; tmp1 = tmp2) {
      tmp2 = tmp1->next;
      tmp1->flite_plus_hts_engine_thread.stopAndRelease();
      delete tmp1;
   }

   /* wait */
   if(m_cond != NULL)
      glfwSignalCond(m_cond);

   if(m_mutex != NULL || m_cond != NULL || m_thread >= 0) {
      if(m_thread >= 0) {
         glfwWaitThread(m_thread, GLFW_WAIT);
         glfwDestroyThread(m_thread);
      }
      if(m_cond != NULL)
         glfwDestroyCond(m_cond);
      if(m_mutex != NULL)
         glfwDestroyMutex(m_mutex);
   }

   if(m_config)
      free(m_config);

   Flite_plus_hts_engine_EventQueue_clear(&m_bufferQueue);

   initialize();
}

/* Flite_plus_hts_engine_Manager::Flite_plus_hts_engine_Manager: constructor */
Flite_plus_hts_engine_Manager::Flite_plus_hts_engine_Manager()
{
   initialize();
}

/* Flite_plus_hts_engine_Manager::~Flite_plus_hts_engine_Manager: destructor */
Flite_plus_hts_engine_Manager::~Flite_plus_hts_engine_Manager()
{
   clear();
}

/* Flite_plus_hts_engine_Manager::loadAndStart: load and start thread */
bool Flite_plus_hts_engine_Manager::loadAndStart(MMDAgent *mmdagent, int id, const char *config)
{
   Flite_plus_hts_engine_Link *link;
   bool ret;

   clear();

   m_mmdagent = mmdagent;
   m_id = id;
   m_config = MMDAgent_strdup(config);

   if(m_mmdagent == NULL || m_config == NULL) {
      clear();
      return false;
   }

   /* test config loading */
   link = new Flite_plus_hts_engine_Link;
   ret = link->flite_plus_hts_engine_thread.load(m_mmdagent, m_id, m_config);
   delete link;
   if (ret == false)
      return false;

   glfwInit();
   m_mutex = glfwCreateMutex();
   m_cond = glfwCreateCond();
   m_thread = glfwCreateThread(mainThread, this);
   if (m_mutex == NULL || m_cond == NULL || m_thread < 0) {
      clear();
      return false;
   }
   return true;
}

/* Flite_plus_hts_engine_Manager::stopAndRelease: stop and release thread */
void Flite_plus_hts_engine_Manager::stopAndRelease()
{
   clear();
}

/* Flite_plus_hts_engine_Manager::run: main loop */
void Flite_plus_hts_engine_Manager::run()
{
   int i;
   Flite_plus_hts_engine_Link *link;
   char *buff, *save;
   char *chara, *style, *text;
   bool ret = true;

   /* create initial threads */
   for(i = 0; i < FLITEPLUSHTSENGINEMANAGER_INITIALNTHREAD; i++) {
      link = new Flite_plus_hts_engine_Link;
      if (link->flite_plus_hts_engine_thread.load(m_mmdagent, m_id, m_config) == false) {
         m_mmdagent->sendLogString(m_id, MLOG_ERROR, "failed to open config file \"%s\"", m_config);
         ret = false;
      }
      if (link->flite_plus_hts_engine_thread.start() == false)
         ret = false;
      link->next = m_list;
      m_list = link;
   }

   if(ret == false)
      return;

   while(m_kill == false) {
      glfwLockMutex(m_mutex);
      while(m_count <= 0) {
         glfwWaitCond(m_cond, m_mutex, GLFW_INFINITY);
         if(m_kill == true)
            return;
      }
      Flite_plus_hts_engine_EventQueue_dequeue(&m_bufferQueue, &buff);
      m_count--;
      glfwUnlockMutex(m_mutex);

      if(buff != NULL) {
         chara = MMDAgent_strtok(buff, "|", &save);
         style = MMDAgent_strtok(NULL, "|", &save);
         text = MMDAgent_strtok(NULL, "|", &save);

         if (chara == NULL || style == NULL || text == NULL) {
            m_mmdagent->sendLogString(m_id, MLOG_ERROR, "message corrupted");
         } else {
            /* check character */
            for(i = 0, link = m_list; link; link = link->next, i++)
               if(link->flite_plus_hts_engine_thread.checkCharacter(chara) == true)
                  break;
            if(link) {
               if (link->flite_plus_hts_engine_thread.isSpeaking() == true) {
                  m_mmdagent->sendLogString(m_id, MLOG_STATUS, "character \"%s\" is burged in", chara);
                  link->flite_plus_hts_engine_thread.stop(); /* if the same character is speaking, stop immediately */
               }
            } else {
               for(i = 0, link = m_list; link; link = link->next, i++)
                  if(link->flite_plus_hts_engine_thread.isRunning() == true && link->flite_plus_hts_engine_thread.isSpeaking() == false)
                     break;
               if(link == NULL) {
                  link = new Flite_plus_hts_engine_Link;
                  if (link->flite_plus_hts_engine_thread.load(m_mmdagent, m_id, m_config) == false)
                     return;
                  if (link->flite_plus_hts_engine_thread.start() == false)
                     return;
                  link->next = m_list;
                  m_list = link;
               }
            }
            /* set */
            link->flite_plus_hts_engine_thread.synthesis(chara, style, text);
         }
         free(buff); /* free buffer */
      }
   }
}

/* Flite_plus_hts_engine_Manager::isRunning: check running */
bool Flite_plus_hts_engine_Manager::isRunning()
{
   if(m_kill == true || m_mutex == NULL || m_cond == NULL || m_thread < 0)
      return false;
   else
      return true;
}

/* Flite_plus_hts_engine_Manager::synthesis: start synthesis */
void Flite_plus_hts_engine_Manager::synthesis(const char *str)
{
   /* check */
   if(isRunning() == false || MMDAgent_strlen(str) <= 0)
      return;

   /* wait buffer mutex */
   glfwLockMutex(m_mutex);

   /* enqueue character name, speaking style, and text */
   Flite_plus_hts_engine_EventQueue_enqueue(&m_bufferQueue, str);
   m_count++;

   /* start synthesis event */
   if(m_count <= 1)
      glfwSignalCond(m_cond);

   /* release buffer mutex */
   glfwUnlockMutex(m_mutex);
}

/* Flite_plus_hts_engine_Manager::stop: stop synthesis */
void Flite_plus_hts_engine_Manager::stop(const char *str)
{
   Flite_plus_hts_engine_Link *link;

   for(link = m_list; link; link = link->next) {
      if(link->flite_plus_hts_engine_thread.checkCharacter(str)) {
         link->flite_plus_hts_engine_thread.stop();
         return;
      }
   }
}

/* Flite_plus_hts_engine_Manager::stopAll: stop all synthesis */
void Flite_plus_hts_engine_Manager::stopAll()
{
   Flite_plus_hts_engine_Link *link;

   for (link = m_list; link; link = link->next) {
      link->flite_plus_hts_engine_thread.stop();
   }
}
