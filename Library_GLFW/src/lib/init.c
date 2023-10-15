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

#define _init_c_
#include "internal.h"


//************************************************************************
//****                    GLFW user functions                         ****
//************************************************************************

//========================================================================
// Initialize various GLFW state
//========================================================================

GLFWAPI int GLFWAPIENTRY glfwInit( void )
{
    // Is GLFW already initialized?
    if( _glfwInitialized )
    {
        return GL_TRUE;
    }

    memset( &_glfwLibrary, 0, sizeof( _glfwLibrary ) );
    memset( &_glfwWin, 0, sizeof( _glfwWin ) );

    // Window is not yet opened
    _glfwWin.opened = GL_FALSE;

    // Default enable/disable settings
    _glfwWin.sysKeysDisabled = GL_FALSE;

    // Clear window hints
    _glfwClearWindowHints();

    // Platform specific initialization
    if( !_glfwPlatformInit() )
    {
        return GL_FALSE;
    }

    // Form now on, GLFW state is valid
    _glfwInitialized = GL_TRUE;

    return GL_TRUE;
}

#ifdef __ANDROID__
GLFWAPI int GLFWAPIENTRY glfwInitForAndroid( void *app, void (*func)(struct android_app*, int32_t))
{
    glfwInit();
    _glfwWin.app = (struct android_app *) app;
    _glfwWin.app->onAppCmd = _glfwPlatformProcWindowEvent;
    _glfwWin.additionalCppCmd = func;
    _glfwWin.app->onInputEvent = _glfwPlatformProcInputEvent;
}
#endif

#if TARGET_OS_IPHONE
GLFWAPI int GLFWAPIENTRY glfwInitForIOS()
{
    glfwInit();
    return GL_TRUE;
}
#endif

//========================================================================
// Close window and kill all threads.
//========================================================================

GLFWAPI void GLFWAPIENTRY glfwTerminate( void )
{
    // Is GLFW initialized?
    if( !_glfwInitialized )
    {
        return;
    }

    // Platform specific termination
    if( !_glfwPlatformTerminate() )
    {
        return;
    }

    // GLFW is no longer initialized
    _glfwInitialized = GL_FALSE;
}


//========================================================================
// Get GLFW version
//========================================================================

GLFWAPI void GLFWAPIENTRY glfwGetVersion( int *major, int *minor, int *rev )
{
    if( major != NULL ) *major = GLFW_VERSION_MAJOR;
    if( minor != NULL ) *minor = GLFW_VERSION_MINOR;
    if( rev   != NULL ) *rev   = GLFW_VERSION_REVISION;
}
