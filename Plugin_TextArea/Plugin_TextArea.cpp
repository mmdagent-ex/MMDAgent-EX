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
#include "CameraImage.h"
#include "TextArea.h"

/* definitions */

#ifdef _WIN32
#define EXPORT extern "C" __declspec(dllexport)
#else
#ifdef MMDAGENT_PLUGIN_STATIC
#define EXPORT
#define extAppStart Plugin_TextArea_extAppStart
#define extProcMessage Plugin_TextArea_extProcMessage
#define extUpdate Plugin_TextArea_extUpdate
#define extRender Plugin_TextArea_extRender
#define extAppEnd Plugin_TextArea_extAppEnd
#else
#define EXPORT extern "C"
#endif
#endif /* _WIN32 */

/* Plugin name, commands and events */
#define PLUGINTEXTAREA_NAME "TextArea"
#define PLUGINTEXTAREA_ADDCOMMAND "TEXTAREA_ADD"
#define PLUGINTEXTAREA_DELETECOMMAND "TEXTAREA_DELETE"
#define PLUGINTEXTAREA_SETCOMMAND  "TEXTAREA_SET"
#define PLUGINTEXTAREA_ADDEVENT   "TEXTAREA_EVENT_ADD"
#define PLUGINTEXTAREA_DELETEEVENT "TEXTAREA_EVENT_DELETE"
#define PLUGINTEXTAREA_SETEVENT  "TEXTAREA_EVENT_SET"

/* structures */
typedef struct _TextAreaList {
   TextArea textarea;
   struct _TextAreaList *next;
} TextAreaList;

/* variables */
static TextAreaList *talist;
static int mid;
static bool enable;

/* togglePlugin: toggle plugin */
static void togglePlugin(MMDAgent *mmdagent)
{
   TextAreaList *t;

   enable = !enable;

   for (t = talist; t; t = t->next) {
      t->textarea.setActiveFlag(enable);
   }

   if(enable)
      mmdagent->sendMessage(mid, MMDAGENT_EVENT_PLUGINENABLE, "%s", PLUGINTEXTAREA_NAME);
   else
      mmdagent->sendMessage(mid, MMDAGENT_EVENT_PLUGINDISABLE, "%s", PLUGINTEXTAREA_NAME);
}

/* extAppStart: initialize */
EXPORT void extAppStart(MMDAgent *mmdagent)
{
   glewInit();
   glfwInit();
   mid = mmdagent->getModuleId(PLUGINTEXTAREA_NAME);
   talist = NULL;
   enable = false;
   togglePlugin(mmdagent);
}

/* extProcMessage: process message */
EXPORT void extProcMessage(MMDAgent *mmdagent, const char *type, const char *args)
{
   TextAreaList *t, *tprev, *n;

   if(MMDAgent_strequal(type, MMDAGENT_COMMAND_PLUGINENABLE)) {
      if(MMDAgent_strequal(args, PLUGINTEXTAREA_NAME) && enable == false) {
         mmdagent->sendLogString(mid, MLOG_MESSAGE_CAPTURED, "%s|%s", type, args);
         togglePlugin(mmdagent);
      }
   } else if(MMDAgent_strequal(type, MMDAGENT_COMMAND_PLUGINDISABLE)) {
      if(MMDAgent_strequal(args, PLUGINTEXTAREA_NAME) && enable == true) {
         mmdagent->sendLogString(mid, MLOG_MESSAGE_CAPTURED, "%s|%s", type, args);
         togglePlugin(mmdagent);
      }
   } else if(MMDAgent_strequal(type, PLUGINTEXTAREA_ADDCOMMAND)) {
      mmdagent->sendLogString(mid, MLOG_MESSAGE_CAPTURED, "%s|%s", type, args);
      for (t = talist; t; t = t->next) {
         if(t->textarea.matchName(args)) {
            break;
         }
      }
      if (t == NULL) {
         n = new TextAreaList;
         if (n->textarea.setup(mmdagent, mid, args) == false) {
            delete n;
         } else {
            n->next = talist;
            talist = n;
            mmdagent->sendMessage(mid, PLUGINTEXTAREA_ADDEVENT, "%s", n->textarea.getName());
         }
      } else {
         mmdagent->sendLogString(mid, MLOG_WARNING, "arg 2: alias %s already exists", t->textarea.getName());
      }
   } else if(MMDAgent_strequal(type, PLUGINTEXTAREA_DELETECOMMAND)) {
      mmdagent->sendLogString(mid, MLOG_MESSAGE_CAPTURED, "%s|%s", type, args);
      tprev = NULL;
      for (t = talist; t; t = t->next) {
         if(t->textarea.matchName(args)) {
            break;
         }
         tprev = t;
      }
      if (t != NULL) {
         if (tprev)
            tprev->next = t->next;
         else
            talist = t->next;
         mmdagent->sendMessage(mid, PLUGINTEXTAREA_DELETEEVENT, "%s", t->textarea.getName());
         delete t;
      }
   } else if(MMDAgent_strequal(type, PLUGINTEXTAREA_SETCOMMAND)) {
      mmdagent->sendLogString(mid, MLOG_MESSAGE_CAPTURED, "%s|%s", type, args);
      for (t = talist; t; t = t->next) {
         if(t->textarea.matchName(args)) {
            break;
         }
      }
      if (t != NULL) {
         t->textarea.setText(args);
         mmdagent->sendMessage(mid, PLUGINTEXTAREA_SETEVENT, "%s", t->textarea.getName());
      }
   }
}

/* extUpdate: update */
EXPORT void extUpdate(MMDAgent *mmdagent, double frame)
{
   TextAreaList *t;
   for (t = talist; t; t = t->next)
      t->textarea.update(frame);
}

/* extRender: render */
EXPORT void extRender(MMDAgent *mmdagent)
{
   TextAreaList *t;

   if (talist == NULL)
      return;

   glDisable(GL_CULL_FACE);
   glDisable(GL_LIGHTING);
   glEnable(GL_TEXTURE_2D);
   glActiveTexture(GL_TEXTURE0);
   glClientActiveTexture(GL_TEXTURE0);
   glEnableClientState(GL_VERTEX_ARRAY);

   for (t = talist; t; t = t->next)
      t->textarea.render();

   glDisableClientState(GL_VERTEX_ARRAY);
   glDisable(GL_TEXTURE_2D);
   glEnable(GL_LIGHTING);
   glEnable(GL_CULL_FACE);
}

/* extAppEnd: end of application */
EXPORT void extAppEnd(MMDAgent *mmdagent)
{
   TextAreaList *t, *tmp;

   t = talist;
   while(t) {
      tmp = t->next;
      delete t;
      t = tmp;
   }
   talist = NULL;
   enable = false;
}
