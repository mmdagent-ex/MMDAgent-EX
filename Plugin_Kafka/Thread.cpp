/* ----------------------------------------------------------------- */
/*           The Toolkit for Building Voice Interaction Systems      */
/*           "MMDAgent" developed by MMDAgent Project Team           */
/*           http://www.mmdagent.jp/                                 */
/* ----------------------------------------------------------------- */
/*                                                                   */
/*  Copyright (c) 2009-2015  Nagoya Institute of Technology          */
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
#include "Thread.h"

Thread::Event::Event()
{
   type = NULL;
   args = NULL;
   next = NULL;
}

Thread::Event::~Event()
{
   if (type != NULL)
      free(type);
   if (args != NULL)
      free(args);
   type = NULL;
   args = NULL;
}

void Thread::initialize()
{
   int i;

   for (i = 0; i < THREAD_QUEUE_NUM; i++) {
      m_queue[i].head = NULL;
      m_queue[i].tail = NULL;
   }
   m_mutex_buf = NULL;
   m_mutex = NULL;
   m_cond = NULL;
   m_thread = -1;
   m_count = 0;
   m_kill = false;
}

void Thread::clear()
{
   int i;
   Event *tmp1, *tmp2;

   m_kill = true;

   if(m_cond != NULL)
      glfwSignalCond(m_cond);

   if(m_mutex_buf != NULL || m_mutex != NULL || m_cond != NULL || m_thread >= 0) {
      if(m_thread >= 0) {
         glfwWaitThread(m_thread, GLFW_WAIT);
         glfwDestroyThread(m_thread);
      }
      if(m_cond != NULL)
         glfwDestroyCond(m_cond);
      if(m_mutex_buf != NULL)
         glfwDestroyMutex(m_mutex_buf);
      if(m_mutex != NULL)
         glfwDestroyMutex(m_mutex);
   }

   for (i = 0; i < THREAD_QUEUE_NUM; i++) {
      for (tmp1 = m_queue[i].head; tmp1 != NULL; tmp1 = tmp2) {
         tmp2 = tmp1->next;
         delete tmp1;
      }
   }
   initialize();
}

Thread::Thread()
{
   initialize();
}

Thread::~Thread()
{
   clear();
}

void Thread::setup()
{
   glfwInit();
   m_mutex_buf = glfwCreateMutex();
   m_mutex = glfwCreateMutex();
   m_cond = glfwCreateCond();
}

void Thread::addThread(GLFWthread thread)
{
   m_thread = thread;
   if(m_mutex_buf == NULL || m_cond == NULL || m_thread < 0) {
      clear();
      return;
   }
}

void Thread::stop()
{
   clear();
}

bool Thread::isRunning()
{
   if (m_kill == true || m_mutex_buf == NULL || m_cond == NULL || m_thread < 0)
      return false;
   else
      return true;
}

void Thread::waitQueue()
{
   glfwLockMutex(m_mutex);
   while (m_count <= 0) {
      glfwWaitCond(m_cond, m_mutex, GLFW_INFINITY);
      if (m_kill == true)
         return;
   }
   glfwUnlockMutex(m_mutex);
}

void Thread::enqueueBuffer(int id, const char *type, const char *args)
{
   Event *n;

   if (id < 0 || id >= THREAD_QUEUE_NUM)
      return;

   /* wait buffer */
   glfwLockMutex(m_mutex_buf);

   n = new Event;
   if (type != NULL)
      n->type = MMDAgent_strdup(type);
   if (args != NULL)
      n->args = MMDAgent_strdup(args);
   if (m_queue[id].tail == NULL) {
      m_queue[id].tail = m_queue[id].head = n;
   } else {
      m_queue[id].tail = m_queue[id].tail->next = n;
   }
   m_count++;

   if(m_count <= 1)
      glfwSignalCond(m_cond);

   /* release buffer */
   glfwUnlockMutex(m_mutex_buf);
}

int Thread::dequeueBuffer(int id, char *type, char *args)
{
   Event *tmp;

   if (id < 0 || id >= THREAD_QUEUE_NUM)
      return 0;

   if (m_queue[id].head == NULL) {
      if (type != NULL)
         strcpy(type, "");
      if (args != NULL)
         strcpy(type, "");
      return 0;
   }

   glfwLockMutex(m_mutex_buf);

   if (type != NULL)
      strcpy(type, m_queue[id].head->type ? m_queue[id].head->type : "");
   if (args != NULL)
      strcpy(args, m_queue[id].head->args ? m_queue[id].head->args : "");

   tmp = m_queue[id].head->next;
   delete m_queue[id].head;
   m_queue[id].head = tmp;
   if (m_queue[id].head == NULL)
      m_queue[id].tail = NULL;

   m_count--;
   /* release buffer */
   glfwUnlockMutex(m_mutex_buf);

   return 1;
}

void Thread::lock()
{
   glfwLockMutex(m_mutex);
}

void Thread::unlock()
{
   glfwUnlockMutex(m_mutex);
}
