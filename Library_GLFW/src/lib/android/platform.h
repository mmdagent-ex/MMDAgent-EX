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

#ifndef _platform_h_
#define _platform_h_


// This is the X11 version of GLFW
#define _GLFW_Android


// Include files
#include <sys/time.h>
#include <unistd.h>
#include <signal.h>

#include <jni.h>
#include <errno.h>
#include <android/sensor.h>
#include <EGL/egl.h>
#include <unistd.h>
#include <sys/time.h>

#include "../../include/GL/glfw.h"

// Do we have pthread support?
#ifdef _GLFW_HAS_PTHREAD
 #include <pthread.h>
 #include <sched.h>
#endif

// We support two different ways for getting the number of processors in
// the system: sysconf (POSIX) and sysctl (BSD?)
#if defined( _GLFW_HAS_SYSCONF )

 // Use a single constant for querying number of online processors using
 // the sysconf function (e.g. SGI defines _SC_NPROC_ONLN instead of
 // _SC_NPROCESSORS_ONLN)
 #ifndef _SC_NPROCESSORS_ONLN
  #ifdef  _SC_NPROC_ONLN
   #define _SC_NPROCESSORS_ONLN _SC_NPROC_ONLN
  #else
   #error POSIX constant _SC_NPROCESSORS_ONLN not defined!
  #endif
 #endif

 // Macro for querying the number of processors
 #define _glfw_numprocessors(n) n=(int)sysconf(_SC_NPROCESSORS_ONLN)

#elif defined( _GLFW_HAS_SYSCTL )

 #include <sys/types.h>
 #include <sys/sysctl.h>

 // Macro for querying the number of processors
 #define _glfw_numprocessors(n) { \
  int mib[2], ncpu; \
  size_t len = 1; \
  mib[0] = CTL_HW; \
  mib[1] = HW_NCPU; \
  n      = 1; \
  if( sysctl( mib, 2, &ncpu, &len, NULL, 0 ) != -1 ) \
    { \
  if( len > 0 ) \
    { \
  n = ncpu; \
    } \
  } \
  }

#else

 // If neither sysconf nor sysctl is supported, assume single processor
 // system
 #define _glfw_numprocessors(n) n=1

#endif

// Pointer length integer
// One day, this will most likely move into glfw.h
typedef intptr_t GLFWintptr;


#ifndef GL_VERSION_3_0

typedef const GLubyte * (APIENTRY *PFNGLGETSTRINGIPROC) (GLenum, GLuint);

#endif /*GL_VERSION_3_0*/

//========================================================================
// Global variables (GLFW internals)
//========================================================================

//------------------------------------------------------------------------
// Window structure
//------------------------------------------------------------------------
typedef struct _GLFWwin_struct _GLFWwin;

struct _GLFWwin_struct {

// ========= PLATFORM INDEPENDENT MANDATORY PART =========================

    // User callback functions
    GLFWwindowsizefun    windowSizeCallback;
    GLFWwindowclosefun   windowCloseCallback;
    GLFWwindowrefreshfun windowRefreshCallback;
#ifdef MMDAGENT
    GLFWdropfilefun      dropFileCallback;
    GLFWurlschemefun     urlSchemeCallback;
#endif /* MMDAGENT */
    GLFWmousebuttonfun   mouseButtonCallback;
    GLFWmouseposfun      mousePosCallback;
    GLFWmousewheelfun    mouseWheelCallback;
    GLFWkeyfun           keyCallback;
    GLFWcharfun          charCallback;

    // User selected window settings
    int       fullscreen;      // Fullscreen flag
    int       mouseLock;       // Mouse-lock flag
    int       autoPollEvents;  // Auto polling flag
    int       sysKeysDisabled; // System keys disabled flag
    int       windowNoResize;  // Resize- and maximize gadgets disabled flag
    int       refreshRate;     // Vertical monitor refresh rate

    // Window status & parameters
    int       opened;          // Flag telling if window is opened or not
    int       active;          // Application active flag
    int       iconified;       // Window iconified flag
    int       width, height;   // Window width and heigth
    int       accelerated;     // GL_TRUE if window is HW accelerated

    // Framebuffer attributes
    int       redBits;
    int       greenBits;
    int       blueBits;
    int       alphaBits;
    int       depthBits;
    int       stencilBits;
    int       accumRedBits;
    int       accumGreenBits;
    int       accumBlueBits;
    int       accumAlphaBits;
    int       auxBuffers;
    int       stereo;
    int       samples;

    // OpenGL extensions and context attributes
    int       has_GL_SGIS_generate_mipmap;
    int       has_GL_ARB_texture_non_power_of_two;
    int       glMajor, glMinor, glRevision;
    int       glForward, glDebug, glProfile;

    PFNGLGETSTRINGIPROC GetStringi;


// ========= PLATFORM SPECIFIC PART ======================================

    // Platform specific window resources
    struct android_app *app;
    void (*additionalCppCmd)(struct android_app *app, int32_t cmd);
    EGLDisplay display;
    EGLContext context;
    EGLSurface surface;
    int initialized;
    int resized;
};

GLFWGLOBAL _GLFWwin _glfwWin;


//------------------------------------------------------------------------
// User input status (most of this should go in _GLFWwin)
//------------------------------------------------------------------------
GLFWGLOBAL struct {

// ========= PLATFORM INDEPENDENT MANDATORY PART =========================

    // Mouse status
   int  MousePosX, MousePosY, MouseId;
   int  MousePosX2, MousePosY2, MouseId2;
    int  WheelPos;
    char MouseButton[ GLFW_MOUSE_BUTTON_LAST+1 ];

    // Keyboard status
    char Key[ GLFW_KEY_LAST+1 ];
    int  LastChar;

    // User selected settings
    int  StickyKeys;
    int  StickyMouseButtons;
    int  KeyRepeat;


// ========= PLATFORM SPECIFIC PART ======================================

    // Platform specific internal variables
    int  MouseMoved, CursorPosX, CursorPosY;

} _glfwInput;


//------------------------------------------------------------------------
// Library global data
//------------------------------------------------------------------------
GLFWGLOBAL struct {

// ========= PLATFORM INDEPENDENT MANDATORY PART =========================

    // Window opening hints
    _GLFWhints      hints;

// ========= PLATFORM SPECIFIC PART ======================================

    // Timer data
    struct {
        double      resolution;
        long long   t0;
    } Timer;

} _glfwLibrary;


//------------------------------------------------------------------------
// Thread record (one for each thread)
//------------------------------------------------------------------------
typedef struct _GLFWthread_struct _GLFWthread;

struct _GLFWthread_struct {

// ========= PLATFORM INDEPENDENT MANDATORY PART =========================

    // Pointer to previous and next threads in linked list
    _GLFWthread   *Previous, *Next;

    // GLFW user side thread information
    GLFWthread    ID;
    GLFWthreadfun Function;

// ========= PLATFORM SPECIFIC PART ======================================

    // System side thread information
#ifdef _GLFW_HAS_PTHREAD
    pthread_t     PosixID;
#endif

};

//------------------------------------------------------------------------
// General thread information
//------------------------------------------------------------------------
GLFWGLOBAL struct {

// ========= PLATFORM INDEPENDENT MANDATORY PART =========================

    // Next thread ID to use (increments for every created thread)
    GLFWthread       NextID;

    // First thread in linked list (always the main thread)
    _GLFWthread      First;

// ========= PLATFORM SPECIFIC PART ======================================

    // Critical section lock
#ifdef _GLFW_HAS_PTHREAD
    pthread_mutex_t  CriticalSection;
#endif

} _glfwThrd;


//========================================================================
// Macros for encapsulating critical code sections (i.e. making parts
// of GLFW thread safe)
//========================================================================

// Thread list management
#ifdef _GLFW_HAS_PTHREAD
 #define ENTER_THREAD_CRITICAL_SECTION \
         pthread_mutex_lock( &_glfwThrd.CriticalSection );
 #define LEAVE_THREAD_CRITICAL_SECTION \
         pthread_mutex_unlock( &_glfwThrd.CriticalSection );
#else
 #define ENTER_THREAD_CRITICAL_SECTION
 #define LEAVE_THREAD_CRITICAL_SECTION
#endif


//========================================================================
// Prototypes for platform specific internal functions
//========================================================================

// Time
void _glfwInitTimer( void );


#endif // _platform_h_
