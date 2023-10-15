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

/* enable rapid determination */
#undef ENABLE_RAPID

#define JULIUSTHREAD_PLUGINNAME    "Julius"
#define JULIUSTHREAD_LATENCY       50
#define JULIUSTHREAD_EVENTSTART    "RECOG_EVENT_START"
#define JULIUSTHREAD_EVENTSTOP     "RECOG_EVENT_STOP"
#define JULIUSTHREAD_EVENTGMM      "RECOG_EVENT_GMM"
#define JULIUSTHREAD_EVENTMODIFY   "RECOG_EVENT_MODIFY"
#ifdef ENABLE_RAPID
#define JULIUSTHREAD_EVENTRAPID    "RECOG_EVENT_RAPID"
#endif
#define JULIUSTHREAD_GAIN          "GAIN"
#define JULIUSTHREAD_USERDICTSET   "USERDICT_SET"
#define JULIUSTHREAD_USERDICTUNSET "USERDICT_UNSET"
#define JULIUSTHREAD_CHANGECONF    "CHANGE_CONF"
#define JULIUSTHREAD_PREDICTWORD   "PREDICTWORD"
#define JULIUSTHREAD_PREDICTWORDFACTOR   "PREDICTWORD_FACTOR"
#define JULIUSTHREAD_STATUSWAIT    0
#define JULIUSTHREAD_STATUSUPDATE  1
#define JULIUSTHREAD_STATUSRENDER  2
#define JULIUSTHREAD_STATUSMODIFY  3
#define JULIUSTHREAD_PREDICTWORD_BOOSTFACTOR_DEFAULT  5.0f
#define JULIUSTHREAD_PREDICTWORD_BOOSTED_MARGIN       0.0f

#define PLUGINJULIUS_LOG_FILE "Plugin_Julius_logfile"


typedef struct _JuliusModificationCommand {
   char *str;
   struct _JuliusModificationCommand *next;
} JuliusModificationCommand;

/* Julius_Thead: thread for Julius */
class Julius_Thread
{
private :

   Jconf *m_jconf; /* configuration parameter data */
   Recog *m_recog; /* engine instance */

   MMDAgent *m_mmdagent;
   int m_id;

   GLFWmutex m_mutex;
   GLFWthread m_thread; /* thread */

   char *m_configFile;
   char *m_userConfigFile;
   char *m_userDictionary;
#ifdef ENABLE_RAPID
   char *m_userRapidWordDictionary;
#endif
   ZFile *m_zf;

   int m_status;
   JuliusModificationCommand *m_command;

   float m_boostFactor;

   Julius_Logger *m_logger;

   Julius_Record *m_recorder;

   float m_currentGain;
   bool m_pausing;

   char m_wordspacing[10];

   /* initialize: initialize thread */
   void initialize();

   /* clear: free thread */
   void clear();

public :

   /* Julius_Thread: thread constructor */
   Julius_Thread();

   /* ~Julius_Thread: thread destructor  */
   ~Julius_Thread();

   /* loadAndStart: load models and start thread */
#ifdef ENABLE_RAPID
   void loadAndStart(MMDAgent *m_mmdagent, int id, const char *configFile, const char *userConfigFile, const char *userDictionary, const char *userRapidWordDictionary);
#else
   void loadAndStart(MMDAgent *m_mmdagent, int id, const char *configFile, const char *userConfigFile, const char *userDictionary);
#endif

   /* stopAndRlease: stop thread and release julius */
   void stopAndRelease();

   /* run: main loop */
   void run();

   /* pause: pause recognition process */
   void pause();

   /* resume: resume recognition process */
   void resume();

   /* isPausing: return true if pausing */
   bool isPausing();

   /* procResult: process recognition result */
   void procResult();

   /* procGMMResult: process GMM result */
   void procGMMResult();

   /* procCommand: process command message to modify recognition condition */
   void procCommand();

   /* procParam: process parameter status */
   void procParam();

#ifdef ENABLE_RAPID
   /* procRapid: process rapid determination */
   void procRapid();
#endif

   /* storeCommand: store command message to modify recognition condition */
   void storeCommand(const char *args);

   /* moveThreshold: move level threshold */
   void moveThreshold(int step);

   /* sendMessage: send message to MMDAgent */
   void sendMessage(const char *str1, const char *str2);

   /* getLogActiveFlag: get active flag of logger */
   bool getLogActiveFlag();

   /* setLogActiveFlag: set active flag of logger */
   void setLogActiveFlag(bool b);

   /* setLogCaption: set closed caption string of logger */
   void setLogCaption(const char *s, int id, float x, float y);

   /* setLogCaptionDuration: set caption duration of logger */
   void setLogCaptionDuration(int id, double frame);

   /* updateLog: update log view per step */
   void updateLog(double frame);

   /* renderLog: render log view */
   void renderLog(float width, float height);

   /* enableRecording: enable recording */
   void enableRecording(const char *dirname);

   /* disableRecording: disable recording */
   void disableRecording();

   /* setWordSpacing: set word spacing */
   void setWordSpacing(const char *str);
};
