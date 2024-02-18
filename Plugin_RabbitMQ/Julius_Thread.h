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

#include "portaudio.h"

/* definitions */

// Julius latency
#define JULIUSTHREAD_LATENCY       50

// Audio buffer size in seconds
#define AUDIO_PLAY_BUFFER_SIZE_IN_SEC 60

class AudioLipSync;

// audio class to wait audio data to recognize and play
class AudioProcess {

public:
   int m_frequency;                            /* sampling frequency */
   bool m_audio_open;                          /* true when audio is opened */
   PaStream *m_play_stream;                    /* audio playing stream for PortAudio */
   SP16 *m_play_buffer;                        /* audio playing buffer */
   unsigned long m_play_buffer_len;            /* assigned length of audio playing buffer */
   unsigned long m_play_buffer_current;        /* current position at audio playing buffer */
   SP16 *m_receive_buffer;                     /* buffer to hold received audio samples */
   unsigned long m_receive_buffer_len;         /* assigned length of receive buffer */
   unsigned long m_receive_buffer_last_point;  /* last processed point at receive buffer */
   unsigned long m_receive_buffer_store_point; /* last storing point at receive buffer */
   GLFWmutex m_play_mutex;                     /* mutex for audio playing thread */
   GLFWmutex m_receive_mutex;                  /* mutex for audio receiving thread */
   bool m_streaming;                           /* true when processing audio is streaming and VAD is required */
   bool m_want_segment;                        /* flag to tell process end of segment */
   ADIn *m_adin;                               /* pointer to audio input structure in Julius */
   int m_maxvol;                               /* maximum volume at latest segment */
   bool m_localAdin;
   bool m_requestPlayFlush;

   // constructor
   AudioProcess(bool local);

   // destructor
   ~AudioProcess();

   /* callback_standby: Julius callback to standby */
   boolean callback_standby(int freq, void *dummy);

   /* callback_begin: Julius callback to begin audio stream */
   boolean callback_begin(char *arg);

   /* callback_end: Julius callback to end audio stream */
   boolean callback_end();

   /* callback_read: Julius callback to return new audio data to be processed */
   int callback_read(SP16 *buf, int sampnum);

   /* callback_pause: Julius callback to pause */
   boolean callback_pause();

   /* callback_terminate: Julius callback to terminate */
   boolean callback_terminate();

   /* callback_resume: Julius callback to resume */
   boolean callback_resume();

   /* callback_input_name: Julius callback to return device description string */
   char *callback_input_name();

   /* audioInitialize: initialize audio */
   bool audioInitialize(Recog *recog);

   /* appendAudioData: set audio data to be processed */
   void appendAudioData(const char *data, int len);
};

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
   ZFile *m_zf;
   float m_currentGain;
   bool m_pausing;
   bool m_running;

   AudioLipSync *m_sync;

   AudioProcess *m_audio;

   bool m_wantLocal;
   bool m_wantPassthrough;

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
   void loadAndStart(MMDAgent *m_mmdagent, int id, const char *configFile, AudioLipSync *sync, bool wantLocal, bool wantPassthrough);

   /* stopAndRlease: stop thread and release julius */
   void stopAndRelease();

   /* run: main loop */
   void run();

   /* pause: pause recognition process */
   void pause();

   /* resume: resume recognition process */
   void resume();

   /* waitRunning: pause until Julius thread starts */
   void waitRunning();

   /* isPausing: return true if pausing */
   bool isPausing();

   /* proc: process partial recognition result */
   void proc();

   /* procEnd: process end of recognition */
   void procEnd();

   /* sendMessage: send message to MMDAgent */
   void sendMessage(const char *str1, const char *str2);

   /* getFrameIntervalMSec: get frame interval msec */
   double getFrameIntervalMSec();

   /* processAudio: process audio */
   void processAudio(const char *data, int len);

   // getStreamingFlag: get streaming flag
   bool getStreamingFlag();

   // setStreamingFlag: set streaming flag
   void setStreamingFlag(bool flag);

   // segmentAudio: segment the current audio
   void segmentAudio();

   /* enableRecording: enable recording */
   void enableRecording(const char *dirname);

   /* disableRecording: disable recording */
   void disableRecording();
};
