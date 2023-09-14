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


#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/opencv.hpp>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavutil/opt.h>
#include <libavutil/imgutils.h>
#include <libavformat/avformat.h>
#include <libavutil/timestamp.h>
#include <libswscale/swscale.h>
}

#include <mutex>

// commands
// SCREENENCODE_START|(ID of camera, -1 to disable)|bitrate|fps|base_width|base_height|camera_zoomrate
#define PLUGIN_COMMAND_SCREENENCODE_START "SCREENENCODE_START"
// SCREENENCODE_STOP
#define PLUGIN_COMMAND_SCREENENCODE_STOP "SCREENENCODE_STOP"

// events
#define PLUGIN_EVENT_SCREENENCODE_START "SCREENENCODE_EVENT_START"
#define PLUGIN_EVENT_SCREENENCODE_STOP  "SCREENENCODE_EVENT_STOP"

// definitions
// use PBO-based screen capture and two-stage screen size reduction for better performance
#define USE_PBO_READPIXEL

// screen encoder class
class ScreenEncoder
{
private:

   std::mutex *m_mutex;

   MMDAgent *m_mmdagent;             /* MMDAgent-EX instance */
   int m_id;                         /* module id of instance */
   int m_bitrate;                    /* video output bit rate */
   int m_fps;                        /* video output frame per second */
   int m_baseWidth;                  /* default video width */
   int m_baseHeight;                 /* default video width */
   int m_zoomRate;                 /* camera zoom rate */

   AVCodecContext *m_codecContext;   /* AV codec context */
#ifdef USE_PBO_READPIXEL
   int m_screenWidth;                /* original screen width */
   int m_screenHeight;               /* original screen height */
   int m_screenStep;                 /* original-to-capture division coef */
#endif /* USE_PBO_READPIXEL */
   int m_captureWidth;               /* screen capture width */
   int m_captureHeight;              /* screen capture height */
   int m_cameraWidth;                /* web camera capture width */
   int m_cameraHeight;               /* web camera capture height */
   int m_imageWidth;                 /* total image width */
   int m_imageHeight;                /* total image width */
   int m_videoWidth;                 /* video output width */
   int m_videoHeight;                /* video output height */

#ifdef USE_PBO_READPIXEL
   GLuint m_pbo;                     /* pixel buffer object to read pixels */
   bool m_pboInitialized;
#endif /* USE_PBO_READPIXEL */
   GLubyte *m_pixelsCapture;         /* pixel buffer to store captured RGB data */
   GLubyte *m_pixelsImage;           /* pixel buffer to store total image data */
   struct SwsContext *m_swsContext;  /* sws context for scaling */
   int m_srcStride;                  /* source image stride */
   int m_frameCounter;               /* frame counter for GOP key frame issue */
   GLFWmutex m_mutexCapture;

   Poco::Net::WebSocket *m_websocket; /* web socket */
   bool m_sendFrameRunning;          /* flag to control send frame thread */
   GLFWcond m_cond;
   GLFWthread m_sendThreadId;        /* frame sending thread id */
   GLFWmutex m_mutexSend;
   bool m_issueFrameFlag;
   bool m_isReady;

   bool m_enableCamera;              /* flag to enable camera capture */
   int m_cameraId;                   /* camera id to open */
   cv::VideoCapture *m_cap;          /* web camera capture instance */
   GLFWthread m_cameraThreadId;      /* camera capturing thread id */
   cv::Mat m_cameraFrame;            /* captured image */
   unsigned char *m_noCameraData;    /* no camera image data */
   bool m_cameraCaptureRunning;      /* flag to control camera capturing thread */

   float m_avatarMaxVol;             /* maximum volume of avatar speaking */
   float m_cameraMaxVol;             /* maximum volume of input camera audio */

   /* initialize */
   void initialize();

   /* clear */
   void clear();

#ifdef USE_PBO_READPIXEL
   /* setCaptureSize: set capture size */
   bool setCaptureSize(int screenWidth, int screenHeight);
#endif /* USE_PBO_READPIXEL */

   /* flipImage: flip image */
   void flipImage(GLubyte *pixels, int width, int height);

   /* flushEncoder: flush encoder data */
   void flushEncoder();

   /* setupCodec: setup codec */
   bool setupCodec();

   // sendFrame: send a frame
   void sendFrame();

public:

   // constructor
   ScreenEncoder();

   // destructor
   ~ScreenEncoder();

   // setup: set up
   bool setup(MMDAgent *mmdagent, int mid, int cameraId, int bitRate, int fps, int width, int height, int zoom, Poco::Net::WebSocket *websocket, std::mutex *mutex);

   /* captureCameraRun: capture camera loop */
   void captureCameraRun();

   // captureFrame: capture screen and store to buffer
   bool captureFrame();

   /* sendFrameRun: send frame loop */
   void sendFrameRun();

   /* start: start thread */
   bool start();

   /* stop: stop thread */
   void stop();

   /* setIssueFrameFlag: set issue frame flag */
   void setIssueFrameFlag(bool value);

   /* getIssueFrameFlag: get issue frame flag */
   bool getIssueFrameFlag();

   /* isCameraCapturing: return true when camera is capturing */
   bool isCameraCapturing();

   /* setAudioVolumes: set avatar audio volumes */
   void setAudioVolumes(float volume_remote, float volume_camera);

   /* isReady: return true when ready */
   bool isReady();

};

