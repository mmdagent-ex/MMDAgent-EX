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

/* header */
#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "portaudio.h"
#include "android/log.h"

static char apiName[] = "OpenGLES";

/* audio output setting (48kHz/16bit) */
#define PLAYER_BUFFERNUMSAMPLES 3200
#define PLAYER_WAITMS 2

/* audio input (16kHz/16bit) */
#define RECORDER_BUFFERNUMSAMPLES 800

/* native sampling rate and buffer length obtained at run time */
static int gNativeSampleRate = 0;
static int gNativeBufferFrames = PLAYER_BUFFERNUMSAMPLES;
static int gNativeBufferLength = PLAYER_BUFFERNUMSAMPLES * 2;

/* OpenSLES engine */
static SLObjectItf engine = NULL;

/* audio output */
static SLObjectItf player = NULL;
static SLObjectItf mixer = NULL;
static SLAndroidSimpleBufferQueueItf playerBufferQueueInterface = NULL;
static char storingPlayerBufferName = 'A';
static short *playerBufferA = NULL;
static short *playerBufferB = NULL;
static size_t currentPlayerBufferSize = 0;
static size_t numQueuedPlayerBuffer = 0;

/* audio input */
static SLObjectItf recorder = NULL;
static SLAndroidSimpleBufferQueueItf recorderBufferQueueInterface = NULL;
static char storingRecorderBufferName = 'A';
static short *recorderBufferA = NULL;
static short *recorderBufferB = NULL;
static PaStreamCallback *sendRecordingBuffer = NULL;

/* Port Audio */
static PaHostApiInfo paHostApiInfo;
static PaDeviceInfo deviceInfo;
static PaStreamInfo paStreamInfo;

static void playerCallbackFunction(SLAndroidSimpleBufferQueueItf bufferQueueInterface, void *context)
{
   numQueuedPlayerBuffer--;
}

static void recorderCallbackFunction(SLAndroidSimpleBufferQueueItf bufferQueueInterface, void *context)
{
   PaStreamCallbackFlags flag;

   if (sendRecordingBuffer == NULL) return;

   if(storingRecorderBufferName == 'A') {
      sendRecordingBuffer(recorderBufferA, NULL, RECORDER_BUFFERNUMSAMPLES, NULL, flag, NULL);
      (*recorderBufferQueueInterface)->Enqueue(recorderBufferQueueInterface, recorderBufferA, RECORDER_BUFFERNUMSAMPLES * sizeof(short));
      storingRecorderBufferName = 'B';
   } else {
      sendRecordingBuffer(recorderBufferB, NULL, RECORDER_BUFFERNUMSAMPLES, NULL, flag, NULL);
      (*recorderBufferQueueInterface)->Enqueue(recorderBufferQueueInterface, recorderBufferB, RECORDER_BUFFERNUMSAMPLES * sizeof(short));
      storingRecorderBufferName = 'A';
   }
}

PaError Pa_Initialize()
{
   SLresult result;

   __android_log_print(ANDROID_LOG_VERBOSE, "MMDAgent", "Pa_initialize");

   /* do nothing if already initialized */
   if (engine != NULL)
      return paNoError;

   /* reset */
   Pa_Terminate();

   /* initialize dummy information for PortAudio */
   memset(&paHostApiInfo, 0, sizeof(PaHostApiInfo));
   memset(&deviceInfo, 0, sizeof(PaDeviceInfo));
   memset(&paStreamInfo, 0, sizeof(PaStreamInfo));

   /* initialize OpenSLES engine */
   result = slCreateEngine(&engine, 0, NULL, 0, NULL, NULL);
   if(result != SL_RESULT_SUCCESS) {
      Pa_Terminate();
      return paInternalError;
   }
   result = (*engine)->Realize(engine, SL_BOOLEAN_FALSE);
   if(result != SL_RESULT_SUCCESS) {
      Pa_Terminate();
      return paInternalError;
   }

   return paNoError;
}

PaError Pa_Terminate()
{
   __android_log_print(ANDROID_LOG_VERBOSE, "MMDAgent", "Pa_Terminate");

   Pa_StopStream((PaStream *) '\x01');
   Pa_StopStream((PaStream *) '\x02');
   SLresult result1 = Pa_CloseStream((PaStream *) '\x01');
   SLresult result2 = Pa_CloseStream((PaStream *) '\x02');

   /* finalize OpenSLES engine */
   if (engine != NULL)
      (*engine)->Destroy(engine);
   engine = NULL;

   if(result1 != paNoError || result2 != paNoError)
      return paInternalError;

   return paNoError;
}

PaError Pa_OpenStream(PaStream** stream, const PaStreamParameters *inputParameters, const PaStreamParameters *outputParameters, double sampleRate, unsigned long framesPerBuffer, PaStreamFlags streamFlags, PaStreamCallback *streamCallback, void *userData)
{
   SLresult result;
   SLEngineItf engineInterface;

   /* get engine interface */
   result = (*engine)->GetInterface(engine, SL_IID_ENGINE, &engineInterface);
   if(result != SL_RESULT_SUCCESS) {
      Pa_Terminate();
      return paInternalError;
   }

   if(outputParameters != NULL) {
      __android_log_print(ANDROID_LOG_VERBOSE, "MMDAgent", "Pa_OpenStream 01");
      /* reset */
      Pa_CloseStream((PaStream *) '\x01');

      /* prepare */
      {
         storingPlayerBufferName = 'A';
         gNativeBufferLength = gNativeBufferFrames * outputParameters->channelCount;
         playerBufferA = (short *) malloc(sizeof(short) * gNativeBufferLength);
         playerBufferB = (short *) malloc(sizeof(short) * gNativeBufferLength);
         currentPlayerBufferSize = 0;
         numQueuedPlayerBuffer = 0;
      }

      /* create mixer */
      {
         result = (*engineInterface)->CreateOutputMix(engineInterface, &mixer, 0, NULL, NULL);
         if(result != SL_RESULT_SUCCESS) {
            Pa_Terminate();
            return paInternalError;
         }
         result = (*mixer)->Realize(mixer, SL_BOOLEAN_FALSE);
         if(result != SL_RESULT_SUCCESS) {
            Pa_Terminate();
            return paInternalError;
         }
      }

      /* create player */
      {
         SLDataLocator_AndroidSimpleBufferQueue androidSimpleBufferQueue = { SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE, 2 };
         SLDataFormat_PCM dataFormat = { SL_DATAFORMAT_PCM, outputParameters->channelCount, 0, SL_PCMSAMPLEFORMAT_FIXED_16, SL_PCMSAMPLEFORMAT_FIXED_16, 0, SL_BYTEORDER_LITTLEENDIAN };
         if (outputParameters->channelCount == 1) {
            dataFormat.channelMask = SL_SPEAKER_FRONT_CENTER;
         } else {
            dataFormat.channelMask = SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT;
         }
         switch((int)sampleRate) {
         case 8000:
            dataFormat.samplesPerSec = SL_SAMPLINGRATE_8;
            break;
         case 11025:
            dataFormat.samplesPerSec = SL_SAMPLINGRATE_11_025;
            break;
         case 12000:
            dataFormat.samplesPerSec = SL_SAMPLINGRATE_12;
            break;
         case 16000:
            dataFormat.samplesPerSec = SL_SAMPLINGRATE_16;
            break;
         case 22050:
            dataFormat.samplesPerSec = SL_SAMPLINGRATE_22_05;
            break;
         case 24000:
            dataFormat.samplesPerSec = SL_SAMPLINGRATE_24;
            break;
         case 32000:
            dataFormat.samplesPerSec = SL_SAMPLINGRATE_32;
            break;
         case 44100:
            dataFormat.samplesPerSec = SL_SAMPLINGRATE_44_1;
            break;
         case 48000:
            dataFormat.samplesPerSec = SL_SAMPLINGRATE_48;
            break;
         case 64000:
            dataFormat.samplesPerSec = SL_SAMPLINGRATE_64;
            break;
         case 88200:
            dataFormat.samplesPerSec = SL_SAMPLINGRATE_88_2;
            break;
         case 96000:
            dataFormat.samplesPerSec = SL_SAMPLINGRATE_96;
            break;
         case 192000:
            dataFormat.samplesPerSec = SL_SAMPLINGRATE_192;
            break;
         default:
            Pa_Terminate();
            return paInternalError;
         }
         __android_log_print(ANDROID_LOG_VERBOSE, "MMDAgent", "output sampling rate %f, buffer frame %d, sample %d", sampleRate, gNativeBufferFrames, gNativeBufferLength);
         SLDataSource dataSource = { &androidSimpleBufferQueue, &dataFormat };
         const SLInterfaceID interfaceID[1] = { SL_IID_ANDROIDSIMPLEBUFFERQUEUE };
         const SLboolean interfaceRequired[1] = { SL_BOOLEAN_TRUE };
         SLDataLocator_OutputMix outputMix = { SL_DATALOCATOR_OUTPUTMIX, mixer };
         SLDataSink dataSink = { &outputMix, NULL };
         result = (*engineInterface)->CreateAudioPlayer(engineInterface, &player, &dataSource, &dataSink, 1, interfaceID, interfaceRequired);
         if(result != SL_RESULT_SUCCESS) {
            Pa_Terminate();
            return paInternalError;
         }
         result = (*player)->Realize(player, SL_BOOLEAN_FALSE);
         if(result != SL_RESULT_SUCCESS) {
            Pa_Terminate();
            return paInternalError;
         }
      }

      /* set callback function */
      {
         result = (*player)->GetInterface(player, SL_IID_ANDROIDSIMPLEBUFFERQUEUE, &playerBufferQueueInterface);
         if(result != SL_RESULT_SUCCESS) {
            Pa_Terminate();
            return paInternalError;
         }
         result = (*playerBufferQueueInterface)->RegisterCallback(playerBufferQueueInterface, playerCallbackFunction, NULL);
         if(result != SL_RESULT_SUCCESS) {
            Pa_Terminate();
            return paInternalError;
         }
      }

      /* set dummy address */
      *stream = (PaStream *) '\x01';
   }

   if(inputParameters != NULL) {
      __android_log_print(ANDROID_LOG_VERBOSE, "MMDAgent", "Pa_OpenStream 02");
      /* reset */
      Pa_CloseStream((PaStream *) '\x02');

      /* prepare */
      {
         storingRecorderBufferName = 'A';
         recorderBufferA = (short*) malloc(sizeof(short) * RECORDER_BUFFERNUMSAMPLES);
         recorderBufferB = (short*) malloc(sizeof(short) * RECORDER_BUFFERNUMSAMPLES);
         sendRecordingBuffer = streamCallback;
      }

      /* create recorder */
      {
         SLDataLocator_IODevice ioDevice = { SL_DATALOCATOR_IODEVICE, SL_IODEVICE_AUDIOINPUT, SL_DEFAULTDEVICEID_AUDIOINPUT, NULL };
         SLDataSource dataSource = { &ioDevice, NULL };
         SLDataLocator_AndroidSimpleBufferQueue androidSimpleBufferQueue = { SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE, 2 };
         SLDataFormat_PCM dataFormat = { SL_DATAFORMAT_PCM, 1, SL_SAMPLINGRATE_16, SL_PCMSAMPLEFORMAT_FIXED_16, SL_PCMSAMPLEFORMAT_FIXED_16, SL_SPEAKER_FRONT_CENTER, SL_BYTEORDER_LITTLEENDIAN };
         SLDataSink dataSink = { &androidSimpleBufferQueue, &dataFormat };
         const SLInterfaceID interfaceID[2] = { SL_IID_ANDROIDSIMPLEBUFFERQUEUE, SL_IID_ANDROIDCONFIGURATION };
         const SLboolean interfaceRequired[2] = { SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE };
         result = (*engineInterface)->CreateAudioRecorder(engineInterface, &recorder, &dataSource, &dataSink, 2, interfaceID, interfaceRequired);
         if (result != SL_RESULT_SUCCESS) {
            Pa_Terminate();
            return paInternalError;
         }
      }

      /* set recording configuration */
      {
         SLAndroidConfigurationItf config;
         SLint32 type = SL_ANDROID_RECORDING_PRESET_VOICE_RECOGNITION;
         result = (*recorder)->GetInterface(recorder, SL_IID_ANDROIDCONFIGURATION, &config);
         if (result != SL_RESULT_SUCCESS) {
            Pa_Terminate();
            return paInternalError;
         }
         result = (*config)->SetConfiguration(config, SL_ANDROID_KEY_RECORDING_PRESET, &type, sizeof(SLint32));
         if (result != SL_RESULT_SUCCESS) {
            Pa_Terminate();
            return paInternalError;
         }
         result = (*recorder)->Realize(recorder, SL_BOOLEAN_FALSE);
         if (result != SL_RESULT_SUCCESS) {
            Pa_Terminate();
            return paInternalError;
         }
      }

      /* set callback function */
      {
         result = (*recorder)->GetInterface(recorder, SL_IID_ANDROIDSIMPLEBUFFERQUEUE, &recorderBufferQueueInterface);
         if (result != SL_RESULT_SUCCESS) {
            Pa_Terminate();
            return paInternalError;
         }
         result = (*recorderBufferQueueInterface)->RegisterCallback(recorderBufferQueueInterface, recorderCallbackFunction, NULL);
         if (result != SL_RESULT_SUCCESS) {
            Pa_Terminate();
            return paInternalError;
         }
         result = (*recorderBufferQueueInterface)->Enqueue(recorderBufferQueueInterface, recorderBufferA, RECORDER_BUFFERNUMSAMPLES * sizeof(short));
         if (result != SL_RESULT_SUCCESS) {
            Pa_Terminate();
            return paInternalError;
         }
         result = (*recorderBufferQueueInterface)->Enqueue(recorderBufferQueueInterface, recorderBufferB, RECORDER_BUFFERNUMSAMPLES * sizeof(short));
         if (result != SL_RESULT_SUCCESS) {
            Pa_Terminate();
            return paInternalError;
         }
      }

      /* set dummy address */
      *stream = (PaStream *) '\x02';
   }

   return paNoError;
}

static void
FlushPlayerQueue()
{
   size_t length;

   if (currentPlayerBufferSize != 0) {
      length = currentPlayerBufferSize;
      currentPlayerBufferSize = 0;
      __android_log_print(ANDROID_LOG_VERBOSE, "MMDAgent", "Pa_WriteStream callback flush %d", length);
      numQueuedPlayerBuffer++;
      if(storingPlayerBufferName == 'A') {
         (*playerBufferQueueInterface)->Enqueue(playerBufferQueueInterface, playerBufferA, sizeof(short) * length);
         storingPlayerBufferName = 'B';
      } else {
         (*playerBufferQueueInterface)->Enqueue(playerBufferQueueInterface, playerBufferB, sizeof(short) * length);
         storingPlayerBufferName = 'A';
      }
   }
}

PaError Pa_WriteStream(PaStream *stream, const void *buffer, unsigned long frames)
{
   unsigned long i;
   PaError result;
   const short *tmp = (const short *) buffer;

   if(stream == (PaStream *) '\x01' && player != NULL) {

      for(i = 0; i < frames; i++) {
         if(storingPlayerBufferName == 'A')
            playerBufferA[currentPlayerBufferSize++] = tmp[i];
         else
            playerBufferB[currentPlayerBufferSize++] = tmp[i];
         if (currentPlayerBufferSize >= gNativeBufferLength) {
            /* enqueue */
            numQueuedPlayerBuffer++;
            if(storingPlayerBufferName == 'A') {
               result = (*playerBufferQueueInterface)->Enqueue(playerBufferQueueInterface, playerBufferA, sizeof(short) * currentPlayerBufferSize);
               if (result != SL_RESULT_SUCCESS) {
                  Pa_Terminate();
                  return paInternalError;
               }
               storingPlayerBufferName = 'B';
            } else {
               result = (*playerBufferQueueInterface)->Enqueue(playerBufferQueueInterface, playerBufferB, sizeof(short) * currentPlayerBufferSize);
               if (result != SL_RESULT_SUCCESS) {
                  Pa_Terminate();
                  return paInternalError;
               }
               storingPlayerBufferName = 'A';
            }
            /* wait */
            while (numQueuedPlayerBuffer >= 2)
               usleep(PLAYER_WAITMS * 1000);
            currentPlayerBufferSize = 0;
         }
      }
   }

   return paNoError;
}

void Pa_Sleep(long msec)
{
   usleep((unsigned int) (msec * 1000));
}

PaError Pa_CloseStream(PaStream *stream)
{
   if(stream == (PaStream *) '\x01') {
      __android_log_print(ANDROID_LOG_VERBOSE, "MMDAgent", "Pa_CloseStream 01");
      FlushPlayerQueue();
      if (player != NULL)
         (*player)->Destroy(player);
      player = NULL;
      if (mixer != NULL)
         (*mixer)->Destroy(mixer);
      mixer = NULL;
      if (playerBufferA != NULL)
         free(playerBufferA);
      playerBufferA = NULL;
      if (playerBufferB != NULL)
         free(playerBufferB);
      playerBufferB = NULL;
      playerBufferQueueInterface = NULL;
      storingPlayerBufferName = 'A';
      currentPlayerBufferSize = 0;
      numQueuedPlayerBuffer = 0;
   }

   if(stream == (PaStream *) '\x02') {
      __android_log_print(ANDROID_LOG_VERBOSE, "MMDAgent", "Pa_CloseStream 02");
      if (recorder != NULL)
         (*recorder)->Destroy(recorder);
      recorder = NULL;
      if (recorderBufferA != NULL)
         free(recorderBufferA);
      recorderBufferA = NULL;
      if (recorderBufferB != NULL)
         free(recorderBufferB);
      recorderBufferB = NULL;
      recorderBufferQueueInterface = NULL;
      storingRecorderBufferName = 'A';
      sendRecordingBuffer = NULL;
   }

   return paNoError;
}

PaError Pa_StartStream(PaStream *stream)
{
   PaError result;

   if(stream == (PaStream *) '\x01' && player != NULL) {
      __android_log_print(ANDROID_LOG_VERBOSE, "MMDAgent", "Pa_StartStream 01");
      SLPlayItf playerInterface;
      result = (*player)->GetInterface(player, SL_IID_PLAY, &playerInterface);
      if(result != SL_RESULT_SUCCESS) {
         Pa_Terminate();
         return paInternalError;
      }
      result = (*playerInterface)->SetPlayState(playerInterface, SL_PLAYSTATE_PLAYING);
      if(result != SL_RESULT_SUCCESS) {
         Pa_Terminate();
         return paInternalError;
      }
   }

   if(stream == (PaStream *) '\x02' && recorder != NULL) {
      __android_log_print(ANDROID_LOG_VERBOSE, "MMDAgent", "Pa_StartStream 02");
      SLRecordItf recorderInterface;
      result = (*recorder)->GetInterface(recorder, SL_IID_RECORD, &recorderInterface);
      if (result != SL_RESULT_SUCCESS) {
         Pa_Terminate();
         return paInternalError;
      }
      result = (*recorderInterface)->SetRecordState(recorderInterface, SL_RECORDSTATE_RECORDING);
      if (result != SL_RESULT_SUCCESS) {
         Pa_Terminate();
         return paInternalError;
      }
   }

   return paNoError;
}

PaError Pa_StopStream(PaStream *stream)
{
   PaError result;

   if(stream == (PaStream *) '\x01' && player != NULL) {
      __android_log_print(ANDROID_LOG_VERBOSE, "MMDAgent", "Pa_StopStream 01");
      SLPlayItf playerInterface;
      result = (*player)->GetInterface(player, SL_IID_PLAY, &playerInterface);
      if (result != SL_RESULT_SUCCESS) {
         Pa_Terminate();
         return paInternalError;
      }
      result = (*playerInterface)->SetPlayState(playerInterface, SL_PLAYSTATE_STOPPED);
      if (result != SL_RESULT_SUCCESS) {
         Pa_Terminate();
         return paInternalError;
      }
   }

   if(stream == (PaStream *) '\x02' && recorder != NULL) {
      SLRecordItf recorderInterface;
      __android_log_print(ANDROID_LOG_VERBOSE, "MMDAgent", "Pa_StopStream 02");
      result = (*recorder)->GetInterface(recorder, SL_IID_RECORD, &recorderInterface);
      if (result != SL_RESULT_SUCCESS) {
         Pa_Terminate();
         return paInternalError;
      }
      result = (*recorderInterface)->SetRecordState(recorderInterface, SL_RECORDSTATE_STOPPED);
      if (result != SL_RESULT_SUCCESS) {
         Pa_Terminate();
         return paInternalError;
      }
   }

   return paNoError;
}

PaError Pa_AbortStream(PaStream *stream)
{
   return Pa_CloseStream(stream);
}

const PaHostApiInfo *Pa_GetHostApiInfo(PaHostApiIndex hostApi)
{
   return &paHostApiInfo;
}

const PaDeviceInfo* Pa_GetDeviceInfo(PaDeviceIndex device)
{
   return &deviceInfo;
}

const PaStreamInfo* Pa_GetStreamInfo(PaStream *stream)
{
   return &paStreamInfo;
}

const char *Pa_GetErrorText(PaError errorCode)
{
   return "";
}

PaDeviceIndex Pa_GetDeviceCount()
{
   return 0;
}

PaDeviceIndex Pa_GetDefaultInputDevice()
{
   return 0;
}

PaDeviceIndex Pa_GetDefaultOutputDevice()
{
   return 0;
}

const char *Pa_AndroidGetApiName()
{
   return apiName;
}
