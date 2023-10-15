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
#include "Audio_Thread.h"
#include "Audio_Manager.h"

/* Audio_Event_initialize: initialize input message buffer */
static void Audio_Event_initialize(Audio_Event *e, const char *str)
{
   if (str != NULL)
      e->event = MMDAgent_strdup(str);
   else
      e->event = NULL;
   e->next = NULL;
}

/* Audio_Event_clear: free input message buffer */
static void Audio_Event_clear(Audio_Event *e)
{
   if (e->event != NULL)
      free(e->event);
   Audio_Event_initialize(e, NULL);
}

/* Audio_EventQueue_initialize: initialize queue */
static void Audio_EventQueue_initialize(Audio_EventQueue *q)
{
   q->head = NULL;
   q->tail = NULL;
}

/* Audio_EventQueue_clear: free queue */
static void Audio_EventQueue_clear(Audio_EventQueue *q)
{
   Audio_Event *tmp1, *tmp2;

   for (tmp1 = q->head; tmp1 != NULL; tmp1 = tmp2) {
      tmp2 = tmp1->next;
      Audio_Event_clear(tmp1);
      free(tmp1);
   }
   Audio_EventQueue_initialize(q);
}

/* Audio_EventQueue_enqueue: enqueue */
static void Audio_EventQueue_enqueue(Audio_EventQueue *q, const char *str)
{
   if(MMDAgent_strlen(str) <= 0)
      return;

   if (q->tail == NULL) {
      q->tail = (Audio_Event *) calloc(1, sizeof (Audio_Event));
      Audio_Event_initialize(q->tail, str);
      q->head = q->tail;
   } else {
      q->tail->next = (Audio_Event *) calloc(1, sizeof (Audio_Event));
      Audio_Event_initialize(q->tail->next, str);
      q->tail = q->tail->next;
   }
}

/* Audio_EventQueue_dequeue: dequeue */
static void Audio_EventQueue_dequeue(Audio_EventQueue *q, char **str)
{
   Audio_Event *tmp;

   if (q->head == NULL) {
      *str = NULL;
      return;
   }
   *str = MMDAgent_strdup(q->head->event);

   tmp = q->head->next;
   Audio_Event_clear(q->head);
   free(q->head);
   q->head = tmp;
   if (tmp == NULL)
      q->tail = NULL;
}

/* mainThread: main thread */
static void mainThread(void *param)
{
   Audio_Manager *audio_manager = (Audio_Manager *) param;
   audio_manager->run();
}

/* Audio_Manager::initialize: initialize */
void Audio_Manager::initialize()
{
   m_mmdagent = NULL;
   m_id = 0;

   m_mutex = NULL;
   m_cond = NULL;
   m_thread = -1;

   m_count = 0;

   m_kill = false;

   Audio_EventQueue_initialize(&m_bufferQueue);
   m_list = NULL;
}

/* Audio_Manager::clear: clear */
void Audio_Manager::clear()
{
   Audio_Link *tmp1, *tmp2;

   m_kill = true;

   /* stop and release all thread */
   for(tmp1 = m_list; tmp1; tmp1 = tmp2) {
      tmp2 = tmp1->next;
      tmp1->audio_thread.stopAndRelease();
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

   Audio_EventQueue_clear(&m_bufferQueue);

   initialize();
}

/* Audio_Manager::Audio_Manager: constructor */
Audio_Manager::Audio_Manager()
{
   initialize();
}

/* Audio_Manager::~Audio_Manager: destructor */
Audio_Manager::~Audio_Manager()
{
   clear();
}

/* Audio_Manager::setupAndStart: setup and start thread */
void Audio_Manager::setupAndStart(MMDAgent *mmdagent, int id)
{
   if(mmdagent == NULL)
      return;

   clear();

   m_mmdagent = mmdagent;
   m_id = id;

   glfwInit();
   m_mutex = glfwCreateMutex();
   m_cond = glfwCreateCond();
   m_thread = glfwCreateThread(mainThread, this);
   if(m_mutex == NULL || m_cond == NULL || m_thread < 0) {
      clear();
      return;
   }
}

/* Audio_Manager::stopAndRelease: stop and release thread */
void Audio_Manager::stopAndRelease()
{
   clear();
}

/* Audio_Manager::run: main loop */
void Audio_Manager::run()
{
   int i;
   Audio_Link *link;
   char *buff, *save;
   char *alias, *file;

   /* create initial threads */
   for(i = 0; i < AUDIOMANAGER_INITIALNTHREAD; i++) {
      link = new Audio_Link;
      link->audio_thread.setupAndStart(m_mmdagent, m_id);
      link->next = m_list;
      m_list = link;
   }

   while(m_kill == false) {
      /* wait playing event */
      glfwLockMutex(m_mutex);
      while(m_count <= 0) {
         glfwWaitCond(m_cond, m_mutex, GLFW_INFINITY);
         if(m_kill == true)
            return;
      }
      Audio_EventQueue_dequeue(&m_bufferQueue, &buff);
      m_count--;
      glfwUnlockMutex(m_mutex);

      if(buff != NULL) {
         alias = MMDAgent_strtok(buff, "|", &save);
         file = MMDAgent_strtok(NULL, "|", &save);

         if(alias != NULL && file != NULL) {
            /* check alias */
            for(i = 0, link = m_list; link; link = link->next, i++)
               if(link->audio_thread.checkAlias(alias))
                  break;
            if(link) {
               link->audio_thread.stop(); /* if the same alias is playing, stop immediately */
            } else {
               for(i = 0, link = m_list; link; link = link->next, i++)
                  if(link->audio_thread.isRunning() && link->audio_thread.isPlaying() == false)
                     break;
               if(link == NULL) {
                  link = new Audio_Link;
                  link->audio_thread.setupAndStart(m_mmdagent, m_id);
                  link->next = m_list;
                  m_list = link;
               }
            }
            /* set */
            link->audio_thread.play(alias, file);
         }
         free(buff); /* free buffer */
      }
   }
}

/* Audio_Manager::isRunning: check running */
bool Audio_Manager::isRunning()
{
   if(m_kill == true || m_mutex == NULL || m_cond == NULL || m_thread < 0)
      return false;
   else
      return true;
}

/* Audio_Manager::play: start playing */
void Audio_Manager::play(const char *str)
{
   /* check */
   if(isRunning() == false)
      return;
   if(MMDAgent_strlen(str) <= 0)
      return;

   /* wait buffer mutex */
   glfwLockMutex(m_mutex);

   /* enqueue alias and file name */
   Audio_EventQueue_enqueue(&m_bufferQueue, str);
   m_count++;

   /* start playing event */
   if(m_count <= 1)
      glfwSignalCond(m_cond);

   /* release buffer mutex */
   glfwUnlockMutex(m_mutex);
}

/* Audio_Manager::stop: stop playing */
void Audio_Manager::stop(const char *str)
{
   Audio_Link *link;

   for(link = m_list; link; link = link->next) {
      if(link->audio_thread.checkAlias(str)) {
         link->audio_thread.stop();
         return;
      }
   }
}
