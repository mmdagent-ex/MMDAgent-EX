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

/* DLLibrary: dynamic link library for MMDAgent */
typedef struct _DLLibrary {
   char *name;
   void *handle;

   void (*appInit)(MMDAgent *mmdagent);
   void (*appStart)(MMDAgent *mmdagent);
   void (*appEnd)(MMDAgent *mmdagent);
   void (*procCommand)(MMDAgent *mmdagent, const char *type, const char *args); /* deprecated function */
   void (*procEvent)(MMDAgent *mmdagent, const char *type, const char *args);   /* deprecated function */
   void (*procMessage)(MMDAgent *mmdagent, const char *type, const char *args);
   void (*update)(MMDAgent *mmdagent, double deltaFrame);
   void (*render)(MMDAgent *mmdagent);
   void (*render2D)(MMDAgent *mmdagent, float screenWidth, float screenHeight);
   void (*log)(MMDAgent *mmdagent, int id, unsigned int flag, const char *text, const char *fulltext);

   struct _DLLibrary *next;
} DLLibrary;

/* Plugin: plugin list */
class Plugin
{
private:

   DLLibrary *m_head;   /* head of list */
   DLLibrary *m_tail;   /* tail of list */

   /* validation order: disable -> enable */
   /*
   plugin will be enabled by default.
   "ALL" can be used as wild card, "NONE" to specify nothing
   if matches disabled, it will be disabled.
   if matches enabled, it will be enabled even if it matched disabled.

   user .mdf setting will "override" previous .mdf setting (not append, overwrite)

   - avoid running some plugins
        disablePlugin=xxx,yyy
   - load only the specified plugins:
        disablePlugin=ALL
        enablePlugin=www,zzz
   - system mdf disable all plugin but a content want to use all
        disablePlugin=NONE
     or
        enablePlugin=ALL

   */
   KeyValue *m_disabledPluginNames; /* list of disabled plugin names */
   KeyValue *m_enabledPluginNames;  /* list of enabled plugin names */
   bool m_disableAll;
   bool m_enableAll;

#ifdef MMDAGENT_PLUGIN_STATIC
   bool enable_Variables;
   bool enable_VIManager;
   bool enable_Julius;
   bool enable_Open_JTalk;
   bool enable_Flite_plus_hts_engine;
   bool enable_Audio;
   bool enable_TextArea;
   bool enable_Kafka;
#endif

   /* initialize: initialize plugin list */
   void initialize();

   /* clear: free plugin list */
   void clear();

public:

   /* Plugin: constructor */
   Plugin();

   /* ~Plugin: destructor */
   ~Plugin();

   /* setList: set enable and disable list */
   void setList(const char *disableListStr, const char *enableListStr);

   /* isAllowed: return if the plugin is allowed to be loaded or not */
   bool isAllowed(MMDAgent *mmdagent, const char *pluginName);

   /* load: load all DLLs in a directory */
   bool load(MMDAgent *mmdagent, int id, const char *dir);

   /* execAppInit: run when application is initialized */
   void execAppInit(MMDAgent *mmdagent);

   /* execAppStart: run when application is ready to start */
   void execAppStart(MMDAgent *mmdagent);

   /* execAppEnd: run when application ends */
   void execAppEnd(MMDAgent *mmdagent);

   /* execProcMessage: process message */
   void execProcMessage(MMDAgent *mmdagent, const char *type, const char *args);

   /* execUpdate: run when motion is updated */
   void execUpdate(MMDAgent *mmdagent, double deltaFrame);

   /* execRender: run when scene is rendered */
   void execRender(MMDAgent *mmdagent);

   /* execRender2D: run when scene is rendered */
   void execRender2D(MMDAgent *mmdagent, float screenWidth, float screenHeight);

   /* hasRender2D: return if there are any plugin with render2D function */
   bool hasRender2D();

   /* execLog: run for each logging message */
   void execLog(MMDAgent *mmdagent, int id, unsigned int flag, const char *text, const char *fulltext);

};
