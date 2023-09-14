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

#include "ScreenEncoder.h"

/* definitions */
/* image file for "no camera" */
#define ERROR_IMAGE_FILE "nocamera.png"

/* audio bar height */
#define AUDIO_BAR_HEIGHT 30

#ifdef USE_PBO_READPIXEL
/* least image height to be captured */
#define CAPTURE_TARGET_DEFAULT_HEIGHT 640
#endif /* USE_PBO_READPIXEL */

/* macros */
#define ROUNDPIXEL(A, R) (((int)(A) / R) * R)

/* ScreenEncoder::initialize: initialize */
void ScreenEncoder::initialize()
{
   m_mutex = NULL;
   m_codecContext = NULL;
   m_baseWidth = 0;
   m_baseHeight = 0;
   m_zoomRate = 1;
#ifdef USE_PBO_READPIXEL
   m_screenWidth = 0;
   m_screenHeight = 0;
   m_screenStep = 1;
#endif /* USE_PBO_READPIXEL */
   m_captureWidth = 0;
   m_captureHeight = 0;
   m_cameraWidth = 0;
   m_cameraHeight = 0;
   m_imageWidth = 0;
   m_imageHeight = 0;
   m_videoWidth = 0;
   m_videoHeight = 0;
#ifdef USE_PBO_READPIXEL
   m_pbo = 0;
   m_pboInitialized = false;
#endif /* USE_PBO_READPIXEL */
   m_pixelsCapture = NULL;
   m_pixelsImage = NULL;
   m_swsContext = NULL;
   m_srcStride = 0;
   m_frameCounter = 0;
   m_mutexCapture = NULL;
   m_websocket = NULL;
   m_sendFrameRunning = false;
   m_mutexSend = NULL;
   m_cond = NULL;
   m_sendThreadId = -1;
   m_issueFrameFlag = false;
   m_isReady = false;
   m_enableCamera = false;
   m_cameraId = -1;
   m_cap = NULL;
   m_cameraThreadId = -1;
   m_cameraCaptureRunning = false;
   m_noCameraData = NULL;
   m_avatarMaxVol = 0;
   m_cameraMaxVol = 0;
}

/* ScreenEncoder::clear: clear */
void ScreenEncoder::clear()
{
   stop();
   if (m_cameraThreadId >= 0) {
      m_cameraCaptureRunning = false;
      glfwWaitThread(m_cameraThreadId, GLFW_WAIT);
      glfwDestroyThread(m_cameraThreadId);
   }
   if (m_cond)
      glfwDestroyCond(m_cond);
   if (m_mutexSend)
      glfwDestroyMutex(m_mutexSend);
   if (m_mutexCapture)
      glfwLockMutex(m_mutexCapture);
   if (m_codecContext) {
      flushEncoder();
      avcodec_free_context(&m_codecContext);
   }
   if (m_noCameraData)
      free(m_noCameraData);
   if (m_pixelsCapture)
      free(m_pixelsCapture);
   if (m_pixelsImage)
      free(m_pixelsImage);
   if (m_swsContext)
      sws_freeContext(m_swsContext);
   if (m_mutexCapture)
      glfwUnlockMutex(m_mutexCapture);
   if (m_mutexCapture != NULL)
      glfwDestroyMutex(m_mutexCapture);
   if (m_cap) {
      m_cap->release();
   }
#ifdef USE_PBO_READPIXEL
   if (m_pboInitialized) {
      glDeleteBuffers(1, &m_pbo);
   }
#endif /* USE_PBO_READPIXEL */
   initialize();
}

/* constructor */
ScreenEncoder::ScreenEncoder()
{
   initialize();
}

/* destructor */
ScreenEncoder::~ScreenEncoder()
{
   clear();
}

#ifdef USE_PBO_READPIXEL

/* ScreenEncoder::setCaptureSize: set capture size */
bool ScreenEncoder::setCaptureSize(int rawWidth, int rawHeight)
{
   int screenWidth = ROUNDPIXEL(rawWidth, 16);
   int screenHeight = ROUNDPIXEL(rawHeight, 2);

   if (m_screenWidth == screenWidth && m_screenHeight == screenHeight)
      return false;

   m_screenWidth = screenWidth;
   m_screenHeight = screenHeight;

   /* find capture size: minumum integer scale larger than video size */
   int targetHeight;
   int h;
   int n;

   targetHeight = m_baseHeight;
   if (targetHeight < CAPTURE_TARGET_DEFAULT_HEIGHT)
      targetHeight = CAPTURE_TARGET_DEFAULT_HEIGHT;
   if (m_enableCamera)
      if (targetHeight < m_cameraHeight)
         targetHeight = m_cameraHeight;

   for (n = 1; n <= 10; n++) {
      h = screenHeight / n;
      if (h < targetHeight)
         break;
   }
   n--;
   if (n <= 0)
      n = 1;
   m_captureWidth = ROUNDPIXEL(screenWidth / n, 16);
   m_captureHeight = ROUNDPIXEL(screenHeight / n, 2);
   m_screenStep = n;

   return true;
}

#endif /* USE_PBO_READPIXEL */

/* ScreenEncoder::setupCodec: setup codec */
bool ScreenEncoder::setupCodec()
{
   AVCodecContext *codecContext;

   // swap to local to avoid other thread writing while updating
   codecContext = m_codecContext;
   m_codecContext = NULL;

   /* clear current context */
   if (codecContext)
      avcodec_free_context(&codecContext);

   /* calculate input image size */
   /* in H264 codec, width and heights should be rounded */
   if (m_enableCamera) {
      m_imageWidth = ROUNDPIXEL((m_captureWidth + m_cameraWidth * m_zoomRate), 16);
      if (m_captureHeight > m_cameraHeight * m_zoomRate)
         m_imageHeight = ROUNDPIXEL(m_captureHeight, 2);
      else
         m_imageHeight = ROUNDPIXEL(m_cameraHeight * m_zoomRate, 2);
   } else {
      m_imageWidth = m_captureWidth;
      m_imageHeight = m_captureHeight;
   }

   /* add height to hold volume bar */
   m_imageHeight = ROUNDPIXEL(m_imageHeight + AUDIO_BAR_HEIGHT, 2);

   /* calculate output video size, keeping aspect from input and also keeps image size */
   float ratio = (float)m_imageHeight / m_imageWidth;
   int w = (int)(sqrtf((m_baseWidth * m_baseHeight) / ratio));
   m_videoWidth = ROUNDPIXEL(w, 16);
   m_videoHeight = ROUNDPIXEL(m_videoWidth * ratio, 2);
   
   /* find H264 encoder */
   const AVCodec *codec = avcodec_find_encoder(AV_CODEC_ID_H264);
   if (!codec) {
      m_mmdagent->sendLogString(m_id, MLOG_ERROR, "H264 codec not found");
      return false;
   }

   /* make context and set up encoder */
   codecContext = avcodec_alloc_context3(codec);
   if (!codecContext) {
      m_mmdagent->sendLogString(m_id, MLOG_ERROR, "Could not allocate video codec context");
      return false;
   }

   codecContext->bit_rate = m_bitrate;
   codecContext->width = m_videoWidth;
   codecContext->height = m_videoHeight;
   codecContext->time_base = { 1, m_fps };
   codecContext->gop_size = 10;
   codecContext->max_b_frames = 1;
   codecContext->pix_fmt = AV_PIX_FMT_YUV420P;
   codecContext->codec_type = AVMEDIA_TYPE_VIDEO;
   codecContext->thread_count = 1;

   /* set encoder options */
   av_opt_set(codecContext->priv_data, "preset", "ultrafast", 0);
   av_opt_set(codecContext->priv_data, "tune", "zerolatency", 0);

   /* open encoder */
   if (avcodec_open2(codecContext, codec, nullptr) < 0) {
      m_mmdagent->sendLogString(m_id, MLOG_ERROR, "Could not open codec");
      avcodec_free_context(&codecContext);
      return false;
   }

   /* prepare pixel buffers */
   GLubyte *buf;
   buf = m_pixelsCapture;
   m_pixelsCapture = (GLubyte *)malloc(m_captureWidth * m_captureHeight * 3);
   if (buf)
      free(buf);
   buf = m_pixelsImage;
   m_pixelsImage = (GLubyte *)malloc(m_imageWidth * m_imageHeight * 3);
   if (buf)
      free(buf);      

   /* prepare scaling context */
   if (m_swsContext)
      sws_freeContext(m_swsContext);
   m_swsContext = sws_getContext(m_imageWidth, m_imageHeight, AV_PIX_FMT_RGB24,
      m_videoWidth, m_videoHeight, AV_PIX_FMT_YUV420P,
      SWS_BILINEAR, NULL, NULL, NULL);
   m_srcStride = av_image_get_linesize(AV_PIX_FMT_RGB24, m_imageWidth, 0);

   /* set created context to instance */
   m_codecContext = codecContext;

   return true;
}

/* captureCameraMain: main thread of camera capturing */
static void captureCameraMain(void *param)
{
   ScreenEncoder *encoder = (ScreenEncoder *)param;
   encoder->captureCameraRun();
}

/* ScreenEncoder::captureCameraRun: capture camera loop */
void ScreenEncoder::captureCameraRun()
{
   cv::Mat localFrame;

   if (m_codecContext == NULL)
      return;
   if (m_cameraWidth == 0 || m_cameraHeight == 0)
      return;
   if (m_pixelsImage == NULL)
      return;
   if (m_cap == NULL)
      return;

   while (m_cameraCaptureRunning) {
      if (m_cap == NULL)
         break;
      if ((*m_cap).read(localFrame) == true) {
         glfwLockMutex(m_mutexCapture);
         m_cameraFrame = localFrame.clone();
         glfwUnlockMutex(m_mutexCapture);
      }
      else {
         m_mmdagent->sendLogString(m_id, MLOG_ERROR, "err frame");
      }
   }
}

/* ScreenEncoder::setup: set up */
bool ScreenEncoder::setup(MMDAgent *mmdagent, int mid, int cameraId, int bitRate, int fps, int width, int height, int zoom, Poco::Net::WebSocket *websocket, std::mutex *mutex)
{
   char buff[MMDAGENT_MAXBUFLEN];
   int w, h;
   int ret;

   m_mutex = mutex;
   m_mmdagent = mmdagent;
   m_id = mid;
   m_cameraId = cameraId;
   m_bitrate = bitRate;
   m_fps = fps;
   m_baseWidth = width;
   m_baseHeight = height;
   m_zoomRate = zoom;

   m_websocket = websocket;

   if (m_cameraId >= 0)
      m_enableCamera = true;
   else
      m_enableCamera = false;

   m_mutexCapture = glfwCreateMutex();
   m_mutexSend = glfwCreateMutex();

   if (m_noCameraData)
      free(m_noCameraData);
   m_noCameraData = NULL;

   if (m_enableCamera) {
      /* try to open camera */
      m_cap = new cv::VideoCapture(m_cameraId);
      if (!m_cap->isOpened()) {
         /* failed to open */
         m_cap->release();
         m_cap = NULL;
      }
      if (m_cap) {
         /* read one frame to get its size */
         *m_cap >> m_cameraFrame;
         m_cameraWidth = m_cameraFrame.cols;
         m_cameraHeight = m_cameraFrame.rows;
         if (m_cameraWidth == 0 || m_cameraHeight == 0) {
            /* failed to get frame */
            m_cap->release();
            m_cap = NULL;
         }
      }
      if (m_cap == NULL) {
         /* fill in the frame with a image from file */
         MMDAgent_snprintf(buff, MMDAGENT_MAXBUFLEN, "%s%c%s", m_mmdagent->getAppDirName(), MMDAGENT_DIRSEPARATOR, ERROR_IMAGE_FILE);
         if (MMDAgent_exist(buff)) {
            PMDTexture *tex = new PMDTexture();
            if (tex->loadPNG(buff) == true) {
               m_cameraWidth = tex->getWidth();
               m_cameraHeight = tex->getHeight();
               m_noCameraData = (unsigned char *)malloc(tex->getDataLength());
               memcpy(m_noCameraData, tex->getData(), tex->getDataLength());
            }
         }
      }
   }

   /* get initial capture size */
   m_mmdagent->getWindowSize(&w, &h);
   setCaptureSize(w, h);

   /* set up codec */
   ret = setupCodec();

   if (m_enableCamera && m_cap) {
      /* start camera capturing thread when camera has been opened */
      m_cameraCaptureRunning = true;
      m_cameraThreadId = glfwCreateThread(captureCameraMain, this);
      if (m_cameraThreadId == -1) {
         m_mmdagent->sendLogString(m_id, MLOG_ERROR, "failed to create thread for camera capture, disabled");
         m_cameraWidth = m_cameraHeight = 0;
         m_cameraCaptureRunning = false;
         m_cap->release();
      }
   }

   return ret;
}

/* ScreenEncoder::flipImage: flip image */
void ScreenEncoder::flipImage(GLubyte *pixels, int width, int height)
{
   int bytes_per_pixel = 3;
   GLubyte *temp_row = new GLubyte[width * bytes_per_pixel];

   for (int y = 0; y < height / 2; y++) {
      memcpy(temp_row, pixels + y * width * bytes_per_pixel, width * bytes_per_pixel);
      memcpy(pixels + y * width * bytes_per_pixel, pixels + (height - y - 1) * width * bytes_per_pixel, width * bytes_per_pixel);
      memcpy(pixels + (height - y - 1) * width * bytes_per_pixel, temp_row, width * bytes_per_pixel);
   }

   delete[] temp_row;
}

/* ScreenEncoder::captureFrame: capture screen and store to buffer */
bool ScreenEncoder::captureFrame()
{
#ifdef USE_PBO_READPIXEL
   bool screenChanged = false;
#endif /* USE_PBO_READPIXEL */

   if (m_codecContext == NULL)
      return false;

   /* if window size has been changed, set up again */
   int w, h;
   m_mmdagent->getWindowSize(&w, &h);
#ifdef USE_PBO_READPIXEL
   if (setCaptureSize(w, h) == true) {
      screenChanged = true;
      if (setupCodec() == false)
         return false;
   }
#else
   w = ROUNDPIXEL(w, 16);
   h = ROUNDPIXEL(h, 2);
   if (w != m_captureWidth || h != m_captureHeight) {
      m_captureWidth = w;
      m_captureHeight = h;
      if (setupCodec() == false)
         return false;
   }
#endif /* USE_PBO_READPIXEL */

   /* capture window image to pixel buffer */
#ifdef USE_PBO_READPIXEL
   if (m_pboInitialized == false) {
      glGenBuffers(1, &m_pbo);
      glBindBuffer(GL_PIXEL_PACK_BUFFER, m_pbo);
      glBufferData(GL_PIXEL_PACK_BUFFER, 3 * m_screenWidth * m_screenHeight, 0, GL_STREAM_READ);
      glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
      m_pboInitialized = true;
   } else if (screenChanged) {
      glBindBuffer(GL_PIXEL_PACK_BUFFER, m_pbo);
      glBufferData(GL_PIXEL_PACK_BUFFER, 3 * m_screenWidth * m_screenHeight, 0, GL_STREAM_READ);
      glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
   }
   glBindBuffer(GL_PIXEL_PACK_BUFFER, m_pbo);
   glReadPixels(0, 0, m_screenWidth, m_screenHeight, GL_RGB, GL_UNSIGNED_BYTE, 0);
   GLubyte *ptr = (GLubyte*)glMapBuffer(GL_PIXEL_PACK_BUFFER, GL_READ_ONLY);
   GLubyte *dst = m_pixelsCapture;
   GLubyte *src;
   for (int y = 0; y < m_captureHeight; y++) {
      src = ptr + y * m_screenStep * m_screenWidth * 3;
      for (int x = 0; x < m_captureWidth; x++) {
         memcpy(dst, src, 3);
         dst += 3;
         src += m_screenStep * 3;
      }
   }
   flipImage(m_pixelsCapture, m_captureWidth, m_captureHeight);
   glUnmapBuffer(GL_PIXEL_PACK_BUFFER);
   glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
#else
   glfwLockMutex(m_mutexCapture);
   glReadPixels(0, 0, m_captureWidth, m_captureHeight, GL_RGB, GL_UNSIGNED_BYTE, m_pixelsCapture);
   /* opengl buffer has up-side down pixels, flip them */
   flipImage(m_pixelsCapture, m_captureWidth, m_captureHeight);
   glfwUnlockMutex(m_mutexCapture);
#endif /* USE_PBO_READPIXEL */

   return true;
}

// ScreenEncoder::sendFrame: send a frame
void ScreenEncoder::sendFrame()
{
   if (m_codecContext == NULL)
      return;

   if (m_websocket == NULL)
      return;

   AVFrame* frame;

   /* create frame to be sent */
   frame = av_frame_alloc();
   frame->format = m_codecContext->pix_fmt;
   frame->width = m_codecContext->width;
   frame->height = m_codecContext->height;
   av_frame_get_buffer(frame, 0);

   /* set image pixels */
   unsigned int dst;
   memset(m_pixelsImage, 0, m_imageWidth * m_imageHeight * 3);
   for (int y = 0; y < m_imageHeight; y++) {
      dst = y * m_imageWidth * 3;
      if (y < m_captureHeight) {
         memcpy(m_pixelsImage + dst, m_pixelsCapture + y * m_captureWidth * 3, m_captureWidth * 3);
      } else if (y < m_captureHeight + AUDIO_BAR_HEIGHT) {
         int len = int(m_avatarMaxVol * m_captureWidth);
         unsigned char rgb[3] = { 255,128,0 };
         for (int x = 0; x < len; x++)
            memcpy(m_pixelsImage + dst + x * 3, rgb, 3);
      }
      if (m_enableCamera && y < m_cameraHeight * m_zoomRate) {
         dst += m_captureWidth * 3;
         if (m_noCameraData) {
            memcpy(m_pixelsImage + dst, m_noCameraData + (y * m_cameraWidth) * 3, m_cameraWidth * 3);
         } else {
            int yindex = y / m_zoomRate;
            glfwLockMutex(m_mutexCapture);
            for (int x = 0; x < m_cameraWidth; x++) {
               cv::Vec3b bgr = m_cameraFrame.at<cv::Vec3b>(yindex, x);
               for (int k = 0; k < m_zoomRate; k++) *(m_pixelsImage + dst + k) = bgr[2];
               dst += m_zoomRate;
               for (int k = 0; k < m_zoomRate; k++) *(m_pixelsImage + dst + k) = bgr[1];
               dst += m_zoomRate;
               for (int k = 0; k < m_zoomRate; k++) *(m_pixelsImage + dst + k) = bgr[0];
               dst += m_zoomRate;
            }
            glfwUnlockMutex(m_mutexCapture);
         }
      } else if (m_enableCamera && y < m_cameraHeight * m_zoomRate + AUDIO_BAR_HEIGHT && m_noCameraData == NULL) {
         dst += m_captureWidth * 3;
         int len = int(m_cameraMaxVol * m_cameraWidth * m_zoomRate);
         unsigned char rgb[3] = { 0,0, 240 };
         for (int x = 0; x < len; x++)
            memcpy(m_pixelsImage + dst + x * 3, rgb, 3);
      }
   }
   /* copy pixels from pixel buffer with scaling */
   sws_scale(m_swsContext, &m_pixelsImage, &m_srcStride,
      0, m_imageHeight, frame->data, frame->linesize);

   /* set key frame info */
   if (m_frameCounter % m_codecContext->gop_size == 0) {
      frame->pict_type = AV_PICTURE_TYPE_I;
      frame->key_frame = 1;
   } else {
      frame->pict_type = AV_PICTURE_TYPE_NONE;
   }

   /* prepare packet */
   AVPacket* pkt = av_packet_alloc();

   /* send frame to encoder */
   int ret = avcodec_send_frame(m_codecContext, frame);
   if (ret < 0) {
      m_mmdagent->sendLogString(m_id, MLOG_ERROR, "Error sending the frame to the encoder");
      return;
   }
   /* receive resulting packets and send them in turn */
   while (ret >= 0) {
      ret = avcodec_receive_packet(m_codecContext, pkt);
      if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
         break;
      } else if (ret < 0) {
         m_mmdagent->sendLogString(m_id, MLOG_ERROR, "Error during encoding");
         return;
      }

      if (pkt->size > 0) {
         /* send data with header */
         std::vector<unsigned char> packetData(pkt->data, pkt->data + pkt->size);
         uint32_t pkt_flags = htonl(pkt->flags);
         std::string header;
         if (pkt_flags != 0)
            header = "V_KEY:";
         else
            header = "VIDEO:";
         std::vector<unsigned char> payload(header.begin(), header.end());
         payload.insert(payload.end(), packetData.begin(), packetData.end());
         std::lock_guard<std::mutex> lock(*m_mutex);
         m_websocket->sendFrame(payload.data(), payload.size(), Poco::Net::WebSocket::FRAME_BINARY);
      }

      av_packet_unref(pkt);
   }

   av_packet_free(&pkt);
   av_frame_free(&frame);
   m_frameCounter++;
}

/* ScreenEncoder::flushEncoder: flush encoder data */
void ScreenEncoder::flushEncoder()
{
   AVPacket* pkt = av_packet_alloc();
   avcodec_send_frame(m_codecContext, NULL);
   while (avcodec_receive_packet(m_codecContext, pkt) == 0)
      av_packet_unref(pkt);
   av_packet_free(&pkt);
}

static void sendFrameMain(void *param)
{
   ScreenEncoder *encoder = (ScreenEncoder *)param;
   encoder->sendFrameRun();
}

/* ScreenEncoder::sendFrameRun: send frame loop */
void ScreenEncoder::sendFrameRun()
{
   if (m_codecContext == NULL)
      return;
   if (m_websocket == NULL)
      return;
   m_cond = glfwCreateCond();
   if (m_cond == NULL) {
      m_mmdagent->sendLogString(m_id, MLOG_ERROR, "failed to create thread for frame sending");
      m_mmdagent->sendMessage(m_id, PLUGIN_EVENT_SCREENENCODE_STOP, NULL);
      return;
   }

   m_mmdagent->sendMessage(m_id, PLUGIN_EVENT_SCREENENCODE_START, NULL);
   while (m_sendFrameRunning) {
      if (m_websocket == NULL)
         break;
      m_isReady = true;
      glfwLockMutex(m_mutexSend);
      glfwWaitCond(m_cond, m_mutexSend, GLFW_INFINITY);
      glfwUnlockMutex(m_mutexSend);
      if (m_sendFrameRunning == false)
         break;
      sendFrame();
      m_issueFrameFlag = false;
   }
   m_mmdagent->sendMessage(m_id, PLUGIN_EVENT_SCREENENCODE_STOP, NULL);
}

/* ScreenEncoder::start: start thread */
bool ScreenEncoder::start()
{
   m_sendFrameRunning = true;
   m_sendThreadId = glfwCreateThread(sendFrameMain, this);
   if (m_sendThreadId == -1) {
      m_mmdagent->sendLogString(m_id, MLOG_ERROR, "failed to create thread for frame sending");
      m_sendFrameRunning = false;
      return false;
   }
   return true;
}

/* ScreenEncoder::stop: stop thread */
void ScreenEncoder::stop()
{
   if (m_sendThreadId >= 0) {
      m_sendFrameRunning = false;
      if (m_cond)
         glfwSignalCond(m_cond);
      glfwWaitThread(m_sendThreadId, GLFW_WAIT);
      glfwDestroyThread(m_sendThreadId);
      m_sendThreadId = -1;
      m_cond = NULL;
      m_isReady = false;
   }
}

/* ScreenEncoder::setIssueFrameFlag: set issue frame flag */
void ScreenEncoder::setIssueFrameFlag(bool value)
{
   if (value) {
      if (m_cond) {
         m_issueFrameFlag = value;
         glfwSignalCond(m_cond);
      }
   } else {
      m_issueFrameFlag = value;
   }
}

/* ScreenEncoder::getIssueFrameFlag: get issue frame flag */
bool ScreenEncoder::getIssueFrameFlag()
{
   return m_issueFrameFlag;
}

/* ScreenEncoder::isCameraCapturing: return true when camera is capturing */
bool ScreenEncoder::isCameraCapturing()
{
   if (m_enableCamera && m_cap)
      return true;
   return false;
}

/* ScreenEncoder::setAudioVolumes: set avatar audio volumes */
void ScreenEncoder::setAudioVolumes(float volume_remote, float volume_camera)
{
   m_avatarMaxVol = volume_remote;
   m_cameraMaxVol = volume_camera;
}

/* ScreenEncoder::isReady: return true when ready */
bool ScreenEncoder::isReady()
{
   return m_isReady;
}
