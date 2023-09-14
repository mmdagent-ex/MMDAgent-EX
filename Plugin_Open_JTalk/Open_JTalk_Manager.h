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

#define OPENJTALKMANAGER_INITIALNTHREAD 1 /* initial number of thread */

/* Open_JTalk_Link: thread list for Open JTalk */
typedef struct _Open_JTalk_Link {
   Open_JTalk_Thread open_jtalk_thread;
   struct _Open_JTalk_Link *next;
} Open_JTalk_Link;

/* Open_JTalk_Event: input message buffer */
typedef struct _Open_JTalk_Event {
   char *event;
   struct _Open_JTalk_Event *next;
} Open_JTalk_Event;

/* Open_JTalk_EventQueue: queue of Open_JTalk_Event */
typedef struct _Open_JTalk_EventQueue {
   Open_JTalk_Event *head;
   Open_JTalk_Event *tail;
} Open_JTalk_EventQueue;

/* Open_JTalk_Manager: multi thread manager for Open JTalk */
class Open_JTalk_Manager
{
private:

   MMDAgent *m_mmdagent;                /* mmdagent */
   int m_id;

   GLFWmutex m_mutex;                   /* mutex */
   GLFWcond m_cond;                     /* condition variable */
   GLFWthread m_thread;                 /* thread */

   int m_count;                         /* number of elements in event queue */

   bool m_kill;                         /* kill flag */

   Open_JTalk_EventQueue m_bufferQueue; /* buffer queque */
   Open_JTalk_Link *m_list;             /* list of threads */

   char *m_dicDir;                      /* dictionary directory */
   char *m_config;                      /* config file */

   /* initialize: initialize */
   void initialize();

   /* clear: clear */
   void clear();

public:

   /* Open_JTalk_Manager: constructor */
   Open_JTalk_Manager();

   /* ~Open_JTalk_Manager: destructor */
   ~Open_JTalk_Manager();

   /* loadAndStart: load and start thread */
   bool loadAndStart(MMDAgent *mmdagent, int id, const char *dicDir, const char *config);

   /* stopAndRelease: stop and release thread */
   void stopAndRelease();

   /* run: main loop */
   void run();

   /* isRunning: check running */
   bool isRunning();

   /* synthesis: start synthesis */
   void synthesis(const char *str);

   /* stop: stop synthesis */
   void stop(const char *str);

   /* stopAll: stop all synthesis */
   void stopAll();
};
