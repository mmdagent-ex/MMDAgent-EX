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

#define PLUGINVIMANAGER_SUB_COMMAND_START "SUBFST_START"
#define PLUGINVIMANAGER_SUB_COMMAND_STARTIF "SUBFST_START_IF"
#define PLUGINVIMANAGER_SUB_EVENT_START "SUBFST_EVENT_START"
#define PLUGINVIMANAGER_SUB_COMMAND_STOP "SUBFST_STOP"
#define PLUGINVIMANAGER_SUB_EVENT_STOP "SUBFST_EVENT_STOP"

/* VIManager_Event: input message buffer */
typedef struct _VIManager_Event {
   char *type;
   char *args;
   struct _VIManager_Event *next;
} VIManager_Event;

/* VIManager_EventQueue: queue of VIManager_Event */
typedef struct _VIManager_EventQueue {
   VIManager_Event *head;
   VIManager_Event *tail;
} VIManager_EventQueue;

typedef struct _VIManager_Link {
   VIManager vim;
   struct _VIManager_Link *next;
} VIManager_Link;

/* VIManager_Thread: thread of VIManager */
class VIManager_Thread
{
private:

   MMDAgent *m_mmdagent;   /* mmdagent */
   int m_id;

   GLFWmutex m_mutex;      /* mutex */
   GLFWmutex m_mutex_sub;  /* mutex */
   GLFWcond m_cond;        /* condition variable */
   GLFWthread m_thread;    /* thread */

   int m_count;            /* number of elements in event queue */

   bool m_kill;            /* kill flag */

   VIManager_EventQueue eventQueue; /* queue of input message */

   VIManager *m_vim;           /* main FST */
   VIManager_Link *m_sub;     /* sub FST */
   VIManager_Logger m_logger; /* logger */

   VIManager **m_vimList; /* all vim list */
   int m_vimNum;

   ZFileKey *m_key;           /* encryption key */

   char m_predictword[MMDAGENT_MAXBUFLEN]; /* comma-separated list of words predicted at current dialogue status */

   /* initialize: initialize thread */
   void initialize();

   /* clear: free thread */
   void clear();

public:

   /* VIManager_Thraed: thread constructor */
   VIManager_Thread();

   /* ~VIManager_Thread: thread destructor */
   ~VIManager_Thread();

   /* addSub: add sub FST */
   bool addSub(const char *label, const char *filename);

   /* delSub: delete sub FST */
   bool delSub(const char *label);

   /* updateSubList: update sub list */
   void updateSubList();

   /* loadAndStart: load FST and start thread */
   void loadAndStart(MMDAgent *mmdagent, int id, const char *file, const char *initial_state_label);

   /* stopAndRelease: stop thread and release */
   void stopAndRelease();

   /* run: main loop */
   void run();

   /* isRunning: check running */
   bool isRunning();

   /* enqueueBuffer: enqueue buffer to check */
   void enqueueBuffer(const char *type, const char *args);

   /* renderLog: render log message */
   void renderLog(float screenWidth, float screenHeight);

   /* reload: reload FST */
   void reload(const char *type, const char *args);

   /* updatePredictWords: update predicted word list and send message if updated */
   void updatePredictWords();

};
