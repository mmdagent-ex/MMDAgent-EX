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
#include "Variables.h"
#include "CountDown_Thread.h"

/* definitions */

#ifdef _WIN32
#define EXPORT extern "C" __declspec(dllexport)
#else
#ifdef MMDAGENT_PLUGIN_STATIC
#define EXPORT
#define extAppStart Plugin_Variables_extAppStart
#define extProcMessage Plugin_Variables_extProcMessage
#define extAppEnd Plugin_Variables_extAppEnd
#else
#define EXPORT extern "C"
#endif
#endif /* _WIN32 */

#define PLUGINVARIABLES_NAME              "Variables"
#define PLUGINVARIABLES_TIMERSTARTCOMMAND "TIMER_START"
#define PLUGINVARIABLES_TIMERSTOPCOMMAND  "TIMER_STOP"
#define PLUGINVARIABLES_TIMERCANCELCOMMAND  "TIMER_CANCEL"
#define PLUGINVARIABLES_VALUESETCOMMAND   "VALUE_SET"
#define PLUGINVARIABLES_VALUEUNSETCOMMAND "VALUE_UNSET"
#define PLUGINVARIABLES_VALUEEVALCOMMAND  "VALUE_EVAL"
#define PLUGINVARIABLES_VALUEGETCOMMAND   "VALUE_GET"

/* variables */

static int mid;
static Variables variables;
static CountDown_Thread countdown_thread;
static bool enable;

/* extAppStart: load models and start thread */
EXPORT void extAppStart(MMDAgent *mmdagent)
{
   mid = mmdagent->getModuleId(PLUGINVARIABLES_NAME);
   variables.setup(mmdagent, mid);
   countdown_thread.setupAndStart(mmdagent, mid);

   enable = true;
   mmdagent->sendMessage(mid, MMDAGENT_EVENT_PLUGINENABLE, "%s", PLUGINVARIABLES_NAME);
}

/* extProcMessage: process message */
EXPORT void extProcMessage(MMDAgent *mmdagent, const char *type, const char *args)
{
   char *buff, *p1, *p2, *p3, *save;

   if(enable == true) {
      if(MMDAgent_strequal(type, MMDAGENT_COMMAND_PLUGINDISABLE)) {
         if(MMDAgent_strequal(args, PLUGINVARIABLES_NAME)) {
            mmdagent->sendLogString(mid, MLOG_MESSAGE_CAPTURED, "%s|%s", type, args);
            enable = false;
            mmdagent->sendMessage(mid, MMDAGENT_EVENT_PLUGINDISABLE, "%s", PLUGINVARIABLES_NAME);
         }
      } else if (MMDAgent_strequal(type, PLUGINVARIABLES_VALUESETCOMMAND)) {
         /* VALUE_SET command */
         mmdagent->sendLogString(mid, MLOG_MESSAGE_CAPTURED, "%s|%s", type, args);
         buff = MMDAgent_strdup(args);
         p1 = MMDAgent_strtok(buff, "|", &save);
         p2 = MMDAgent_strtok(NULL, "|", &save);
         p3 = MMDAgent_strtok(NULL, "|", &save);
         variables.set(p1, p2, p3);
         if(buff)
            free(buff);
      } else if (MMDAgent_strequal(type, PLUGINVARIABLES_VALUEUNSETCOMMAND)) {
         /* VALUE_UNSET command */
         mmdagent->sendLogString(mid, MLOG_MESSAGE_CAPTURED, "%s|%s", type, args);
         variables.unset(args);
      } else if (MMDAgent_strequal(type, PLUGINVARIABLES_VALUEEVALCOMMAND)) {
         /* VALUE_EVAL command */
         mmdagent->sendLogString(mid, MLOG_MESSAGE_CAPTURED, "%s|%s", type, args);
         buff = MMDAgent_strdup(args);
         p1 = MMDAgent_strtok(buff, "|", &save);
         p2 = MMDAgent_strtok(NULL, "|", &save);
         p3 = MMDAgent_strtok(NULL, "|", &save);
         variables.evaluate(p1, p2, p3);
         if(buff)
            free(buff);
      } else if (MMDAgent_strequal(type, PLUGINVARIABLES_VALUEGETCOMMAND)) {
         /* VALUE_GET command */
         mmdagent->sendLogString(mid, MLOG_MESSAGE_CAPTURED, "%s|%s", type, args);
         variables.get(args);
      } else if (MMDAgent_strequal(type, PLUGINVARIABLES_TIMERSTARTCOMMAND)) {
         /* TIMER_START command */
         mmdagent->sendLogString(mid, MLOG_MESSAGE_CAPTURED, "%s|%s", type, args);
         buff = MMDAgent_strdup(args);
         p1 = MMDAgent_strtok(buff, "|", &save);
         p2 = MMDAgent_strtok(NULL, "|", &save);
         countdown_thread.set(p1, p2);
         if(buff)
            free(buff);
      } else if (MMDAgent_strequal(type, PLUGINVARIABLES_TIMERSTOPCOMMAND)) {
         /* TIMER_STOP command */
         mmdagent->sendLogString(mid, MLOG_MESSAGE_CAPTURED, "%s|%s", type, args);
         countdown_thread.unset(args, true);
      } else if (MMDAgent_strequal(type, PLUGINVARIABLES_TIMERCANCELCOMMAND)) {
         /* TIMER_CANCEL command */
         mmdagent->sendLogString(mid, MLOG_MESSAGE_CAPTURED, "%s|%s", type, args);
         countdown_thread.unset(args, false);
      }
   } else {
      if(MMDAgent_strequal(type, MMDAGENT_COMMAND_PLUGINENABLE)) {
         if(MMDAgent_strequal(args, PLUGINVARIABLES_NAME)) {
            mmdagent->sendLogString(mid, MLOG_MESSAGE_CAPTURED, "%s|%s", type, args);
            enable = true;
            mmdagent->sendMessage(mid, MMDAGENT_EVENT_PLUGINENABLE, "%s", PLUGINVARIABLES_NAME);
         }
      }
   }
}

/* extAppEnd: stop and free thread */
EXPORT void extAppEnd(MMDAgent *mmdagent)
{
   countdown_thread.stopAndRelease();
}
