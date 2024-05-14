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

/* headers */

#include "MMDAgent.h"

#ifndef WINDOWSIZESAVEPATH
#define WINDOWSIZESAVEPATH "_window"
#endif

/* ScreenWindow::initialize: initialize screen */
void ScreenWindow::initialize()
{
   m_enable = false;

   m_vsync = true;
   m_intervalFrameOfVsync = 1;
   m_numMultiSampling = 0;
   m_showMouse = true;
   m_mouseActiveLeftFrame = 0.0;
   m_fullScreen = false;
   m_HideTitleBar = false;

#ifdef MMDAGENT_TRANSPARENT_WINDOW
   m_transparentWindow = false;
#endif /* MMDAGENT_TRANSPARENT_WINDOW */
}

/* ScreenWindow::clear: free screen */
void ScreenWindow::clear()
{
   initialize();
}

/* ScreenWindow::ScreenWindow: constructor */
ScreenWindow::ScreenWindow()
{
   initialize();
}

/* ScreenWindow::ScreenWindow: destructor */
ScreenWindow::~ScreenWindow()
{
   clear();
}

/* ScreenWindow::setup: create window */
bool ScreenWindow::setup(const int *size, const char *title, int maxMultiSampling)
{
   clear();

#ifdef _WIN32
   // tell OS that this application is DPI-aware
   SetProcessDPIAware();
#endif

   /* try to set number of multi-sampling */
   glfwOpenWindowHint(GLFW_FSAA_SAMPLES, maxMultiSampling);

   /* create window */
   if(glfwOpenWindow(size[0], size[1], 8, 8, 8, 8, 24, 8, GLFW_WINDOW) == GL_FALSE) {
      return false;
   }

   /* initialize glew */
   glewInit();

   /* store number of multi-sampling */
   m_numMultiSampling = glfwGetWindowParam(GLFW_FSAA_SAMPLES);

   /* set title */
   glfwSetWindowTitle(title);
   m_HideTitleBar = false;

   /* set vertical sync. */
   if (glfwExtensionSupported("WGL_EXT_swap_control_tear") == GL_TRUE || glfwExtensionSupported("GLX_EXT_swap_control_tear") == GL_TRUE)
      m_intervalFrameOfVsync = -1;
   else
      m_intervalFrameOfVsync = 1;

   glfwSwapInterval(m_intervalFrameOfVsync);

#ifdef MMDAGENT_TRANSPARENT_WINDOW
   /* set initial transparent state */
   m_transparentWindow = false;
#endif /* MMDAGENT_TRANSPARENT_WINDOW */

   m_enable = true;
   return true;
}

/* ScreenWindow::setTransparentWindow: set transparent window */
bool ScreenWindow::setTransparentWindow(const float *transparentColor)
{
#ifdef MMDAGENT_TRANSPARENT_WINDOW
   if (transparentColor) {
      glfwEnableTransparent(transparentColor);
      m_transparentWindow = true;
   } else {
      glfwDisableTransparent();
      m_transparentWindow = false;
   }
   return true;
#else /* ~MMDAGENT_TRANSPARENT_WINDOW */
   return false;
#endif /* MMDAGENT_TRANSPARENT_WINDOW */
}

/* ScreenWindow::isTransparentWindow: return true when transparent window */
bool ScreenWindow::isTransparentWindow()
{
   return m_transparentWindow;
}

/* ScreenWindow::swapBuffers: swap buffers */
void ScreenWindow::swapBuffers()
{
   if(m_enable == false)
      return;

   glfwSwapBuffers();
}

/* ScreenWindow::getVSync: get vertical sync. flag */
bool ScreenWindow::getVSync()
{
   return m_vsync;
}

/* ScreenWindow::toggleVSync: toggle vertical sync. flag */
void ScreenWindow::toggleVSync()
{
   if(m_enable == false)
      return;

   if(m_vsync == true) {
      glfwSwapInterval(0);
      m_vsync = false;
   } else {
      glfwSwapInterval(m_intervalFrameOfVsync);
      m_vsync = true;
   }
}

/* ScreenWindow::getNumMultiSmapling: get number of multi-sampling */
int ScreenWindow::getNumMultiSampling()
{
   return m_numMultiSampling;
}

/* ScreenWindow::setMouseActiveTime: set mouse active time */
void ScreenWindow::setMouseActiveTime(double frame)
{
#ifndef MMDAGENT_DONTUSEMOUSE
   if(m_enable == false)
      return;

   m_mouseActiveLeftFrame = frame;

   /* if full screen, disable mouse cursor */
   if (m_fullScreen == true && m_showMouse == false) {
      m_showMouse = true;
      glfwEnable(GLFW_MOUSE_CURSOR);
   }
#endif /* !MMDAGENT_DONTUSEMOUSE */
}

/* ScreenWindow::updateMouseActiveTime: update mouse active time */
void ScreenWindow::updateMouseActiveTime(double frame)
{
#ifndef MMDAGENT_DONTUSEMOUSE
   if(m_enable == false)
      return;

   if(m_mouseActiveLeftFrame == 0.0)
      return;

   m_mouseActiveLeftFrame -= frame;
   if (m_mouseActiveLeftFrame <= 0.0) {
      m_mouseActiveLeftFrame = 0.0;
      /* if full screen, disable mouse cursor */
      if (m_fullScreen == true && m_showMouse == true) {
         m_showMouse = false;
         glfwDisable(GLFW_MOUSE_CURSOR);
      }
   }
#endif /* !MMDAGENT_DONTUSEMOUSE */
}

/* ScreenWindow::setFullScreen: set fullscreen */
void ScreenWindow::setFullScreen()
{
#ifndef MMDAGENT_DONTUSEWINDOW
   if(m_enable == false)
      return;

   /* set full screen */
   if(m_fullScreen == false) {
      glfwEnableFullScreen();
      m_fullScreen = true;
   }

   /* disable mouse cursor */
   if (m_showMouse == true) {
      m_showMouse = false;
      glfwDisable(GLFW_MOUSE_CURSOR);
   }
#endif /* !MMDAGENT_DONTUSEWINDOW */
}

/* ScreenWindow::exitFullScreen: exit fullscreen */
void ScreenWindow::exitFullScreen()
{
#ifndef MMDAGENT_DONTUSEWINDOW
   if(m_enable == false)
      return;

   /* exit full screen */
   if(m_fullScreen == true) {
      glfwDisableFullScreen();
      m_fullScreen = false;
   }

   /* enable mouse cursor */
   if (m_showMouse == false) {
      m_showMouse = true;
      glfwEnable(GLFW_MOUSE_CURSOR);
   }
#endif /* !MMDAGENT_DONTUSEWINDOW */
}

/* ScreenWindow::saveWindowPosition: save current window postion to file */
void ScreenWindow::saveWindowPosition()
{
   char *contentDirName;
   char buff[MMDAGENT_MAXBUFLEN];
   KeyValue *v;
   int x, y, width, height;
   int maximized, fullscreen;
   bool err;

   contentDirName = MMDAgent_contentDirMakeDup();
   if (contentDirName == NULL)
      return;
   MMDAgent_snprintf(buff, MMDAGENT_MAXBUFLEN, "%s%c%s", contentDirName, MMDAGENT_DIRSEPARATOR, WINDOWSIZESAVEPATH);
   free(contentDirName);

   err = 1;

#ifdef _WIN32
   glfwGetWindowPlacementInfo(&x, &y, &width, &height, &maximized, &fullscreen);
   err = 0;
#endif /* _WIN32 */

   if (err == 0) {
      v = new KeyValue;
      v->setup();
      v->setString("x", "%d", x);
      v->setString("y", "%d", y);
      v->setString("width", "%d", width);
      v->setString("height", "%d", height);
      v->setString("maximized", "%d", maximized ? 1 : 0);
      v->setString("fullscreen", "%d", fullscreen ? 1 : 0);
      v->setString("hidetitlebar", "%d", m_HideTitleBar ? 1 : 0);
      v->save(buff);
      delete v;
   }
}

/* ScreenWindow::loadWindowPosition: load window position from file */
bool ScreenWindow::loadWindowPosition(bool *fullscreenized)
{
   char *contentDirName;
   char buff[MMDAGENT_MAXBUFLEN];
   KeyValue *v;
   int x, y, width, height, maximized, fullscreen, hidetitlebar;

   contentDirName = MMDAgent_contentdirdup();
   if (contentDirName == NULL)
      return false;
   MMDAgent_snprintf(buff, MMDAGENT_MAXBUFLEN, "%s%c%s", contentDirName, MMDAGENT_DIRSEPARATOR, WINDOWSIZESAVEPATH);
   free(contentDirName);
   v = new KeyValue;
   v->setup();
   if (v->load(buff, NULL) == false) {
      delete v;
      return false;
   }
   x = atoi(v->getString("x", NULL));
   y = atoi(v->getString("y", NULL));
   width = atoi(v->getString("width", NULL));
   height = atoi(v->getString("height", NULL));
   maximized = (atoi(v->getString("maximized", NULL)) != 0) ? true : false;
   fullscreen = (atoi(v->getString("fullscreen", NULL)) != 0) ? true : false;
   hidetitlebar = (atoi(v->getString("hidetitlebar", NULL)) != 0) ? true : false;
   delete v;

#ifdef _WIN32
   glfwSetWindowPlacementInfo(x, y, width, height, maximized, fullscreen);
   if (fullscreen != 0) {
      setFullScreen();
      *fullscreenized = true;
   } else {
      exitFullScreen();
      *fullscreenized = false;
   }
   if (m_HideTitleBar == false && hidetitlebar != 0) {
      toggleTitleBar();
   } else if (m_HideTitleBar == true && hidetitlebar == 0) {
      toggleTitleBar();
   }
#endif /* _WIN32 */

   return true;
}

/* ScreenWindow::toggleTitleBar: toggle window title bar */
void ScreenWindow::toggleTitleBar()
{
   if (m_HideTitleBar == false) {
      /* hide title bar */
      glfwDisableTitleBar();
      m_HideTitleBar = true;
   } else {
      /* show title bar */
      glfwEnableTitleBar();
      m_HideTitleBar = false;
   }
}

/* ScreenWindow::getTitleBarHide: get if title bar is hiding */
bool ScreenWindow::getTitleBarHide()
{
   return m_HideTitleBar;
}

/* ScreenWindow::setWindowSize: set window size */
void ScreenWindow::setWindowSize(int width, int height)
{
   glfwSetWindowSize(width, height);
}
