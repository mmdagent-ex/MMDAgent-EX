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

#define OPENJTALKTHREAD_EVENTSTART      "SYNTH_EVENT_START"
#define OPENJTALKTHREAD_EVENTSTOP       "SYNTH_EVENT_STOP"
#define OPENJTALKTHREAD_COMMANDSTARTLIP "LIPSYNC_START"
#define OPENJTALKTHREAD_COMMANDSTOPLIP  "LIPSYNC_STOP"

/* Open_JTalk_Thread: thread for Open JTalk */
class Open_JTalk_Thread
{
private:

   MMDAgent *m_mmdagent;   /* mmdagent */
   int m_id;

   GLFWmutex m_mutex;      /* mutex */
   GLFWcond m_cond;        /* condition variable */
   GLFWthread m_thread;    /* thread */

   int m_count;            /* number of elements in event queue */

   bool m_speaking;        /* speaking flag */
   bool m_kill;            /* kill flag */

   char *m_charaBuff;      /* character name */
   char *m_styleBuff;      /* speaking style */
   char *m_textBuff;       /* text */

   Open_JTalk m_openJTalk; /* Japanese TTS system */
   int m_numModels;        /* number of models */
   char **m_modelNames;    /* model names */
   int m_numStyles;        /* number of styles */
   char **m_styleNames;    /* style names */

   ZFileKey *m_key;        /* encryption file key */
   bool m_keyInit;         /* true when initialized */

   /* initialize: initialize thread */
   void initialize();

   /* clear: free thread */
   void clear();

public:

   /* Open_JTalk_Thraed: thread constructor */
   Open_JTalk_Thread();

   /* ~Open_JTalk_Thread: thread destructor */
   ~Open_JTalk_Thread();

   /* load: load models */
   bool load(MMDAgent *mmdagent, int id, const char *dicDir, const char *config);

   /* start: start thread */
   bool start();

   /* stopAndRelease: stop thread and free Open JTalk */
   void stopAndRelease();

   /* run: main thread loop for TTS */
   void run();

   /* isRunning: check running */
   bool isRunning();

   /* isSpeaking: check speaking */
   bool isSpeaking();

   /* checkCharacter: check speaking character */
   bool checkCharacter(const char *chara);

   /* synthesis: start synthesis */
   void synthesis(const char *chara, const char *style, const char *text);

   /* stop: stop synthesis */
   void stop();
};
