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

#define AUDIOTHREAD_ENDSLEEPSEC   0.2                 /* check per 0.2 sec at end */
#define AUDIOTHREAD_STARTSLEEPSEC 0.02                /* check per 0.02 sec at start*/
#define AUDIOTHREAD_EVENTSTART    "SOUND_EVENT_START"
#define AUDIOTHREAD_EVENTSTOP     "SOUND_EVENT_STOP"

#ifdef __ANDROID__
#define AUDIOTHREAD_FRAMEPERBUFFER     3200  /* frames per buffer in PortAudio */
#define AUDIOTHREAD_TIMEOUTUS          5000  /* timeout in microseconds */
#define AUDIOTHREAD_OUTPUTFLUSHWAITSEC 0.005 /* output flush wait time in seconds */
#endif /* __ANDROID__ */

/* Audio_Thread: thread for audio */
class Audio_Thread
{
private:

   MMDAgent *m_mmdagent; /* mmdagent */
   int m_id;

   GLFWmutex m_mutex;    /* mutex */
   GLFWcond m_cond;      /* condition variable */
   GLFWthread m_thread;  /* thread */

   int m_count;          /* number of elements in event queue */

   bool m_playing;       /* playing flag */
   bool m_kill;          /* kill flag */

   char *m_alias;        /* alias name */
   char *m_file;         /* file */

   ZFileKey *m_key;      /* encryption key */
   ZFile *m_zf;          /* encrypted file loading class */

   /* initialize: initialize thread */
   void initialize();

   /* clear: free thread */
   void clear();

   /* startLipsync: start lipsync if HTK label file is existing */
   bool startLipsync(const char *file);

   /* stopLipsync: stop lipsync */
   void stopLipsync();

public:

   /* Audio_Thraed: thread constructor */
   Audio_Thread();

   /* ~Audio_Thread: thread destructor */
   ~Audio_Thread();

   /* setupAndStart: setup audio and start thread */
   void setupAndStart(MMDAgent *mmdagent, int id);

   /* stopAndRelease: stop thread and free audio */
   void stopAndRelease();

   /* run: main thread loop for audio */
   void run();

   /* isRunning: check running */
   bool isRunning();

   /* isPlaying: check playing */
   bool isPlaying();

   /* checkAlias: check playing alias */
   bool checkAlias(const char *alias);

   /* play: start playing */
   void play(const char *alias, const char *file);

   /* stop: stop playing */
   void stop();
};
