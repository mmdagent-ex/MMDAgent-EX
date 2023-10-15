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

#include "mecab.h"
#include "njd.h"
#include "jpcommon.h"
#include "HTS_engine.h"

#include "text2mecab.h"
#include "mecab2njd.h"
#include "njd2jpcommon.h"

#include "njd_set_pronunciation.h"
#include "njd_set_digit.h"
#include "njd_set_accent_phrase.h"
#include "njd_set_accent_type.h"
#include "njd_set_unvoiced_vowel.h"
#include "njd_set_long_vowel.h"

#include "Open_JTalk.h"
#include "Open_JTalk_Thread.h"
#include "Open_JTalk_Manager.h"

/* definitions */

#ifdef _WIN32
#define EXPORT extern "C" __declspec(dllexport)
#else
#ifdef MMDAGENT_PLUGIN_STATIC
#define EXPORT
#define extAppStart Plugin_Open_JTalk_extAppStart
#define extProcMessage Plugin_Open_JTalk_extProcMessage
#define extAppEnd Plugin_Open_JTalk_extAppEnd
#else
#define EXPORT extern "C"
#endif
#endif /* _WIN32 */

#define PLUGINOPENJTALK_NAME         "Open_JTalk"
#define PLUGINOPENJTALK_STARTCOMMAND "SYNTH_START"
#define PLUGINOPENJTALK_STOPCOMMAND  "SYNTH_STOP"

/* variables */

static int mid;
static Open_JTalk_Manager open_jtalk_manager;
static bool enable;
static bool error_config;

/* extAppStart: load amodels and start thread */
EXPORT void extAppStart(MMDAgent *mmdagent)
{
   int len;
   char dic[MMDAGENT_MAXBUFLEN];
   char *config;

   mid = mmdagent->getModuleId(PLUGINOPENJTALK_NAME);

   /* get dictionary directory name */
   sprintf(dic, "%s%c%s", mmdagent->getAppDirName(), MMDAGENT_DIRSEPARATOR, PLUGINOPENJTALK_NAME);

   /* get config file */
   /* since current directory is config dir, we should get only base name */
   config = MMDAgent_basename(mmdagent->getConfigFileName());
   len = MMDAgent_strlen(config);

   /* load */
   if (len > 4) {
      config[len - 4] = '.';
      config[len - 3] = 'o';
      config[len - 2] = 'j';
      config[len - 1] = 't';
      if (open_jtalk_manager.loadAndStart(mmdagent, mid, dic, config) == false) {
         if (config)
            free(config);
         error_config = true;
         enable = false;
         return;
      }
      mmdagent->sendLogString(mid, MLOG_STATUS, "config file %s", config);
   }

   if(config)
      free(config);

   error_config = false;
   enable = true;
   mmdagent->sendMessage(mid, MMDAGENT_EVENT_PLUGINENABLE, "%s", PLUGINOPENJTALK_NAME);
}

/* extProcMessage: process message */
EXPORT void extProcMessage(MMDAgent *mmdagent, const char *type, const char *args)
{
   if(enable == true) {
      if(MMDAgent_strequal(type, MMDAGENT_COMMAND_PLUGINDISABLE)) {
         if(MMDAgent_strequal(args, PLUGINOPENJTALK_NAME)) {
            mmdagent->sendLogString(mid, MLOG_MESSAGE_CAPTURED, "%s|%s", type, args);
            enable = false;
            mmdagent->sendMessage(mid, MMDAGENT_EVENT_PLUGINDISABLE, "%s", PLUGINOPENJTALK_NAME);
         }
      } else if (open_jtalk_manager.isRunning()) {
         if (MMDAgent_strequal(type, PLUGINOPENJTALK_STARTCOMMAND)) {
            mmdagent->sendLogString(mid, MLOG_MESSAGE_CAPTURED, "%s|%s", type, args);
            open_jtalk_manager.synthesis(args);
         } else if (MMDAgent_strequal(type, PLUGINOPENJTALK_STOPCOMMAND)) {
            mmdagent->sendLogString(mid, MLOG_MESSAGE_CAPTURED, "%s|%s", type, args);
            open_jtalk_manager.stop(args);
         }
      }
   } else {
      if(MMDAgent_strequal(type, MMDAGENT_COMMAND_PLUGINENABLE)) {
         if(MMDAgent_strequal(args, PLUGINOPENJTALK_NAME)) {
            mmdagent->sendLogString(mid, MLOG_MESSAGE_CAPTURED, "%s|%s", type, args);
            if (error_config == false) {
               enable = true;
               mmdagent->sendMessage(mid, MMDAGENT_EVENT_PLUGINENABLE, "%s", PLUGINOPENJTALK_NAME);
            }
         }
      } else if (MMDAgent_strequal(type, PLUGINOPENJTALK_STARTCOMMAND)) {
         if (error_config == true) {
            mmdagent->sendLogString(mid, MLOG_WARNING, "%s: not configured, \"%s\" ignored", PLUGINOPENJTALK_NAME, type);
         }
      }
   }
}

/* extAppEnd: stop and free thread */
EXPORT void extAppEnd(MMDAgent *mmdagent)
{
   open_jtalk_manager.stopAll();
   open_jtalk_manager.stopAndRelease();
}
