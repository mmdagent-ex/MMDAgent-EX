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

/* headers */

#include "MMDAgent.h"
#include "julius/juliuslib.h"
#include "Julius_Thread.h"
#include "AudioLipSync.h"
#include "portaudio.h"

// audioPlayCallback: PortAudio play callback function
static int audioPlayCallback(const void *inputBuffer, void *outputBuffer,
   unsigned long framesPerBuffer,
   const PaStreamCallbackTimeInfo* timeInfo,
   PaStreamCallbackFlags statusFlags,
   void *userData)
{
   SP16 *out = (SP16 *)outputBuffer;
   (void)inputBuffer; /* Prevent unused variable warning. */
   AudioProcess *d = (AudioProcess *)userData;

#if 1
   if (d->m_play_buffer_current < framesPerBuffer) {
      // fill with 0
      memset(out, 0, framesPerBuffer * sizeof(SP16));
      return 0;
   }
#else
   while (d->m_play_buffer_current < framesPerBuffer) {
      Pa_Sleep(20);
   }
#endif
   glfwLockMutex(d->m_play_mutex);
   memcpy(out, d->m_play_buffer, framesPerBuffer * sizeof(SP16));
   memmove(d->m_play_buffer, &(d->m_play_buffer[framesPerBuffer]), (d->m_play_buffer_current - framesPerBuffer) * sizeof(SP16));
   d->m_play_buffer_current -= framesPerBuffer;
   glfwUnlockMutex(d->m_play_mutex);

   for (unsigned long i = 0; i < framesPerBuffer; i++) {
      int v = out[i] > 0 ? out[i] : -out[i];
      if (d->m_maxvol < v) d->m_maxvol = v;
   }

   return 0;
}

// constructor
AudioProcess::AudioProcess(bool local)
{
   m_audio_open = false;
   m_play_stream = NULL;
   m_play_buffer = NULL;
   m_receive_buffer = NULL;
   m_play_mutex = NULL;
   m_receive_mutex = NULL;
   m_streaming = true;
   m_want_segment = false;
   m_maxvol = 0;
   m_localAdin = local;
}

// destructor
AudioProcess::~AudioProcess()
{
   callback_end();
}

/* AudioProcess::callback_standby: Julius callback to standby */
boolean AudioProcess::callback_standby(int freq, void *dummy)
{
   m_frequency = freq;
   if (m_localAdin)
      adin_mic_standby(freq, dummy, NULL);
   return TRUE;
}

/* AudioProcess::callback_begin: Julius callback to begin audio stream */
boolean AudioProcess::callback_begin(char *arg)
{
   PaError err;

   if (m_audio_open == false) {
      if (m_play_buffer)
         free(m_play_buffer);
      m_play_buffer_current = 0;
      m_play_buffer_len = m_frequency * AUDIO_PLAY_BUFFER_SIZE_IN_SEC;
      m_play_buffer = (SP16 *)malloc(sizeof(SP16) * m_play_buffer_len);
      if (m_receive_buffer)
         free(m_receive_buffer);
      m_receive_buffer_last_point = 0;
      m_receive_buffer_store_point = 0;
      m_receive_buffer_len = m_frequency * AUDIO_PLAY_BUFFER_SIZE_IN_SEC;
      m_receive_buffer = (SP16 *)malloc(sizeof(SP16) * m_receive_buffer_len);
      err = Pa_Initialize();
      if (err != paNoError) {
         return FALSE;
      }
      if (m_play_mutex != NULL)
         glfwDestroyMutex(m_play_mutex);
      m_play_mutex = glfwCreateMutex();
      if (m_receive_mutex != NULL)
         glfwDestroyMutex(m_receive_mutex);
      m_receive_mutex = glfwCreateMutex();
      err = Pa_OpenDefaultStream(&m_play_stream, 0, 1, paInt16, m_frequency, 256, audioPlayCallback, this);
      if (err != paNoError)
         return FALSE;
      err = Pa_StartStream(m_play_stream);
      if (err != paNoError)
         return FALSE;
      if (m_localAdin)
         adin_mic_begin(arg, NULL);
      m_audio_open = true;
   }
   return TRUE;
}

/* AudioProcess::callback_end: Julius callback to end audio stream */
boolean AudioProcess::callback_end()
{
   PaError err;

   if (m_play_mutex != NULL)
      glfwLockMutex(m_play_mutex);
   if (m_audio_open == true) {
      /* close audio device */
      err = Pa_StopStream(m_play_stream);
      if (err != paNoError) return FALSE;
      err = Pa_CloseStream(m_play_stream);
      if (err != paNoError) return FALSE;

      if (m_play_buffer)
         free(m_play_buffer);
      m_play_buffer = NULL;
      if (m_receive_buffer)
         free(m_receive_buffer);
      m_receive_buffer = NULL;

      m_audio_open = false;
      if (m_play_mutex != NULL) {
         glfwUnlockMutex(m_play_mutex);
         glfwDestroyMutex(m_play_mutex);
      }
      m_play_mutex = NULL;
      if (m_receive_mutex != NULL)
         glfwDestroyMutex(m_receive_mutex);
      m_receive_mutex = NULL;

      if (m_localAdin)
         adin_mic_end(NULL);

   }
   return TRUE;
}

/* AudioProcess::callback_read: Julius callback to return new audio data to be processed */
int AudioProcess::callback_read(SP16 *buf, int sampnum)
{
   unsigned long buflen;
   int num;

   if (m_audio_open == false)
      return -2;

   if (m_localAdin) {
      int len = adin_mic_read(buf, sampnum, NULL);
      appendAudioData((const char *)buf, len * 2);
   }

   /* enable silence segmentation on streaming mode, else disable segmentation */
   if (m_streaming && m_adin) {
      m_adin->adin_cut_on = m_adin->silence_cut_default = TRUE;
   } else {
      m_adin->adin_cut_on = m_adin->silence_cut_default = FALSE;
   }

   /* return 0 if no data exist in the buffer to be processed */
   if (m_receive_buffer_store_point == 0) {
      /* if segmentation is required, trigger segmentation here */
      if (m_want_segment) {
         m_want_segment = false;
         return -3;
      }
      Pa_Sleep(15);
      return 0;
   }

   glfwLockMutex(m_receive_mutex);

   /* write at most sampnum samples at head of the buffer to audio recognition buf */
   buflen = m_receive_buffer_store_point;
   if (buflen > (unsigned long)sampnum)
      buflen = sampnum;
   memcpy(buf, m_receive_buffer, buflen * sizeof(SP16));

   /* also store the new part to audio playing buffer */
   glfwLockMutex(m_play_mutex);
   if (m_audio_open == false)
      return -2;
   num = m_receive_buffer_store_point - m_receive_buffer_last_point;
   unsigned long len;
   if (m_play_buffer_current + num >= m_play_buffer_len) {
      len = m_play_buffer_len - m_play_buffer_current;
   } else {
      len = num;
   }
   if (len > 0) {
      memcpy(&(m_play_buffer[m_play_buffer_current]), &(m_receive_buffer[m_receive_buffer_last_point]), len * sizeof(SP16));
      m_play_buffer_current += len;
   }
   glfwUnlockMutex(m_play_mutex);

   /* shrink the buffer for the written samples */
   if (m_receive_buffer_store_point > buflen)
      memmove(m_receive_buffer, &(m_receive_buffer[buflen]), (m_receive_buffer_store_point - buflen) * sizeof(SP16));
   m_receive_buffer_store_point -= buflen;
   m_receive_buffer_last_point = m_receive_buffer_store_point;
   glfwUnlockMutex(m_receive_mutex);

   return (int)buflen;
}

/* AudioProcess::callback_pause: Julius callback to pause */
boolean AudioProcess::callback_pause()
{
   if (m_localAdin)
      adin_mic_pause(NULL);
   return TRUE;
}

/* AudioProcess::callback_terminate: Julius callback to terminate */
boolean AudioProcess::callback_terminate()
{
   if (m_localAdin)
      adin_mic_terminate(NULL);
   return TRUE;
}

/* AudioProcess::callback_resume: Julius callback to resume */
boolean AudioProcess::callback_resume()
{
   if (m_localAdin)
      adin_mic_resume(NULL);
   return TRUE;
}

/* AudioProcess::callback_input_name: Julius callback to return device description string */
char *AudioProcess::callback_input_name()
{
   return("fetch audio segment from TCP to play and recognize");
}

// static functions to pass instance methods to callback
static boolean local_standby(int freq, void *dummy, void *user_data)
{
   AudioProcess *a = (AudioProcess *)user_data;
   return(a->callback_standby(freq, dummy));
}

static boolean local_begin(char *arg, void *user_data)
{
   AudioProcess *a = (AudioProcess *)user_data;
   return(a->callback_begin(arg));
}

static boolean local_end(void *user_data)
{
   AudioProcess *a = (AudioProcess *)user_data;
   return(a->callback_end());
}

static int local_read(SP16 *buf, int sampnum, void *user_data)
{
   AudioProcess *a = (AudioProcess *)user_data;
   return(a->callback_read(buf, sampnum));
}

static boolean local_pause(void *user_data)
{
   AudioProcess *a = (AudioProcess *)user_data;
   return(a->callback_pause());
}

static boolean local_terminate(void *user_data)
{
   AudioProcess *a = (AudioProcess *)user_data;
   return(a->callback_terminate());
}

static boolean local_resume(void *user_data)
{
   AudioProcess *a = (AudioProcess *)user_data;
   return(a->callback_resume());
}

static char *local_input_name(void *user_data)
{
   AudioProcess *a = (AudioProcess *)user_data;
   return(a->callback_input_name());
}

/* AudioProcess::appendAudioData: set audio data to be processed */
void AudioProcess::appendAudioData(const char *data, int len)
{
   /* append the given audio data into Julius ad-in thread buffer */
   int samples = len / 2;

   // store to receive buffer
   if (m_receive_buffer_store_point + samples >= m_receive_buffer_len)
      return;
   glfwLockMutex(m_receive_mutex);
   memcpy(&(m_receive_buffer[m_receive_buffer_store_point]), data, len);
   m_receive_buffer_store_point += samples;
   glfwUnlockMutex(m_receive_mutex);
}

/* AudioProcess::audioInitialize: initialize audio */
bool AudioProcess::audioInitialize(Recog *recog)
{
   // original: just initialize according to given configuration in Julius library
   //return j_adin_init(recog);

   // try UDP A/D-in
   ADIn *adin = recog->adin;

   /* set this instance to data hook */
   adin->user_data = this;

   /* set up callback functions and parameters */
   adin->ad_standby = local_standby;
   adin->ad_begin = local_begin;
   adin->ad_end = local_end;
   adin->ad_resume = local_resume;
   adin->ad_pause = local_pause;
   adin->ad_terminate = local_terminate;
   adin->ad_read = local_read;
   adin->ad_input_name = local_input_name;
   adin->silence_cut_default = TRUE;
   adin->enable_thread = TRUE;

   /* stand-by A/D-in, dealing 48kHz-to-16kHz downsampling */
   if (recog->jconf->input.use_ds48to16) {
      if (recog->jconf->input.use_ds48to16 && recog->jconf->input.sfreq != 16000) {
         jlog("ERROR: m_adin: in 48kHz input mode, target sampling rate should be 16k!\n");
         return FALSE;
      }
      adin->ds = ds48to16_new();
      adin->down_sample = TRUE;
      if (adin_standby(adin, 48000, NULL) == FALSE) { /* fail */
         jlog("ERROR: m_adin: failed to ready input device\n");
         return false;
      }
   } else {
      adin->ds = NULL;
      adin->down_sample = FALSE;
      if (adin_standby(adin, recog->jconf->input.sfreq, NULL) == FALSE) { /* fail */
         jlog("ERROR: m_adin: failed to ready input device\n");
         return false;
      }
   }

   /* initialize audio detector module */
   if (adin_setup_param(adin, recog->jconf) == FALSE) {
      jlog("ERROR: m_adin: failed to set parameter for input device\n");
      return false;
   }

   adin->input_side_segment = FALSE;

   m_adin = adin;

   return true;
}

/**********************************************************/

/* callbackProc: callback for processing */
static void callbackProc(Recog *recog, void *data)
{
   Julius_Thread *j = (Julius_Thread *)data;
   j->proc();
}

/* callbackRecogEnd: callback for end of recognition */
static void callbackRecogEnd(Recog *recog, void *data)
{
   Julius_Thread *j = (Julius_Thread *)data;
   j->procEnd();
}

/* mainThread: main thread */
static void mainThread(void *param)
{
   Julius_Thread *julius_thread = (Julius_Thread *) param;
   julius_thread->run();
}

/* Julius_Thread::initialize: initialize thread */
void Julius_Thread::initialize()
{
   m_jconf = NULL;
   m_recog = NULL;

   m_mmdagent = NULL;
   m_id = 0;

   m_mutex = NULL;
   m_thread = -1;

   m_configFile = NULL;
   m_zf = NULL;

   m_currentGain = 1.0f;
   m_pausing = false;
   m_running = false;

   m_sync = NULL;
   m_audio = NULL;

   m_wantLocal = false;
   m_wantPassthrough = false;

}

/* Julius_Thread::clear: free thread */
void Julius_Thread::clear()
{
   if(m_thread >= 0) {
      if(m_recog)
         j_close_stream(m_recog);
      if (m_audio)
         m_audio->callback_end();
      //glfwWaitThread(m_thread, GLFW_WAIT);
      glfwDestroyThread(m_thread);
   }
   if(m_mutex != NULL)
      glfwDestroyMutex(m_mutex);
   if (m_recog)
      j_recog_free(m_recog); /* jconf is also released in j_recog_free */
   else if (m_jconf)
      j_jconf_free(m_jconf);

   if (m_configFile != NULL)
      free(m_configFile);
   if (m_zf)
      delete m_zf;

   if (m_audio)
      delete m_audio;

   initialize();
}

/* Julius_Thread::Julius_Thread: thread constructor */
Julius_Thread::Julius_Thread()
{
   initialize();
}

/* Julius_Thread::~Julius_Thread: thread destructor */
Julius_Thread::~Julius_Thread()
{
   clear();
}

/* Julius_Thread::loadAndStart: load models and start thread */
void Julius_Thread::loadAndStart(MMDAgent *mmdagent, int id, const char *configFile, AudioLipSync *sync, bool wantLocal, bool wantPassthrough)
{
   /* reset */
   clear();

   m_mmdagent = mmdagent;
   m_id = id;
   m_sync = sync;
   m_wantLocal = wantLocal;
   m_wantPassthrough = wantPassthrough;

   m_configFile = MMDAgent_strdup(configFile);

   if(m_mmdagent == NULL || m_configFile == NULL) {
      clear();
      return;
   }

   /* create recognition thread */
   glfwInit();
   m_mutex = glfwCreateMutex();
   m_thread = glfwCreateThread(mainThread, this);
   if(m_mutex == NULL || m_thread < 0) {
      clear();
      return;
   }
}

/* Julius_Thread::stopAndRelease: stop thread and release julius */
void Julius_Thread::stopAndRelease()
{
   clear();
}

/* Julius_Thread::run: main loop */
void Julius_Thread::run()
{
   char *tmp;
   char buff[MMDAGENT_MAXBUFLEN];
   ZFileKey *key;
   ZFile *zf;

   if(m_jconf != NULL || m_recog != NULL || m_mmdagent == NULL || m_thread < 0 || m_configFile == NULL)
      return;

   /* set latency */
#if !defined(__APPLE__) && !defined(__ANDROID__)
   MMDAgent_snprintf(buff, MMDAGENT_MAXBUFLEN, "PA_MIN_LATENCY_MSEC=%d", JULIUSTHREAD_LATENCY);
   putenv(buff);
   MMDAgent_snprintf(buff, MMDAGENT_MAXBUFLEN, "LATENCY_MSEC=%d", JULIUSTHREAD_LATENCY);
   putenv(buff);
#endif

   /* turn off log */
   jlog_set_output(NULL);

   /* check SIMD availability */
   get_builtin_simd_string(buff);
   m_mmdagent->sendLogString(m_id, MLOG_STATUS, "built-in SIMD instruction set: %s", buff);
   switch (check_avail_simd()) {
   case USE_SIMD_SSE:
      m_mmdagent->sendLogString(m_id, MLOG_STATUS, "use SSE");
      break;
   case USE_SIMD_AVX:
      m_mmdagent->sendLogString(m_id, MLOG_STATUS, "use AVX");
      break;
   case USE_SIMD_FMA:
      m_mmdagent->sendLogString(m_id, MLOG_STATUS, "use FMA");
      break;
   case USE_SIMD_NEONV2:
      m_mmdagent->sendLogString(m_id, MLOG_STATUS, "use NEONv2");
      break;
   case USE_SIMD_NEON:
      m_mmdagent->sendLogString(m_id, MLOG_STATUS, "use NEON");
      break;
   case USE_SIMD_NONE:
      m_mmdagent->sendLogString(m_id, MLOG_STATUS, "NONE AVAILABLE, DNN computation may be too slow");
      break;
   }

   /* load config file */
   tmp = MMDAgent_pathdup_from_application_to_system_locale(m_configFile);
   m_jconf = j_config_load_file_new(tmp);
   if (m_jconf == NULL) {
      m_mmdagent->sendLogString(m_id, MLOG_ERROR, "failed to load config file \"%s\"", m_configFile);
      free(tmp);
      return;
   }
   free(tmp);

   /* load encryption key */
   key = new ZFileKey();
   if (key->loadKeyDir(m_mmdagent->getConfigDirName()) == false) {
      delete key;
      key = NULL;
   }
   zf = NULL;
   if (m_zf)
      delete m_zf;
   m_zf = NULL;


   /* override some configurations here */
   /* force "-fvad 2" */
   m_jconf->detect.fvad_mode = 2;

   /* create instance */
   m_recog = j_create_instance_from_jconf(m_jconf);
   if (m_recog == NULL) {
      m_mmdagent->sendLogString(m_id, MLOG_ERROR, "failed to startup engine");
      j_jconf_free(m_jconf);
      m_jconf = NULL;
      if (key) delete key;
      if (m_zf) delete m_zf;
      m_zf = NULL;
      return;
   }
   m_mmdagent->sendLogString(m_id, MLOG_STATUS, "instance ok");

   /* register callback functions */
   callback_add(m_recog, CALLBACK_RESULT_PASS1_INTERIM, callbackProc, this);
   callback_add(m_recog, CALLBACK_EVENT_RECOGNITION_END, callbackRecogEnd, this);

   /* open audio device */
   bool ret = false;
   if (m_audio)
      delete m_audio;
   m_audio = NULL;

   if (m_wantLocal) {
      if (m_wantPassthrough) {
         /* When EnableLocalPassthrough is also set */
         /* local, out */
         m_audio = new AudioProcess(true);
         ret = m_audio->audioInitialize(m_recog);
      } else {
         /* local, no out */
         ret = j_adin_init(m_recog);
      }
   } else {
      /* remote, out */
      m_audio = new AudioProcess(false);
      ret = m_audio->audioInitialize(m_recog);
   }
   if (ret == false) {
      m_mmdagent->sendLogString(m_id, MLOG_ERROR, "failed to initialize audio device");
      j_recog_free(m_recog); /* jconf is also released in j_recog_free */
      m_recog = NULL;
      m_jconf = NULL;
      if (key) delete key;
      if (m_zf) delete m_zf;
      m_zf = NULL;
      return;
   }
   if (j_open_stream(m_recog, NULL) != 0) {
      m_mmdagent->sendLogString(m_id, MLOG_ERROR, "failed to open audio stream");
      j_recog_free(m_recog); /* jconf is also released in j_recog_free */
      m_recog = NULL;
      m_jconf = NULL;
      if (key) delete key;
      if (m_zf) delete m_zf;
      m_zf = NULL;
      return;
   }
   m_mmdagent->sendLogString(m_id, MLOG_STATUS, "openstream ok");

   /* start recognize */
   m_running = true;
   for (;;) {
      int ret = j_recognize_stream(m_recog);
      if (ret == -1)
         break;
   }
   m_running = false;
}

/* Julius_Thread::pause: pause recognition process */
void Julius_Thread::pause()
{
   if(m_recog == NULL)
      return;
   if (m_pausing == true)
      return;
   m_pausing = true;
   j_adin_change_input_scaling_factor(m_recog, 0.0f);
}

/* Julius_Thread::resume: resume recognition process */
void Julius_Thread::resume()
{
   if(m_recog == NULL)
      return;
   if (m_pausing == false)
      return;
   m_pausing = false;
   j_adin_change_input_scaling_factor(m_recog, m_currentGain);
}

/* Julius_Thread::waitRunning: pause until Julius thread starts */
void Julius_Thread::waitRunning()
{
   while (m_running == false)
      MMDAgent_sleep(0.05);
}

/* Julius_Thread::isPausing: return true if pausing */
bool Julius_Thread::isPausing()
{
   return m_pausing;
}

/* Julius_Thread::proc: process recognition result */
void Julius_Thread::proc()
{
   int i;
   Sentence *sentence;
   RecogProcess *process;
   char result[MMDAGENT_MAXBUFLEN];
   char lastPhone[MMDAGENT_MAXBUFLEN];
   bool found = false;

   for (process = m_recog->process_list; process; process = process->next) {
      if (!process->live)
         continue;
      if (!process->have_interim)
         continue;

      sentence = &(process->result.pass1);
      strcpy(result, "");
      for (i = 0; i < sentence->word_num; i++) {
         if (MMDAgent_strlen(process->lm->winfo->woutput[sentence->word[i]]) > 0) {
            strcat(result, " ");
            strcat(result, process->lm->winfo->woutput[sentence->word[i]]);
         }
      }
      if (sentence->word_num >= 1) {
         strcpy(lastPhone, process->lm->winfo->woutput[sentence->word[sentence->word_num - 1]]);
         found = true;
      }
   }

   if (found) {
      //m_mmdagent->sendLogString(m_id, MLOG_STATUS, "%s", lastPhone);
      if (m_sync)
         m_sync->storeMouthShape(lastPhone);
   }
}

/* Julius_Thread::procEnd: process end of recognition */
void Julius_Thread::procEnd()
{
   m_sync->storeMouthShape(NULL);
}

/* Julius_Thread::sendMessage: send message to MMDAgent */
void Julius_Thread::sendMessage(const char *str1, const char *str2)
{
   m_mmdagent->sendMessage(m_id, str1, "%s", str2);
}

/* Julius_Thread::getFrameIntervalMSec: get frame interval msec */
double Julius_Thread::getFrameIntervalMSec()
{
   double ret = 0.0;
   if (m_recog && m_recog->jconf)
      ret = m_recog->jconf->search_root->output.progout_interval;
   return ret;
}

/* Julius_Thread::processAudio: process audio */
void Julius_Thread::processAudio(const char *data, int len)
{
   if (m_audio && m_running)
      m_audio->appendAudioData(data, len);
}

// Julius_Thread::getStreamingFlag: get streaming flag
bool Julius_Thread::getStreamingFlag()
{
   if (m_audio)
      return m_audio->m_streaming;
   return false;
}

// Julius_Thread::setStreamingFlag: set streaming flag
void Julius_Thread::setStreamingFlag(bool flag)
{
   if (m_audio)
      m_audio->m_streaming = flag;
}

// Julius_Thread::segmentAudio: segment the current audio
void Julius_Thread::segmentAudio()
{
   if (m_audio)
      m_audio->m_want_segment = true;
}
