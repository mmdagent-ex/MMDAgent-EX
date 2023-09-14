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

#include <time.h>
#include "MMDAgent.h"
#include "CountDown_Thread.h"

/* mainThread: main thread */
static void mainThread(void *param)
{
   CountDown_Thread *cdt = (CountDown_Thread *) param;
   cdt->run();
}

/* CountDown_Thread::initialize: initialize thread */
void CountDown_Thread::initialize()
{
   m_head = NULL;
   m_tail = NULL;

   m_mmdagent = NULL;
   m_id = 0;

   m_mutex = NULL;
   m_thread = -1;

   m_kill = false;
}

/* CountDown_Thread::clear: free thread */
void CountDown_Thread::clear()
{
   CountDown *tmp1, *tmp2;

   m_kill = true;

   /* wait end of thread */
   if(m_mutex != NULL || m_thread >= 0) {
      if(m_thread >= 0) {
         glfwWaitThread(m_thread, GLFW_WAIT);
         glfwDestroyThread(m_thread);
      }
      if(m_mutex != NULL)
         glfwDestroyMutex(m_mutex);
   }

   for(tmp1 = m_head; tmp1 ; tmp1 = tmp2) {
      tmp2 = tmp1->next;
      free(tmp1->name);
      free(tmp1);
   }

   initialize();
}

/* CountDown_Thread::CountDown_Thread: thread constructor */
CountDown_Thread::CountDown_Thread()
{
   initialize();
}

/* CountDown_Thread::~CountDown_Thread: thread destructor */
CountDown_Thread::~CountDown_Thread()
{
   clear();
}

/* CountDown_Thread::setupAndStart: load variables and start thread */
void CountDown_Thread::setupAndStart(MMDAgent *mmdagent, int id)
{
   m_mmdagent = mmdagent;
   m_id = id;

   glfwInit();
   m_mutex = glfwCreateMutex();
   m_thread = glfwCreateThread(mainThread, this);
   if(m_mutex == NULL || m_thread < 0) {
      clear();
      return;
   }
}

/* CountDown_Thread::run: check timers */
void CountDown_Thread::run()
{
   CountDown *tmp1, *tmp2;
   double now;

   while(m_kill == false) {
      /* wait */
      glfwLockMutex(m_mutex);

      now = MMDAgent_getTime();

      for(tmp1 = m_head; tmp1; tmp1 = tmp2) {
         tmp2 = tmp1->next;
         if(tmp1->goal <= now) {
            if(tmp1 == m_head) {
               if(tmp1 == m_tail) {
                  m_head = NULL;
                  m_tail = NULL;
               } else {
                  m_head = tmp1->next;
                  tmp1->next->prev = NULL;
               }
            } else {
               if(tmp1 == m_tail) {
                  tmp1->prev->next = NULL;
                  m_tail = tmp1->prev;
               } else {
                  tmp1->prev->next = tmp1->next;
                  tmp1->next->prev = tmp1->prev;
               }
            }
            m_mmdagent->sendMessage(m_id, COUNTDOWNTHREAD_TIMERSTOPEVENT, "%s", tmp1->name);
            free(tmp1->name);
            free(tmp1);
         }
      }

      /* release */
      glfwUnlockMutex(m_mutex);

      MMDAgent_sleep(COUNTDOWNTHREAD_SLEEPSEC);
   }
}

/* CountDown_Thread::isRunning: check running */
bool CountDown_Thread::isRunning()
{
   if (m_kill == true || m_mutex == NULL || m_thread < 0)
      return false;
   else
      return true;
}

/* CountDown_Thread::stopAndRelease: stop thread and free Open JTalk */
void CountDown_Thread::stopAndRelease()
{
   clear();
}

/* CountDown_Thread::set: set timer */
void CountDown_Thread::set(const char *alias, const char *str)
{
   CountDown *countDown;
   double sec, now;

   if(MMDAgent_strlen(alias) <= 0) {
      m_mmdagent->sendLogString(m_id, MLOG_ERROR, "set: null alias string?");
      return;
   }
   sec = MMDAgent_str2double(str);
   if(sec <= 0.0)
      return;

   /* wait */
   glfwLockMutex(m_mutex);

   now = MMDAgent_getTime();

   /* check the same alias */
   for(countDown = m_head; countDown; countDown = countDown->next) {
      if(MMDAgent_strequal(countDown->name, alias))
         break;
   }

   /* push timer */
   if(countDown == NULL) {
      countDown = (CountDown *) malloc(sizeof(CountDown));
      countDown->name = MMDAgent_strdup(alias);
      countDown->next = NULL;
      if(m_tail == NULL) {
         m_head = countDown;
         countDown->prev = NULL;
      } else {
         m_tail->next = countDown;
         countDown->prev = m_tail;
      }
      m_tail = countDown;
   } else {
      m_mmdagent->sendMessage(m_id, COUNTDOWNTHREAD_TIMERCANCELLEDEVENT, "%s", countDown->name);
   }
   countDown->goal = now + sec;

   m_mmdagent->sendMessage(m_id, COUNTDOWNTHREAD_TIMERSTARTEVENT, "%s",  countDown->name);

   /* release */
   glfwUnlockMutex(m_mutex);
}

/* CountDown_Thread::unset: unset timer */
void CountDown_Thread::unset(const char *alias, bool issue_stop)
{
   CountDown *tmp1, *tmp2;

   /* wait */
   glfwLockMutex(m_mutex);

   for(tmp1 = m_head; tmp1; tmp1 = tmp2) {
      tmp2 = tmp1->next;
      if(MMDAgent_strequal(tmp1->name, alias)) {
         if(tmp1 == m_head) {
            if(tmp1 == m_tail) {
               m_head = NULL;
               m_tail = NULL;
            } else {
               m_head = tmp1->next;
               tmp1->next->prev = NULL;
            }
         } else {
            if(tmp1 == m_tail) {
               m_tail = tmp1->prev;
               tmp1->prev->next = NULL;
            } else {
               tmp1->next->prev = tmp1->prev;
               tmp1->prev->next = tmp1->next;
            }
         }
         if (issue_stop)
            m_mmdagent->sendMessage(m_id, COUNTDOWNTHREAD_TIMERSTOPEVENT, "%s", tmp1->name);
         else
            m_mmdagent->sendMessage(m_id, COUNTDOWNTHREAD_TIMERCANCELLEDEVENT, "%s", tmp1->name);
         free(tmp1->name);
         free(tmp1);
         break;
      }
   }

   if (tmp1 == NULL) {
      if (issue_stop)
         m_mmdagent->sendLogString(m_id, MLOG_WARNING, "unset: alias \"%s\" not exist", alias);
      else
         m_mmdagent->sendMessage(m_id, COUNTDOWNTHREAD_TIMERCANCELLEDEVENT, "%s", alias);
   }
   /* release */
   glfwUnlockMutex(m_mutex);

}
