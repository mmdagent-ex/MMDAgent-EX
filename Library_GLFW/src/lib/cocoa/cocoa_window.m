//========================================================================
// GLFW - An OpenGL framework
// Platform:    Cocoa/NSOpenGL
// API Version: 2.7
// WWW:         http://www.glfw.org/
//------------------------------------------------------------------------
// Copyright (c) 2009-2010 Camilla Berglund <elmindreda@elmindreda.org>
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

#include <AvailabilityMacros.h>

// Needed for _NSGetProgname
#include <crt_externs.h>

#ifdef MMDAGENT
#ifdef MODIFYCLASSNAME
#define NAMECONNECTER(a,b) a##_##b
#define CLASSNAMECONNECTER(a,b) NAMECONNECTER(a,b)
#define CLASSNAMECONVERTER(name) CLASSNAMECONNECTER(MODIFYCLASSNAME,name)
#else
#define CLASSNAMECONVERTER(name) name
#endif /* MMDAGENT_MODIFYCLASSNAME */
#endif /* MMDAGENT */

//========================================================================
// GLFW application class
//========================================================================

#ifdef MMDAGENT
@interface CLASSNAMECONVERTER(GLFWApplication) : NSApplication
@end
#define GLFWApplication CLASSNAMECONVERTER(GLFWApplication)
#else
@interface GLFWApplication : NSApplication
@end
#endif /* MMDAGENT */

@implementation GLFWApplication

// From http://cocoadev.com/index.pl?GameKeyboardHandlingAlmost
// This works around an AppKit bug, where key up events while holding
// down the command key don't get sent to the key window.
- (void)sendEvent:(NSEvent *)event
{
    if( [event type] == NSKeyUp && ( [event modifierFlags] & NSCommandKeyMask ) )
    {
        [[self keyWindow] sendEvent:event];
    }
    else
    {
        [super sendEvent:event];
    }
}

@end

// Prior to Snow Leopard, we need to use this oddly-named semi-private API
// to get the application menu working properly.  Need to be careful in
// case it goes away in a future OS update.
@interface NSApplication (NSAppleMenu)
- (void)setAppleMenu:(NSMenu *)m;
@end

//========================================================================
// Try to figure out what the calling application is called
//========================================================================

static NSString *findAppName( void )
{
    // Keys to search for as potential application names
    NSString *keys[] =
    {
        @"CFBundleDisplayName",
        @"CFBundleName",
        @"CFBundleExecutable",
    };

    NSDictionary *infoDictionary = [[NSBundle mainBundle] infoDictionary];

    unsigned int i;
    for( i = 0; i < sizeof(keys) / sizeof(keys[0]); i++ )
    {
        id name = [infoDictionary objectForKey:keys[i]];
        if( name &&
            [name isKindOfClass:[NSString class]] &&
            ![@"" isEqualToString:name] )
        {
            return name;
        }
    }

    // Could do this only if we discover we're unbundled, but it should
    // do no harm...
    ProcessSerialNumber psn = { 0, kCurrentProcess };
    TransformProcessType( &psn, kProcessTransformToForegroundApplication );

    // Having the app in front of the terminal window is also generally
    // handy.  There is an NSApplication API to do this, but...
    SetFrontProcess( &psn );

    char **progname = _NSGetProgname();
    if( progname && *progname )
    {
        // TODO: UTF8?
        return [NSString stringWithUTF8String:*progname];
    }

    // Really shouldn't get here
    return @"GLFW Application";
}


//========================================================================
// Set up the menu bar (manually)
// This is nasty, nasty stuff -- calls to undocumented semi-private APIs that
// could go away at any moment, lots of stuff that really should be
// localize(d|able), etc.  Loading a nib would save us this horror, but that
// doesn't seem like a good thing to require of GLFW's clients.
//========================================================================

static void setUpMenuBar( void )
{
    NSString *appName = findAppName();

    NSMenu *bar = [[NSMenu alloc] init];
    [NSApp setMainMenu:bar];

    NSMenuItem *appMenuItem =
        [bar addItemWithTitle:@"" action:NULL keyEquivalent:@""];
    NSMenu *appMenu = [[NSMenu alloc] init];
    [appMenuItem setSubmenu:appMenu];

    [appMenu addItemWithTitle:[NSString stringWithFormat:@"About %@", appName]
                       action:@selector(orderFrontStandardAboutPanel:)
                keyEquivalent:@""];
    [appMenu addItem:[NSMenuItem separatorItem]];
    NSMenu *servicesMenu = [[NSMenu alloc] init];
    [NSApp setServicesMenu:servicesMenu];
    [[appMenu addItemWithTitle:@"Services"
                       action:NULL
                keyEquivalent:@""] setSubmenu:servicesMenu];
    [appMenu addItem:[NSMenuItem separatorItem]];
    [appMenu addItemWithTitle:[NSString stringWithFormat:@"Hide %@", appName]
                       action:@selector(hide:)
                keyEquivalent:@"h"];
    [[appMenu addItemWithTitle:@"Hide Others"
                       action:@selector(hideOtherApplications:)
                keyEquivalent:@"h"]
        setKeyEquivalentModifierMask:NSAlternateKeyMask | NSCommandKeyMask];
    [appMenu addItemWithTitle:@"Show All"
                       action:@selector(unhideAllApplications:)
                keyEquivalent:@""];
    [appMenu addItem:[NSMenuItem separatorItem]];
    [appMenu addItemWithTitle:[NSString stringWithFormat:@"Quit %@", appName]
                       action:@selector(terminate:)
                keyEquivalent:@"q"];

    NSMenuItem *windowMenuItem =
        [bar addItemWithTitle:@"" action:NULL keyEquivalent:@""];
    NSMenu *windowMenu = [[NSMenu alloc] initWithTitle:@"Window"];
    [NSApp setWindowsMenu:windowMenu];
    [windowMenuItem setSubmenu:windowMenu];

    [windowMenu addItemWithTitle:@"Miniaturize"
                          action:@selector(performMiniaturize:)
                   keyEquivalent:@"m"];
    [windowMenu addItemWithTitle:@"Zoom"
                          action:@selector(performZoom:)
                   keyEquivalent:@""];
    [windowMenu addItem:[NSMenuItem separatorItem]];
    [windowMenu addItemWithTitle:@"Bring All to Front"
                          action:@selector(arrangeInFront:)
                   keyEquivalent:@""];

    // At least guard the call to private API to avoid an exception if it
    // goes away.  Hopefully that means the worst we'll break in future is to
    // look ugly...
    if( [NSApp respondsToSelector:@selector(setAppleMenu:)] )
    {
        [NSApp setAppleMenu:appMenu];
    }
}


//========================================================================
// Initialize the Cocoa Application Kit
//========================================================================

static GLboolean initializeAppKit( void )
{
    if( NSApp )
    {
        return GL_TRUE;
    }

    // Implicitly create shared NSApplication instance
    [GLFWApplication sharedApplication];

    // Setting up the menu bar must go between sharedApplication
    // above and finishLaunching below, in order to properly emulate the
    // behavior of NSApplicationMain
    setUpMenuBar();

    return GL_TRUE;
}


//========================================================================
// Delegate for window related notifications
// (but also used as an application delegate)
//========================================================================

#ifdef MMDAGENT
@interface CLASSNAMECONVERTER(GLFWWindowDelegate) : NSObject
@end
#define GLFWWindowDelegate CLASSNAMECONVERTER(GLFWWindowDelegate)
#else
@interface GLFWWindowDelegate : NSObject
@end
#endif /* MMDAGENT */

@implementation GLFWWindowDelegate

#ifdef MMDAGENT
- (BOOL)application:(NSApplication *)theApplication openFile:(NSString *)filename
{
   if (_glfwWin.urlSchemeCallback)
      _glfwWin.urlSchemeCallback([filename UTF8String]);
   return YES;
}
#endif /* MMDAGENT */

- (BOOL)windowShouldClose:(id)window
{
    if( _glfwWin.windowCloseCallback )
    {
        if( !_glfwWin.windowCloseCallback() )
        {
            return NO;
        }
    }

    // This is horribly ugly, but it works
    glfwCloseWindow();
    return NO;
}

- (void)windowDidResize:(NSNotification *)notification
{
    [_glfwWin.context update];
#ifdef MMDAGENT
    [_glfwWin.context2 update];
#endif

    NSRect contentRect =
        [_glfwWin.window contentRectForFrameRect:[_glfwWin.window frame]];
    _glfwWin.width = contentRect.size.width;
    _glfwWin.height = contentRect.size.height;

    if( _glfwWin.windowSizeCallback )
    {
        _glfwWin.windowSizeCallback( _glfwWin.width, _glfwWin.height );
    }
}

- (void)windowDidMove:(NSNotification *)notification
{
    NSPoint point = [_glfwWin.window mouseLocationOutsideOfEventStream];
    _glfwInput.MousePosX = lround(floor(point.x));
    _glfwInput.MousePosY = _glfwWin.height - lround(ceil(point.y));

    if( _glfwWin.mousePosCallback )
    {
#ifdef MMDAGENT
        _glfwWin.mousePosCallback( _glfwInput.MousePosX,
                                  _glfwInput.MousePosY,
                                  ( _glfwInput.Key[GLFW_KEY_LSHIFT] == GLFW_PRESS || _glfwInput.Key[GLFW_KEY_RSHIFT] == GLFW_PRESS ) ? GLFW_PRESS : GLFW_RELEASE,
                                  ( _glfwInput.Key[GLFW_KEY_LCTRL] == GLFW_PRESS || _glfwInput.Key[GLFW_KEY_RCTRL] == GLFW_PRESS ) ? GLFW_PRESS : GLFW_RELEASE );
#else
        _glfwWin.mousePosCallback( _glfwInput.MousePosX, _glfwInput.MousePosY );
#endif /* MMDAGENT */
    }
}

- (void)windowDidMiniaturize:(NSNotification *)notification
{
    _glfwWin.iconified = GL_TRUE;
}

- (void)windowDidDeminiaturize:(NSNotification *)notification
{
    _glfwWin.iconified = GL_FALSE;
}

- (void)windowDidBecomeKey:(NSNotification *)notification
{
    _glfwWin.active = GL_TRUE;
}

- (void)windowDidResignKey:(NSNotification *)notification
{
    _glfwWin.active = GL_FALSE;
    _glfwInputDeactivation();
}

- (NSApplicationTerminateReply)applicationShouldTerminate:(NSApplication *)sender
{
    if( _glfwWin.windowCloseCallback )
    {
        if( !_glfwWin.windowCloseCallback() )
        {
            return NSTerminateCancel;
        }
    }

    // This is horribly ugly, but it works
    glfwCloseWindow();
    return NSTerminateCancel;
}

- (void)applicationWillFinishLaunching:(NSNotification *)aNotification
{
  [[NSAppleEventManager sharedAppleEventManager] setEventHandler:self andSelector:@selector(handleURLEvent:withReplyEvent:) forEventClass:kInternetEventClass andEventID:kAEGetURL];
}

- (void)handleURLEvent:(NSAppleEventDescriptor *)theEvent withReplyEvent:(NSAppleEventDescriptor *)replyEvent
{
  NSString *path = [[[theEvent paramDescriptorForKeyword:keyDirectObject] stringValue] stringByReplacingPercentEscapesUsingEncoding:NSUTF8StringEncoding];

  if (_glfwWin.urlSchemeCallback)
    _glfwWin.urlSchemeCallback([path UTF8String]);
}

@end

//========================================================================
// Converts a Mac OS X keycode to a GLFW keycode
//========================================================================

static int convertMacKeyCode( unsigned int macKeyCode )
{
    // TODO: Need to find mappings for F13-F15, volume down/up/mute, and eject.
    static const unsigned int table[128] =
    {
        /* 00 */ 'A',
        /* 01 */ 'S',
        /* 02 */ 'D',
        /* 03 */ 'F',
        /* 04 */ 'H',
        /* 05 */ 'G',
        /* 06 */ 'Z',
        /* 07 */ 'X',
        /* 08 */ 'C',
        /* 09 */ 'V',
        /* 0a */ -1,
        /* 0b */ 'B',
        /* 0c */ 'Q',
        /* 0d */ 'W',
        /* 0e */ 'E',
        /* 0f */ 'R',
        /* 10 */ 'Y',
        /* 11 */ 'T',
        /* 12 */ '1',
        /* 13 */ '2',
        /* 14 */ '3',
        /* 15 */ '4',
        /* 16 */ '6',
        /* 17 */ '5',
        /* 18 */ '=',
        /* 19 */ '9',
        /* 1a */ '7',
        /* 1b */ '-',
        /* 1c */ '8',
        /* 1d */ '0',
        /* 1e */ ']',
        /* 1f */ 'O',
        /* 20 */ 'U',
        /* 21 */ '[',
        /* 22 */ 'I',
        /* 23 */ 'P',
        /* 24 */ GLFW_KEY_ENTER,
        /* 25 */ 'L',
        /* 26 */ 'J',
        /* 27 */ '\'',
        /* 28 */ 'K',
        /* 29 */ ';',
        /* 2a */ '\\',
        /* 2b */ ',',
        /* 2c */ '/',
        /* 2d */ 'N',
        /* 2e */ 'M',
        /* 2f */ '.',
        /* 30 */ GLFW_KEY_TAB,
        /* 31 */ GLFW_KEY_SPACE,
        /* 32 */ '`',
        /* 33 */ GLFW_KEY_BACKSPACE,
        /* 34 */ -1,
        /* 35 */ GLFW_KEY_ESC,
        /* 36 */ GLFW_KEY_RSUPER,
        /* 37 */ GLFW_KEY_LSUPER,
        /* 38 */ GLFW_KEY_LSHIFT,
        /* 39 */ GLFW_KEY_CAPS_LOCK,
        /* 3a */ GLFW_KEY_LALT,
        /* 3b */ GLFW_KEY_LCTRL,
        /* 3c */ GLFW_KEY_RSHIFT,
        /* 3d */ GLFW_KEY_RALT,
        /* 3e */ GLFW_KEY_RCTRL,
        /* 3f */ -1, /*Function*/
        /* 40 */ GLFW_KEY_F17,
        /* 41 */ GLFW_KEY_KP_DECIMAL,
        /* 42 */ -1,
        /* 43 */ GLFW_KEY_KP_MULTIPLY,
        /* 44 */ -1,
        /* 45 */ GLFW_KEY_KP_ADD,
        /* 46 */ -1,
        /* 47 */ -1, /*KeypadClear*/
        /* 48 */ -1, /*VolumeUp*/
        /* 49 */ -1, /*VolumeDown*/
        /* 4a */ -1, /*Mute*/
        /* 4b */ GLFW_KEY_KP_DIVIDE,
        /* 4c */ GLFW_KEY_KP_ENTER,
        /* 4d */ -1,
        /* 4e */ GLFW_KEY_KP_SUBTRACT,
        /* 4f */ GLFW_KEY_F18,
        /* 50 */ GLFW_KEY_F19,
        /* 51 */ GLFW_KEY_KP_EQUAL,
        /* 52 */ GLFW_KEY_KP_0,
        /* 53 */ GLFW_KEY_KP_1,
        /* 54 */ GLFW_KEY_KP_2,
        /* 55 */ GLFW_KEY_KP_3,
        /* 56 */ GLFW_KEY_KP_4,
        /* 57 */ GLFW_KEY_KP_5,
        /* 58 */ GLFW_KEY_KP_6,
        /* 59 */ GLFW_KEY_KP_7,
        /* 5a */ GLFW_KEY_F20,
        /* 5b */ GLFW_KEY_KP_8,
        /* 5c */ GLFW_KEY_KP_9,
        /* 5d */ -1,
        /* 5e */ -1,
        /* 5f */ -1,
        /* 60 */ GLFW_KEY_F5,
        /* 61 */ GLFW_KEY_F6,
        /* 62 */ GLFW_KEY_F7,
        /* 63 */ GLFW_KEY_F3,
        /* 64 */ GLFW_KEY_F8,
        /* 65 */ GLFW_KEY_F9,
        /* 66 */ -1,
        /* 67 */ GLFW_KEY_F11,
        /* 68 */ -1,
        /* 69 */ GLFW_KEY_F13,
        /* 6a */ GLFW_KEY_F16,
        /* 6b */ GLFW_KEY_F14,
        /* 6c */ -1,
        /* 6d */ GLFW_KEY_F10,
        /* 6e */ -1,
        /* 6f */ GLFW_KEY_F12,
        /* 70 */ -1,
        /* 71 */ GLFW_KEY_F15,
        /* 72 */ GLFW_KEY_INSERT, /*Help*/
        /* 73 */ GLFW_KEY_HOME,
        /* 74 */ GLFW_KEY_PAGEUP,
        /* 75 */ GLFW_KEY_DEL,
        /* 76 */ GLFW_KEY_F4,
        /* 77 */ GLFW_KEY_END,
        /* 78 */ GLFW_KEY_F2,
        /* 79 */ GLFW_KEY_PAGEDOWN,
        /* 7a */ GLFW_KEY_F1,
        /* 7b */ GLFW_KEY_LEFT,
        /* 7c */ GLFW_KEY_RIGHT,
        /* 7d */ GLFW_KEY_DOWN,
        /* 7e */ GLFW_KEY_UP,
        /* 7f */ -1,
    };

    if( macKeyCode >= 128 )
    {
        return -1;
    }

    // This treats keycodes as *positional*; that is, we'll return 'a'
    // for the key left of 's', even on an AZERTY keyboard.  The charInput
    // function should still get 'q' though.
    return table[macKeyCode];
}


//========================================================================
// Content view class for the GLFW window
//========================================================================

#ifdef MMDAGENT
@interface CLASSNAMECONVERTER(GLFWContentView) : NSView
@end
#define GLFWContentView CLASSNAMECONVERTER(GLFWContentView)
#else
@interface GLFWContentView : NSView
@end
#endif /* MMDAGENT */

@implementation GLFWContentView

- (BOOL)isOpaque
{
    return YES;
}

- (BOOL)canBecomeKeyView
{
    return YES;
}

- (BOOL)acceptsFirstResponder
{
    return YES;
}

- (void)mouseDown:(NSEvent *)event
{
    _glfwInputMouseClick( GLFW_MOUSE_BUTTON_LEFT, GLFW_PRESS );
}

- (void)mouseDragged:(NSEvent *)event
{
    [self mouseMoved:event];
}

- (void)mouseUp:(NSEvent *)event
{
    _glfwInputMouseClick( GLFW_MOUSE_BUTTON_LEFT, GLFW_RELEASE );
}

- (void)mouseMoved:(NSEvent *)event
{
    if( _glfwWin.mouseLock )
    {
        _glfwInput.MousePosX += [event deltaX];
        _glfwInput.MousePosY += [event deltaY];
    }
    else
    {
        NSPoint p = [event locationInWindow];

        // Cocoa coordinate system has origin at lower left
        _glfwInput.MousePosX = p.x;
        _glfwInput.MousePosY = _glfwWin.height - p.y;
    }

    if( _glfwWin.mousePosCallback )
    {
#ifdef MMDAGENT
        _glfwWin.mousePosCallback( _glfwInput.MousePosX,
                                  _glfwInput.MousePosY,
                                  ( _glfwInput.Key[GLFW_KEY_LSHIFT] == GLFW_PRESS || _glfwInput.Key[GLFW_KEY_RSHIFT] == GLFW_PRESS ) ? GLFW_PRESS : GLFW_RELEASE,
                                  ( _glfwInput.Key[GLFW_KEY_LCTRL] == GLFW_PRESS || _glfwInput.Key[GLFW_KEY_RCTRL] == GLFW_PRESS ) ? GLFW_PRESS : GLFW_RELEASE );
#else
        _glfwWin.mousePosCallback( _glfwInput.MousePosX, _glfwInput.MousePosY );
#endif /* MMDAGENT */
    }
}

- (void)rightMouseDown:(NSEvent *)event
{
    _glfwInputMouseClick( GLFW_MOUSE_BUTTON_RIGHT, GLFW_PRESS );
}

- (void)rightMouseDragged:(NSEvent *)event
{
    [self mouseMoved:event];
}

- (void)rightMouseUp:(NSEvent *)event
{
    _glfwInputMouseClick( GLFW_MOUSE_BUTTON_RIGHT, GLFW_RELEASE );
}

- (void)otherMouseDown:(NSEvent *)event
{
    _glfwInputMouseClick( [event buttonNumber], GLFW_PRESS );
}

- (void)otherMouseDragged:(NSEvent *)event
{
    [self mouseMoved:event];
}

- (void)otherMouseUp:(NSEvent *)event
{
    _glfwInputMouseClick( [event buttonNumber], GLFW_RELEASE );
}

- (void)keyDown:(NSEvent *)event
{
    NSUInteger length;
    NSString* characters;
    int i, code = convertMacKeyCode( [event keyCode] );

    if( code != -1 )
    {
        _glfwInputKey( code, GLFW_PRESS );

        if( [event modifierFlags] & NSCommandKeyMask )
        {
            if( !_glfwWin.sysKeysDisabled )
            {
                [super keyDown:event];
            }
        }
        else
        {
            characters = [event characters];
            length = [characters length];

            for( i = 0;  i < length;  i++ )
            {
                _glfwInputChar( [characters characterAtIndex:i], GLFW_PRESS );
            }
        }
    }
}

- (void)flagsChanged:(NSEvent *)event
{
    unsigned int newModifierFlags = [event modifierFlags] | NSDeviceIndependentModifierFlagsMask;
    int mode;

    if( newModifierFlags > _glfwWin.modifierFlags )
    {
        mode = GLFW_PRESS;
    }
    else
    {
        mode = GLFW_RELEASE;
    }

    _glfwWin.modifierFlags = newModifierFlags;
    _glfwInputKey( convertMacKeyCode( [event keyCode] ), mode );
}

- (void)keyUp:(NSEvent *)event
{
    NSUInteger length;
    NSString* characters;
    int i, code = convertMacKeyCode( [event keyCode] );

    if( code != -1 )
    {
        _glfwInputKey( code, GLFW_RELEASE );

        characters = [event characters];
        length = [characters length];

        for( i = 0;  i < length;  i++ )
        {
            _glfwInputChar( [characters characterAtIndex:i], GLFW_RELEASE );
        }
    }
}

- (void)scrollWheel:(NSEvent *)event
{
    _glfwInput.WheelPosFloating += [event deltaY];
    _glfwInput.WheelPos = lrint( _glfwInput.WheelPosFloating );

    if( _glfwWin.mouseWheelCallback )
    {
        _glfwWin.mouseWheelCallback( _glfwInput.WheelPos );
    }
}

@end


//************************************************************************
//****               Platform implementation functions                ****
//************************************************************************

//========================================================================
// Here is where the window is created, and the OpenGL rendering context is
// created
//========================================================================

int  _glfwPlatformOpenWindow( int width, int height,
                              const _GLFWwndconfig *wndconfig,
                              const _GLFWfbconfig *fbconfig )
{
    int colorBits;

    _glfwWin.windowNoResize = wndconfig->windowNoResize;

    if( !initializeAppKit() )
    {
        return GL_FALSE;
    }

#if MAC_OS_X_VERSION_MAX_ALLOWED >= 1070
    // Fail if any OpenGL version above 2.1 other than 3.2 was requested
    if( wndconfig->glMajor > 3 ||
        ( wndconfig->glMajor == 3 && wndconfig->glMinor != 2 ) )
    {
        return GL_FALSE;
    }

    if( wndconfig->glProfile )
    {
        // Fail if a profile other than core was explicitly selected
        if( wndconfig->glProfile != GLFW_OPENGL_CORE_PROFILE )
        {
            return GL_FALSE;
        }
    }
#else
    // Fail if OpenGL 3.0 or above was requested
    if( wndconfig->glMajor > 2 )
    {
        return GL_FALSE;
    }
#endif /*MAC_OS_X_VERSION_MAX_ALLOWED*/

    _glfwWin.delegate = [[GLFWWindowDelegate alloc] init];
    if( _glfwWin.delegate == nil )
    {
        return GL_FALSE;
    }

    [NSApp setDelegate:_glfwWin.delegate];

    // Mac OS X needs non-zero color size, so set resonable values
    colorBits = fbconfig->redBits + fbconfig->greenBits + fbconfig->blueBits;
    if( colorBits == 0 )
    {
        colorBits = 24;
    }
    else if( colorBits < 15 )
    {
        colorBits = 15;
    }

    // Ignored hints:
    // OpenGLDebug
    //     pending it meaning anything on Mac OS X

    // Don't use accumulation buffer support; it's not accelerated
    // Aux buffers probably aren't accelerated either


#ifdef MMDAGENT
    // disable fullscreen mode
#else
    CFDictionaryRef fullscreenMode = NULL;
    if( wndconfig->mode == GLFW_FULLSCREEN )
    {
        fullscreenMode =
            // I think it's safe to pass 0 to the refresh rate for this function
            // rather than conditionalizing the code to call the version which
            // doesn't specify refresh...
            CGDisplayBestModeForParametersAndRefreshRateWithProperty(
            CGMainDisplayID(),
            colorBits + fbconfig->alphaBits,
            width,
            height,
            wndconfig->refreshRate,
            // Controversial, see macosx_fullscreen.m for discussion
            kCGDisplayModeIsSafeForHardware,
            NULL);

        width = [[(id)fullscreenMode objectForKey:(id)kCGDisplayWidth] intValue];
        height = [[(id)fullscreenMode objectForKey:(id)kCGDisplayHeight] intValue];
    }
#endif

    unsigned int styleMask = 0;
    if( wndconfig->mode == GLFW_WINDOW )
    {
        styleMask = NSTitledWindowMask | NSClosableWindowMask | NSMiniaturizableWindowMask;

        if( !wndconfig->windowNoResize )
        {
            styleMask |= NSResizableWindowMask;
        }
    }
    else
    {
        styleMask = NSBorderlessWindowMask;
    }

    _glfwWin.window = [[NSWindow alloc]
        initWithContentRect:NSMakeRect( 0, 0, width, height )
                  styleMask:styleMask
                    backing:NSBackingStoreBuffered
                      defer:NO];
#ifdef MMDAGENT
    id view = [[GLFWContentView alloc] init];
    [view setWantsBestResolutionOpenGLSurface:YES];
    [_glfwWin.window setContentView:view];
#else
    [_glfwWin.window setContentView:[[GLFWContentView alloc] init]];
#endif /* MMDAGENT */
    [_glfwWin.window setDelegate:_glfwWin.delegate];
    [_glfwWin.window setAcceptsMouseMovedEvents:YES];
    [(NSWindow *)_glfwWin.window center];

    if( [_glfwWin.window respondsToSelector:@selector(setRestorable)] )
    {
        [_glfwWin.window setRestorable:NO];
    }

#ifdef MMDAGENT
    // disable fullscreen mode
#else
    if( wndconfig->mode == GLFW_FULLSCREEN )
    {
        _glfwLibrary.originalMode = (NSDictionary*)
            CGDisplayCurrentMode( CGMainDisplayID() );

        CGCaptureAllDisplays();
        CGDisplaySwitchToMode( CGMainDisplayID(), fullscreenMode );
    }
#endif

    unsigned int attribute_count = 0;
#define ADD_ATTR(x) attributes[attribute_count++] = x
#define ADD_ATTR2(x, y) (void)({ ADD_ATTR(x); ADD_ATTR(y); })
#define MAX_ATTRS 64 // urrgh
    NSOpenGLPixelFormatAttribute attributes[MAX_ATTRS];

    ADD_ATTR( NSOpenGLPFADoubleBuffer );

#ifdef MMDAGENT
    // disable fullscreen mode
#else
    if( wndconfig->mode == GLFW_FULLSCREEN )
    {
#if MAC_OS_X_VERSION_MAX_ALLOWED < 1070
        ADD_ATTR( NSOpenGLPFAFullScreen );
#endif /*MAC_OS_X_VERSION_MAX_ALLOWED*/

        ADD_ATTR( NSOpenGLPFANoRecovery );
        ADD_ATTR2( NSOpenGLPFAScreenMask,
                   CGDisplayIDToOpenGLDisplayMask( CGMainDisplayID() ) );
    }
#endif

#if MAC_OS_X_VERSION_MAX_ALLOWED >= 1070
    if( wndconfig->glMajor > 2 )
    {
        ADD_ATTR2( NSOpenGLPFAOpenGLProfile, NSOpenGLProfileVersion3_2Core );
    }
#endif /*MAC_OS_X_VERSION_MAX_ALLOWED*/

    ADD_ATTR2( NSOpenGLPFAColorSize, colorBits );

    if( fbconfig->alphaBits > 0)
    {
        ADD_ATTR2( NSOpenGLPFAAlphaSize, fbconfig->alphaBits );
    }

    if( fbconfig->depthBits > 0)
    {
        ADD_ATTR2( NSOpenGLPFADepthSize, fbconfig->depthBits );
    }

    if( fbconfig->stencilBits > 0)
    {
        ADD_ATTR2( NSOpenGLPFAStencilSize, fbconfig->stencilBits );
    }

    int accumBits = fbconfig->accumRedBits + fbconfig->accumGreenBits +
                    fbconfig->accumBlueBits + fbconfig->accumAlphaBits;

    if( accumBits > 0)
    {
        ADD_ATTR2( NSOpenGLPFAAccumSize, accumBits );
    }

    if( fbconfig->auxBuffers > 0)
    {
        ADD_ATTR2( NSOpenGLPFAAuxBuffers, fbconfig->auxBuffers );
    }

    if( fbconfig->stereo)
    {
        ADD_ATTR( NSOpenGLPFAStereo );
    }

    if( fbconfig->samples > 0)
    {
        ADD_ATTR2( NSOpenGLPFASampleBuffers, 1 );
        ADD_ATTR2( NSOpenGLPFASamples, fbconfig->samples );
    }

    ADD_ATTR( 0 );

    _glfwWin.pixelFormat = [[NSOpenGLPixelFormat alloc] initWithAttributes:attributes];
    if( _glfwWin.pixelFormat == nil )
    {
        return GL_FALSE;
    }

    _glfwWin.context = [[NSOpenGLContext alloc] initWithFormat:_glfwWin.pixelFormat
                                                  shareContext:nil];
    if( _glfwWin.context == nil )
    {
        return GL_FALSE;
    }

#ifdef MMDAGENT
    _glfwWin.context2 = [[NSOpenGLContext alloc] initWithFormat:_glfwWin.pixelFormat
                                                  shareContext:_glfwWin.context];
    if( _glfwWin.context2 == nil )
    {
        return GL_FALSE;
    }
#endif

    [_glfwWin.window makeKeyAndOrderFront:nil];
    [_glfwWin.context setView:[_glfwWin.window contentView]];
#ifdef MMDAGENT
    [_glfwWin.context2 setView:[_glfwWin.window contentView]];
#endif

#ifdef MMDAGENT
    // disable fullscreen mode
#else
    if( wndconfig->mode == GLFW_FULLSCREEN )
    {
        // TODO: Make this work on pre-Leopard systems
        [[_glfwWin.window contentView] enterFullScreenMode:[NSScreen mainScreen]
                                               withOptions:nil];
    }
#endif

    [_glfwWin.context makeCurrentContext];

    NSPoint point = [_glfwWin.window mouseLocationOutsideOfEventStream];
    _glfwInput.MousePosX = point.x;
    _glfwInput.MousePosY = _glfwWin.height - point.y;

    [NSApp finishLaunching];

    return GL_TRUE;
}

#ifdef MMDAGENT
void _glfwPlatformMakeCurrentAnotherContext()
{
   [_glfwWin.context makeCurrentContext];
}
#endif

//========================================================================
// Properly kill the window / video display
//========================================================================

void _glfwPlatformCloseWindow( void )
{
    [_glfwWin.window orderOut:nil];

#ifdef MMDAGENT
    // disable fullscreen mode
#else
    if( _glfwWin.fullscreen )
    {
        [[_glfwWin.window contentView] exitFullScreenModeWithOptions:nil];
        CGDisplaySwitchToMode( CGMainDisplayID(),
                               (CFDictionaryRef)_glfwLibrary.originalMode );
        CGReleaseAllDisplays();
    }
#endif

    [_glfwWin.pixelFormat release];
    _glfwWin.pixelFormat = nil;

    [NSOpenGLContext clearCurrentContext];
    [_glfwWin.context release];
    _glfwWin.context = nil;

#ifdef MMDAGENT
    [_glfwWin.context2 release];
    _glfwWin.context2 = nil;
#endif
    [_glfwWin.window setDelegate:nil];
    [NSApp setDelegate:nil];
    [_glfwWin.delegate release];
    _glfwWin.delegate = nil;

    [_glfwWin.window close];
    _glfwWin.window = nil;

    // TODO: Probably more cleanup
}


//========================================================================
// Set the window title
//========================================================================

void _glfwPlatformSetWindowTitle( const char *title )
{
    [_glfwWin.window setTitle:[NSString stringWithCString:title
                     encoding:NSISOLatin1StringEncoding]];
}

#ifdef MMDAGENT
void _glfwPlatformEnableFullScreen()
{
    if (!([_glfwWin.window styleMask] & NSWindowStyleMaskFullScreen)) {
            [_glfwWin.window toggleFullScreen:nil];
    }
}
void _glfwPlatformDisableFullScreen()
{
    if ([_glfwWin.window styleMask] & NSWindowStyleMaskFullScreen) {
        [_glfwWin.window toggleFullScreen:nil];
    }
}
void _glfwPlatformGetRenderingSize( int *width, int *height)
{
    NSRect backingBounds = [[_glfwWin.window contentView] convertRectToBacking:[[_glfwWin.window contentView] bounds]];
    *width  = backingBounds.size.width;
    *height = backingBounds.size.height;
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
    [_glfwWin.window setContentSize:NSMakeSize( width, height )];
}


//========================================================================
// Set the window position
//========================================================================

void _glfwPlatformSetWindowPos( int x, int y )
{
    NSRect contentRect = [_glfwWin.window contentRectForFrameRect:[_glfwWin.window frame]];

    // We assume here that the client code wants to position the window within the
    // screen the window currently occupies
    NSRect screenRect = [[_glfwWin.window screen] visibleFrame];
    contentRect.origin = NSMakePoint(screenRect.origin.x + x,
                                     screenRect.origin.y + screenRect.size.height -
                                         y - contentRect.size.height);

    [_glfwWin.window setFrame:[_glfwWin.window frameRectForContentRect:contentRect]
                      display:YES];
}


//========================================================================
// Iconify the window
//========================================================================

void _glfwPlatformIconifyWindow( void )
{
    [_glfwWin.window miniaturize:nil];
}


//========================================================================
// Restore (un-iconify) the window
//========================================================================

void _glfwPlatformRestoreWindow( void )
{
    [_glfwWin.window deminiaturize:nil];
}


//========================================================================
// Swap buffers
//========================================================================

void _glfwPlatformSwapBuffers( void )
{
    // ARP appears to be unnecessary, but this is future-proof
    [_glfwWin.context flushBuffer];
}


//========================================================================
// Set double buffering swap interval
//========================================================================

void _glfwPlatformSwapInterval( int interval )
{
    GLint sync = interval;
    [_glfwWin.context setValues:&sync forParameter:NSOpenGLCPSwapInterval];
}


//========================================================================
// Write back window parameters into GLFW window structure
//========================================================================

void _glfwPlatformRefreshWindowParams( void )
{
    GLint value;

    // Since GLFW 2.x doesn't understand screens, we use virtual screen zero

    [_glfwWin.pixelFormat getValues:&value
                       forAttribute:NSOpenGLPFAAccelerated
                   forVirtualScreen:0];
    _glfwWin.accelerated = value;

    [_glfwWin.pixelFormat getValues:&value
                       forAttribute:NSOpenGLPFAAlphaSize
                   forVirtualScreen:0];
    _glfwWin.alphaBits = value;

    // It seems that the color size includes the size of the alpha channel
    [_glfwWin.pixelFormat getValues:&value
                       forAttribute:NSOpenGLPFAColorSize
                   forVirtualScreen:0];
    value -= _glfwWin.alphaBits;
    _glfwWin.redBits = value / 3;
    _glfwWin.greenBits = value / 3;
    _glfwWin.blueBits = value / 3;

    [_glfwWin.pixelFormat getValues:&value
                       forAttribute:NSOpenGLPFADepthSize
                   forVirtualScreen:0];
    _glfwWin.depthBits = value;

    [_glfwWin.pixelFormat getValues:&value
                       forAttribute:NSOpenGLPFAStencilSize
                   forVirtualScreen:0];
    _glfwWin.stencilBits = value;

    [_glfwWin.pixelFormat getValues:&value
                       forAttribute:NSOpenGLPFAAccumSize
                   forVirtualScreen:0];
    _glfwWin.accumRedBits = value / 3;
    _glfwWin.accumGreenBits = value / 3;
    _glfwWin.accumBlueBits = value / 3;

    // TODO: Figure out what to set this value to
    _glfwWin.accumAlphaBits = 0;

    [_glfwWin.pixelFormat getValues:&value
                       forAttribute:NSOpenGLPFAAuxBuffers
                   forVirtualScreen:0];
    _glfwWin.auxBuffers = value;

    [_glfwWin.pixelFormat getValues:&value
                       forAttribute:NSOpenGLPFAStereo
                   forVirtualScreen:0];
    _glfwWin.stereo = value;

    [_glfwWin.pixelFormat getValues:&value
                       forAttribute:NSOpenGLPFASamples
                   forVirtualScreen:0];
    _glfwWin.samples = value;

    _glfwWin.glDebug = GL_FALSE;
}


//========================================================================
// Poll for new window and input events
//========================================================================

void _glfwPlatformPollEvents( void )
{
    NSEvent *event;

    do
    {
        event = [NSApp nextEventMatchingMask:NSAnyEventMask
                                   untilDate:[NSDate distantPast]
                                      inMode:NSDefaultRunLoopMode
                                     dequeue:YES];

        if( event )
        {
            [NSApp sendEvent:event];
        }
    }
    while( event );

    [_glfwLibrary.autoreleasePool drain];
    _glfwLibrary.autoreleasePool = [[NSAutoreleasePool alloc] init];
}


//========================================================================
// Wait for new window and input events
//========================================================================

void _glfwPlatformWaitEvents( void )
{
    // I wanted to pass NO to dequeue:, and rely on PollEvents to
    // dequeue and send.  For reasons not at all clear to me, passing
    // NO to dequeue: causes this method never to return.
    NSEvent *event = [NSApp nextEventMatchingMask:NSAnyEventMask
                                        untilDate:[NSDate distantFuture]
                                           inMode:NSDefaultRunLoopMode
                                          dequeue:YES];
    [NSApp sendEvent:event];

    _glfwPlatformPollEvents();
}


//========================================================================
// Hide mouse cursor (lock it)
//========================================================================

void _glfwPlatformHideMouseCursor( void )
{
    [NSCursor hide];
    CGAssociateMouseAndMouseCursorPosition( false );
}


//========================================================================
// Show mouse cursor (unlock it)
//========================================================================

void _glfwPlatformShowMouseCursor( void )
{
    [NSCursor unhide];
    CGAssociateMouseAndMouseCursorPosition( true );
}


//========================================================================
// Set physical mouse cursor position
//========================================================================

void _glfwPlatformSetMouseCursorPos( int x, int y )
{
    if( _glfwWin.fullscreen )
    {
#ifdef MMDAGENT
    // disable fullscreen mode
#else
        NSPoint globalPoint = NSMakePoint( x, y );
        CGDisplayMoveCursorToPoint( CGMainDisplayID(), globalPoint );
#endif
    }
    else
    {
        NSPoint localPoint = NSMakePoint( x, _glfwWin.height - y - 1 );
        NSPoint globalPoint = [_glfwWin.window convertBaseToScreen:localPoint];
        CGPoint mainScreenOrigin = CGDisplayBounds( CGMainDisplayID() ).origin;
        double mainScreenHeight = CGDisplayBounds( CGMainDisplayID() ).size.height;
        CGPoint targetPoint = CGPointMake( globalPoint.x - mainScreenOrigin.x,
                                          mainScreenHeight - globalPoint.y -
                                              mainScreenOrigin.y );
        CGDisplayMoveCursorToPoint( CGMainDisplayID(), targetPoint );
    }
}
