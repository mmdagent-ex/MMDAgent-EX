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
#include "CameraImage.h"

/* CameraImage::initialize: initialize */
void CameraImage::initialize()
{
   m_mmdagent = NULL;
   m_id = 0;

   m_cameraId = -1;
   m_cap = NULL;
   m_cameraWidth = 0;
   m_cameraHeight = 0;

   m_pixelsCapture = NULL;
   m_mirror = true;

   m_cameraThreadId = -1;
   m_cameraThreadRunning = false;
   m_cameraCapturing = false;
   m_cameraCaptureUpdated = false;

   m_textureId = PMDTEXTURE_UNINITIALIZEDID;
}

/* CameraImage::clear: clear */
void CameraImage::clear()
{
   stop();
   if (m_pixelsCapture)
      free(m_pixelsCapture);
   if (m_cap)
      m_cap->release();
   if (m_textureId != PMDTEXTURE_UNINITIALIZEDID)
      glDeleteTextures(1, &m_textureId);
   initialize();
}

/* constructor */
CameraImage::CameraImage()
{
   initialize();
}

/* destructor */
CameraImage::~CameraImage()
{
   clear();
}

/* CameraImage::setup: set up */
void CameraImage::setup(MMDAgent *mmdagent, int mid)
{
   m_mmdagent = mmdagent;
   m_id = mid;
}

/* CameraImage::captureCameraRun: capture camera loop */
void CameraImage::captureCameraRun()
{
   cv::Mat cameraFrame;            /* captured image */

   /* open camera */
   m_cap = new cv::VideoCapture(m_cameraId);
   if (!m_cap->isOpened()) {
      /* failed to open */
      m_mmdagent->sendLogString(m_id, MLOG_ERROR, "failed to open camera #%d for background", m_cameraId);
      m_cap->release();
      m_cap = NULL;
      m_cameraThreadRunning = false;
      m_cameraCapturing = false;
      return;
   }

   /* read one frame to get its size */
   *m_cap >> cameraFrame;
   m_cameraWidth = cameraFrame.cols;
   m_cameraHeight = cameraFrame.rows;
   if (m_cameraWidth == 0 || m_cameraHeight == 0) {
      /* failed to get frame */
      m_mmdagent->sendLogString(m_id, MLOG_ERROR, "failed to obtain image from camera #%d for background", m_cameraId);
      m_cap->release();
      m_cap = NULL;
      m_cameraThreadRunning = false;
      m_cameraCapturing = false;
      return;
   }

   {
      float fps = (float)m_cap->get(cv::CAP_PROP_FPS);
      m_mmdagent->sendLogString(m_id, MLOG_STATUS, "fps=%f", (float)fps);
      m_cap->set(cv::CAP_PROP_FOURCC, cv::VideoWriter::fourcc('M', 'J', 'P', 'G'));
      m_cap->set(cv::CAP_PROP_BUFFERSIZE, 1);
   }

   /* allocate buffer */
   if (m_pixelsCapture)
      free(m_pixelsCapture);
   m_pixelsCapture = (GLubyte *)malloc(m_cameraWidth * m_cameraHeight * 3);

   /* main loop */
   int count = 0;
   while (m_cameraThreadRunning) {
      if (m_cap == NULL)
         break;
      *m_cap >> cameraFrame;
      /* tell main thread that we are ready to offer captured frame */
      if (cameraFrame.empty() == false)
         m_cameraCapturing = true;
      /* copy from cv::Frame to RGB buffer */
      GLubyte *dst = m_pixelsCapture;
      if (m_mirror) {
         for (int y = 0; y < m_cameraHeight; y++) {
            for (int x = 0; x < m_cameraWidth; x++) {
               cv::Vec3b bgr = cameraFrame.at<cv::Vec3b>(y, m_cameraWidth - 1 - x);
               *(dst++) = bgr[2];
               *(dst++) = bgr[1];
               *(dst++) = bgr[0];
            }
         }
      } else {
         for (int y = 0; y < m_cameraHeight; y++) {
            for (int x = 0; x < m_cameraWidth; x++) {
               cv::Vec3b bgr = cameraFrame.at<cv::Vec3b>(y, x);
               *(dst++) = bgr[2];
               *(dst++) = bgr[1];
               *(dst++) = bgr[0];
            }
         }
      }
      /* tell main thread that the next frame data has been prepared */
      m_cameraCaptureUpdated = true;
      count++;
      if (count == 100) {
         m_mmdagent->sendLogString(m_id, MLOG_STATUS, "100");
      }
   }

   /* end of thread */
   m_cameraCapturing = false;
   free(m_pixelsCapture);
   m_pixelsCapture = NULL;
}

static void captureCameraMain(void *param)
{
   CameraImage *cb = (CameraImage *)param;
   cb->captureCameraRun();
}

/* CameraImage::isCameraCapturing: return true when camera is capturing */
bool CameraImage::isCameraCapturing()
{
   if (m_cap && m_cameraThreadId >= 0 && m_cameraThreadRunning && m_cameraCapturing)
      return true;
   return false;
}

/* CameraImage::start: start thread */
bool CameraImage::start(int cameraId)
{
   // do nothing if already running
   if (isCameraCapturing())
      return false;

   /* store camera id */
   m_cameraId = cameraId;

   /* start camera capturing thread */
   m_cameraCapturing = false;
   m_cameraThreadRunning = true;
   m_cameraThreadId = glfwCreateThread(captureCameraMain, this);
   if (m_cameraThreadId == -1) {
      /* failed to start thread */
      m_mmdagent->sendLogString(m_id, MLOG_ERROR, "failed to create thread for camera capture, disabled");
      m_cameraWidth = m_cameraHeight = 0;
      m_cameraThreadRunning = false;
      if (m_cap)
         m_cap->release();
      m_cap = NULL;
      return false;
   }

   return true;
}

/* CameraImage::stop: stop capturing thread */
void CameraImage::stop()
{
   if (m_cameraThreadId >= 0) {
      m_cameraThreadRunning = false;
      glfwWaitThread(m_cameraThreadId, GLFW_WAIT);
      glfwDestroyThread(m_cameraThreadId);
      m_cameraThreadId = -1;
   }
}

/* CameraImage::updateTexture: update texture */
void CameraImage::updateTexture()
{
   /* if camera capture is not running, do nothing */
   if (isCameraCapturing() == false)
      return;

   /* if update is too fast and no newer frame is captured, return */
   if (m_cameraCaptureUpdated == false)
      return;

   /* send the captured data to background texture */
      /* prepare texture */
   if (m_textureId == PMDTEXTURE_UNINITIALIZEDID) {
      /* allocate texture id */
      glGenTextures(1, &m_textureId);
      glBindTexture(GL_TEXTURE_2D, m_textureId);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
#if defined(__ANDROID__) || TARGET_OS_IPHONE
      if (isPowerOfTwo(m_width) && isPowerOfTwo(m_height)) {
         glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
         glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
      } else {
         /* some Android device cannot do GL_REPEAT on non-power-of-two sizes */
         /* since MMD seems to run with no error with edge clamp, use it */
         glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
         glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
      }
#else
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
#endif /* __ANDROID__ || TARGET_OS_IPHONE */
      glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, m_cameraWidth, m_cameraHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, m_pixelsCapture);
      GLfloat priority = 1.0f;
      glPrioritizeTextures(1, &m_textureId, &priority);
      glBindTexture(GL_TEXTURE_2D, 0);
   }

   /* update texture */
   glBindTexture(GL_TEXTURE_2D, m_textureId);
//   glfwLockMutex(m_mutexCapture);
   glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, m_cameraWidth, m_cameraHeight, GL_RGB, GL_UNSIGNED_BYTE, m_pixelsCapture);
//   glfwUnlockMutex(m_mutexCapture);
   glBindTexture(GL_TEXTURE_2D, 0);

   /* set flag to check if next frame has been captured */
   m_cameraCaptureUpdated = false;
}

/* CameraImage::getTextureId: get texture Id */
GLuint CameraImage::getTextureId()
{
   return m_textureId;
}

/* CameraImage::getWidth: get width */
int CameraImage::getWidth()
{
   return m_cameraWidth;
}

/* CameraImage::getHeight: get height */
int CameraImage::getHeight()
{
   return m_cameraHeight;
}
