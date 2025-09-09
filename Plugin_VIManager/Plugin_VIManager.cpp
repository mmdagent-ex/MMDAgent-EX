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
#include "VIManager.h"
#include "VIManager_Logger.h"
#include "VIManager_Thread.h"

/* definitions */

#ifdef _WIN32
#define EXPORT extern "C" __declspec(dllexport)
#else
#ifdef MMDAGENT_PLUGIN_STATIC
#define EXPORT
#define extAppStart Plugin_VIManager_extAppStart
#define extProcMessage Plugin_VIManager_extProcMessage
#define extRender Plugin_VIManager_extRender
#define extAppEnd Plugin_VIManager_extAppEnd
#else
#define EXPORT extern "C"
#endif
#endif /* _WIN32 */

#define PLUGINVIMANAGER_NAME "VIManager"
#define PLUGINVIMANAGER_LOADCOMMAND "FST_LOAD"

/* variables */

static int mid;
static VIManager_Thread vimanager_thread;
static bool enable;
static bool enable_log;

/* extAppStart: load FST and start thread */
EXPORT void extAppStart(MMDAgent *mmdagent)
{
   char *buf;
   size_t len;

   if ((mid = mmdagent->getModuleId(PLUGINVIMANAGER_NAME)) == -1) {
      enable = false;
      return;
   }

   glewInit();

   /* try to load FST */
   buf = MMDAgent_strdup(mmdagent->getKeyValue()->getString("ContentFile", NULL));
   if (buf == NULL) {
      /* try to load default FST, with same base name with main .mdf file with suffix .fst */
      /* since current directory is config dir, we should get only base name */
      buf = MMDAgent_basename(mmdagent->getConfigFileName());
      if (buf) {
         len = MMDAgent_strlen(buf);
         if (len > 4) {
            buf[len - 4] = '.';
            buf[len - 3] = 'f';
            buf[len - 2] = 's';
            buf[len - 1] = 't';
         } else {
            free(buf);
            buf = NULL;
         }
      }
   }
   if (buf != NULL) {
      if (MMDAgent_exist(buf)) {
         vimanager_thread.loadAndStart(mmdagent, mid, buf, VIMANAGER_INITIAL_STATE_LABEL_DEFAULT);
      } else {
         mmdagent->sendLogString(mid, MLOG_WARNING, "default FST not exist, starting with no FST: \"%s\"", buf);
      }
      free(buf);
   } else {
      mmdagent->sendLogString(mid, MLOG_WARNING, "could not determine FST file, starting with no FST");
   }

   enable = true;
   enable_log = false;
   mmdagent->sendMessage(mid, MMDAGENT_EVENT_PLUGINENABLE, "%s", PLUGINVIMANAGER_NAME);
}

/* extProcMessage: process message */
EXPORT void extProcMessage(MMDAgent *mmdagent, const char *type, const char *args)
{
   if(enable == true) {
      if(MMDAgent_strequal(type, MMDAGENT_COMMAND_PLUGINDISABLE)) {
         if(MMDAgent_strequal(args, PLUGINVIMANAGER_NAME)) {
            mmdagent->sendLogString(mid, MLOG_MESSAGE_CAPTURED, "%s|%s", type, args);
            enable = false;
            mmdagent->sendMessage(mid, MMDAGENT_EVENT_PLUGINDISABLE, "%s", PLUGINVIMANAGER_NAME);
         }
      } else if (MMDAgent_strequal(type, PLUGINVIMANAGER_LOADCOMMAND)) {
         mmdagent->sendLogString(mid, MLOG_MESSAGE_CAPTURED, "%s|%s", type, args);
         vimanager_thread.reload(type, args);
      } else if (MMDAgent_strequal(type, PLUGINVIMANAGER_SUB_COMMAND_START)) {
         if (args) {
            mmdagent->sendLogString(mid, MLOG_MESSAGE_CAPTURED, "%s|%s", type, args);
            char *buff = MMDAgent_strdup(args);
            char *save;
            char *p1 = MMDAgent_strtok(buff, "|", &save);
            char *p2 = MMDAgent_strtok(NULL, "|", &save);
            if (p1 != NULL && p2 != NULL)
               vimanager_thread.addSub(p1, p2);
            free(buff);
         }
      } else if (MMDAgent_strequal(type, PLUGINVIMANAGER_SUB_COMMAND_STARTIF)) {
         if (args) {
            mmdagent->sendLogString(mid, MLOG_MESSAGE_CAPTURED, "%s|%s", type, args);
            char *buff = MMDAgent_strdup(args);
            char *save;
            char *p1 = MMDAgent_strtok(buff, "|", &save);
            char *p2 = MMDAgent_strtok(NULL, "|", &save);
            if (p1 != NULL && p2 != NULL && MMDAgent_exist(p2))
               vimanager_thread.addSub(p1, p2);
            free(buff);
         }
      } else if (MMDAgent_strequal(type, PLUGINVIMANAGER_SUB_COMMAND_STOP)) {
         if (args) {
            mmdagent->sendLogString(mid, MLOG_MESSAGE_CAPTURED, "%s|%s", type, args);
            vimanager_thread.delSub(args);
         }
      } else if (vimanager_thread.isRunning()) {
         if (type != NULL) {
            vimanager_thread.enqueueBuffer(type, args); /* enqueue */
         }
      }
      if(MMDAgent_strequal(type, MMDAGENT_EVENT_KEY) && MMDAgent_strequal(args, "F")) {
         mmdagent->sendLogString(mid, MLOG_MESSAGE_CAPTURED, "%s|%s", type, args);
         if (enable_log == true) {
            /* on -> off */
            enable_log = false;
            mmdagent->setLog2DFlag(false);
         } else {
            /* off -> on */
            enable_log = true;
            mmdagent->setLog2DFlag(true);
         }
      }
   } else {
      if(MMDAgent_strequal(type, MMDAGENT_COMMAND_PLUGINENABLE)) {
         if(MMDAgent_strequal(args, PLUGINVIMANAGER_NAME)) {
            mmdagent->sendLogString(mid, MLOG_MESSAGE_CAPTURED, "%s|%s", type, args);
            enable = true;
            mmdagent->sendMessage(mid, MMDAGENT_EVENT_PLUGINENABLE, "%s", PLUGINVIMANAGER_NAME);
         }
      }
   }
}

/* extRender: render log */
EXPORT void extRender(MMDAgent *mmdagent)
{
}

/* extRender2D: render in 2D screen */
EXPORT void extRender2D(MMDAgent *mmdagent, float screenWidth, float screenHeight)
{
   if (enable == true && enable_log == true)
      vimanager_thread.renderLog(screenWidth, screenHeight);
}

/* extAppEnd: stop and free thread */
EXPORT void extAppEnd(MMDAgent *mmdagent)
{
   vimanager_thread.stopAndRelease();
}
