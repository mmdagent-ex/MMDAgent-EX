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

#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/imgproc.hpp>

// camera image class
class CameraImage
{
private:

   MMDAgent *m_mmdagent;             /* MMDAgent-EX instance */
   int m_id;                         /* module id of instance */

   int m_cameraId;                   /* camera id to open */
   cv::VideoCapture *m_cap;          /* web camera capture instance */
   int m_cameraWidth;                /* camera capture width */
   int m_cameraHeight;               /* camera capture height */

   GLubyte *m_pixelsCapture;         /* pixel buffer to store captured RGB data */
   bool m_mirror;                    /* true when left-right side should be swapped */

   GLFWthread m_cameraThreadId;      /* camera capturing thread id */
   bool m_cameraThreadRunning;       /* flag to control camera capturing thread */
   bool m_cameraCapturing;           /* true while camera is capturing */
   bool m_cameraCaptureUpdated;      /* true when camera capture has been updated */

   GLuint m_textureId;               /* texture ID */

   /* initialize */
   void initialize();

   /* clear */
   void clear();

public:

   // constructor
   CameraImage();

   // destructor
   ~CameraImage();

   // setup: set up
   void setup(MMDAgent *mmdagent, int mid);

   /* captureCameraRun: capture camera loop */
   void captureCameraRun();

   /* isCameraCapturing: return true when camera is capturing */
   bool isCameraCapturing();

   /* start: start thread */
   bool start(int cameraId);

   /* stop: stop thread */
   void stop();

   /* updateTexture: update texture */
   void updateTexture();

   /* getTextureId: get texture Id */
   GLuint getTextureId();

   /* getWidth: get width */
   int getWidth();

   /* getHeight: get height */
   int getHeight();
};

