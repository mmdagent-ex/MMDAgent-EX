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

/* definitions */

#define MAIN_TITLE        "MMDAgent-EX - Toolkit for conversational user interface and voice interaction"

/* headers */

#include <locale.h>
#ifdef _WIN32
#include <windows.h>
#include <Shlwapi.h>
#include <string>
#endif /* _WIN32 */
#ifdef __APPLE__
#include <CoreFoundation/CoreFoundation.h>
#endif /* __APPLE__ */
#if !defined(_WIN32) && !defined(__APPLE__) && !defined(__ANDROID__)
#include <limits.h>
#include <iconv.h>
#include <unistd.h>
#endif /* !_WIN32 && !__APPLE__ && !__ANDROID__ */
#if TARGET_OS_IPHONE
#include "main_ios.h"
#endif /* TARGET_OS_IPHONE */
#ifdef __ANDROID__
#include <jni.h>
#include <errno.h>
#include <android/sensor.h>
#include <android_native_app_glue.h>
#include <android/looper.h>
#include <EGL/egl.h>
#include <unistd.h>
#include <sys/time.h>
#include "android/log.h"
extern "C" {
   void Pa_AndroidSetNativeParam(int sampleRate, int bufferLength);
}
static struct android_app *g_app;
static JNIEnv *g_env;
#endif /* __ANDROID__ */

#include "MMDAgent.h"

#include <string>

/* MMDAgent */

MMDAgent *mmdagent = NULL;  /* MMDAgent main instance */
bool enable;                /* true when main processing is enabled */

/* Key status */

/* Mouse status */
static int mouseCurrentPosX;       /* current mouse position X */
static int mouseCurrentPosY;       /* current mouse position Y */
static double mousePressedTime;    /* time of last mouse press */
static int mousePressedPosX;       /* position X of last mouse press */
static int mousePressedPosY;       /* position Y of last mouse press */
static double mouseMoveStartTime;  /* time at drag start detection */
static int mouseMoveStartPosX;     /* position X at gtag start detection */
static int mouseMoveStartPosY;     /* position Y at gtag start detection */
static int mousePanningDirectionX; /* X direction of move detected at panning*/
static int mousePanningDirectionY; /* Y direction of move detected at panning */
static bool mousePanningDirectionDetermined;  /* true when the last update has detected panning direction */
static int mouseState;             /* current number of current gesture state machine */
static int mouseLastWheel;         /* last wheel parameter */
#ifdef _WIN32
static bool mouseTestingLeave;
#endif /* _WIN32 */

/* definitions for flick directions */
#define MOUSE_FLICK_UP_LEFT      0
#define MOUSE_FLICK_UP_RIGHT     1
#define MOUSE_FLICK_DOWN_LEFT    2
#define MOUSE_FLICK_DOWN_RIGHT   3
#define MOUSE_FLICK_LEFT_UPPER   4
#define MOUSE_FLICK_LEFT_LOWER   5
#define MOUSE_FLICK_RIGHT_UPPER  6
#define MOUSE_FLICK_RIGHT_LOWER  7
#define MOUSE_FLICK_EDGE_RANGE   0.2f  /* edge flick beginning point as ratio of screen */

/* thresholds for gesture state machine */
#define GESTURE_NOTMOVE_DISTANCE2_THRES (0.0001)  /* range^2 thres to determine moved / not moved */
#define GESTURE_FLICK_SPEED_THRES (0.17)          /* speed thres to determine flick / pan */
#define GESTURE_PANDIRECTION_FIXDISTANCE 5        /* distance thres to determine pan direction */

/* debug definition: undef GESLOG to disable logging */
//#define GESLOG(A) if (mmdagent) mmdagent->sendLogString(0, MLOG_STATUS, A)
#define GESLOG(A)


#ifdef __ANDROID__
/* check if user has granted permission to access microphone, and if not, prompt for user */
bool android_check_microphone_permission_granted(struct android_app *app, JNIEnv *env)
{
   jobject me = app->activity->clazz;
   jclass acl = env->GetObjectClass(me);

   /* get audio permission */
   // checkReulst = ContextCompat.checkSelfPermission(this, Manifest.permission.RECORD_AUDIO)
   jmethodID checkMethod = env->GetMethodID(acl, "checkSelfPermission", "(Ljava/lang/String;)I");
   jstring permissionName = env->NewStringUTF("android.permission.RECORD_AUDIO");
   jint checkResult = env->CallIntMethod(me, checkMethod, permissionName);
   if (checkResult != 0) { /* not granted */
      __android_log_print(ANDROID_LOG_ERROR, "MMDAgent", "RECORD_AUDIO permission not granted, asking");
      //ActivityCompat.requestPermissions(thisActivity, new String[]{Manifest.permission.RECORD_AUDIO}, MY_PERMISSIONS_REQUEST_RECORD_AUDIO);
      jmethodID requestPermissions = env->GetMethodID(acl, "requestPermissions", "([Ljava/lang/String;I)V");
      jobjectArray a = (jobjectArray)env->NewObjectArray(1, env->FindClass("java/lang/String"), env->NewStringUTF(""));
      env->SetObjectArrayElement(a, 0, permissionName);
      env->CallVoidMethod(me, requestPermissions, a, (jint)0);
   }

   return (checkResult != 0 ? false : true);
}

/* poll event of android messages, just used for waiting permission prompt */
static void android_poll_event()
{
    int events;
    struct android_poll_source *source;

    while (ALooper_pollAll(0, NULL, &events, (void**) &source) >= 0) {
        if (source != NULL)
            source->process(g_app, source);
    }
}

/* additional window event handler for android */
void AndroidProcWindowEvent( struct android_app* app, int32_t cmd )
{
   if (enable == false)
      return;

   switch (cmd) {
   case APP_CMD_INPUT_CHANGED:
      __android_log_print(ANDROID_LOG_VERBOSE, "MMDAgent", "APP_CMD_INPUT_CHANGED");
      break;
   case APP_CMD_INIT_WINDOW:
      // surfaceCreated
      __android_log_print(ANDROID_LOG_VERBOSE, "MMDAgent", "APP_CMD_INIT_WINDOW");
      /* restore surface */
      mmdagent->restoreSurface();
      break;
   case APP_CMD_TERM_WINDOW:
      // surfaceDestroyed
      __android_log_print(ANDROID_LOG_VERBOSE, "MMDAgent", "APP_CMD_TERM_WINDOW");
      break;
   case APP_CMD_WINDOW_RESIZED:
      // surfaceChanged
      __android_log_print(ANDROID_LOG_VERBOSE, "MMDAgent", "APP_CMD_WINDOW_RESIZED");
      break;
   case APP_CMD_WINDOW_REDRAW_NEEDED:
      __android_log_print(ANDROID_LOG_VERBOSE, "MMDAgent", "APP_CMD_REDRAW_NEEDED");
      break;
   case APP_CMD_CONTENT_RECT_CHANGED:
      __android_log_print(ANDROID_LOG_VERBOSE, "MMDAgent", "APP_CMD_CONTENT_RECT_CHANGED");
      break;
   case APP_CMD_GAINED_FOCUS:
      // onWindowFocusChanged
      __android_log_print(ANDROID_LOG_VERBOSE, "MMDAgent", "APP_CMD_GAINED_FOCUS");
      break;
   case APP_CMD_LOST_FOCUS:
      // onWindowFocusChanged
      __android_log_print(ANDROID_LOG_VERBOSE, "MMDAgent", "APP_CMD_LOST_FOCUS");
      break;
   case APP_CMD_CONFIG_CHANGED:
      // onConfigurationChanged
      __android_log_print(ANDROID_LOG_VERBOSE, "MMDAgent", "APP_CMD_CONFIG_CHANGED");
      break;
   case APP_CMD_LOW_MEMORY:
      __android_log_print(ANDROID_LOG_VERBOSE, "MMDAgent", "APP_CMD_LOW_MEMORY");
      break;
   case APP_CMD_START:
      // onStart
      __android_log_print(ANDROID_LOG_VERBOSE, "MMDAgent", "APP_CMD_START");
      break;
   case APP_CMD_RESUME:
      // onResume
      __android_log_print(ANDROID_LOG_VERBOSE, "MMDAgent", "APP_CMD_RESUME");
      /* resume threads and processing */
      mmdagent->resumeThreads();
      break;
   case APP_CMD_SAVE_STATE:
      // onResume
      __android_log_print(ANDROID_LOG_VERBOSE, "MMDAgent", "APP_CMD_SAVE_STATE");
      break;
   case APP_CMD_PAUSE:
      // onPause
      __android_log_print(ANDROID_LOG_VERBOSE, "MMDAgent", "APP_CMD_PAUSE");
      /* pause threads and processing */
      mmdagent->pauseThreads();
      break;
   case APP_CMD_STOP:
      // onStop
      __android_log_print(ANDROID_LOG_VERBOSE, "MMDAgent", "APP_CMD_STOP");
      break;
   case APP_CMD_DESTROY:
      // onDestroy
      __android_log_print(ANDROID_LOG_VERBOSE, "MMDAgent", "APP_CMD_DESTROY");
      enable = false;
      mmdagent->resumeThreads();
      break;
   default:
      break;
   }
}
#endif

#ifdef __APPLE__
#if TARGET_OS_IPHONE
#else
/* resetWithURLCallback: process scheme URL event */
void GLFWCALL resetWithURLCallback(const char *url)
{
   if (mmdagent)
      mmdagent->setResetFlag(url);
}
static char *startFile = NULL;
void GLFWCALL storeURLCallback(const char *url)
{
   if (startFile)
      free(startFile);
   startFile = MMDAgent_strdup(url);
   printf("[[[[%s]]]]\n", startFile);
}
void execStoredURLCallback()
{
   if (startFile) {
      if (mmdagent)
         mmdagent->setResetFlag(startFile);
      free(startFile);
      startFile = NULL;
   }
}
#endif /* TARGET_OS_IPHONE */
#endif /* __APPLE__ */

static wchar_t start_directory[2048];
static wchar_t work_directory[2048];

#ifdef _WIN32
static void do_hard_reset()
{
   std::wstring updaterPath(L".\\run_updater.bat");
   std::wstring command = updaterPath + L" \"" + start_directory + L"\" " + GetCommandLineW();

   SetCurrentDirectoryW(work_directory);

   STARTUPINFOW si;
   PROCESS_INFORMATION pi;

   ZeroMemory(&si, sizeof(si));
   si.cb = sizeof(si);
   ZeroMemory(&pi, sizeof(pi));

   // restart this program
   if (CreateProcessW(NULL,   // No module name (use command line)
      const_cast<wchar_t *>(command.c_str()), // Command line
      NULL,   // Process handle not inheritable
      NULL,   // Thread handle not inheritable
      FALSE,  // Set handle inheritance to FALSE
      0,      // No creation flags
      NULL,   // Use parent's environment block
      NULL,   // Use parent's starting directory
      &si,    // Pointer to STARTUPINFO structure
      &pi)    // Pointer to PROCESS_INFORMATION structure
      )
   {
      // close process and handles
      CloseHandle(pi.hProcess);
      CloseHandle(pi.hThread);

      // exit current program
      exit(0);
   }
}
#endif


/* procWindowSizeMessage: process window resize message */
void GLFWCALL procWindowSizeMessage(int w, int h)
{
   if(enable == false)
      return;

   mmdagent->procWindowSizeMessage(w, h);
}

/* procDropFileMessage: process drop files message */
void GLFWCALL procDropFileMessage(const char *file, int x, int y)
{
   if(enable == false)
      return;

   mmdagent->procDropFileMessage(file, mouseCurrentPosX, mouseCurrentPosY);
}

#ifdef _WIN32
/* procMouseLeaveMessage: process mouse leave message */
void GLFWCALL procMouseLeaveMessage()
{
   if (enable == false)
      return;

   mouseTestingLeave = false;
   if (mmdagent->getTabbar())
      mmdagent->getTabbar()->resetPointingButton();
}
#endif /* _WIN32 */

/* procKeyMessage: process key message */
void GLFWCALL procKeyMessage(int key, int action)
{
   if(enable == false)
      return;
   if (mmdagent == NULL)
      return;

   enable = mmdagent->getKeyHandler()->processKeyMessage(key, action);
}

/* procCharMessage: process char message */
void GLFWCALL procCharMessage(int key, int action)
{
   if(enable == false)
      return;
   if (mmdagent == NULL)
      return;

   enable = mmdagent->getKeyHandler()->processCharMessage(key, action);
}

/* tap action callback */
void actionTapped(int x, int y, int w, int h)
{
   Button *b;

   GESLOG("TAPPED");

   if (mmdagent->getTabbar()) {
      if (mmdagent->getTabbar()->isShowing()) {
         if (mmdagent->getTabbar()->isPointed(x, y, w, h)) {
            mmdagent->getTabbar()->execByTap(x, y, w, h);
            return;
         }
      } else {
         mmdagent->getTabbar()->show();
      }
   }

   if (mmdagent->getInfoText() && mmdagent->getInfoText()->isShowing()) {
      mmdagent->getInfoText()->execByTap(x, y, w, h);
      return;
   }

   if (mmdagent->getSlider() && mmdagent->getSlider()->isShowing()) {
      if (mmdagent->getSlider()->isPointed(x, y, w, h)) {
         mmdagent->getSlider()->execByTap(x, y, w, h);
      } else {
         mmdagent->getSlider()->setExecuteFlag(false);
         mmdagent->getSlider()->hide();
      }
      return;
   }

   if (mmdagent->getMenu() && mmdagent->getMenu()->isShowing()) {
      if (mmdagent->getMenu()->isPointed(x, y, w, h))
         mmdagent->getMenu()->execByTap(x, y, w, h);
      else if (mmdagent->getMenu()->isPopping())
         mmdagent->getMenu()->releasePopup();
      else
         mmdagent->getMenu()->hide();
      return;
   }

   if (mmdagent->getFileBrowser() && mmdagent->getFileBrowser()->isShowing()) {
      if (mmdagent->getFileBrowser()->isPointed(x, y, w, h))
         mmdagent->getFileBrowser()->execByTap(x, y, w, h);
      else
         mmdagent->getFileBrowser()->hide();
      return;
   }

   if (mmdagent->getPrompt() && mmdagent->getPrompt()->isShowing()) {
      if (mmdagent->getPrompt()->isPointed(x, y, w, h))
         mmdagent->getPrompt()->execByTap(x, y, w, h);
      return;
   }

   b = mmdagent->pointedButton(x, y, w, h);
   if (b) {
      b->exec();
      return;
   }

   mmdagent->getKeyHandler()->processMouseLeftButtonDownMessage(x, y);
   mmdagent->procMouseLeftButtonUpMessage();
}

/* double tap action callback */
void actionDoubleTapped(int x, int y, int w, int h)
{
   GESLOG("DOUBLE TAPPED");
   mmdagent->procMouseLeftButtonDoubleClickMessage(x, y);
}

/* flick action callback */
void actionFlicked(int x, int y, int w, int h, int dir)
{
   float rx = x / (float)w;
   float ry = y / (float)h;

   GESLOG("FLICKED");

   if (mmdagent->getInfoText() && mmdagent->getInfoText()->isShowing()) {
      return;
   }

   if (mmdagent->getSlider() && mmdagent->getSlider()->isShowing()) {
      return;
   }

   if (mmdagent->getMenu() && mmdagent->getMenu()->isShowing()) {
      if (mmdagent->getMenu()->isPointed(x, y, w, h)) {
         switch (dir) {
         case MOUSE_FLICK_RIGHT_LOWER:
         case MOUSE_FLICK_RIGHT_UPPER:
            if (mmdagent->getMenu()->isPopping())
               mmdagent->getMenu()->releasePopup();
            mmdagent->getMenu()->backward();
            break;
         case MOUSE_FLICK_LEFT_LOWER:
         case MOUSE_FLICK_LEFT_UPPER:
            if (mmdagent->getMenu()->isPopping())
               mmdagent->getMenu()->releasePopup();
            mmdagent->getMenu()->forward();
            break;
         }
      }
      return;
   }

#if 0
   switch (dir) {
   case MOUSE_FLICK_RIGHT_LOWER:
   case MOUSE_FLICK_RIGHT_UPPER:
      if (mmdagent->getSlider() && rx < MOUSE_FLICK_EDGE_RANGE)
         mmdagent->getSlider()->show();
      break;
   case MOUSE_FLICK_LEFT_LOWER:
   case MOUSE_FLICK_LEFT_UPPER:
      if (mmdagent->getMenu() && rx > 1.0f - MOUSE_FLICK_EDGE_RANGE)
         mmdagent->getMenu()->show();
      break;
   }
#endif
}

/* long tap action start callback */
void actionLongTapStart(int x, int y, int w, int h)
{
   GESLOG("LONG TAP START");

   if (mmdagent->getInfoText()->isShowing())
      return;

   if (mmdagent->getMenu() && mmdagent->getMenu()->isShowing() && mmdagent->getMenu()->isPointed(x, y, w, h)) {
      mmdagent->getMenu()->togglePopupByTap(x, y, w, h);
      return;
   }

   mmdagent->procMouseLeftButtonLongPressedMessage(x, y, w, h);
}

/* long tap action end callback */
void actionLongTapEnd(int x, int y, int w, int h)
{
   GESLOG("LONG TAP END");

   if (mmdagent->getInfoText()->isShowing())
      return;

   if (mmdagent->getMenu() && mmdagent->getMenu()->isShowing() && mmdagent->getMenu()->isPointed(x, y, w, h))
      return;

   mmdagent->procMouseLeftButtonLongReleasedMessage(x, y, w, h);
}

static int scroll_Y;
static int scroll_pastY;
static int mousePanningStartX;
static int mousePanningStartY;

/* panning action start callback */
void actionPanningStart(int x, int y, int w, int h)
{
   float rx = x / (float)w;
   float ry = y / (float)h;

   GESLOG("PANNING START");

   mousePanningStartX = x;
   mousePanningStartY = y;

   if (mmdagent->getInfoText() && mmdagent->getInfoText()->isShowing()) {
      mmdagent->getInfoText()->setStartingPoint(x, y, w, h);
      return;
   }
   if (mmdagent->getSlider() && mmdagent->getSlider()->isShowing()) {
      if (mmdagent->getSlider()->isPointed(x, y, w, h)) {
         mmdagent->getSlider()->execByTap(x, y, w, h);
         mmdagent->getSlider()->setExecuteFlag(true);
      }
      return;
   }

   mmdagent->getKeyHandler()->processMouseLeftButtonDownMessage(x, y);

   scroll_Y = scroll_pastY = 0;
}

/* panning action callback */
void actionPanning(int x, int y, int w, int h)
{
   int vx;
   int vy;

   //GESLOG("PANNING");

   vx = x - mousePanningStartX;
   vy = y - mousePanningStartY;

   if (mmdagent->getInfoText() && mmdagent->getInfoText()->isShowing()) {
      mmdagent->getInfoText()->setCurrentPoint(x, y);
      return;
   }

   if (mmdagent->getSlider() && mmdagent->getSlider()->isShowing() && mmdagent->getSlider()->getExecuteFlag() == true) {
      mmdagent->getSlider()->execByTap(x, y, w, h);
      return;
   }

   if (mmdagent->getMenu() && mmdagent->getMenu()->isShowing() && mmdagent->getMenu()->isPointed(mousePanningStartX, mousePanningStartY, w, h)) {
      if (mousePanningDirectionDetermined) {
         if (mousePanningDirectionX == -1) {
            if (mmdagent->getMenu()->isPopping())
               mmdagent->getMenu()->releasePopup();
            mmdagent->getMenu()->forward();
         } else if (mousePanningDirectionX == 1) {
            if (mmdagent->getMenu()->isPopping())
               mmdagent->getMenu()->releasePopup();
            mmdagent->getMenu()->backward();
         }
      }
      if (mousePanningDirectionX == 1) {
         mmdagent->getMenu()->forceBackwardAnimationRate(vx / (float)w);
      } else if (mousePanningDirectionX == -1) {
         mmdagent->getMenu()->forceForwardAnimationRate(- vx / (float)w);
      } else {
         scroll_Y = 20 * vy / h;
         if (scroll_Y != scroll_pastY)
            mmdagent->getMenu()->scroll(scroll_pastY - scroll_Y);
         scroll_pastY = scroll_Y;
      }
      return;
   }

   if (mmdagent->getFileBrowser() && mmdagent->getFileBrowser()->isShowing() && mmdagent->getFileBrowser()->isPointed(mousePanningStartX, mousePanningStartY, w, h)) {
      if (fabs(vx) < fabs(vy)) {
         mmdagent->getFileBrowser()->scroll(-mousePanningDirectionY);
      } else if (fabs(vx / (float)w) > 0.1f) {
         mmdagent->getFileBrowser()->setBackSlideAnimationRate(vx / (float)w);
      }
      return;
   }

   mmdagent->getKeyHandler()->processMousePosMessage(x, y);
}

/* panning action end callback */
void actionPanningEnd(int x, int y, int w, int h)
{
   int vx;
   int vy;

   GESLOG("PANNING END");

   vx = x - mousePanningStartX;
   vy = y - mousePanningStartY;

   if (mmdagent->getInfoText() && mmdagent->getInfoText()->isShowing()) {
      mmdagent->getInfoText()->releasePoint();
      return;
   }

   if (mmdagent->getSlider() && mmdagent->getSlider()->isShowing()) {
      mmdagent->getSlider()->setExecuteFlag(false);
      return;
   }

   if (mmdagent->getMenu() && mmdagent->getMenu()->isShowing() && mmdagent->getMenu()->isPointed(mousePanningStartX, mousePanningStartY, w, h)) {
      if (mousePanningDirectionX == 1) {
         mmdagent->getMenu()->forceBackwardAnimationRate(-1.0f);
      } else if (mousePanningDirectionX == -1) {
         mmdagent->getMenu()->forceForwardAnimationRate(-1.0f);
      }
      return;
   }

   if (mmdagent->getFileBrowser() && mmdagent->getFileBrowser()->isPointed(x, y, w, h)) {
      mmdagent->getFileBrowser()->setBackSlideAnimationRate(0.0f);
      if (fabs(vx) > fabs(vy)) {
         if (vx / (float)w > 0.2f) {
            mmdagent->getFileBrowser()->back();
         } else if (vx / (float)w < -0.2f) {
            mmdagent->getFileBrowser()->hide();
         }
      }
      return;
   }

   mmdagent->procMouseLeftButtonUpMessage();
}

/* long tapped panning action start callback */
void actionLongTappedPanningStart(int x, int y, int w, int h)
{
   GESLOG("LONG TAPPED PANNING START");
}

/* long tapped panning action callback */
void actionLongTappedPanning(int x, int y, int w, int h)
{
   //GESLOG("LONG TAPPED PANNING");
}

/* long tapped panning action end callback */
void actionLongTappedPanningEnd(int x, int y, int w, int h)
{
   GESLOG("LONG TAPPED PANNING END");
}

/* double tapped panning action start callback */
void actionDoubleTappedPanningStart(int x, int y, int w, int h)
{
   GESLOG("DOUBLE TAPPED PANNING START");

   mmdagent->procMouseLeftButtonDownMessage(x, y, false, true);
}

/* double tapped panning action callback */
void actionDoubleTappedPanning(int x, int y, int w, int h)
{
   //GESLOG("DOUBLE TAPPED PANNING");

   mmdagent->procMousePosMessage(x, y, false, true);
}

/* double tapped panning action end callback */
void actionDoubleTappedPanningEnd(int x, int y, int w, int h)
{
   GESLOG("DOUBLE TAPPED PANNING END");

   mmdagent->procMouseLeftButtonUpMessage();
}

/* return relative distance^2, as relative to screen size */
double getRelativeDistance2(int x1, int y1, int x2, int y2, int w, int h)
{
   double rx = (x2 - x1) / (float)w;
   double ry = (y2 - y1) / (float)h;

   return rx * rx + ry * ry;
}

/* update and check if panning direction has been determined */
void gestureUpdatePanningDirection()
{
   int vx, vy;

   mousePanningDirectionDetermined = false;
   if (mousePanningDirectionX == 0 && mousePanningDirectionY == 0) {
      vx = mouseCurrentPosX - mousePressedPosX;
      vy = mouseCurrentPosY - mousePressedPosY;
      if (vx < -GESTURE_PANDIRECTION_FIXDISTANCE) {
         mousePanningDirectionX = -1;
         mousePanningDirectionDetermined = true;
      } else if (vx > GESTURE_PANDIRECTION_FIXDISTANCE) {
         mousePanningDirectionX = 1;
         mousePanningDirectionDetermined = true;
      } else if (vy < -GESTURE_PANDIRECTION_FIXDISTANCE) {
         mousePanningDirectionY = -1;
         mousePanningDirectionDetermined = true;
      } else if (vy > GESTURE_PANDIRECTION_FIXDISTANCE) {
         mousePanningDirectionY = 1;
         mousePanningDirectionDetermined = true;
      }
   }
}

/* main gesture recognition state machine update */
void gestureStateMachineUpdate(int button, int x, int y)
{
   int w, h;
   int dx, dy, dir;
   double t;
   double dist2;
   double difftime;
   bool proceedState;

   if (x >= 0)
      mouseCurrentPosX = x;
   if (y >= 0)
      mouseCurrentPosY = y;

   mmdagent->getWindowSize(&w, &h);

   proceedState = true;
   while (proceedState) {
      proceedState = false;
      switch (mouseState) {
      case 0: /* initial state */
         if (button == GLFW_PRESS) {
            /* save pressed time and position */
            t = MMDAgent_getTime();
            mousePressedTime = t;
            mousePressedPosX = mouseCurrentPosX;
            mousePressedPosY = mouseCurrentPosY;
            /* go to pressing state */
            GESLOG("init -> pressing");
            mouseState = 1;
         }
         break;
      case 1: /* pressing */
         t = MMDAgent_getTime();
         difftime = t - mousePressedTime;
         dist2 = getRelativeDistance2(mouseCurrentPosX, mouseCurrentPosY, mousePressedPosX, mousePressedPosY, w, h);
         if (button == GLFW_RELEASE && difftime < 0.2 && dist2 < GESTURE_NOTMOVE_DISTANCE2_THRES) {
            /* goto next tap wait state */
            GESLOG("pressing -> tap wait");
            mouseState = 3;
         } else if (difftime >= 1.0 && dist2 < GESTURE_NOTMOVE_DISTANCE2_THRES) {
            /* go to long tapping state */
            actionLongTapStart(mouseCurrentPosX, mouseCurrentPosY, w, h);
            GESLOG("pressing -> long tapping");
            mouseState = 4;
         } else if (dist2 >= GESTURE_NOTMOVE_DISTANCE2_THRES) {
            GESLOG("pressing -> pan vs flick");
            /* save last move-start time and position */
            mouseMoveStartTime = t;
            mouseMoveStartPosX = mouseCurrentPosX;
            mouseMoveStartPosY = mouseCurrentPosY;
            mouseState = 2;
            /* do not wait for next call, repeat this switch clause */
            proceedState = true;
         }
         break;
      case 2: /* pan vs flick */
         t = MMDAgent_getTime();
         difftime = t - mouseMoveStartTime;
         dist2 = getRelativeDistance2(mouseCurrentPosX, mouseCurrentPosY, mouseMoveStartPosX, mouseMoveStartPosY, w, h);
         if (difftime >= 0.05) {
            if (dist2 / difftime >= GESTURE_FLICK_SPEED_THRES) {
               /* detect flick */
               dx = mouseCurrentPosX - mousePressedPosX;
               dy = mouseCurrentPosY - mousePressedPosY;
               if (dx >= 0 && dy >= 0) { /* right-down space */
                  if (dx >= dy)
                     dir = MOUSE_FLICK_RIGHT_LOWER;
                  else
                     dir = MOUSE_FLICK_DOWN_RIGHT;
               } else if (dx >= 0 && dy < 0) { /* right-up space */
                  if (dx >= -dy)
                     dir = MOUSE_FLICK_RIGHT_UPPER;
                  else
                     dir = MOUSE_FLICK_UP_RIGHT;
               } else if (dx < 0 && dy >= 0) { /* left-down space */
                  if (-dx >= dy)
                     dir = MOUSE_FLICK_LEFT_LOWER;
                  else
                     dir = MOUSE_FLICK_DOWN_LEFT;
               } else if (dx < 0 && dy < 0) { /* left-up space */
                  if (-dx >= -dy)
                     dir = MOUSE_FLICK_LEFT_UPPER;
                  else
                     dir = MOUSE_FLICK_UP_LEFT;
               }
               actionFlicked(mousePressedPosX, mousePressedPosY, w, h, dir);
               GESLOG("reset");
               mouseState = 0;
            } else {
               /* go to pannning state */
               actionPanningStart(mouseCurrentPosX, mouseCurrentPosY, w, h);
               mousePanningDirectionX = 0;
               mousePanningDirectionY = 0;
               GESLOG("pressing -> panning");
               mouseState = 5;
            }
         }
         break;
      case 3: /* next tap wait */
         t = MMDAgent_getTime();
         difftime = t - mousePressedTime;
         if (button == GLFW_PRESS) {
            if (difftime < 0.22) {
               /* go to double tapping state */
               GESLOG("tap wait -> double tapping");
               mouseState = 7;
               break;
            }
         }
         if (difftime >= 0.22) {
            /* detect tap */
            actionTapped(mousePressedPosX, mousePressedPosY, w, h);
            GESLOG("reset");
            mouseState = 0;
         }
         break;
      case 4: /* long tapping */
         dist2 = getRelativeDistance2(mouseCurrentPosX, mouseCurrentPosY, mousePressedPosX, mousePressedPosY, w, h);
         if (button == GLFW_RELEASE && dist2 < GESTURE_NOTMOVE_DISTANCE2_THRES) {
            actionLongTapEnd(mouseCurrentPosX, mouseCurrentPosY, w, h);
            GESLOG("reset");
            mouseState = 0;
            break;
         }
         if (dist2 >= GESTURE_NOTMOVE_DISTANCE2_THRES) {
            /* go to long tapped panning state */
            actionLongTappedPanningStart(mouseCurrentPosX, mouseCurrentPosY, w, h);
            mousePanningDirectionX = 0;
            mousePanningDirectionY = 0;
            GESLOG("long tapping -> long tapped panningT");
            mouseState = 6;
         }
         break;
      case 5: /* panning */
         gestureUpdatePanningDirection();
         actionPanning(mouseCurrentPosX, mouseCurrentPosY, w, h);
         if (button == GLFW_RELEASE) {
            actionPanningEnd(mouseCurrentPosX, mouseCurrentPosY, w, h);
            GESLOG("reset");
            mouseState = 0;
         }
         break;
      case 6: /* long tapped panning */
         gestureUpdatePanningDirection();
         actionLongTappedPanning(mouseCurrentPosX, mouseCurrentPosY, w, h);
         if (button == GLFW_RELEASE) {
            actionLongTappedPanningEnd(mouseCurrentPosX, mouseCurrentPosY, w, h);
            GESLOG("reset");
            mouseState = 0;
         }
         break;
      case 7: /* double tapping */
         t = MMDAgent_getTime();
         difftime = t - mousePressedTime;
         if (button == GLFW_RELEASE && difftime < 0.4) {
            /* detect double tap */
            actionDoubleTapped(mouseCurrentPosX, mouseCurrentPosY, w, h);
            GESLOG("reset");
            mouseState = 0;
            break;
         }
         if (difftime >= 0.4) {
            /* go to double tapped panning state */
            actionDoubleTappedPanningStart(mouseCurrentPosX, mouseCurrentPosY, w, h);
            mousePanningDirectionX = 0;
            mousePanningDirectionY = 0;
            GESLOG("double tapping -> double tapped panning");
            mouseState = 8;
         }
         break;
      case 8: /* double tapped panning */
         gestureUpdatePanningDirection();
         actionDoubleTappedPanning(mouseCurrentPosX, mouseCurrentPosY, w, h);
         if (button == GLFW_RELEASE) {
            actionDoubleTappedPanningEnd(mouseCurrentPosX, mouseCurrentPosY, w, h);
            GESLOG("reset");
            mouseState = 0;
         }
         break;
      }
   }
}

/* procMouseButtonMessage: process mouse button message */
void GLFWCALL procMouseButtonMessage(int button, int action)
{
   if (enable == false)
      return;

   if (button == GLFW_MOUSE_BUTTON_LEFT)
      gestureStateMachineUpdate(action, -1, -1);
}

/* procMousePosMessage: process mouse position message */
void GLFWCALL procMousePosMessage(int x, int y, int shift, int ctrl)
{
   if(enable == false)
      return;

   bool kks = (shift == GLFW_PRESS) ? true : false;
   bool kkc = (ctrl == GLFW_PRESS) ? true : false;

   mmdagent->getKeyHandler()->setModifierStatus(kks, kks, kkc, kkc);

   gestureStateMachineUpdate(-1, x, y);

   mmdagent->getKeyHandler()->processMouseStatusMessage(x, y);

   if (mmdagent->getTabbar() && mouseState == 0) {
      mmdagent->getTabbar()->show();
   }
   if (mmdagent->getTabbar()) {
      if (mmdagent->getTabbar()->isShowing()) {
         int w, h;
         mmdagent->getWindowSize(&w, &h);
#ifdef _WIN32
         if (mmdagent->getTabbar()->procMousePos(x, y, w, h) == true && mouseTestingLeave == false) {
            mouseTestingLeave = true;
            glfwPlatformEnableTrackMouseLeave();
         }
#else
         mmdagent->getTabbar()->procMousePos(x, y, w, h);
#endif /* _WIN32 */
      }
   }
   if (mmdagent->getInfoText()) {
      if (mmdagent->getInfoText()->isShowing()) {
         int w, h;
         mmdagent->getWindowSize(&w, &h);
         mmdagent->getInfoText()->procMousePos(x, y, w, h);
      }
   }

}

/* procMouseWheelMessage: process mouse wheel message */
void GLFWCALL procMouseWheelMessage(int x)
{
   int w, h;

   if(enable == false)
      return;

   /* avoid two-tap scroll */
   if (mouseLastWheel == x)
      return;

   if (mmdagent->getFileBrowser() && mmdagent->getFileBrowser()->isShowing()) {
      /* when file browser is showing */
      mmdagent->getWindowSize(&w, &h);
      if (mmdagent->getFileBrowser()->isPointed(mouseCurrentPosX, mouseCurrentPosY, w, h)) {
         if (x > mouseLastWheel)
            mmdagent->getFileBrowser()->scroll(-1);
         else
            mmdagent->getFileBrowser()->scroll(1);
      }
      mouseLastWheel = x;
      return;
   }
   mmdagent->getKeyHandler()->processMouseWheelMessage(x > mouseLastWheel ? true : false);
   mouseLastWheel = x;
}

/* openURL: open url */
static void openURI(const char *urlstring)
{
#if !defined(_WIN32) && !defined(__APPLE__) && !defined(__ANDROID__)
   if (fork() == 0) {
     execl("/usr/bin/xdg-open", "/usr/bin/xdg-open", urlstring, NULL);
     exit(-1);
   }
#endif
#if defined(_WIN32)
   ShellExecute(0, 0, urlstring, 0, 0, SW_SHOW);
#endif
}

/* commonMain: common main function */
int commonMain(int argc, char **argv, const char *systemDirName, const char *pluginDirName, const char *systemConfigFileName, const char *systemDownloadURI)
{
   enable = false;

   mousePressedTime = 0.0;
   mouseCurrentPosX = 0;
   mouseCurrentPosY = 0;
   mouseState = 0;
   mouseLastWheel = 0;
#ifdef _WIN32
   mouseTestingLeave = false;
#endif /* _WIN32 */

#ifdef __APPLE__
#if TARGET_OS_IPHONE
#else
   /* URL scheme */
   glfwSetURLSchemeCallback(storeURLCallback);
#endif /* TARGET_OS_IPHONE */
#endif /* __APPLE__ */

   /* initialize library */
   MMDAgent_enablepoco();

   /* reset timer */
   MMDAgent_setTime(0.0);

   /* create MMDAgent window */
   glfwInit();
   void* ptr = MMDFiles_alignedmalloc(sizeof(MMDAgent), 16);
   mmdagent = new(ptr) MMDAgent();

   /* setup system */
   if (mmdagent->setupSystem(systemDirName, pluginDirName, systemConfigFileName, systemDownloadURI, MAIN_TITLE) == false) {
      mmdagent->~MMDAgent();
      MMDFiles_alignedfree(mmdagent);
      mmdagent = NULL;
      glfwTerminate();
      return -1;
   }

   /* create menu in MMDAgent */
   mmdagent->createMenu();

   /* window */
   glfwSetWindowSizeCallback(procWindowSizeMessage);

   /* drag and drop */
   glfwSetDropFileCallback(procDropFileMessage);

#ifdef _WIN32
   /* mouse leave */
   glfwSetMouseLeaveCallback(procMouseLeaveMessage);
#endif /* _WIN32 */

   /* key */
   glfwSetKeyCallback(procKeyMessage);
   glfwEnable(GLFW_KEY_REPEAT);

   /* char */
   glfwSetCharCallback(procCharMessage);

   /* mouse */
   glfwSetMouseButtonCallback(procMouseButtonMessage);
   glfwSetMousePosCallback(procMousePosMessage);
   glfwSetMouseWheelCallback(procMouseWheelMessage);

#ifdef __APPLE__
#if TARGET_OS_IPHONE
#else
   /* URL scheme */
   glfwSetURLSchemeCallback(resetWithURLCallback);
   execStoredURLCallback();
#endif /* TARGET_OS_IPHONE */
#endif /* __APPLE__ */

#ifdef __ANDROID__
   /* check microphone permission */
   if (android_check_microphone_permission_granted(g_app, g_env) == false) {
      /* wait until permission prompt is closed */
      while(g_app->activityState != APP_CMD_PAUSE)
         android_poll_event();
      while(g_app->activityState == APP_CMD_PAUSE)
         android_poll_event();
   }
#endif

   /* setup content */
   if (mmdagent->setupContent(argc, argv) == false) {
      mmdagent->~MMDAgent();
      MMDFiles_alignedfree(mmdagent);
      mmdagent = NULL;
      glfwTerminate();
      return -1;
   }

   /* check if reset required while setup */
   if (mmdagent->getResetFlag() == true) {
      mmdagent->restart(systemDirName, pluginDirName, systemConfigFileName, systemDownloadURI, MAIN_TITLE);
      mmdagent->createMenu();
   }

   /* main loop */
   enable = true;
   while(enable == true && glfwGetWindowParam(GLFW_OPENED) == GL_TRUE) {
      /* update and render the world */
      mmdagent->updateAndRender();
      if (mmdagent->getHardResetFlag() == true) {
#ifdef _WIN32
         /* hard reset */
         do_hard_reset();
#else
         /* soft reset */
         mmdagent->restart(systemDirName, pluginDirName, systemConfigFileName, systemDownloadURI, MAIN_TITLE);
         mmdagent->createMenu();
#endif
      }
      if (mmdagent->getResetFlag() == true) {
         /* soft reset */
         mmdagent->restart(systemDirName, pluginDirName, systemConfigFileName, systemDownloadURI, MAIN_TITLE);
         mmdagent->createMenu();
      }
      /* check if URL loading is requested */
      if (mmdagent->getKeyValue() && !MMDAgent_strequal(mmdagent->getKeyValue()->getString("_RequestedURL", "NO"), "NO")) {
         openURI(mmdagent->getKeyValue()->getString("_RequestedURL", "NO"));
         mmdagent->getKeyValue()->setString("_RequestedURL", "NO");
      }
      /* update gesture time */
      gestureStateMachineUpdate(-1, -1, -1);
   }

   /* free */
#ifdef __ANDROID__
   /* stop log while dying to avoid segfault by calling localtime() */
   mmdagent->stopLogString();
#endif
   mmdagent->procWindowDestroyMessage();
   mmdagent->~MMDAgent();
   MMDFiles_alignedfree(mmdagent);
   mmdagent = NULL;
   glfwTerminate();
   return 0;
}

/* commonMainExec: common main function for desktop OS using executable path as base and get command arguments */
int commonMainExec(int argc, char **argv)
{
   size_t len;
   int result;

   char buff[MMDAGENT_MAXBUFLEN];
   char *systemDirName;
   char *systemConfigFileName;
   char *pluginDirName;
   char *sysDownloadURI = NULL;

   /* set paths from executable path in argv[0] */
   strcpy(buff, argv[0]);
   if (MMDAgent_strtailmatch(buff, ".exe") == true || MMDAgent_strtailmatch(buff, ".EXE") == true) {
      len = MMDAgent_strlen(buff);
      buff[len - 4] = '.';
      buff[len - 3] = 'm';
      buff[len - 2] = 'd';
      buff[len - 1] = 'f';
   } else {
      strcat(buff, ".mdf");
   }
   systemConfigFileName = MMDAgent_strdup(buff);
   systemDirName = MMDAgent_dirname(argv[0]);
   MMDAgent_snprintf(buff, MMDAGENT_MAXBUFLEN, "%s%c%s", systemDirName, MMDAGENT_DIRSEPARATOR, "Plugins");
   pluginDirName = MMDAgent_strdup(buff);

   /* if system URL is defined in a file, try to update the system */
   MMDAgent_snprintf(buff, MMDAGENT_MAXBUFLEN, "%s%c%s", systemDirName, MMDAGENT_DIRSEPARATOR, "site");
   FILE *fp = MMDAgent_fopen(buff, "rb");
   if (fp) {
      char readbuf[1024];
      if (fgets(readbuf, 1024, fp) != NULL) {
         int len = (int)strlen(readbuf) - 1;
         while (len >= 0 && (readbuf[len] == '\r' || readbuf[len] == '\n')) {
            readbuf[len] = '\0';
            len--;
         }
         if (len > 0) {
            sysDownloadURI = MMDAgent_strdup(readbuf);
         }
      }
   }

   /* run MMDAgent */
   result = commonMain(argc, argv, systemDirName, pluginDirName, systemConfigFileName, sysDownloadURI);

   /* free */
   free(systemDirName);
   free(pluginDirName);
   free(systemConfigFileName);
   if (sysDownloadURI)
      free(sysDownloadURI);

   return result;
}

/* main: main function */
#if defined(_WIN32) && !defined(__MINGW32__)
bool wTailMatch(const wchar_t *str1, const wchar_t *str2)
{
   size_t len1, len2;

   if (str1 == NULL || str2 == NULL)
      return false;
   if (str1 == str2)
      return true;
   len1 = wcslen(str1);
   len2 = wcslen(str2);
   if (len1 < len2)
      return false;
   if (_wcsicmp(&str1[len1 - len2], str2) == 0)
      return true;
   else
      return false;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
   int i;
   size_t len;
   int argc;
   wchar_t **wargv;
   char **argv;
   int result;
   bool error = false;
   wchar_t wbuf[MAX_PATH + 1];
   wchar_t wbuf2[MAX_PATH + 1];

   /* change LC_CTYPE from C to system locale */
   setlocale(LC_CTYPE, "");

   /* get UTF8 arguments */
   wargv = CommandLineToArgvW(GetCommandLineW(), &argc);
   if (argc < 1) return 0;

   /* save current directory and system directory in wide-char for hard reset */
   GetCurrentDirectoryW(2048, start_directory);
   GetFullPathNameW(wargv[0], MAX_PATH, work_directory, NULL);
   PathRemoveFileSpecW(work_directory);

   /* set DLL directory */
   GetFullPathNameW(wargv[0], MAX_PATH, wbuf, NULL);
   PathRemoveFileSpecW(wbuf);
#ifdef _WIN64
   /* use "DLLs64" folder instead of "DLLs" for WIN64 */
   PathCombineW(wbuf2, wbuf, L"DLLs64");
#else
   PathCombineW(wbuf2, wbuf, L"DLLs");
#endif
   AddDllDirectory(wbuf2);

   argv = (char **) malloc(sizeof(char *) * argc);
   for(i = 0; i < argc; i++) {
      wchar_t *warg = wargv[i];
      if (i == 0 || wTailMatch(wargv[i], L".mdf") || wTailMatch(wargv[i], L".mmda")) {
         /* convert to full path */
         GetFullPathNameW(wargv[i], MAX_PATH, wbuf, NULL);
         warg = wbuf;
      }
      argv[i] = NULL;
      result = WideCharToMultiByte(CP_UTF8, 0, (LPCWSTR)warg, -1, NULL, 0, NULL, NULL );
      if(result <= 0) {
         error = true;
         continue;
      }
      len = (size_t) result;
      argv[i] = (char *) malloc(sizeof(char) * (len + 1));
      if(argv[i] == NULL) {
         error = true;
         continue;
      }
      result = WideCharToMultiByte(CP_UTF8, 0, (LPCWSTR)warg, -1, (LPSTR) argv[i], (int)len, NULL, NULL);
      if((size_t) result != len) {
         error = true;
         continue;
      }
   }

   /* run MMDAgent */
   if (error == false)
      result = commonMainExec(argc, argv);

   /* free */
   for(i = 0; i < argc; i++) {
      if(argv[i])
         free(argv[i]);
   }
   free(argv);

   return (error == true) ? -1 : result;
}
#endif /* _WIN32 && !__MINGW32__ */
#if defined(_WIN32) && defined(__MINGW32__)
int main(int argc, char **argv)
{
   return(commonMainExec(argc, argv));
}
#endif /* _WIN32 && __MIGW32__ */
#ifdef __APPLE__
#if TARGET_OS_IPHONE
int main(int argc, char **argv)
{
   commonMainForIOS(argc, argv);
}
#else
int main(int argc, char **argv)
{
   int i;
   char buff[PATH_MAX + 1];
   char **newArgv;
   int result;
   bool error = false;

   newArgv = (char **) malloc(sizeof(char *) * argc);
   for(i = 0; i < argc; i++) {
      if (argv[i][0] == '-' || MMDAgent_strheadmatch(argv[i], "http:") || MMDAgent_strheadmatch(argv[i], "https:") || MMDAgent_strheadmatch(argv[i], "mmdagent:")) {
         newArgv[i] = MMDAgent_strdup(argv[i]);
         continue;
      }
      newArgv[i] = NULL;
      memset(buff, 0, PATH_MAX + 1);
      realpath(argv[i], buff);
      newArgv[i] = MMDAgent_strdup(buff);
   }

   if (error == false)
      result = commonMainExec(argc, newArgv);

   for(i = 0; i < argc; i++) {
      if(newArgv[i])
         free(newArgv[i]);
   }
   free(newArgv);

   return (error == true) ? -1 : result;
}
#endif
#endif /* __APPLE__ */
#ifdef __ANDROID__
void android_main(struct android_app *app)
{
   int result;
   char buff[MMDAGENT_MAXBUFLEN];
   const char *path;
   char *argv[2];
   int argc;
   char *systemDirName;
   char *systemConfigFileName;
   char *pluginDirName;
   char *contentDownloadDir;
   char *sysDownloadURI = NULL;
   DIRECTORY *d;
   int i;

   glfwInitForAndroid(app, AndroidProcWindowEvent);
   g_app = app;

   /* app_dummy() is deprecated in recent sdk */
   //app_dummy();

   /* get internal file path */
   path = app->activity->internalDataPath;
   if (path == NULL)
      return;

   /* change current dir to there */
   if(MMDAgent_chdir(path) == false)
      return;

   JNIEnv *env;
   app->activity->vm->AttachCurrentThread(&env, 0);
   g_env = env;
   jobject me = app->activity->clazz;
   jclass acl = env->GetObjectClass(me); //class pointer of NativeActivity

   /* set content directory to internal file storage */
   jmethodID getContentDir = env->GetMethodID(acl, "getFilesDir", "()Ljava/io/File;");
   jobject file = env->CallObjectMethod(app->activity->clazz, getContentDir);
   jclass fileClass = env->FindClass("java/io/File");
   jmethodID getAbsolutePath = env->GetMethodID(fileClass, "getAbsolutePath", "()Ljava/lang/String;");
   jstring jpath = (jstring)env->CallObjectMethod(file, getAbsolutePath);
   MMDAgent_setContentDir(env->GetStringUTFChars(jpath, NULL));

   /* set paths */
   systemDirName = MMDAgent_strdup(path);
   MMDAgent_snprintf(buff, MMDAGENT_MAXBUFLEN, "%s%c%s", systemDirName, MMDAGENT_DIRSEPARATOR, "MMDAgent-EX.mdf");
   systemConfigFileName = MMDAgent_strdup(buff);
   pluginDirName = MMDAgent_strdup("/data/data/org.lee_lab.pocketmmdagent/lib");

   /* if AppData does not exist in system dir, download to ContentDir/_sys */
   MMDAgent_snprintf(buff, MMDAGENT_MAXBUFLEN, "%s%c%s", systemDirName, MMDAGENT_DIRSEPARATOR, "AppData");
   if (MMDAgent_existdir(buff)) {
      /* AppData exist on the same dir of executable: no download */
   } else {
      /* AppData not exist on the same dir of executable */
      contentDownloadDir = MMDAgent_contentdirdup();
      if (contentDownloadDir != NULL) {
         if (MMDAgent_existdir(contentDownloadDir) || MMDAgent_mkdir(contentDownloadDir) == true) {
            /* content dir exists */
            MMDAgent_snprintf(buff, MMDAGENT_MAXBUFLEN, "%s%c%s", contentDownloadDir, MMDAGENT_DIRSEPARATOR, "_sys");
            if (MMDAgent_existdir(buff) || MMDAgent_mkdir(buff) == true) {
               /* system cache dir under content dir exists */
               free(systemDirName);
               systemDirName = MMDAgent_strdup(buff);
               sysDownloadURI = MMDAgent_strdup(SYSTEMDOWNLOADURI);
            }
         }
         free(contentDownloadDir);
      }
   }

   argv[0] = argv[1] = NULL;
   argv[0] = MMDAgent_strdup("dummy.exe");
   argc = 1;

   /* get intent URI */
   jmethodID giid = env->GetMethodID(acl, "getIntent", "()Landroid/content/Intent;");
   jobject intent = env->CallObjectMethod(me, giid); //Got our intent
   if (intent) {
      jclass icl = env->GetObjectClass(intent); //class pointer of Intent
      jobject u = env->CallObjectMethod(intent,env->GetMethodID(icl, "getData", "()Landroid/net/Uri;")); //Got our intent
      if (u) {
         jclass ics = env->GetObjectClass(u); //class pointer of the URI
         jstring scm = (jstring) env->CallObjectMethod(u, env->GetMethodID(ics, "toString", "()Ljava/lang/String;"));
         const char *uripath = env->GetStringUTFChars(scm, 0);
         if (MMDAgent_strheadmatch(uripath, "mmdagent")) {
            argv[argc] = MMDAgent_strdup(uripath);
         } else {
            jstring urs = (jstring) env->CallObjectMethod(u, env->GetMethodID(ics, "getPath", "()Ljava/lang/String;"));
            const char *Param1 = env->GetStringUTFChars(urs, 0);
            argv[argc] = MMDAgent_strdup(Param1);
            env->ReleaseStringUTFChars(urs, Param1);
         }
         env->ReleaseStringUTFChars(scm, uripath);
         argc++;
      }
   }

   /* run MMDAgent */
   result = commonMain(argc, argv, systemDirName, pluginDirName, systemConfigFileName, sysDownloadURI);

   /* free */
   for(i = 0; i < 2; i++)
      if (argv[i]) free(argv[i]);
   free(systemDirName);
   free(pluginDirName);
   free(systemConfigFileName);
   if (sysDownloadURI)
      free(sysDownloadURI);

   app->activity->vm->DetachCurrentThread();
}
#endif /* __ANDROID__ */
#if !defined(_WIN32) && !defined(__APPLE__) && !defined(__ANDROID__)
int main(int argc, char **argv)
{
   int i;
   iconv_t ic;
   char **newArgv;
   char inBuff[PATH_MAX + 1], outBuff[PATH_MAX + 1];
   char *inStr, *outStr;
   size_t inLen, outLen;
   int result = 0;

   setlocale(LC_CTYPE, "");

   ic = iconv_open("UTF-8", "");
   if(ic == (iconv_t) -1) {
      fprintf(stderr, "Error: failed to open iconv");
      return -1;
   }

   newArgv = (char **) malloc(sizeof(char *) * argc);
   for(i = 0; i < argc; i++) {

      if (argv[i][0] == '-' || MMDAgent_strheadmatch(argv[i], "http:") || MMDAgent_strheadmatch(argv[i], "https:") || MMDAgent_strheadmatch(argv[i], "mmdagent:")) {
         newArgv[i] = MMDAgent_strdup(argv[i]);
         continue;
      }

      /* prepare buffer */
      memset(inBuff, 0, PATH_MAX + 1);
      memset(outBuff, 0, PATH_MAX + 1);
      if (realpath(argv[i], inBuff) == NULL) {
         fprintf(stderr, "Error: failed to resolve real path: %s", argv[i]);
         return -1;
      }

      inStr = &inBuff[0];
      outStr = &outBuff[0];

      inLen = MMDAgent_strlen(inStr);
      outLen = MMDAGENT_MAXBUFLEN;

      if(iconv(ic, &inStr, &inLen, &outStr, &outLen) < 0) {
         result = -1;
         strcpy(outBuff, "");
      }

      newArgv[i] = MMDAgent_strdup(outBuff);
   }

   iconv_close(ic);

   if(result >= 0)
      result = commonMainExec(argc, newArgv);

   for(i = 0; i < argc; i++) {
      if(newArgv[i] != NULL)
         free(newArgv[i]);
   }
   free(newArgv);

   return result;
}
#endif /* !_WIN32 && !__APPLE__ && !__ANDROID__ */
