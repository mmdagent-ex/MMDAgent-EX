//========================================================================
// GLFW - An OpenGL framework
// Platform:    Carbon/AGL/CGL
// API Version: 2.7
// WWW:         http://www.glfw.org/
//------------------------------------------------------------------------
// Copyright (c) 2002-2006 Marcus Geelnard
// Copyright (c) 2003      Keith Bauer
// Copyright (c) 2003-2010 Camilla Berglund <elmindreda@elmindreda.org>
// Copyright (c) 2006-2007 Robin Leffmann
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

#ifdef MMDAGENT
#define GLFW_MAXBUFLEN 2048
#endif /* MMDAGENT */

#define _glfwTestModifier( modifierMask, glfwKey ) \
if ( changed & modifierMask ) \
{ \
    _glfwInputKey( glfwKey, (modifiers & modifierMask ? GLFW_PRESS : GLFW_RELEASE) ); \
}

//************************************************************************
//****                  GLFW internal functions                       ****
//************************************************************************

static void handleMacModifierChange( UInt32 modifiers )
{
    UInt32 changed = modifiers ^ _glfwInput.Modifiers;

    // The right *key variants below never actually occur
    // There also isn't even a broken right command key constant
    _glfwTestModifier( shiftKey,        GLFW_KEY_LSHIFT );
    _glfwTestModifier( rightShiftKey,   GLFW_KEY_RSHIFT );
    _glfwTestModifier( controlKey,      GLFW_KEY_LCTRL );
    _glfwTestModifier( rightControlKey, GLFW_KEY_RCTRL );
    _glfwTestModifier( optionKey,       GLFW_KEY_LALT );
    _glfwTestModifier( rightOptionKey,  GLFW_KEY_RALT );
    _glfwTestModifier( cmdKey,          GLFW_KEY_LSUPER );

    _glfwInput.Modifiers = modifiers;
}

static void handleMacKeyChange( UInt32 keyCode, int action )
{
    switch ( keyCode )
    {
        case MAC_KEY_ENTER:       _glfwInputKey( GLFW_KEY_ENTER,       action); break;
        case MAC_KEY_RETURN:      _glfwInputKey( GLFW_KEY_KP_ENTER,    action); break;
        case MAC_KEY_ESC:         _glfwInputKey( GLFW_KEY_ESC,         action); break;
        case MAC_KEY_F1:          _glfwInputKey( GLFW_KEY_F1,          action); break;
        case MAC_KEY_F2:          _glfwInputKey( GLFW_KEY_F2,          action); break;
        case MAC_KEY_F3:          _glfwInputKey( GLFW_KEY_F3,          action); break;
        case MAC_KEY_F4:          _glfwInputKey( GLFW_KEY_F4,          action); break;
        case MAC_KEY_F5:          _glfwInputKey( GLFW_KEY_F5,          action); break;
        case MAC_KEY_F6:          _glfwInputKey( GLFW_KEY_F6,          action); break;
        case MAC_KEY_F7:          _glfwInputKey( GLFW_KEY_F7,          action); break;
        case MAC_KEY_F8:          _glfwInputKey( GLFW_KEY_F8,          action); break;
        case MAC_KEY_F9:          _glfwInputKey( GLFW_KEY_F9,          action); break;
        case MAC_KEY_F10:         _glfwInputKey( GLFW_KEY_F10,         action); break;
        case MAC_KEY_F11:         _glfwInputKey( GLFW_KEY_F11,         action); break;
        case MAC_KEY_F12:         _glfwInputKey( GLFW_KEY_F12,         action); break;
        case MAC_KEY_F13:         _glfwInputKey( GLFW_KEY_F13,         action); break;
        case MAC_KEY_F14:         _glfwInputKey( GLFW_KEY_F14,         action); break;
        case MAC_KEY_F15:         _glfwInputKey( GLFW_KEY_F15,         action); break;
        case MAC_KEY_UP:          _glfwInputKey( GLFW_KEY_UP,          action); break;
        case MAC_KEY_DOWN:        _glfwInputKey( GLFW_KEY_DOWN,        action); break;
        case MAC_KEY_LEFT:        _glfwInputKey( GLFW_KEY_LEFT,        action); break;
        case MAC_KEY_RIGHT:       _glfwInputKey( GLFW_KEY_RIGHT,       action); break;
        case MAC_KEY_TAB:         _glfwInputKey( GLFW_KEY_TAB,         action); break;
        case MAC_KEY_BACKSPACE:   _glfwInputKey( GLFW_KEY_BACKSPACE,   action); break;
        case MAC_KEY_HELP:        _glfwInputKey( GLFW_KEY_INSERT,      action); break;
        case MAC_KEY_DEL:         _glfwInputKey( GLFW_KEY_DEL,         action); break;
        case MAC_KEY_PAGEUP:      _glfwInputKey( GLFW_KEY_PAGEUP,      action); break;
        case MAC_KEY_PAGEDOWN:    _glfwInputKey( GLFW_KEY_PAGEDOWN,    action); break;
        case MAC_KEY_HOME:        _glfwInputKey( GLFW_KEY_HOME,        action); break;
        case MAC_KEY_END:         _glfwInputKey( GLFW_KEY_END,         action); break;
        case MAC_KEY_KP_0:        _glfwInputKey( GLFW_KEY_KP_0,        action); break;
        case MAC_KEY_KP_1:        _glfwInputKey( GLFW_KEY_KP_1,        action); break;
        case MAC_KEY_KP_2:        _glfwInputKey( GLFW_KEY_KP_2,        action); break;
        case MAC_KEY_KP_3:        _glfwInputKey( GLFW_KEY_KP_3,        action); break;
        case MAC_KEY_KP_4:        _glfwInputKey( GLFW_KEY_KP_4,        action); break;
        case MAC_KEY_KP_5:        _glfwInputKey( GLFW_KEY_KP_5,        action); break;
        case MAC_KEY_KP_6:        _glfwInputKey( GLFW_KEY_KP_6,        action); break;
        case MAC_KEY_KP_7:        _glfwInputKey( GLFW_KEY_KP_7,        action); break;
        case MAC_KEY_KP_8:        _glfwInputKey( GLFW_KEY_KP_8,        action); break;
        case MAC_KEY_KP_9:        _glfwInputKey( GLFW_KEY_KP_9,        action); break;
        case MAC_KEY_KP_DIVIDE:   _glfwInputKey( GLFW_KEY_KP_DIVIDE,   action); break;
        case MAC_KEY_KP_MULTIPLY: _glfwInputKey( GLFW_KEY_KP_MULTIPLY, action); break;
        case MAC_KEY_KP_SUBTRACT: _glfwInputKey( GLFW_KEY_KP_SUBTRACT, action); break;
        case MAC_KEY_KP_ADD:      _glfwInputKey( GLFW_KEY_KP_ADD,      action); break;
        case MAC_KEY_KP_DECIMAL:  _glfwInputKey( GLFW_KEY_KP_DECIMAL,  action); break;
        case MAC_KEY_KP_EQUAL:    _glfwInputKey( GLFW_KEY_KP_EQUAL,    action); break;
        case MAC_KEY_KP_ENTER:    _glfwInputKey( GLFW_KEY_KP_ENTER,    action); break;
        case MAC_KEY_NUMLOCK:     _glfwInputKey( GLFW_KEY_KP_NUM_LOCK, action); break;
        default:
        {
            extern void *KCHRPtr;
            UInt32 state = 0;
            char charCode = (char)KeyTranslate( KCHRPtr, keyCode, &state );
            UppercaseText( &charCode, 1, smSystemScript );
            _glfwInputKey( (unsigned char)charCode, action );
        }
        break;
    }
}

// The set of event class/kind combinations supported by keyEventHandler
// This is used by installEventHandlers below
static const EventTypeSpec GLFW_KEY_EVENT_TYPES[] =
{
    { kEventClassKeyboard, kEventRawKeyDown },
    { kEventClassKeyboard, kEventRawKeyUp },
    { kEventClassKeyboard, kEventRawKeyRepeat },
    { kEventClassKeyboard, kEventRawKeyModifiersChanged }
};

static OSStatus keyEventHandler( EventHandlerCallRef handlerCallRef,
                                 EventRef event,
                                 void *userData )
{
    UInt32 keyCode;
    short int keyChar;
    UInt32 modifiers;

    switch( GetEventKind( event ) )
    {
        case kEventRawKeyRepeat:
        case kEventRawKeyDown:
        {
            if( GetEventParameter( event,
                                   kEventParamKeyCode,
                                   typeUInt32,
                                   NULL,
                                   sizeof( UInt32 ),
                                   NULL,
                                   &keyCode ) == noErr )
            {
                handleMacKeyChange( keyCode, GLFW_PRESS );
            }
            if( GetEventParameter( event,
                                   kEventParamKeyUnicodes,
                                   typeUnicodeText,
                                   NULL,
                                   sizeof(keyChar),
                                   NULL,
                                   &keyChar) == noErr )
            {
                _glfwInputChar( keyChar, GLFW_PRESS );
            }
            return noErr;
        }

        case kEventRawKeyUp:
        {
            if( GetEventParameter( event,
                                   kEventParamKeyCode,
                                   typeUInt32,
                                   NULL,
                                   sizeof( UInt32 ),
                                   NULL,
                                   &keyCode ) == noErr )
            {
                handleMacKeyChange( keyCode, GLFW_RELEASE );
            }
            if( GetEventParameter( event,
                                   kEventParamKeyUnicodes,
                                   typeUnicodeText,
                                   NULL,
                                   sizeof(keyChar),
                                   NULL,
                                   &keyChar) == noErr )
            {
                _glfwInputChar( keyChar, GLFW_RELEASE );
            }
            return noErr;
        }

        case kEventRawKeyModifiersChanged:
        {
            if( GetEventParameter( event,
                                   kEventParamKeyModifiers,
                                   typeUInt32,
                                   NULL,
                                   sizeof( UInt32 ),
                                   NULL,
                                   &modifiers ) == noErr )
            {
                handleMacModifierChange( modifiers );
                return noErr;
            }
        }
        break;
    }

    return eventNotHandledErr;
}

// The set of event class/kind combinations supported by mouseEventHandler
// This is used by installEventHandlers below
static const EventTypeSpec GLFW_MOUSE_EVENT_TYPES[] =
{
    { kEventClassMouse, kEventMouseDown },
    { kEventClassMouse, kEventMouseUp },
    { kEventClassMouse, kEventMouseMoved },
    { kEventClassMouse, kEventMouseDragged },
    { kEventClassMouse, kEventMouseWheelMoved },
};

static OSStatus mouseEventHandler( EventHandlerCallRef handlerCallRef,
                                   EventRef event,
                                   void *userData )
{
    switch( GetEventKind( event ) )
    {
        case kEventMouseDown:
        {
            WindowRef window;
            EventRecord oldStyleMacEvent;
            ConvertEventRefToEventRecord( event, &oldStyleMacEvent );
            if( FindWindow ( oldStyleMacEvent.where, &window ) == inMenuBar )
            {
                MenuSelect( oldStyleMacEvent.where );
                HiliteMenu(0);
                return noErr;
            }
            else
            {
                EventMouseButton button;
                if( GetEventParameter( event,
                                       kEventParamMouseButton,
                                       typeMouseButton,
                                       NULL,
                                       sizeof( EventMouseButton ),
                                       NULL,
                                       &button ) == noErr )
                {
                    button -= kEventMouseButtonPrimary;
                    if( button <= GLFW_MOUSE_BUTTON_LAST )
                    {
                        _glfwInputMouseClick( button + GLFW_MOUSE_BUTTON_LEFT,
                                              GLFW_PRESS );
                    }
                    return noErr;
                }
            }
            break;
        }

        case kEventMouseUp:
        {
            EventMouseButton button;
            if( GetEventParameter( event,
                                   kEventParamMouseButton,
                                   typeMouseButton,
                                   NULL,
                                   sizeof( EventMouseButton ),
                                   NULL,
                                   &button ) == noErr )
            {
                button -= kEventMouseButtonPrimary;
                if( button <= GLFW_MOUSE_BUTTON_LAST )
                {
                    _glfwInputMouseClick( button + GLFW_MOUSE_BUTTON_LEFT,
                                          GLFW_RELEASE );
                }
                return noErr;
            }
            break;
        }

        case kEventMouseMoved:
        case kEventMouseDragged:
        {
            HIPoint mouseLocation;
            if( _glfwWin.mouseLock )
            {
                if( GetEventParameter( event,
                                       kEventParamMouseDelta,
                                       typeHIPoint,
                                       NULL,
                                       sizeof( HIPoint ),
                                       NULL,
                                       &mouseLocation ) != noErr )
                {
                    break;
                }

                _glfwInput.MousePosX += mouseLocation.x;
                _glfwInput.MousePosY += mouseLocation.y;
            }
            else
            {
                if( GetEventParameter( event,
                                       kEventParamMouseLocation,
                                       typeHIPoint,
                                       NULL,
                                       sizeof( HIPoint ),
                                       NULL,
                                       &mouseLocation ) != noErr )
                {
                    break;
                }

                _glfwInput.MousePosX = mouseLocation.x;
                _glfwInput.MousePosY = mouseLocation.y;

                if( !_glfwWin.fullscreen )
                {
                    Rect content;
                    GetWindowBounds( _glfwWin.window,
                                     kWindowContentRgn,
                                     &content );

                    _glfwInput.MousePosX -= content.left;
                    _glfwInput.MousePosY -= content.top;
                }
            }

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

            break;
        }

        case kEventMouseWheelMoved:
        {
            EventMouseWheelAxis axis;
            if( GetEventParameter( event,
                                   kEventParamMouseWheelAxis,
                                   typeMouseWheelAxis,
                                   NULL,
                                   sizeof( EventMouseWheelAxis ),
                                   NULL,
                                   &axis) == noErr )
            {
                long wheelDelta;
                if( axis == kEventMouseWheelAxisY &&
                    GetEventParameter( event,
                                       kEventParamMouseWheelDelta,
                                       typeLongInteger,
                                       NULL,
                                       sizeof( long ),
                                       NULL,
                                       &wheelDelta ) == noErr )
                {
                    _glfwInput.WheelPos += wheelDelta;
                    if( _glfwWin.mouseWheelCallback )
                    {
                        _glfwWin.mouseWheelCallback( _glfwInput.WheelPos );
                    }
                    return noErr;
                }
            }
            break;
        }
    }

    return eventNotHandledErr;
}

// The set of event class/kind combinations supported by commandHandler
// This is used by installEventHandlers below
static const EventTypeSpec GLFW_COMMAND_EVENT_TYPES[] =
{
    { kEventClassCommand, kEventCommandProcess }
};

static OSStatus commandHandler( EventHandlerCallRef handlerCallRef,
                                EventRef event,
                                void *userData )
{
    if( _glfwWin.sysKeysDisabled )
    {
        // TODO: Give adequate UI feedback that this is the case
        return eventNotHandledErr;
    }

    HICommand command;
    if( GetEventParameter( event,
                           kEventParamDirectObject,
                           typeHICommand,
                           NULL,
                           sizeof( HICommand ),
                           NULL,
                           &command ) == noErr )
    {
        switch( command.commandID )
        {
            case kHICommandClose:
            case kHICommandQuit:
            {
                // Check if the program wants us to close the window
                if( _glfwWin.windowCloseCallback )
                {
                    if( _glfwWin.windowCloseCallback() )
                    {
                        glfwCloseWindow();
                    }
                }
                else
                {
                    glfwCloseWindow();
                }
                return noErr;
            }
        }
    }

    return eventNotHandledErr;
}

#ifdef MMDAGENT
static OSStatus dragReceiveHandler( WindowPtr window, long *data, DragReference dragReference ){
    OSErr err;
    Point point;
    UInt16 i, count;
    DragItemRef item;
    Size size;
    HFSFlavor hfsFlavor;
    char path[GLFW_MAXBUFLEN];
    FSRef fsRef;

    if( !_glfwWin.dropFileCallback ){
        return noErr;
    }

    err = GetDragMouse( dragReference, &point, NULL );
    if( err )
        return err;
    GlobalToLocal( &point );
    err = CountDragItems( dragReference, &count );
    if( err )
        return err;

    for ( i = 1; i <= count; i++ ){
        err = GetDragItemReferenceNumber( dragReference, i, &item );
        if( err )
            continue;
        err = GetFlavorData( dragReference, item, kDragFlavorTypeHFS, &hfsFlavor, &size, 0 );
        if( err )
            continue;
        err = FSpMakeFSRef( &hfsFlavor.fileSpec, &fsRef );
        if( err )
            continue;
        err = FSRefMakePath( &fsRef, (UInt8 *) path, GLFW_MAXBUFLEN );
        if( err )
            continue;
        _glfwWin.dropFileCallback( path, point.h, point.v );
    }

    return noErr;
}
#endif /* MMDAGENT */

// The set of event class/kind combinations supported by windowEventHandler
// This is used by installEventHandlers below
static const EventTypeSpec GLFW_WINDOW_EVENT_TYPES[] =
{
  { kEventClassWindow, kEventWindowBoundsChanged },
  { kEventClassWindow, kEventWindowClose },
  { kEventClassWindow, kEventWindowDrawContent },
  { kEventClassWindow, kEventWindowActivated },
  { kEventClassWindow, kEventWindowDeactivated },
};

static OSStatus windowEventHandler( EventHandlerCallRef handlerCallRef,
                                    EventRef event,
                                    void *userData )
{
    switch( GetEventKind(event) )
    {
        case kEventWindowBoundsChanged:
        {
            WindowRef window;
            GetEventParameter( event,
                               kEventParamDirectObject,
                               typeWindowRef,
                               NULL,
                               sizeof(WindowRef),
                               NULL,
                               &window );

            Rect rect;
            GetWindowPortBounds( window, &rect );

            if( _glfwWin.width != rect.right ||
                _glfwWin.height != rect.bottom )
            {
                aglUpdateContext( _glfwWin.aglContext );

                _glfwWin.width  = rect.right;
                _glfwWin.height = rect.bottom;
                if( _glfwWin.windowSizeCallback )
                {
                    _glfwWin.windowSizeCallback( _glfwWin.width, _glfwWin.height );
                }
                // Emulate (force) content invalidation
                if( _glfwWin.windowRefreshCallback )
                {
                    _glfwWin.windowRefreshCallback();
                }
            }
            break;
        }

        case kEventWindowClose:
        {
            // Check if the client wants us to close the window
            if( _glfwWin.windowCloseCallback )
            {
                if( _glfwWin.windowCloseCallback() )
                {
                        glfwCloseWindow();
                }
            }
            else
            {
                glfwCloseWindow();
            }
            return noErr;
        }

        case kEventWindowDrawContent:
        {
            if( _glfwWin.windowRefreshCallback )
            {
                _glfwWin.windowRefreshCallback();
            }
            break;
        }

        case kEventWindowActivated:
        {
            _glfwWin.active = GL_TRUE;
            break;
        }

        case kEventWindowDeactivated:
        {
            _glfwWin.active = GL_FALSE;
            _glfwInputDeactivation();
            break;
        }
    }

    return eventNotHandledErr;
}

static int installEventHandlers( void )
{
    OSStatus error;

    _glfwWin.mouseUPP = NewEventHandlerUPP( mouseEventHandler );

    error = InstallEventHandler( GetApplicationEventTarget(),
                                 _glfwWin.mouseUPP,
                                 GetEventTypeCount( GLFW_MOUSE_EVENT_TYPES ),
                                 GLFW_MOUSE_EVENT_TYPES,
                                 NULL,
                                 NULL );
    if( error != noErr )
    {
        fprintf( stderr, "Failed to install Carbon application mouse event handler\n" );
        return GL_FALSE;
    }

    _glfwWin.commandUPP = NewEventHandlerUPP( commandHandler );

    error = InstallEventHandler( GetApplicationEventTarget(),
                                 _glfwWin.commandUPP,
                                 GetEventTypeCount( GLFW_COMMAND_EVENT_TYPES ),
                                 GLFW_COMMAND_EVENT_TYPES,
                                 NULL,
                                 NULL );
    if( error != noErr )
    {
        fprintf( stderr, "Failed to install Carbon application command event handler\n" );
        return GL_FALSE;
    }

    _glfwWin.keyboardUPP = NewEventHandlerUPP( keyEventHandler );

    error = InstallEventHandler( GetApplicationEventTarget(),
                                 _glfwWin.keyboardUPP,
                                 GetEventTypeCount( GLFW_KEY_EVENT_TYPES ),
                                 GLFW_KEY_EVENT_TYPES,
                                 NULL,
                                 NULL );
    if( error != noErr )
    {
        fprintf( stderr, "Failed to install Carbon application key event handler\n" );
        return GL_FALSE;
    }

#ifdef MMDAGENT
    _glfwWin.dragUPP = NewDragReceiveHandlerUPP( (DragReceiveHandlerProcPtr) dragReceiveHandler );
    error = InstallReceiveHandler( _glfwWin.dragUPP, NULL, NULL );
    if( error != noErr )
    {
        fprintf( stderr, "Failed to install Carbon application drag receive handler\n" );
        return GL_FALSE;
    }
#endif /* MMDAGENT */

    return GL_TRUE;
}

//************************************************************************
//****               Platform implementation functions                ****
//************************************************************************

#define _setAGLAttribute( aglAttributeName, AGLparameter ) \
if ( AGLparameter != 0 ) \
{ \
    AGLpixelFormatAttributes[numAGLAttrs++] = aglAttributeName; \
    AGLpixelFormatAttributes[numAGLAttrs++] = AGLparameter; \
}

#define _setCGLAttribute( cglAttributeName, CGLparameter ) \
if ( CGLparameter != 0 ) \
{ \
    CGLpixelFormatAttributes[ numCGLAttrs++ ] = cglAttributeName; \
    CGLpixelFormatAttributes[ numCGLAttrs++ ] = CGLparameter; \
}

//========================================================================
// Here is where the window is created, and
// the OpenGL rendering context is created
//========================================================================

int  _glfwPlatformOpenWindow( int width, int height,
                              const _GLFWwndconfig *wndconfig,
                              const _GLFWfbconfig *fbconfig )
{
    OSStatus error;
    unsigned int windowAttributes;
    ProcessSerialNumber psn;

    // TODO: Break up this function!

#ifdef MMDAGENT
    _glfwWin.dragUPP        = NULL;
#endif /* MMDAGENT */
    _glfwWin.refreshRate    = wndconfig->refreshRate;
    _glfwWin.windowNoResize = wndconfig->windowNoResize;

    // Fail if OpenGL 3.0 or above was requested
    if( wndconfig->glMajor > 2 )
    {
        fprintf( stderr, "OpenGL 3.0+ is not yet supported on Mac OS X\n" );
        return GL_FALSE;
    }

    if( _glfwLibrary.Unbundled )
    {
        if( GetCurrentProcess( &psn ) != noErr )
        {
            fprintf( stderr, "Failed to get the process serial number\n" );
            return GL_FALSE;
        }

        if( TransformProcessType( &psn, kProcessTransformToForegroundApplication ) != noErr )
        {
            fprintf( stderr, "Failed to become a foreground application\n" );
            return GL_FALSE;
        }

        if( wndconfig->mode == GLFW_FULLSCREEN )
        {
            if( SetFrontProcess( &psn ) != noErr )
            {
                fprintf( stderr, "Failed to become the front process\n" );
                return GL_FALSE;
            }
        }
    }

    if( !installEventHandlers() )
    {
        fprintf( stderr, "Failed to install Carbon application event handlers\n" );
        return GL_FALSE;
    }

    // Windowed or fullscreen; AGL or CGL? Quite the mess...
    // AGL appears to be the only choice for attaching OpenGL contexts to
    // Carbon windows, but it leaves the user no control over fullscreen
    // mode stretching. Solution: AGL for windowed, CGL for fullscreen.
    if( wndconfig->mode == GLFW_WINDOW )
    {
        // create AGL pixel format attribute list
        GLint AGLpixelFormatAttributes[256];
        int numAGLAttrs = 0;

        AGLpixelFormatAttributes[numAGLAttrs++] = AGL_RGBA;
        AGLpixelFormatAttributes[numAGLAttrs++] = AGL_DOUBLEBUFFER;
        AGLpixelFormatAttributes[numAGLAttrs++] = AGL_CLOSEST_POLICY;

        if( fbconfig->stereo )
        {
            AGLpixelFormatAttributes[numAGLAttrs++] = AGL_STEREO;
        }

        _setAGLAttribute( AGL_AUX_BUFFERS,      fbconfig->auxBuffers);
        _setAGLAttribute( AGL_RED_SIZE,         fbconfig->redBits );
        _setAGLAttribute( AGL_GREEN_SIZE,       fbconfig->greenBits );
        _setAGLAttribute( AGL_BLUE_SIZE,        fbconfig->blueBits );
        _setAGLAttribute( AGL_ALPHA_SIZE,       fbconfig->alphaBits );
        _setAGLAttribute( AGL_DEPTH_SIZE,       fbconfig->depthBits );
        _setAGLAttribute( AGL_STENCIL_SIZE,     fbconfig->stencilBits );
        _setAGLAttribute( AGL_ACCUM_RED_SIZE,   fbconfig->accumRedBits );
        _setAGLAttribute( AGL_ACCUM_GREEN_SIZE, fbconfig->accumGreenBits );
        _setAGLAttribute( AGL_ACCUM_BLUE_SIZE,  fbconfig->accumBlueBits );
        _setAGLAttribute( AGL_ACCUM_ALPHA_SIZE, fbconfig->accumAlphaBits );

        if( fbconfig->samples > 1 )
        {
            _setAGLAttribute( AGL_SAMPLE_BUFFERS_ARB, 1 );
            _setAGLAttribute( AGL_SAMPLES_ARB, fbconfig->samples );
            AGLpixelFormatAttributes[numAGLAttrs++] = AGL_NO_RECOVERY;
        }

        AGLpixelFormatAttributes[numAGLAttrs++] = AGL_NONE;

        // create pixel format descriptor
        AGLDevice mainMonitor = GetMainDevice();
        _glfwWin.aglPixelFormat = aglChoosePixelFormat( &mainMonitor,
                                                        1,
                                                        AGLpixelFormatAttributes );
        if( _glfwWin.aglPixelFormat == NULL )
        {
            fprintf( stderr,
                     "Failed to choose AGL pixel format: %s\n",
                     aglErrorString( aglGetError() ) );

            return GL_FALSE;
        }

        // create AGL context
        _glfwWin.aglContext = aglCreateContext( _glfwWin.aglPixelFormat, NULL );

        if( _glfwWin.aglContext == NULL )
        {
            fprintf( stderr,
                     "Failed to create AGL context: %s\n",
                     aglErrorString( aglGetError() ) );

            return GL_FALSE;
        }

        // create window
        Rect windowContentBounds;
        windowContentBounds.left = 0;
        windowContentBounds.top = 0;
        windowContentBounds.right = width;
        windowContentBounds.bottom = height;

        windowAttributes = ( kWindowCloseBoxAttribute |
                             kWindowCollapseBoxAttribute |
                             kWindowStandardHandlerAttribute );

        if( wndconfig->windowNoResize )
        {
            windowAttributes |= kWindowLiveResizeAttribute;
        }
        else
        {
            windowAttributes |= ( kWindowFullZoomAttribute |
                                  kWindowResizableAttribute );
        }

        error = CreateNewWindow( kDocumentWindowClass,
                                 windowAttributes,
                                 &windowContentBounds,
                                 &( _glfwWin.window ) );
        if( ( error != noErr ) || ( _glfwWin.window == NULL ) )
        {
            fprintf( stderr, "Failed to create Carbon window\n" );

            return GL_FALSE;
        }

        _glfwWin.windowUPP = NewEventHandlerUPP( windowEventHandler );

        error = InstallWindowEventHandler( _glfwWin.window,
                                           _glfwWin.windowUPP,
                                           GetEventTypeCount( GLFW_WINDOW_EVENT_TYPES ),
                                           GLFW_WINDOW_EVENT_TYPES,
                                           NULL,
                                           NULL );
        if( error != noErr )
        {
            fprintf( stderr, "Failed to install Carbon window event handler\n" );

            return GL_FALSE;
        }

        // Don't care if we fail here
        (void)SetWindowTitleWithCFString( _glfwWin.window, CFSTR( "GLFW Window" ) );
        (void)RepositionWindow( _glfwWin.window,
                                NULL,
                                kWindowCenterOnMainScreen );

        if( !aglSetDrawable( _glfwWin.aglContext,
                             GetWindowPort( _glfwWin.window ) ) )
        {
            fprintf( stderr,
                     "Failed to set the AGL context as the Carbon window drawable: %s\n",
                     aglErrorString( aglGetError() ) );

            return GL_FALSE;
        }

        // Make OpenGL context current
        if( !aglSetCurrentContext( _glfwWin.aglContext ) )
        {
            fprintf( stderr,
                     "Failed to make AGL context current: %s\n",
                     aglErrorString( aglGetError() ) );

            return GL_FALSE;
        }

        ShowWindow( _glfwWin.window );
    }
    else
    {
        CGDisplayErr cgErr;
        CGLError cglErr;

        CFDictionaryRef optimalMode;

        GLint numCGLvs = 0;

        CGLPixelFormatAttribute CGLpixelFormatAttributes[64];
        int numCGLAttrs = 0;

        // variables for enumerating color depths
        GLint rgbColorDepth;

        // CGL pixel format attributes
        _setCGLAttribute( kCGLPFADisplayMask,
                          CGDisplayIDToOpenGLDisplayMask( kCGDirectMainDisplay ) );

        if( fbconfig->stereo )
        {
            CGLpixelFormatAttributes[ numCGLAttrs++ ] = kCGLPFAStereo;
        }

        if( fbconfig->samples > 1 )
        {
            _setCGLAttribute( kCGLPFASamples,       (CGLPixelFormatAttribute)fbconfig->samples );
            _setCGLAttribute( kCGLPFASampleBuffers, (CGLPixelFormatAttribute)1 );
            CGLpixelFormatAttributes[ numCGLAttrs++ ] = kCGLPFANoRecovery;
        }

        CGLpixelFormatAttributes[ numCGLAttrs++ ] = kCGLPFAFullScreen;
        CGLpixelFormatAttributes[ numCGLAttrs++ ] = kCGLPFADoubleBuffer;
        CGLpixelFormatAttributes[ numCGLAttrs++ ] = kCGLPFAAccelerated;
        CGLpixelFormatAttributes[ numCGLAttrs++ ] = kCGLPFANoRecovery;
        CGLpixelFormatAttributes[ numCGLAttrs++ ] = kCGLPFAMinimumPolicy;

        _setCGLAttribute( kCGLPFAAccumSize,
                          (CGLPixelFormatAttribute)( fbconfig->accumRedBits \
                                                   + fbconfig->accumGreenBits \
                                                   + fbconfig->accumBlueBits \
                                                   + fbconfig->accumAlphaBits ) );

        _setCGLAttribute( kCGLPFAAlphaSize,   (CGLPixelFormatAttribute)fbconfig->alphaBits );
        _setCGLAttribute( kCGLPFADepthSize,   (CGLPixelFormatAttribute)fbconfig->depthBits );
        _setCGLAttribute( kCGLPFAStencilSize, (CGLPixelFormatAttribute)fbconfig->stencilBits );
        _setCGLAttribute( kCGLPFAAuxBuffers,  (CGLPixelFormatAttribute)fbconfig->auxBuffers );

        CGLpixelFormatAttributes[ numCGLAttrs++ ] = (CGLPixelFormatAttribute)NULL;

        // create a suitable pixel format with above attributes..
        cglErr = CGLChoosePixelFormat( CGLpixelFormatAttributes,
                                       &_glfwWin.cglPixelFormat,
                                       &numCGLvs );
        if( cglErr != kCGLNoError )
        {
            fprintf( stderr,
                     "Failed to choose CGL pixel format: %s\n",
                     CGLErrorString( cglErr ) );

            return GL_FALSE;
        }

        // ..and create a rendering context using that pixel format
        cglErr = CGLCreateContext( _glfwWin.cglPixelFormat, NULL, &_glfwWin.cglContext );
        if( cglErr != kCGLNoError )
        {
            fprintf( stderr,
                     "Failed to create CGL context: %s\n",
                     CGLErrorString( cglErr ) );

            return GL_FALSE;
        }

        // enumerate depth of RGB channels - unlike AGL, CGL works with
        // a single parameter reflecting the full depth of the frame buffer
        (void)CGLDescribePixelFormat( _glfwWin.cglPixelFormat,
                                      0,
                                      kCGLPFAColorSize,
                                      &rgbColorDepth );

        // capture the display for our application
        cgErr = CGCaptureAllDisplays();
        if( cgErr != kCGErrorSuccess )
        {
            fprintf( stderr,
                     "Failed to capture Core Graphics displays\n");

            return GL_FALSE;
        }

        // find closest matching NON-STRETCHED display mode..
        optimalMode = CGDisplayBestModeForParametersAndRefreshRateWithProperty(
                            kCGDirectMainDisplay,
                            rgbColorDepth,
                            width,
                            height,
                            wndconfig->refreshRate,
                            NULL,
                            NULL );
        if( optimalMode == NULL )
        {
            fprintf( stderr,
                     "Failed to retrieve Core Graphics display mode\n");

            return GL_FALSE;
        }

        // ..and switch to that mode
        cgErr = CGDisplaySwitchToMode( kCGDirectMainDisplay, optimalMode );
        if( cgErr != kCGErrorSuccess )
        {
            fprintf( stderr,
                     "Failed to switch to Core Graphics display mode\n");

            return GL_FALSE;
        }

        // switch to our OpenGL context, and bring it up fullscreen
        cglErr = CGLSetCurrentContext( _glfwWin.cglContext );
        if( cglErr != kCGLNoError )
        {
            fprintf( stderr,
                     "Failed to make CGL context current: %s\n",
                     CGLErrorString( cglErr ) );

            return GL_FALSE;
        }

        cglErr = CGLSetFullScreen( _glfwWin.cglContext );
        if( cglErr != kCGLNoError )
        {
            fprintf( stderr,
                     "Failed to set CGL fullscreen mode: %s\n",
                     CGLErrorString( cglErr ) );

            return GL_FALSE;
        }
    }

    return GL_TRUE;
}

//========================================================================
// Properly kill the window/video display
//========================================================================

void _glfwPlatformCloseWindow( void )
{
    if( _glfwWin.mouseUPP != NULL )
    {
        DisposeEventHandlerUPP( _glfwWin.mouseUPP );
        _glfwWin.mouseUPP = NULL;
    }
    if( _glfwWin.commandUPP != NULL )
    {
        DisposeEventHandlerUPP( _glfwWin.commandUPP );
        _glfwWin.commandUPP = NULL;
    }
    if( _glfwWin.keyboardUPP != NULL )
    {
        DisposeEventHandlerUPP( _glfwWin.keyboardUPP );
        _glfwWin.keyboardUPP = NULL;
    }
#ifdef MMDAGENT
    if( _glfwWin.dragUPP != NULL )
    {
        DisposeDragReceiveHandlerUPP( _glfwWin.dragUPP );
        _glfwWin.dragUPP = NULL;
    }
#endif /* MMDAGENT */
    if( _glfwWin.windowUPP != NULL )
    {
        DisposeEventHandlerUPP( _glfwWin.windowUPP );
        _glfwWin.windowUPP = NULL;
    }

    if( _glfwWin.fullscreen )
    {
        if( _glfwWin.cglContext != NULL )
        {
            CGLSetCurrentContext( NULL );
            CGLClearDrawable( _glfwWin.cglContext );
            CGLDestroyContext( _glfwWin.cglContext );
            CGReleaseAllDisplays();
            _glfwWin.cglContext = NULL;
        }

        if( _glfwWin.cglPixelFormat != NULL )
        {
            CGLDestroyPixelFormat( _glfwWin.cglPixelFormat );
            _glfwWin.cglPixelFormat = NULL;
        }
    }
    else
    {
        if( _glfwWin.aglContext != NULL )
        {
            aglSetCurrentContext( NULL );
            aglSetDrawable( _glfwWin.aglContext, NULL );
            aglDestroyContext( _glfwWin.aglContext );
            _glfwWin.aglContext = NULL;
        }

        if( _glfwWin.aglPixelFormat != NULL )
        {
            aglDestroyPixelFormat( _glfwWin.aglPixelFormat );
            _glfwWin.aglPixelFormat = NULL;
        }
    }

    if( _glfwWin.window != NULL )
    {
        ReleaseWindow( _glfwWin.window );
        _glfwWin.window = NULL;
    }
}

//========================================================================
// Set the window title
//========================================================================

void _glfwPlatformSetWindowTitle( const char *title )
{
    CFStringRef windowTitle;

    if( !_glfwWin.fullscreen )
    {
        windowTitle = CFStringCreateWithCString( kCFAllocatorDefault,
                                                 title,
                                                 kCFStringEncodingISOLatin1 );

        (void)SetWindowTitleWithCFString( _glfwWin.window, windowTitle );

        CFRelease( windowTitle );
    }
}

#ifdef MMDAGENT
void _glfwPlatformEnableFullScreen()
{
}
void _glfwPlatformDisableFullScreen()
{
}
void _glfwPlatformGetWindowPlacementInfo(int *x, int *y, int *width, int *height, int *maximized, int *fullscreen)
{
}
void _glfwPlatformSetWindowPlacementInfo(int x, int y, int width, int height, int maximized, int fullscreen)
{
}
void _glfwPlatformEnableTitleBar()
{
}
void _glfwPlatformDisableTitleBar()
{
}
#endif /* MMDAGENT */

//========================================================================
// Set the window size
//========================================================================

void _glfwPlatformSetWindowSize( int width, int height )
{
    if( !_glfwWin.fullscreen )
    {
        SizeWindow( _glfwWin.window, width, height, TRUE );
    }
}

//========================================================================
// Set the window position
//========================================================================

void _glfwPlatformSetWindowPos( int x, int y )
{
    if( !_glfwWin.fullscreen )
    {
        MoveWindow( _glfwWin.window, x, y, FALSE );
    }
}

//========================================================================
// Window iconification
//========================================================================

void _glfwPlatformIconifyWindow( void )
{
    if( !_glfwWin.fullscreen )
    {
        (void)CollapseWindow( _glfwWin.window, TRUE );
    }
}

//========================================================================
// Window un-iconification
//========================================================================

void _glfwPlatformRestoreWindow( void )
{
    if( !_glfwWin.fullscreen )
    {
        (void)CollapseWindow( _glfwWin.window, FALSE );
    }
}

//========================================================================
// Swap buffers (double-buffering) and poll any new events
//========================================================================

void _glfwPlatformSwapBuffers( void )
{
    if( _glfwWin.fullscreen )
    {
        CGLFlushDrawable( _glfwWin.cglContext );
    }
    else
    {
        aglSwapBuffers( _glfwWin.aglContext );
    }
}

//========================================================================
// Set double buffering swap interval
//========================================================================

void _glfwPlatformSwapInterval( int interval )
{
    GLint AGLparameter = interval;

    // CGL doesn't seem to like intervals other than 0 (vsync off) or 1 (vsync on)
    long CGLparameter = ( interval ? 1 : 0 );

    if( _glfwWin.fullscreen )
    {
        // Don't care if we fail here..
        (void)CGLSetParameter( _glfwWin.cglContext,
                               kCGLCPSwapInterval,
                               (GLint*) &CGLparameter );
    }
    else
    {
        // ..or here
        (void)aglSetInteger( _glfwWin.aglContext,
                             AGL_SWAP_INTERVAL,
                             &AGLparameter );
    }
}

//========================================================================
// Read back framebuffer parameters from the context
//========================================================================

#define _getAGLAttribute( aglAttributeName, variableName ) \
{ \
    GLint aglValue; \
    (void)aglDescribePixelFormat( _glfwWin.aglPixelFormat, aglAttributeName, &aglValue ); \
    variableName = aglValue; \
}

#define _getCGLAttribute( cglAttributeName, variableName ) \
{ \
    GLint cglValue; \
    (void)CGLDescribePixelFormat( _glfwWin.cglPixelFormat, 0, cglAttributeName, &cglValue ); \
    variableName = cglValue; \
}

void _glfwPlatformRefreshWindowParams( void )
{
    GLint rgbColorDepth;
    GLint rgbaAccumDepth = 0;
    GLint rgbChannelDepth = 0;

    if( _glfwWin.fullscreen )
    {
        _getCGLAttribute( kCGLPFAAccelerated, _glfwWin.accelerated );
        _getCGLAttribute( kCGLPFAAlphaSize,   _glfwWin.alphaBits );
        _getCGLAttribute( kCGLPFADepthSize,   _glfwWin.depthBits );
        _getCGLAttribute( kCGLPFAStencilSize, _glfwWin.stencilBits );
        _getCGLAttribute( kCGLPFAAuxBuffers,  _glfwWin.auxBuffers );
        _getCGLAttribute( kCGLPFAStereo,      _glfwWin.stereo );
        _getCGLAttribute( kCGLPFASamples,     _glfwWin.samples );

        // Enumerate depth of RGB channels - unlike AGL, CGL works with
        // a single parameter reflecting the full depth of the frame buffer
        (void)CGLDescribePixelFormat( _glfwWin.cglPixelFormat,
                                      0,
                                      kCGLPFAColorSize,
                                      &rgbColorDepth );

        if( rgbColorDepth == 24 || rgbColorDepth == 32 )
        {
            rgbChannelDepth = 8;
        }
        if( rgbColorDepth == 16 )
        {
            rgbChannelDepth = 5;
        }

        _glfwWin.redBits   = rgbChannelDepth;
        _glfwWin.greenBits = rgbChannelDepth;
        _glfwWin.blueBits  = rgbChannelDepth;

        // Get pixel depth of accumulator - I haven't got the slightest idea
        // how this number conforms to any other channel depth than 8 bits,
        // so this might end up giving completely knackered results...
        _getCGLAttribute( kCGLPFAColorSize, rgbaAccumDepth );
        if( rgbaAccumDepth == 32 )
        {
            rgbaAccumDepth = 8;
        }

        _glfwWin.accumRedBits   = rgbaAccumDepth;
        _glfwWin.accumGreenBits = rgbaAccumDepth;
        _glfwWin.accumBlueBits  = rgbaAccumDepth;
        _glfwWin.accumAlphaBits = rgbaAccumDepth;
    }
    else
    {
        _getAGLAttribute( AGL_ACCELERATED,      _glfwWin.accelerated );
        _getAGLAttribute( AGL_RED_SIZE,         _glfwWin.redBits );
        _getAGLAttribute( AGL_GREEN_SIZE,       _glfwWin.greenBits );
        _getAGLAttribute( AGL_BLUE_SIZE,        _glfwWin.blueBits );
        _getAGLAttribute( AGL_ALPHA_SIZE,       _glfwWin.alphaBits );
        _getAGLAttribute( AGL_DEPTH_SIZE,       _glfwWin.depthBits );
        _getAGLAttribute( AGL_STENCIL_SIZE,     _glfwWin.stencilBits );
        _getAGLAttribute( AGL_ACCUM_RED_SIZE,   _glfwWin.accumRedBits );
        _getAGLAttribute( AGL_ACCUM_GREEN_SIZE, _glfwWin.accumGreenBits );
        _getAGLAttribute( AGL_ACCUM_BLUE_SIZE,  _glfwWin.accumBlueBits );
        _getAGLAttribute( AGL_ACCUM_ALPHA_SIZE, _glfwWin.accumAlphaBits );
        _getAGLAttribute( AGL_AUX_BUFFERS,      _glfwWin.auxBuffers );
        _getAGLAttribute( AGL_STEREO,           _glfwWin.stereo );
        _getAGLAttribute( AGL_SAMPLES_ARB,      _glfwWin.samples );
    }
}

//========================================================================
// Poll for new window and input events
//========================================================================

void _glfwPlatformPollEvents( void )
{
    EventRef event;
    EventTargetRef eventDispatcher = GetEventDispatcherTarget();

    while ( ReceiveNextEvent( 0, NULL, 0.0, TRUE, &event ) == noErr )
    {
        SendEventToEventTarget( event, eventDispatcher );
        ReleaseEvent( event );
    }
}

//========================================================================
// Wait for new window and input events
//========================================================================

void _glfwPlatformWaitEvents( void )
{
    EventRef event;

    // Wait for new events
    ReceiveNextEvent( 0, NULL, kEventDurationForever, FALSE, &event );

    // Process the new events
    _glfwPlatformPollEvents();
}

//========================================================================
// Hide mouse cursor (lock it)
//========================================================================

void _glfwPlatformHideMouseCursor( void )
{
    CGDisplayHideCursor( kCGDirectMainDisplay );
    CGAssociateMouseAndMouseCursorPosition( false );
}

//========================================================================
// Show mouse cursor (unlock it)
//========================================================================

void _glfwPlatformShowMouseCursor( void )
{
    CGDisplayShowCursor( kCGDirectMainDisplay );
    CGAssociateMouseAndMouseCursorPosition( true );
}

//========================================================================
// Set physical mouse cursor position
//========================================================================

void _glfwPlatformSetMouseCursorPos( int x, int y )
{
    Rect content;

    if( _glfwWin.fullscreen )
    {
        CGDisplayMoveCursorToPoint( kCGDirectMainDisplay,
                                    CGPointMake( x, y ) );
    }
    else
    {
        GetWindowBounds(_glfwWin.window, kWindowContentRgn, &content);

        _glfwInput.MousePosX = x + content.left;
        _glfwInput.MousePosY = y + content.top;

        CGDisplayMoveCursorToPoint( kCGDirectMainDisplay,
                                    CGPointMake( _glfwInput.MousePosX,
                                                _glfwInput.MousePosY ) );
    }
}

