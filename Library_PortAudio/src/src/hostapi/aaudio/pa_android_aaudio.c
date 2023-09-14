/* ----------------------------------------------------------------- */
/*           The Toolkit for Building Voice Interaction Systems      */
/*           "MMDAgent" developed by MMDAgent Project Team           */
/*           http://www.mmdagent.jp/                                 */
/* ----------------------------------------------------------------- */
/*                                                                   */
/*  Copyright (c) 2009-2019  Nagoya Institute of Technology          */
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
#include "portaudio.h"
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <aaudio/AAudio.h>
#include "android/log.h"

/*----------------------------------------------------------------------*/
/* maximum buffer length in milliseconds */
#define BUFFER_LENGTH_IN_MSEC 1000

/* maximum number of audio handlers at an app */
#define MAX_NUM_HANDLERS 10

/* state wait timeout in milliseconds */
#define STATE_WAIT_TIMEOUT_MSEC 20

/* time to wait while no data is arrived in ms  */
#define WAIT_MS 4

/* structure to hold handler and ring buffer for both input and output */
#define MYAUDIOHANDLERTYPE_RECORD 0
#define MYAUDIOHANDLERTYPE_PLAYBACK 1

typedef struct {
   int type;                 /* type of this handler, either of MYAUDIOHANDLERTYPE_* */
   int active;               /* 1 when this handle is active, 0 when inactive */
   int running;              /* 1 when running the unit, 0 when stopped */
   int channels;
   int16_t *buffer;
   int32_t maxFrame;
   int32_t currentFrame;
   pthread_mutex_t mutex;    /* thread mutex */
   AAudioStream *stream;     /* AAudio stream */
   PaStreamCallback *callback; /* stream callback for processing buffer */
} MyAudioHandler;

static MyAudioHandler allHandler[MAX_NUM_HANDLERS];
static int allHandlerNum = 0;
static pthread_mutex_t handlerMutex;
static int globalInitialized = 0;

/*----------------------------------------------------------------------*/

/* callback function for audio input */
static aaudio_data_callback_result_t myStreamInputCallback(AAudioStream *stream, void *userData, void *audioData, int32_t numFrames)
{
   MyAudioHandler *handle = (MyAudioHandler *)userData;

   /* send captured data to application callback */
   if (handle->callback) {
      (*handle->callback)(audioData, NULL, numFrames, NULL, 0, NULL);
   }

   return AAUDIO_CALLBACK_RESULT_CONTINUE;
}

/* callback function for audio output */
static aaudio_data_callback_result_t myStreamOutputCallback(AAudioStream *stream, void *userData, void *audioData, int32_t numFrames)
{
   MyAudioHandler *handle = (MyAudioHandler *)userData;
   int32_t cframe;
   int c = handle->channels;

   /* wait until required length can be filled by application thread */
   pthread_mutex_lock(&(handle->mutex));
   cframe = handle->currentFrame;
   pthread_mutex_unlock(&(handle->mutex));
   while (cframe < numFrames) {
      usleep(WAIT_MS * 1000);
      if (handle->active == 0) {
         /* resource released on other thread */
         return AAUDIO_CALLBACK_RESULT_STOP;
      }
      if (handle->running == 0) {
         return AAUDIO_CALLBACK_RESULT_CONTINUE;
      }
      pthread_mutex_lock(&(handle->mutex));
      cframe = handle->currentFrame;
      pthread_mutex_unlock(&(handle->mutex));
   }

   pthread_mutex_lock(&(handle->mutex));
   /* transfer the filled data into audio */
   memcpy(audioData, handle->buffer, sizeof(int16_t) * c * numFrames);
   /* cut up the shared buffer */
   memmove(handle->buffer, &(handle->buffer[c * numFrames]), sizeof(int16_t) * c * (handle->currentFrame - numFrames));
   handle->currentFrame -= numFrames;
   pthread_mutex_unlock(&(handle->mutex));

   return AAUDIO_CALLBACK_RESULT_CONTINUE;
}

/*----------------------------------------------------------------------*/
/* PortAudio information to pass to client via portaudio API (dummy) */
static PaHostApiInfo paHostApiInfo;
static PaDeviceInfo deviceInfo;
static PaStreamInfo paStreamInfo;

/* library termination function at app exit */
static void TerminateLibrary(void)
{
   /* make sure all audio device was closed at app exit */
   for (int i = 0; i < allHandlerNum; i++) {
      Pa_CloseStream((PaStream *)&(allHandler[i]));
   }
}

/* cleanup function at app exit */
static int terminate_registered = 0;

static void RegisterTerminate(void)
{
   if (terminate_registered == 0) {
      terminate_registered = 1;
      atexit(TerminateLibrary);
   }
}

PaError Pa_Initialize()
{
   memset(&paHostApiInfo, 0, sizeof(PaHostApiInfo));
   memset(&deviceInfo, 0, sizeof(PaDeviceInfo));
   memset(&paStreamInfo, 0, sizeof(PaStreamInfo));
   RegisterTerminate();
   if (globalInitialized == 0) {
      /* once per app startup */
      globalInitialized = 1;
      pthread_mutex_init(&(handlerMutex), NULL);
      for (int i = 0; i < MAX_NUM_HANDLERS; i++) {
         allHandler[i].active = 0;
         allHandler[i].running = 0;
         allHandler[i].buffer = NULL;
      }
   }
   return paNoError;
}

PaError Pa_Terminate()
{
   RegisterTerminate();
   return paNoError;
}

/*----------------------------------------------------------------------*/

PaError Pa_OpenStream(PaStream** stream, const PaStreamParameters *inputParameters, const PaStreamParameters *outputParameters, double sampleRate, unsigned long framesPerBuffer, PaStreamFlags streamFlags, PaStreamCallback *streamCallback, void *userData)
{
   AAudioStreamBuilder *builder;
   aaudio_result_t result;
   AAudioStream *astream;
   MyAudioHandler *handler = NULL;

   if (inputParameters == NULL && outputParameters == NULL)
      return paInternalError;

   /* create stream buidler */
   result = AAudio_createStreamBuilder(&builder);
   if (result != AAUDIO_OK || builder == NULL) {
      __android_log_print(ANDROID_LOG_ERROR, "MMDAgent", "failed to create stream buidler");
      return paInternalError;
   }

   /* locate an available handler */
   pthread_mutex_lock(&handlerMutex);
   for (int i = 0; i < allHandlerNum; i++) {
      if (allHandler[i].active == 0) {
         handler = &(allHandler[i]);
         break;
      }
   }
   if (handler == NULL) {
      if (allHandlerNum >= MAX_NUM_HANDLERS) {
         /* no more handler */
         AAudioStreamBuilder_delete(builder);
         return paInternalError;
      }
      handler = &(allHandler[allHandlerNum]);
      allHandlerNum++;
   }
   handler->active = 1;
   handler->running = 0;
   pthread_mutex_unlock(&handlerMutex);

   /* configure stream */
   /* device selection: use primary device */
   AAudioStreamBuilder_setDeviceId(builder, AAUDIO_UNSPECIFIED);
   /* direction */
   if (inputParameters) {
      AAudioStreamBuilder_setDirection(builder, AAUDIO_DIRECTION_INPUT);
   } else {
      AAudioStreamBuilder_setDirection(builder, AAUDIO_DIRECTION_OUTPUT);
   }
   /* sample rate */
   AAudioStreamBuilder_setSampleRate(builder, sampleRate);
   /* number of channel */
   if (inputParameters) {
      AAudioStreamBuilder_setChannelCount(builder, inputParameters->channelCount);
      handler->channels = inputParameters->channelCount;
   } else {
      AAudioStreamBuilder_setChannelCount(builder, outputParameters->channelCount);
      handler->channels = outputParameters->channelCount;
   }
   /* format */
   AAudioStreamBuilder_setFormat(builder, AAUDIO_FORMAT_PCM_I16);
   /* try to set low latency performance mode */
   AAudioStreamBuilder_setPerformanceMode(builder, AAUDIO_PERFORMANCE_MODE_LOW_LATENCY);
   /* try to set input preset and content type, supported at API 28 */
   /* this is default when not call this function, so leave it */
   //AAudioStreamBuilder_setInputPreset(builder, AAUDIO_INPUT_PRESET_VOICE_RECOGNITION);
   //if (inputParameters) {
      /* set input audio content type as speech */
   //AAudioStreamBuilder_setContentType(builder, AAUDIO_CONTENT_TYPE_SPEECH);
   //}

   /* set callback */
   if (inputParameters) {
      handler->callback = streamCallback;
      if (streamCallback) {
         AAudioStreamBuilder_setDataCallback(builder, myStreamInputCallback, handler);
      }
   } else {
      AAudioStreamBuilder_setDataCallback(builder, myStreamOutputCallback, handler);
   }

   /* create stream */
   result = AAudioStreamBuilder_openStream(builder, &astream);
   if (result != AAUDIO_OK || astream == NULL) {
      AAudioStreamBuilder_delete(builder);
      __android_log_print(ANDROID_LOG_ERROR, "MMDAgent", "failed to create stream");
      return paInternalError;
   }

   /* check the pamameters of opened stream */
   if (AAudioStream_getFormat(astream) != AAUDIO_FORMAT_PCM_I16) {
      AAudioStreamBuilder_delete(builder);
      __android_log_print(ANDROID_LOG_ERROR, "MMDAgent", "failed to set audio format to 16bit integer");
      return paInternalError;
   }

   /* set up other handler information */
   if (inputParameters) {
      handler->type = MYAUDIOHANDLERTYPE_RECORD;
   } else {
      handler->type = MYAUDIOHANDLERTYPE_PLAYBACK;
   }
   pthread_mutex_init(&(handler->mutex), NULL);
   handler->stream = astream;
   if (outputParameters) {
      handler->maxFrame = (BUFFER_LENGTH_IN_MSEC * sampleRate) / 1000;
      if (handler->buffer) free(handler->buffer);
      handler->buffer = (int16_t *)malloc(sizeof(int16_t) * handler->channels * handler->maxFrame);
      handler->currentFrame = 0;
   }

   /* delete builder */
   AAudioStreamBuilder_delete(builder);

   /* return the handler */
   *stream = (PaStream *)handler;

   return paNoError;
}

PaError Pa_StartStream(PaStream *stream)
{
   MyAudioHandler *handler = (MyAudioHandler *)stream;

   if (stream == NULL)
        return paInternalError;
   if (handler->active == 0)
        return paInternalError;
   if (handler->running == 0) {
      handler->running = 1;
      if (AAudioStream_requestStart(handler->stream) != AAUDIO_OK) {
         return paInternalError;
      }
      aaudio_stream_state_t nextState = AAUDIO_STREAM_STATE_UNINITIALIZED;
      pthread_mutex_lock(&(handler->mutex));
      AAudioStream_waitForStateChange(handler->stream, AAUDIO_STREAM_STATE_STARTING, &nextState, STATE_WAIT_TIMEOUT_MSEC * 1000000);
      pthread_mutex_unlock(&(handler->mutex));
   }

   return paNoError;
}

PaError Pa_StopStream(PaStream *stream)
{
   MyAudioHandler *handler = (MyAudioHandler *)stream;

   if (stream == NULL)
       return paInternalError;
   if (handler->active == 0)
        return paInternalError;
   if (handler->running == 1) {
      handler->running = 0;
      pthread_mutex_unlock(&(handler->mutex));
      aaudio_result_t result;
      if (AAudioStream_requestStop(handler->stream) != AAUDIO_OK) {
         return paInternalError;
      }
      aaudio_stream_state_t nextState = AAUDIO_STREAM_STATE_UNINITIALIZED;
      pthread_mutex_lock(&(handler->mutex));
      AAudioStream_waitForStateChange(handler->stream, AAUDIO_STREAM_STATE_STOPPING, &nextState, STATE_WAIT_TIMEOUT_MSEC * 1000000);
      pthread_mutex_unlock(&(handler->mutex));
   }

   return paNoError;
}

PaError Pa_CloseStream(PaStream *stream)
{
   MyAudioHandler *handler = (MyAudioHandler *)stream;
   PaError ret = paNoError;

   if (stream == NULL)
       return paInternalError;

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
   AAudioStream_close(handler->stream);
   handler->active = 0;
   pthread_mutex_unlock(&(handler->mutex));
   pthread_mutex_destroy(&(handler->mutex));

   return ret;
}

PaError Pa_WriteStream(PaStream *stream, const void *buffer, unsigned long frames)
{
   MyAudioHandler *handler = (MyAudioHandler *)stream;
   int32_t writeFrame;
   int32_t posFrame;
   unsigned int waitcount = 0;
   int16_t *audio = (int16_t *)buffer;
   int c = handler->channels;

   /* In PortAudio declaration, the argument "frames" should be a
      number of frame, but MMDAgent's Plugin_Audio and
      Plugin_Open_JTalk calls Pa_WriteStream with "frames" as number of
      samples, not a number of frame.  This is wrong, but we should
      keep that since the OpenSL/ES implementation can not handle
      multiple output instance with different number of channels.  So
      in this AAudio implementation we convert the number of samples
      given as "frames" to the actual number of frames here */
   frames /= c;

   if (stream == NULL)
       return paInternalError;

   if (handler->type != MYAUDIOHANDLERTYPE_PLAYBACK)
      return paInternalError;

   posFrame = 0;
   while (posFrame < frames) {
      /* obtain maximum number of samples that can be written to buffer */
      writeFrame = frames - posFrame;
      pthread_mutex_lock(&(handler->mutex));
      if (writeFrame > handler->maxFrame - handler->currentFrame) {
         writeFrame = handler->maxFrame - handler->currentFrame;
      }
      pthread_mutex_unlock(&(handler->mutex));
      if (writeFrame == 0) {
         /* buffer is full now, wait a while and then retry */
         usleep(WAIT_MS * 1000);
         waitcount += WAIT_MS;
      } else {
         /* append samples to buffer */
         pthread_mutex_lock(&(handler->mutex));
         memcpy(&(handler->buffer[c * handler->currentFrame]), &(audio[c * posFrame]), sizeof(int16_t) * c * writeFrame);
         handler->currentFrame += writeFrame;
         pthread_mutex_unlock(&(handler->mutex));
         posFrame += writeFrame;
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

void Pa_AndroidSetNativeParam(int sampleRate, int frames)
{
}
