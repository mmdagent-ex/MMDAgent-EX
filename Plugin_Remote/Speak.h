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

/* commands */
// SPEAK_START|(model alias)|filename.wav
#define PLUGIN_COMMAND_SPEAK_START "SPEAK_START"
// SPEAK_EVENT_START|(model alias)
#define PLUGIN_EVENT_SPEAK_START "SPEAK_EVENT_START"
// SPEAK_EVENT_STOP|(model alias)
#define PLUGIN_EVENT_SPEAK_STOP "SPEAK_EVENT_STOP"

// audio file speak class
class Speak
{
private:

   MMDAgent *m_mmdagent;             /* MMDAgent-EX instance */
   int m_id;                         /* module id of instance */

   Avatar *m_avatarForSpeak;         /* avatar controller for lip sync */
   char *m_avatarForSpeakModelName;  /* model name for lip sync */

   char *m_givenModelName;           /* buffer for threading: model name */
   char *m_givenFileName;            /* buffer for threading: audio file name */
   Thread *m_threadForSpeak;         /* thread instance */
   bool m_speakingThreadrunning;     /* thread running flag */
   bool m_avatarForSpeakSpeaking;    /* flag, true while speaking */

   /* speakAudio: speak audio */
   bool speakAudio(const char *modelName, const char *audio, unsigned int len);

public:

   /* constructor */
   Speak();

   /* destructor */
   ~Speak();

   /* initialize */
   void initialize();

   /* clear */
   void clear();

   /* setup: set up */
   void setup(MMDAgent *mmdagent, int mid);

   /* update: update */
   void update(float frames);

   /* refreshAvatar: refresh avatar */
   void refreshAvatar();

   /* setAvatarEnableFlag: set avatar enable flag */
   void setAvatarEnableFlag(bool flag);

   /* loadWaveAndSpeak: load waveform file and do speak */
   void loadWaveAndSpeak();

   /* startSpeakingThread: start speaking thread */
   void startSpeakingThread(const char *modelName, const char *filename);

   // getMaxVol: get max volume of avatar's speaking since last call
   int getMaxVol();
};

