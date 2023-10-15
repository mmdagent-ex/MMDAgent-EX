//========================================================================
// GLFW - An OpenGL framework
// Platform:    Any
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

#ifndef _internal_h_
#define _internal_h_

#ifdef __ANDROID__
#include <jni.h>
#include <errno.h>
#include <android/sensor.h>
#include <android_native_app_glue.h>
#include <EGL/egl.h>
#include <unistd.h>
#include <sys/time.h>
#endif

#if TARGET_OS_IPHONE
#include "../include/GL/glfw.h"
#endif

//========================================================================
// GLFWGLOBAL is a macro that places all global variables in the init.c
// module (all other modules reference global variables as 'extern')
//========================================================================

#if defined( _init_c_ )
#define GLFWGLOBAL
#else
#define GLFWGLOBAL extern
#endif


//========================================================================
// Input handling definitions
//========================================================================

// Internal key and button state/action definitions
#define GLFW_STICK 2


//========================================================================
// System independent include files
//========================================================================

#include <stdlib.h>
#include <string.h>
#include <stdio.h>


//------------------------------------------------------------------------
// Window opening hints (set by glfwOpenWindowHint)
// A bucket of semi-random stuff bunched together for historical reasons
// This is used only by the platform independent code and only to store
// parameters passed to us by glfwOpenWindowHint
//------------------------------------------------------------------------
typedef struct {
    int         refreshRate;
    int         accumRedBits;
    int         accumGreenBits;
    int         accumBlueBits;
    int         accumAlphaBits;
    int         auxBuffers;
    int         stereo;
    int         windowNoResize;
    int         samples;
    int         glMajor;
    int         glMinor;
    int         glForward;
    int         glDebug;
    int         glProfile;
} _GLFWhints;


//------------------------------------------------------------------------
// Platform specific definitions goes in platform.h (which also includes
// glfw.h)
//------------------------------------------------------------------------

#include "platform.h"


//------------------------------------------------------------------------
// Parameters relating to the creation of the context and window but not
// directly related to the properties of the framebuffer
// This is used to pass window and context creation parameters from the
// platform independent code to the platform specific code
//------------------------------------------------------------------------
typedef struct {
    int         mode;
    int         refreshRate;
    int         windowNoResize;
    int         glMajor;
    int         glMinor;
    int         glForward;
    int         glDebug;
    int         glProfile;
} _GLFWwndconfig;


//------------------------------------------------------------------------
// Framebuffer configuration descriptor, i.e. buffers and their sizes
// Also a platform specific ID used to map back to the actual backend APIs
// This is used to pass framebuffer parameters from the platform independent
// code to the platform specific code, and also to enumerate and select
// available framebuffer configurations
//------------------------------------------------------------------------
typedef struct {
    int         redBits;
    int         greenBits;
    int         blueBits;
    int         alphaBits;
    int         depthBits;
    int         stencilBits;
    int         accumRedBits;
    int         accumGreenBits;
    int         accumBlueBits;
    int         accumAlphaBits;
    int         auxBuffers;
    int         stereo;
    int         samples;
    GLFWintptr  platformID;
} _GLFWfbconfig;


//========================================================================
// System independent global variables (GLFW internals)
//========================================================================

// Flag indicating if GLFW has been initialized
#if defined( _init_c_ )
int _glfwInitialized = 0;
#else
GLFWGLOBAL int _glfwInitialized;
#endif


//------------------------------------------------------------------------
// Abstract data stream (for image I/O)
//------------------------------------------------------------------------
typedef struct {
    FILE*   file;
    void*   data;
    long    position;
    long    size;
} _GLFWstream;


//========================================================================
// Prototypes for platform specific implementation functions
//========================================================================

// Init/terminate
int _glfwPlatformInit( void );
int _glfwPlatformTerminate( void );

// Enable/Disable
void _glfwPlatformEnableSystemKeys( void );
void _glfwPlatformDisableSystemKeys( void );

// Fullscreen
int  _glfwPlatformGetVideoModes( GLFWvidmode *list, int maxcount );
void _glfwPlatformGetDesktopMode( GLFWvidmode *mode );

// OpenGL extensions
int _glfwPlatformExtensionSupported( const char *extension );
void * _glfwPlatformGetProcAddress( const char *procname );

// Joystick
int _glfwPlatformGetJoystickParam( int joy, int param );
int _glfwPlatformGetJoystickPos( int joy, float *pos, int numaxes );
int _glfwPlatformGetJoystickButtons( int joy, unsigned char *buttons, int numbuttons );

// Threads
GLFWthread _glfwPlatformCreateThread( GLFWthreadfun fun, void *arg );
void _glfwPlatformDestroyThread( GLFWthread ID );
int _glfwPlatformWaitThread( GLFWthread ID, int waitmode );
GLFWthread _glfwPlatformGetThreadID( void );
GLFWmutex _glfwPlatformCreateMutex( void );
void _glfwPlatformDestroyMutex( GLFWmutex mutex );
void _glfwPlatformLockMutex( GLFWmutex mutex );
void _glfwPlatformUnlockMutex( GLFWmutex mutex );
GLFWcond _glfwPlatformCreateCond( void );
void _glfwPlatformDestroyCond( GLFWcond cond );
void _glfwPlatformWaitCond( GLFWcond cond, GLFWmutex mutex, double timeout );
void _glfwPlatformSignalCond( GLFWcond cond );
void _glfwPlatformBroadcastCond( GLFWcond cond );
int _glfwPlatformGetNumberOfProcessors( void );

// Time
double _glfwPlatformGetTime( void );
void _glfwPlatformSetTime( double time );
void _glfwPlatformSleep( double time );

// Window management
int  _glfwPlatformOpenWindow( int width, int height, const _GLFWwndconfig *wndconfig, const _GLFWfbconfig *fbconfig );
void _glfwPlatformCloseWindow( void );
void _glfwPlatformSetWindowTitle( const char *title );
#ifdef MMDAGENT
#ifdef _WIN32
HDC  _glfwPlatformGetDeviceContext( void );
void _glfwPlatformEnableTrackMouseLeave(void);
#endif /* _WIN32 */
void _glfwPlatformEnableFullScreen( void );
void _glfwPlatformDisableFullScreen( void );
void _glfwPlatformGetRenderingSize( int *width, int *height );
void _glfwPlatformGetWindowPlacementInfo(int *x, int *y, int *width, int *height, int *maximized, int *fullscreen);
void _glfwPlatformSetWindowPlacementInfo(int x, int y, int width, int height, int maximized, int fullscreen);
void _glfwPlatformEnableTitleBar(void);
void _glfwPlatformDisableTitleBar(void);
#endif /* MMDAGENT */
void _glfwPlatformSetWindowSize( int width, int height );
void _glfwPlatformSetWindowPos( int x, int y );
void _glfwPlatformIconifyWindow( void );
void _glfwPlatformRestoreWindow( void );
void _glfwPlatformSwapBuffers( void );
void _glfwPlatformSwapInterval( int interval );
void _glfwPlatformRefreshWindowParams( void );
void _glfwPlatformPollEvents( void );
void _glfwPlatformWaitEvents( void );
void _glfwPlatformHideMouseCursor( void );
void _glfwPlatformShowMouseCursor( void );
void _glfwPlatformSetMouseCursorPos( int x, int y );
#ifdef __ANDROID__
void _glfwPlatformProcWindowEvent( struct android_app *app, int32_t cmd );
int32_t _glfwPlatformProcInputEvent( struct android_app *app, AInputEvent *event );
#endif


//========================================================================
// Prototypes for platform independent internal functions
//========================================================================

// Window management (window.c)
void _glfwClearWindowHints( void );

// Input handling (window.c)
void _glfwClearInput( void );
void _glfwInputDeactivation( void );
void _glfwInputKey( int key, int action );
void _glfwInputChar( int character, int action );
void _glfwInputMouseClick( int button, int action );

// Threads (thread.c)
_GLFWthread * _glfwGetThreadPointer( int ID );
void _glfwAppendThread( _GLFWthread * t );
void _glfwRemoveThread( _GLFWthread * t );

// OpenGL extensions (glext.c)
void _glfwParseGLVersion( int *major, int *minor, int *rev );
int _glfwStringInExtensionString( const char *string, const GLubyte *extensions );
void _glfwRefreshContextParams( void );

// Abstracted data streams (stream.c)
int _glfwOpenFileStream( _GLFWstream *stream, const char *name, const char *mode );
int _glfwOpenBufferStream( _GLFWstream *stream, void *data, long size );
long _glfwReadStream( _GLFWstream *stream, void *data, long size );
long _glfwTellStream( _GLFWstream *stream );
int _glfwSeekStream( _GLFWstream *stream, long offset, int whence );
void _glfwCloseStream( _GLFWstream *stream );

// Targa image I/O (tga.c)
int _glfwReadTGA( _GLFWstream *s, GLFWimage *img, int flags );

// Framebuffer configs
const _GLFWfbconfig *_glfwChooseFBConfig( const _GLFWfbconfig *desired,
                                          const _GLFWfbconfig *alternatives,
                                          unsigned int count );


#endif // _internal_h_
