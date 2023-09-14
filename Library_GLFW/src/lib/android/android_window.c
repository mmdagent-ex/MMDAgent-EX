//========================================================================
// GLFW - An OpenGL framework
// Platform:    X11/GLX
// API version: 2.7
// WWW:         http://www.glfw.org/
//------------------------------------------------------------------------
// Copyright (c) 2002-2006 Marcus Geelnard
// Copyright (c) 2006-2010 Camilla Berglund <elmindreda@elmindreda.org>
//
// This software is provided 'as-is', without any express or implied
// warranty. In no event will the authors be held liable for any damages
// arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would
//    be appreciated but is not required.
//
// 2. Altered source versions must be plainly marked as such, and must not
//    be misrepresented as being the original software.
//
// 3. This notice may not be removed or altered from any source
//    distribution.
//
//========================================================================

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

#include "internal.h"

#include <limits.h>

#include <jni.h>
#include <errno.h>
#include <android/sensor.h>
#include <android_native_app_glue.h>
#include <EGL/egl.h>
#include <unistd.h>
#include <sys/time.h>
#include "android/log.h"

#define MOUSEMOVEDETECTRESOLUTION 256

//************************************************************************
//****               Platform implementation functions                ****
//************************************************************************

//========================================================================
// Here is where the window is created, and
// the OpenGL rendering context is created
//========================================================================

int _glfwPlatformOpenWindow( int width, int height,
                             const _GLFWwndconfig* wndconfig,
                             const _GLFWfbconfig* fbconfig )
{
    while( _glfwWin.initialized != GL_TRUE ) {
        _glfwPlatformPollEvents();
    }

    {
        const EGLint attribs[] = { EGL_SURFACE_TYPE, EGL_WINDOW_BIT, EGL_BLUE_SIZE, 8, EGL_GREEN_SIZE, 8, EGL_RED_SIZE, 8, EGL_DEPTH_SIZE, 16, EGL_STENCIL_SIZE, 8, EGL_NONE };
        EGLint w, h, format;
        EGLint numConfigs;
        EGLConfig config;
        EGLSurface surface;
        EGLContext context;
        EGLDisplay display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
        eglInitialize(display, 0, 0);
        eglChooseConfig(display, attribs, &config, 1, &numConfigs);
        eglGetConfigAttrib(display, config, EGL_NATIVE_VISUAL_ID, &format);
        ANativeWindow_setBuffersGeometry(_glfwWin.app->window, 0, 0, format
);
        surface = eglCreateWindowSurface(display, config, _glfwWin.app->window, NULL);
        context = eglCreateContext(display, config, NULL, NULL);
        if (eglMakeCurrent(display, surface, surface, context) == EGL_FALSE) {
           return GL_FALSE;
        }

        eglQuerySurface(display, surface, EGL_WIDTH, &w);
        eglQuerySurface(display, surface, EGL_HEIGHT, &h);

        _glfwWin.display = display;
        _glfwWin.context = context;
        _glfwWin.surface = surface;
        _glfwWin.width = w;
        _glfwWin.height = h;
        _glfwWin.opened = GL_TRUE;
        _glfwWin.resized = GL_TRUE;
    }
    return GL_TRUE;
}


//========================================================================
// Properly kill the window/video display
//========================================================================

void _glfwPlatformCloseWindow( void )
{
    if ( _glfwWin.display != EGL_NO_DISPLAY ) {
        eglMakeCurrent( _glfwWin.display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT );
        if ( _glfwWin.context != EGL_NO_CONTEXT ) {
            eglDestroyContext( _glfwWin.display, _glfwWin.context );
        }
        if ( _glfwWin.surface != EGL_NO_SURFACE ) {
            eglDestroySurface( _glfwWin.display, _glfwWin.surface );
        }
        eglTerminate(_glfwWin.display);
    }
    _glfwWin.display = EGL_NO_DISPLAY;
    _glfwWin.context = EGL_NO_CONTEXT;
    _glfwWin.surface = EGL_NO_SURFACE;
    _glfwWin.opened = GL_FALSE;
    _glfwWin.initialized = GL_FALSE;
}


//========================================================================
// Set the window title
//========================================================================

void _glfwPlatformSetWindowTitle( const char *title )
{
}

#ifdef MMDAGENT
void _glfwPlatformEnableFullScreen( void )
{
}

void _glfwPlatformDisableFullScreen( void )
{
}

void _glfwPlatformGetWindowPlacementInfo(int *x, int *y, int *width, int *height, int *maximized, int *fullscreen)
{
}

void _glfwPlatformSetWindowPlacementInfo(int x, int y, int width, int height, int maximized, int fullscreen)
{
}

void _glfwPlatformEnableTitleBar( void )
{
}

void _glfwPlatformDisableTitleBar( void )
{
}

#endif /* MMDAGENT */

//========================================================================
// Set the window size
//========================================================================

void _glfwPlatformSetWindowSize( int width, int height )
{
}


//========================================================================
// Set the window position.
//========================================================================

void _glfwPlatformSetWindowPos( int x, int y )
{
}


//========================================================================
// Window iconification
//========================================================================

void _glfwPlatformIconifyWindow( void )
{
}


//========================================================================
// Window un-iconification
//========================================================================

void _glfwPlatformRestoreWindow( void )
{
   const EGLint attribs[] = { EGL_SURFACE_TYPE, EGL_WINDOW_BIT, EGL_BLUE_SIZE, 8, EGL_GREEN_SIZE, 8, EGL_RED_SIZE, 8, EGL_DEPTH_SIZE, 16, EGL_STENCIL_SIZE, 8, EGL_NONE };
   EGLConfig config;
   EGLint numConfigs;
   EGLSurface surface;

   eglChooseConfig(_glfwWin.display, attribs, &config, 1, &numConfigs);
   surface = eglCreateWindowSurface(_glfwWin.display, config, _glfwWin.app->pendingWindow, NULL);
   eglMakeCurrent(_glfwWin.display, surface, surface, _glfwWin.context);
   _glfwWin.surface = surface;
}

//========================================================================
// Swap OpenGL ES buffers and poll any new events
//========================================================================

void _glfwPlatformSwapBuffers( void )
{
   if( _glfwWin.display != EGL_NO_DISPLAY && _glfwWin.surface != EGL_NO_SURFACE )
        eglSwapBuffers( _glfwWin.display, _glfwWin.surface );
}


//========================================================================
// Set double buffering swap interval
//========================================================================

void _glfwPlatformSwapInterval( int interval )
{
    if( _glfwWin.display != EGL_NO_DISPLAY )
        eglSwapInterval( _glfwWin.display, interval );
}


//========================================================================
// Read back framebuffer parameters from the context
//========================================================================

void _glfwPlatformRefreshWindowParams( void )
{
}


//========================================================================
// Poll for new window and input events
//========================================================================

void _glfwPlatformPollEvents( void )
{
    EGLint w, h;
    int events;
    struct android_poll_source *source;

    // Callback resized window event
    if(_glfwWin.display != EGL_NO_DISPLAY && _glfwWin.surface != EGL_NO_SURFACE) {
       /* check every frame the screen size and resize if changed */
       eglQuerySurface( _glfwWin.display, _glfwWin.surface, EGL_WIDTH, &w );
       eglQuerySurface( _glfwWin.display, _glfwWin.surface, EGL_HEIGHT, &h );
       if (_glfwWin.width != w || _glfwWin.height != h || _glfwWin.resized == GL_TRUE) {
          _glfwWin.width = w;
          _glfwWin.height = h;
          if( _glfwWin.windowSizeCallback ) {
             _glfwWin.windowSizeCallback( _glfwWin.width, _glfwWin.height );
          }
       }
       _glfwWin.resized = GL_FALSE;
    }

    // Callback other events
    while (ALooper_pollAll(0, NULL, &events, (void**) &source) >= 0) {
        if (source != NULL) {
            source->process(_glfwWin.app, source);
        }
        if(_glfwWin.app->destroyRequested != 0){
           _glfwWin.app->destroyRequested = 0;
           _glfwWin.opened = GL_FALSE;
           return;
        }
    }
}


//========================================================================
// Wait for new window and input events
//========================================================================

void _glfwPlatformWaitEvents( void )
{
}


//========================================================================
// Hide mouse cursor (lock it)
//========================================================================

void _glfwPlatformHideMouseCursor( void )
{
}


//========================================================================
// Show mouse cursor (unlock it)
//========================================================================

void _glfwPlatformShowMouseCursor( void )
{
}


//========================================================================
// Set physical mouse cursor position
//========================================================================

void _glfwPlatformSetMouseCursorPos( int x, int y )
{
}

void _glfwPlatformProcWindowEvent( struct android_app* app, int32_t cmd )
{
    switch (cmd) {
    case APP_CMD_INPUT_CHANGED:
        break;
    case APP_CMD_INIT_WINDOW:
        _glfwWin.initialized = GL_TRUE;
        if (_glfwWin.opened == GL_TRUE)
           _glfwPlatformRestoreWindow();
        break;
    case APP_CMD_TERM_WINDOW:
       if (_glfwWin.display != EGL_NO_DISPLAY && _glfwWin.surface != EGL_NO_SURFACE) {
          eglDestroySurface(_glfwWin.display, _glfwWin.surface);
          _glfwWin.surface = EGL_NO_SURFACE;
       }
        break;
    case APP_CMD_WINDOW_RESIZED:
        break;
    case APP_CMD_WINDOW_REDRAW_NEEDED:
        break;
    case APP_CMD_CONTENT_RECT_CHANGED:
        break;
    case APP_CMD_GAINED_FOCUS:
        break;
    case APP_CMD_LOST_FOCUS:
        break;
    case APP_CMD_CONFIG_CHANGED:
      break;
   case APP_CMD_LOW_MEMORY:
      break;
   case APP_CMD_START:
      break;
   case APP_CMD_RESUME:
      break;
   case APP_CMD_SAVE_STATE:
      break;
   case APP_CMD_PAUSE:
      break;
   case APP_CMD_STOP:
      break;
   case APP_CMD_DESTROY:
      break;
   default:
      break;
   }
   if(_glfwWin.additionalCppCmd)
      _glfwWin.additionalCppCmd(app, cmd);
}

int32_t _glfwPlatformProcInputEvent( struct android_app* app, AInputEvent *event )
{
    // Motion events
    if ( AInputEvent_getType(event) == AINPUT_EVENT_TYPE_MOTION ) {
        int action = AKeyEvent_getAction(event);
        int count = AMotionEvent_getPointerCount(event);
        int id;
        int32_t index;
        switch (action & AMOTION_EVENT_ACTION_MASK) {
            case AMOTION_EVENT_ACTION_DOWN:
               /* first finger down, start tracking and send press event */
               _glfwInput.MousePosX = AMotionEvent_getX(event, 0);
               _glfwInput.MousePosY = AMotionEvent_getY(event, 0);
               _glfwInput.MouseId = AMotionEvent_getPointerId(event, 0);
               _glfwInput.MouseId2 = -1;
               if(_glfwWin.mousePosCallback)
                  _glfwWin.mousePosCallback(_glfwInput.MousePosX, _glfwInput.MousePosY, GLFW_RELEASE, GLFW_RELEASE);
               _glfwInputMouseClick(GLFW_MOUSE_BUTTON_LEFT, GLFW_PRESS);
                break;
            case AMOTION_EVENT_ACTION_POINTER_DOWN:
               /* second finger down: begin pinch mode, send pseudo release event */
               if (count == 2) {
                  index = (action & AMOTION_EVENT_ACTION_POINTER_INDEX_MASK) >> AMOTION_EVENT_ACTION_POINTER_INDEX_SHIFT;
                  _glfwInput.MousePosX2 = AMotionEvent_getX(event, index);
                  _glfwInput.MousePosY2 = AMotionEvent_getY(event, index);
                  _glfwInput.MouseId2 = AMotionEvent_getPointerId(event, index);
                  _glfwInputMouseClick(GLFW_MOUSE_BUTTON_LEFT, GLFW_RELEASE);
               }
               break;
            case AMOTION_EVENT_ACTION_UP:
               /* released last finger, do nothing on pinch mode, else release event */
               if (_glfwInput.MouseId2 == -1) {
                  _glfwInput.MousePosX = AMotionEvent_getX(event, 0);
                  _glfwInput.MousePosY = AMotionEvent_getY(event, 0);
                  if(_glfwWin.mousePosCallback)
                     _glfwWin.mousePosCallback(_glfwInput.MousePosX, _glfwInput.MousePosY, GLFW_RELEASE, GLFW_RELEASE);
                  _glfwInputMouseClick(GLFW_MOUSE_BUTTON_LEFT, GLFW_RELEASE);
               }
               /* end pinch mode */
               _glfwInput.MouseId = -1;
               _glfwInput.MouseId2 = -1;
               break;
            case AMOTION_EVENT_ACTION_POINTER_UP:
               /* released a finger, do nothing */
               break;
            case AMOTION_EVENT_ACTION_MOVE:
                if (count == 1) {
                   /* moving with one finger */
                   /* send move event only if not pinch mode */
                   if (_glfwInput.MouseId2 == -1) {
                      _glfwInput.MousePosX = AMotionEvent_getX(event, 0);
                      _glfwInput.MousePosY = AMotionEvent_getY(event, 0);
                      if(_glfwWin.mousePosCallback)
                         _glfwWin.mousePosCallback(_glfwInput.MousePosX, _glfwInput.MousePosY, GLFW_RELEASE, GLFW_RELEASE);
                   }
                } else if (count == 2 && _glfwInput.MouseId != -1) {
                   /* moving with two fingers */
                    int old_x1 = (int) _glfwInput.MousePosX;
                    int old_y1 = (int) _glfwInput.MousePosY;
                    int old_x2 = (int) _glfwInput.MousePosX2;
                    int old_y2 = (int) _glfwInput.MousePosY2;
                    int new_x1, new_y1, new_x2, new_y2;
                    if (AMotionEvent_getPointerId(event, 0) == _glfwInput.MouseId) {
                       new_x1 = (int) AMotionEvent_getX(event, 0);
                       new_y1 = (int) AMotionEvent_getY(event, 0);
                       new_x2 = (int) AMotionEvent_getX(event, 1);
                       new_y2 = (int) AMotionEvent_getY(event, 1);
                    } else {
                       new_x1 = (int) AMotionEvent_getX(event, 1);
                       new_y1 = (int) AMotionEvent_getY(event, 1);
                       new_x2 = (int) AMotionEvent_getX(event, 0);
                       new_y2 = (int) AMotionEvent_getY(event, 0);
                    }
                    if (old_x1 == new_x1 && old_y1 == new_y1) {
                    } else if (old_x2 == new_x2 && old_y2 == new_y2) {
                    } else {
                        if (abs((new_x2 - new_x1) * (new_y2 - new_y1)) > abs((old_x2 - old_x1) * (old_y2 - old_y1))){
                            _glfwInput.WheelPos++;
                        } else {
                            _glfwInput.WheelPos--;
                        }
                        if (_glfwWin.mouseWheelCallback)
                             _glfwWin.mouseWheelCallback(_glfwInput.WheelPos);
                    }
                    _glfwInput.MousePosX  = new_x1;
                    _glfwInput.MousePosY  = new_y1;
                    _glfwInput.MousePosX2 = new_x2;
                    _glfwInput.MousePosY2 = new_y2;
                }
                break;
        }
        return 1;
    }
    return 0;
}
