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
/* BE LIABLE FOR ANY DIRECT, INhDIRECT, INCIDENTAL, SPECIAL,          */
/* EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED   */
/* TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,     */
/* DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON */
/* ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,   */
/* OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY    */
/* OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE           */
/* POSSIBILITY OF SUCH DAMAGE.                                       */
/* ----------------------------------------------------------------- */

/* definitions */

#define FLITEPLUSHTSENGINETHREAD_EVENTSTART      "SYNTH_EVENT_START"
#define FLITEPLUSHTSENGINETHREAD_EVENTSTOP       "SYNTH_EVENT_STOP"
#define FLITEPLUSHTSENGINETHREAD_COMMANDSTARTLIP "LIPSYNC_START"
#define FLITEPLUSHTSENGINETHREAD_COMMANDSTOPLIP  "LIPSYNC_STOP"

/* Flite_plus_hts_engine_Thread: thread for Flite+hts_engine */
class Flite_plus_hts_engine_Thread
{
private:

   MMDAgent *m_mmdagent;
   int m_id;

   GLFWmutex m_mutex;
   GLFWcond m_cond;
   GLFWthread m_thread;

   int m_count;

   bool m_speaking;
   bool m_kill;

   char *m_charaBuff;
   char *m_styleBuff;
   char *m_textBuff;

   Flite_plus_hts_engine m_flite_plus_hts_engine; /* Engilsh TTS system */
   int m_numModels;        /* number of models */
   char **m_modelNames;    /* model names */
   int m_numStyles;        /* number of styles */
   char **m_styleNames;    /* style names */

   /* initialize: initialize thread */
   void initialize();

   /* clear: free thread */
   void clear();

public:

   /* Flite_plus_hts_engine_Thread: thread constructor */
   Flite_plus_hts_engine_Thread();

   /* ~Flite_plus_hts_engine_Thread: thread destructor */
   ~Flite_plus_hts_engine_Thread();

   /* loadAndStart: load models and start thread */
   bool loadAndStart(MMDAgent *mmdagent, int id, const char *config);

   /* stopAndRelease: stop thread and free Flite_HTS_Engine */
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
