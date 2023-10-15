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

/* memo */
/* I/O unit: */
/* send to input scope of element 0 (output element) for output, receive from output scope of element 1 (input element) for input */
/* set output scope of element 0 for output format, set input scope of element 1 for input format */

/*

  Description:
  - use kAudioUnitSubType_VoiceProcessingIO for recording
  - use kAudioUnitSubType_RemoteIO for playing
  - maximum number of available unit is defined in MAX_NUM_HANDLERS
  - 16bit sampling

  Assumptions:

  - Audio Session has been initialized before any call, once at application startup
  - PaStreamCallback only works for recording (input reading), not for playing (output writing)
  - Input error status (EOF, overflow, underflow, etc) are not handled to PaStreamCallback
  - Number of channel is fixed to 1 for both recording and playing

 */


/* header */
#include <AudioToolbox/AudioUnitProperties.h>
#include <AudioUnit/AudioUnit.h>
#include "portaudio.h"
#include "unistd.h"
#include <pthread.h>

/*----------------------------------------------------------------------*/
/* maximum buffer length in milliseconds */
#define BUFFER_LENGTH_IN_MSEC 500
/* maximum number of audio handlers at an app */
#define MAX_NUM_HANDLERS 10
/* time to wait while no data is arrived in ms  */
#define WAIT_MS 4

/* structure to hold handler and ring buffer for both input and output */
#define MYAUDIOHANDLERTYPE_RECORD 0
#define MYAUDIOHANDLERTYPE_PLAYBACK 1
typedef struct {
   int type;                 /* type of this handler, either of MYAUDIOHANDLERTYPE_* */
   AudioUnit instance;      /* audio unit instance associated to this handler */
   SInt16 *buffer;           /* shared buffer */
   UInt32 maxlen;            /* maximum length of the buffer */
   UInt32 currentlen;        /* length of current stored data */
   pthread_mutex_t mutex;    /* thread mutex */
   PaStreamCallback *callback; /* stream callback for processing recorded buffer */
   int active;               /* 1 when this handle is active, 0 when inactive */
   int running;              /* 1 when running the unit, 0 when stopped */
    AudioBufferList *audiobuf; /* audio buffer list, work area to handle acquiring recorded data in callback */
} MyAudioHandler;

static MyAudioHandler allHandler[MAX_NUM_HANDLERS];
static int allHandlerNum = 0;

/*----------------------------------------------------------------------*/

/* PortAudio information to pass to client via portaudio API (dummy) */
static PaHostApiInfo paHostApiInfo;
static PaDeviceInfo deviceInfo;
static PaStreamInfo paStreamInfo;

/* library termination function at app exit */
static void TerminateLibrary(void)
{
   for (int i = 0; i < allHandlerNum; i++) {
      Pa_CloseStream((PaStream *)&(allHandler[i]));
   }
}

/* cleanup function at app exit */
static bool terminate_registered = false;

static void RegisterTerminate(void)
{
   if (terminate_registered == false) {
      terminate_registered = true;
      atexit(TerminateLibrary);
   }
}

PaError Pa_Initialize()
{
   memset(&paHostApiInfo, 0, sizeof(PaHostApiInfo));
   memset(&deviceInfo, 0, sizeof(PaDeviceInfo));
   memset(&paStreamInfo, 0, sizeof(PaStreamInfo));
   RegisterTerminate();
   return paNoError;
}

PaError Pa_Terminate()
{
   RegisterTerminate();
   return paNoError;
}

/* ---------------------------------------------------------------------- */
/* input callback for recording */
static int checkBufferListSizeMatch(AudioBufferList *audiobuf, UInt32 numberFrames)
{
   if (audiobuf == NULL)
      return 0;
   if (audiobuf->mBuffers[0].mDataByteSize != numberFrames * sizeof(SInt16))
      return 0;
   return 1;
}

static AudioBufferList *mallocBufferList(UInt32 numberFrames)
{
   AudioBufferList *bufferList = (AudioBufferList *)malloc(sizeof(AudioBufferList));
   bufferList->mNumberBuffers = 1;
   bufferList->mBuffers[0].mNumberChannels = 1;
   bufferList->mBuffers[0].mDataByteSize = numberFrames * sizeof(SInt16);
   bufferList->mBuffers[0].mData = malloc(sizeof(SInt16) * numberFrames);
   return bufferList;
}

static void freeBufferList(AudioBufferList *bufferList)
{
   if (bufferList == NULL)
      return;
   free(bufferList->mBuffers[0].mData);
   free(bufferList);
}

static OSStatus MyRecordingCallback (
    void                        *inRefCon,
    AudioUnitRenderActionFlags  *ioActionFlags,
    const AudioTimeStamp        *inTimeStamp,
    UInt32                      inBusNumber,
    UInt32                      inNumberFrames,
    AudioBufferList             *ioData
)
{
   MyAudioHandler *handle = (MyAudioHandler *)inRefCon;
   OSStatus status;

   if (handle->active == 0) {
      /* resource released on other thread */
      return noErr;
   }

   if (handle->audiobuf != NULL && checkBufferListSizeMatch(handle->audiobuf, inNumberFrames) == 0) {
      freeBufferList(handle->audiobuf);
      handle->audiobuf = NULL;
   }
   if (handle->audiobuf == NULL)
      handle->audiobuf = mallocBufferList(inNumberFrames);

   status = AudioUnitRender(handle->instance, ioActionFlags, inTimeStamp, inBusNumber, inNumberFrames, handle->audiobuf);
   if (status != noErr)
      return status;

   if (handle->callback)
      (*handle->callback)(handle->audiobuf->mBuffers[0].mData, NULL, inNumberFrames, NULL, 0, NULL);

   return noErr;
}

/* render callback for output */
static OSStatus MyRenderCallback (
    void                        *inRefCon,
    AudioUnitRenderActionFlags  *ioActionFlags,
    const AudioTimeStamp        *inTimeStamp,
    UInt32                      inBusNumber,
    UInt32                      inNumberFrames,
    AudioBufferList             *ioData
)
{
   MyAudioHandler *handle = (MyAudioHandler *)inRefCon;
   UInt32 clen;
   UInt32 len;

   pthread_mutex_lock(&(handle->mutex));
   clen = handle->currentlen;
   pthread_mutex_unlock(&(handle->mutex));
   len = inNumberFrames;
   if (len > clen) len = clen;
   if (len < inNumberFrames) {
      memset(ioData->mBuffers[0].mData, 0, sizeof(SInt16) * inNumberFrames);
   }
   if (len > 0) {
      pthread_mutex_lock(&(handle->mutex));
      memcpy(ioData->mBuffers[0].mData, handle->buffer, sizeof(SInt16) * len);
      memmove(handle->buffer, &(handle->buffer[len]), sizeof(SInt16) * (handle->currentlen - len));
      handle->currentlen -= len;
      pthread_mutex_unlock(&(handle->mutex));
   }

   return noErr;
}

/*----------------------------------------------------------------------*/

PaError Pa_OpenStream(PaStream** stream, const PaStreamParameters *inputParameters, const PaStreamParameters *outputParameters, double sampleRate, unsigned long framesPerBuffer, PaStreamFlags streamFlags, PaStreamCallback *streamCallback, void *userData)
{

   if (inputParameters == NULL && outputParameters == NULL)
      return paInternalError;

   /* set up output io unit description */
   OSStatus result;
   AudioComponentDescription ioUnitDescription;
   ioUnitDescription.componentType          = kAudioUnitType_Output;
   if (inputParameters) {
      ioUnitDescription.componentSubType       = kAudioUnitSubType_VoiceProcessingIO;
   } else {
      ioUnitDescription.componentSubType       = kAudioUnitSubType_RemoteIO;
   }
   ioUnitDescription.componentManufacturer  = kAudioUnitManufacturer_Apple;
   ioUnitDescription.componentFlags         = 0;
   ioUnitDescription.componentFlagsMask     = 0;

   /* obtain an audio unit instance according to the unit description */
   AudioComponent foundIoUnitReference = AudioComponentFindNext(NULL, &ioUnitDescription);
   AudioUnit ioUnitInstance;
   AudioComponentInstanceNew(foundIoUnitReference, &ioUnitInstance);

   if (inputParameters) {
      /* enable recording hardware, disable output hardware */
      UInt32 enableIO = 1;
      result = AudioUnitSetProperty(ioUnitInstance,
                                    kAudioOutputUnitProperty_EnableIO,
                                    kAudioUnitScope_Input,
                                    1,
                                    &enableIO,
                                    sizeof(UInt32));
      enableIO = 0;
      result = AudioUnitSetProperty(ioUnitInstance,
                                    kAudioOutputUnitProperty_EnableIO,
                                    kAudioUnitScope_Output,
                                    0,
                                    &enableIO,
                                    sizeof(UInt32));
   } else {
      /* enable playback (bus0's output is enabled by default but ensure) */
      UInt32 enableIO = 1;
      result = AudioUnitSetProperty(ioUnitInstance,
                                    kAudioOutputUnitProperty_EnableIO,
                                    kAudioUnitScope_Output,
                                    0,
                                    &enableIO,
                                    sizeof(UInt32));
   }
   if (result != 0) {
      return paInternalError;
   }

   /* apply audio format to pass to the input of the output bus */
   AudioStreamBasicDescription audioFormat;
   UInt32 size = sizeof(audioFormat);
   audioFormat.mSampleRate = sampleRate;
   audioFormat.mFormatID = kAudioFormatLinearPCM;
   audioFormat.mFormatFlags = kAudioFormatFlagIsSignedInteger | kAudioFormatFlagsNativeEndian | kAudioFormatFlagIsPacked | kAudioFormatFlagIsNonInterleaved;
   audioFormat.mBytesPerPacket = 2;
   audioFormat.mFramesPerPacket = 1;
   audioFormat.mBytesPerFrame = 2;
   audioFormat.mChannelsPerFrame = 1;
   audioFormat.mBitsPerChannel = 16;
   audioFormat.mReserved = 0;
   if (inputParameters) {
      result = AudioUnitSetProperty(ioUnitInstance,
                                    kAudioUnitProperty_StreamFormat,
                                    kAudioUnitScope_Output,
                                    1,
                                    &audioFormat,
                                    size);
   } else {
      result = AudioUnitSetProperty(ioUnitInstance,
                                    kAudioUnitProperty_StreamFormat,
                                    kAudioUnitScope_Input,
                                    0,
                                    &audioFormat,
                                    size);
   }
   if (result != 0) {
      return paInternalError;
   }

   if (inputParameters) {
      // Set the MaximumFramesPerSlice property. This property is used to describe to an audio unit the maximum number
      // of samples it will be asked to produce on any single given call to AudioUnitRender
      UInt32 maxFramesPerSlice = 4096;
      AudioUnitSetProperty(ioUnitInstance,
                           kAudioUnitProperty_MaximumFramesPerSlice,
                           kAudioUnitScope_Global,
                           0,
                           &maxFramesPerSlice,
                           sizeof(UInt32));
      /*
        kAUVoiceIOProperty_BypassVoiceProcessing
        Indicates whether voice processing is bypassed (any nonzero value) or active (a value of 0). Voice processing
        is active by default.

        kAUVoiceIOProperty_VoiceProcessingEnableAGC
        Indicates whether automatic gain control is enabled (any nonzero value) or disabled (a value of 0).
        Automatic gain control is enabled by default.

        kAUVoiceIOProperty_MuteOutput
        Mutes the output of the Voice-Processing I/O unit. Output muting is off (0) by default. To mute the output,
        set this property’s value to 1.
      */
       /* disable voice processing, just enable AGC only */
      UInt32 value = 1;
#if 0
      AudioUnitSetProperty(ioUnitInstance,
                           kAUVoiceIOProperty_BypassVoiceProcessing,
                           kAudioUnitScope_Global,
                           1,
                           &value,
                           sizeof(UInt32));
#endif
   }

   /* assign a new handler */
   MyAudioHandler *handler = NULL;
   for (int i = 0; i < allHandlerNum; i++) {
      if (allHandler[i].active == 0) {
         handler = &(allHandler[i]);
         break;
      }
   }
   if (handler == NULL) {
      if (allHandlerNum >= MAX_NUM_HANDLERS) {
         /* no more handler */
         return paInternalError;
      }
      handler = &(allHandler[allHandlerNum]);
      allHandlerNum++;
   }
   if (inputParameters) {
      handler->type = MYAUDIOHANDLERTYPE_RECORD;
   } else {
      handler->type = MYAUDIOHANDLERTYPE_PLAYBACK;
   }
   handler->instance = ioUnitInstance;
   handler->maxlen = (BUFFER_LENGTH_IN_MSEC * sampleRate) / 1000;
   handler->buffer = (SInt16 *)malloc(sizeof(SInt16) * handler->maxlen);
   handler->currentlen = 0;
   pthread_mutex_init(&(handler->mutex), NULL);
   handler->callback = streamCallback;
   handler->active = 1;
   handler->running = 0;
   handler->audiobuf = 0;

   /* set callback to the unit */
   AURenderCallbackStruct callback;
   callback.inputProcRefCon = handler;
   if (inputParameters) {
      callback.inputProc = MyRecordingCallback;
      result = AudioUnitSetProperty(ioUnitInstance,
                                    kAudioOutputUnitProperty_SetInputCallback,
                                    kAudioUnitScope_Global,
                                    1,
                                    &callback,
                                    sizeof(AURenderCallbackStruct));
   } else {
      callback.inputProc = MyRenderCallback;
      result = AudioUnitSetProperty(ioUnitInstance,
                                    kAudioUnitProperty_SetRenderCallback,
                                    kAudioUnitScope_Global,
                                    0,
                                    &callback,
                                    sizeof(AURenderCallbackStruct));
   }
   if (result != 0) {
      return paInternalError;
   }

   /* initialize unit */
   result = AudioUnitInitialize(handler->instance);
   if (result != 0)
      return paInternalError;

   /* return the handler */
   *stream = (PaStream *)handler;

   return paNoError;
}

PaError Pa_StartStream(PaStream *stream)
{
   MyAudioHandler *handler = (MyAudioHandler *)stream;

   if (stream == NULL)
        return paInternalError;
   if (handler->running == 0) {
      handler->running = 1;
      if (AudioOutputUnitStart(handler->instance) != noErr)
         return paInternalError;
   }

   return paNoError;
}

PaError Pa_StopStream(PaStream *stream)
{
   MyAudioHandler *handler = (MyAudioHandler *)stream;

   if (stream == NULL)
       return paInternalError;
   if (handler->running == 1) {
      handler->running = 0;
      pthread_mutex_unlock(&(handler->mutex));
      if (AudioOutputUnitStop(handler->instance) != noErr)
         return paInternalError;
   }

   return paNoError;
}

PaError Pa_CloseStream(PaStream *stream)
{
   MyAudioHandler *handler = (MyAudioHandler *)stream;
   OSStatus status;
   PaError ret = paNoError;

   if (handler->active == 0) {
      return paNoError;
   }

   pthread_mutex_lock(&(handler->mutex));
   if (handler->active == 0) {
      /* released on other thread while waiting */
      pthread_mutex_unlock(&(handler->mutex));
      return paNoError;
   }
   Pa_StopStream(stream);
   status = AudioUnitUninitialize(handler->instance);
   if (status != noErr) {
      ret = paInternalError;
   }
   status = AudioComponentInstanceDispose(handler->instance);
   if (status != noErr)
      ret = paInternalError;
   if (handler->buffer != NULL) {
      free(handler->buffer);
      handler->buffer = NULL;
   }
   handler->instance = NULL;
   handler->callback = NULL;
   handler->active = 0;
   pthread_mutex_unlock(&(handler->mutex));
   pthread_mutex_destroy(&(handler->mutex));

   return ret;
}

PaError Pa_WriteStream(PaStream *stream, const void *buffer, unsigned long frames)
{
   MyAudioHandler *handler = (MyAudioHandler *)stream;
   unsigned long writelen;
   unsigned long pos;
   unsigned int waitcount = 0;
   SInt16 *audio = (SInt16 *)buffer;

   if (handler->type != MYAUDIOHANDLERTYPE_PLAYBACK)
      return paInternalError;

   pos = 0;
   while (pos < frames) {
      /* obtain maximum number of samples that can be written to buffer */
      writelen = frames - pos;
      pthread_mutex_lock(&(handler->mutex));
      if (writelen > handler->maxlen - handler->currentlen) {
         writelen = handler->maxlen - handler->currentlen;
      }
      pthread_mutex_unlock(&(handler->mutex));
      if (writelen == 0) {
         /* buffer is full now, wait a while and then retry */
         usleep(WAIT_MS * 1000);
         waitcount += WAIT_MS;
      } else {
         /* append samples to buffer */
         pthread_mutex_lock(&(handler->mutex));
         memcpy(&(handler->buffer[handler->currentlen]), &(audio[pos]), sizeof(SInt16) * writelen);
         handler->currentlen += writelen;
         pthread_mutex_unlock(&(handler->mutex));
         pos += writelen;
         waitcount = 0;
      }
      /* fail safe: if waits more than 1 sec, leave here */
      if (waitcount > 1000) break;
   }

   return paNoError;
}

void Pa_Sleep(long msec)
{
   usleep((unsigned int) (msec * 1000));
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
