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
#include "julius/juliuslib.h"
#include "Julius_Logger.h"
#include "Julius_Record.h"
#include "Julius_Thread.h"

/* definitions */

#ifdef _WIN32
#define EXPORT extern "C" __declspec(dllexport)
#else
#ifdef MMDAGENT_PLUGIN_STATIC
#define EXPORT
#define extAppStart Plugin_Julius_extAppStart
#define extProcMessage Plugin_Julius_extProcMessage
#define extAppEnd Plugin_Julius_extAppEnd
#define extUpdate Plugin_Julius_extUpdate
#define extRender2D Plugin_Julius_extRender2D
#else
#define EXPORT extern "C"
#endif
#endif /* _WIN32 */

#define PLUGINJULIUS_NAME          JULIUSTHREAD_PLUGINNAME
#define PLUGINJULIUS_MODIFYCOMMAND "RECOG_MODIFY"
#define PLUGINJULIUS_LOGLOCCOMMAND "RECOG_LOGLOC"

#define PLUGINJULIUS_THRESHOLD_STEP 200

#define PLUGINJULIUS_RECORDSTART   MMDAGENT_COMMAND_RECOGRECORDSTART
#define PLUGINJULIUS_RECORDSTOP    MMDAGENT_COMMAND_RECOGRECORDSTOP

#define PLUGINJULIUS_CONFIG_CAPTION "show_caption"
#define PLUGINJULIUS_CONFIG_NAME     "Plugin_Julius_conf"
#define PLUGINJULIUS_CONFIG_LANGUAGE "Plugin_Julius_lang"
#define PLUGINJULIUS_CONFIG_WORDSPACE "Plugin_Julius_wordspacing"

/* variables */

static int mid;
static Julius_Thread julius_thread;
static bool enable = false;
#ifdef ENABLE_RAPID
static bool rapid_enabled = false;
#endif
static bool caption_enabled = true;
static bool boost_enabled = false;

/* send caption string */
static void sendCaption(const char *args, int id, float x, float y)
{
   char buff2[MMDAGENT_MAXBUFLEN];
   int i, j, len;
   const char *c;
   unsigned char size;

   /* remove comma and kuten (japanese period) */
   len = MMDAgent_strlen(args);
   c = args;
   j = 0;
   for (i = 0; i < len; i += size) {
      size = MMDAgent_getcharsize(c);
      if (size == 0) {/* fail safe */
         break;
      } else if (size == 1 && *c == ',') {
         c += size;
         continue;
      } else if (size == 3 && (unsigned char)*c == 0xe3 && (unsigned char)*(c + 1) == 0x80 && (unsigned char)*(c + 2) == 0x82) {
         c += size;
         continue;
      }
      memcpy(&(buff2[j]), c, size);
      j += size;
      c += size;
   }
   buff2[j] = '\0';

   julius_thread.setLogCaption(buff2, id, x, y);
}

/* setMenu: set menu */
static void setMenu(Menu *menu, int id, int target)
{
   if (target == -1 || target == 0)
      menu->setItem(id, 0, julius_thread.isPausing() == false ? "\xE2\x96\xa3 Listening audio" : "\xE2\x96\xa2 Listening audio", NULL, NULL, NULL);
   if (target == -1 || target == 1)
      menu->setItem(id, 1, caption_enabled ? "\xE2\x96\xa3 Text caption" : "\xE2\x96\xa2 Text caption", NULL, NULL, NULL);
   if (target == -1 || target == 2)
      menu->setItem(id, 2, "Adjust input scale...", NULL, NULL, NULL);
   if (target == -1 || target == 3)
      menu->setItem(id, 3, boost_enabled ? "\xE2\x96\xa3 (debug) WordBoost" : "\xE2\x96\xa2 (debug) WordBoost", NULL, NULL, NULL);
#ifdef ENABLE_RAPID
   if (target == -1 || target == 4)
      menu->setItem(id, 4, rapid_enabled ? "\xE2\x96\xa3 (debug) RapidVoice" : "\xE2\x96\xa2 (debug) RapidVoice", NULL, NULL, NULL);
#endif
}

/* menu handler */
static void menuHandler(int id, int item, void *data)
{
   MMDAgent *mmdagent = (MMDAgent *)data;
   Menu *menu = mmdagent->getMenu();

   if (menu->find("[Speech]") == id) {
      switch (item) {
      case 0: /* mic on/off */
         if (julius_thread.isPausing()) {
            julius_thread.resume();
            mmdagent->sendMessage(mid, "RECOG_EVENT_AWAY", "OFF");
         } else {
            julius_thread.pause();
            mmdagent->sendMessage(mid, "RECOG_EVENT_AWAY", "ON");
         }
         setMenu(menu, id, 0);
         break;
      case 1: /* caption sw */
         caption_enabled = !caption_enabled;
         setMenu(menu, id, 2);
         break;
      case 2: /* Adjust input scalee */
         if (mmdagent->getSlider()) {
            if (mmdagent->getSlider()->isShowing())
               mmdagent->getSlider()->hide();
            else
               mmdagent->getSlider()->show();
         }
         menu->hide();
         break;
      case 3:/* boost sw */
         boost_enabled = !boost_enabled;
         setMenu(menu, id, 3);
         break;
#ifdef ENABLE_RAPID
      case 4: /* rapid sw */
         rapid_enabled = !rapid_enabled;
         setMenu(menu, id, 4);
         break;
#endif
      }
   }
}

/* createMenu: create menu */
static void createMenu(MMDAgent *mmdagent)
{
   int id;
   Menu *menu;
   GLfloat col[4] = { 0.7f, 0.4f, 0.1f, 1.0f };

   if (mmdagent == NULL)
      return;

   menu = mmdagent->getMenu();
   if (menu == NULL)
      return;

   id = menu->add("[Speech]", MENUPRIORITY_SYSTEM, menuHandler, mmdagent);
   menu->setTitleColor(id, col);
   menu->setSkipFlag(id, true);
   setMenu(menu, id, -1);
}

/* startJulius: start Julius */
static bool startJulius(MMDAgent *mmdagent, const char *conffile)
{
   int len;
   char configFile[MMDAGENT_MAXBUFLEN];
   char buff[MMDAGENT_MAXBUFLEN];
   char *userConfigFile = NULL;
   char *userDictionary = NULL;
#ifdef ENABLE_RAPID
   char userRapidWordDictionary[MMDAGENT_MAXBUFLEN];
#endif

   /* config file */
   MMDAgent_snprintf(configFile, MMDAGENT_MAXBUFLEN, "%s%c%s%c%s", mmdagent->getAppDirName(), MMDAGENT_DIRSEPARATOR, PLUGINJULIUS_NAME, MMDAGENT_DIRSEPARATOR, conffile);
   if (MMDAgent_exist(configFile) == false)
      return false;

   /* user config file */
   MMDAgent_snprintf(buff, MMDAGENT_MAXBUFLEN, "%s", mmdagent->getConfigFileName());
   len = MMDAgent_strlen(buff);
   if (len > 4 && len < MMDAGENT_MAXBUFLEN - 2) {
      buff[len - 4] = '.';
      buff[len - 3] = 'j';
      buff[len - 2] = 'c';
      buff[len - 1] = 'o';
      buff[len] = 'n';
      buff[len + 1] = 'f';
      buff[len + 2] = '\0';
      if (MMDAgent_exist(buff)) {
         userConfigFile = MMDAgent_strdup(buff);
         mmdagent->sendLogString(mid, MLOG_STATUS, "loading additional user jconf \"%s\"", userConfigFile);
      }
   }

   /* user dictionary */
   MMDAgent_snprintf(buff, MMDAGENT_MAXBUFLEN, "%s", mmdagent->getConfigFileName());
   len = MMDAgent_strlen(buff);
   if (len > 4) {
      buff[len - 4] = '.';
      buff[len - 3] = 'd';
      buff[len - 2] = 'i';
      buff[len - 1] = 'c';
      if (MMDAgent_exist(buff)) {
         userDictionary = MMDAgent_strdup(buff);
         mmdagent->sendLogString(mid, MLOG_STATUS, "loading additional user dictionary \"%s\"", userDictionary);
      }
   }

#ifdef ENABLE_RAPID
   /* user rapid word dictionary */
   strcpy(userRapidWordDictionary, mmdagent->getConfigFileName());
   len = MMDAgent_strlen(userRapidWordDictionary);
   if (len > 4 && len < MMDAGENT_MAXBUFLEN - 5) {
      userRapidWordDictionary[len - 4] = '.';
      userRapidWordDictionary[len - 3] = 'r';
      userRapidWordDictionary[len - 2] = 'a';
      userRapidWordDictionary[len - 1] = 'p';
      userRapidWordDictionary[len    ] = 'i';
      userRapidWordDictionary[len + 1] = 'd';
      userRapidWordDictionary[len + 2] = 'd';
      userRapidWordDictionary[len + 3] = 'i';
      userRapidWordDictionary[len + 4] = 'c';
      userRapidWordDictionary[len + 5] = '\0';
   } else {
      strcpy(userRapidWordDictionary, "");
   }
#endif /* ENABLE_RAPID */

   /* load models and start thread */
#ifdef ENABLE_RAPID
   julius_thread.loadAndStart(mmdagent, mid, configFile, userConfigFile, userDictionary, userRapidWordDictionary);
#else
   julius_thread.loadAndStart(mmdagent, mid, configFile, userConfigFile, userDictionary);
#endif /* ENABLE_RAPID */

   if (userConfigFile)
      free(userConfigFile);
   if (userDictionary)
      free(userDictionary);

   return true;
}

/* extAppStart: load models and start thread */
EXPORT void extAppStart(MMDAgent *mmdagent)
{
   char buff[MMDAGENT_MAXBUFLEN];
   mid = mmdagent->getModuleId(PLUGINJULIUS_NAME);

   /* caption setting */
   if (MMDAgent_strequal(mmdagent->getKeyValue()->getString(PLUGINJULIUS_CONFIG_CAPTION, "true"), "true")) {
      caption_enabled = true;
   } else {
      caption_enabled = false;
   }

   /* determine configuration */
   if (!mmdagent->getKeyValue()->exist(PLUGINJULIUS_CONFIG_NAME)) {
      mmdagent->sendLogString(mid, MLOG_WARNING, "no config name specified, Plugin_Julius disabled. Set \"%s\" in .mdf to enable.", PLUGINJULIUS_CONFIG_NAME);
      return;
   }
   const char *name = mmdagent->getKeyValue()->getString(PLUGINJULIUS_CONFIG_NAME, NULL);
   if (MMDAgent_strlen(name) == 0) {
      mmdagent->sendLogString(mid, MLOG_WARNING, "no config name specified, Plugin_Julius disabled. Set \"%s\" in .mdf to enable.", PLUGINJULIUS_CONFIG_NAME);
      return;
   }
   if (!mmdagent->getKeyValue()->exist(PLUGINJULIUS_CONFIG_LANGUAGE)) {
      mmdagent->sendLogString(mid, MLOG_WARNING, "no language specified, start with Japanese model. For English, write \"%s=en\" in .mdf", PLUGINJULIUS_CONFIG_LANGUAGE);
   }
   char *lang = MMDAgent_strdup(mmdagent->getKeyValue()->getString(PLUGINJULIUS_CONFIG_LANGUAGE, "ja"));
   MMDAgent_snprintf(buff, MMDAGENT_MAXBUFLEN, "jconf_%s_%s.txt", name, lang);

   /* start Julius */
   if (startJulius(mmdagent, buff) == false) {
      mmdagent->sendLogString(mid, MLOG_ERROR, "conf=%s,lang=%s: jconf file \"%s\" not exist, Plugin_Julius disabled", name, lang, buff);
      return;
   }


   /* set word spacing */
   julius_thread.setWordSpacing("");
   if (mmdagent->getKeyValue()->exist(PLUGINJULIUS_CONFIG_WORDSPACE)) {
      /* specified */
      const char *val = mmdagent->getKeyValue()->getString(PLUGINJULIUS_CONFIG_WORDSPACE, NULL);
      if (MMDAgent_strlen(val) > 0) {
         if (MMDAgent_strequal(val, "true") || MMDAgent_strequal(val, "True")
            || MMDAgent_strequal(val, "on") || MMDAgent_strequal(val, "On")
            || MMDAgent_strequal(val, "yes") || MMDAgent_strequal(val, "Yes")) {
            julius_thread.setWordSpacing(" ");
         } else if (MMDAgent_strequal(val, "comma") || MMDAgent_strequal(val, "Comma")) {
            julius_thread.setWordSpacing(",");
         }
      }
   } else {
      /* default depends on language */
      if (!MMDAgent_strequal(lang, "ja")) {
         julius_thread.setWordSpacing(" ");
      }
   }

   mmdagent->sendLogString(mid, MLOG_STATUS, "starting: conf=%s,lang=%s: jconf file \"%s\"", name, lang, buff);
   mmdagent->sendLogString(mid, MLOG_STATUS, "you can save Julius log by specifying \"%s=filename\" in .mdf", PLUGINJULIUS_LOG_FILE);

   glewInit();

   /* create menu */
   createMenu(mmdagent);

   enable = true;
}

/* extProcMessage: process message */
EXPORT void extProcMessage(MMDAgent *mmdagent, const char *type, const char *args)
{
   char *buff;
   char buff2[MMDAGENT_MAXBUFLEN];
   char *p1, *p2, *save;

   if (MMDAgent_strequal(type, MMDAGENT_COMMAND_PLUGINDISABLE)) {
      if (MMDAgent_strequal(args, PLUGINJULIUS_NAME) && enable == true) {
         mmdagent->sendLogString(mid, MLOG_MESSAGE_CAPTURED, "%s|%s", type, args);
         julius_thread.pause();
         enable = false;
         mmdagent->sendMessage(mid, MMDAGENT_EVENT_PLUGINDISABLE, "%s", PLUGINJULIUS_NAME);
      }
   } else if (MMDAgent_strequal(type, MMDAGENT_COMMAND_PLUGINENABLE)) {
      if (MMDAgent_strequal(args, PLUGINJULIUS_NAME) && enable == false) {
         mmdagent->sendLogString(mid, MLOG_MESSAGE_CAPTURED, "%s|%s", type, args);
         julius_thread.resume();
         enable = true;
         mmdagent->sendMessage(mid, MMDAGENT_EVENT_PLUGINENABLE, "%s", PLUGINJULIUS_NAME);
      }
   } else if (MMDAgent_strequal(type, MMDAGENT_EVENT_KEY)) {
      if (MMDAgent_strequal(args, "<")) {
         mmdagent->sendLogString(mid, MLOG_MESSAGE_CAPTURED, "%s|%s", type, args);
         julius_thread.moveThreshold(-PLUGINJULIUS_THRESHOLD_STEP);
      } else if (MMDAgent_strequal(args, ">")) {
         mmdagent->sendLogString(mid, MLOG_MESSAGE_CAPTURED, "%s|%s", type, args);
         julius_thread.moveThreshold(PLUGINJULIUS_THRESHOLD_STEP);
      }
   } else if (enable == true && MMDAgent_strequal(type, PLUGINJULIUS_MODIFYCOMMAND)) {
      mmdagent->sendLogString(mid, MLOG_MESSAGE_CAPTURED, "%s|%s", type, args);
      buff = MMDAgent_strdup(args);
      p1 = MMDAgent_strtok(buff, "|", &save);
      p2 = MMDAgent_strtok(NULL, "|", &save);
      if (MMDAgent_strequal(p1, JULIUSTHREAD_CHANGECONF)) {
         MMDAgent_snprintf(buff2, MMDAGENT_MAXBUFLEN, "%s.txt", p2);
         julius_thread.stopAndRelease();
         mmdagent->sendMessage(mid, MMDAGENT_EVENT_PLUGINDISABLE, "%s", PLUGINJULIUS_NAME);
         startJulius(mmdagent, buff2);
         mmdagent->sendMessage(mid, JULIUSTHREAD_EVENTMODIFY, "%s|%s", JULIUSTHREAD_CHANGECONF, p2);
      } else if (MMDAgent_strequal(p1, "PREDICTWORD") && boost_enabled == false) {
         /* ignore predicted words when word boosting is disabled */
      } else {
         julius_thread.storeCommand(args);
      }
      free(buff);
   } else if (enable == true && MMDAgent_strequal(type, PLUGINJULIUS_RECORDSTART)) {
      mmdagent->sendLogString(mid, MLOG_MESSAGE_CAPTURED, "%s|%s", type, args);
      julius_thread.enableRecording(args);
   } else if (enable == true && MMDAgent_strequal(type, PLUGINJULIUS_RECORDSTOP)) {
      mmdagent->sendLogString(mid, MLOG_MESSAGE_CAPTURED, "%s", type);
      julius_thread.disableRecording();
#ifdef ENABLE_RAPID
   } else if (enable == true && rapid_enabled && MMDAgent_strequal(type, JULIUSTHREAD_EVENTRAPID)) {
      mmdagent->sendLogString(mid, MLOG_MESSAGE_CAPTURED, "%s", type);
      if (mmdagent->getMenu())
         if (mmdagent->getMenu()->isShowing() == false && MMDAgent_strequal(args, "command:menu"))
            mmdagent->getMenu()->show();
      if (MMDAgent_strequal(args, "command:log"))
         mmdagent->procDisplayLogMessage();
   } else if (enable == true && rapid_enabled && MMDAgent_strequal(type, JULIUSTHREAD_EVENTSTOP)) {
      mmdagent->sendLogString(mid, MLOG_MESSAGE_CAPTURED, "%s", type);
      if (mmdagent->getMenu())
         if (MMDAgent_strequal(args, "command:menu"))
            mmdagent->getMenu()->show();
      if (MMDAgent_strequal(args, "command:log"))
         mmdagent->procDisplayLogMessage();
#endif /* ENABLE_RAPID */
   }
   if (caption_enabled) {
      if (enable == true && MMDAgent_strequal(type, JULIUSTHREAD_EVENTSTOP)) {
         mmdagent->sendLogString(mid, MLOG_MESSAGE_CAPTURED, "%s|%s", type, args);
         sendCaption(args, JULIUSLOGGER_CCUSER, 0.2f, 14.0f);
      } else if (enable == true && MMDAgent_strequal(type, "SYNTH_START")) {
         mmdagent->sendLogString(mid, MLOG_MESSAGE_CAPTURED, "%s|%s", type, args);
         buff = MMDAgent_strdup(args);
         p1 = MMDAgent_strtok(buff, "|", &save);
         p1 = MMDAgent_strtok(NULL, "|", &save);
         p2 = MMDAgent_strtok(NULL, "|", &save);
         sendCaption(p2, JULIUSLOGGER_CCSYSTEM, -0.2f, 19.0f);
         /* set long duration to keep displaying until SYNTH_STOP comes */
         julius_thread.setLogCaptionDuration(JULIUSLOGGER_CCSYSTEM, 600.0f);
         free(buff);
      } else if (enable == true && MMDAgent_strequal(type, "SYNTH_EVENT_STOP")) {
         mmdagent->sendLogString(mid, MLOG_MESSAGE_CAPTURED, "%s|%s", type, args);
         /* at end of tts, set duration timer to erase caption */
         julius_thread.setLogCaptionDuration(JULIUSLOGGER_CCSYSTEM, JULIUSLOGGER_CCDURATIONFRAME);
      }
   }
}

/* extAppEnd: stop and free thread */
EXPORT void extAppEnd(MMDAgent *mmdagent)
{
   julius_thread.stopAndRelease();
   enable = false;
}

/* extUpdate: update log view */
EXPORT void extUpdate(MMDAgent *mmdagent, double frame)
{
   if (enable == true)
      julius_thread.updateLog(frame);
}

/* extRender2D: render in 2D screen */
EXPORT void extRender2D(MMDAgent *mmdagent, float screenWidth, float screenHeight)
{
   if (enable == true)
      julius_thread.renderLog(screenWidth, screenHeight);
}
