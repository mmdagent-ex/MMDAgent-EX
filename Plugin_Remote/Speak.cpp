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

/* headers */
#include <sndfile.h>
#include <samplerate.h>

#include "MMDAgent.h"
#include "Avatar.h"
#include "Thread.h"
#include "Speak.h"

/* definitions */
#define OUTPUT_SAMPLE_RATE 16000
#define OUTPUT_CHANNELS 1
#define OUTPUT_FORMAT (SF_FORMAT_WAV | SF_FORMAT_PCM_16)
#define AUDIO_LENGTH_LIMIT_IN_SEC 60.0

/* Speak::initialize: initialize */
void Speak::initialize()
{
   m_mmdagent = NULL;
   m_id = 0;

   m_avatarForSpeak = NULL;
   m_avatarForSpeakModelName = NULL;
   m_givenModelName = NULL;
   m_givenFileName = NULL;
   m_threadForSpeak = NULL;
   m_avatarForSpeakSpeaking = false;
   m_speakingThreadrunning = false;
   m_wantSpeakStop = false;
}

/* Speak::clear: clear */
void Speak::clear()
{
   if (m_avatarForSpeak)
      delete m_avatarForSpeak;
   if (m_avatarForSpeakModelName)
      free(m_avatarForSpeakModelName);
   if (m_givenModelName)
      free(m_givenModelName);
   if (m_givenFileName)
      free(m_givenFileName);
   if (m_threadForSpeak)
      delete m_threadForSpeak;

   initialize();
}

/* constructor */
Speak::Speak()
{
   initialize();
}

/* destructor */
Speak::~Speak()
{
   clear();
}

/* Speak::setup: set up */
void Speak::setup(MMDAgent *mmdagent, int mid)
{
   m_mmdagent = mmdagent;
   m_id = mid;
}

/* Speak::update: update */
void Speak::update(float frames)
{
   if (m_avatarForSpeak && m_threadForSpeak && m_avatarForSpeakSpeaking) {
      m_threadForSpeak->lock();
      m_avatarForSpeak->update(frames);
      m_threadForSpeak->unlock();
   }
}

/* Speak::setAvatarEnableFlag: set avatar enable flag */
void Speak::setAvatarEnableFlag(bool flag)
{
   if (m_avatarForSpeak && m_threadForSpeak && m_avatarForSpeakSpeaking)
      m_avatarForSpeak->setEnableFlag(flag);
}

/* Speak::speakAudio: speak audio */
bool Speak::speakAudio(const char *modelName, const char *audio, unsigned int len)
{
   char buff[MMDAGENT_MAXBUFLEN];
   Avatar *av;

   // if no model of the name exists, return with no op
   if (m_mmdagent->findModelAlias(modelName) < 0) {
      m_mmdagent->sendLogString(m_id, MLOG_ERROR, "model alias \"%s\" not found, speaking terminated", modelName);
      return false;
   }

   if (m_avatarForSpeak == NULL) {
      // newly assign avatar instance for lip sync
      av = new Avatar();
      av->setup(m_mmdagent, m_id, false, false);
      av->processMessage("__AV_START\n");
      av->waitAudioThreadStart();
   } else {
      av = m_avatarForSpeak;
   }

   if (m_avatarForSpeakModelName && MMDAgent_strequal(m_avatarForSpeakModelName, modelName) == false) {
      // already assigned model differs from specified name, clear it
      free(m_avatarForSpeakModelName);
      m_avatarForSpeakModelName = NULL;
   }
   if (m_avatarForSpeakModelName == NULL) {
      // initialize avatar control
      MMDAgent_snprintf(buff, MMDAGENT_MAXBUFLEN, "__AV_SETMODEL,%s\n", modelName);
      m_threadForSpeak->lock();
      av->processMessage(buff);
      m_threadForSpeak->unlock();
      m_avatarForSpeakModelName = MMDAgent_strdup(modelName);
   }

   // set file mode
   av->setStreamingSoundDataFlag(false);
   // feed sound samples to processing thread
   av->processSoundData(audio, len, true);

   // set avatar instance to model to start update in other thread
   m_threadForSpeak->lock();
   m_avatarForSpeak = av;
   m_threadForSpeak->unlock();

   return true;
}

/* Speak::loadWaveAndSpeak: load waveform file and do speak */
void Speak::loadWaveAndSpeak()
{
   SF_INFO input_info;
   SNDFILE *input_sndfile;

   m_speakingThreadrunning = true;

   if (m_givenModelName == NULL || m_givenFileName == NULL) {
      m_speakingThreadrunning = false;
      return;
   }

   if (m_wantSpeakStop == true) {
      m_speakingThreadrunning = false;
      return;
   }

   // load audio file
   char *filepath = MMDFiles_pathdup_from_application_to_system_locale(m_givenFileName);
   input_sndfile = sf_open(filepath, SFM_READ, &input_info);
   free(filepath);

   if (!input_sndfile) {
      m_mmdagent->sendLogString(m_id, MLOG_ERROR, "failed to open input file: %s", filepath);
      m_speakingThreadrunning = false;
      return;
   }

   // sampling rate conversion
   SF_INFO output_info = input_info;
   output_info.samplerate = OUTPUT_SAMPLE_RATE;
   output_info.channels = OUTPUT_CHANNELS;
   output_info.format = OUTPUT_FORMAT;

   int err;
   SRC_STATE *src_state = src_new(SRC_SINC_FASTEST, OUTPUT_CHANNELS, &err);
   if (!src_state) {
      m_mmdagent->sendLogString(m_id, MLOG_ERROR, "src_new() failed: %s", filepath);
      sf_close(input_sndfile);
      m_speakingThreadrunning = false;
      return;
   }

   double src_ratio = (double)OUTPUT_SAMPLE_RATE / (double)input_info.samplerate;
   long input_frames = (long)input_info.frames;
   long output_frames = (long)(input_frames * src_ratio) + 1;

   double duration_sec = (double)input_frames / (double)input_info.samplerate;
   if (duration_sec >= AUDIO_LENGTH_LIMIT_IN_SEC) {
      m_mmdagent->sendLogString(m_id, MLOG_ERROR, "too long input (> %.1f sec.), skipped", AUDIO_LENGTH_LIMIT_IN_SEC);
      sf_close(input_sndfile);
      m_speakingThreadrunning = false;
      return;
   }

   float *input_buffer = (float *)malloc(input_frames * input_info.channels * sizeof(float));
   float *output_buffer = (float *)malloc(output_frames * OUTPUT_CHANNELS * sizeof(float));

   sf_readf_float(input_sndfile, input_buffer, input_frames);

   SRC_DATA src_data;
   src_data.data_in = input_buffer;
   src_data.input_frames = input_frames;
   src_data.data_out = output_buffer;
   src_data.output_frames = output_frames;
   src_data.src_ratio = src_ratio;
   src_data.end_of_input = 0;

   err = src_process(src_state, &src_data);
   if (err) {
      m_mmdagent->sendLogString(m_id, MLOG_ERROR, "src_process() failed: %s", filepath);
      free(input_buffer);
      free(output_buffer);
      sf_close(input_sndfile);
      src_delete(src_state);
      m_speakingThreadrunning = false;
      return;
   }

   output_frames = src_data.output_frames_gen;

   int16_t *output_int16_buffer = (int16_t *)malloc(output_frames * OUTPUT_CHANNELS * sizeof(int16_t));
   unsigned char *output_uchar_buffer = (unsigned char *)malloc(output_frames * OUTPUT_CHANNELS * sizeof(int16_t));

   src_float_to_short_array(output_buffer, output_int16_buffer, output_frames * OUTPUT_CHANNELS);
   memcpy(output_uchar_buffer, output_int16_buffer, output_frames * OUTPUT_CHANNELS * sizeof(int16_t));

   free(input_buffer);
   free(output_buffer);
   free(output_int16_buffer);

   sf_close(input_sndfile);
   src_delete(src_state);

   if (m_wantSpeakStop == true) {
      m_speakingThreadrunning = false;
      return;
   }

   float audio_sec = (float)output_frames / OUTPUT_SAMPLE_RATE;

   m_mmdagent->sendLogString(m_id, MLOG_STATUS, "speaking audio length: %.2f sec.", audio_sec);

   // send audio data to audio playing buffer to start audio playing on another thread
   bool ret = speakAudio(m_givenModelName, (char *)output_uchar_buffer, output_frames * OUTPUT_CHANNELS * sizeof(int16_t));

   free(output_uchar_buffer);

   m_avatarForSpeakSpeaking = true;

   m_mmdagent->sendMessage(m_id, PLUGIN_EVENT_SPEAK_START, "%s", m_givenModelName);

   // wait the audio to be processed by sleeping this thread to the audio length
   float wait_audio_sec = audio_sec + 0.1f;
   float current_audio_sec = 0.0f;
   while (current_audio_sec < wait_audio_sec) {
      if (m_wantSpeakStop == true) {
         if (m_avatarForSpeak)
            m_avatarForSpeak->clearSoundData();
         break;
      }
      MMDAgent_sleep(0.05f);
      current_audio_sec += 0.05f;
   }

   m_mmdagent->sendMessage(m_id, PLUGIN_EVENT_SPEAK_STOP, "%s", m_givenModelName);

   m_avatarForSpeakSpeaking = false;

   m_speakingThreadrunning = false;

   return;

}

/* speak thread function, just call Speak::loadWaveAndSpeak() */
static void speakThread(void *param)
{
   Speak *s = (Speak *)param;
   s->loadWaveAndSpeak();
}

/* Speak::startSpeakingThread: start speaking thread */
void Speak::startSpeakingThread(const char *modelName, const char *filename)
{
   Thread *th;

   // wait running thread to finish
   while (m_speakingThreadrunning == true)
      MMDAgent_sleep(0.02);

   if (m_givenModelName)
      free(m_givenModelName);
   m_givenModelName = MMDAgent_strdup(modelName);

   if (m_givenFileName)
      free(m_givenFileName);
   m_givenFileName = MMDAgent_strdup(filename);

   // start new thread
   th = m_threadForSpeak;
   if (m_threadForSpeak) {
      m_threadForSpeak = NULL;
      delete th;
   }

   m_wantSpeakStop = false;
   th = new Thread;
   th->setup();
   th->addThread(glfwCreateThread(speakThread, this));
   m_threadForSpeak = th;
}

/* Speak::stopSpeakingThread: stop speaking thread */
bool Speak::stopSpeakingThread(const char *modelName)
{
   if (m_mmdagent->findModelAlias(modelName) < 0) {
      m_mmdagent->sendLogString(m_id, MLOG_ERROR, "model alias \"%s\" not found", modelName);
      return false;
   }
   
   if (m_givenModelName == NULL || MMDAgent_strequal(m_givenModelName, modelName) == false) {
      m_mmdagent->sendLogString(m_id, MLOG_WARNING, "model alias \"%s\" not speaking", modelName);
      return false;
   }

   if (m_speakingThreadrunning == false) {
      m_mmdagent->sendLogString(m_id, MLOG_WARNING, "model alias \"%s\" not speaking", modelName);
      return false;
   }

   m_wantSpeakStop = true;

   return true;
}


// Speak::getMaxVol: get max volume of avatar's speaking since last call
int Speak::getMaxVol()
{
   if (m_avatarForSpeak && m_threadForSpeak && m_avatarForSpeakSpeaking)
      return m_avatarForSpeak->getMaxVol();
   return 0;
}
