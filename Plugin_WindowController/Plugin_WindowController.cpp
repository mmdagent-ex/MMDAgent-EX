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

#ifdef _WIN32
#define EXPORT extern "C" __declspec(dllexport)
#else
#define EXPORT extern "C"
#endif /* _WIN32 */

#define PLUGINWINDOWCONTROLLER_NAME    "WindowController"
#define PLUGINWINDOWCONTROLLER_EXECUTE "EXECUTE"
#define PLUGINWINDOWCONTROLLER_KEYPOST "KEY_POST"

/* headers */

#include <windows.h>
#include <winuser.h>
#include <ctype.h>
#include "MMDAgent.h"

/* variables */

static int mid;
static bool enable;

typedef struct _KeySet {
   const char *name;
   const unsigned int id;
} KeySet;

const static KeySet keys[] = {
   {"BACKSPACE", VK_BACK},
   {"TAB", VK_TAB},
   {"RETURN", VK_RETURN},
   {"SHIFT", VK_SHIFT},
   {"CONTROL", VK_CONTROL},
   {"ALT", VK_MENU},
   {"CAPSLOCK", VK_CAPITAL},
   {"ESCAPE", VK_ESCAPE},
   {"SPACE", VK_SPACE},
   {"PAGEUP", VK_PRIOR},
   {"PAGEDOWN", VK_NEXT},
   {"END", VK_END},
   {"HOME", VK_HOME},
   {"LEFT", VK_LEFT},
   {"UP", VK_UP},
   {"RIGHT", VK_RIGHT},
   {"DOWN", VK_DOWN},
   {"PRINTSCREEN", VK_SNAPSHOT},
   {"INSERT", VK_INSERT},
   {"DELETE", VK_DELETE},
   {"F1", VK_F1},
   {"F2", VK_F2},
   {"F3", VK_F3},
   {"F4", VK_F4},
   {"F5", VK_F5},
   {"F6", VK_F6},
   {"F7", VK_F7},
   {"F8", VK_F8},
   {"F9", VK_F9},
   {"F10", VK_F10},
   {"F11", VK_F11},
   {"F12", VK_F12},
   {NULL, 0}
};

/* postKeyMessage: post key message */
static void postKeyMessage(const char *args)
{
   int i;
   char *buff, *param1, *param2, *param3, *param4, *param5, *save;
   HWND window;
   unsigned int id = 0;
   INPUT input[8];
   int size;
   bool alt = false, ctrl = false, shift = false;
   LPARAM info;

   buff = MMDAgent_strdup(args);
   param1 = MMDAgent_strtok(buff, "|", &save); /* window name */
   param2 = MMDAgent_strtok(NULL, "|", &save); /* key */
   param3 = MMDAgent_strtok(NULL, "|", &save); /* shift-key */
   param4 = MMDAgent_strtok(NULL, "|", &save); /* control-key */
   param5 = MMDAgent_strtok(NULL, "|", &save); /* alt-key */

   /* check */
   if(buff == NULL || param1 == NULL || param2 == NULL) {
      free(buff);
      return;
   }

   /* get window handle */
   window = FindWindowA(param1, NULL);
   if(window == 0) {
      free(buff);
      return;
   }

   /* get key ID */
   if(MMDAgent_strlen(param2) == 1) {
      id = toupper(param2[0]);
   } else {
      for(i = 0; keys[i].name != NULL; i++) {
         if(MMDAgent_strequal(param2, keys[i].name) == true) {
            id = keys[i].id;
            break;
         }
      }
   }
   if(id == 0) {
      free(buff);
      return;
   }

   /* set forground window */
   SetForegroundWindow(window);

   /* get options */
   shift = MMDAgent_strequal(param3, "ON");
   ctrl = MMDAgent_strequal(param4, "ON");
   alt = MMDAgent_strequal(param5, "ON");

   /* create key message */
   size = 0;
   info = GetMessageExtraInfo();
   if(shift == true && id != VK_SHIFT) {
      input[size].type = INPUT_KEYBOARD;
      input[size].ki.wVk = VK_SHIFT;
      input[size].ki.wScan = MapVirtualKey(VK_SHIFT, 0);
      input[size].ki.dwFlags = 0;
      input[size].ki.time = 0;
      input[size].ki.dwExtraInfo = info;
      size++;
   }
   if(ctrl == true && id != VK_CONTROL) {
      input[size].type = INPUT_KEYBOARD;
      input[size].ki.wVk = VK_CONTROL;
      input[size].ki.wScan = MapVirtualKey(VK_CONTROL, 0);
      input[size].ki.dwFlags = 0;
      input[size].ki.time = 0;
      input[size].ki.dwExtraInfo = info;
      size++;
   }
   if(alt == true && id != VK_MENU) {
      input[size].type = INPUT_KEYBOARD;
      input[size].ki.wVk = VK_MENU;
      input[size].ki.wScan = MapVirtualKey(VK_MENU, 0);
      input[size].ki.dwFlags = 0;
      input[size].ki.time = 0;
      input[size].ki.dwExtraInfo = info;
      size++;
   }
   input[size].type = INPUT_KEYBOARD;
   input[size].ki.wVk = id;
   input[size].ki.wScan = MapVirtualKey(id, 0);
   input[size].ki.dwFlags = 0;
   input[size].ki.time = 0;
   input[size].ki.dwExtraInfo = info;
   size++;
   input[size].type = INPUT_KEYBOARD;
   input[size].ki.wVk = id;
   input[size].ki.wScan = MapVirtualKey(id, 0);
   input[size].ki.dwFlags = KEYEVENTF_KEYUP;
   input[size].ki.time = 0;
   input[size].ki.dwExtraInfo = info;
   size++;
   if(alt == true && id != VK_MENU) {
      input[size].type = INPUT_KEYBOARD;
      input[size].ki.wVk = VK_MENU;
      input[size].ki.wScan = MapVirtualKey(VK_MENU, 0);
      input[size].ki.dwFlags = KEYEVENTF_KEYUP;
      input[size].ki.time = 0;
      input[size].ki.dwExtraInfo = info;
      size++;
   }
   if(ctrl == true && id != VK_CONTROL) {
      input[size].type = INPUT_KEYBOARD;
      input[size].ki.wVk = VK_CONTROL;
      input[size].ki.wScan = MapVirtualKey(VK_CONTROL, 0);
      input[size].ki.dwFlags = KEYEVENTF_KEYUP;
      input[size].ki.time = 0;
      input[size].ki.dwExtraInfo = info;
      size++;
   }
   if(shift == true && id != VK_SHIFT) {
      input[size].type = INPUT_KEYBOARD;
      input[size].ki.wVk = VK_SHIFT;
      input[size].ki.wScan = MapVirtualKey(VK_SHIFT, 0);
      input[size].ki.dwFlags = KEYEVENTF_KEYUP;
      input[size].ki.time = 0;
      input[size].ki.dwExtraInfo = info;
      size++;
   }

   /* send key message */
   SendInput(size, input, sizeof(INPUT));

   free(buff);
}

/* extAppStart: initialize controller */
EXPORT void extAppStart(MMDAgent *mmdagent)
{
   if ((mid = mmdagent->getModuleId(PLUGINWINDOWCONTROLLER_NAME)) == -1) {
      enable = false;
      return;
   }
   enable = true;
   mmdagent->sendMessage(mid, MMDAGENT_EVENT_PLUGINENABLE, "%s", PLUGINWINDOWCONTROLLER_NAME);
}

/* extProcMessage: process message */
EXPORT void extProcMessage(MMDAgent *mmdagent, const char *type, const char *args)
{
   if(enable == true) {
      if(MMDAgent_strequal(type, MMDAGENT_COMMAND_PLUGINDISABLE) == true) {
         if(MMDAgent_strequal(args, PLUGINWINDOWCONTROLLER_NAME)) {
            enable = false;
            mmdagent->sendMessage(mid, MMDAGENT_EVENT_PLUGINDISABLE, "%s", PLUGINWINDOWCONTROLLER_NAME);
         }
      } else if(MMDAgent_strequal(type, PLUGINWINDOWCONTROLLER_EXECUTE) == true) {
         if (MMDAgent_strlen(args) > 0) {
            WCHAR *wp = MMDFiles_pathdup_from_application_to_widechar(args);
            if (wp) {
               STARTUPINFOW si;
               PROCESS_INFORMATION pi;
               ZeroMemory(&si, sizeof(si));
               si.cb = sizeof(si);
               ZeroMemory(&pi, sizeof(pi));
               if (!CreateProcessW(NULL, wp, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {
                  mmdagent->sendLogString(0, MLOG_ERROR, "CreateProcess failed: %s", args);
               }
               free(wp);
            }
      } else if(MMDAgent_strequal(type, PLUGINWINDOWCONTROLLER_KEYPOST) == true) {
         postKeyMessage(args);
      }
   } else {
      if(MMDAgent_strequal(type, MMDAGENT_COMMAND_PLUGINENABLE) == true) {
         if(MMDAgent_strequal(args, PLUGINWINDOWCONTROLLER_NAME) == true) {
            enable = true;
            mmdagent->sendMessage(mid, MMDAGENT_EVENT_PLUGINENABLE, "%s", PLUGINWINDOWCONTROLLER_NAME);
         }
      }
   }
}

/* extAppEnd: stop controller */
EXPORT void extAppEnd(MMDAgent *mmdagent)
{
   enable = false;
}
