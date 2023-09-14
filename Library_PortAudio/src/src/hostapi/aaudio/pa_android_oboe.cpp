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
#include <oboe/Oboe.h>
#include "android/log.h"


/* since Oboe callback does not allow blocking, define this to feed 0 samples instead of blocking */
#define FILL_IN_ZERO_SAMPLES_WHEN_NO_DATA_IS_AVAILABLE_FOR_PLAYBACK

extern "C" {
   const char *Pa_AndroidGetApiName();
   PaError Pa_Oboe_Initialize();
   PaError Pa_Oboe_Terminate();
   PaError Pa_Oboe_OpenStream(PaStream** stream, const PaStreamParameters *inputParameters, const PaStreamParameters *outputParameters, double sampleRate, unsigned long framesPerBuffer, PaStreamFlags streamFlags, PaStreamCallback *streamCallback, void *userData);
   PaError Pa_Oboe_StartStream(PaStream *stream);
   PaError Pa_Oboe_StopStream(PaStream *stream);
   PaError Pa_Oboe_CloseStream(PaStream *stream);
   PaError Pa_Oboe_WriteStream(PaStream *stream, const void *buffer, unsigned long frames);
   PaError Pa_OpenSLES_Initialize();
   PaError Pa_OpenSLES_Terminate();
   PaError Pa_OpenSLES_OpenStream(PaStream** stream, const PaStreamParameters *inputParameters, const PaStreamParameters *outputParameters, double sampleRate, unsigned long framesPerBuffer, PaStreamFlags streamFlags, PaStreamCallback *streamCallback, void *userData);
   PaError Pa_OpenSLES_StartStream(PaStream *stream);
   PaError Pa_OpenSLES_StopStream(PaStream *stream);
   PaError Pa_OpenSLES_CloseStream(PaStream *stream);
   PaError Pa_OpenSLES_WriteStream(PaStream *stream, const void *buffer, unsigned long frames);
}

/*----------------------------------------------------------------------*/
/* Oboe / OpenSLES switcher:
 * if AAudio is available and recommended, use Oboe.
 * if not, switch to old OpenSL ES implementation
 * This is because Oboe does not work well on multi-threaded duplex
 * handling, which can not be implemented normally on OpenSL ES.
 */
static char *apiName = NULL;
static int use_opensles_initialized = 0;
static int use_opensles = 0;

PaError Pa_Initialize()
{
   if (use_opensles_initialized == 0) {
      use_opensles_initialized = 1;
      oboe::AudioStreamBuilder builder;
      if (builder.isAAudioSupported() && builder.isAAudioRecommended()) {
         /* use AAudio via oboe */
         use_opensles = 0;
      } else {
         /* use old OpenSL ES implementation */
         use_opensles = 1;
         if (apiName != NULL)
            free(apiName);
         apiName = (char *)malloc(9);
         strcpy(apiName, "OpenSLES");
      }
   }
   if (use_opensles == 1) {
      return Pa_OpenSLES_Initialize();
   } else {
      return Pa_Oboe_Initialize();
   }
}

PaError Pa_Terminate()
{
   if (use_opensles == 1) {
      return Pa_OpenSLES_Terminate();
   } else {
      return Pa_Oboe_Terminate();
   }
}

PaError Pa_OpenStream(PaStream** stream, const PaStreamParameters *inputParameters, const PaStreamParameters *outputParameters, double sampleRate, unsigned long framesPerBuffer, PaStreamFlags streamFlags, PaStreamCallback *streamCallback, void *userData)
{
   if (use_opensles == 1) {
      return Pa_OpenSLES_OpenStream(stream, inputParameters, outputParameters, sampleRate, framesPerBuffer, streamFlags, streamCallback, userData);
   } else {
      return Pa_Oboe_OpenStream(stream, inputParameters, outputParameters, sampleRate, framesPerBuffer, streamFlags, streamCallback, userData);
   }
}

PaError Pa_StartStream(PaStream *stream)
{
   if (use_opensles == 1) {
      return Pa_OpenSLES_StartStream(stream);
   } else {
      return Pa_Oboe_StartStream(stream);
   }
}

PaError Pa_StopStream(PaStream *stream)
{
   if (use_opensles == 1) {
      return Pa_OpenSLES_StopStream(stream);
   } else {
      return Pa_Oboe_StopStream(stream);
   }
}

PaError Pa_CloseStream(PaStream *stream)
{
   if (use_opensles == 1) {
      return Pa_OpenSLES_CloseStream(stream);
   } else {
      return Pa_Oboe_CloseStream(stream);
   }
}

PaError Pa_WriteStream(PaStream *stream, const void *buffer, unsigned long frames)
{
   if (use_opensles == 1) {
      return Pa_OpenSLES_WriteStream(stream, buffer, frames);
   } else {
      return Pa_Oboe_WriteStream(stream, buffer, frames);
   }
}


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
   oboe::AudioStream *stream;
   int type;                 /* type of this handler, either of MYAUDIOHANDLERTYPE_* */
   int active;               /* 1 when this handle is active, 0 when inactive */
   int running;              /* 1 when running the unit, 0 when stopped */
   int channels;
   int16_t *buffer;
   int32_t maxFrame;
   int32_t currentFrame;
   pthread_mutex_t mutex;    /* thread mutex */
   pthread_mutex_t mutex2;    /* thread mutex 2 */
   PaStreamCallback *callback; /* stream callback for processing buffer */
} MyAudioHandler;

static MyAudioHandler allHandler[MAX_NUM_HANDLERS];
static int allHandlerNum = 0;
static pthread_mutex_t handlerMutex;
static int globalInitialized = 0;

/*----------------------------------------------------------------------*/

/* callback function for audio input */
class MyInputCallback : public oboe::AudioStreamCallback {

private:

   MyAudioHandler *handle;

public:

   void setHandle(MyAudioHandler *h) {
      handle = h;
   }

   oboe::DataCallbackResult onAudioReady(oboe::AudioStream *stream, void *audioData, int32_t numFrames) {
      /* send captured data to application callback */
      if (handle->callback) {
         (*handle->callback)(audioData, NULL, numFrames, NULL, 0, NULL);
      }

      return oboe::DataCallbackResult::Continue;
   }
};

/* callback function for audio output */
class MyOutputCallback : public oboe::AudioStreamCallback {

private:

   MyAudioHandler *handle;

public:

   void setHandle(MyAudioHandler *h) {
      handle = h;
   }

   oboe::DataCallbackResult onAudioReady(oboe::AudioStream *stream, void *audioData, int32_t numFrames) {
      int32_t cframe;
      int c = handle->channels;

      /* wait until required length can be filled by application thread */
      pthread_mutex_lock(&(handle->mutex));
      cframe = handle->currentFrame;
      pthread_mutex_unlock(&(handle->mutex));
#ifdef FILL_IN_ZERO_SAMPLES_WHEN_NO_DATA_IS_AVAILABLE_FOR_PLAYBACK
      if (cframe < numFrames) {
         memset(audioData, 0, sizeof(int16_t) * c * numFrames);
         return oboe::DataCallbackResult::Continue;
      }
#else
      while (cframe < numFrames) {
         usleep(WAIT_MS * 1000);
         if (handle->active == 0) {
            /* resource released on other thread */
            return oboe::DataCallbackResult::Stop;
         }
         if (handle->running == 0) {
            return oboe::DataCallbackResult::Continue;
         }
         pthread_mutex_lock(&(handle->mutex));
         cframe = handle->currentFrame;
         pthread_mutex_unlock(&(handle->mutex));
      }
#endif

      pthread_mutex_lock(&(handle->mutex));
      /* transfer the filled data into audio */
      memcpy(audioData, handle->buffer, sizeof(int16_t) * c * numFrames);
      /* cut up the shared buffer */
      memmove(handle->buffer, &(handle->buffer[c * numFrames]), sizeof(int16_t) * c * (handle->currentFrame - numFrames));
      handle->currentFrame -= numFrames;
      pthread_mutex_unlock(&(handle->mutex));

      return oboe::DataCallbackResult::Continue;
   }
};

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
      Pa_Oboe_CloseStream((PaStream *)&(allHandler[i]));
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

PaError Pa_Oboe_Initialize()
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

PaError Pa_Oboe_Terminate()
{
   RegisterTerminate();
   return paNoError;
}

/*----------------------------------------------------------------------*/

PaError Pa_Oboe_OpenStream(PaStream** stream, const PaStreamParameters *inputParameters, const PaStreamParameters *outputParameters, double sampleRate, unsigned long framesPerBuffer, PaStreamFlags streamFlags, PaStreamCallback *streamCallback, void *userData)
{
   oboe::AudioStreamBuilder builder;
   MyAudioHandler *handler = NULL;

   if (inputParameters == NULL && outputParameters == NULL)
      return paInternalError;

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
         return paInternalError;
      }
      handler = &(allHandler[allHandlerNum]);
      allHandlerNum++;
   }
   handler->active = 1;
   handler->running = 0;
   pthread_mutex_unlock(&handlerMutex);

   /* configure stream */
   /* direction */
   if (inputParameters) {
      builder.setDirection(oboe::Direction::Input);
   } else {
      builder.setDirection(oboe::Direction::Output);
   }
   /* sample rate */
   builder.setSampleRate(sampleRate);
   /* number of channel */
   if (inputParameters) {
      builder.setChannelCount(inputParameters->channelCount);
      handler->channels = inputParameters->channelCount;
   } else {
      builder.setChannelCount(outputParameters->channelCount);
      handler->channels = outputParameters->channelCount;
   }
   /* format */
   builder.setFormat(oboe::AudioFormat::I16);
   /* try to set low latency performance mode */
   builder.setPerformanceMode(oboe::PerformanceMode::LowLatency);
   /* try to set input preset and content type, supported at API 28 */
   builder.setInputPreset(oboe::InputPreset::VoiceRecognition);
   if (inputParameters) {
      builder.setContentType(oboe::ContentType::Speech);
   }

   /* set up handler information */
   pthread_mutex_init(&(handler->mutex), NULL);
   pthread_mutex_init(&(handler->mutex2), NULL);
   if (inputParameters) {
      handler->type = MYAUDIOHANDLERTYPE_RECORD;
   } else {
      handler->type = MYAUDIOHANDLERTYPE_PLAYBACK;
   }
   if (outputParameters) {
      handler->maxFrame = (BUFFER_LENGTH_IN_MSEC * sampleRate) / 1000;
      if (handler->buffer) free(handler->buffer);
      handler->buffer = (int16_t *)malloc(sizeof(int16_t) * handler->channels * handler->maxFrame);
      handler->currentFrame = 0;
   }

   /* set callback */
   if (inputParameters) {
      handler->callback = streamCallback;
      if (streamCallback) {
         MyInputCallback *mycallback = new MyInputCallback();
         mycallback->setHandle(handler);
         builder.setCallback(mycallback);
      }
   } else {
      MyOutputCallback *mycallback = new MyOutputCallback();
      mycallback->setHandle(handler);
      builder.setCallback(mycallback);
   }

   /* create stream */
   oboe::Result result = builder.openStream(&handler->stream);
   if (result != oboe::Result::OK) {
      __android_log_print(ANDROID_LOG_ERROR, "MMDAgent", "failed to create stream");
      return paInternalError;
   }

   /* check the pamameters of opened stream */
   if (handler->stream->getFormat() != oboe::AudioFormat::I16) {
      __android_log_print(ANDROID_LOG_ERROR, "MMDAgent", "failed to set audio format to 16bit integer");
      return paInternalError;
   }

   /* store api name to device info */
   if (apiName != NULL)
      free(apiName);
   apiName = (char *)malloc(12);
   if (handler->stream->usesAAudio()) {
      strcpy(apiName, "Oboe/AAudio");
   } else {
      strcpy(apiName, "Oboe/OpenSL");
   }

   /* return the handler */
   *stream = (PaStream *)handler;

   return paNoError;
}

PaError Pa_Oboe_StartStream(PaStream *stream)
{
   MyAudioHandler *handler = (MyAudioHandler *)stream;

   if (stream == NULL)
        return paInternalError;
   if (handler->active == 0)
        return paInternalError;
   pthread_mutex_lock(&(handler->mutex2));
   if (handler->running == 0) {
      handler->running = 1;
      if (handler->stream->requestStart() != oboe::Result::OK) {
         return paInternalError;
      }
      oboe::StreamState nextState = oboe::StreamState::Uninitialized;
      handler->stream->waitForStateChange(oboe::StreamState::Starting, &nextState, STATE_WAIT_TIMEOUT_MSEC * 1000000);
   }
   pthread_mutex_unlock(&(handler->mutex2));

   return paNoError;
}

PaError Pa_Oboe_StopStream(PaStream *stream)
{
   MyAudioHandler *handler = (MyAudioHandler *)stream;

   if (stream == NULL)
       return paInternalError;
   if (handler->active == 0)
        return paInternalError;
   pthread_mutex_lock(&(handler->mutex2));
   if (handler->running == 1) {
      handler->running = 0;
      if (handler->stream->requestStop() != oboe::Result::OK) {
         return paInternalError;
      }
      oboe::StreamState nextState = oboe::StreamState::Uninitialized;
      handler->stream->waitForStateChange(oboe::StreamState::Stopping, &nextState, STATE_WAIT_TIMEOUT_MSEC * 1000000);
   }
   pthread_mutex_unlock(&(handler->mutex2));

   return paNoError;
}

PaError Pa_Oboe_CloseStream(PaStream *stream)
{
   MyAudioHandler *handler = (MyAudioHandler *)stream;
   PaError ret = paNoError;

   if (stream == NULL)
       return paInternalError;

   if (handler->active == 0) {
      /* released on other thread while waiting */
      ret = paNoError;
   } else {
      Pa_Oboe_StopStream(stream);
      handler->stream->close();
      handler->active = 0;
   }

   return ret;
}

PaError Pa_Oboe_WriteStream(PaStream *stream, const void *buffer, unsigned long frames)
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

   if (stream == NULL) {
      __android_log_print(ANDROID_LOG_ERROR, "MMDAgent", "PaOboeWriteStream: stream == NULL");
       return paInternalError;
   }

   if (handler->type != MYAUDIOHANDLERTYPE_PLAYBACK) {
      __android_log_print(ANDROID_LOG_ERROR, "MMDAgent", "PaOboeWriteStream: not playback stream");
      return paInternalError;
   }

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

const char *Pa_AndroidGetApiName()
{
   return apiName;
}
