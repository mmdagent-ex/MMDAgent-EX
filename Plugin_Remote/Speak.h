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

