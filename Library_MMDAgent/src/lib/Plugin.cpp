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
#if !defined(_WIN32) && !defined(__ANDROID__)
#if TARGET_OS_IPHONE
#else
#include "dlfcn.h"
#endif
#endif

#ifdef MMDAGENT_PLUGIN_STATIC
/* since iOS cannot use DLL, plugin functions are specified in this class */
void Plugin_Audio_extAppStart(MMDAgent *mmdagent);
void Plugin_Audio_extProcMessage(MMDAgent *mmdagent, const char *type, const char *args);
void Plugin_Audio_extAppEnd(MMDAgent *mmdagent);
void Plugin_Flite_plus_hts_engine_extAppStart(MMDAgent *mmdagent);
void Plugin_Flite_plus_hts_engine_extProcMessage(MMDAgent *mmdagent, const char *type, const char *args);
void Plugin_Flite_plus_hts_engine_extAppEnd(MMDAgent *mmdagent);
void Plugin_Julius_extAppStart(MMDAgent *mmdagent);
void Plugin_Julius_extProcMessage(MMDAgent *mmdagent, const char *type, const char *args);
void Plugin_Julius_extAppEnd(MMDAgent *mmdagent);
void Plugin_Julius_extUpdate(MMDAgent *mmdagent, double deltaFrame);
void Plugin_Julius_extRender2D(MMDAgent *mmdagent, float screenWidth, float screenHeight);
void Plugin_Open_JTalk_extAppStart(MMDAgent *mmdagent);
void Plugin_Open_JTalk_extProcMessage(MMDAgent *mmdagent, const char *type, const char *args);
void Plugin_Open_JTalk_extAppEnd(MMDAgent *mmdagent);
void Plugin_TextArea_extAppStart(MMDAgent *mmdagent);
void Plugin_TextArea_extProcMessage(MMDAgent *mmdagent, const char *type, const char *args);
void Plugin_TextArea_extUpdate(MMDAgent *mmdagent, double deltaFrame);
void Plugin_TextArea_extRender(MMDAgent *mmdagent);
void Plugin_TextArea_extAppEnd(MMDAgent *mmdagent);
void Plugin_Variables_extAppStart(MMDAgent *mmdagent);
void Plugin_Variables_extProcMessage(MMDAgent *mmdagent, const char *type, const char *args);
void Plugin_Variables_extAppEnd(MMDAgent *mmdagent);
void Plugin_VIManager_extAppStart(MMDAgent *mmdagent);
void Plugin_VIManager_extProcMessage(MMDAgent *mmdagent, const char *type, const char *args);
void Plugin_VIManager_extRender(MMDAgent *mmdagent);
void Plugin_VIManager_extAppEnd(MMDAgent *mmdagent);
void Plugin_Kafka_extAppStart(MMDAgent *mmdagent);
void Plugin_Kafka_extProcMessage(MMDAgent *mmdagent, const char *type, const char *args);
void Plugin_Kafka_extUpdate(MMDAgent *mmdagent, double deltaFrame);
void Plugin_Kafka_extLog(MMDAgent *mmdagent, int id, unsigned int flag, const char *text, const char *fulltext);
void Plugin_Kafka_extRender2D(MMDAgent *mmdagent, float screenWidth, float screenHeight);
void Plugin_Kafka_extAppEnd(MMDAgent *mmdagent);
#endif /* MMDAGENT_PLUGIN_STATIC */

/* DLLibrary_initialize: initialize dynamic link library */
void DLLibrary_initialize(DLLibrary *d)
{
   d->name = NULL;
   d->handle = NULL;

   d->appInit = NULL;
   d->appStart = NULL;
   d->appEnd = NULL;
   d->procCommand = NULL; /* deprecated function */
   d->procEvent = NULL;   /* deprecated function */
   d->procMessage = NULL;
   d->update = NULL;
   d->render = NULL;
   d->render2D = NULL;
   d->log = NULL;

   d->next = NULL;
}

/* DLLibrary_clear: free dynamic link library */
void DLLibrary_clear(DLLibrary *d)
{
   if(d->handle != NULL)
      MMDAgent_dlclose(d->handle);
   if (d->name)
      free(d->name);

   DLLibrary_initialize(d);
}

/* DLLibrary_load: load dynamic link library */
bool DLLibrary_load(DLLibrary *d, const char *dir, const char *file)
{
   char *buf;

   if(d == NULL || dir == NULL || file == NULL) return false;
   DLLibrary_clear(d);

   /* open */
   int plen = sizeof(char) * (MMDAgent_strlen(dir) + 1 + MMDAgent_strlen(file) + 1);
   buf = (char *) malloc(plen);
   MMDAgent_snprintf(buf, plen, "%s%c%s", dir, MMDAGENT_DIRSEPARATOR, file);
   d->handle = MMDAgent_dlopen(buf);
   free(buf);
   if (!d->handle)
      return false;

   /* set function pointers */
   d->appInit = (void(*)(MMDAgent *)) MMDAgent_dlsym(d->handle, "extAppInit");
   d->appStart = (void(*)(MMDAgent *)) MMDAgent_dlsym(d->handle, "extAppStart");
   d->appEnd = (void (*)(MMDAgent *)) MMDAgent_dlsym(d->handle, "extAppEnd");
   d->procCommand = (void (*)(MMDAgent *, const char *, const char *)) MMDAgent_dlsym(d->handle, "extProcCommand"); /* deprecated function */
   d->procEvent = (void (*)(MMDAgent *, const char *, const char *)) MMDAgent_dlsym(d->handle, "extProcEvent");     /* deprecated function */
   d->procMessage = (void (*)(MMDAgent *, const char *, const char *)) MMDAgent_dlsym(d->handle, "extProcMessage");
   d->update = (void (*)(MMDAgent *, double)) MMDAgent_dlsym(d->handle, "extUpdate");
   d->render = (void (*)(MMDAgent *)) MMDAgent_dlsym(d->handle, "extRender");
   d->render2D = (void(*)(MMDAgent *, float, float)) MMDAgent_dlsym(d->handle, "extRender2D");
   d->log = (void(*)(MMDAgent *, int, unsigned int, const char *, const char *)) MMDAgent_dlsym(d->handle, "extLog");

   if (d->appInit || d->appStart || d->appEnd || d->procCommand || d->procEvent || d->procMessage || d->update || d->render || d->log) {
      /* save file name */
      d->name = MMDAgent_strdup(file);
      return true;
   } else {
      /* if none, exit */
      DLLibrary_clear(d);
      return false;
   }
}

/* Plugin::initialize: initialize plugin list */
void Plugin::initialize()
{
   m_head = NULL;
   m_tail = NULL;
#ifdef MMDAGENT_PLUGIN_STATIC
   enable_Variables = true;
   enable_VIManager = true;
   enable_Julius = true;
   enable_Open_JTalk = true;
   enable_Flite_plus_hts_engine = true;
   enable_Audio = true;
   enable_TextArea = true;
   enable_Kafka = true;
#endif
   m_disabledPluginNames = NULL;
   m_enabledPluginNames = NULL;
   m_disableAll = false;
   m_enableAll = false;

}

/* Plugin::clear: free plugin list */
void Plugin::clear()
{
   DLLibrary *d1, *d2;

   for (d1 = m_head; d1; d1 = d2) {
      d2 = d1->next;
      DLLibrary_clear(d1);
      free(d1);
   }
   if (m_disabledPluginNames)
      delete m_disabledPluginNames;
   if (m_enabledPluginNames)
      delete m_enabledPluginNames;

   initialize();
}

/* Plugin::Plugin: constructor */
Plugin::Plugin()
{
   initialize();
}

/* Plugin::~Plugin: destructor */
Plugin::~Plugin()
{
   clear();
}

/* Plugin::setList: set enable and disable list */
void Plugin::setList(const char *disableListStr, const char *enableListStr)
{
   char buff[MMDAGENT_MAXBUFLEN];
   char *p, *psave;

   if (disableListStr) {
      if (m_disabledPluginNames)
         delete m_disabledPluginNames;
      m_disabledPluginNames = NULL;
      if (MMDAgent_strequal(disableListStr, "ALL")) {
         m_disableAll = true;
      } else if (MMDAgent_strequal(disableListStr, "NONE")) {
         m_disableAll = false;
      } else {
         m_disableAll = false;
         m_disabledPluginNames = new KeyValue;
         m_disabledPluginNames->setup();
         strncpy(buff, disableListStr, MMDAGENT_MAXBUFLEN - 1);
         buff[MMDAGENT_MAXBUFLEN - 1] = '\0';
         for (p = MMDAgent_strtok(buff, ",", &psave); p; p = MMDAgent_strtok(NULL, ",", &psave)) {
            m_disabledPluginNames->setString(p, "hasEntry");
         }
      }
   }
   if (enableListStr) {
      if (m_enabledPluginNames)
         delete m_enabledPluginNames;
      m_enabledPluginNames = NULL;
      if (MMDAgent_strequal(enableListStr, "ALL")) {
         m_enableAll = true;
      } else if (MMDAgent_strequal(enableListStr, "NONE")) {
         m_enableAll = false;
      } else {
         m_enableAll = false;
         m_enabledPluginNames = new KeyValue;
         m_enabledPluginNames->setup();
         strncpy(buff, enableListStr, MMDAGENT_MAXBUFLEN - 1);
         buff[MMDAGENT_MAXBUFLEN - 1] = '\0';
         for (p = MMDAgent_strtok(buff, ",", &psave); p; p = MMDAgent_strtok(NULL, ",", &psave)) {
            m_enabledPluginNames->setString(p, "hasEntry");
         }
      }
   }
}

/* Plugin::isAllowed: return if the plugin is allowed to be loaded or not */
bool Plugin::isAllowed(MMDAgent *mmdagent, const char *pluginName)
{
   char buff[MMDAGENT_MAXBUFLEN];

   if (m_enableAll == true)
      return true;

   if (m_enabledPluginNames && m_enabledPluginNames->exist(pluginName))
      return true;

   if (m_disableAll == true)
      return false;

   if (m_disabledPluginNames && m_disabledPluginNames->exist(pluginName))
      return false;

   /* old compatibility */
   /* exlude it if "exclude_[filename]=true" exists in .mdf */
   MMDAgent_snprintf(buff, MMDAGENT_MAXBUFLEN, "exclude_Plugin_%s", pluginName);
   if (MMDAgent_strequal(mmdagent->getKeyValue()->getString(buff, "no"), "yes") || MMDAgent_strequal(mmdagent->getKeyValue()->getString(buff, "no"), "true"))
      return false;

   return true;
}

/* Plugin::load: load all DLLs in a directory */
bool Plugin::load(MMDAgent *mmdagent, int id, const char *dir)
{
   DIRECTORY *dp;
   char buf[MMDAGENT_MAXBUFLEN];
   char buf2[MMDAGENT_MAXBUFLEN];
   bool ret = false;
   DLLibrary *d;
   int i;

#ifdef MMDAGENT_PLUGIN_STATIC
   /* just check if they can be enabled */
   const char *s;

   s = mmdagent->getKeyValue()->getString("exclude_Plugin_Variable", "no");
   if (MMDAgent_strequal(s, "yes") || MMDAgent_strequal(s, "true")) {
      mmdagent->sendLogString(id, MLOG_STATUS, "plugin \"Plugin_Variable\" -> not loaded", buf);
      enable_Variables = false;
   }
   s = mmdagent->getKeyValue()->getString("exclude_Plugin_VIManager", "no");
   if (MMDAgent_strequal(s, "yes") || MMDAgent_strequal(s, "true")) {
      mmdagent->sendLogString(id, MLOG_STATUS, "plugin \"Plugin_VIManager\" -> not loaded", buf);
      enable_VIManager = false;
   }
   s = mmdagent->getKeyValue()->getString("exclude_Plugin_Julius", "no");
   if (MMDAgent_strequal(s, "yes") || MMDAgent_strequal(s, "true")) {
      mmdagent->sendLogString(id, MLOG_STATUS, "plugin \"Plugin_Julius\" -> not loaded", buf);
      enable_Julius = false;
   }
   s = mmdagent->getKeyValue()->getString("exclude_Plugin_Open_JTalk", "no");
   if (MMDAgent_strequal(s, "yes") || MMDAgent_strequal(s, "true")) {
      mmdagent->sendLogString(id, MLOG_STATUS, "plugin \"Plugin_Open_JTalk\" -> not loaded", buf);
      enable_Open_JTalk = false;
   }
   s = mmdagent->getKeyValue()->getString("exclude_Plugin_Flite_plus_hts_engine", "no");
   if (MMDAgent_strequal(s, "yes") || MMDAgent_strequal(s, "true")) {
      mmdagent->sendLogString(id, MLOG_STATUS, "plugin \"Plugin_Flite_plus_hts_engine\" -> not loaded", buf);
      enable_Flite_plus_hts_engine = false;
   }
   s = mmdagent->getKeyValue()->getString("exclude_Plugin_Audio", "no");
   if (MMDAgent_strequal(s, "yes") || MMDAgent_strequal(s, "true")) {
      mmdagent->sendLogString(id, MLOG_STATUS, "plugin \"Plugin_Audio\" -> not loaded", buf);
      enable_Audio = false;
   }
   s = mmdagent->getKeyValue()->getString("exclude_Plugin_TextArea", "no");
   if (MMDAgent_strequal(s, "yes") || MMDAgent_strequal(s, "true")) {
      mmdagent->sendLogString(id, MLOG_STATUS, "plugin \"Plugin_TextArea\" -> not loaded", buf);
      enable_TextArea = false;
   }
   s = mmdagent->getKeyValue()->getString("exclude_Plugin_Kafka", "no");
   if (MMDAgent_strequal(s, "yes") || MMDAgent_strequal(s, "true")) {
      mmdagent->sendLogString(id, MLOG_STATUS, "plugin \"Plugin_Kafka\" -> not loaded", buf);
      enable_Kafka = false;
   }
   return true;
#endif

   if(dir == NULL) {
      mmdagent->sendLogString(id, MLOG_ERROR, "no plugin dir");
      return false;
   }

   /* search file */
   dp = MMDAgent_opendir(dir);
   if(dp == NULL) {
      mmdagent->sendLogString(id, MLOG_ERROR, "failed to open plugin dir");
      return false;
   }

   /* add */
   while(MMDAgent_readdir(dp, buf) == true) {
      if(MMDAgent_strtailmatch(buf, ".dll") == true || MMDAgent_strtailmatch(buf, ".DLL") == true || MMDAgent_strtailmatch(buf, ".so") == true || MMDAgent_strtailmatch(buf, ".SO") == true) {
#ifdef __ANDROID__
		   /* avoid loading self */
         if (MMDAgent_strequal(buf, "libmain.so"))
            continue;
#endif
         /* determine if the plugin is allowed to be loaded or not */
         /* Plugin_*.dll */
#ifdef __ANDROID__
         /* android plugin has "lib" prefix */
         if (MMDAgent_strlen(buf) < 10)
            continue;
         strncpy(buf2, &(buf[10]), MMDAGENT_MAXBUFLEN - 1);
#else
         if (MMDAgent_strlen(buf) < 7)
            continue;
         strncpy(buf2, &(buf[7]), MMDAGENT_MAXBUFLEN - 1);
#endif
         i = MMDAgent_strlen(buf2) - 1;
         while (i >= 0 && buf2[i] != '.') i--;
         if (i < 0)
            continue;
         buf2[i] = '\0';

         if (isAllowed(mmdagent, buf2) == false) {
            mmdagent->sendLogString(id, MLOG_STATUS, "plugin \"Plugin_%s\" -> not loaded", buf);
            continue;
         }

         d = (DLLibrary *) malloc(sizeof(DLLibrary));
         DLLibrary_initialize(d);
         if(DLLibrary_load(d, dir, buf) == false) {
#if !defined(_WIN32) && !defined(__ANDROID__)
#if TARGET_OS_IPHONE
#else
            mmdagent->sendLogString(id, MLOG_ERROR, "Error: dlopen: %s", dlerror());
#endif
#endif
            mmdagent->sendLogString(id, MLOG_ERROR, "failed to load plugin \"%s\"", buf);
            free(d);
         } else {
            mmdagent->sendLogString(id, MLOG_STATUS, "plugin \"%s\"", buf);
            if(m_tail == NULL)
               m_head = d;
            else
               m_tail->next = d;
            m_tail = d;
            ret = true;
         }
      }
   }

   /* end */
   MMDAgent_closedir(dp);

   return ret;
}

/* Plugin::execAppInit: run when application is initialized */
void Plugin::execAppInit(MMDAgent *mmdagent)
{
   DLLibrary *d;

   for (d = m_head; d; d = d->next)
      if (d->appInit != NULL)
         d->appInit(mmdagent);
}

/* Plugin::execAppStart: run when application is ready to start */
void Plugin::execAppStart(MMDAgent *mmdagent)
{
   DLLibrary *d;

   for (d = m_head; d; d = d->next)
      if (d->appStart != NULL)
         d->appStart(mmdagent);

#ifdef MMDAGENT_PLUGIN_STATIC
   if (enable_Variables) Plugin_Variables_extAppStart(mmdagent);
   if (enable_VIManager) Plugin_VIManager_extAppStart(mmdagent);
   if (enable_Julius) Plugin_Julius_extAppStart(mmdagent);
   if (enable_Open_JTalk) Plugin_Open_JTalk_extAppStart(mmdagent);
   if (enable_Flite_plus_hts_engine) Plugin_Flite_plus_hts_engine_extAppStart(mmdagent);
   if (enable_Audio) Plugin_Audio_extAppStart(mmdagent);
   if (enable_TextArea) Plugin_TextArea_extAppStart(mmdagent);
   if (enable_Kafka) Plugin_Kafka_extAppStart(mmdagent);
#endif
}

/* Plugin::execAppEnd: run when application ends */
void Plugin::execAppEnd(MMDAgent *mmdagent)
{
   DLLibrary *d;

   for (d = m_head; d; d = d->next)
      if (d->appEnd != NULL)
         d->appEnd(mmdagent);

#ifdef MMDAGENT_PLUGIN_STATIC
   if (enable_Variables) Plugin_Variables_extAppEnd(mmdagent);
   if (enable_VIManager) Plugin_VIManager_extAppEnd(mmdagent);
   if (enable_Julius) Plugin_Julius_extAppEnd(mmdagent);
   if (enable_Open_JTalk) Plugin_Open_JTalk_extAppEnd(mmdagent);
   if (enable_Flite_plus_hts_engine) Plugin_Flite_plus_hts_engine_extAppEnd(mmdagent);
   if (enable_Audio) Plugin_Audio_extAppEnd(mmdagent);
   if (enable_TextArea) Plugin_TextArea_extAppEnd(mmdagent);
   if (enable_Kafka) Plugin_Kafka_extAppEnd(mmdagent);
#endif
}

/* Plugin::execProcMessage: run for each message */
void Plugin::execProcMessage(MMDAgent *mmdagent, const char *type, const char *args)
{
   DLLibrary *d;

   for (d = m_head; d; d = d->next)
      if (d->procMessage != NULL)
         d->procMessage(mmdagent, type, args);

   /* deprecated functions */
   for (d = m_head; d; d = d->next)
      if (d->procCommand != NULL)
         d->procCommand(mmdagent, type, args);

   for (d = m_head; d; d = d->next)
      if (d->procEvent != NULL)
         d->procEvent(mmdagent, type, args);

#ifdef MMDAGENT_PLUGIN_STATIC
   if (enable_Variables) Plugin_Variables_extProcMessage(mmdagent, type, args);
   if (enable_VIManager) Plugin_VIManager_extProcMessage(mmdagent, type, args);
   if (enable_Julius) Plugin_Julius_extProcMessage(mmdagent, type, args);
   if (enable_Open_JTalk) Plugin_Open_JTalk_extProcMessage(mmdagent, type, args);
   if (enable_Flite_plus_hts_engine)Plugin_Flite_plus_hts_engine_extProcMessage(mmdagent, type, args);
   if (enable_Audio) Plugin_Audio_extProcMessage(mmdagent, type, args);
   if (enable_TextArea) Plugin_TextArea_extProcMessage(mmdagent, type, args);
   if (enable_Kafka) Plugin_Kafka_extProcMessage(mmdagent, type, args);
#endif
}

/* Plugin::execUpdate: run when motion is updated */
void Plugin::execUpdate(MMDAgent *mmdagent, double deltaFrame)
{
   DLLibrary *d;

   for (d = m_head; d; d = d->next)
      if (d->update != NULL)
         d->update(mmdagent, deltaFrame);

#ifdef MMDAGENT_PLUGIN_STATIC
   if (enable_Julius) Plugin_Julius_extUpdate(mmdagent, deltaFrame);
   if (enable_TextArea) Plugin_TextArea_extUpdate(mmdagent, deltaFrame);
   if (enable_Kafka) Plugin_Kafka_extUpdate(mmdagent, deltaFrame);
#endif
}

/* Plugin::execRender: run when scene is rendered */
void Plugin::execRender(MMDAgent *mmdagent)
{
   DLLibrary *d;

   for (d = m_head; d; d = d->next)
      if (d->render != NULL)
         d->render(mmdagent);

#ifdef MMDAGENT_PLUGIN_STATIC
   if (enable_VIManager) Plugin_VIManager_extRender(mmdagent);
   if (enable_TextArea) Plugin_TextArea_extRender(mmdagent);
#endif
}

/* Plugin::execRender2D: run when scene is rendered */
void Plugin::execRender2D(MMDAgent *mmdagent, float screenWidth, float screenHeight)
{
   DLLibrary *d;

   for (d = m_head; d; d = d->next)
      if (d->render2D != NULL)
         d->render2D(mmdagent, screenWidth, screenHeight);

#ifdef MMDAGENT_PLUGIN_STATIC
   if (enable_Julius) Plugin_Julius_extRender2D(mmdagent, screenWidth, screenHeight);
   if (enable_Kafka) Plugin_Kafka_extRender2D(mmdagent, screenWidth, screenHeight);
#endif
}

/* Plugin::hasRender2D: return if there are any plugin with render2D function */
bool Plugin::hasRender2D()
{
   bool ret = false;
   DLLibrary *d;

   for (d = m_head; d; d = d->next) {
      if (d->render2D != NULL) {
         ret = true;
         break;
      }
   }
#ifdef MMDAGENT_PLUGIN_STATIC
   if (ret == false) {
      if (enable_Julius || enable_Kafka)
         ret = true;
   }
#endif
   return ret;
}

/* Plugin::execLog: run for each logging message */
void Plugin::execLog(MMDAgent *mmdagent, int id, unsigned int flag, const char *text, const char *fulltext)
{
   DLLibrary *d;

   for (d = m_head; d; d = d->next)
      if (d->log != NULL)
         d->log(mmdagent, id, flag, text, fulltext);
#ifdef MMDAGENT_PLUGIN_STATIC
   if (enable_Kafka) Plugin_Kafka_extLog(mmdagent, id, flag, text, fulltext);
#endif
}
