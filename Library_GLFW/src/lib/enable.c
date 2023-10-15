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

#include "internal.h"


//************************************************************************
//****                  GLFW internal functions                       ****
//************************************************************************

//========================================================================
// Enable (show) mouse cursor
//========================================================================

static void enableMouseCursor( void )
{
#ifndef MMDAGENT
    int centerPosX, centerPosY;
#endif

    if( !_glfwWin.opened || !_glfwWin.mouseLock )
    {
        return;
    }

    // Show mouse cursor
    _glfwPlatformShowMouseCursor();

#ifndef MMDAGENT

    centerPosX = _glfwWin.width / 2;
    centerPosY = _glfwWin.height / 2;

    if( centerPosX != _glfwInput.MousePosX || centerPosY != _glfwInput.MousePosY )
    {
        _glfwPlatformSetMouseCursorPos( centerPosX, centerPosY );

        _glfwInput.MousePosX = centerPosX;
        _glfwInput.MousePosY = centerPosY;

        if( _glfwWin.mousePosCallback )
        {
#ifdef MMDAGENT
            _glfwWin.mousePosCallback( _glfwInput.MousePosX,
                                       _glfwInput.MousePosY,
                                       ( _glfwInput.Key[GLFW_KEY_LSHIFT] == GLFW_PRESS || _glfwInput.Key[GLFW_KEY_RSHIFT] == GLFW_PRESS ) ? GLFW_PRESS : GLFW_RELEASE,
                                       ( _glfwInput.Key[GLFW_KEY_LCTRL] == GLFW_PRESS || _glfwInput.Key[GLFW_KEY_RCTRL] == GLFW_PRESS ) ? GLFW_PRESS : GLFW_RELEASE );
#else
            _glfwWin.mousePosCallback( _glfwInput.MousePosX,
                                       _glfwInput.MousePosY );
#endif /* MMDAGENT */
        }
    }
#endif /* !MMDAGENT */

    // From now on the mouse is unlocked
    _glfwWin.mouseLock = GL_FALSE;
}

//========================================================================
// Disable (hide) mouse cursor
//========================================================================

static void disableMouseCursor( void )
{
    if( !_glfwWin.opened || _glfwWin.mouseLock )
    {
        return;
    }

    // Hide mouse cursor
    _glfwPlatformHideMouseCursor();

    // From now on the mouse is locked
    _glfwWin.mouseLock = GL_TRUE;
}


//========================================================================
// Enable sticky keys
//========================================================================

static void enableStickyKeys( void )
{
    _glfwInput.StickyKeys = 1;
}

//========================================================================
// Disable sticky keys
//========================================================================

static void disableStickyKeys( void )
{
    int i;

    _glfwInput.StickyKeys = 0;

    // Release all sticky keys
    for( i = 0; i <= GLFW_KEY_LAST; i++ )
    {
        if( _glfwInput.Key[ i ] == 2 )
        {
            _glfwInput.Key[ i ] = 0;
        }
    }
}


//========================================================================
// Enable sticky mouse buttons
//========================================================================

static void enableStickyMouseButtons( void )
{
    _glfwInput.StickyMouseButtons = 1;
}

//========================================================================
// Disable sticky mouse buttons
//========================================================================

static void disableStickyMouseButtons( void )
{
    int i;

    _glfwInput.StickyMouseButtons = 0;

    // Release all sticky mouse buttons
    for( i = 0; i <= GLFW_MOUSE_BUTTON_LAST; i++ )
    {
        if( _glfwInput.MouseButton[ i ] == 2 )
        {
            _glfwInput.MouseButton[ i ] = 0;
        }
    }
}


//========================================================================
// Enable system keys
//========================================================================

static void enableSystemKeys( void )
{
    if( !_glfwWin.sysKeysDisabled )
    {
        return;
    }

    _glfwPlatformEnableSystemKeys();

    // Indicate that system keys are no longer disabled
    _glfwWin.sysKeysDisabled = GL_FALSE;
}

//========================================================================
// Disable system keys
//========================================================================

static void disableSystemKeys( void )
{
    if( _glfwWin.sysKeysDisabled )
    {
        return;
    }

    _glfwPlatformDisableSystemKeys();

    // Indicate that system keys are now disabled
    _glfwWin.sysKeysDisabled = GL_TRUE;
}


//========================================================================
// Enable key repeat
//========================================================================

static void enableKeyRepeat( void )
{
    _glfwInput.KeyRepeat = 1;
}

//========================================================================
// Disable key repeat
//========================================================================

static void disableKeyRepeat( void )
{
    _glfwInput.KeyRepeat = 0;
}


//========================================================================
// Enable automatic event polling
//========================================================================

static void enableAutoPollEvents( void )
{
    _glfwWin.autoPollEvents = 1;
}

//========================================================================
// Disable automatic event polling
//========================================================================

static void disableAutoPollEvents( void )
{
    _glfwWin.autoPollEvents = 0;
}



//************************************************************************
//****                    GLFW user functions                         ****
//************************************************************************

//========================================================================
// Enable certain GLFW/window/system functions.
//========================================================================

GLFWAPI void GLFWAPIENTRY glfwEnable( int token )
{
    // Is GLFW initialized?
    if( !_glfwInitialized )
    {
        return;
    }

    switch( token )
    {
        case GLFW_MOUSE_CURSOR:
            enableMouseCursor();
            break;
        case GLFW_STICKY_KEYS:
            enableStickyKeys();
            break;
        case GLFW_STICKY_MOUSE_BUTTONS:
            enableStickyMouseButtons();
            break;
        case GLFW_SYSTEM_KEYS:
            enableSystemKeys();
            break;
        case GLFW_KEY_REPEAT:
            enableKeyRepeat();
            break;
        case GLFW_AUTO_POLL_EVENTS:
            enableAutoPollEvents();
            break;
        default:
            break;
    }
}


//========================================================================
// Disable certain GLFW/window/system functions.
//========================================================================

GLFWAPI void GLFWAPIENTRY glfwDisable( int token )
{
    // Is GLFW initialized?
    if( !_glfwInitialized )
    {
        return;
    }

    switch( token )
    {
        case GLFW_MOUSE_CURSOR:
            disableMouseCursor();
            break;
        case GLFW_STICKY_KEYS:
            disableStickyKeys();
            break;
        case GLFW_STICKY_MOUSE_BUTTONS:
            disableStickyMouseButtons();
            break;
        case GLFW_SYSTEM_KEYS:
            disableSystemKeys();
            break;
        case GLFW_KEY_REPEAT:
            disableKeyRepeat();
            break;
        case GLFW_AUTO_POLL_EVENTS:
            disableAutoPollEvents();
            break;
        default:
            break;
    }
}

