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

#define PLUGINLOOKAT_NAME "LookAt"

/* headers */

#include "MMDAgent.h"
#include "BoneController.h"

/* variables */

typedef struct _ControllerList {
   BoneController head;
   BoneController eye;
   struct _ControllerList *next;
} ControllerList;

static int mid;
static ControllerList *controllerList;
static bool enable;
static bool updating;

/* setHeadController: set bone controller to head */
static void setHeadController(BoneController *controller, PMDModel *model)
{
   const char *bone[] = {"\xe9\xa0\xad"}; /* head */
   controller->setup(model, bone, 1, 0.060f, 0.008f, 0.0f, 0.0f, 1.0f, 20.0f, 60.0f, 0.0f, -45.0f, -60.0f, 0.0f, 0.0f, -1.0f, 0.0f);
}

/* setEyeController: set eye controller to eyes */
static void setEyeController(BoneController *controller, PMDModel *model)
{
   const char *bone[] = {"\xe4\xb8\xa1\xe7\x9b\xae"}; /* both eye */
   controller->setup(model, bone, 1, 0.300f, 0.008f, 0.0f, 0.0f, 1.0f, 30.0f, 60.0f, 0.0f, -30.0f, -60.0f, 0.0f, 0.0f, 0.0f, 0.0f);
}

/* changeLookAt: switch LookAt */
static void changeLookAt(PMDObject *objs, int num, MMDAgent *mmdagent)
{
   int i;
   ControllerList *tmp;

   for(i = 0, tmp = controllerList; i < num; i++) {
      if(objs[i].isEnable() == true && tmp != NULL) {
         if(enable == true) {
            tmp->head.setEnableFlag(false);
            tmp->eye.setEnableFlag(false);
         } else {
            tmp->head.setEnableFlag(true);
            tmp->eye.setEnableFlag(true);
         }
      }
      if(tmp != NULL)
         tmp = tmp->next;
   }
   updating = true;
   enable = !enable;
   if(enable)
      mmdagent->sendMessage(mid, MMDAGENT_EVENT_PLUGINENABLE, "%s", PLUGINLOOKAT_NAME);
   else
      mmdagent->sendMessage(mid, MMDAGENT_EVENT_PLUGINDISABLE, "%s", PLUGINLOOKAT_NAME);
}

/* extAppStart: initialize controller */
EXPORT void extAppStart(MMDAgent *mmdagent)
{
   controllerList = NULL;
   enable = false;
   updating = false;
   mid = mmdagent->getModuleId(PLUGINLOOKAT_NAME);
}

/* extProcMessage: process message */
EXPORT void extProcMessage(MMDAgent *mmdagent, const char *type, const char *args)
{
   int i, id;
   char *p, *buf, *save;
   PMDObject *objs;
   ControllerList *tmp1, *tmp2 = NULL;

   if(MMDAgent_strequal(type, MMDAGENT_COMMAND_PLUGINENABLE)) {
      if(MMDAgent_strequal(args, PLUGINLOOKAT_NAME) && enable == false) {
         mmdagent->sendLogString(mid, MLOG_MESSAGE_CAPTURED, "%s|%s", type, args);
         changeLookAt(mmdagent->getModelList(), mmdagent->getNumModel(), mmdagent);
      }
   } else if(MMDAgent_strequal(type, MMDAGENT_COMMAND_PLUGINDISABLE)) {
      if(MMDAgent_strequal(args, PLUGINLOOKAT_NAME) && enable == true) {
         mmdagent->sendLogString(mid, MLOG_MESSAGE_CAPTURED, "%s|%s", type, args);
         changeLookAt(mmdagent->getModelList(), mmdagent->getNumModel(), mmdagent);
      }
   } else if(MMDAgent_strequal(type, MMDAGENT_EVENT_KEY)) {
      if(MMDAgent_strequal(args, "l")) {
         mmdagent->sendLogString(mid, MLOG_MESSAGE_CAPTURED, "%s|%s", type, args);
         changeLookAt(mmdagent->getModelList(), mmdagent->getNumModel(), mmdagent);
      }
   } else if(MMDAgent_strequal(type, MMDAGENT_EVENT_MODELCHANGE) || MMDAgent_strequal(type, MMDAGENT_EVENT_MODELADD)) {
      mmdagent->sendLogString(mid, MLOG_MESSAGE_CAPTURED, "%s|%s", type, args);
      objs = mmdagent->getModelList();
      buf = MMDAgent_strdup(args);
      p = MMDAgent_strtok(buf, "|", &save);
      if(p) {
         id = mmdagent->findModelAlias(p);
         if(id != -1) {
            for(i = 0, tmp1 = controllerList; i <= id; i++) {
               if(tmp1 == NULL) {
                  tmp1 = new ControllerList;
                  tmp1->next = NULL;
                  if(i == 0)
                     controllerList = tmp1;
                  else
                     tmp2->next = tmp1;
               }
               if(i == id) {
                  setHeadController(&tmp1->head, objs[id].getPMDModel());
                  setEyeController(&tmp1->eye, objs[id].getPMDModel());
                  tmp1->head.setEnableFlag(enable);
                  tmp1->eye.setEnableFlag(enable);
               }
               tmp2 = tmp1;
               tmp1 = tmp1->next;
            }
         }
      }
      if(buf != NULL)
         free(buf);
   }
}

/* extUpdate: update motions */
EXPORT void extUpdate(MMDAgent *mmdagent, double deltaFrame)
{
   int i;
   float rate;
   PMDObject *objs;
   btVector3 targetPos, pointPos;
   int windowWidth, windowHeight;
   int mousePosX, mousePosY;
   ControllerList *tmp;
   bool hasUpdates;

   if (updating == false)
      return;
   if (controllerList == NULL)
      return;

   /* set target position */
   mmdagent->getWindowSize(&windowWidth, &windowHeight);
   mmdagent->getMousePosition(&mousePosX, &mousePosY);
   mousePosX -= windowWidth / 2;
   mousePosY -= windowHeight / 2;
   rate = 200.0f / (float)(windowWidth);
   pointPos.setValue(btScalar(mousePosX * rate), btScalar(-mousePosY * rate), btScalar(0.0f));
   mmdagent->getScreenPointPosition(&targetPos, &pointPos);

   /* calculate direction of all controlled bones */
   hasUpdates = false;
   objs = mmdagent->getModelList();
   for (i = 0, tmp = controllerList; i < mmdagent->getNumModel(); i++) {
      if (objs[i].isEnable() == true && tmp != NULL) {
         if (tmp->head.update(&targetPos, (float) deltaFrame) == true)
            hasUpdates = true;
         if (tmp->eye.update(&targetPos, (float) deltaFrame) == true)
            hasUpdates = true;
      }
      if(tmp != NULL)
         tmp = tmp->next;
   }

   if (hasUpdates == false) {
      /* no further update, stop updating */
      updating = false;
   }
}

EXPORT void extAppEnd(MMDAgent *mmdagent)
{
   ControllerList *tmp1, *tmp2;

   for(tmp1 = controllerList; tmp1 != NULL; tmp1 = tmp2) {
      tmp2 = tmp1->next;
      delete tmp1;
   }
   controllerList = NULL;
   enable = false;
}
