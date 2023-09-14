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

// plugin name
#define PLUGINANYSCRIPT_NAME                "AnyScript"

// note:
//   - Child Process's standard output will be sent to MMDAgent
//   - Child Process's standard error will be sent to console window
//   - Child Process should FLUSH OUTPUT AT EACH LINE to avoid blocking
//       Example 1: use system flush
//           sys.stderr.write("starting\n")
//           sys.stderr.flush()
//       Example 2: add flush option to print
//           print(str, flush=True)
//       Example 3: in Python, add "-u" option to disable buffering
//

// configuration in .mdf:
//
//   Specify an execution command with arguments,
//
//      Plugin_AnyScript_Command=...
//
//   Running several scripts simultaneously (can specify up to 9 commands),
//
//      Plugin_AnyScript_Command1=...
//      Plugin_AnyScript_Command2=...
//
//   Text format (strings after the first '=' will be used as command AS IS):
//      Plugin_AnyScript_Command=C:\Program Files\Python310\python.exe -u test.py
//
//   MMDAgent-EX By default sends message only, if want to receive all log, write in .mdf:
//
//      Plugin_AnyScript_AllLog=true
//
//

// configuration key for executing command string
#define PLUGINANYSCRIPT_CONFIG_COMMAND      "Plugin_AnyScript_Command"
// configuration key for send all log switch (false to send only message (default))
#define PLUGINANYSCRIPT_CONFIG_ALLLOG       "Plugin_AnyScript_AllLog"
// maximum number of child processes
#define PLUGINANYSCRIPT_MAXCOMMANDNUM       10

/* headers */

#include "MMDAgent.h"
#include "ChildProcess.h"

/* variables */


static int mid;
static bool enable = false;
static ChildProcess *cp[PLUGINANYSCRIPT_MAXCOMMANDNUM];
static int cpnum = 0;
static bool send_log = false;  // true: send log, false: send message

/* extAppStart: initialize controller */
EXPORT void extAppStart(MMDAgent *mmdagent)
{
   KeyValue *k = mmdagent->getKeyValue();
   const char *comstr[PLUGINANYSCRIPT_MAXCOMMANDNUM];
   char buff[MMDAGENT_MAXBUFLEN];

   if ((mid = mmdagent->getModuleId(PLUGINANYSCRIPT_NAME)) == -1) {
      enable = false;
      return;
   }

   cpnum = 0;
   for (int i = 0; i < PLUGINANYSCRIPT_MAXCOMMANDNUM; i++) {
      cp[i] = NULL;
   }

   if (k->exist(PLUGINANYSCRIPT_CONFIG_COMMAND)) {
      comstr[cpnum] = k->getString(PLUGINANYSCRIPT_CONFIG_COMMAND, NULL);
      cpnum++;
   }
   for (int i = 1; i < PLUGINANYSCRIPT_MAXCOMMANDNUM; i++) {
      MMDAgent_snprintf(buff, MMDAGENT_MAXBUFLEN, "%s%d", PLUGINANYSCRIPT_CONFIG_COMMAND, i);
      if (k->exist(buff)) {
         comstr[cpnum] = k->getString(buff, NULL);
         cpnum++;
      }
   }

   if (cpnum == 0) {
      enable = false;
      return;
   }

   if (MMDAgent_strequal(k->getString(PLUGINANYSCRIPT_CONFIG_ALLLOG, "false"), "false") == false) {
      send_log = true;
   }

   glfwInit();

   for (int i = 0; i < cpnum; i++) {
      // may not necessary but it may be needed for CreatePipe success...
      if (i > 0)
         MMDAgent_sleep(0.1);
      MMDAgent_snprintf(buff, MMDAGENT_MAXBUFLEN, "Plugin_AnyScript #%d", i);
      cp[i] = new ChildProcess(mmdagent, mid);
      cp[i]->runProcess(buff, comstr[i]);
   }

   enable = true;
   mmdagent->sendMessage(mid, MMDAGENT_EVENT_PLUGINENABLE, "%s", PLUGINANYSCRIPT_NAME);
}

/* extProcMessage: process message */
EXPORT void extProcMessage(MMDAgent *mmdagent, const char *type, const char *args)
{
   char buff[MMDAGENT_MAXBUFLEN];

   if (enable == true) {
      if (MMDAgent_strequal(type, MMDAGENT_COMMAND_PLUGINDISABLE) == true) {
         if (MMDAgent_strequal(args, PLUGINANYSCRIPT_NAME)) {
            enable = false;
            mmdagent->sendMessage(mid, MMDAGENT_EVENT_PLUGINDISABLE, "%s", PLUGINANYSCRIPT_NAME);
         }
      }
   }
   else {
      if (MMDAgent_strequal(type, MMDAGENT_COMMAND_PLUGINENABLE) == true) {
         if (MMDAgent_strequal(args, PLUGINANYSCRIPT_NAME) == true) {
            enable = true;
            mmdagent->sendMessage(mid, MMDAGENT_EVENT_PLUGINENABLE, "%s", PLUGINANYSCRIPT_NAME);
         }
      }
   }
   if (send_log == false) {
      if (enable == true && cpnum > 0) {
         if (MMDAgent_strlen(type) > 0) {
            if (MMDAgent_strlen(args) > 0) {
               MMDAgent_snprintf(buff, MMDAGENT_MAXBUFLEN, "%s|%s\n", type, args);
            } else {
               MMDAgent_snprintf(buff, MMDAGENT_MAXBUFLEN, "%s\n", type);
            }
            for (int i = 0; i < cpnum; i++) {
               if (cp[i]->isRunning())
                  cp[i]->writeToProcess(buff);
            }
         }
      }
   }
}

EXPORT void extLog(MMDAgent *mmdagent, int id, unsigned int flag, const char *text, const char *fulltext)
{
   char buff[MMDAGENT_MAXBUFLEN];

   if (send_log == true) {
      if (enable == true && cpnum > 0) {
         MMDAgent_snprintf(buff, MMDAGENT_MAXBUFLEN, "%s\n", fulltext);
         if (MMDAgent_strlen(buff) > 0) {
            for (int i = 0; i < cpnum; i++) {
               if (cp[i]->isRunning())
                  cp[i]->writeToProcess(buff);
            }
         }
      }
   }
}

/* extUpdate: update */
EXPORT void extUpdate(MMDAgent *mmdagent, double deltaFrame)
{
   if (enable)
      if (cpnum > 0)
         for (int i = 0; i < cpnum; i++)
            cp[i]->update();
}

/* extAppEnd: stop controller */
EXPORT void extAppEnd(MMDAgent *mmdagent)
{
   enable = false;
   if (cpnum > 0) {
      for (int i = 0; i < cpnum; i++) {
         delete cp[i];
         cp[i] = NULL;
      }
   }
   cpnum = 0;
}
