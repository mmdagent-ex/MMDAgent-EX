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

/* headers */

#include "MMDAgent.h"

#define POCO_NO_AUTOMATIC_LIBS
#ifdef _WIN32
/* use static library built with MMDAgent-EX */
#define POCO_STATIC
#endif
#include <iostream>
#include <Poco/Net/WebSocket.h>
#include <Poco/Net/HTTPClientSession.h>
#include <Poco/Net/HTTPRequest.h>
#include <Poco/Net/HTTPResponse.h>
#include <Poco/Net/Socket.h>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavutil/channel_layout.h>
#include "portaudio.h"
}

#include "AudioEncoder.h"

/* AudioEncoder::initialize: initialize */
void AudioEncoder::initialize()
{
   m_mmdagent = NULL;
   m_id = 0;
   m_samplingRate = 16000;
   m_stream = NULL;
   m_data.codec = NULL;
   m_data.codec_context = NULL;
   m_data.frame = NULL;
   m_data.packet = NULL;
}

/* AudioEncoder::clear: clear */
void AudioEncoder::clear()
{
   stop();
   initialize();
}

/* constructor */
AudioEncoder::AudioEncoder()
{
   initialize();
}

/* destructor */
AudioEncoder::~AudioEncoder()
{
   clear();
}

/* audio capture callback for Portaudio */
int paCallback(const void *inputBuffer, void *outputBuffer, unsigned long framesPerBuffer, const PaStreamCallbackTimeInfo *timeInfo, PaStreamCallbackFlags statusFlags, void *userData)
{
   auto *encoder = (AudioEncoderData *)userData;

   /* get max */
   short *spbuf = (short *)inputBuffer;
   for (int i = 0; i < framesPerBuffer; i++) {
      int v = spbuf[i] > 0 ? spbuf[i] : -spbuf[i];
      if (encoder->max_vol < v) encoder->max_vol = v;
   }

   /* get framesPerBuffer samples from input buffer */
#if 1
   size_t buffer_size = framesPerBuffer * av_get_bytes_per_sample(encoder->codec_context->sample_fmt) * 1;
   memcpy(encoder->frame->data[0], inputBuffer, buffer_size);
#else
   memcpy(encoder->frame->data[0], inputBuffer, framesPerBuffer * sizeof(int16_t));
#endif
   encoder->frame->nb_samples = framesPerBuffer;
   /* get presentation time stamp */
   encoder->frame->pts += av_rescale_q(encoder->frame->nb_samples, { 1, encoder->codec_context->sample_rate }, encoder->codec_context->time_base);
   /* send obtained samples to encoder */
   int ret = avcodec_send_frame(encoder->codec_context, encoder->frame);
   if (ret < 0) {
      std::cerr << "Error sending the frame to the encoder: " << ret << std::endl;
      return paContinue;
   }
   /* get processed packet from encoder */

   while ((ret = avcodec_receive_packet(encoder->codec_context, encoder->packet)) >= 0) {
      /* send the encoded packet via WebSocket */
      auto* ws = (Poco::Net::WebSocket*)encoder->socket_ptr;
      std::vector<unsigned char> packetData(encoder->packet->data, encoder->packet->data + encoder->packet->size);
      std::string header = "AUDIO:";
      std::vector<unsigned char> payload(header.begin(), header.end());
      payload.insert(payload.end(), packetData.begin(), packetData.end());
      {
         std::lock_guard<std::mutex> lock(*(encoder->mutex));
         ws->sendFrame(payload.data(), payload.size(), Poco::Net::WebSocket::FRAME_BINARY);
      }
      av_packet_unref(encoder->packet);
   }
   if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
      return paContinue;
   } else if (ret < 0) {
      std::cerr << "Error receiving the packet from the encoder: " << ret << std::endl;
      return paContinue;
   }

   return paContinue;
}


/* AudioEncoder::start: start */
bool AudioEncoder::start(MMDAgent *mmdagent, int mid, int samplingRate, Poco::Net::WebSocket *websocket, std::mutex *mutex)
{
   m_mmdagent = mmdagent;
   m_id = mid;
   m_samplingRate = samplingRate;

   /* initialize portaudio */
   PaError err = Pa_Initialize();
   if (err != paNoError) {
      std::cerr << "Error initializing PortAudio" << std::endl;
      return false;
   }

   if (m_stream) {
      stop();
      m_stream = NULL;
   }

   /* initialize audio encoder */
   m_data.codec = avcodec_find_encoder(AV_CODEC_ID_OPUS);
   if (!m_data.codec) {
      std::cerr << "Error finding the encoder" << std::endl;
      return false;
   }

   /* allocate codec context */
   m_data.codec_context = avcodec_alloc_context3(m_data.codec);
   if (!m_data.codec_context) {
      std::cerr << "Error allocating the codec context" << std::endl;
      return false;
   }

   /* set up codec */
#ifdef _WIN32
   av_channel_layout_uninit(&m_data.codec_context->ch_layout);
   m_data.codec_context->ch_layout.order = AV_CHANNEL_ORDER_NATIVE;
   m_data.codec_context->ch_layout.nb_channels = 1;
   m_data.codec_context->ch_layout.u.mask = AV_CH_LAYOUT_MONO;
#else
   m_data.codec_context->channels = 1;
   m_data.codec_context->channel_layout = av_get_default_channel_layout(1);
#endif
   m_data.codec_context->sample_rate = m_samplingRate;
   m_data.codec_context->sample_fmt = m_data.codec->sample_fmts[0];
   m_data.codec_context->time_base = { 1, m_samplingRate };
   m_data.codec_context->thread_count = 1;

   /* open codec */
   if (avcodec_open2(m_data.codec_context, m_data.codec, nullptr) < 0) {
      std::cerr << "Error opening the codec" << std::endl;
      return false;
   }

   /* allocate frame */
   m_data.frame = av_frame_alloc();
   if (!m_data.frame) {
      std::cerr << "Error allocating the AVFrame" << std::endl;
      return false;
   }
   m_data.frame->nb_samples = m_data.codec_context->frame_size;
   m_data.frame->format = m_data.codec_context->sample_fmt;

   /* get buffer */
   if (av_frame_get_buffer(m_data.frame, 0) < 0) {
      std::cerr << "Error getting the buffer for the AVFrame" << std::endl;
      return false;
   }

   /* allocate packet */
   m_data.packet = av_packet_alloc();
   if (!m_data.packet) {
      std::cerr << "Error allocating the AVPacket" << std::endl;
      return false;
   }

   /* set the WebSocket pointer */
   m_data.socket_ptr = websocket;
   m_data.mutex = mutex;

   /* reset max vol */
   m_data.max_vol = 0;

   /* open PortAudio stream */
   PaStream *stream;
   PaStreamParameters inputParameters;
   inputParameters.device = Pa_GetDefaultInputDevice();
   inputParameters.channelCount = 1;
   inputParameters.sampleFormat = paInt16;
   inputParameters.suggestedLatency = Pa_GetDeviceInfo(inputParameters.device)->defaultLowInputLatency;
   inputParameters.hostApiSpecificStreamInfo = nullptr;

   err = Pa_OpenStream(
      &stream,
      &inputParameters,
      nullptr, // No output
      m_samplingRate,
      m_data.frame->nb_samples,
      paClipOff,
      paCallback,
      &m_data
   );

   if (err != paNoError) {
      std::cerr << "PortAudio error: " << Pa_GetErrorText(err) << std::endl;
      return 1;
   }

   err = Pa_StartStream(stream);
   if (err != paNoError) {
      std::cerr << "PortAudio error: " << Pa_GetErrorText(err) << std::endl;
      return 1;
   }

   m_stream = stream;

   return true;
}

/* AudioEncoder::stop: stop */
void AudioEncoder::stop()
{
   if (m_stream == NULL)
      return;

   PaStream *stream = m_stream;
   m_stream = NULL;

   PaError err = Pa_StopStream(stream);
   if (err != paNoError) {
      std::cerr << "PortAudio error: " << Pa_GetErrorText(err) << std::endl;
   }

   err = Pa_CloseStream(stream);
   if (err != paNoError) {
      std::cerr << "PortAudio error: " << Pa_GetErrorText(err) << std::endl;
   }

   av_frame_free(&m_data.frame);
   av_packet_free(&m_data.packet);
   avcodec_free_context(&m_data.codec_context);
}

/* AudioEncoder::getMaxVol: get max volume since last call */
int AudioEncoder::getMaxVol()
{
   int v = 0;

   v = m_data.max_vol;
   m_data.max_vol = 0;

   return v;
}
