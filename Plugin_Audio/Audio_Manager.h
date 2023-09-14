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

#define AUDIOMANAGER_INITIALNTHREAD 1 /* initial number of thread */

/* Audio_Link: thread list for audio */
typedef struct _Audio_Link {
   Audio_Thread audio_thread;
   struct _Audio_Link *next;
} Audio_Link;

/* Audio_Event: input message buffer */
typedef struct _Audio_Event {
   char *event;
   struct _Audio_Event *next;
} Audio_Event;

/* Audio_EventQueue: queue of Audio_Event */
typedef struct _Audio_EventQueue {
   Audio_Event *head;
   Audio_Event *tail;
} Audio_EventQueue;

/* Audio_Manager: multi thread manager for audio */
class Audio_Manager
{
private:

   MMDAgent *m_mmdagent;           /* mmdagent */
   int m_id;

   GLFWmutex m_mutex;              /* mutex */
   GLFWcond m_cond;                /* condition variable*/
   GLFWthread m_thread;            /* thread */

   int m_count;                    /* number of elements in event queue */

   bool m_kill;                    /* kill flag */

   Audio_EventQueue m_bufferQueue; /* buffer queue */
   Audio_Link *m_list;             /* list of threads */

   /* initialize: initialize */
   void initialize();

   /* clear: clear */
   void clear();

public:

   /* Audio_Manager: constructor */
   Audio_Manager();

   /* ~Audio_Manager: destructor */
   ~Audio_Manager();

   /* setupAndStart: setup and start thread */
   void setupAndStart(MMDAgent *mmdagent, int id);

   /* stopAndRelease: stop and release thread */
   void stopAndRelease();

   /* run: main loop */
   void run();

   /* isRunning: check running */
   bool isRunning();

   /* play: start playing */
   void play(const char *str);

   /* stop: stop playing */
   void stop(const char *str);
};
