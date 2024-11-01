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
#include "VIManager.h"
#include "VIManager_Logger.h"
#include "VIManager_Thread.h"

/* VIManager_Event_initialize: initialize input message buffer */
static void VIManager_Event_initialize(VIManager_Event *e, const char *type, const char *args)
{
   if (type != NULL)
      e->type = MMDAgent_strdup(type);
   else
      e->type = NULL;
   if (args != NULL)
      e->args = MMDAgent_strdup(args);
   else
      e->args = NULL;
   e->next = NULL;
}

/* VIManager_Event_clear: free input message buffer */
static void VIManager_Event_clear(VIManager_Event *e)
{
   if (e->type != NULL)
      free(e->type);
   if (e->args != NULL)
      free(e->args);
   VIManager_Event_initialize(e, NULL, NULL);
}

/* VIManager_EventQueue_initialize: initialize queue */
static void VIManager_EventQueue_initialize(VIManager_EventQueue *q)
{
   q->head = NULL;
   q->tail = NULL;
}

/* VIManager_EventQueue_clear: free queue */
static void VIManager_EventQueue_clear(VIManager_EventQueue *q)
{
   VIManager_Event *tmp1, *tmp2;

   for (tmp1 = q->head; tmp1 != NULL; tmp1 = tmp2) {
      tmp2 = tmp1->next;
      VIManager_Event_clear(tmp1);
      free(tmp1);
   }
   VIManager_EventQueue_initialize(q);
}

/* VIManager_EventQueue_enqueue: enqueue */
static void VIManager_EventQueue_enqueue(VIManager_EventQueue *q, const char *type, const char *args)
{
   if (q->tail == NULL) {
      q->tail = (VIManager_Event *) calloc(1, sizeof (VIManager_Event));
      VIManager_Event_initialize(q->tail, type, args);
      q->head = q->tail;
   } else {
      q->tail->next = (VIManager_Event *) calloc(1, sizeof (VIManager_Event));
      VIManager_Event_initialize(q->tail->next, type, args);
      q->tail = q->tail->next;
   }
}

/* VIManager_EventQueue_dequeue: dequeue */
static int VIManager_EventQueue_dequeue(VIManager_EventQueue *q, char *type, char *args)
{
   VIManager_Event *tmp;

   if (q->head == NULL) {
      if (type != NULL)
         strcpy(type, "");
      if (args != NULL)
         strcpy(type, "");
      return 0;
   }
   if (type != NULL)
      strcpy(type, q->head->type);
   if (args != NULL)
      if (q->head->args)
         strcpy(args, q->head->args);
      else
         strcpy(args, "");
   tmp = q->head->next;
   VIManager_Event_clear(q->head);
   free(q->head);
   q->head = tmp;
   if (tmp == NULL)
      q->tail = NULL;
   return 1;
}

/* mainThread: main thread */
static void mainThread(void *param)
{
   VIManager_Thread *vimanager_thread = (VIManager_Thread *) param;
   vimanager_thread->run();
}

/* VIManager_Thread::initialize: initialize thread */
void VIManager_Thread::initialize()
{
   m_vim = NULL;
   m_sub = NULL;

   m_mmdagent = NULL;
   m_id = 0;

   m_kill = false;

   m_mutex = NULL;
   m_mutex_sub = NULL;
   m_cond = NULL;
   m_thread = -1;
   m_vimList = NULL;
   m_vimNum = 0;
   m_key = NULL;

   m_count = 0;

   m_predictword[0] = '\0';

   VIManager_EventQueue_initialize(&eventQueue);
}

/* VIManager_Thread::clear: free thread */
void VIManager_Thread::clear()
{
   VIManager_Link *tmp1, *tmp2;

   m_kill = true;

   if(m_cond != NULL)
      glfwSignalCond(m_cond);

   /* stop thread & close mutex */
   if(m_mutex != NULL || m_mutex_sub != NULL || m_cond != NULL || m_thread >= 0) {
      if(m_thread >= 0) {
         glfwWaitThread(m_thread, GLFW_WAIT);
         glfwDestroyThread(m_thread);
      }
      if(m_cond != NULL)
         glfwDestroyCond(m_cond);
      if(m_mutex != NULL)
         glfwDestroyMutex(m_mutex);
      if (m_mutex_sub != NULL)
         glfwDestroyMutex(m_mutex_sub);
   }

   /* free */
   VIManager_EventQueue_clear(&eventQueue);

   for(tmp1 = m_sub; tmp1 != NULL; tmp1 = tmp2) {
      tmp2 = tmp1->next;
      delete tmp1;
   }
   if (m_vim != NULL)
      delete m_vim;

   if (m_key)
      delete m_key;

   if (m_vimList)
      free(m_vimList);

   initialize();
}

/* VIManager_Thread::VIManager_Thread: thread constructor */
VIManager_Thread::VIManager_Thread()
{
   initialize();
}

/* VIManager_Thread::~VIManager_Thread: thread destructor */
VIManager_Thread::~VIManager_Thread()
{
   clear();
}

/* VIManager_Thread::addSub: add sub FST */
bool VIManager_Thread::addSub(const char *label, const char *filename)
{
   VIManager_Link *l = NULL;

   /* load file */
   l = new VIManager_Link;
   if (l->vim.load(m_mmdagent, m_id, m_key, filename, label) == false) {
      delete l;
      return false;
   }

   /* sub FST's initial state is always set to default initial state */
   if (l->vim.setCurrentState(VIMANAGER_INITIAL_STATE_LABEL_DEFAULT) == false) {
      m_mmdagent->sendLogString(m_id, MLOG_ERROR, "%s (%s): failed to set initial state to %s, skipped", label, filename, VIMANAGER_INITIAL_STATE_LABEL_DEFAULT);
      delete l;
      return false;
   }

   /* add to list */
   glfwLockMutex(m_mutex_sub);
   l->next = NULL;
   if (m_sub == NULL) {
      m_sub = l;
   } else {
      VIManager_Link *ltmp = m_sub;
      while (ltmp->next) {
         ltmp = ltmp->next;
      }
      ltmp->next = l;
   }

   updateSubList();

   glfwUnlockMutex(m_mutex_sub);

   m_mmdagent->sendMessage(m_id, PLUGINVIMANAGER_SUB_EVENT_START, "%s", label);

   return true;
}

/* VIManager_Thread::delSub: delete sub FST */
bool VIManager_Thread::delSub(const char *label)
{
   bool deleted = false;
   bool jumped = false;

   glfwLockMutex(m_mutex_sub);
   if (m_sub && MMDAgent_strequal(m_sub->vim.getName(), label)) {
      if (m_sub->vim.jumpToState(VIMANAGER_ATEXIST_STATE_LABEL) == true) {
         /* succeeded in jumping to at-exit state, do not delete here */
         m_mmdagent->sendLogString(m_id, MLOG_STATUS, "%s: deletion ordered, jump to %s state", label, VIMANAGER_ATEXIST_STATE_LABEL);
         jumped = true;
         /* force trigger state transition after jump */
         enqueueBuffer(VIMANAGER_EPSILON, NULL);
      } else {
         m_mmdagent->sendLogString(m_id, MLOG_STATUS, "%s: deleted by order", label);
         m_mmdagent->sendMessage(m_id, PLUGINVIMANAGER_SUB_EVENT_STOP, "%s", m_sub->vim.getName());
         delete m_sub;
         m_sub = NULL;
         deleted = true;
      }
   }
   if (jumped == false && deleted == false) {
      for (VIManager_Link *l = m_sub; l != NULL; l = l->next) {
         if (l->next && MMDAgent_strequal(l->next->vim.getName(), label) == true) {
            if (l->next->vim.jumpToState(VIMANAGER_ATEXIST_STATE_LABEL) == true) {
               /* succeeded in jumping to at-exit state, do not delete here */
               m_mmdagent->sendLogString(m_id, MLOG_STATUS, "%s: deletion ordered, jump to %s state", label, VIMANAGER_ATEXIST_STATE_LABEL);
               /* force trigger state transition after jump */
               enqueueBuffer(VIMANAGER_EPSILON, NULL);
            } else {
               m_mmdagent->sendLogString(m_id, MLOG_STATUS, "%s: deleted by order", label);
               VIManager_Link *ltmp = l->next;
               m_mmdagent->sendMessage(m_id, PLUGINVIMANAGER_SUB_EVENT_STOP, "%s", ltmp->vim.getName());
               l->next = l->next->next;
               delete ltmp;
               deleted = true;
            }
         }
      }
   }
   if (deleted)
      updateSubList();
   glfwUnlockMutex(m_mutex_sub);

   return deleted;
}

/* VIManager_Thread::updateSubList: update sub list */
void VIManager_Thread::updateSubList()
{
   VIManager_Link *l;

   /* update vim list */
   if (m_vimList)
      free(m_vimList);
   int n = 1;
   for (l = m_sub; l != NULL; l = l->next)
      n++;
   m_vimNum = n;
   m_vimList = (VIManager **)malloc(sizeof(VIManager *) * m_vimNum);
   m_vimList[0] = m_vim;
   n = 1;
   for (l = m_sub; l != NULL; l = l->next)
      m_vimList[n++] = &(l->vim);
}


/* VIManager_Thread::loadAndStart: load FST and start thread */
void VIManager_Thread::loadAndStart(MMDAgent *mmdagent, int id, const char *file, const char *initial_state_label)
{
   DIRECTORY *dp;
   char buf[MMDAGENT_MAXBUFLEN];
   char buf2[MMDAGENT_MAXBUFLEN];
   char *dir, *fst;
   int i;

   if(mmdagent == NULL)
      return;

   m_mmdagent = mmdagent;
   m_id = id;

   /* load encryption key if exist */
   m_key = new ZFileKey();
   if (m_key->loadKeyDir(m_mmdagent->getConfigDirName()) == false) {
      delete m_key;
      m_key = NULL;
   }

   /* load FST for VIManager */
   m_vim = new VIManager;
   if (m_vim->load(m_mmdagent, m_id, m_key, file, "main") == false)
      return;

   /* set initial state */
   if (m_vim->setCurrentState(initial_state_label) == false) {
      m_mmdagent->sendLogString(m_id, MLOG_ERROR, "initial state \"%s\" not exist in fst, skipped", initial_state_label);
      return;
   }

   updateSubList();

   /* setup logger */
   m_logger.setup(m_mmdagent);

   /* get dir and fst */
   dir = MMDAgent_dirname(file);
   fst = MMDAgent_basename(file);

   /* load sub fst */
   i = 0;
   dp = MMDAgent_opendir(dir);
   if(dp != NULL) {
      while(MMDAgent_readdir(dp, buf) == true) {
         if(MMDAgent_strequal(buf, fst) == false && MMDAgent_strheadmatch(buf, fst) == true && (MMDAgent_strtailmatch(buf, ".fst") == true || MMDAgent_strtailmatch(buf, ".FST") == true)) {
            MMDAgent_snprintf(buf2, MMDAGENT_MAXBUFLEN, "sub%d", ++i);
            addSub(buf2, buf);
         }
      }
      MMDAgent_closedir(dp);
   }

   free(dir);
   free(fst);

   /* start thread */
   glfwInit();
   m_mutex = glfwCreateMutex();
   m_mutex_sub = glfwCreateMutex();
   m_cond = glfwCreateCond();
   m_thread = glfwCreateThread(mainThread, this);
   if(m_mutex == NULL || m_cond == NULL || m_thread < 0) {
      clear();
      return;
   }
}

/* VIManager_Thread::stopAndRelease: stop thread and release */
void VIManager_Thread::stopAndRelease()
{
   clear();
}

/* VIManager_Thread::run: main loop */
void VIManager_Thread::run()
{
   char itype[MMDAGENT_MAXBUFLEN];
   char iargs[MMDAGENT_MAXBUFLEN];
   char otype[MMDAGENT_MAXBUFLEN];
   char oargs[MMDAGENT_MAXBUFLEN];
   InputArguments ia;
   VIManager_Link *l, *ltmp;
   bool trans;
   bool sub_deleted;

   /* first epsilon step */
   while (m_vim->transition(VIMANAGER_EPSILON, NULL, otype, oargs)) {
      if (MMDAgent_strequal(otype, VIMANAGER_EPSILON) == false)
         m_mmdagent->sendMessage(m_id, otype, "%s", oargs);
   }

   glfwLockMutex(m_mutex_sub);

   for(l = m_sub; l != NULL; l = l->next) {
      while (l->vim.transition(VIMANAGER_EPSILON, NULL, otype, oargs)) {
         if (MMDAgent_strequal(otype, VIMANAGER_EPSILON) == false)
            m_mmdagent->sendMessage(m_id, otype, "%s", oargs);
      }
   }

   updatePredictWords();

   glfwUnlockMutex(m_mutex_sub);

   while(m_kill == false) {
      /* wait transition event */
      glfwLockMutex(m_mutex);
      while(m_count <= 0) {
         glfwWaitCond(m_cond, m_mutex, GLFW_INFINITY);
         if(m_kill == true)
            return;
      }
      VIManager_EventQueue_dequeue(&eventQueue, itype, iargs);
      m_count--;
      glfwUnlockMutex(m_mutex);

      /* pause running while main thread is inactive */
      m_mmdagent->waitWhenPaused();

      InputArguments_initialize(&ia, iargs);

      trans = false;

      /* state transition with input symbol */
      if (m_vim->transition(itype, &ia, otype, oargs))
         trans = true;
      if (MMDAgent_strequal(otype, VIMANAGER_EPSILON) == false)
         m_mmdagent->sendMessage(m_id, otype, "%s", oargs);

      /* state transition with epsilon */
      while (m_vim->transition(VIMANAGER_EPSILON, NULL, otype, oargs)) {
         trans = true;
         if (MMDAgent_strequal(otype, VIMANAGER_EPSILON) == false)
            m_mmdagent->sendMessage(m_id, otype, "%s", oargs);
      }

      glfwLockMutex(m_mutex_sub);

      for(l = m_sub; l != NULL; l = l->next) {
         if (l->vim.transition(itype, &ia, otype, oargs))
            trans = true;
         if (MMDAgent_strequal(otype, VIMANAGER_EPSILON) == false)
            m_mmdagent->sendMessage(m_id, otype, "%s", oargs);

         /* state transition with epsilon */
         while (l->vim.transition(VIMANAGER_EPSILON, NULL, otype, oargs)) {
            trans = true;
            if (MMDAgent_strequal(otype, VIMANAGER_EPSILON) == false)
               m_mmdagent->sendMessage(m_id, otype, "%s", oargs);
         }
      }

      /* check if a sub fst has reached no arc state and delete it */
      sub_deleted = false;
      if (m_sub && m_sub->vim.getEndFlag() == true) {
         m_mmdagent->sendLogString(m_id, MLOG_STATUS, "%s: reached state with no arc, finished itself", m_sub->vim.getName());
         m_mmdagent->sendMessage(m_id, PLUGINVIMANAGER_SUB_EVENT_STOP, "%s", m_sub->vim.getName());
         delete m_sub;
         m_sub = NULL;
         sub_deleted = true;
      }
      for (l = m_sub; l != NULL; l = l->next) {
         if (l->next && l->next->vim.getEndFlag() == true) {
            ltmp = l->next;
            m_mmdagent->sendLogString(m_id, MLOG_STATUS, "%s: reached state with no arc, finished itself", ltmp->vim.getName());
            m_mmdagent->sendMessage(m_id, PLUGINVIMANAGER_SUB_EVENT_STOP, "%s", ltmp->vim.getName());
            l->next = l->next->next;
            delete ltmp;
            sub_deleted = true;
         }
      }
      if (sub_deleted)
         updateSubList();

      InputArguments_clear(&ia);

      /* when some transition occurs, check updates of predicted words in current status */
      if (trans == true)
         updatePredictWords();

      glfwUnlockMutex(m_mutex_sub);

   }
}

/* VIManager_Thread::isRunning: check running */
bool VIManager_Thread::isRunning()
{
   if (m_kill == true || m_mutex == NULL || m_cond == NULL || m_thread < 0)
      return false;
   else
      return true;
}

/* VIManager_Thread::enqueueBuffer: enqueue buffer to check */
void VIManager_Thread::enqueueBuffer(const char *type, const char *args)
{
   /* wait buffer */
   glfwLockMutex(m_mutex);

   /* save event */
   VIManager_EventQueue_enqueue(&eventQueue, type, args);
   m_count++;

   /* start state transition thread */
   if(m_count <= 1)
      glfwSignalCond(m_cond);

   /* release buffer */
   glfwUnlockMutex(m_mutex);
}

/* VIManager_Thread::renderLog: render log message */
void VIManager_Thread::renderLog(float screenWidth, float screenHeight)
{
   if (m_vim)
      m_logger.render(m_vimList, m_vimNum, screenWidth, screenHeight);
}

/* VIManager_Thread::reload: reload FST */
void VIManager_Thread::reload(const char *type, const char *args)
{
   InputArguments ia;
   MMDAgent *mmdagent;
   int id;
   char initial_state_label[VIMANAGER_STATE_LABEL_MAXLEN];
   char *filename;

   InputArguments_initialize(&ia, args);
   if (ia.size < 1 || ia.size > 2) {
      m_mmdagent->sendLogString(m_id, MLOG_ERROR, "wrong argument format in %s", type);
      InputArguments_clear(&ia);
      return;
   }
   if (ia.argc[0] != 1) {
      m_mmdagent->sendLogString(m_id, MLOG_ERROR, "wrong argument format in %s (filename must be one)", type);
      InputArguments_clear(&ia);
      return;
   }
   filename = ia.args[0][0];
   strcpy(initial_state_label, VIMANAGER_INITIAL_STATE_LABEL_DEFAULT);
   if (ia.size == 2) {
      if (ia.argc[1] != 1) {
         m_mmdagent->sendLogString(m_id, MLOG_ERROR, "wrong argument format in %s (state number must be one)", type);
         InputArguments_clear(&ia);
         return;
      }
      MMDAgent_snprintf(initial_state_label, VIMANAGER_STATE_LABEL_MAXLEN, "%s", ia.args[1][0]);
   }

   mmdagent = m_mmdagent;
   id = m_id;
   m_mmdagent->sendLogString(m_id, MLOG_STATUS, "stopping all current FSTs and (re-)loading new FST \"%s\"", filename);
   stopAndRelease();
   loadAndStart(mmdagent, id, filename, initial_state_label);
   InputArguments_clear(&ia);
}

/* VIManager_Thread::updatePredictWords: update predicted word list and send message if updated */
void VIManager_Thread::updatePredictWords()
{
   VIManager_Link *l;
   VIManager_Arc *arc;
   VIManager_State *state;
   char buf[MMDAGENT_MAXBUFLEN];
   size_t slen, len;
   size_t maxlen = MMDAGENT_MAXBUFLEN - 100; /* maximum allowed buffer length excluding log headers */
   bool toolong = false;

   buf[0] = '\0';
   len = 0;
   state = m_vim->getCurrentState();
   if (state == NULL)
      return;
   for (arc = state->arc_list.head; arc; arc = arc->next) {
      if (MMDAgent_strequal(arc->input_event_type, "RECOG_EVENT_STOP")) {
         slen = MMDAgent_strlen(arc->input_event_args.str) + 1;
         if (len + slen >= maxlen) {
            toolong = true;
            break;
         }
         strcat(buf, arc->input_event_args.str);
         strcat(buf, ",");
         len += slen;
      }
   }
   if (toolong == false) {
      for (l = m_sub; l != NULL; l = l->next) {
         state = l->vim.getCurrentState();
         if (state == NULL)
            continue;
         for (arc = state->arc_list.head; arc; arc = arc->next) {
            if (MMDAgent_strequal(arc->input_event_type, "RECOG_EVENT_STOP")) {
               slen = MMDAgent_strlen(arc->input_event_args.str) + 1;
               if (len + slen >= maxlen) {
                  toolong = true;
                  break;
               }
               strcat(buf, arc->input_event_args.str);
               strcat(buf, ",");
               len += slen;
            }
         }
         if (toolong == true) break;
      }
   }
   buf[MMDAGENT_MAXBUFLEN - 1] = '\0';

   /* if waiting some and list updated, send message */
   if (MMDAgent_strlen(buf) > 0 && !MMDAgent_strequal(m_predictword, buf)) {
      strcpy(m_predictword, buf);
      m_mmdagent->sendMessage(m_id, "RECOG_MODIFY", "PREDICTWORD|%s", buf);
   }
}
