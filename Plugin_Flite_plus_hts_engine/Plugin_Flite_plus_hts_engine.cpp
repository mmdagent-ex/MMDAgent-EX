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
#include "HTS_engine.h"
#include "flite_hts_engine.h"
#include "Flite_plus_hts_engine.h"
#include "Flite_plus_hts_engine_Thread.h"
#include "Flite_plus_hts_engine_Manager.h"

/* definitions */

#ifdef _WIN32
#define EXPORT extern "C" __declspec(dllexport)
#else
#ifdef MMDAGENT_PLUGIN_STATIC
#define EXPORT
#define extAppStart Plugin_Flite_plus_hts_engine_extAppStart
#define extProcMessage Plugin_Flite_plus_hts_engine_extProcMessage
#define extAppEnd Plugin_Flite_plus_hts_engine_extAppEnd
#else
#define EXPORT extern "C"
#endif
#endif /* _WIN32 */

#define PLUGINFLITEHTSENGINE_NAME         "Flite_plus_hts_engine"
#define PLUGINFLITEHTSENGINE_STARTCOMMAND "SYNTH_START"
#define PLUGINFLITEHTSENGINE_STOPCOMMAND  "SYNTH_STOP"


/* variables */

static int mid;
static Flite_plus_hts_engine_Manager flite_plus_hts_engine_manager;
static bool enable;
static bool error_config;

/* extAppStart: load amodels and start thread */
EXPORT void extAppStart(MMDAgent *mmdagent)
{
   int len;
   char *config;

   mid = mmdagent->getModuleId(PLUGINFLITEHTSENGINE_NAME);

   /* get config file */
   config = MMDAgent_strdup(mmdagent->getConfigFileName());
   len = MMDAgent_strlen(config);

   /* load */
   if (len > 4) {
      config[len - 4] = '.';
      config[len - 3] = 'f';
      config[len - 2] = 'p';
      config[len - 1] = 'h';
      if (flite_plus_hts_engine_manager.loadAndStart(mmdagent, mid, config) == false) {
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
   mmdagent->sendMessage(mid, MMDAGENT_EVENT_PLUGINENABLE, "%s", PLUGINFLITEHTSENGINE_NAME);
}

/* extProcMessage: process message */
EXPORT void extProcMessage(MMDAgent *mmdagent, const char *type, const char *args)
{
   if(enable == true) {
      if(MMDAgent_strequal(type, MMDAGENT_COMMAND_PLUGINDISABLE)) {
         if(MMDAgent_strequal(args, PLUGINFLITEHTSENGINE_NAME)) {
            mmdagent->sendLogString(mid, MLOG_MESSAGE_CAPTURED, "%s|%s", type, args);
            enable = false;
            mmdagent->sendMessage(mid, MMDAGENT_EVENT_PLUGINDISABLE, "%s", PLUGINFLITEHTSENGINE_NAME);
         }
      } else if (flite_plus_hts_engine_manager.isRunning()) {
         if (MMDAgent_strequal(type, PLUGINFLITEHTSENGINE_STARTCOMMAND)) {
            mmdagent->sendLogString(mid, MLOG_MESSAGE_CAPTURED, "%s|%s", type, args);
            flite_plus_hts_engine_manager.synthesis(args);
         } else if (MMDAgent_strequal(type, PLUGINFLITEHTSENGINE_STOPCOMMAND)) {
            mmdagent->sendLogString(mid, MLOG_MESSAGE_CAPTURED, "%s|%s", type, args);
            flite_plus_hts_engine_manager.stop(args);
         }
      }
   } else {
      if(MMDAgent_strequal(type, MMDAGENT_COMMAND_PLUGINENABLE)) {
         if(MMDAgent_strequal(args, PLUGINFLITEHTSENGINE_NAME)) {
            mmdagent->sendLogString(mid, MLOG_MESSAGE_CAPTURED, "%s|%s", type, args);
            if (error_config == false) {
               enable = true;
               mmdagent->sendMessage(mid, MMDAGENT_EVENT_PLUGINENABLE, "%s", PLUGINFLITEHTSENGINE_NAME);
            }
         }
      } else if (MMDAgent_strequal(type, PLUGINFLITEHTSENGINE_STARTCOMMAND)) {
         if (error_config == true) {
            mmdagent->sendLogString(mid, MLOG_WARNING, "%s: not configured, \"%s\" ignored", PLUGINFLITEHTSENGINE_NAME, type);
         }
      }
   }
}

/* extAppEnd: stop and free thread */
EXPORT void extAppEnd(MMDAgent *mmdagent)
{
   flite_plus_hts_engine_manager.stopAll();
   flite_plus_hts_engine_manager.stopAndRelease();
}
