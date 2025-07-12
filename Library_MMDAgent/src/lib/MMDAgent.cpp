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
/*           Toolkit for Building Voice Interaction Systems          */
/*           MMDAgent developed by MMDAgent Project Team             */
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

#include <stdarg.h>
#include "MMDAgent.h"
#ifdef __ANDROID__
#include <sys/types.h>
#include <unistd.h>
#include "android/log.h"
#endif

#include <omp.h>

#ifndef HISTORYSAVEPATH
#define HISTORYSAVEPATH "_history"
#endif
#define HISTORYMAXNUM 10

#ifndef LASTPLAYEDDATAPATH
#define LASTPLAYEDDATAPATH "_lastplayed"
#endif

#define CENTER_LOGO_SIZE_RATE          0.3f

#ifdef __ANDROID__
/* debug function to get audio api name */
extern "C" {
   const char *Pa_AndroidGetApiName();
}
#endif

#ifdef MMDAGENT_CURRENTTIME

#define MMDAGENT_CURRENTTIME_INTERVAL_SEC 30
#define MMDAGENT_CURRENTTIME_INTERVAL_FRAME (MMDAGENT_CURRENTTIME_INTERVAL_SEC * 30.0)

/* thread function for issuing current time */
static void issueCurrentTimeMain(void *param)
{
   MMDAgent *mmdagent = (MMDAgent *)param;
   int year, month, day, hour, minute, sec, msec;
   MMDAgent_gettimeinfo(&year, &month, &day, &hour, &minute, &sec, &msec);
   mmdagent->sendMessage(mmdagent->getModuleId("MMDAgent"), "CURRENT_TIME", "%02d|%02d", hour, minute);
}

void MMDAgent::issueTimeMessage(double ellapsedFrame)
{

   m_timeMessageFrame += ellapsedFrame;
   if (m_timeMessageFrame < MMDAGENT_CURRENTTIME_INTERVAL_FRAME)
      return;
   while (m_timeMessageFrame >= MMDAGENT_CURRENTTIME_INTERVAL_FRAME) {
      m_timeMessageFrame -= MMDAGENT_CURRENTTIME_INTERVAL_FRAME;
   }

   glfwCreateThread(issueCurrentTimeMain, (void *)this);
}
#endif /* MMDAGENT_CURRENTTIME */

/* get history file path */
static char *getHistoryFilePathDup()
{
   char buff[MMDAGENT_MAXBUFLEN];
   char *contentDirName;

   /* get content dir */
   contentDirName = MMDAgent_contentDirMakeDup();
   if (contentDirName == NULL)
      return NULL;

   MMDAgent_snprintf(buff, MMDAGENT_MAXBUFLEN, "%s%c%s", contentDirName, MMDAGENT_DIRSEPARATOR, HISTORYSAVEPATH);

   free(contentDirName);

   return(MMDAgent_strdup(buff));
}

/* save content mdf history */
static void saveContentHistory(const char *mdfpath, const char *contentdir, const char *sysmdfpath)
{
   char *historyFile;
   char buff[MMDAGENT_MAXBUFLEN];
   KeyValue *v, *vnew;
   const char *p1, *p2, *p3;
   int i, k;

   if (MMDAgent_strequal(mdfpath, sysmdfpath))
      return;

   historyFile = getHistoryFilePathDup();
   if (historyFile == NULL)
      return;

   /* load db */
   v = new KeyValue;
   v->setup();
   v->load(historyFile, NULL);

   /* prepare new db */
   vnew = new KeyValue;
   vnew->setup();

   /* make new mapping  */
   vnew->setString("mdf0", "%s", mdfpath);
   vnew->setString("dir0", "%s", contentdir);
   MMDAgent_gettimestampstr(buff, MMDAGENT_MAXBUFLEN, "%4d/%02d/%02d %02d:%02d:%02d.%03d");
   buff[16] = '\0';
   vnew->setString("time0", "%s", buff);
   k = 1;
   for (i = 0; i < HISTORYMAXNUM; i++) {
      MMDAgent_snprintf(buff, MMDAGENT_MAXBUFLEN, "mdf%d", i);
      p1 = v->getString(buff, NULL);
      MMDAgent_snprintf(buff, MMDAGENT_MAXBUFLEN, "dir%d", i);
      p2 = v->getString(buff, NULL);
      MMDAgent_snprintf(buff, MMDAGENT_MAXBUFLEN, "time%d", i);
      p3 = v->getString(buff, NULL);
      if (p1 == NULL || p2 == NULL) continue;
      if (MMDAgent_strequal(p1, sysmdfpath)) continue;
      if (MMDAgent_strequal(p1, mdfpath) == false || MMDAgent_strequal(p2, contentdir) == false) {
         MMDAgent_snprintf(buff, MMDAGENT_MAXBUFLEN, "mdf%d", k);
         vnew->setString(buff, "%s", p1);
         MMDAgent_snprintf(buff, MMDAGENT_MAXBUFLEN, "dir%d", k);
         vnew->setString(buff, "%s", p2);
         if (p3) {
            MMDAgent_snprintf(buff, MMDAGENT_MAXBUFLEN, "time%d", k);
            vnew->setString(buff, "%s", p3);
         }
         k++;
      }
   }

   /* save the new history */
   vnew->save(historyFile);

   free(historyFile);
   delete vnew;
   delete v;
}

/* clear content mdf history */
static void clearContentHistory()
{
   char buff[MMDAGENT_MAXBUFLEN];
   char *contentDirName;

   contentDirName = MMDAgent_contentdirdup();
   if (contentDirName == NULL)
      return;
   MMDAgent_snprintf(buff, MMDAGENT_MAXBUFLEN, "%s%c%s", contentDirName, MMDAGENT_DIRSEPARATOR, HISTORYSAVEPATH);
   MMDAgent_removefile(buff);
   free(contentDirName);
}

/* get last played data file path */
static char *getLastPlayedDataPathDup()
{
   char buff[MMDAGENT_MAXBUFLEN];
   char *contentDirName;

   /* get content dir */
   contentDirName = MMDAgent_contentDirMakeDup();
   if (contentDirName == NULL)
      return NULL;

   MMDAgent_snprintf(buff, MMDAGENT_MAXBUFLEN, "%s%c%s", contentDirName, MMDAGENT_DIRSEPARATOR, LASTPLAYEDDATAPATH);

   free(contentDirName);

   return(MMDAgent_strdup(buff));
}

/* save last played time of a mdf file to database */
static void saveContentLastPlayed(const char *mdfpath)
{
   char *lastPlayedFile;
   char buff[MMDAGENT_MAXBUFLEN];
   KeyValue *v;

   if (mdfpath == NULL)
      return;

   lastPlayedFile = getLastPlayedDataPathDup();
   if (lastPlayedFile == NULL)
      return;

   /* set up new keyvalue */
   v = new KeyValue;
   v->setup();

   /* set time */
   MMDAgent_gettimestampstr(buff, MMDAGENT_MAXBUFLEN, "%4d/%02d/%02d %02d:%02d:%02d.%03d");
   buff[16] = '\0';
   v->setString(mdfpath, "%s", buff);

   /* append existing db (if overflowed, last one will be omitted) */
   v->loadText(lastPlayedFile, NULL, true);

   /* save */
   v->saveText(lastPlayedFile);

   free(lastPlayedFile);
   delete v;
}

/* clear content mdf last played data filey */
static void clearContentLastPlayed()
{
   char buff[MMDAGENT_MAXBUFLEN];
   char *contentDirName;

   contentDirName = MMDAgent_contentdirdup();
   if (contentDirName == NULL)
      return;
   MMDAgent_snprintf(buff, MMDAGENT_MAXBUFLEN, "%s%c%s", contentDirName, MMDAGENT_DIRSEPARATOR, LASTPLAYEDDATAPATH);
   MMDAgent_removefile(buff);
   free(contentDirName);
}

/* return allocated cache dir path */
static char *prepareCacheDirDup(char *basename)
{
   char buff[MMDAGENT_MAXBUFLEN];
   char *contentDirName;
   char *cacheDirName;

   /* get content directory name */
   contentDirName = MMDAgent_contentDirMakeDup();
   if (contentDirName == NULL)
      return NULL;

   MMDAgent_snprintf(buff, MMDAGENT_MAXBUFLEN, "%s%c%s", contentDirName, MMDAGENT_DIRSEPARATOR, basename);
   cacheDirName = MMDAgent_strdup(buff);

   if (MMDAgent_stat(cacheDirName) != MMDAGENT_STAT_DIRECTORY) {
      if (MMDAgent_mkdir(cacheDirName) == false) {
         free(contentDirName);
         free(cacheDirName);
         return NULL;
      }
   }

   free(contentDirName);
   return(cacheDirName);
}

static bool stdInputThreadRunning = false;
static MMDAgent *mmdagent_for_stdin = NULL;

static void assignStdInputInstance(MMDAgent *mmdagent)
{
   mmdagent_for_stdin = mmdagent;
}

/* thread main function receiving standard input and post them to message queue */
static void stdInputReceivingThreadMain(void *param)
{
   char buff[MMDAGENT_MAXBUFLEN];
   char *save, *type, *args;

   stdInputThreadRunning = true;

   mmdagent_for_stdin = (MMDAgent *)param;

   mmdagent_for_stdin->sendLogString(0, MLOG_STATUS, "stdin thread started");

   while (1) {
      if (fgets(buff, MMDAGENT_MAXBUFLEN, stdin) == NULL) {
         // EOF
         break;
      }
      if (MMDAgent_strlen(buff) > 0) {
         type = MMDAgent_strtok(buff, "|\r\n", &save);
         if (type) {
            args = MMDAgent_strtok(NULL, "\r\n", &save);
            mmdagent_for_stdin->sendMessage(0, type, "%s", args ? args : "");
         }
      }
   }

   stdInputThreadRunning = false;

   mmdagent_for_stdin->sendLogString(0, MLOG_WARNING, "stdin thread exited");
}

/* MMDAgent::getNewModelId: return new model ID */
int MMDAgent::getNewModelId()
{
   int i;

   for (i = 0; i < m_numModel; i++)
      if (m_model[i].isEnable() == false)
         return i; /* re-use it */

   if (m_numModel >= m_option->getMaxNumModel())
      return -1; /* no more room */

   i = m_numModel;
   m_numModel++;

   m_model[i].setEnableFlag(false); /* model is not loaded yet */
   return i;
}

/* MMDAgent::removeRelatedModels: delete a model */
void MMDAgent::removeRelatedModels(int modelId)
{
   int i;
   MotionPlayer *motionPlayer;

   /* remove assigned accessories */
   for (i = 0; i < m_numModel; i++)
      if (m_model[i].isEnable() == true && m_model[i].getAssignedModel() == &(m_model[modelId]))
         removeRelatedModels(i);

   /* remove motion */
   for (motionPlayer = m_model[modelId].getMotionManager()->getMotionPlayerList(); motionPlayer; motionPlayer = motionPlayer->next) {
      /* send message */
      if (MMDAgent_strequal(motionPlayer->name, LIPSYNC_MOTIONNAME))
         sendMessage(m_moduleId, MMDAGENT_EVENT_LIPSYNCSTOP, "%s", m_model[modelId].getAlias());
      else
         sendMessage(m_moduleId, MMDAGENT_EVENT_MOTIONDELETE, "%s|%s", m_model[modelId].getAlias(), motionPlayer->name);
      /* unload from motion stocker */
      m_motion->unload(motionPlayer->vmd);
   }

   /* issue delete messages defined in a file "${modelFileName}.deletemessage", if it exists */
   if (m_model[modelId].getDeleteMessagesNum() > 0) {
      char buf[MMDAGENT_MAXBUFLEN];
      char *p, *q, *psave;
      const char **list = m_model[modelId].getDeleteMessages();
      for (int i = 0; i < m_model[modelId].getDeleteMessagesNum(); i++) {
         strncpy(buf, list[i], MMDAGENT_MAXBUFLEN);
         p = MMDAgent_strtok(buf, "|", &psave);
         q = MMDAgent_strtok(NULL, "\r\n", &psave);
         sendMessage(m_moduleId, p, "%s", q);
      }
   }

   /* remove model */
   sendMessage(m_moduleId, MMDAGENT_EVENT_MODELDELETE, "%s", m_model[modelId].getAlias());
   m_model[modelId].release();
}

/* MMDAgent::updateLight: update light */
void MMDAgent::updateLight()
{
   int i;
   float *f;
   btVector3 l;

   /* udpate OpenGL light */
   m_render->updateLight(m_option->getUseMMDLikeCartoon(), m_option->getUseCartoonRendering(), m_option->getLightIntensity(), m_option->getLightDirection(), m_option->getLightColor());
   /* update shadow matrix */
   f = m_option->getLightDirection();
   m_stage->updateShadowMatrix(f);
   /* update vector for cartoon */
   l = btVector3(btScalar(f[0]), btScalar(f[1]), btScalar(f[2]));
   for (i = 0; i < m_numModel; i++)
      if (m_model[i].isEnable() == true)
         m_model[i].setLightForToon(&l);
}

/* MMDAgent::setHighLight: set high-light of selected model */
void MMDAgent::setHighLight(int modelId)
{
   float color[4];

   if (m_highLightingModel == modelId)
      return;

   if (m_highLightingModel != -1 && m_model[m_highLightingModel].isEnable() == true) {
      /* reset current highlighted model */
      color[0] = PMDMODEL_EDGECOLORR;
      color[1] = PMDMODEL_EDGECOLORG;
      color[2] = PMDMODEL_EDGECOLORB;
      color[3] = PMDMODEL_EDGECOLORA;
      m_model[m_highLightingModel].getPMDModel()->setEdgeColor(color);
      /* disable force edge flag */
      m_model[m_highLightingModel].getPMDModel()->setForceEdgeFlag(false);
   }
   if (modelId != -1 && m_model[modelId].isEnable() == true) {
      /* set highlight to the specified model */
      m_model[modelId].getPMDModel()->setEdgeColor(m_option->getCartoonEdgeSelectedColor());
      /* enable force edge flag */
      m_model[modelId].getPMDModel()->setForceEdgeFlag(true);
   }

   m_highLightingModel = modelId;
}

/* MMDAgent::addModel: add model */
bool MMDAgent::addModel(const char *modelAlias, const char *fileName, btVector3 *pos, btQuaternion *rot, bool useCartoonRendering, const char *baseModelAlias, const char *baseBoneName)
{
   int i;
   int id;
   int baseID;
   char *name;
   btVector3 offsetPos(btScalar(0.0f), btScalar(0.0f), btScalar(0.0f));
   btQuaternion offsetRot(btScalar(0.0f), btScalar(0.0f), btScalar(0.0f), btScalar(1.0f));
   bool forcedPosition = false;
   PMDBone *assignBone = NULL;
   PMDObject *assignObject = NULL;
   float *l = m_option->getLightDirection();
   btVector3 light = btVector3(btScalar(l[0]), btScalar(l[1]), btScalar(l[2]));

   /* set */
   if (pos)
      offsetPos = (*pos);
   if (rot)
      offsetRot = (*rot);
   if (pos || rot)
      forcedPosition = true;
   if (baseModelAlias) {
      baseID = findModelAlias(baseModelAlias);
      if (baseID < 0) {
         sendLogString(m_moduleId, MLOG_ERROR, "addModel: %s is not found.", baseModelAlias);
         return false;
      }
      if (baseBoneName) {
         assignBone = m_model[baseID].getPMDModel()->getBone(baseBoneName);
      } else {
         assignBone = m_model[baseID].getPMDModel()->getCenterBone();
      }
      if (assignBone == NULL) {
         if (baseBoneName)
            sendLogString(m_moduleId, MLOG_ERROR, "addModel: %s does not exist on %s.", baseBoneName, baseModelAlias);
         else
            sendLogString(m_moduleId, MLOG_ERROR, "addModel: %s don't have center bone.", baseModelAlias);
         return false;
      }
      assignObject = &m_model[baseID];
   }

   /* ID */
   id = getNewModelId();
   if (id == -1) {
      sendLogString(m_moduleId, MLOG_ERROR, "addModel: number of models exceed the limit.");
      return false;
   }

   /* determine name */
   if (MMDAgent_strlen(modelAlias) > 0) {
      /* check the same alias */
      name = MMDAgent_strdup(modelAlias);
      if (findModelAlias(name) >= 0) {
         sendLogString(m_moduleId, MLOG_WARNING, "addModel: model alias \"%s\" is already used.", name);
         free(name);
         return false;
      }
   } else {
      /* if model alias is not specified, unused digit is used */
      for(i = 0;; i++) {
         name = MMDAgent_intdup(i);
         if (findModelAlias(name) >= 0)
            free(name);
         else
            break;
      }
   }

   /* add model */
   if (MMDAgent_exist(fileName) == false) {
      sendLogString(m_moduleId, MLOG_ERROR, "addModel: not found: %s", fileName);
      m_model[id].release();
      free(name);
      return false;
   }
   if (!m_model[id].load(fileName, name, &offsetPos, &offsetRot, forcedPosition, assignBone, assignObject, m_bullet, m_systex, m_lipSync, useCartoonRendering, m_option->getCartoonEdgeWidth(), m_option->getLightEdge(), &light, m_option->getDisplayCommentTime() * 30.0f, NULL, m_appDirName)) {
      sendLogString(m_moduleId, MLOG_ERROR, "addModel: %s cannot be loaded.", fileName);
      m_model[id].release();
      free(name);
      return false;
   }
   if (m_model[id].getShapeMap() && m_model[id].isShapeMapDefault())
      sendLogString(m_moduleId, MLOG_WARNING, "addModel: \"%s\": no model-specific shapemap exist, use system default shapemap", fileName);

   /* initialize motion manager */
   m_model[id].resetMotionManager();

   /* update for initial positions and skins */
   m_model[id].updateRootBone();
   m_model[id].updateMotion(0.0);
   m_model[id].updateSkin();

   /* send message */
   sendMessage(m_moduleId, MMDAGENT_EVENT_MODELADD, "%s", name);

   /* issue load messages defined in a file "${modelFileName}.loadmessage", if it exists */
   if (m_model[id].getLoadMessagesNum() > 0) {
      char buf[MMDAGENT_MAXBUFLEN];
      char *p, *q, *psave;
      const char **list = m_model[id].getLoadMessages();
      for (int i = 0; i < m_model[id].getLoadMessagesNum(); i++) {
         strncpy(buf, list[i], MMDAGENT_MAXBUFLEN);
         p = MMDAgent_strtok(buf, "|", &psave);
         q = MMDAgent_strtok(NULL, "\r\n", &psave);
         sendMessage(m_moduleId, p, "%s", q);
      }
   }

   free(name);
   return true;
}

/* MMDAgent::changeModel: change model */
bool MMDAgent::changeModel(const char *modelAlias, const char *fileName, PMDModel *pmd)
{
   int i;
   int id;
   float *l = m_option->getLightDirection();
   btVector3 light = btVector3(btScalar(l[0]), btScalar(l[1]), btScalar(l[2]));
   char buf[MMDAGENT_MAXBUFLEN];
   const char *boneName = NULL;

   /* ID */
   id = findModelAlias(modelAlias);
   if (id < 0) {
      sendLogString(m_moduleId, MLOG_WARNING, "changeModel: %s is not found.", modelAlias);
      return false;
   }

   /* issue delete messages defined in a file "${modelFileName}.deletemessage", if it exists */
   if (m_model[id].getDeleteMessagesNum() > 0) {
      char *p, *q, *psave;
      const char **list = m_model[id].getDeleteMessages();
      for (int i = 0; i < m_model[id].getDeleteMessagesNum(); i++) {
         strncpy(buf, list[i], MMDAGENT_MAXBUFLEN);
         p = MMDAgent_strtok(buf, "|", &psave);
         q = MMDAgent_strtok(NULL, "\r\n", &psave);
         sendMessage(m_moduleId, p, "%s", q);
      }
   }

   /* when on-model camera is running, store the bone name */
   boneName = m_render->getCameraBoneName();

   /* load model */
   if (MMDAgent_exist(fileName) == false) {
      sendLogString(m_moduleId, MLOG_ERROR, "changeModel: not found: %s", fileName);
      return false;
   }
   if (!m_model[id].load(fileName, modelAlias, NULL, NULL, false, NULL, NULL, m_bullet, m_systex, m_lipSync, m_model[id].useCartoonRendering(), m_option->getCartoonEdgeWidth(), m_option->getLightEdge(), &light, m_option->getDisplayCommentTime() * 30.0f, pmd, m_appDirName)) {
      sendLogString(m_moduleId, MLOG_ERROR, "changeModel: %s cannot be loaded.", fileName);
      return false;
   }
   if (m_model[id].getShapeMap() && m_model[id].isShapeMapDefault())
      sendLogString(m_moduleId, MLOG_WARNING, "changeModel: \"%s\": no model-specific shapemap exist, use system default shapemap", fileName);

   /* update motion manager */
   if (m_model[id].getMotionManager())
      m_model[id].getMotionManager()->updateModel(m_model[id].getPMDModel());

   /* update for initial positions and skins */
   m_model[id].updateRootBone();
   m_model[id].updateMotion(0.0);
   m_model[id].updateSkin();

   /* delete accessories immediately */
   for (i = 0; i < m_numModel; i++)
      if (m_model[i].isEnable() && m_model[i].getAssignedModel() == &(m_model[id]))
         removeRelatedModels(i);

   /* send message */
   sendMessage(m_moduleId, MMDAGENT_EVENT_MODELCHANGE, "%s", modelAlias);

   /* issue load messages defined in a file "${modelFileName}.loadmessage", if it exists */
   if (m_model[id].getLoadMessagesNum() > 0) {
      char *p, *q, *psave;
      const char **list = m_model[id].getLoadMessages();
      for (int i = 0; i < m_model[id].getLoadMessagesNum(); i++) {
         strncpy(buf, list[i], MMDAGENT_MAXBUFLEN);
         p = MMDAgent_strtok(buf, "|", &psave);
         q = MMDAgent_strtok(NULL, "\r\n", &psave);
         sendMessage(m_moduleId, p, "%s", q);
      }
   }

   /* when on-model camera is running, set the bone name, or stop if not exist */
   if (boneName) {
      PMDBone *bone = m_model[id].getPMDModel()->getBone(boneName);
      if (bone == NULL)
         bone = m_model[id].getPMDModel()->getCenterBone();
      m_render->updateCameraBone(bone);
   }

   return true;
}

/* MMDAgent::deleteModel: delete model */
bool MMDAgent::deleteModel(const char *modelAlias)
{
   int i;
   int id;

   /* ID */
   id = findModelAlias(modelAlias);
   if (id < 0) {
      /* wrong alias */
      sendLogString(m_moduleId, MLOG_WARNING, "deleteModel: %s is not found.", modelAlias);
      return false;
   }

   /* delete accessories  */
   for (i = 0; i < m_numModel; i++)
      if (m_model[i].isEnable() && m_model[i].getAssignedModel() == &(m_model[id]))
         deleteModel(m_model[i].getAlias());

   /* set frame from now to disappear */
   m_model[id].startDisappear();

   /* don't send message yet */
   return true;
}

/* MMDAgent::addMotion: add motion */
bool MMDAgent::addMotion(const char *modelAlias, const char *motionAlias, const char *fileName, bool full, bool once, bool enableSmooth, bool enableRePos, float priority)
{
   int i;
   int id;
   VMD *vmd;
   char *name;
   MotionPlayer *motionPlayer;

   /* motion file */
   if (MMDAgent_exist(fileName) == false) {
      sendLogString(m_moduleId, MLOG_ERROR, "addMotion: not found: %s", fileName);
      return false;
   }
   vmd = m_motion->loadFromFile(fileName);
   if (vmd == NULL) {
      sendLogString(m_moduleId, MLOG_ERROR, "addMotion: %s cannot be loaded.", fileName);
      return false;
   }

   /* ID */
   id = findModelAlias(modelAlias);
   if (id < 0) {
      sendLogString(m_moduleId, MLOG_WARNING, "addMotion: %s is not found, unable to start motion %s", modelAlias, motionAlias);
      return false;
   }

   /* alias */
   if (MMDAgent_strlen(motionAlias) > 0) {
      /* check the same alias */
      name = MMDAgent_strdup(motionAlias);
      motionPlayer = m_model[id].getMotionManager()->getRunning(name);
      if (motionPlayer != NULL) {
         sendLogString(m_moduleId, MLOG_WARNING, "addMotion: motion alias \"%s\" already exist, swap it.", name);
         VMD *old = motionPlayer->vmd;
         /* when smoothing has been disabled, skip next physics simulation for warping at beginning of changed motion */
         if (motionPlayer->enableSmooth == false)
            m_model[id].skipNextSimulation();
         /* change motion */
         if (m_model[id].swapMotion(vmd, motionAlias) == false) {
            sendLogString(m_moduleId, MLOG_WARNING, "addMotion: %s: failed to change existing motion.", motionAlias);
            m_motion->unload(vmd);
            return false;
         }
         m_motion->unload(old);
         sendMessage(m_moduleId, MMDAGENT_EVENT_MOTIONADD, "%s|%s", modelAlias, name);
         free(name);
         return true;
      }
   } else {
      /* if motion alias is not specified, unused digit is used */
      for(i = 0;; i++) {
         name = MMDAgent_intdup(i);
         if (m_model[id].getMotionManager()->getRunning(name) == NULL)
            break;
         free(name);
      }
   }

   /* when smoothing is disabled, skip next physics simulation for warping at beginning of motion */
   if (enableSmooth == false)
      m_model[id].skipNextSimulation();

   /* start motion */
   if (m_model[id].startMotion(vmd, name, full, once, enableSmooth, enableRePos, priority) == false) {
      free(name);
      return false;
   }

   sendMessage(m_moduleId, MMDAGENT_EVENT_MOTIONADD, "%s|%s", modelAlias, name);
   free(name);
   return true;
}

/* MMDAgent::changeMotion: change motion */
bool MMDAgent::changeMotion(const char *modelAlias, const char *motionAlias, const char *fileName)
{
   int id;
   VMD *vmd, *old = NULL;
   MotionPlayer *motionPlayer;

   /* ID */
   id = findModelAlias(modelAlias);
   if (id < 0) {
      sendLogString(m_moduleId, MLOG_WARNING, "changeMotion: %s is not found.", modelAlias);
      return false;
   }

   /* check */
   if (!motionAlias) {
      sendLogString(m_moduleId, MLOG_ERROR, "changeMotion: motion alias is not specified.");
      return false;
   }

   /* motion file */
   if (MMDAgent_exist(fileName) == false) {
      sendLogString(m_moduleId, MLOG_ERROR, "changeMotion: not found: %s", fileName);
      return false;
   }
   vmd = m_motion->loadFromFile(fileName);
   if (vmd == NULL) {
      sendLogString(m_moduleId, MLOG_ERROR, "changeMotion: %s cannot be loaded.", fileName);
      return false;
   }

   /* get motion before change */
   motionPlayer = m_model[id].getMotionManager()->getRunning(motionAlias);
   if (motionPlayer == NULL) {
      sendLogString(m_moduleId, MLOG_WARNING, "changeMotion: %s is not found.", motionAlias);
      m_motion->unload(vmd);
      return false;
   }
   old = motionPlayer->vmd;
   /* when smoothing has been disabled, skip next physics simulation for warping at beginning of changed motion */
   if (motionPlayer->enableSmooth == false)
      m_model[id].skipNextSimulation();

   /* change motion */
   if (m_model[id].swapMotion(vmd, motionAlias) == false) {
      sendLogString(m_moduleId, MLOG_WARNING, "changeMotion: %s: failed to change motion", motionAlias);
      m_motion->unload(vmd);
      return false;
   }

   /* unload old motion from motion stocker */
   m_motion->unload(old);

   /* send message */
   sendMessage(m_moduleId, MMDAGENT_EVENT_MOTIONCHANGE, "%s|%s", modelAlias, motionAlias);
   return true;
}

/* MMDAgent::resetMotion: reset motion */
bool MMDAgent::resetMotion(const char *modelAlias, const char *motionAlias)
{
   int id;
   MotionPlayer *motionPlayer;

   /* ID */
   id = findModelAlias(modelAlias);
   if (id < 0) {
      sendLogString(m_moduleId, MLOG_WARNING, "resetMotion: %s is not found.", modelAlias);
      return false;
   }

   /* check */
   if (!motionAlias) {
      sendLogString(m_moduleId, MLOG_ERROR, "resetMotion: motion alias is not specified.");
      return false;
   }

   /* get current motion */
   motionPlayer = m_model[id].getMotionManager()->getRunning(motionAlias);
   if (motionPlayer == NULL) {
      sendLogString(m_moduleId, MLOG_WARNING, "resetMotion: %s is not found.", motionAlias);
      return false;
   }

   /* when smoothing has been disabled, skip next physics simulation for warping at beginning of reset motion */
   if (motionPlayer->enableSmooth == false)
      m_model[id].skipNextSimulation();

   /* reset motion */
   if (m_model[id].swapMotion(motionPlayer->vmd, motionAlias) == false) {
      sendLogString(m_moduleId, MLOG_WARNING, "resetMotion: failed to reset %s", motionAlias);
      return false;
   }

   /* send message */
   sendMessage(m_moduleId, MMDAGENT_EVENT_MOTIONRESET, "%s|%s", modelAlias, motionAlias);
   return true;
}

/* MMDAgent::accelerateMotion: accelerate motion */
bool MMDAgent::accelerateMotion(const char *modelAlias, const char *motionAlias, float speed, float durationTime, float targetTime)
{
   int id;

   /* ID */
   id = findModelAlias(modelAlias);
   if (id < 0) {
      sendLogString(m_moduleId, MLOG_WARNING, "accelerateMotion: %s is not found.", modelAlias);
      return false;
   }

   /* check */
   if (!motionAlias) {
      sendLogString(m_moduleId, MLOG_ERROR, "accelerateMotion: motion alias is not specified.");
      return false;
   }

   /* change motion speed */
   if (m_model[id].getMotionManager()->setMotionSpeedRate(motionAlias, speed, durationTime * 30.0f, targetTime * 30.0f) == false) {
      sendLogString(m_moduleId, MLOG_WARNING, "accelerateMotion: %s is not found.", motionAlias);
      return false;
   }

   /* don't send message yet */
   return true;
}

/* MMDAgent::deleteMotion: delete motion */
bool MMDAgent::deleteMotion(const char *modelAlias, const char *motionAlias)
{
   int id;

   /* ID */
   id = findModelAlias(modelAlias);
   if (id < 0) {
      sendLogString(m_moduleId, MLOG_WARNING, "deleteMotion: %s is not found.", modelAlias);
      return false;
   }

   /* delete motion */
   if (m_model[id].getMotionManager()->deleteMotion(motionAlias) == false) {
      sendLogString(m_moduleId, MLOG_WARNING, "deleteMotion: %s is not found.", motionAlias);
      return false;
   }

   /* don't send message yet */
   return true;
}

/* MMDAgent::configureMotion: configure motion */
bool MMDAgent::configureMotion(const char *modelAlias, const char *motionAlias, const char *key, const char *value)
{
   int id;

   /* ID */
   id = findModelAlias(modelAlias);
   if (id < 0) {
      sendLogString(m_moduleId, MLOG_WARNING, "configureMotion: %s is not found.", modelAlias);
      return false;
   }

   /* configure motion */
   if (m_model[id].getMotionManager()->configureMotion(motionAlias, key, value) == false) {
      if (value) {
         sendLogString(m_moduleId, MLOG_ERROR, "configureMotion: failed to set %s|%s to %s.", key, value, motionAlias);
      } else {
         sendLogString(m_moduleId, MLOG_ERROR, "configureMotion: failed to set %s to %s.", key, motionAlias);
      }
      return false;
   }

   sendMessage(m_moduleId, MMDAGENT_EVENT_MOTIONCONFIGURE, "%s|%s", modelAlias, motionAlias);

   return true;
}

/* MMDAgent::startMove: start moving */
bool MMDAgent::startMove(const char *modelAlias, btVector3 *pos, bool local, float speed)
{
   int id;
   btVector3 currentPos;
   btQuaternion currentRot;
   btVector3 targetPos;
   btTransform tr;

   /* ID */
   id = findModelAlias(modelAlias);
   if (id < 0) {
      sendLogString(m_moduleId, MLOG_WARNING, "startMove: %s is not found.", modelAlias);
      return false;
   }

   if(m_model[id].isMoving() == true)
      sendMessage(m_moduleId, MMDAGENT_EVENT_MOVESTOP, "%s", modelAlias);

   /* get */
   m_model[id].getCurrentPosition(&currentPos);
   targetPos = (*pos);

   /* local or global */
   if (local) {
      m_model[id].getCurrentRotation(&currentRot);
      tr = btTransform(currentRot, currentPos);
      targetPos = tr * targetPos;
   }

   /* not need to start */
   if (currentPos == targetPos) {
      sendMessage(m_moduleId, MMDAGENT_EVENT_MOVESTART, "%s", modelAlias);
      sendMessage(m_moduleId, MMDAGENT_EVENT_MOVESTOP, "%s", modelAlias);
      return true;
   }

   m_model[id].setMoveSpeed(speed);
   m_model[id].setPosition(&targetPos);
   sendMessage(m_moduleId, MMDAGENT_EVENT_MOVESTART, "%s", modelAlias);
   return true;
}

/* MMDAgent::stopMove: stop moving */
bool MMDAgent::stopMove(const char *modelAlias)
{
   int id;
   btVector3 currentPos;

   /* ID */
   id = findModelAlias(modelAlias);
   if (id < 0) {
      sendLogString(m_moduleId, MLOG_WARNING, "stopMove: %s is not found.", modelAlias);
      return false;
   }

   if(m_model[id].isMoving() == false) {
      sendLogString(m_moduleId, MLOG_WARNING, "stopMove: %s is not moving.", modelAlias);
      return false;
   }

   /* get */
   m_model[id].getCurrentPosition(&currentPos);

   m_model[id].setPosition(&currentPos);
   sendMessage(m_moduleId, MMDAGENT_EVENT_MOVESTOP, "%s", modelAlias);
   return true;
}

/* MMDAgent::startTurn: start turn */
bool MMDAgent::startTurn(const char *modelAlias, btVector3 *pos, bool local, float speed)
{
   int id;
   btVector3 currentPos;
   btQuaternion currentRot;
   btVector3 targetPos;
   btQuaternion targetRot;

   float z, rad;
   btVector3 axis;

   /* ID */
   id = findModelAlias(modelAlias);
   if (id < 0) {
      sendLogString(m_moduleId, MLOG_WARNING, "startTurn: %s is not found.", modelAlias);
      return false;
   }

   if(m_model[id].isRotating() == true) {
      if(m_model[id].isTurning() == true)
         sendMessage(m_moduleId, MMDAGENT_EVENT_TURNSTOP, "%s", modelAlias);
      else
         sendMessage(m_moduleId, MMDAGENT_EVENT_ROTATESTOP, "%s", modelAlias);
   }

   /* get */
   m_model[id].getCurrentPosition(&currentPos);
   m_model[id].getCurrentRotation(&currentRot);

   /* get vector from current position to target position */
   if(local == true)
      targetPos = (*pos);
   else
      targetPos = (*pos) - currentPos;
   if (targetPos.fuzzyZero())
      return false;
   targetPos.normalize();

   /* calculate target rotation from (0,0,1) */
   z = targetPos.z();
   if (z > 1.0f) z = 1.0f;
   if (z < -1.0f) z = -1.0f;
   rad = acosf(z);
   axis = btVector3(btScalar(0.0f), btScalar(0.0f), btScalar(1.0f)).cross(targetPos);
   if(axis.length2() < PMDOBJECT_MINSPINDIFF) {
      targetRot = btQuaternion(btScalar(0.0f), btScalar(0.0f), btScalar(0.0f), btScalar(1.0f));
   } else {
      axis.normalize();
      targetRot = btQuaternion(axis, btScalar(rad));
   }

   /* local or global */
   if (local)
      targetRot = currentRot * targetRot;
   else
      targetRot = currentRot.nearest(targetRot);

   /* not need to turn */
   if (currentRot == targetRot) {
      sendMessage(m_moduleId, MMDAGENT_EVENT_TURNSTART, "%s", modelAlias);
      sendMessage(m_moduleId, MMDAGENT_EVENT_TURNSTOP, "%s", modelAlias);
      return true;
   }

   m_model[id].setSpinSpeed(speed);
   m_model[id].setRotation(&targetRot);
   m_model[id].setTurningFlag(true);
   sendMessage(m_moduleId, MMDAGENT_EVENT_TURNSTART, "%s", modelAlias);
   return true;
}

/* MMDAgent::stopTurn: stop turn */
bool MMDAgent::stopTurn(const char *modelAlias)
{
   int id;
   btQuaternion currentRot;

   id = findModelAlias(modelAlias);
   if (id < 0) {
      sendLogString(m_moduleId, MLOG_WARNING, "stopTurn: %s is not found.", modelAlias);
      return false;
   }

   /* not need to stop turn */
   if (m_model[id].isRotating() == false || m_model[id].isTurning() == false) {
      sendLogString(m_moduleId, MLOG_WARNING, "stopTurn: %s is not turning.", modelAlias);
      return false;
   }

   /* get */
   m_model[id].getCurrentRotation(&currentRot);

   m_model[id].setRotation(&currentRot);
   sendMessage(m_moduleId, MMDAGENT_EVENT_TURNSTOP, "%s", modelAlias);
   return true;
}

/* MMDAgent::startRotation: start rotation */
bool MMDAgent::startRotation(const char *modelAlias, btQuaternion *rot, bool local, float speed)
{
   int id;
   btQuaternion targetRot;
   btQuaternion currentRot;

   id = findModelAlias(modelAlias);
   if (id < 0) {
      sendLogString(m_moduleId, MLOG_WARNING, "startRotation: %s is not found.", modelAlias);
      return false;
   }

   if(m_model[id].isRotating() == true) {
      if(m_model[id].isTurning() == true)
         sendMessage(m_moduleId, MMDAGENT_EVENT_TURNSTOP, "%s", modelAlias);
      else
         sendMessage(m_moduleId, MMDAGENT_EVENT_ROTATESTOP, "%s", modelAlias);
   }

   /* get */
   m_model[id].getCurrentRotation(&currentRot);
   targetRot = (*rot);

   /* local or global */
   if (local)
      targetRot = currentRot * targetRot;
   else
      targetRot = currentRot.nearest(targetRot);

   /* not need to start */
   if (currentRot == targetRot) {
      sendMessage(m_moduleId, MMDAGENT_EVENT_ROTATESTART, "%s", modelAlias);
      sendMessage(m_moduleId, MMDAGENT_EVENT_ROTATESTOP, "%s", modelAlias);
      return true;
   }

   m_model[id].setSpinSpeed(speed);
   m_model[id].setRotation(&targetRot);
   m_model[id].setTurningFlag(false);
   sendMessage(m_moduleId, MMDAGENT_EVENT_ROTATESTART, "%s", modelAlias);
   return true;
}

/* MMDAgent::stopRotation: stop rotation */
bool MMDAgent::stopRotation(const char *modelAlias)
{
   int id;
   btQuaternion currentRot;

   id = findModelAlias(modelAlias);
   if (id < 0) {
      sendLogString(m_moduleId, MLOG_WARNING, "stopRotation: %s is not found.", modelAlias);
      return false;
   }

   /* not need to stop rotation */
   if (m_model[id].isRotating() == false || m_model[id].isTurning() == true) {
      sendLogString(m_moduleId, MLOG_WARNING, "stopRotation: %s is not rotating.", modelAlias);
      return false;
   }

   /* get */
   m_model[id].getCurrentRotation(&currentRot);

   m_model[id].setRotation(&currentRot);
   sendMessage(m_moduleId, MMDAGENT_EVENT_ROTATESTOP, "%s", modelAlias);
   return true;
}

/* MMDAgent::setFloor: set floor image */
bool MMDAgent::setFloor(const char *fileName)
{
   /* load floor */
   if (MMDAgent_exist(fileName) == false) {
      sendLogString(m_moduleId, MLOG_ERROR, "setFloor: not found: %s", fileName);
      return false;
   }
   if (m_stage->loadFloor(fileName) == false) {
      sendLogString(m_moduleId, MLOG_ERROR, "setFloor: %s cannot be set for floor.", fileName);
      return false;
   }

   /* don't send message */
   return true;
}

/* MMDAgent::setBackground: set background image */
bool MMDAgent::setBackground(const char *fileName)
{
   /* load background */
   if (MMDAgent_exist(fileName) == false) {
      sendLogString(m_moduleId, MLOG_ERROR, "setBackground: not found: %s", fileName);
      return false;
   }
   if (m_stage->loadBackground(fileName) == false) {
      sendLogString(m_moduleId, MLOG_ERROR, "setBackground: %s cannot be set for background.", fileName);
      return false;
   }

   /* don't send message */
   return true;
}

/* MMDAgent::setStage: set stage */
bool MMDAgent::setStage(const char *fileName)
{
   if (MMDAgent_exist(fileName) == false) {
      sendLogString(m_moduleId, MLOG_ERROR, "setStage: not found: %s", fileName);
      return false;
   }
   if (m_stage->loadStagePMD(fileName, m_bullet, m_systex) == false) {
      sendLogString(m_moduleId, MLOG_ERROR, "setStage: %s cannot be set for stage.", fileName);
      return false;
   }

   /* don't send message */
   return true;
}

/* MMDAgent::setWindowFrame: set window frame */
bool MMDAgent::setWindowFrame(const char *fileName)
{
   if (MMDAgent_exist(fileName) == false) {
      sendLogString(m_moduleId, MLOG_ERROR, "setWindowFrame: not found: %s", fileName);
      return false;
   }
   if (MMDAgent_strequal(fileName, "NONE")) {
      /* delete window frame */
      m_stage->deleteAllFrameTexture();
   } else if (m_stage->addFrameTexture("__default", fileName) == false) {
      sendLogString(m_moduleId, MLOG_ERROR, "setWindowFrame: failed to load: %s", fileName);
      return false;
   }

   /* don't send message */
   return true;
}

/* MMDAgent::addWindowFrame: add window frame */
bool MMDAgent::addWindowFrame(const char *frameAlias, const char *fileName)
{
   if (MMDAgent_exist(fileName) == false) {
      sendLogString(m_moduleId, MLOG_ERROR, "addWindowFrame: %s: not found: %s", frameAlias, fileName);
      return false;
   }
   if (m_stage->addFrameTexture(frameAlias, fileName) == false) {
      sendLogString(m_moduleId, MLOG_ERROR, "addWindowFrame: %s: failed to load or exceeds limit: %s", frameAlias, fileName);
      return false;
   }
   sendMessage(m_moduleId, MMDAGENT_EVENT_WINDOWFRAME_ADD, "%s", frameAlias);
   return true;
}

/* MMDAgent::deleteWindowFrame: delete window frame */
bool MMDAgent::deleteWindowFrame(const char *frameAlias)
{
   if (m_stage->deleteFrameTexture(frameAlias) == false) {
      sendLogString(m_moduleId, MLOG_WARNING, "deleteWindowFrame: frame alias not exist: %s", frameAlias);
      return false;
   }
   sendMessage(m_moduleId, MMDAGENT_EVENT_WINDOWFRAME_DELETE, "%s", frameAlias);
   return true;
}

/* MMDAgent::deleteAllWindowFrame: delete all window frame */
bool MMDAgent::deleteAllWindowFrame()
{
   m_stage->deleteAllFrameTexture();
   /* don't send message */
   return true;
}

/* MMDAgent::changeCamera: change camera setting */
bool MMDAgent::changeCamera(const char *posOrVMD, const char *rot, const char *distance, const char *fovy, const char *time, const char *modelAlias, const char *boneName)
{
   float p[3], r[3];
   int modelId;
   PMDBone *bone = NULL;
   VMD *vmd;

   if(MMDAgent_str2fvec(posOrVMD, p, 3) == true && MMDAgent_str2fvec(rot, r, 3) == true) {
      m_cameraControlled = false;
      if (modelAlias) {
         modelId = findModelAlias(modelAlias);
         if (modelId == -1) {
            sendLogString(m_moduleId, MLOG_WARNING, "changeCamera: model alias \"%s\" not found", modelAlias);
            return false;
         }
         if (boneName) {
            bone = m_model[modelId].getPMDModel()->getBone(boneName);
            if (bone == NULL) {
               sendLogString(m_moduleId, MLOG_ERROR, "changeCamera: bone \"%s\" not exist on model alias \"%s\"", boneName, modelAlias);
               return false;
            }
         } else {
            bone = m_model[modelId].getPMDModel()->getCenterBone();
            if (bone == NULL) {
               sendLogString(m_moduleId, MLOG_ERROR, "changeCamera: no center bone on model alias \"%s\"", modelAlias);
               return false;
            }
         }
      }
      m_render->setCameraView(p, r, MMDAgent_str2float(distance), MMDAgent_str2float(fovy), bone, boneName ? false : true);
      if (time) {
         m_render->setViewMoveTimer(MMDAgent_str2double(time));
         m_timer->start(MMDAGENT_TIMERMOVE);
      } else
         m_render->setViewMoveTimer(-1.0);
      return true;
   }

   if (MMDAgent_exist(posOrVMD) == false) {
      sendLogString(m_moduleId, MLOG_ERROR, "changeCamera: not found: %s", posOrVMD);
      return false;
   }
   vmd = m_motion->loadFromFile(posOrVMD);
   if (vmd == NULL) {
      sendLogString(m_moduleId, MLOG_ERROR, "changeCamera: failed to load %s", posOrVMD);
      return false;
   }
   m_camera.setup(vmd);
   m_camera.reset();
   m_cameraControlled = true;
   return true;
}

/* MMDAgent::changeLightColor: change light color */
bool MMDAgent::changeLightColor(float r, float g, float b)
{
   float f[3];

   f[0] = r;
   f[1] = g;
   f[2] = b;
   m_option->setLightColor(f);
   updateLight();

   /* don't send message */
   return true;
}

/* MMDAgent::changeLightDirection: change light direction */
bool MMDAgent::changeLightDirection(float x, float y, float z)
{
   float f[4];

   f[0] = x;
   f[1] = y;
   f[2] = z;
   f[3] = 0.0f;
   m_option->setLightDirection(f);
   updateLight();

   /* don't send message */
   return true;
}

/* MMDAgent::startLipSync: start lip sync */
bool MMDAgent::startLipSync(const char *modelAlias, const char *seq)
{
   int id;
   unsigned char *vmdData;
   unsigned int vmdSize;
   VMD *vmd;

   /* ID */
   id = findModelAlias(modelAlias);
   if (id < 0) {
      sendLogString(m_moduleId, MLOG_WARNING, "startLipSync: %s is not found.", modelAlias);
      return false;
   }

   /* create motion */
   if(m_model[id].createLipSyncMotion(seq, &vmdData, &vmdSize) == false) {
      sendLogString(m_moduleId, MLOG_ERROR, "startLipSync: cannot create lip motion.");
      return false;
   }
   vmd = m_motion->loadFromData(vmdData, vmdSize);
   free(vmdData);

   /* search running lip motion */

   if (m_model[id].getMotionManager()->getRunning(LIPSYNC_MOTIONNAME) != NULL) {
      if (m_model[id].swapMotion(vmd, LIPSYNC_MOTIONNAME) == false) {
         sendLogString(m_moduleId, MLOG_WARNING, "startLipSync: lip sync cannot be started.");
         m_motion->unload(vmd);
         return false;
      }
      sendMessage(m_moduleId, MMDAGENT_EVENT_LIPSYNCSTOP, "%s", modelAlias);
   } else {
      if (m_model[id].startMotion(vmd, LIPSYNC_MOTIONNAME, true, true, true, false, m_option->getLipsyncPriority()) == false) {
         sendLogString(m_moduleId, MLOG_WARNING, "startLipSync: lip sync cannot be started.");
         m_motion->unload(vmd);
         return false;
      }
   }

   /* send message */
   sendMessage(m_moduleId, MMDAGENT_EVENT_LIPSYNCSTART, "%s", modelAlias);
   return true;
}

/* MMDAgent::stopLipSync: stop lip sync */
bool MMDAgent::stopLipSync(const char *modelAlias)
{
   int id;

   /* ID */
   id = findModelAlias(modelAlias);
   if (id < 0) {
      sendLogString(m_moduleId, MLOG_WARNING, "stopLipSync: %s is not found.", modelAlias);
      return false;
   }

   /* stop lip sync */
   if (m_model[id].getMotionManager()->deleteMotion(LIPSYNC_MOTIONNAME) == false) {
      sendLogString(m_moduleId, MLOG_WARNING, "stopLipSync: lipsync motion is not found.");
      return false;
   }

   /* don't send message yet */
   return true;
}

/* MMDAgent::addBoneControl: add bone control */
bool MMDAgent::addBoneControl(const char *valueName, float valueMin, float valueMax, const char *modelAlias, const char *boneName, btVector3 *pos1, btQuaternion *rot1, btVector3 *pos2, btQuaternion *rot2)
{
   int id;
   PMDBone *bone;
   BoneFaceControl *control;

   id = findModelAlias(modelAlias);
   if (id < 0) {
      sendLogString(m_moduleId, MLOG_WARNING, "addBoneControl: Model %s not exist", modelAlias);
      return false;
   }
   bone = m_model[id].getPMDModel()->getBone(boneName);
   if (bone == NULL) {
      sendLogString(m_moduleId, MLOG_ERROR, "addBoneControl: Bone %s not exist in Model %s", boneName, modelAlias);
      return false;
   }
   void* ptr = MMDFiles_alignedmalloc(sizeof(BoneFaceControl), 16);
   control = new(ptr) BoneFaceControl;
   control->setupBone(m_keyvalue, valueName, valueMin, valueMax, boneName, pos1, pos2, rot1, rot2);
   if (m_model[id].findBoneFaceControl(control) == true) {
      // already exist, override
      m_model[id].unsetBoneFaceControl(control);
   }
   if (m_model[id].setBoneFaceControl(control) == false) {
      sendLogString(m_moduleId, MLOG_ERROR, "addBoneControl: cannot bind %s to %s of %s", valueName, boneName, modelAlias);
      return false;
   }

   return true;
}

/* MMDAgent::addMorphControl: add morph control */
bool MMDAgent::addMorphControl(const char *valueName, float valueMin, float valueMax, const char *modelAlias, const char *morphName, float fmin, float fmax)
{
   int id;
   PMDFace *face;
   PMDBoneMorph *boneMorph;
   PMDVertexMorph *vertexMorph;
   PMDUVMorph *uvMorph;
   PMDMaterialMorph *materialMorph;
   PMDGroupMorph *groupMorph;
   BoneFaceControl *control;

   id = findModelAlias(modelAlias);
   if (id < 0) {
      sendLogString(m_moduleId, MLOG_WARNING, "addMorphControl: Model %s not exist", modelAlias);
      return false;
   }
   boneMorph = m_model[id].getPMDModel()->getBoneMorph(morphName);
   vertexMorph = m_model[id].getPMDModel()->getVertexMorph(morphName);
   uvMorph = m_model[id].getPMDModel()->getUVMorph(morphName);
   materialMorph = m_model[id].getPMDModel()->getMaterialMorph(morphName);
   groupMorph = m_model[id].getPMDModel()->getGroupMorph(morphName);
   face = m_model[id].getPMDModel()->getFace(morphName);
   if (boneMorph == NULL && vertexMorph == NULL && uvMorph == NULL && materialMorph == NULL && groupMorph == NULL && face == NULL) {
      sendLogString(m_moduleId, MLOG_ERROR, "addMorphControl: Morph %s not exist in Model %s", morphName, modelAlias);
      return false;
   }
   void* ptr = MMDFiles_alignedmalloc(sizeof(BoneFaceControl), 16);
   control = new(ptr) BoneFaceControl;
   control->setupMorph(m_keyvalue, valueName, valueMin, valueMax, morphName, fmin, fmax);
   if (m_model[id].findBoneFaceControl(control) == true) {
      // already exist, override
      m_model[id].unsetBoneFaceControl(control);
   }
   if (m_model[id].setBoneFaceControl(control) == false) {
      if (valueName)
         sendLogString(m_moduleId, MLOG_ERROR, "addMorphControl: cannot bind %s to %s of %s", valueName, morphName, modelAlias);
      else
         sendLogString(m_moduleId, MLOG_ERROR, "addMorphControl: cannot bind value %f to %s of %s",fmin, morphName, modelAlias);
      return false;
   }

   return true;
}

/* MMDAgent::removeBoneControl: remove bone control */
bool MMDAgent::removeBoneControl(const char *modelAlias, const char *boneName)
{
   int id;
   PMDBone *bone;
   BoneFaceControl *control;

   id = findModelAlias(modelAlias);
   if (id < 0) {
      sendLogString(m_moduleId, MLOG_WARNING, "removeBoneControl: Model %s not exist", modelAlias);
      return false;
   }
   bone = m_model[id].getPMDModel()->getBone(boneName);
   if (bone == NULL) {
      sendLogString(m_moduleId, MLOG_WARNING, "removeBoneControl: Bone %s not exist in Model %s", boneName, modelAlias);
      return false;
   }
   void* ptr = MMDFiles_alignedmalloc(sizeof(BoneFaceControl), 16);
   control = new(ptr) BoneFaceControl;
   control->setupSkeleton(boneName, true);
   if (m_model[id].findBoneFaceControl(control) == false) {
      sendLogString(m_moduleId, MLOG_WARNING, "removeBoneControl: bind %s of %s does not exist", boneName, modelAlias);
   } else {
      m_model[id].unsetBoneFaceControl(control);
   }
   control->~BoneFaceControl();
   MMDFiles_alignedfree(control);

   return true;
}

/* MMDAgent::removeMorphControl: remove morph control */
bool MMDAgent::removeMorphControl(const char *modelAlias, const char *morphName)
{
   int id;
   PMDFace *face;
   PMDBoneMorph *boneMorph;
   PMDVertexMorph *vertexMorph;
   PMDUVMorph *uvMorph;
   PMDMaterialMorph *materialMorph;
   PMDGroupMorph *groupMorph;
   BoneFaceControl *control;

   id = findModelAlias(modelAlias);
   if (id < 0) {
      sendLogString(m_moduleId, MLOG_WARNING, "removeMorphControl: Model %s not exist", modelAlias);
      return false;
   }
   boneMorph = m_model[id].getPMDModel()->getBoneMorph(morphName);
   vertexMorph = m_model[id].getPMDModel()->getVertexMorph(morphName);
   uvMorph = m_model[id].getPMDModel()->getUVMorph(morphName);
   materialMorph = m_model[id].getPMDModel()->getMaterialMorph(morphName);
   groupMorph = m_model[id].getPMDModel()->getGroupMorph(morphName);
   face = m_model[id].getPMDModel()->getFace(morphName);
   if (boneMorph == NULL && vertexMorph == NULL && uvMorph == NULL && materialMorph == NULL && groupMorph == NULL && face == NULL) {
      sendLogString(m_moduleId, MLOG_WARNING, "removeMorphControl: Morph %s not exist in Model %s", morphName, modelAlias);
      return false;
   }
   void* ptr = MMDFiles_alignedmalloc(sizeof(BoneFaceControl), 16);
   control = new(ptr) BoneFaceControl;
   control->setupSkeleton(morphName, false);
   if (m_model[id].findBoneFaceControl(control) == false) {
      sendLogString(m_moduleId, MLOG_WARNING, "removeMorphControl: bind %s of %s does not exist",morphName, modelAlias);
   } else {
      m_model[id].unsetBoneFaceControl(control);
   }
   control->~BoneFaceControl();
   MMDFiles_alignedfree(control);

   return true;
}

/* MMDAgent::startMotionCapture: start motion capture */
bool MMDAgent::startMotionCapture(const char *modelAlias, const char *fileName)
{
   int id;
   unsigned int maxMinutes;

   id = findModelAlias(modelAlias);
   if (id < 0) {
      sendLogString(m_moduleId, MLOG_WARNING, "startMotionCapture: model alias \"%s\" not found.", modelAlias);
      return false;
   }

   maxMinutes = 0;
   if (m_keyvalue)
      maxMinutes = atoi(m_keyvalue->getString("motion_capture_max_minutes", "0"));
   sendLogString(m_moduleId, MLOG_STATUS, "startMotionCapture: starting capturing motion of \"%s\" to VMD file %s.", modelAlias, fileName);
   if (m_model[id].startCapture(fileName, maxMinutes) == false) {
      sendLogString(m_moduleId, MLOG_ERROR, "startMotionCapture: failed to start motion capturing.");
      return false;
   }

   return true;
}

/* MMDAgent::stopMotionCapture: stop motion capture */
bool MMDAgent::stopMotionCapture(const char *modelAlias)
{
   int id;

   id = findModelAlias(modelAlias);
   if (id < 0) {
      sendLogString(m_moduleId, MLOG_WARNING, "stopMotionCapture: model alias \"%s\" not found.", modelAlias);
      return false;
   }

   if (m_model[id].stopCapture() == false) {
      sendLogString(m_moduleId, MLOG_ERROR, "stopMotionCapture: error occured at file operation, failed to save captured motions to file.");
      return false;
   }
   sendLogString(m_moduleId, MLOG_STATUS, "stopMotionCapture: done saving motion of \"%s\" to file.", modelAlias);


   return true;
}

/* MMDAgent::setTexureAnimationSpeed: set texture animation speed rate */
bool MMDAgent::setTexureAnimationSpeedRate(const char *modelAlias, const char *textureFileName, double rate)
{
   int id;

   id = findModelAlias(modelAlias);
   if (id < 0) {
      sendLogString(m_moduleId, MLOG_WARNING, "setTexureAnimationSpeedRate: model alias \"%s\" not found.", modelAlias);
      return false;
   }

   if (m_model[id].getPMDModel()->getTextureLoader()->setAnimationSpeedRate(textureFileName, rate) == false) {
      sendLogString(m_moduleId, MLOG_WARNING, "setTexureAnimationSpeedRate: texture \"%s\" not exist in loaded texture list of model alias \"%s\"", textureFileName, modelAlias);
      return false;
   }

   sendLogString(m_moduleId, MLOG_STATUS, "setTexureAnimationSpeedRate: animation speed of texture \"%s\" on model \"%s\" was set to %lf", textureFileName, modelAlias, rate);

   return true;
}

/* MMDAgent::initialize: initialize MMDAgent */
void MMDAgent::initialize()
{
   m_enable = false;
   m_moduleId = 0;
   m_title = NULL;
   m_sysDownloadURL = NULL;
   m_pluginDirName = NULL;
   m_systemConfigFileName = NULL;
   m_tickCount = 0;
   m_userDirName = NULL;
   m_tempDirName = NULL;
   m_configFileName = NULL;
   m_configDirName = NULL;
   m_systemDirName = NULL;
   m_appDirName = NULL;
   m_argv = NULL;
   m_argc = 0;
   m_loadHome = true;
   m_screenSize[0] = m_screenSize[1] = 0;
   m_optionalStatusString[0] = '\0';

   m_option = NULL;
   m_screen = NULL;
   m_message = NULL;
   m_bullet = NULL;
   m_plugin = NULL;
   m_stage = NULL;
   m_systex = NULL;
   m_lipSync = NULL;
   m_render = NULL;
   m_timer = NULL;
   m_keyvalue = NULL;
   m_atlas = NULL;
   m_font = NULL;
   m_fontAwesome = NULL;
   memset(&m_elem, 0, sizeof(FTGLTextDrawElements));
   memset(&m_elemAwesome, 0, sizeof(FTGLTextDrawElements));
   m_loggerLog = NULL;
   m_loggerMessage = NULL;
   m_menu = NULL;
   m_button = NULL;
   m_buttonTop = NULL;
   m_buttonShowing = false;
   m_filebrowser = NULL;
   m_prompt = NULL;
   m_notify = NULL;
   m_infotext = NULL;
   m_slider = NULL;
   m_tabbar = NULL;

   m_model = NULL;
   m_renderOrder = NULL;
   m_numModel = 0;
   m_motion = NULL;
   m_hasExtModel = false;

   m_cameraControlled = false;

   m_keyCtrl = false;
   m_keyShift = false;
   m_selectedModel = -1;
   m_highLightingModel = -1;
   m_doubleClicked = false;
   m_mousePosY = 0;
   m_mousePosX = 0;
   m_leftButtonPressed = false;
   m_restFrame = 0.0;
   m_stepFrame = 0.25;
   m_maxStep = 120 * MMDAGENT_MAXPAUSESEC;
   m_enablePhysicsSimulation = true;
   m_dispLog = false;
   m_dispLogConsole = false;
   m_dispBulletBodyFlag = false;
   m_dispModelDebug = false;
   m_holdMotion = false;

   m_content = NULL;

   for (int i = 0; i < MMDAGENT_MAXNUMCONTENTBUTTONS; i++)
      m_contentButtons[i] = NULL;

   m_fpLog = NULL;
   m_resetFlag = false;
   m_hardResetFlag = false;
   m_startingFrame = MMDAGENT_STARTANIMATIONFRAME;
   m_startingFrameSkipCount = 0;
   m_startingFramePattern = 0;
   m_contentInErrorPrompt = false;
   m_pluginStarted = false;
   m_contentUpdateStarted = false;
   m_contentUpdateChecked = false;
   m_contentUpdateWait = 0;
   m_logToFile = NULL;
   m_logUploader = NULL;
   m_contentLaunched = false;
   m_contentDocViewing = false;

   m_autoUpdateFiles = NULL;
   m_autoUpdatePeriod = 0.0;
   m_tinyDownload = NULL;

   m_threadsPauseFlag = false;
   m_threadsPauseMutex = NULL;
   m_threadsPauseCond = NULL;
#ifdef MMDAGENT_CURRENTTIME
   m_timeMessageFrame = MMDAGENT_CURRENTTIME_INTERVAL_FRAME;
#endif
   m_cameraCanMove = false;
   m_showUsageFrame = MMDAGENT_SHOWUSAGEFRAME;

   m_threadedLoading = NULL;
   m_offscreen = NULL;
   m_keyHandler.setup(this);
   m_caption = NULL;
   m_httpServer = NULL;
   m_logoTex = NULL;

   m_errorMessages[0] = '\0';
   memset(&m_elemErrorMessage, 0, sizeof(FTGLTextDrawElements));
}

/* MMDAgent::clear: free MMDAgent */
void MMDAgent::clear()
{
   int id;
   unsigned int flag;
   static char buf1[MMDAGENT_MAXBUFLEN];
   static char buf2[MMDAGENT_MAXBUFLEN];
   int i;
   Button *b, *btmp;

   if (m_message) {
      /* process stored message */
      while(m_message->dequeueLogString(&id, &flag, buf1, buf2) == true)
         procReceivedLogString(id, flag, buf1, buf2);
   }

   m_enable = false;

   if (m_title)
      free(m_title);
   if (m_sysDownloadURL)
      free(m_sysDownloadURL);
   if (m_pluginDirName)
      free(m_pluginDirName);
   if (m_systemConfigFileName)
      free(m_systemConfigFileName);
   if(m_userDirName) {
      MMDAgent_chdir(m_userDirName);
      free(m_userDirName);
   }
   if(m_tempDirName) {
      MMDAgent_rmdir(m_tempDirName);
      free(m_tempDirName);
   }
   if (m_content)
      delete m_content;
   if (m_fpLog)
      fclose(m_fpLog);
   if(m_configFileName)
      free(m_configFileName);
   if(m_configDirName)
      free(m_configDirName);
   if(m_systemDirName)
      free(m_systemDirName);
   if(m_appDirName)
      free(m_appDirName);
   if(m_argv) {
      for (i = 0; i < m_argc; i++)
         free(m_argv[i]);
      free(m_argv);
   }
   if(m_motion)
      delete m_motion;
   if (m_renderOrder)
      free(m_renderOrder);
   if (m_model) {
      for (int i = 0; i < m_option->getMaxNumModel(); i++)
         m_model[i].~PMDObject();
      MMDFiles_alignedfree(m_model);
   }
   if (m_elem.vertices) free(m_elem.vertices);
   if (m_elem.texcoords) free(m_elem.texcoords);
   if (m_elem.indices) free(m_elem.indices);
   if (m_elemAwesome.vertices) free(m_elemAwesome.vertices);
   if (m_elemAwesome.texcoords) free(m_elemAwesome.texcoords);
   if (m_elemAwesome.indices) free(m_elemAwesome.indices);
   if (m_atlas)
      delete m_atlas;
   if (m_font)
      delete m_font;
   if (m_fontAwesome)
      delete m_fontAwesome;
   if (m_loggerLog)
      delete m_loggerLog;
   if (m_loggerMessage)
      delete m_loggerMessage;
   if (m_timer)
      delete m_timer;
   if (m_keyvalue)
      delete m_keyvalue;
   if (m_render) {
      m_render->~Render();
      MMDFiles_alignedfree(m_render);
   }
   if (m_menu)
      delete m_menu;
   b = m_button;
   while (b) {
      btmp = b->getNext();
      delete b;
      b = btmp;
   }
   for (int i = 0; i < MMDAGENT_MAXNUMCONTENTBUTTONS; i++) {
      if (m_contentButtons[i])
         delete m_contentButtons[i];
   }
   if (m_tabbar)
      delete m_tabbar;
   if (m_slider)
      delete m_slider;
   if (m_infotext)
      delete m_infotext;
   if (m_prompt)
      delete m_prompt;
   if (m_notify)
      delete m_notify;
   if (m_filebrowser)
      delete m_filebrowser;
   if (m_lipSync)
      delete m_lipSync;
   if (m_systex)
      delete m_systex;
   if (m_stage) {
      m_stage->~Stage();
      MMDFiles_alignedfree(m_stage);
   }
   if (m_plugin)
      delete m_plugin;
   if (m_bullet)
      delete m_bullet;
   if (m_message)
      delete m_message;
   if (m_screen)
      delete m_screen;
   if (m_option)
      delete m_option;
   if (m_logToFile)
      delete m_logToFile;
   if (m_logUploader)
      delete m_logUploader;
   if (m_autoUpdateFiles)
      free(m_autoUpdateFiles);
   if (m_tinyDownload)
      delete m_tinyDownload;

   if (m_threadsPauseCond)
      glfwDestroyCond(m_threadsPauseCond);
   if (m_threadsPauseMutex)
      glfwDestroyMutex(m_threadsPauseMutex);

   if (m_threadedLoading)
      delete m_threadedLoading;

   if (m_offscreen)
      delete m_offscreen;

   if (m_caption)
      delete m_caption;

   if (m_httpServer)
      delete m_httpServer;

   if (m_logoTex)
      delete m_logoTex;

   if (m_elemErrorMessage.vertices) free(m_elemErrorMessage.vertices);
   if (m_elemErrorMessage.texcoords) free(m_elemErrorMessage.texcoords);
   if (m_elemErrorMessage.indices) free(m_elemErrorMessage.indices);

   initialize();
}

/* MMDAgent::MMDAgent: constructor */
MMDAgent::MMDAgent()
{
   initialize();
}

/* MMDAgent::~MMDAgent: destructor */
MMDAgent::~MMDAgent()
{
   clear();
}

/* MMDAgent::restart: restart MMDAgent */
bool MMDAgent::restart(const char *systemDirName, const char *pluginDirName, const char *systemConfigFileName, const char *sysDownloadURL, const char *title)
{
   ScreenWindow *screen;
   int argc;
   char **argv;
   int startingFramePattern;
   int w, h;
   bool ret;

   sendLogString(m_moduleId, MLOG_STATUS, "---------- restart");
   /* save startup arguments, window objects and properties */
   argv = m_argv;
   argc = m_argc;
   screen = m_screen;
   startingFramePattern = m_startingFramePattern;
   w = m_screenSize[0];
   h = m_screenSize[1];
   /* make sure they are not cleared */
   m_argv = NULL;
   m_argc = 0;
   m_screen = NULL;
   /* process end of application */
   procWindowDestroyMessage();
   /* initialize whole data */
   initialize();
   /* restore */
   m_startingFramePattern = startingFramePattern;
   m_screen = screen;
   m_argv = argv;
   m_argc = argc;
   m_screenSize[0] = w;
   m_screenSize[1] = h;
   /* execute setup with current arguments */
   ret = setupSystem(systemDirName, pluginDirName, systemConfigFileName, sysDownloadURL, title);
   if (ret == true)
      ret = setupContent(0, NULL);

   /* make sure not loop restart */
   m_resetFlag = false;
   m_hardResetFlag = false;

   return ret;
}

/* MMDAgent::setupSystem: setup system */
bool MMDAgent::setupSystem(const char *systemDirName, const char *pluginDirName, const char *systemConfigFileName, const char *sysDownloadURL, const char *title)
{
   char buff[MMDAGENT_MAXBUFLEN];
   char *errmsg;

   /* store startup directory */
   m_userDirName = MMDAgent_pwddup();

   /* store window title */
   m_title = MMDAgent_strdup(title);

   /* store system download URL */
   m_sysDownloadURL = MMDAgent_strdup(sysDownloadURL);

   /* store plugin directory */
   m_pluginDirName = MMDAgent_strdup(pluginDirName);

   /* initialize log message queue */
   m_message = new Message();
   if (m_message->setup() == false) {
      clear();
      return false;
   }

   /* get module id */
   m_moduleId = m_message->getId("MMDAgent");
   if (m_moduleId == -1) {
      clear();
      return false;
   }

   /* initialize generic key-value pairs */
   m_keyvalue = new KeyValue;
   m_keyvalue->setup();

   /* set paths */
   if (m_systemDirName)
      free(m_systemDirName);
   m_systemDirName = MMDAgent_strdup(systemDirName);
   sendLogString(m_moduleId, MLOG_STATUS, "system dir = %s", m_systemDirName);
#if TARGET_OS_IPHONE
   /* default current directory is "/" in ios */
   MMDAgent_chdir(m_systemDirName);
#endif
   if (m_appDirName)
      free(m_appDirName);
   MMDAgent_snprintf(buff, MMDAGENT_MAXBUFLEN, "%s%c%s", m_systemDirName, MMDAGENT_DIRSEPARATOR, "AppData");
   m_appDirName = MMDAgent_strdup(buff);
   sendLogString(m_moduleId, MLOG_STATUS, "AppData dir = %s", m_appDirName);

   /* initialize Option */
   m_option = new Option();

   /* load system config file */
   if (m_option->load(systemConfigFileName, NULL, &errmsg) == true)
      sendLogString(m_moduleId, MLOG_STATUS, "system mdf file = %s", systemConfigFileName);
   else
      sendLogString(m_moduleId, MLOG_WARNING, "%s\nerror in reading file, skipped (%s)", errmsg, systemConfigFileName);
   m_keyvalue->load(systemConfigFileName, NULL);

   /* create window */
   if (m_screen == NULL) {
      m_screen = new ScreenWindow();
      if (m_screen->setup(m_option->getWindowSize(), title, m_option->getMaxMultiSampling()) == false) {
         sendLogString(m_moduleId, MLOG_ERROR, "failed to initialize screen");
         clear();
         return false;
      }
      m_screenSize[0] = m_option->getWindowSize()[0];
      m_screenSize[1] = m_option->getWindowSize()[1];
   }

   /* load toon textures from system directory */
   m_systex = new SystemTexture();
   if (m_systex->load(m_appDirName) == false) {
      sendLogString(m_moduleId, MLOG_ERROR, "failed to load system toon texture in %s", m_appDirName);
      delete m_systex;
      m_systex = NULL;
   }

   /* setup lipsync */
   m_lipSync = new LipSync();
   MMDAgent_snprintf(buff, MMDAGENT_MAXBUFLEN, "%s%c%s", m_appDirName, MMDAGENT_DIRSEPARATOR, LIPSYNC_CONFIGFILE);
   if (m_lipSync->load(buff) == false) {
      sendLogString(m_moduleId, MLOG_ERROR, "failed to load lipsync config file %s", buff);
      delete m_lipSync;
      m_lipSync = NULL;
   } else {
      sendLogString(m_moduleId, MLOG_STATUS, "lipsync config file = %s", buff);
   }

   /* setup stage */
   void* ptr = MMDFiles_alignedmalloc(sizeof(Stage), 16);
   m_stage = new(ptr) Stage();
   m_stage->setSize(m_option->getStageSize(), 1.0f, 1.0f);

   /* setup render */
   ptr = MMDFiles_alignedmalloc(sizeof(Render), 16);
   m_render = new(ptr) Render();
   if (m_render->setup(m_screenSize, m_option->getCampusColor(), m_option->getCameraTransition(), m_option->getCameraRotation(), m_option->getCameraDistance(), m_option->getCameraFovy(), m_option->getUseShadow(), m_option->getUseShadowMapping(), m_option->getShadowMappingTextureSize(), m_option->getMaxNumModel()) == false) {
      sendLogString(m_moduleId, MLOG_ERROR, "failed to initialize renderer");
      clear();
      return false;
   }

   /* setup timer */
   m_timer = new Timer();
   m_timer->setup();
   m_timer->startAdjustment();

   /* set up text rendering */
   m_atlas = new FTGLTextureAtlas();
   if (m_atlas->setup() == false) {
      sendLogString(m_moduleId, MLOG_ERROR, "failed to initialize font texture atlas");
      delete m_atlas;
      m_atlas = NULL;
   } else {
      m_font = new FTGLTextureFont();
      MMDAgent_snprintf(buff, MMDAGENT_MAXBUFLEN, "%s%c%s%c%s", m_appDirName, MMDAGENT_DIRSEPARATOR, FREETYPEGL_FONTDIR, MMDAGENT_DIRSEPARATOR, FREETYPEGL_FONTFILE);
      if (m_font->setup(m_atlas, buff) == false) {
         sendLogString(m_moduleId, MLOG_WARNING, "failed to load font from %s", buff);
         if (m_font->setup(m_atlas, NULL) == false) {
            sendLogString(m_moduleId, MLOG_ERROR, "failed to load embedded default font");
            delete m_font;
            m_font = NULL;
         }
         sendLogString(m_moduleId, MLOG_WARNING, "use embedded BreeSerif font");
      }
      m_fontAwesome = new FTGLTextureFont();
      MMDAgent_snprintf(buff, MMDAGENT_MAXBUFLEN, "%s%c%s", m_appDirName, MMDAGENT_DIRSEPARATOR, FREETYPEGL_FONTFILE_AWESOME);
      if (m_fontAwesome->setup(m_atlas, buff) == false) {
         sendLogString(m_moduleId, MLOG_WARNING, "failed to load font from %s", buff);
         delete m_fontAwesome;
         m_fontAwesome = NULL;
      }
   }

   /* setup content manager */
   m_content = new ContentManager();
   m_content->setup(this, m_moduleId);

   /* setup motions */
   m_motion = new MotionStocker();

   /* set mouse enable timer */
   m_screen->setMouseActiveTime(45.0f);

   /* set default config file name */
   m_systemConfigFileName = MMDAgent_strdup(systemConfigFileName);
   m_configFileName = MMDAgent_strdup(systemConfigFileName);

   /* setup menus */
   m_menu = new Menu();
   m_menu->setup(this, m_moduleId, m_font, m_appDirName);
   m_content->setMenu(m_menu);

   /* setup infotext */
   m_infotext = new InfoText();
   m_infotext->setup(this, m_moduleId);

   /* setup slider */
   m_slider = new Slider();
   m_slider->setup(this, m_moduleId);

   /* setup tabbar */
   m_tabbar = new Tabbar();
   m_tabbar->setup(this, m_moduleId, m_fontAwesome, m_font);

   /* setup logtofile */
   m_logToFile = new LogToFile();

   /* setup threaded loading */
   m_threadedLoading = new ThreadedLoading(this);

#ifndef NO_OFFSCREEN_RENDERING
   /* setup off-screen rendering */
   m_offscreen = new RenderOffScreen();
   if (m_offscreen->setup(m_render, m_screenSize[0], m_screenSize[1]) == false) {
      sendLogString(m_moduleId, MLOG_ERROR, "off-screen rendering failed: %s", m_offscreen->getInfoLog());
      delete m_offscreen;
      m_offscreen = NULL;
   }
#endif /* NO_OFFSCREEN_RENDERING */

   /* setup caption */
   m_caption = new Caption();
   m_caption->setup(this, m_moduleId);

   /* setup logo */
   if (m_logoTex)
      delete m_logoTex;
   MMDAgent_snprintf(buff, MMDAGENT_MAXBUFLEN, "%s%c%s", m_appDirName, MMDAGENT_DIRSEPARATOR, "logo.png");
   m_logoTex = new PMDTexture;
   if (m_logoTex->loadImage(buff) == false) {
      delete m_logoTex;
      m_logoTex = NULL;
   }

   m_enable = true;

   /* force camera lock */
   m_cameraCanMove = false;

   return true;
}

/* MMDAgent::setupContent: setup content */
bool MMDAgent::setupContent(int argc, char **argv)
{
   int i;
   char *cacheDirName;
   char *targetDir;
   char buff[MMDAGENT_MAXBUFLEN];
   char *urlPath;
   int urlPathStartIdx;
   bool hasMDF;
   char *errmsg;

   /* store whole arguments */
   if (argv != NULL) {
      if (m_argv != NULL) {
         for (i = 0; i < m_argc; i++)
            free(m_argv[i]);
         free(m_argv);
      }
      m_argc = argc;
      m_argv = (char **)malloc(sizeof(char *) * argc);
      for (i = 0; i < argc; i++)
         m_argv[i] = MMDAgent_strdup(argv[i]);
   }

   if (m_argc < 1 || MMDAgent_strlen(m_argv[0]) <= 0)
      return false;

   if (m_sysDownloadURL) {
      /* check if system files has been downloaded by cached status */
      if (m_content->checkContentComplete(m_systemDirName) == false) {
         /* download not complete or cache is not found: try to sync system directory with the URL */
         m_content->startExtractContent(m_sysDownloadURL, m_systemDirName, true, false, true);
         return true;
      }
   }

   /* load URL ban list */
   m_content->loadBanList(m_systemDirName);

   /* load given mdf files.  The last .mdf file will be the launch content */
   hasMDF = false;
   for (i = 1; i < m_argc; i++) {
      if (MMDAgent_strtailmatch(m_argv[i], ".mdf") || MMDAgent_strtailmatch(m_argv[i], ".MDF")) {
         if (m_option->load(m_argv[i], NULL, &errmsg)) {
            sendLogString(m_moduleId, MLOG_STATUS, "loading user mdf file = %s", m_argv[i]);
            m_keyvalue->load(m_argv[i], NULL);
            if(m_configFileName)
               free(m_configFileName);
            m_configFileName = MMDAgent_strdup(m_argv[i]);
         } else {
            sendLogString(m_moduleId, MLOG_WARNING, "%s\nfailed to load mdf, skipped (%s)", errmsg, m_argv[i]);
         }
         hasMDF = true;
      }
   }

   /* determine which content to launch */
   /* if any .mmda file or URL is given in argv, start from it (override mdf) */
   urlPath = NULL;
   for (i = 1; i < m_argc; i++) {
      if (MMDAgent_strtailmatch(m_argv[i], ".mmda")) {
         cacheDirName = MMDAgent_tmpdirdup();
         sendLogString(m_moduleId, MLOG_STATUS, "extracting %s to cache dir %s", m_argv[i], cacheDirName);
         targetDir = prepareCacheDirDup(cacheDirName);
         free(cacheDirName);
         if (targetDir == NULL) {
            sendLogString(m_moduleId, MLOG_ERROR, "failed to prepare content cache directory");
            return false;
         }
         m_content->startExtractContent(m_argv[i], targetDir, false, false, false);
         break;
      } else if (MMDAgent_strheadmatch(m_argv[i], "http://")) {
         if (m_content->isBanned(m_argv[i])) {
            sendLogString(m_moduleId, MLOG_WARNING, "URL matches ban patten, skipped: %s", m_argv[i]);
         } else {
            urlPath = MMDAgent_strdup(m_argv[i]);
            urlPathStartIdx = 7;
         }
         break;
      } else if (MMDAgent_strheadmatch(m_argv[i], "https://")) {
         if (m_content->isBanned(m_argv[i])) {
            sendLogString(m_moduleId, MLOG_WARNING, "URL matches ban patten, skipped: %s", m_argv[i]);
         } else {
            urlPath = MMDAgent_strdup(m_argv[i]);
            urlPathStartIdx = 8;
         }
         break;
      } else if (MMDAgent_strheadmatch(m_argv[i], "mmdagent://")) {
         if (m_content->isBanned(m_argv[i])) {
            sendLogString(m_moduleId, MLOG_WARNING, "URL matches ban patten, skipped: %s", m_argv[i]);
         } else {
            urlPath = MMDAgent_strdup(m_argv[i]);
            urlPathStartIdx = 11;
         }
         break;
      }
   }

   if (hasMDF == false && urlPath == NULL) {
      sendLogString(m_moduleId, MLOG_STATUS, "no content specified at spawn...");
      if (m_loadHome) {
         /* load home content */
         urlPath = m_content->getHomeURLdup(&urlPathStartIdx);
         if (urlPath) {
            sendLogString(m_moduleId, MLOG_STATUS, "starting with home content: %s", urlPath);
            if (urlPathStartIdx == 0) {
               /* this is file home */
               hasMDF = true;
               if (m_configFileName)
                  free(m_configFileName);
               m_configFileName = MMDAgent_strdup(urlPath);
               free(urlPath);
               urlPath = NULL;
            }
         }
      }
   }

   if (urlPath != NULL) {
      /* download from the URL */
      /* generate 32bit FNV hash string as cache dir name */
      unsigned int hash = 2166136261U;
      char *p = &(urlPath[urlPathStartIdx]);
      while (*p != '\0') {
         hash = (16777619U * hash) ^ *p;
         p++;
      }
      MMDAgent_snprintf(buff, MMDAGENT_MAXBUFLEN, "%u", hash);

      sendLogString(m_moduleId, MLOG_STATUS, "downloading %s to cache dir %s", urlPath, buff);
      targetDir = prepareCacheDirDup(buff);
      if (targetDir == NULL) {
         sendLogString(m_moduleId, MLOG_ERROR, "failed to prepare content cache directory");
         return false;
      }
      m_content->startExtractContent(urlPath, targetDir, false, false, false);
   }

   if (urlPath == NULL && hasMDF == true) {
      /* define config dir as the place .mdf is located */
      /* update local package info */
      char *dir = MMDAgent_dirname(m_configFileName);
      if (g_enckey)
         delete g_enckey;
      g_enckey = new ZFileKey();
      if (g_enckey->loadKeyDir(dir) == false) {
         delete g_enckey;
         g_enckey = NULL;
      }
      m_content->setupLocalPackageInfo(dir);
      free(dir);
      /* re-try reading .mdf with encryption option */
      if (m_option->load(m_configFileName, g_enckey, &errmsg)) {
         sendLogString(m_moduleId, MLOG_STATUS, "loading user mdf file = %s", m_configFileName);
         m_keyvalue->load(m_configFileName, g_enckey);
      } else {
         sendLogString(m_moduleId, MLOG_WARNING, "%s failed to load mdf, skipped (%s)", errmsg, m_configFileName);
      }
   }

   if (urlPath)
      free(urlPath);

   return true;
}

/* MMDAgent::setupWorld: load world */
bool MMDAgent::setupWorld()
{
   /* re-set window size if needed */
   int *sizes = m_option->getWindowSize();
   if ((m_screenSize[0] != sizes[0] || m_screenSize[1] != sizes[1]) && m_option->getFullScreen() == false)
      m_screen->setWindowSize(sizes[0], sizes[1]);

   /* initialize BulletPhysics */
   m_bullet = new BulletPhysics();
   m_bullet->setup(m_option->getBulletFps(), m_option->getGravityFactor());
   if (m_option->getBulletFloor() == true)
      m_bullet->addFloor();

   /* set update step */
   m_stepFrame = 30.0 / m_option->getBulletFps();
   m_maxStep = m_option->getBulletFps() * MMDAGENT_MAXPAUSESEC;

   /* re-set stage size */
   m_stage->setSize(m_option->getStageSize(), 1.0f, 1.0f);

   /* re-setup render */
   if (m_render->setup(m_screenSize, m_option->getCampusColor(), m_option->getCameraTransition(), m_option->getCameraRotation(), m_option->getCameraDistance(), m_option->getCameraFovy(), m_option->getUseShadow(), m_option->getUseShadowMapping(), m_option->getShadowMappingTextureSize(), m_option->getMaxNumModel()) == false) {
      sendLogString(m_moduleId, MLOG_ERROR, "failed to initialize renderer");
      clear();
      return false;
   }
   m_render->setDoppelShadowFlag(m_option->getUseDoppelShadow());
   m_render->setDoppelShadowParam(m_option->getDoppelShadowColor(), m_option->getDoppelShadowOffset());

   /* setup logger */
   int logSize[2];
   float logPosition[3];
   int *lsize = m_option->getLogSize();
   float *lpos = m_option->getLogPosition();

   logSize[0] = lsize[0];
   logSize[1] = lsize[1] / 2;
   logPosition[0] = lpos[0];
   logPosition[1] = lpos[1];
   logPosition[2] = lpos[2];
   m_loggerLog = new LogText();
   if (m_loggerLog->setup(m_font, logSize, logPosition, m_option->getLogScale()) == false) {
      sendLogString(m_moduleId, MLOG_ERROR, "failed to setup logger");
      delete m_loggerLog;
      m_loggerLog = NULL;
   } else {
      m_loggerLog->setStatus(m_dispLog);
   }
   logSize[0] = lsize[0];
   logSize[1] = lsize[1] / 2;
   logPosition[0] = lpos[0];
   logPosition[1] = lpos[1] + logSize[1] * 0.85f + 1.2f;
   logPosition[2] = lpos[2];
   m_loggerMessage = new LogText();
   if (m_loggerMessage->setup(m_font, logSize, logPosition, m_option->getLogScale()) == false) {
      sendLogString(m_moduleId, MLOG_ERROR, "failed to setup logger");
      delete m_loggerMessage;
      m_loggerMessage = NULL;
   } else {
      m_loggerMessage->setStatus(m_dispLog);
   }

   /* setup prompter */
   m_prompt = new Prompt();
   m_prompt->setup(this, m_moduleId, m_font);

   /* setup notifier */
   m_notify = new Prompt();
   m_notify->setup(this, m_moduleId, m_font);

   /* setup models */
   m_model = (PMDObject*)MMDFiles_alignedmalloc(sizeof(PMDObject) * m_option->getMaxNumModel(), 16);
   for (int i = 0; i < m_option->getMaxNumModel(); i++)
      new (m_model + i) PMDObject();
   m_renderOrder = (int *)malloc(sizeof(int) * m_option->getMaxNumModel());

   /* set full screen */
   if (m_option->getFullScreen() == true)
      m_screen->setFullScreen();

   /* update light */
   updateLight();

   /* open log file */
   if (MMDAgent_strlen(m_option->getLogFile()) > 0)
      m_fpLog = MMDAgent_fopen(m_option->getLogFile(), "a");

   /* load plugins */
   m_plugin = new Plugin();
   sendLogString(m_moduleId, MLOG_STATUS, "plugin dir = %s", m_pluginDirName);
   sendLogString(m_moduleId, MLOG_STATUS, "disablePlugin = %s", m_keyvalue->getString("disablePlugin", "NONE"));
   sendLogString(m_moduleId, MLOG_STATUS, "enablePlugin = %s", m_keyvalue->getString("enablePlugin", "NONE"));
   m_plugin->setList(m_keyvalue->getString("disablePlugin", NULL), m_keyvalue->getString("enablePlugin", NULL));
   m_plugin->load(this, m_moduleId, m_pluginDirName);

   /* execute plugin initialize functions */
   m_plugin->execAppInit(this);

   /* setup file browser */
   m_filebrowser = new FileBrowser();
   m_filebrowser->setup(this, m_font, m_configDirName, NULL);

   /* setup bookmark menu */
   m_content->updateMenu(m_configDirName, m_configFileName);
   m_content->setContentCursorToCurrent();

   /* set buttons if defined */
   setButtonsInDir(m_configDirName);

   /* set off-screen rendering */
   if (m_offscreen && m_option->getUseDiffusionFilter() == true)
      m_offscreen->setParam(m_option->getDiffusionFilterIntensity(), m_option->getDiffusionFilterScale());

   /* set parallel skinning thread num */
   omp_set_num_threads(m_option->getParallelSkinningNumthreads());
   sendLogString(m_moduleId, MLOG_STATUS, "numthreads for skinning = %d", m_option->getParallelSkinningNumthreads());

   /* setup http server */
   if (m_option->getUseHttpServer() == true) {
      m_httpServer = new HttpServer(this, m_option->getHttpServerPortNumber());
      m_httpServer->start();
   }

   /* set transparent window handling */
   if (m_offscreen) {
      if (m_option->getTransparentWindow() == true && m_offscreen->isTransparentWindow() == false) {
         /* enable */
         const float *tcol = m_option->getTransparentColor();
         if (m_offscreen->enableTransparentWindow(tcol, m_option->getTransparentPixmap()) == false) {
            sendLogString(m_moduleId, MLOG_ERROR, "failed to make screen transparent");
            clear();
            return false;
         }
         sendLogString(m_moduleId, MLOG_STATUS, "transparent screen enabled");
      } else if (m_option->getTransparentWindow() == false && m_offscreen->isTransparentWindow() == true) {
         /* disable */
         if (m_offscreen->disableTransparentWindow() == false) {
            sendLogString(m_moduleId, MLOG_ERROR, "failed to make screen non-transparent");
            clear();
            return false;
         }
         sendLogString(m_moduleId, MLOG_STATUS, "transparent screen disabled");
      }
   }

   if (m_option->getUseStdInOut() == true) {
      if (stdInputThreadRunning == false) {
         /* start standard input receiving thread */
         if (glfwCreateThread(stdInputReceivingThreadMain, this) == -1)
            sendLogString(m_moduleId, MLOG_ERROR, "failed to create standard input receiving thread");
      } else {
         assignStdInputInstance(this);
      }
   }

   return true;
}

/* MMDAgent::updateAndRender: update and render the whole scene */
bool MMDAgent::updateAndRender()
{
   int id;
   unsigned int flag;
   static char buf1[MMDAGENT_MAXBUFLEN];
   static char buf2[MMDAGENT_MAXBUFLEN];
   char *errmsg;

   if(m_enable == false)
      return false;

   if (m_threadsPauseFlag == true) {
      /* pausing */
      while (m_message->dequeueLogString(&id, &flag, buf1, buf2) == true)
         procReceivedLogString(id, flag, buf1, buf2);
      /* swap buffer */
      m_screen->swapBuffers();
      return true;
   }

   if (m_content->isRunning()) {
      /* loading content in another thread */
      while (m_message->dequeueLogString(&id, &flag, buf1, buf2) == true)
         procReceivedLogString(id, flag, buf1, buf2);
      /* update */
      if (updateScene() != true)
         return false;
      /* render */
      if (renderScene() != true)
         return false;
      if (m_content->hasFinished()) {
         /* just finished */
         if (m_content->wasError()) {
            /* error */
            sendMessage(m_moduleId, "PROMPT_SHOW", "Failed to fetch content\nPlease retry with internet connection|Reload");
            m_contentInErrorPrompt = true;
         } else {
            /* success */
            if (m_content->needReset()) {
               m_hardResetFlag = true;
               return true;
            }
            if (m_content->getContentMDFFile()) {
               /* get config directory name */
               if (m_configFileName)
                  free(m_configFileName);
               m_configFileName = MMDAgent_strdup(m_content->getContentMDFFile());
               /* if home is under loaded content directory, use it */
               if (m_loadHome) {
                  char* updir = MMDAgent_dirname(m_configFileName);
                  int urlPathStartIdx;
                  char *urlPath = m_content->getHomeURLdup(&urlPathStartIdx);
                  if (urlPath && urlPathStartIdx == 0) {
                     /* this is file home */
                     if (MMDAgent_strheadmatch(urlPath, updir)) {
                        /* use home */
                        if (m_configFileName)
                           free(m_configFileName);
                        m_configFileName = MMDAgent_strdup(urlPath);
                     }
                     free(urlPath);
                  }
                  free(updir);
               }
            } else {
               sendLogString(m_moduleId, MLOG_WARNING, "contents has no mdf file");
               sendMessage(m_moduleId, "PROMPT_SHOW", "no mdf file in content|OK");
            }
         }
      }
      return true;
   }
   if (m_configDirName == NULL) {
      /* get config directory name */
      m_configDirName = MMDAgent_dirname(m_configFileName);
      /* load global decrypt key if exist */
      if (g_enckey)
         delete g_enckey;
      g_enckey = new ZFileKey();
      if (g_enckey->loadKeyDir(m_configDirName) == false) {
         delete g_enckey;
         g_enckey = NULL;
      }
      if (m_contentInErrorPrompt == false) {
         /* load content mdf */
         if (MMDAgent_exist(m_configFileName)) {
            if (m_option->load(m_configFileName, g_enckey, &errmsg)) {
               sendLogString(m_moduleId, MLOG_STATUS, "user mdf file = %s", m_configFileName);
               m_keyvalue->load(m_configFileName, g_enckey);
            } else {
               sendLogString(m_moduleId, MLOG_WARNING, "%s\nfailed to load mdf (%s)", errmsg, m_configFileName);
               sendMessage(m_moduleId, "PROMPT_SHOW", "Failed to load mdf:\n%s|Reload", m_configFileName);
               m_contentInErrorPrompt = true;
            }
         } else {
            sendLogString(m_moduleId, MLOG_WARNING, "missing mdf file to start");
         }
      }
      /* change current directory to that of content */
      if (MMDAgent_chdir(m_configDirName) == false) {
         clear();
         return false;
      }
      sendLogString(m_moduleId, MLOG_STATUS, "current dir changed to %s", m_configDirName);
      /* setup world */
      setupWorld();
      m_content->outputContentInfoToLog(m_configDirName);
      if (m_contentInErrorPrompt == false) {
         /* content launched, save the mdf to history */
         saveContentHistory(m_configFileName, m_configDirName, m_systemConfigFileName);
         /* get and set user context */
         getUserContext(m_configFileName);
      }
      m_timer->setup();
      m_timer->startAdjustment();
      /* render for the first frame before message processing start */
      if (renderScene() != true)
         return false;
      return true;
   }

   if (m_contentInErrorPrompt) {
      while (m_message->dequeueMessage(&id, buf1, buf2) == true) {
         procReceivedMessage(buf1, buf2);
         if (MMDAgent_strequal(buf1, PROMPT_EVENT_SELECTED)) {
            setResetFlag(NULL);
            return true;
         }
      }
      while (m_message->dequeueLogString(&id, &flag, buf1, buf2) == true)
         procReceivedLogString(id, flag, buf1, buf2);
      /* update */
      if (updateScene() != true)
         return false;
      /* render */
      if (renderScene() != true)
         return false;
      return true;
   }

   if (m_plugin && m_pluginStarted == false) {
      /* execute plugin start functions */
      m_plugin->execAppStart(this);
      m_pluginStarted = true;
   }

   /* show doc on first time */
   if (m_contentLaunched == false) {
      if (m_contentDocViewing == false) {
         /* showing content readme */
         if (m_content->showDocOnFirstTime(m_configDirName) == false) {
            /* no startup document is presented, skip */
            m_contentLaunched = true;
            saveContentLastPlayed(m_configFileName);
         } else {
            m_contentDocViewing = true;
         }
      }
      if (m_contentDocViewing == true) {
         /* showing startup document */
         if (updateScene() != true)
            return false;
         if (renderScene() != true)
            return false;
         if (m_infotext->onScreen() == false) {
            /* infotext closed */
            if (m_infotext->getAgreementFlag() == true && MMDAgent_strequal(m_infotext->getSelectedButtonLabel(), "Decline") == true){
               /* declined, delete this and restart with default content */
               MMDAgent_rmdir(m_configDirName);
               char *home = m_content->getHomeURLdup(NULL);
               setResetFlag(home);
               free(home);
            } else {
               /* accepted */
               m_infotext->setAgreementFlag(false);
               m_content->saveLastPlayedTime(m_configDirName);
            }
            m_contentDocViewing = false;
            m_contentLaunched = true;
            saveContentLastPlayed(m_configFileName);
            m_timer->setup();
            m_timer->startAdjustment();
         }
         return true;
      }
   }

   /* check threaded loading status and execute functions if done */
   if (m_threadedLoading)
      m_threadedLoading->update();

   /* check stored message */
   while (m_message->dequeueMessage(&id, buf1, buf2) == true)
      procReceivedMessage(buf1, buf2);
   while (m_message->dequeueLogString(&id, &flag, buf1, buf2) == true)
      procReceivedLogString(id, flag, buf1, buf2);

   /* free log uploader if exists and finished */
   if (m_logUploader != NULL && m_logUploader->isFinished()) {
      delete m_logUploader;
      m_logUploader = NULL;
   }

   /* increment tick count */
   m_tickCount++;

   /* content update check, start after a certain uptime */
   if (m_contentUpdateStarted == false && m_timer->getSystemUpFrame() > MMDAGENT_CONTENTUPDATECHECKDELAYFRAME) {
      m_contentUpdateStarted = true;
      m_content->startCheckUpdate(m_systemDirName);
      /* also, check if content has auto-update */
      char *p = m_content->getContentInfoDup(m_configDirName, "AutoUpdateFiles");
      if (p) {
         char *sec = m_content->getContentInfoDup(m_configDirName, "AutoUpdatePeriod");
         if (sec) {
            if (m_autoUpdateFiles)
               free(m_autoUpdateFiles);
            m_autoUpdateFiles = MMDAgent_strdup(p);
            m_autoUpdatePeriod = MMDAgent_str2double(sec);
            m_timer->start(MMDAGENT_TIMERUPDATE);
            sendLogString(m_moduleId, MLOG_STATUS, "start auto-fetch at every %.1f sec.: %s", (float)m_autoUpdatePeriod, m_autoUpdateFiles);
            free(sec);
         }
         free(p);
      }
   }

   if (m_contentUpdateStarted == true && m_content->processCheckUpdate() == true && getInfoText()->isShowing() == false) {
      /* update check was done, now start checking for immidate system/content updates */
      if (m_contentUpdateChecked == false) {
         m_contentUpdateChecked = true;
         if (requireSystemUpdate() == true) {
            /* system has been updated on server, prompt info text and wait for response */
            getInfoText()->setText("Notice", "!Warning!\n\nSystem update detected on server.\nNow restart in 2 sec.", "OK");
            getInfoText()->setAutoHideFrame(60.0);
            getInfoText()->show();
            m_contentUpdateWait = 1;
         } else if (m_content->currentContentRequireUpdate(m_configDirName) == true) {
            /* currently playing content has been updated, prompt and wait */
            getInfoText()->setText("Notice", "Content update detected.\nNow restart in 2 sec.", "OK");
            getInfoText()->setAutoHideFrame(60.0);
            getInfoText()->show();
            m_contentUpdateWait = 2;
         }
      }
   }
   if (m_contentUpdateWait > 0 && getInfoText()->isShowing() == false) {
      /* update info text was closed, now restart */
      if (m_contentUpdateWait == 1)
         updateCurrentSystem();
      else if (m_contentUpdateWait == 2)
         m_content->restartCurrentUpdate(m_configDirName);
      m_contentUpdateWait = 0;
   }

   if (m_autoUpdateFiles) {
      /* has auto-update file, check and update if reaches timeout */
      if (m_timer->ellapsed(MMDAGENT_TIMERUPDATE) > m_autoUpdatePeriod) {
         sendLogString(m_moduleId, MLOG_STATUS, "%.1f sec has ellapsed, try to fetch %s", (float)m_autoUpdatePeriod, m_autoUpdateFiles);
         if (m_tinyDownload) {
            m_tinyDownload->restart();
         } else {
            m_tinyDownload = new TinyDownload();
            m_tinyDownload->setupAndStart(this, m_moduleId, m_content->getContentInfoDup(m_configDirName, "SourceURL"), m_autoUpdateFiles);
         }
         /* reset timer for next update */
         m_timer->start(MMDAGENT_TIMERUPDATE);
      }
   }

   /* update */
   if(updateScene() != true)
      return false;

   /* render */
   if(renderScene() != true)
      return false;

   return true;
}

/* MMDAgent::updateScene: update the whole scene */
bool MMDAgent::updateScene()
{
   int i, ite;
   double intervalFrame;
   double stepFrame;
   double waitFrame;
   double restFrame;
   double procFrame;
   double adjustFrame;
   double processedFrame;
   MotionPlayer *motionPlayer;
   Button *b;

   if(m_enable == false)
      return false;

   if (m_cameraControlled == false)
      m_render->setCameraFromController(NULL);

   /* get frame interval */
   intervalFrame = m_timer->getFrameInterval();

   if (m_content->isRunning()) {
      return true;
   }

   if (m_holdMotion == true) {
      /* minimal update with no frame advance */
      m_hasExtModel = false;
      for (i = 0; i < m_numModel; i++) {
         if (m_model[i].isEnable() == false) continue;
         if(m_model[i].isMoving() == true) {
            m_model[i].updateRootBone();
            m_model[i].updateMotion(0);
         }
         m_model[i].updateAfterSimulation(m_enablePhysicsSimulation);
         m_model[i].updateSkin();
         if (m_model[i].getPMDModel()->hasExtParam())
            m_hasExtModel = true;
      }
      if (m_menu) m_menu->update(intervalFrame);
      for (b = m_button; b; b = b->getNext())
         b->update(intervalFrame);
      for (i = 0; i < MMDAGENT_MAXNUMCONTENTBUTTONS; i++) {
         if (m_contentButtons[i])
            m_contentButtons[i]->update(intervalFrame);
      }
      if (m_filebrowser) m_filebrowser->update(intervalFrame);
      if (m_prompt) m_prompt->update(intervalFrame);
      if (m_notify) m_notify->update(intervalFrame);
      if (m_infotext) m_infotext->update(intervalFrame);
      if (m_slider) m_slider->update(intervalFrame);
      if (m_tabbar) m_tabbar->update(intervalFrame);
      return true;
   }

   stepFrame = m_stepFrame;
   waitFrame = m_stepFrame * 0.5;
   restFrame = intervalFrame + m_restFrame;
   m_restFrame = 0.0;

   if (restFrame <= waitFrame && m_screen->getVSync() == true) {
      /* skip update and render */
      m_restFrame = restFrame;
      return false;
   }

   processedFrame = 0.0;
   for (ite = 0; ite < m_maxStep; ite++) {
      /* determine frame amount */
      if (restFrame <= stepFrame) {
         if (m_screen->getVSync() == true) {
            if (restFrame > stepFrame * 0.5) {
               /* process one step in advance */
               procFrame = stepFrame;
               m_restFrame = restFrame - stepFrame;
            } else if (restFrame <= stepFrame * 0.5) {
               /* leave for next call */
               m_restFrame = restFrame;
               break;
            } else {
               /* process as is */
               procFrame = restFrame;
            }
         } else {
            /* process as is */
            procFrame = restFrame;
         }
         ite = m_maxStep;
      } else {
         /* process by stepFrame */
         procFrame = stepFrame;
         restFrame -= stepFrame;
      }
      processedFrame += procFrame;
      /* skip world stepping when viewing content doc */
      if (m_contentDocViewing)
         continue;
      /* calculate adjustment time for audio */
      adjustFrame = m_timer->getAdditionalFrame(procFrame);
      /* update motion */
      for (i = 0; i < m_numModel; i++) {
         if (m_model[i].isEnable() == false) continue;
         /* update motion speed */
         if (m_model[i].getMotionManager()->updateMotionSpeedRate(procFrame + adjustFrame)) {
            /* search event in motion */
            for (motionPlayer = m_model[i].getMotionManager()->getMotionPlayerList(); motionPlayer; motionPlayer = motionPlayer->next) {
               if (motionPlayer->accelerationStatusFlag == ACCELERATION_STATUS_ENDED) {
                  /* send message */
                  sendMessage(m_moduleId, MMDAGENT_EVENT_MOTIONACCELERATE, "%s|%s", m_model[i].getAlias(), motionPlayer->name);
               }
            }
         }

         /* look through the last motion status to check if the next motion update needs physics reset */
         for (motionPlayer = m_model[i].getMotionManager()->getMotionPlayerList(); motionPlayer; motionPlayer = motionPlayer->next) {
            if (motionPlayer->statusFlag == MOTION_STATUS_DELETED || motionPlayer->statusFlag == MOTION_STATUS_LOOPED) {
               if (motionPlayer->enableSmooth == false)
                  m_model[i].skipNextSimulation();
            }
         }

         /* update root bone */
         m_model[i].updateRootBone();
         if (m_model[i].updateMotion(procFrame + adjustFrame)) {
            /* search end of motion */
            for (motionPlayer = m_model[i].getMotionManager()->getMotionPlayerList(); motionPlayer; motionPlayer = motionPlayer->next) {
               if (motionPlayer->statusFlag == MOTION_STATUS_DELETED) {
                  /* send message */
                  if (MMDAgent_strequal(motionPlayer->name, LIPSYNC_MOTIONNAME))
                     sendMessage(m_moduleId, MMDAGENT_EVENT_LIPSYNCSTOP, "%s", m_model[i].getAlias());
                  else
                     sendMessage(m_moduleId, MMDAGENT_EVENT_MOTIONDELETE, "%s|%s", m_model[i].getAlias(), motionPlayer->name);
               }
               /* periodically unload unused motion from motion stocker */
               if (motionPlayer->active == false)
                  m_motion->unload(motionPlayer->vmd);
            }
         }
         /* update alpha for appear or disappear */
         if (m_model[i].updateAlpha(procFrame + adjustFrame))
            removeRelatedModels(i); /* remove model and accessories */
      }
      /* execute plugin */
      if (m_plugin)
         m_plugin->execUpdate(this, procFrame + adjustFrame);
      /* update bullet physics */
      m_bullet->update((float) procFrame);
      /* update logger */
      if (m_loggerLog) m_loggerLog->updateStatus(procFrame);
      if (m_loggerMessage) m_loggerMessage->updateStatus(procFrame);

      /* camera motion */
      if (m_cameraControlled == true) {
         if (m_camera.advance(procFrame + adjustFrame) == true && m_camera.getCurrentFrame() == m_camera.getPreviousFrame()) {
            /* reached end */
            m_cameraControlled = false;
         }
         m_render->setCameraFromController(&m_camera);
      }
   }

   /* decrement starting / restarting animation frame */
   /* this is not a realtime-forced animation,.so skip jumping frames for smoothness */
   double prevStartingFrame = m_startingFrame;
   if (processedFrame > m_stepFrame * 5) {
      m_startingFrameSkipCount++;
      if (m_startingFrameSkipCount > 10) {
         /* more than two successive skips for > 0.5 sec, skip animation */
         if (m_resetFlag == false)
            m_startingFrame = 0.0f;
         else
            m_startingFrame = MMDAGENT_STARTANIMATIONFRAME;
         m_startingFrameSkipCount = 0;
      }
   } else {
      m_startingFrameSkipCount = 0;
      if (m_resetFlag == false) {
         if (m_startingFrame != 0.0f) {
            m_startingFrame -= processedFrame;
            if (m_startingFrame < 0.0f)
               m_startingFrame = 0.0f;
         }
      } else {
         if (m_startingFrame != MMDAGENT_STARTANIMATIONFRAME) {
            m_startingFrame += processedFrame;
            if (m_startingFrame > MMDAGENT_STARTANIMATIONFRAME)
               m_startingFrame = MMDAGENT_STARTANIMATIONFRAME;
         }
      }
   }

   if (m_resetFlag == false && prevStartingFrame != 0.0 && m_startingFrame == 0.0f) {
      /* execute just-after-screen-clear functions */
      /* show top buttons */
      if (m_buttonShowing) {
         for (b = m_buttonTop; b; b = b->getMember()) {
            b->show();
            b->resetTimer();
         }
      }
   }

   if (m_contentDocViewing) {
      m_infotext->update(processedFrame);
      return true;
   }

   /* update after simulation */
   m_hasExtModel = false;
   for (i = 0; i < m_numModel; i++)
      if (m_model[i].isEnable() == true) {
         m_model[i].updateAfterSimulation(m_enablePhysicsSimulation);
         m_model[i].updateSkin();
         if (m_model[i].getPMDModel()->hasExtParam())
            m_hasExtModel = true;
         /* capture at this timing if capture is enabled */
         if (m_model[i].doCapture(processedFrame) == false) {
            /* reaches maximum, stop immediately */
            if (m_model[i].stopCapture() == false) {
               sendLogString(m_moduleId, MLOG_ERROR, "motion capture length reaches limit, tried to save, but failed.");
            }
            sendLogString(m_moduleId, MLOG_STATUS, "motion capture length reaches limit, saved to file.");
         }
      }

   /* update stage */
   m_stage->update(processedFrame);

   /* calculate rendering range for shadow mapping */
   if(m_option->getUseShadowMapping())
      m_render->updateDepthTextureViewParam(m_model, m_numModel);

   /* decrement mouse active time */
   m_screen->updateMouseActiveTime(processedFrame);

   /* decrement logger typing mode active time */
   if (m_loggerLog) m_loggerLog->updateTypingActiveTime(processedFrame);
   if (m_loggerMessage) m_loggerMessage->updateTypingActiveTime(processedFrame);

   /* update menu */
   if (m_menu)
      m_menu->update(processedFrame);

   /* update button */
   for (b = m_button; b; b = b->getNext())
      b->update(processedFrame);
   for (i = 0; i < MMDAGENT_MAXNUMCONTENTBUTTONS; i++) {
      if (m_contentButtons[i]) {
         if (m_contentButtons[i]->canDelete()) {
            sendMessage(m_moduleId, MMDAGENT_EVENT_BUTTONDELETE, "%s", m_contentButtons[i]->getName());
            delete m_contentButtons[i];
            m_contentButtons[i] = NULL;
         } else {
            m_contentButtons[i]->update(processedFrame);
         }
      }
   }

   /* update file browser */
   if (m_filebrowser)
      m_filebrowser->update(processedFrame);

   /* update prompter */
   if (m_prompt)
      m_prompt->update(processedFrame);

   /* update notifier */
   if (m_notify)
      m_notify->update(processedFrame);

   /* update infotext */
   if (m_infotext)
      m_infotext->update(processedFrame);

   /* update slider */
   if (m_slider)
      m_slider->update(processedFrame);

#ifdef MMDAGENT_CURRENTTIME
   issueTimeMessage(processedFrame);
#endif

   /* update tabbar */
   if (m_tabbar)
      m_tabbar->update(processedFrame);

   /* update start up message duration time */
   if (m_showUsageFrame > 0.0) {
      m_showUsageFrame -= processedFrame;
      if (m_showUsageFrame < 0.0)
         m_showUsageFrame = 0.0;
   }

   /* update caption */
   if (m_caption)
      m_caption->update(processedFrame);

   return true;
}

/* MMDAgent::renderScene: render the whole scene */
bool MMDAgent::renderScene()
{
   int i;
   btVector3 pos;
   float fps;
   char buff[MMDAGENT_MAXBUFLEN];
   Button *b, *blast;
   bool drawLogEdge;

   if (m_enable == false)
      return false;

   /* update model position and rotation */
   fps = m_timer->getFps();
   for (i = 0; i < m_numModel; i++) {
      if (m_model[i].isEnable() == true) {
         if (m_model[i].updateModelRootOffset(fps))
            sendMessage(m_moduleId, MMDAGENT_EVENT_MOVESTOP, "%s", m_model[i].getAlias());
         if (m_model[i].updateModelRootRotation(fps)) {
            if (m_model[i].isTurning()) {
               sendMessage(m_moduleId, MMDAGENT_EVENT_TURNSTOP, "%s", m_model[i].getAlias());
               m_model[i].setTurningFlag(false);
            } else {
               sendMessage(m_moduleId, MMDAGENT_EVENT_ROTATESTOP, "%s", m_model[i].getAlias());
            }
         }
      }
   }

   /* switch to off-screen rendering frame buffer for following draws to write to the frame buffer */
   if (m_offscreen)
      m_offscreen->start();

   /* update rendering order */
   m_render->getRenderOrder(m_renderOrder, m_model, m_numModel);

   /* render scene */
   m_render->render(m_model, m_renderOrder, m_numModel, m_stage, m_option->getUseMMDLikeCartoon(), m_option->getUseCartoonRendering(), m_option->getLightIntensity(), m_option->getLightDirection(), m_option->getLightColor(), m_option->getUseShadowMapping(), m_option->getShadowMappingTextureSize(), m_option->getShadowMappingSelfDensity(), m_option->getShadowMappingFloorDensity(), m_render->isViewMoving() ? m_timer->ellapsed(MMDAGENT_TIMERMOVE) : 0.0, m_option->getShadowDensity());

   if (m_content->isRunning() == true) {
      /* update font texture for new characters */
      if (m_font)
         m_font->updateGlyphInfo();
      if (m_fontAwesome)
         m_fontAwesome->updateGlyphInfo();
      /* just render content loading status only */
      m_content->updateAndRender();
      if (m_offscreen)
         m_offscreen->finish();
      m_screen->swapBuffers();
      return true;
   }

   if (m_contentDocViewing) {
      /* update font texture for new characters */
      if (m_font)
         m_font->updateGlyphInfo();
      if (m_fontAwesome)
         m_fontAwesome->updateGlyphInfo();
      if (m_tabbar)
         m_tabbar->render();
      m_infotext->render();
      renderAppearingAnimation();
      if (m_offscreen)
         m_offscreen->finish();
      m_screen->swapBuffers();
      return true;
   }

   /* show bullet body */
   if (m_dispBulletBodyFlag)
      m_bullet->debugDisplay();

   /* show log window */
   if (m_loggerLog) m_loggerLog->render();
   if (m_loggerMessage) m_loggerMessage->render();

   /* execute plugin */
   if (m_plugin)
      m_plugin->execRender(this);

   /* count fps */
   m_timer->countFrame();

   /* render debug information */
   float width;
   float height;
   float y_offset;
   float top_space;
   if (m_render->getHeight() < m_render->getWidth()) {
      width = (float)m_render->getWidth() * MMDAGENT_SCREENUNITLENGTH / (float)m_render->getHeight();
      height = MMDAGENT_SCREENUNITLENGTH;
   } else {
      width = MMDAGENT_SCREENUNITLENGTH;
      height = (float)m_render->getHeight() * MMDAGENT_SCREENUNITLENGTH / (float)m_render->getWidth();
   }
   m_elem.textLen = 0; /* reset */
   m_elem.numIndices = 0;
   m_elemAwesome.textLen = 0;
   m_elemAwesome.numIndices = 0;
   m_elemErrorMessage.textLen = 0;
   m_elemErrorMessage.numIndices = 0;
   if (m_tabbar) {
      float r = m_tabbar->getCurrentShowRate();
      y_offset = (1.0f - r) * 0.2f + r * (m_tabbar->getBarHeight() * height + 0.1f);
   } else {
      y_offset = 0.2f;
   }
   top_space = 0.0f;
#ifdef MOBILE_SCREEN
   top_space += 2.0f;
#endif /* MOBILE_SCREEN */

   /* top-left indicator */
   buff[0] = '\0';
   if (m_option->getShowFps() || m_dispLog) {
      /* show fps */
      if (m_screen->getNumMultiSampling() > 0) {
#ifdef MY_LUMINOUS
         if (m_render->getLuminousIntensity() != 0.0f)
            MMDAgent_snprintf(buff, MMDAGENT_MAXBUFLEN, "%5.1ffps %dx MSAA [AL%.1f]", m_timer->getFps(), m_screen->getNumMultiSampling(), m_render->getLuminousIntensity());
         else
            MMDAgent_snprintf(buff, MMDAGENT_MAXBUFLEN, "%5.1ffps %dx MSAA", m_timer->getFps(), m_screen->getNumMultiSampling());
#else
         MMDAgent_snprintf(buff, MMDAGENT_MAXBUFLEN, "%5.1ffps %dx MSAA", m_timer->getFps(), m_screen->getNumMultiSampling());
#endif
      } else {
#ifdef MY_LUMINOUS
         if (m_render->getLuminousIntensity() != 0.0f)
            MMDAgent_snprintf(buff, MMDAGENT_MAXBUFLEN, "%5.1ffps No AA [AL%.1f]", m_timer->getFps(), m_render->getLuminousIntensity());
         else
            MMDAgent_snprintf(buff, MMDAGENT_MAXBUFLEN, "%5.1ffps No AA", m_timer->getFps());
#else
         MMDAgent_snprintf(buff, MMDAGENT_MAXBUFLEN, "%5.1ffps No AA", m_timer->getFps());
#endif
      }
      if (m_option->getUseShadow()) {
         if (m_option->getUseShadowMapping()) {
            strcat(buff, " SM");
         } else {
            strcat(buff, " S");
         }
      }
      if (m_offscreen) {
         float f = m_offscreen->getIntensity();
         if (f != 0.0f) {
            char tmpbuf[20];
            float s = m_offscreen->getScaling();
            MMDAgent_snprintf(tmpbuf, 20, " [DF%.1f|%.1f]", f, s);
            strcat(buff, tmpbuf);
         }
      }
#ifdef __ANDROID__
      /* get api name from audio library */
      strcat(buff, " ");
      strcat(buff, Pa_AndroidGetApiName());
#endif
   }
   if (m_holdMotion) {
      /* show holding message */
      strcat(buff, " <<HOLD>>");
   }
   if (MMDAgent_strlen(buff) > 0) {
      if (m_font == NULL || m_font->getTextDrawElements(buff, &m_elem, m_elem.textLen, MMDAGENT_INDICATOR_OFFSET, height - 1.2f - top_space, 0.0f) == false) {
         m_elem.textLen = 0; /* reset */
         m_elem.numIndices = 0;
      }
   }
   if (MMDAgent_strlen(m_optionalStatusString) > 0) {
      if (m_fontAwesome == NULL || m_fontAwesome->getTextDrawElements(m_optionalStatusString, &m_elemAwesome, m_elemAwesome.textLen, m_elem.width + MMDAGENT_INDICATOR_OFFSET + 1.0f, height - 1.2f - top_space, 0.0f) == false) {
         m_elemAwesome.textLen = 0; /* reset */
         m_elemAwesome.numIndices = 0;
      }
   }
   if (MMDAgent_strlen(m_errorMessages) > 0) {
      if (m_font == NULL || m_font->getTextDrawElementsWithScale(m_errorMessages, &m_elemErrorMessage, m_elemErrorMessage.textLen, MMDAGENT_INDICATOR_OFFSET, height - 1.2f - top_space - 2.0f, 0.0f, 0.95f) == false) {
         m_elemErrorMessage.textLen = 0; /* reset */
         m_elemErrorMessage.numIndices = 0;
      }
   }

   /* top-right indicator */
   if (m_hasExtModel) {
      /* show using ext model mark */
      MMDAgent_snprintf(buff, MMDAGENT_MAXBUFLEN, "PMX");
      if (m_font == NULL || m_font->getTextDrawElementsWithScale(buff, &m_elem, m_elem.textLen, width - 3.0f - MMDAGENT_INDICATOR_OFFSET, height - 1.5f - top_space, 0.0f, 1.5f) == false) {
         m_elem.textLen = 0; /* reset */
         m_elem.numIndices = 0;
      }
   }

   if (m_showUsageFrame > 0.0 && m_argc <= 1 && m_loadHome == false) {
      /* start up message */
      float row = 0.0f;
      if (m_cameraCanMove) {
#ifdef MOBILE_SCREEN
         row += 1.0f;
#else
         row += 3.0f;
#endif
      }
      if (m_dispLog)
         row += 3.0f;
      MMDAgent_snprintf(buff, MMDAGENT_MAXBUFLEN, "[Shift+R] (re)load last played, [Shift+H or tap button] show recent list");
      if (m_font == NULL || m_font->getTextDrawElements(buff, &m_elem, m_elem.textLen, MMDAGENT_INDICATOR_OFFSET, y_offset + 0.4f + 1.1f * row, 0.0f) == false) {
         m_elem.textLen = 0; /* reset */
         m_elem.numIndices = 0;
      }

   }

   /* bottom-left indicators */
   if (m_cameraCanMove) {
      /* camera move instruction */
#ifdef MOBILE_SCREEN
      MMDAgent_snprintf(buff, MMDAGENT_MAXBUFLEN, "- ROTATE: drag  MOVE: double-tap-drag  ZOOM: pinch");
      if (m_font == NULL || m_font->getTextDrawElements(buff, &m_elem, m_elem.textLen, MMDAGENT_INDICATOR_OFFSET, y_offset + 1.1f * (m_dispLog ? 3.0f : 0.0f), 0.0f) == false) {
         m_elem.textLen = 0; /* reset */
         m_elem.numIndices = 0;
      }
#else
      MMDAgent_snprintf(buff, MMDAGENT_MAXBUFLEN, "- ROTATE: drag or arrow-keys");
      if (m_font == NULL || m_font->getTextDrawElements(buff, &m_elem, m_elem.textLen, MMDAGENT_INDICATOR_OFFSET, y_offset + 1.1f * (m_dispLog ? 5.0f : 2.0f), 0.0f) == false) {
         m_elem.textLen = 0; /* reset */
         m_elem.numIndices = 0;
      }
      MMDAgent_snprintf(buff, MMDAGENT_MAXBUFLEN, "- MOVE: double-tap-and-drag, shift+drag or shift+arrows");
      if (m_font == NULL || m_font->getTextDrawElements(buff, &m_elem, m_elem.textLen, MMDAGENT_INDICATOR_OFFSET, y_offset + 1.1f * (m_dispLog ? 4.0f : 1.0f), 0.0f) == false) {
         m_elem.textLen = 0; /* reset */
         m_elem.numIndices = 0;
      }
      MMDAgent_snprintf(buff, MMDAGENT_MAXBUFLEN, "- ZOOM: pinch, wheel or '+' '-' keys");
      if (m_font == NULL || m_font->getTextDrawElements(buff, &m_elem, m_elem.textLen, MMDAGENT_INDICATOR_OFFSET, y_offset + 1.1f * (m_dispLog ? 3.0f : 0.0f), 0.0f) == false) {
         m_elem.textLen = 0; /* reset */
         m_elem.numIndices = 0;
      }
#endif /* MOBILE_SCREEN */
   }
   if (m_dispLog) {
      /* show adjustment time for audio */
      if (m_option->getMotionAdjustTime() > 0.0f)
         MMDAgent_snprintf(buff, MMDAGENT_MAXBUFLEN, "%d msec advance (current motion: %+d)", (int)(m_option->getMotionAdjustTime() * 1000.0f + 0.5f), (int)(m_timer->getCurrentAdjustmentFrame() * 1000.0 / 30.0 + 0.5f));
      else if (m_option->getMotionAdjustTime() < 0.0f)
         MMDAgent_snprintf(buff, MMDAGENT_MAXBUFLEN, "%d msec delay (current motion: %+d)", (int)(m_option->getMotionAdjustTime() * 1000.0f - 0.5f), (int)(m_timer->getCurrentAdjustmentFrame() * 1000.0 / 30.0 - 0.5f));
      else
         MMDAgent_snprintf(buff, MMDAGENT_MAXBUFLEN, "%d msec (current motion: %+d)", (int)(m_option->getMotionAdjustTime() * 1000.0f + 0.5f), (int)(m_timer->getCurrentAdjustmentFrame() * 1000.0 / 30.0 + 0.5f));
      if (m_font == NULL || m_font->getTextDrawElements(buff, &m_elem, m_elem.textLen, MMDAGENT_INDICATOR_OFFSET, y_offset + 1.1f * 2.0f, 0.0f) == false) {
         m_elem.textLen = 0; /* reset */
         m_elem.numIndices = 0;
      }
   }
   if (m_dispLog) {
      /* show camera parameters */
      m_render->getInfoString(buff, MMDAGENT_MAXBUFLEN);
      if (m_font == NULL || m_font->getTextDrawElements(buff, &m_elem, m_elem.textLen, MMDAGENT_INDICATOR_OFFSET, y_offset + 1.1f * 1.0f, 0.0f) == false) {
         m_elem.textLen = 0; /* reset */
         m_elem.numIndices = 0;
      }
   }
   buff[0] = '\0';
   if (m_dispLog) {
      /* show model position */
      for (i = 0; i < m_numModel; i++) {
         if (m_model[i].isEnable() == true) {
            m_model[i].getCurrentPosition(&pos);
            if (MMDAgent_strlen(buff) <= 0)
               MMDAgent_snprintf(buff, MMDAGENT_MAXBUFLEN, "(%.2f, %.2f, %.2f)", pos.x(), pos.y(), pos.z());
            else
               MMDAgent_snprintf(buff, MMDAGENT_MAXBUFLEN, "%s (%.2f, %.2f, %.2f)", buff, pos.x(), pos.y(), pos.z());
         }
      }
   }
   if (MMDAgent_strlen(buff) > 0) {
      if (m_font == NULL || m_font->getTextDrawElements(buff, &m_elem, m_elem.textLen, MMDAGENT_INDICATOR_OFFSET, y_offset + 1.1f * 0.0f, 0.0f) == false) {
         m_elem.textLen = 0; /* reset */
         m_elem.numIndices = 0;
      }
   }

   if (m_logToFile->isLogging() || m_logToFile->getRecordingFlag() || (m_logUploader && m_logUploader->isFinished() == false))
      drawLogEdge = true;
   else
      drawLogEdge = false;

   /* render button */
   blast = NULL;
   for (b = m_button; b; b = b->getNext()) {
      if (b->isAnimating()) {
         if (blast == NULL) {
            blast = b;
            b->renderBegin();
         }
         b->render();
      }
   }
   for (i = 0; i < MMDAGENT_MAXNUMCONTENTBUTTONS; i++) {
      if (m_contentButtons[i]) {
         if (m_contentButtons[i]->isAnimating()) {
            if (blast == NULL) {
               blast = m_contentButtons[i];
               m_contentButtons[i]->renderBegin();
            }
            m_contentButtons[i]->render();
         }
      }
   }
   if (blast)
      blast->renderEnd();

   /* render prompter */
   if (m_prompt)
      m_prompt->render();

   /* render notifier */
   if (m_notify)
      m_notify->render();

   /* render slider */
   if (m_slider)
      m_slider->render();

   /* update font texture for new characters */
   if (m_font)
      m_font->updateGlyphInfo();
   if (m_fontAwesome)
      m_fontAwesome->updateGlyphInfo();

   /* render main */
   if (m_elem.numIndices > 0 || m_elemErrorMessage.numIndices > 0 || drawLogEdge == true || m_plugin->hasRender2D() == true || (m_loggerLog && m_loggerLog->get2dflag() == true) || (m_loggerMessage && m_loggerMessage->get2dflag() == true) || (m_stage && m_stage->hasFrameTexture()) || (m_caption && m_caption->isShowing())) {
      /* start drawing as 2-D screen */
      glDisable(GL_LIGHTING);
      glMatrixMode(GL_PROJECTION);
      glPushMatrix();
      glLoadIdentity();
      MMDAgent_setOrtho(0, width, 0, height, -1, 1);
      glMatrixMode(GL_MODELVIEW);
      glPushMatrix();
      glLoadIdentity();
      glTranslatef(0.0f, 0.0f, 0.0f);
      glEnableClientState(GL_VERTEX_ARRAY);
      /* execute 2D window frame rendering */
      if (m_stage) m_stage->renderFrameTexture2D(width, height);
      if (m_elem.numIndices > 0 || m_elemErrorMessage.numIndices > 0) {
         glEnable(GL_TEXTURE_2D);
         glActiveTexture(GL_TEXTURE0);
         glClientActiveTexture(GL_TEXTURE0);
         if (m_font)
            glBindTexture(GL_TEXTURE_2D, m_font->getTextureID());
         glEnableClientState(GL_TEXTURE_COORD_ARRAY);
         glColor4f(1.0f, 0.0f, 0.0f, 1.0f);
         if (m_elem.numIndices > 0) {
            glVertexPointer(3, GL_FLOAT, 0, m_elem.vertices);
            glTexCoordPointer(2, GL_FLOAT, 0, m_elem.texcoords);
            glDrawElements(GL_TRIANGLES, m_elem.numIndices, GL_INDICES, (const GLvoid*)m_elem.indices);
         }
         if (m_elemErrorMessage.numIndices > 0) {
            glVertexPointer(3, GL_FLOAT, 0, m_elemErrorMessage.vertices);
            glTexCoordPointer(2, GL_FLOAT, 0, m_elemErrorMessage.texcoords);
            glDrawElements(GL_TRIANGLES, m_elemErrorMessage.numIndices, GL_INDICES, (const GLvoid*)m_elemErrorMessage.indices);
         }
         if (m_elemAwesome.numIndices > 0) {
            if (m_fontAwesome)
               glBindTexture(GL_TEXTURE_2D, m_fontAwesome->getTextureID());
            glColor4f(1.0f, 0.8f, 0.0f, 1.0f);
            glVertexPointer(3, GL_FLOAT, 0, m_elemAwesome.vertices);
            glTexCoordPointer(2, GL_FLOAT, 0, m_elemAwesome.texcoords);
            glDrawElements(GL_TRIANGLES, m_elemAwesome.numIndices, GL_INDICES, (const GLvoid *)m_elemAwesome.indices);
         }
         glDisableClientState(GL_TEXTURE_COORD_ARRAY);
         glDisable(GL_TEXTURE_2D);
      }
      /* execute 2D log rendering */
      if (m_loggerLog) m_loggerLog->render2d(width, height, 0);
      if (m_loggerMessage) m_loggerMessage->render2d(width, height, 1);
      /* execute 2D rendering plugin */
      m_plugin->execRender2D(this, width, height);
      if (drawLogEdge) {
         if (m_logToFile->getRecordingFlag())
            glColor4f(1.0f, 0.7f, 0.0f, 1.0f);
         else if (m_logToFile->isLogging())
            glColor4f(0.0f, 0.0f, 0.3f, 1.0f);
         else
            glColor4f(0.0f, 0.4f, 1.0f, 1.0f);
         MMDFiles_drawedge(0.0f, 0.0f, width, height, 0.2f);
      }
      if (m_caption && m_caption->isShowing())
         m_caption->render2D(width, height);
      glDisableClientState(GL_VERTEX_ARRAY);
      /* restore 3-D rendering */
      glPopMatrix();
      glMatrixMode(GL_PROJECTION);
      glPopMatrix();
      glMatrixMode(GL_MODELVIEW);
      glEnable(GL_LIGHTING);
   }

   if (m_dispLog) {
      /* show camera eye point */
      glDisable(GL_LIGHTING);
      glPushMatrix();
      m_render->getCurrentViewCenterPos(&pos);
      glTranslatef(pos.x(), pos.y(), pos.z());
      glColor4f(0.9f, 0.4f, 0.0f, 1.0f);
      glScalef(0.3f, 0.3f, 0.3f);
      glEnableClientState(GL_VERTEX_ARRAY);
      MMDFiles_drawcube();
      glDisableClientState(GL_VERTEX_ARRAY);
      glPopMatrix();
      glEnable(GL_LIGHTING);
   }

   /* show model debug display, comments and error */
   for (i = 0; i < m_numModel; i++) {
      if (m_model[m_renderOrder[i]].isEnable() == true) {
         m_model[m_renderOrder[i]].renderText(m_font, m_dispModelDebug);
         if (m_dispModelDebug)
            m_model[m_renderOrder[i]].renderModelDebug();
      }
   }

   /* render file browser */
   if (m_filebrowser)
      m_filebrowser->render();

   /* render menu */
   if (m_menu)
      m_menu->render();

   /* render tabbar */
   if (m_tabbar)
      m_tabbar->render();

   /* render infotext */
   if (m_infotext)
      m_infotext->render();

   /* render appearing animation */
   renderAppearingAnimation();

   /* process off-line rendering result */
   if (m_offscreen)
      m_offscreen->finish();

   /* swap buffer */
   m_screen->swapBuffers();

   return true;
}

/* MMDAgent::renderAppearingAnimation: render appearing animation */
void MMDAgent::renderAppearingAnimation()
{
   if (m_startingFrame > 0.0f) {
      /* do appearing animation at start up */
      int w, h;
      float width;
      float height;
      getWindowSize(&w, &h);
      if (h > w) {
         width = (float)w * MMDAGENT_SCREENUNITLENGTH / (float)h;
         height = MMDAGENT_SCREENUNITLENGTH;
      } else {
         width = MMDAGENT_SCREENUNITLENGTH;
         height = (float)h * MMDAGENT_SCREENUNITLENGTH / (float)w;
      }
      glDisable(GL_LIGHTING);
      glMatrixMode(GL_PROJECTION);
      glPushMatrix();
      glLoadIdentity();
      MMDAgent_setOrtho(0, width, 0, height, -1, 1);
      glMatrixMode(GL_MODELVIEW);
      glPushMatrix();
      glLoadIdentity();
      glTranslatef(0.0f, 0.0f, 0.0f);
      glEnableClientState(GL_VERTEX_ARRAY);
      renderInterContentTransition(width, height, (float)m_startingFrame, MMDAGENT_STARTANIMATIONFRAME, m_startingFramePattern, m_resetFlag);
      glDisableClientState(GL_TEXTURE_COORD_ARRAY);
      glDisableClientState(GL_VERTEX_ARRAY);
      glPopMatrix();
      glMatrixMode(GL_PROJECTION);
      glPopMatrix();
      glMatrixMode(GL_MODELVIEW);
      glEnable(GL_LIGHTING);
   }
}

/* MMDAgent::renderInterContentTransition: render inter-content transition */
void MMDAgent::renderInterContentTransition(float width, float height, float currentFrame, float maxFrame, int patternId, bool closing)
{
   /* do appearing animation at start up */
   GLfloat v[12];
   v[2] = v[5] = v[8] = v[11] = 0.9f;
   GLindices idx[6] = { 0, 1, 2, 0, 2, 3 };
   float progress, r;
   /* for logo rendering */
   GLfloat logo_vertices[12];
   GLfloat logo_texcoords[8] = { 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f };
   float logoWidth, logoHeight;

   if (m_logoTex) {
      if (width > height) {
         logoHeight = height * CENTER_LOGO_SIZE_RATE;
         logoWidth = logoHeight * (float)(m_logoTex->getWidth()) / (float)(m_logoTex->getHeight());
      } else {
         logoWidth = width * CENTER_LOGO_SIZE_RATE;
         logoHeight = logoWidth * (float)(m_logoTex->getHeight()) / (float)(m_logoTex->getWidth());
      }
      float logoX1 = (width - logoWidth) * 0.5f;
      float logoY1 = (height - logoHeight) * 0.5f;
      float logoX2 = width - logoX1;
      float logoY2 = height - logoY1;
      float logoZ = 1.0f;
      logo_vertices[0] = logo_vertices[3] = logoX1;
      logo_vertices[4] = logo_vertices[7] = logoY1;
      logo_vertices[1] = logo_vertices[10] = logoY2;
      logo_vertices[6] = logo_vertices[9] = logoX2;
      logo_vertices[2] = logo_vertices[5] = logo_vertices[8] = logo_vertices[11] = logoZ;
   }

   progress = (float)currentFrame / maxFrame;

   if (patternId == 0) {
      /* pattern 1 */
      r = progress;
      if (r > 0.8f)
         r = 2.0f;
      else
         r = (r / 0.8f) * 2.0f;
      v[0] = v[9] = 0.0f;
      v[1] = v[4] = 0.0f;
      v[3] = v[6] = width * 0.25f;
      v[7] = v[10] = height * (r - 1.0f);
      glColor4f(0.95f, 0.51f, 0.51f, 1.0f);
      glVertexPointer(3, GL_FLOAT, 0, v);
      glDrawElements(GL_TRIANGLES, 6, GL_INDICES, (const GLvoid *)idx);
      v[0] = v[9] = width * 0.25f;
      v[1] = v[4] = height * (1.0f - (r - 0.75f));
      v[3] = v[6] = width * 0.5f;
      v[7] = v[10] = height;
      glColor4f(0.99f, 0.89f, 0.54f, 1.0f);
      glVertexPointer(3, GL_FLOAT, 0, v);
      glDrawElements(GL_TRIANGLES, 6, GL_INDICES, (const GLvoid *)idx);
      v[0] = v[9] = width * 0.5f;
      v[1] = v[4] = 0.0f;
      v[3] = v[6] = width * 0.75f;
      v[7] = v[10] = height * (r - 0.5f);
      glColor4f(0.92f, 1.00f, 0.82f, 1.0f);
      glVertexPointer(3, GL_FLOAT, 0, v);
      glDrawElements(GL_TRIANGLES, 6, GL_INDICES, (const GLvoid *)idx);
      v[0] = v[9] = width * 0.75f;
      v[1] = v[4] = height * (1.0f - (r - 0.25f));
      v[3] = v[6] = width;
      v[7] = v[10] = height;
      glColor4f(0.58f, 0.88f, 0.83f, 1.0f);
      glVertexPointer(3, GL_FLOAT, 0, v);
      glDrawElements(GL_TRIANGLES, 6, GL_INDICES, (const GLvoid *)idx);
   } else {
      /* pattern 2 and 3 */
      float fshift = 0.1f;
      float fsize = 1.0f - fshift * 5.0f;
      for (int i = 0; i < 6; i++) {
         float rr = progress - fshift * i;
         if (closing == false)
            rr = progress - fshift * (5 - i);
         rr /= fsize;
         float wr;
         if (rr < 0.0f)
            wr = 0.0f;
         else if (rr < 1.0f) {
            rr = rr * (2.0f - rr);
            wr = rr * (2.0f - rr);
         } else
            wr = 1.0f;
         if (patternId == 1) {
            /* pattern 2 */
            static GLfloat cols[6][4] = {
               { 0.29f, 0.23f, 0.27f, 1.0f },
               { 0.24f, 0.43f, 0.45f, 1.0f },
               { 0.44f, 0.68f, 0.44f, 1.0f },
               { 0.75f, 0.93f, 0.38f, 1.0f },
               { 0.96f, 0.45f, 0.23f, 1.0f },
               { 0.96f, 0.95f, 0.96f, 1.0f } };
            v[0] = v[9] = (i + 0.5f - 0.5f * wr) * 0.17f * width;
            v[3] = v[6] = (i + 0.5f + 0.5f * wr) * 0.17f * width;
            v[1] = v[4] = 0.0f;
            v[7] = v[10] = height;
            glColor4f(cols[i][0], cols[i][1], cols[i][2], cols[i][3]);
         } else {
            /* pattern 3 */
            static GLfloat cols[6][4] = {
               { 0.00f, 0.47f, 0.74f, 1.0f },
               { 0.90f, 1.00f, 0.88f, 1.0f },
               { 0.65f, 0.97f, 0.71f, 1.0f },
               { 0.25f, 0.79f, 0.64f, 1.0f },
               { 0.40f, 0.25f, 0.28f, 1.0f },
               { 0.33f, 0.33f, 0.33f, 1.0f } };
            v[0] = v[9] = (0.5f - 0.5f * wr) * width;
            v[3] = v[6] = (0.5f + 0.5f * wr) * width;
            v[1] = v[4] = (1.0f - (i + 1) * 0.17f) * height;
            v[7] = v[10] = (1.0f - i * 0.17f) * height;
            glColor4f(cols[i][0], cols[i][1], cols[i][2], cols[i][3]);
         }
         glVertexPointer(3, GL_FLOAT, 0, v);
         glDrawElements(GL_TRIANGLES, 6, GL_INDICES, (const GLvoid *)idx);
      }
   }
   if (m_logoTex) {
      /* 0.0 from 0 to 0.25, 1.0 from 0.75 to 1.0*/
      float rate = progress * 2.5f - 1.25f;
      if (rate < 0.0f) rate = 0.0f;
      if (rate > 1.0f) rate = 1.0f;
      glEnable(GL_TEXTURE_2D);
      glBindTexture(GL_TEXTURE_2D, m_logoTex->getID());
      glEnableClientState(GL_TEXTURE_COORD_ARRAY);
      glVertexPointer(3, GL_FLOAT, 0, logo_vertices);
      glTexCoordPointer(2, GL_FLOAT, 0, logo_texcoords);
      glColor4f(1.0f, 1.0f, 1.0f, rate);
      glDrawElements(GL_TRIANGLES, 6, GL_INDICES, (const GLvoid *)idx);
      glBindTexture(GL_TEXTURE_2D, 0);
      glDisableClientState(GL_TEXTURE_COORD_ARRAY);
      glDisable(GL_TEXTURE_2D);
   }
}

/* MMDAgent::restoreSurface: restore surface */
void MMDAgent::restoreSurface()
{
   if (m_enable == false)
      return;

   if (m_render)
      m_render->initSurface();
}

/* MMDAgent::resetAdjustmentTimer: reset adjustment timer */
void MMDAgent::resetAdjustmentTimer()
{
   if(m_enable == false)
      return;

   m_timer->setTargetAdjustmentFrame((double) m_option->getMotionAdjustTime() * 30.0f);
   m_timer->startAdjustment();
}

/* MMDAgent::getModuleId: get module id */
int MMDAgent::getModuleId(const char *ident)
{
   if (m_enable == false)
      return 0;

   return m_message->getId(ident);
}

/* MMDAgent::sendMessage: send message to grobal message queue */
void MMDAgent::sendMessage(int id, const char * type, const char * format, ...)
{
   va_list argv;
   char buf[MMDAGENT_MAXBUFLEN];
   char buf2[MMDAGENT_MAXBUFLEN];

   if(m_enable == false)
      return;

   /* dirty filter to prevent audio recording by message other than MMDAgent class... */
   if (id != m_moduleId && MMDAgent_strheadmatch(type, "RECOG_RECORD"))
      return;

   if (format == NULL) {
      m_message->enqueueMessage(id, type, NULL);
      m_message->enqueueLogString(id, MLOG_MESSAGE_SENT, type);
      return;
   }

   va_start(argv, format);
   vsnprintf(buf, MMDAGENT_MAXBUFLEN, format, argv);
   va_end(argv);

   m_message->enqueueMessage(id, type, buf);

   if (MMDAgent_strlen(buf) > 0)
      MMDAgent_snprintf(buf2, MMDAGENT_MAXBUFLEN, "%s|%s", type, buf);
   else
      strcpy(buf2, type);
   m_message->enqueueLogString(id, MLOG_MESSAGE_SENT, buf2);
}

/* MMDAgent::sendLogString: show log string */
void MMDAgent::sendLogString(int id, unsigned int flag, const char * format, ...)
{
   va_list argv;
   char buf[MMDAGENT_MAXBUFLEN];

   if (MMDAgent_strlen(format) <= 0)
      return;

   va_start(argv, format);
   vsnprintf(buf, MMDAGENT_MAXBUFLEN, format, argv);
   va_end(argv);

   if (m_message != NULL)
      m_message->enqueueLogString(id, flag, buf);
}

/* MMDAgent::stopLogString: stop log string */
void MMDAgent::stopLogString()
{
   if (m_enable == false)
      return;

   m_message->setLogSkipFlag(true);
}

/* MMDAgent::resumeLogString: resume log string */
void MMDAgent::resumeLogString()
{
   if (m_enable == false)
      return;

   m_message->setLogSkipFlag(false);
}

/* MMDAgent::findModelAlias: find a model with the specified alias */
int MMDAgent::findModelAlias(const char * alias)
{
   int i;

   if(m_enable == false)
      return -1;

   if(alias)
      for (i = 0; i < m_numModel; i++)
         if (m_model[i].isEnable() && MMDAgent_strequal(m_model[i].getAlias(), alias))
            return i;

   return -1;
}

/* MMDAgent::getMoelList: get model list */
PMDObject *MMDAgent::getModelList()
{
   if(m_enable == false)
      return NULL;

   return m_model;
}

/* MMDAgent::getNumModel: get number of models */
short MMDAgent::getNumModel()
{
   if(m_enable == false)
      return 0;

   return m_numModel;
}

/* MMDAgent::getMousePosition:: get mouse position */
void MMDAgent::getMousePosition(int *x, int *y)
{
   if(m_enable == false)
      return;

   *x = m_mousePosX;
   *y = m_mousePosY;
}

/* MMDAgent::getScreenPointPosition: convert screen position to object position */
void MMDAgent::getScreenPointPosition(btVector3 * dst, btVector3 * src)
{
   if(m_enable == false)
      return;

   m_render->getScreenPointPosition(dst, src);
}

/* MMDAgent::getWindowSize: get window size */
void MMDAgent::getWindowSize(int *w, int *h)
{
   if(m_enable == false)
      return;

   *w = m_screenSize[0];
   *h = m_screenSize[1];
}

/* MMDAgent::getConfigFileName: get config file name for plugin */
char *MMDAgent::getConfigFileName()
{
   if(m_enable == false)
      return NULL;

   return m_configFileName;
}

/* MMDAgent::getConfigDirName: get directory of config file for plugin */
char *MMDAgent::getConfigDirName()
{
   if(m_enable == false)
      return NULL;

   return m_configDirName;
}

/* MMDAgent::getSystemDirName: get system directory name for plugin */
char *MMDAgent::getSystemDirName()
{
   if (m_enable == false)
      return NULL;

   return m_systemDirName;
}

/* MMDAgent::getAppDirName: get application directory name for plugin */
char *MMDAgent::getAppDirName()
{
   if(m_enable == false)
      return NULL;

   return m_appDirName;
}

/* MMDAgent::getTextureFont: get texture font for plugin */
FTGLTextureFont *MMDAgent::getTextureFont()
{
   return m_font;
}

/* MMDAgent::getRenderer: return renderer */
Render *MMDAgent::getRenderer()
{
   return m_render;
}

/* MMDAgent::getMenu: return menu */
Menu *MMDAgent::getMenu()
{
   return m_menu;
}

/* MMDAgent::setButtonsInDir: set buttons in dir */
void MMDAgent::setButtonsInDir(const char *dir)
{
   char buff[MMDAGENT_MAXBUFLEN];
   int i, j;
   Button *blocal, *b, *b2;
   int n = 0;

   m_buttonTop = NULL;
   for (i = 0; i < 10; i++) {
      /* read button defs */
      MMDAgent_snprintf(buff, MMDAGENT_MAXBUFLEN, "%s%cBUTTON%d.txt", dir, MMDAGENT_DIRSEPARATOR, i);
      if (MMDAgent_exist(buff) == false)
         continue;
      b = new Button;
      if (b->load(this, m_moduleId, buff) == false) {
         delete b;
         continue;
      }
      /* link to global */
      b->setNext(m_button);
      m_button = b;
      /* link to local */
      b->setMember(m_buttonTop);
      m_buttonTop = b;
      n++;
      /* find child */
      blocal = NULL;
      for (j = 0; j < 10; j++) {
         MMDAgent_snprintf(buff, MMDAGENT_MAXBUFLEN, "%s%cBUTTON%d-%d.txt", dir, MMDAGENT_DIRSEPARATOR, i, j);
         if (MMDAgent_exist(buff) == false)
            continue;
         b2 = new Button;
         if (b2->load(this, m_moduleId, buff, b) == false) {
            delete b2;
            continue;
         }
         /* link to global */
         b2->setNext(m_button);
         m_button = b2;
         /* link to local */
         b2->setMember(blocal);
         blocal = b2;
         n++;
      }
      b->setChild(blocal);
   }
   sendLogString(m_moduleId, MLOG_STATUS, "loaded %d buttons", n);

   m_buttonShowing = m_option->getShowButton();
}

/* MMDAgent::pointedButton: return pointed button */
Button *MMDAgent::pointedButton(int x, int y, int screenWidth, int screenHeight)
{
   Button *b, *b2;

   /* content buttons */
   for (int i = 0; i < MMDAGENT_MAXNUMCONTENTBUTTONS; i++) {
      if (m_contentButtons[i]) {
         b = m_contentButtons[i];
         if (b->isShowing() == false) continue;
         if (b->isPointed(x, y, screenWidth, screenHeight))
            return b;
      }
   }

   /* app buttons */
   for (b = m_button; b; b = b->getNext()) {
      if (b->isShowing() == false) continue;
      if (b->isPointed(x, y, screenWidth, screenHeight)) {
         if (b->getChild()) {
            for (b2 = b->getChild(); b2; b2 = b2->getMember()) {
               if (b2->isShowing())
                  b2->hide();
               else
                  b2->show();
            }
            return b;
         }
         return b;
      }
   }

   return NULL;
}

/* MMDAgent::toggleButtons: show/hide toggle buttons */
void MMDAgent::toggleButtons()
{
   Button *b;

   if (m_buttonShowing == false) {
      /* show top buttons */
      for (b = m_buttonTop; b; b = b->getMember())
         b->show();
      m_buttonShowing = true;
   } else {
      /* hide all buttons */
      for (b = m_button; b; b = b->getNext())
         if (b->isShowing())
            b->hide();
      m_buttonShowing = false;
   }
}

/* MMDAgent::getFileBrowser: return file browser */
FileBrowser *MMDAgent::getFileBrowser()
{
   return m_filebrowser;
}

/* MMDAgent::getPrompt: return prompt */
Prompt *MMDAgent::getPrompt()
{
   return m_prompt;
}

/* MMDAgent::getInfoText: return infotext */
InfoText *MMDAgent::getInfoText()
{
   return m_infotext;
}

/* MMDAgent::getSlider: return slider */
Slider *MMDAgent::getSlider()
{
   return m_slider;
}

/* MMDAgent::getTabbar: return tabbar */
Tabbar *MMDAgent::getTabbar()
{
   return m_tabbar;
}

/* MMDAgent::requireSystemUpdate: check if system update is required */
bool MMDAgent::requireSystemUpdate()
{
   char buff[MMDAGENT_MAXBUFLEN];
   KeyValue *prop;
   bool ret;

   if (m_systemDirName == NULL)
      return false;

   ret = false;
   MMDAgent_snprintf(buff, MMDAGENT_MAXBUFLEN, "%s%c%s", m_systemDirName, MMDAGENT_DIRSEPARATOR, MMDAGENT_CONTENTINFOFILE);
   prop = new KeyValue;
   prop->setup();
   if (prop->load(buff, NULL) && prop->exist("LastModifiedEpochTime")) {
      if (atoll(prop->getString("LastModifiedEpochTime", "0")) > atoll(prop->getString("ExtractedEpochTime", "0"))) {
         ret = true;
      }
   }
   delete prop;
   return ret;
}

/* callback for menu handling */
static void menuHandler(int id, int item, void *data)
{
   MMDAgent *mmdagent = (MMDAgent *)data;
   mmdagent->procMenu(id, item);
}

/* MMDAgent::procMenu: process menu */
void MMDAgent::procMenu(int id, int item)
{
   if (m_menu->find("[Action]") == id) {
      switch (item) {
      case 0: /* toggle camera */
         procToggleCameraMoveMessage();
         getMenu()->hide();
         break;
      case 1: /* reset camera position */
         procMoveResetMessage();
         getMenu()->hide();
         break;
#if 0
      case 2: /* Show Buttons */
         toggleButtons();
         getMenu()->hide();
         break;NOTIFY_COMMAND_SHOW
#endif
      case 2: /* Show Files */
         if (getFileBrowser()) {
            if (getFileBrowser()->isShowing()) {
               getFileBrowser()->hide();
            } else {
               getFileBrowser()->show();
            }
            getMenu()->hide();
         }
         break;
      case 4: /* Set Home */
         m_content->setHomeFile(m_configFileName);
         m_notify->processMessage(NOTIFY_COMMAND_SHOW, "Saved this content as home");
         getMenu()->hide();
         break;
      case 5: /* Clear Home */
         m_content->clearHome();
         m_notify->processMessage(NOTIFY_COMMAND_SHOW, "Cleared home");
         getMenu()->hide();
         break;
      case 6: /* Reload */
         setResetFlag(NULL);
         break;
      }
   } else if (m_menu->find("[View]") == id) {
      switch (item) {
      case 0: /* log window */
         procDisplayLogMessage();
         if (m_dispLog)
            m_menu->setItemLabel(id, 0, "\xE2\x97\x89 Debug log");
         else
            m_menu->setItemLabel(id, 0, "\xE2\x97\x8b Debug log");
         break;
      case 1: /* Status */
         procInfoStringMessage();
         if (m_option->getShowFps() == true)
            m_menu->setItemLabel(id, 1, "\xE2\x97\x89 Top indicator");
         else
            m_menu->setItemLabel(id, 1, "\xE2\x97\x8b Top indicator");
         break;
      case 2: /* shadow */
         procShadowMessage();
         if (m_option->getUseShadow() == true)
            m_menu->setItemLabel(id, 2, "\xE2\x97\x89 Shadow");
         else
            m_menu->setItemLabel(id, 2, "\xE2\x97\x8b Shadow");
         break;
      case 3: /* ShadowMapping */
#ifndef MOBILE_SCREEN
         procShadowMappingMessage();
         if (m_option->getUseShadowMapping() == true)
            m_menu->setItemLabel(id, 3, "\xE2\x97\x89 ShadowMapping");
         else
            m_menu->setItemLabel(id, 3, "\xE2\x97\x8b ShadowMapping");
#endif /* MOBILE_SCREEN */
         break;
      case 4: /* Bone */
         procDisplayBoneMessage();
         if (m_dispModelDebug)
            m_menu->setItemLabel(id, 4, "\xE2\x97\x89 Bone");
         else
            m_menu->setItemLabel(id, 4, "\xE2\x97\x8b Bone");
         break;
      case 5: /* Wire */
#ifndef MMDAGENT_DONTRENDERDEBUG
         procDisplayWireMessage();
         GLint polygonMode[2];
         glGetIntegerv(GL_POLYGON_MODE, polygonMode);
         if (polygonMode[1] == GL_LINE)
            m_menu->setItemLabel(id, 5, "\xE2\x97\x89 Wire");
         else
            m_menu->setItemLabel(id, 5, "\xE2\x97\x8b Wire");
#endif /* MMDAGENT_DONTRENDERDEBUG */
         break;
      case 6: /* RigidBody */
         procDisplayRigidBodyMessage();
         if (m_dispBulletBodyFlag)
            m_menu->setItemLabel(id, 6, "\xE2\x97\x89 RigidBody");
         else
            m_menu->setItemLabel(id, 6, "\xE2\x97\x8b RigidBody");
         break;
      case 7: /* AutoLuminous */
#ifdef MY_LUMINOUS
         procLuminousMessage();
#endif
         break;
      }
#ifndef MOBILE_SCREEN
   } else if (m_menu->find("[Window]") == id) {
      switch (item) {
      case 0: /* Fullscreen */
         procFullScreenMessage();
         if (m_option->getFullScreen() == true)
            m_menu->setItemLabel(id, 0, "\xE2\x97\x89 Fullscreen");
         else
            m_menu->setItemLabel(id, 0, "\xE2\x97\x8b Fullscreen");
         break;
         getMenu()->hide();
      case 1: /* Hide window bar */
         procToggleTitleBarMessage();
         if (m_screen->getTitleBarHide() == true)
            m_menu->setItemLabel(id, 1, "\xE2\x97\x89 Hide title bar");
         else
            m_menu->setItemLabel(id, 1, "\xE2\x97\x8b Hide title bar");
         break;
      case 2: /* Save window position */
         procSaveWindowPlacementMessage();
         break;
      case 3: /* Restore window position */
         procLoadWindowPlacementMessage();
         break;
      case 4: /* Open FST with Editor */
         procOpenContentFileMessage();
         break;
      case 5: /* Open with Explorer */
         procOpenContentDirMessage();
         break;
      case 6: /* Open External log window */
         procDisplayLogConsoleMessage();
         break;
      }
#endif /* MOBILE_SCREEN */
   } else if (m_menu->find("[System]"), id) {
      switch (item) {
      case 0: /* Toggle VSync */
#ifndef MOBILE_SCREEN
         procVSyncMessage();
         if (m_screen->getVSync() == true)
            m_menu->setItemLabel(id, 0, "\xE2\x97\x89 VSync");
         else
            m_menu->setItemLabel(id, 0, "\xE2\x97\x8b VSync");
#endif /* MOBILE_SCREEN */
         break;
      case 2: /* Force AppData update */
         updateCurrentSystem();
         break;
      case 3: /* Clear favorites */
         deleteFavorites();
         break;
      case 4: /* clear cache */
         deleteContents();
         break;
      case 5: /* clear all data */
         deleteAllContentsInCache();
         setResetFlag(NULL);
         break;
#if 0
      case 6: /* try glMapBuffer */
         for (i = 0; i < getNumModel(); i++)
            getModelList()[i].getPMDModel()->setRenderingVBOFunction(PMDMODEL_VBO_MAPBUFFER);
         break;
      case 7: /* try glMapBufferRange */
         for (i = 0; i < getNumModel(); i++)
            getModelList()[i].getPMDModel()->setRenderingVBOFunction(PMDMODEL_VBO_MAPBUFFERRANGE);
         break;
      case 8: /* try glBufferData */
         for (i = 0; i < getNumModel(); i++)
            getModelList()[i].getPMDModel()->setRenderingVBOFunction(PMDMODEL_VBO_BUFFERDATA);
         break;
#endif
      }
   }
}

#define MENUKEY(A) (mobile ? NULL : A)

/* MMDAgent::createMenu: create menu */
void MMDAgent::createMenu()
{
   int id;
#ifdef MOBILE_SCREEN
   bool mobile = true;
#else
   bool mobile = false;
#endif

   if (m_menu == NULL)
      return;

#ifndef MOBILE_SCREEN
   id = m_menu->add("[Window]", MENUPRIORITY_SYSTEM, menuHandler, this);
   if (m_option->getFullScreen() == true)
      m_menu->setItem(id, 0, "\xE2\x97\x89 Fullscreen", NULL, NULL, NULL, MENUKEY("<F>"));
   else
      m_menu->setItem(id, 0, "\xE2\x97\x8b Fullscreen", NULL, NULL, NULL, MENUKEY("<F>"));
   if (m_screen->getTitleBarHide() == true)
      m_menu->setItem(id, 1, "\xE2\x97\x89 Hide title bar", NULL, NULL, NULL, NULL);
   else
      m_menu->setItem(id, 1, "\xE2\x97\x8b Hide title bar", NULL, NULL, NULL, NULL);
   m_menu->setItem(id, 2, "Save position", NULL, NULL, NULL, NULL);
   m_menu->setItem(id, 3, "Load position", NULL, NULL, NULL, NULL);
   m_menu->setItem(id, 4, "Open FST Editor...", NULL, NULL, NULL, MENUKEY("<E>"));
   m_menu->setItem(id, 5, "Open Explorer...", NULL, NULL, NULL, MENUKEY("<Shift+E>"));
   m_menu->setItem(id, 6, "Open External log...", NULL, NULL, NULL, MENUKEY("<Shift+D>"));
#endif /* MOBILE_SCREEN */

   id = m_menu->add("[View]", MENUPRIORITY_SYSTEM, menuHandler, this);
   if (m_dispLog)
      m_menu->setItem(id, 0, "\xE2\x97\x89 Debug log", NULL, NULL, NULL, MENUKEY("<D>"));
   else
      m_menu->setItem(id, 0, "\xE2\x97\x8b Debug log", NULL, NULL, NULL, MENUKEY("<D>"));
   if (m_option->getShowFps() == true)
      m_menu->setItem(id, 1, "\xE2\x97\x89 Top indicator", NULL, NULL, NULL, MENUKEY("<S>"));
   else
      m_menu->setItem(id, 1, "\xE2\x97\x8b Top indicator", NULL, NULL, NULL, MENUKEY("<S>"));
   if (m_option->getUseShadow() == true)
      m_menu->setItem(id, 2, "\xE2\x97\x89 Shadow", NULL, NULL, NULL, MENUKEY("<Shift+S>"));
   else
      m_menu->setItem(id, 2, "\xE2\x97\x8b Shadow", NULL, NULL, NULL, MENUKEY("<Shift+S>"));
#ifdef MOBILE_SCREEN
   m_menu->setItem(id, 3, "\xE2\x97\x8b ShadowMapping", NULL, NULL, NULL, NULL);
   m_menu->setItemStatus(id, 3, MENUITEM_STATUS_DISABLED);
#else
   if (m_option->getUseShadowMapping() == true)
      m_menu->setItem(id, 3, "\xE2\x97\x89 ShadowMapping", NULL, NULL, NULL, MENUKEY("<X>"));
   else
      m_menu->setItem(id, 3, "\xE2\x97\x8b ShadowMapping", NULL, NULL, NULL, MENUKEY("<X>"));
#endif
   m_menu->setItem(id, 4, "\xE2\x97\x8b Bone", NULL, NULL, NULL, MENUKEY("<B>"));
   m_menu->setItem(id, 5, "\xE2\x97\x8b Wire", NULL, NULL, NULL, MENUKEY("<W>"));
#ifdef MMDAGENT_DONTRENDERDEBUG
   m_menu->setItemStatus(id, 5, MENUITEM_STATUS_DISABLED);
#endif /* MMDAGENT_DONTRENDERDEBUG */
   if (m_dispBulletBodyFlag)
      m_menu->setItem(id, 6, "\xE2\x97\x89 RigidBody", NULL, NULL, NULL, MENUKEY("<Shift+W>"));
   else
      m_menu->setItem(id, 6, "\xE2\x97\x8b RigidBody", NULL, NULL, NULL, MENUKEY("<Shift+W>"));
   m_menu->setItem(id, 7, "\xE2\x87\x86 AutoLuminous", NULL, NULL, NULL, MENUKEY("<Shift+L>"));
#ifndef MY_LUMINOUS
   m_menu->setItemStatus(id, 7, MENUITEM_STATUS_DISABLED);
#endif

   id = m_menu->add("[Action]", MENUPRIORITY_SYSTEM, menuHandler, this);
   m_menu->setItem(id, 0, "Toggle camera move", NULL, NULL, NULL, MENUKEY("<C>"));
   m_menu->setItem(id, 1, "Reset camera", NULL, NULL, NULL, MENUKEY("<Shift+C>"));
//   m_menu->setItem(id, 2, "Button Show/Hide", NULL, NULL, NULL, MENUKEY("<Q>"));
   m_menu->setItem(id, 2, "Browse content", NULL, NULL, NULL, MENUKEY("<Shift+O>"));
   m_menu->setItem(id, 4, "\xE2\x87\xA8 Set current as Home", NULL, NULL, NULL, NULL);
   m_menu->setItem(id, 5, "\xE2\x87\xA8 Clear Home", NULL, NULL, NULL, NULL);
   m_menu->setItem(id, 6, "\xE2\x87\xA8 Reload", NULL, NULL, NULL, MENUKEY("<Shift+R>"));

   id = m_menu->add("[System]", MENUPRIORITY_DEVELOP, menuHandler, this);
#ifdef MOBILE_SCREEN
   m_menu->setItem(id, 0, "\xE2\x97\x8b VSync", NULL, NULL, NULL, MENUKEY("<Shift+V>"));
   m_menu->setItemStatus(id, 0, MENUITEM_STATUS_DISABLED);
#else
   if (m_screen->getVSync() == true)
      m_menu->setItem(id, 0, "\xE2\x97\x89 VSync", NULL, NULL, NULL, MENUKEY("<Shift+V>"));
   else
      m_menu->setItem(id, 0, "\xE2\x97\x8b VSync", NULL, NULL, NULL, MENUKEY("<Shift+V>"));
#endif
   m_menu->setItem(id, 2, "\xE2\x9A\xA0 Check for system updates", NULL, NULL, NULL);
   m_menu->setItem(id, 3, "\xE2\x9A\xA0 Delete bookmark", NULL, NULL, NULL);
   m_menu->setItem(id, 4, "\xE2\x9A\xA0 Delete content cache", NULL, NULL, NULL);
   m_menu->setItem(id, 5, "\xE2\x9A\xA0 Clear system and content", NULL, NULL, NULL);
#if 0
   m_menu->setItem(id, 6, "<OpenGLDebug> try MapBuffer", NULL, NULL, NULL);
   m_menu->setItem(id, 7, "<OpenGLDebug> try MapBufferRange", NULL, NULL, NULL);
   m_menu->setItem(id, 8, "<OpenGLDebug> try BufferData", NULL, NULL, NULL);
#endif
}

/* MMDAgent::updateCurrentSystem: update current system */
void MMDAgent::updateCurrentSystem()
{
   char buff[MMDAGENT_MAXBUFLEN];
   KeyValue *prop;

   if (m_systemDirName == NULL)
      return;

   MMDAgent_snprintf(buff, MMDAGENT_MAXBUFLEN, "%s%c%s", m_systemDirName, MMDAGENT_DIRSEPARATOR, MMDAGENT_CONTENTINFOFILE);
   prop = new KeyValue;
   prop->setup();
   if (prop->load(buff, NULL) && prop->exist("DownloadCompleted")) {
      /* temporary reset "DownloadCompleted" to force update check at next start */
      prop->setString("DownloadCompleted", "false");
      prop->save(buff);
      /* flag for restart */
      setResetFlag(NULL);
   }
   delete prop;
}

/* MMDAgent::deleteContents: delete contents in cache */
void MMDAgent::deleteContents()
{
   char buff[MMDAGENT_MAXBUFLEN];
   char buff2[MMDAGENT_MAXBUFLEN];
   char *contentDirName;
   DIRECTORY *d;

   /* get content directory name */
   contentDirName = MMDAgent_contentdirdup();
   if (contentDirName == NULL)
      return;

   d = MMDAgent_opendir(contentDirName);
   if (d == NULL)
      return;
   while (MMDAgent_readdir(d, buff) == true) {
      if (buff[0] != '.' && buff[0] != '_') {
         MMDAgent_snprintf(buff2, MMDAGENT_MAXBUFLEN, "%s%c%s", contentDirName, MMDAGENT_DIRSEPARATOR, buff);
         MMDAgent_rmdir(buff2);
      }
   }
   MMDAgent_closedir(d);
   return;
}

/* MMDAgent::resetHome: reset home */
void MMDAgent::resetHome()
{
   char *contentDirName;
   char buff[MMDAGENT_MAXBUFLEN];

   contentDirName = MMDAgent_contentdirdup();
   if (contentDirName == NULL)
      return;
   MMDAgent_snprintf(buff, MMDAGENT_MAXBUFLEN, "%s%c%s", contentDirName, MMDAGENT_DIRSEPARATOR, CONTENTMANAGER_HOMEFILE);
   MMDAgent_removefile(buff);
   free(contentDirName);
}

/* MMDAgent::deleteFavorite: delete favorites */
void MMDAgent::deleteFavorites()
{
   char *contentDirName;
   char buff[MMDAGENT_MAXBUFLEN];

   contentDirName = MMDAgent_contentdirdup();
   if (contentDirName == NULL)
      return;
   MMDAgent_snprintf(buff, MMDAGENT_MAXBUFLEN, "%s%c%s", contentDirName, MMDAGENT_DIRSEPARATOR, CONTENTMANAGER_BOOKMARKFILE);
   MMDAgent_removefile(buff);
   free(contentDirName);

   /* also delete home */
   resetHome();
}

/* MMDAgent::hasReadme: return if current content has readme */
bool MMDAgent::hasReadme()
{
   KeyValue *prop;
   char buff[MMDAGENT_MAXBUFLEN];
   bool flag = false;

   /* consult content information cache (web content) */
   prop = m_content->getContentInfoNew(m_configDirName);
   if (prop) {
      if (prop->exist("Readme")) {
         MMDAgent_snprintf(buff, MMDAGENT_MAXBUFLEN, "%s%c%s", m_configDirName, MMDAGENT_DIRSEPARATOR, prop->getString("Readme", ""));
         if (MMDAgent_exist(buff))
            flag = true;
      }
      delete prop;
   }

   /* else, consult local package info file (local content) */
   if (flag == false) {
      prop = new KeyValue;
      prop->setup();
      MMDAgent_snprintf(buff, MMDAGENT_MAXBUFLEN, "%s%c%s", m_configDirName, MMDAGENT_DIRSEPARATOR, CONTENTMANAGER_PACKAGEFILE);
      if (prop->load(buff, g_enckey) == true) {
         if (prop->exist("readme")) {
            MMDAgent_snprintf(buff, MMDAGENT_MAXBUFLEN, "%s%c%s", m_configDirName, MMDAGENT_DIRSEPARATOR, prop->getString("readme", ""));
            if (MMDAgent_exist(buff))
               flag = true;
         }
      }
      delete prop;
   }

   return flag;
}

/* MMDAgent::showReadme: show readme */
void MMDAgent::showReadme()
{
   KeyValue *prop;
   char buff[MMDAGENT_MAXBUFLEN];

   /* consult content information cache (web content) */
   prop = m_content->getContentInfoNew(m_configDirName);
   if (prop) {
      if (prop->exist("Readme")) {
         MMDAgent_snprintf(buff, MMDAGENT_MAXBUFLEN, "%s%c%s", m_configDirName, MMDAGENT_DIRSEPARATOR, prop->getString("Readme", ""));
         if (MMDAgent_exist(buff) == false) {
            sendLogString(m_moduleId, MLOG_ERROR, "showReadme: not found: %s", buff);
            return;
         }
         if (m_infotext->load(buff) == false) {
            sendLogString(m_moduleId, MLOG_ERROR, "showReadme: cannot load %s", buff);
            return;
         }
         m_infotext->show();
      }
      delete prop;
      return;
   }

   /* else, consult local package info file (local content) */
   prop = new KeyValue;
   prop->setup();
   MMDAgent_snprintf(buff, MMDAGENT_MAXBUFLEN, "%s%c%s", m_configDirName, MMDAGENT_DIRSEPARATOR, CONTENTMANAGER_PACKAGEFILE);
   if (prop->load(buff, g_enckey) == true) {
      if (prop->exist("readme")) {
         MMDAgent_snprintf(buff, MMDAGENT_MAXBUFLEN, "%s%c%s", m_configDirName, MMDAGENT_DIRSEPARATOR, prop->getString("readme", ""));
         if (MMDAgent_exist(buff) == false) {
            sendLogString(m_moduleId, MLOG_ERROR, "showReadme: not found: %s", buff);
            return;
         }
         if (m_infotext->load(buff) == false) {
            sendLogString(m_moduleId, MLOG_ERROR, "showReadme: cannot load %s", buff);
            return;
         }
         m_infotext->show();
      }
   }
   delete prop;

   return;
}

/* MMDAgent::deleteAllContentsInCache delete all data in cache */
void MMDAgent::deleteAllContentsInCache()
{
   char buff[MMDAGENT_MAXBUFLEN];
   char buff2[MMDAGENT_MAXBUFLEN];
   char *contentDirName;
   DIRECTORY *d;
   MMDAGENT_STAT s;

   /* get content directory name */
   contentDirName = MMDAgent_contentdirdup();
   if (contentDirName == NULL)
      return;

   d = MMDAgent_opendir(contentDirName);
   if (d == NULL)
      return;
   while (MMDAgent_readdir(d, buff) == true) {
      if (buff[0] != '.') {
         MMDAgent_snprintf(buff2, MMDAGENT_MAXBUFLEN, "%s%c%s", contentDirName, MMDAGENT_DIRSEPARATOR, buff);
         s = MMDAgent_stat(buff2);
         if (s == MMDAGENT_STAT_DIRECTORY)
            MMDAgent_rmdir(buff2);
         else if (s == MMDAGENT_STAT_NORMAL)
            MMDAgent_removefile(buff2);
      }
   }
   MMDAgent_closedir(d);
   return;
}


/* MMDAgent::getKeyValue: get key-value instance for plugin */
KeyValue *MMDAgent::getKeyValue()
{
   return m_keyvalue;
}

/* MMDAgent::getModuleName: get module name from module id for plugin */
const char *MMDAgent::getModuleName(int module_id)
{
   return m_message->getIdString(module_id);
}

/* MMDAgent::setResetFlag: set reset flag */
void MMDAgent::setResetFlag(const char *argv)
{
   int i, n;
   char **newArgv;
   int newArgc;

   if(m_argc < 1)
      return;

   if (argv == NULL) {
      /* keep arguments */
      m_resetFlag = true;
      return;
   }

   /* check if arguments include .mdf or .mmda or url */
   for (n = m_argc - 1; n >= 1; n--) {
      if (MMDAgent_strtailmatch(m_argv[n], ".mmda"))
         break;
   }
   if (n < 1) {
      for (n = m_argc - 1; n >= 1; n--) {
         if (MMDAgent_strheadmatch(m_argv[n], "http://") || MMDAgent_strheadmatch(m_argv[n], "https://") || MMDAgent_strheadmatch(m_argv[n], "mmdagent://"))
            break;
      }
   }
   if (n < 1) {
      for (n = m_argc - 1; n >= 1; n--) {
         if (MMDAgent_strtailmatch(m_argv[n], ".mdf"))
            break;
      }
   }
   if (n < 1) {
      /* no contents nor .mdf (playing default content) */
      if (MMDAgent_strlen(argv) > 0) {
         /* append given argument */
         newArgv = (char **)malloc(sizeof(char *) * (m_argc + 1));
         for (i = 0; i < m_argc; i++)
            newArgv[i] = MMDAgent_strdup(m_argv[i]);
         newArgv[m_argc] = MMDAgent_strdup(argv);
         newArgc = m_argc + 1;
      } else {
         /* keep default */
         newArgv = (char **)malloc(sizeof(char *) * m_argc);
         for (i = 0; i < m_argc; i++)
            newArgv[i] = MMDAgent_strdup(m_argv[i]);
         newArgc = m_argc;
      }
   } else {
      /* has contents or .mdf (playing specified content) */
      if (MMDAgent_strlen(argv) > 0) {
         /* replace last with given argument */
         newArgv = (char **)malloc(sizeof(char *) * m_argc);
         for (i = 0; i < m_argc; i++) {
            if (i == n)
               newArgv[i] = MMDAgent_strdup(argv);
            else
               newArgv[i] = MMDAgent_strdup(m_argv[i]);
         }
         newArgc = m_argc;
      } else {
         /* delete all .mdf and contents and fallback into default */
         n = 1;
         for (i = 1; i < m_argc; i++) {
            if (!(MMDAgent_strtailmatch(m_argv[i], ".mmda") || MMDAgent_strtailmatch(m_argv[i], ".mdf") || MMDAgent_strheadmatch(m_argv[i], "http://") || MMDAgent_strheadmatch(m_argv[i], "https://") || MMDAgent_strheadmatch(m_argv[i], "mmdagent://")))
               n++;
         }
         newArgv = (char **)malloc(sizeof(char *) * n);
         newArgv[0] = MMDAgent_strdup(m_argv[0]);
         n = 1;
         for (i = 1; i < m_argc; i++) {
            if (!(MMDAgent_strtailmatch(m_argv[i], ".mmda") || MMDAgent_strtailmatch(m_argv[i], ".mdf") || MMDAgent_strheadmatch(m_argv[i], "http://") || MMDAgent_strheadmatch(m_argv[i], "https://") || MMDAgent_strheadmatch(m_argv[i], "mmdagent://"))) {
               newArgv[n] = MMDAgent_strdup(m_argv[i]);
               n++;
            }
         }
         newArgc = n;
      }
   }

   for(i = 0; i < m_argc; i++)
      free(m_argv[i]);
   free(m_argv);

   m_argv = newArgv;
   m_argc = newArgc;

   if (++m_startingFramePattern >= MMDAGENT_STARTANIMATIONPATTERNNUM)
      m_startingFramePattern = 0;

   m_resetFlag = true;
}

/* MMDAgent::getResetFlag: return reset flag */
bool MMDAgent::getResetFlag()
{
   if (m_startingFrame == MMDAGENT_STARTANIMATIONFRAME)
      return m_resetFlag;
   return false;
}

/* MMDAgent::getHardResetFlag: return hard reset flag */
bool MMDAgent::getHardResetFlag()
{
   return m_hardResetFlag;
}

/* MMDAgent::reload: reload current content */
void MMDAgent::reload()
{
   char buff[MMDAGENT_MAXBUFLEN];

   if (m_enable == false)
      return;

   if (m_argc > 1 || m_loadHome == true) {
      /* if argv has some specific content or loadHome is enabled, */
      /* set reset flag to load the current content */
      setResetFlag(m_configFileName);
   } else {
      /* load the last entry in the history */
      char *historyFile = getHistoryFilePathDup();
      if (historyFile) {
         KeyValue *v;
         const char *mdfName = NULL;
         v = new KeyValue;
         v->setup();
         v->load(historyFile, NULL);
         for (int i = 0; i < HISTORYMAXNUM; i++) {
            const char *p;
            MMDAgent_snprintf(buff, MMDAGENT_MAXBUFLEN, "dir%d", i);
            p = v->getString(buff, NULL);
            if (p && MMDAgent_stat(p) == MMDAGENT_STAT_DIRECTORY) {
               MMDAgent_snprintf(buff, MMDAGENT_MAXBUFLEN, "mdf%d", i);
               mdfName = v->getString(buff, NULL);
               if (p) {
                  break;
               }
            }
         }
         setResetFlag(mdfName ? mdfName : m_configFileName);
      }
   }
}

/* MMDAgent::isViewMovable: return true if view can be moved */
bool MMDAgent::isViewMovable()
{
   return m_cameraCanMove;
}

/* MMDAgent::pauseThreads: pause all threads */
void MMDAgent::pauseThreads()
{
   if (m_threadsPauseMutex == NULL)
      m_threadsPauseMutex = glfwCreateMutex();
   if (m_threadsPauseCond == NULL)
      m_threadsPauseCond = glfwCreateCond();

   glfwLockMutex(m_threadsPauseMutex);
   m_threadsPauseFlag = true;
   glfwUnlockMutex(m_threadsPauseMutex);
   sendLogString(m_moduleId, MLOG_STATUS, "pause threads");
}

/* MMDAgent::resumeThreads: resume paused threads */
void MMDAgent::resumeThreads()
{
   if (m_threadsPauseMutex == NULL)
      return;

   glfwLockMutex(m_threadsPauseMutex);
   m_threadsPauseFlag = false;
   glfwBroadcastCond(m_threadsPauseCond);
   glfwUnlockMutex(m_threadsPauseMutex);
   m_timer->setup();
   m_timer->startAdjustment();
   sendLogString(m_moduleId, MLOG_STATUS, "resume threads");
}

/* MMDAgent::waitWhenPaused: wait when paused, till resume */
void MMDAgent::waitWhenPaused()
{
   if (m_threadsPauseMutex == NULL)
      return;

   glfwLockMutex(m_threadsPauseMutex);
   while (m_threadsPauseFlag == true)
      glfwWaitCond(m_threadsPauseCond, m_threadsPauseMutex, GLFW_INFINITY);
   glfwUnlockMutex(m_threadsPauseMutex);
}

/* MMDAgent::getArguments: return arguments given at start up */
char **MMDAgent::getArguments(int *num_ret)
{
   *num_ret = m_argc;
   return m_argv;
}

/* MMDAgent::procWindowDestroyMessage: process window destroy message */
void MMDAgent::procWindowDestroyMessage()
{
   if(m_enable == false)
      return;

   if (m_plugin && m_pluginStarted == true)
      m_plugin->execAppEnd(this);

   clear();
}

/* MMDAgent::procMouseLeftButtonDoubleClickMessage: process mouse left button double click message */
void MMDAgent::procMouseLeftButtonDoubleClickMessage(int x, int y)
{
   if(m_enable == false)
      return;

   /* double click */
   m_mousePosX = x;
   m_mousePosY = y;
   /* store model ID */
   m_selectedModel = m_render->pickModel(m_model, m_numModel, x, y, NULL);
   /* make model highlight */
   setHighLight(m_selectedModel);
   m_doubleClicked = true;
}

/* MMDAgent::procMouseLeftButtonDownMessage: process mouse left button down message */
void MMDAgent::procMouseLeftButtonDownMessage(int x, int y, bool withCtrl, bool withShift)
{
   if(m_enable == false)
      return;

   /* start hold */
   m_mousePosX = x;
   m_mousePosY = y;
   m_leftButtonPressed = true;
   m_doubleClicked = false;

   /* store model ID */
   m_selectedModel = m_render->pickModel(m_model, m_numModel, x, y, NULL);
   if (withCtrl == true && withShift == false) /* with Ctrl-key */
      setHighLight(m_selectedModel);

   /* send message */
   if(m_selectedModel >= 0) {
      sendMessage(m_moduleId, MMDAGENT_EVENT_MODELSELECT, "%s", m_model[m_selectedModel].getAlias());
   }

#ifdef MMDAGENT_TAPPED
   sendMessage(m_moduleId, "TAPPED", "%02d|%02d", x, y);
#endif
}

/* MMDAgent::procMouseLeftButtonUpMessage: process mouse left button up message */
void MMDAgent::procMouseLeftButtonUpMessage()
{
   if(m_enable == false)
      return;

   /* if highlight, trun off */
   if (!m_doubleClicked)
      setHighLight(-1);
   /* end of hold */
   m_leftButtonPressed = false;
}

/* MMDAgent::procMouseLeftButtonLongPressedMessage: process mouse left button long pressed message */
void MMDAgent::procMouseLeftButtonLongPressedMessage(int x, int y, int screenWidth, int screenHeight)
{
   sendMessage(m_moduleId, MMDAGENT_EVENT_LONGPRESSED, "%05d_%05d_%05d_%05d", x, y, screenWidth, screenHeight);
}

/* MMDAgent::procMouseLeftButtonLongReleasedMessage: process mouse left button long released message */
void MMDAgent::procMouseLeftButtonLongReleasedMessage(int x, int y, int screenWidth, int screenHeight)
{
   sendMessage(m_moduleId, MMDAGENT_EVENT_LONGRELEASED, "%05d_%05d_%05d_%05d", x, y, screenWidth, screenHeight);
}


/* MMDAgent::procMouseWheelMessage: process mouse wheel message */
void MMDAgent::procMouseWheelMessage(bool zoomup, bool withCtrl, bool withShift)
{
   float rate;

   if(m_enable == false)
      return;

   if (m_cameraCanMove == false)
      return;

   if (withCtrl && withShift) {
      /* move camera fovy */
      procCameraFovyMessage(zoomup);
   } else {
      /* move camera distance */
      if (withCtrl) /* faster */
         rate = 5.0f;
      else if (withShift) /* slower */
         rate = 0.2f;
      else
         rate = 1.0f;
      procCameraMoveMessage(zoomup, rate);
   }
}

/* MMDAgent::procMouseStatusMessage: process mouse status message */
void MMDAgent::procMouseStatusMessage(int x, int y, bool withCtrl, bool withShift)
{
   if (m_enable == false)
      return;

   /* store Ctrl-key and Shift-key state for drag and drop */
   m_keyCtrl = withCtrl;
   m_keyShift = withShift;

   if (m_mousePosX != x || m_mousePosY != y) {
      /* set mouse enable timer */
      m_screen->setMouseActiveTime(45.0f);
   }

   /* store cursor position */
   m_mousePosX = x;
   m_mousePosY = y;
}

/* MMDAgent::procMousePosMessage: process mouse position message */
void MMDAgent::procMousePosMessage(int x, int y, bool withCtrl, bool withShift)
{
   float *f;
   int r1, r2;
   btVector3 v;
   btMatrix3x3 bm;
   btTransform tr;
   float factor;

   if(m_enable == false)
      return;

   /* store Ctrl-key and Shift-key state for drag and drop */
   m_keyCtrl = withCtrl;
   m_keyShift = withShift;

   /* left-button is dragged in window */
   if (m_leftButtonPressed && m_cameraCanMove) {
      r1 = x;
      r2 = y;
      r1 -= m_mousePosX;
      r2 -= m_mousePosY;
      if (r1 > 32767) r1 -= 65536;
      if (r1 < -32768) r1 += 65536;
      if (r2 > 32767) r2 -= 65536;
      if (r2 < -32768) r2 += 65536;
      factor = (float)fabs(m_render->getDistance());
      if (factor < 10.0f) factor = 10.0f;
      if (withShift && withCtrl && m_selectedModel == -1) {
         /* if Shift- and Ctrl-key, and no model is pointed, rotate light direction */
         f = m_option->getLightDirection();
         v = btVector3(btScalar(f[0]), btScalar(f[1]), btScalar(f[2]));
         bm = btMatrix3x3(btQuaternion(btScalar(0.0f), btScalar(r2 * 0.1f * MMDFILES_RAD(m_option->getRotateStep())), btScalar(0.0f)) * btQuaternion(btScalar(r1 * 0.1f * MMDFILES_RAD(m_option->getRotateStep())), btScalar(0.0f), btScalar(0.0f)));
         v = bm * v;
         changeLightDirection(v.x(), v.y(), v.z());
      } else if (withCtrl) {
         /* if Ctrl-key and model is pointed, move the model */
         if (m_selectedModel != -1) {
            setHighLight(m_selectedModel);
            m_model[m_selectedModel].getTargetPosition(&v);
            if (withShift) {
               /* with Shift-key, move on XY (coronal) plane */
               v.setX(btScalar(v.x() + r1 * 0.001f * m_option->getTranslateStep() * factor));
               v.setY(btScalar(v.y() - r2 * 0.001f * m_option->getTranslateStep() * factor));
            } else {
               /* else, move on XZ (axial) plane */
               v.setX(btScalar(v.x() + r1 * 0.001f * m_option->getTranslateStep() * factor));
               v.setZ(btScalar(v.z() + r2 * 0.001f * m_option->getTranslateStep() * factor));
            }
            m_model[m_selectedModel].setPosition(&v);
            m_model[m_selectedModel].setMoveSpeed(-1.0f);
         }
      } else if (withShift) {
         /* if Shift-key, translate display */
         v = btVector3(btScalar(r1 * 0.0005f * factor), btScalar(-r2 * 0.0005f * factor), btScalar(0.0f));
         m_render->getCurrentViewTransform(&tr);
         tr.setOrigin(btVector3(btScalar(0.0f), btScalar(0.0f), btScalar(0.0f)));
         v = tr.inverse() * v;
         m_render->translate(-v.x(), -v.y(), -v.z());
      } else {
         /* if no key, rotate display */
         m_render->rotate(r2 * 0.1f * m_option->getRotateStep(), r1 * 0.1f * m_option->getRotateStep(), 0.0f);
      }
   } else if (m_mousePosX != x || m_mousePosY != y) {
      /* set mouse enable timer */
      m_screen->setMouseActiveTime(45.0f);
   }
   m_mousePosX = x;
   m_mousePosY = y;
}

/* MMDAgent::procFullScreenMessage: process full screen message */
void MMDAgent::procFullScreenMessage()
{
   if(m_enable == false)
      return;

   if (m_option->getFullScreen() == true) {
      m_screen->exitFullScreen();
      m_option->setFullScreen(false);
   } else {
      m_screen->setFullScreen();
      m_option->setFullScreen(true);
   }
}

/* MMDAgent::procInfoStringMessage: process information string message */
void MMDAgent::procInfoStringMessage()
{
   if(m_enable == false)
      return;

   if(m_option->getShowFps() == true)
      m_option->setShowFps(false);
   else
      m_option->setShowFps(true);
}

/* MMDAgent::procVSyncMessage: process vsync message */
void MMDAgent::procVSyncMessage()
{
   if(m_enable == false)
      return;

   m_screen->toggleVSync();
}

/* MMDAgent::procShadowMessage: process shadow message */
void MMDAgent::procShadowMessage()
{
   if (m_enable == false)
      return;

   if (m_option->getUseShadow() == true) {
      m_option->setUseShadow(false);
   } else {
      m_option->setUseShadow(true);
   }
   m_render->setShadow(m_option->getUseShadow());
}

/* MMDAgent::procShadowMappingMessage: process shadow mapping message */
void MMDAgent::procShadowMappingMessage()
{
   if(m_enable == false)
      return;

   if(m_option->getUseShadowMapping() == true)
      m_option->setUseShadowMapping(false);
   else
      m_option->setUseShadowMapping(true);

   m_render->setShadowMapping(m_option->getUseShadowMapping(), m_option->getShadowMappingTextureSize());
}

/* MMDAgent::procDisplayRigidBodyMessage: process display rigid body message */
void MMDAgent::procDisplayRigidBodyMessage()
{
   if(m_enable == false)
      return;

   if (m_dispBulletBodyFlag == true) {
      m_dispBulletBodyFlag = false;
      if (m_offscreen)
         m_offscreen->resume();
   } else {
      m_dispBulletBodyFlag = true;
      if (m_offscreen)
         m_offscreen->pause();
   }
}

/* MMDAnget::procDisplayWireMessage: process display wire message */
void MMDAgent::procDisplayWireMessage()
{
#ifndef MMDAGENT_DONTRENDERDEBUG
   GLint polygonMode[2];

   if(m_enable == false)
      return;

   glGetIntegerv(GL_POLYGON_MODE, polygonMode);
   if (polygonMode[1] == GL_LINE) {
      if (m_offscreen)
         m_offscreen->resume();
      glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
   } else {
      if (m_offscreen)
         m_offscreen->pause();
      glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
   }
#endif /* !MMDAGENT_DONTRENDERDEBUG */
}

/* MMDAgent::procDisplayBoneMessage: process display bone message */
void MMDAgent::procDisplayBoneMessage()
{
   if(m_enable == false)
      return;

   m_dispModelDebug = !m_dispModelDebug;
}

/* MMDAgent::procCartoonEdgeMessage: process cartoon edge message */
void MMDAgent::procCartoonEdgeMessage(bool plus)
{
   int i;

   if(m_enable == false)
      return;

   if(plus)
      m_option->setCartoonEdgeWidth(m_option->getCartoonEdgeWidth() * m_option->getCartoonEdgeStep());
   else
      m_option->setCartoonEdgeWidth(m_option->getCartoonEdgeWidth() / m_option->getCartoonEdgeStep());
   for (i = 0; i < m_numModel; i++) {
      if (m_model[i].isEnable())
         m_model[i].getPMDModel()->setEdgeThin(m_option->getCartoonEdgeWidth());
   }
}

/* MMDAgent::procTimeAdjustMessage: process time adjust message */
void MMDAgent::procTimeAdjustMessage(bool plus)
{
   if(m_enable == false)
      return;

   if(plus)
      m_option->setMotionAdjustTime(m_option->getMotionAdjustTime() + 0.01f);
   else
      m_option->setMotionAdjustTime(m_option->getMotionAdjustTime() - 0.01f);
   m_timer->setTargetAdjustmentFrame(m_option->getMotionAdjustTime() * 30.0);
}

/* MMDAgent::procHorizontalRotateMessage: process horizontal rotate message */
void MMDAgent::procHorizontalRotateMessage(bool right)
{
   if(m_enable == false)
      return;

   if(right)
      m_render->rotate(0.0f, m_option->getRotateStep(), 0.0f);
   else
      m_render->rotate(0.0f, -m_option->getRotateStep(), 0.0f);
}

/* MMDAgent::procVerticalRotateMessage: process vertical rotate message */
void MMDAgent::procVerticalRotateMessage(bool up)
{
   if(m_enable == false)
      return;

   if(up)
      m_render->rotate(-m_option->getRotateStep(), 0.0f, 0.0f);
   else
      m_render->rotate(m_option->getRotateStep(), 0.0f, 0.0f);
}

/* MMDAgent::procHorizontalMoveMessage: process horizontal move message */
void MMDAgent::procHorizontalMoveMessage(bool right)
{
   if(m_enable == false)
      return;

   if(right)
      m_render->translate(m_option->getTranslateStep(), 0.0f, 0.0f);
   else
      m_render->translate(-m_option->getTranslateStep(), 0.0f, 0.0f);
}

/* MMDAgent::procVerticalMoveMessage: process vertical move message */
void MMDAgent::procVerticalMoveMessage(bool up)
{
   if(m_enable == false)
      return;

   if(up)
      m_render->translate(0.0f, m_option->getTranslateStep(), 0.0f);
   else
      m_render->translate(0.0f, -m_option->getTranslateStep(), 0.0f);
}

/* MMDAgent::procCameraMoveMessage: process camera move message */
void MMDAgent::procCameraMoveMessage(bool zoomup, float stepFactor)
{
   float step, distance;

   if (m_enable == false)
      return;

   step = m_option->getDistanceStep() * stepFactor;
   distance = m_render->getDistance();
   if (step != 0.0) {
      if (zoomup)
         distance -= step;
      else
         distance += step;
      m_render->setDistance(distance);
   }
}

/* MMDAgent::procCameraFovyMessage: process camera fovy change message */
void MMDAgent::procCameraFovyMessage(bool zoomup)
{
   float step, fovy;

   if (m_enable == false)
      return;

   step = m_option->getFovyStep();
   fovy = m_render->getFovy();
   if (step != 0.0) {
      if (zoomup)
         fovy -= step;
      else
         fovy += step;
      m_render->setFovy(fovy);
   }
}

/* MMDAgent::procMoveResetMessage: process move reset message */
void MMDAgent::procMoveResetMessage()
{
   if(m_enable == false)
      return;
   m_render->resetCamera();
}

/* MMDAgent::procToggleCameraMoveMessage: process toggle camera move message */
void MMDAgent::procToggleCameraMoveMessage()
{
   if (m_enable == false)
      return;
   m_cameraCanMove = !m_cameraCanMove;
}

/* MMDAgent::procDeleteModelMessage: process delete model message */
void MMDAgent::procDeleteModelMessage()
{
   if(m_enable == false)
      return;

   if (m_doubleClicked && m_selectedModel != -1) {
      deleteModel(m_model[m_selectedModel].getAlias());
      m_doubleClicked = false;
   }
}

/* MMDAgent::procPhysicsMessage: process physics message */
void MMDAgent::procPhysicsMessage()
{
   int i;

   if(m_enable == false)
      return;

   m_enablePhysicsSimulation = !m_enablePhysicsSimulation;
   for (i = 0; i < m_numModel; i++) {
      if (m_model[i].isEnable() == false) continue;
#ifdef MY_RESETPHYSICS
      m_model[i].getPMDModel()->setPhysicsControl(m_enablePhysicsSimulation, true);
#else
      m_model[i].getPMDModel()->setPhysicsControl(m_enablePhysicsSimulation);
#endif
   }
}

#ifdef MY_RESETPHYSICS
/* MMDAgent::procResetPhysicsMessage: process physics reset message */
void MMDAgent::procResetPhysicsMessage()
{
   int i;

   if(m_enable == false)
      return;

   m_enablePhysicsSimulation = !m_enablePhysicsSimulation;
   for (i = 0; i < m_numModel; i++) {
      if (m_model[i].isEnable())
         m_model[i].getPMDModel()->setPhysicsControl(m_enablePhysicsSimulation, false);
   }
}
#endif

#ifdef MY_LUMINOUS
/* MMDAgent::procLuminousMessage: process luminoust message */
void MMDAgent::procLuminousMessage()
{
   if (m_enable == false)
      return;

   m_render->toggleLuminous();
}
#endif

/* MMDAgent::procShaderEffectMessage: process shader effect message */
void MMDAgent::procShaderEffectMessage()
{
   if (m_enable == false)
      return;

   if (m_offscreen == NULL)
      return;

   m_offscreen->setRelativeIntensity(0.2f);
}

/* MMDAgent::procShaderEffectScalingMessage: process shader effect scaling message */
void MMDAgent::procShaderEffectScalingMessage()
{
   if (m_enable == false)
      return;

   if (m_offscreen == NULL)
      return;

   m_offscreen->setRelativeScaling(1.4f);
}

/* MMDAgent::procDisplayLogMessage: process display log message */
void MMDAgent::procDisplayLogMessage()
{
   if(m_enable == false)
      return;

   m_dispLog = !m_dispLog;
   if (m_loggerLog) m_loggerLog->setStatus(m_dispLog);
   if (m_loggerMessage) m_loggerMessage->setStatus(m_dispLog);
}

/* MMDAgent::procDisplayLogConsoleMessage: process display log console message */
void MMDAgent::procDisplayLogConsoleMessage()
{
   if (m_enable == false)
      return;

   m_dispLogConsole = !m_dispLogConsole;

#ifdef _WIN32
   if (m_dispLogConsole == true) {
      AllocConsole();
      freopen("CON", "r", stdin);
      freopen("CON", "w", stdout);
   } else {
      FreeConsole();
   }
#endif
}

/* MMDAgent::procHoldMessage: process hold message */
void MMDAgent::procHoldMessage()
{
   if(m_enable == false)
      return;

   m_holdMotion = !m_holdMotion;
}

/* MMDAgent::procWindowSizeMessage: process window size message */
void MMDAgent::procWindowSizeMessage(int x, int y)
{
   Button *b;

   if(m_enable == false)
      return;

   /* make sure not perform resize when size was not changed */
   if (m_screenSize[0] == x && m_screenSize[1] == y)
      return;

   /* set new size */
   m_screenSize[0] = x;
   m_screenSize[1] = y;
   m_option->setWindowSize(m_screenSize);

   /* set rendering size */
   m_render->setSize(x, y);
   if (m_offscreen)
      m_offscreen->changeScreenSize(x, y);

   /* update button position */
   for (b = m_button; b; b = b->getNext())
      b->updatePosition();

}

/* MMDAgent::procLogNarrowingMessage: process log narrowing message */
void MMDAgent::procLogNarrowingMessage()
{
   if (m_enable == false)
      return;

   if (m_dispLog == false)
      return;

   if (m_loggerLog) m_loggerLog->startTyping();
   if (m_loggerMessage) m_loggerMessage->startTyping();
}

/* MMDAgent::procKeyMessage: process key message */
bool MMDAgent::procKeyMessage(int key, int action)
{
   bool ret = false;

   if (m_enable == false)
      return false;

   if (action == GLFW_PRESS && m_loggerLog && m_loggerLog->isTypingActive()) {
      /* typing for narrowing */
      if (key == GLFW_KEY_ESC) {
         m_loggerLog->resetNarrowString();
         m_loggerLog->endTyping();
      } else if (key == GLFW_KEY_ENTER)
         m_loggerLog->endTyping();
      else if (key == GLFW_KEY_DEL || key == GLFW_KEY_BACKSPACE)
         m_loggerLog->backwardCharToNarrowString();
      /* return true to disable other key functioning while typing */
      ret = true;
   }

   if (action == GLFW_PRESS && m_loggerMessage && m_loggerMessage->isTypingActive()) {
      /* typing for narrowing */
      if (key == GLFW_KEY_ESC) {
         m_loggerMessage->resetNarrowString();
         m_loggerMessage->endTyping();
      } else if (key == GLFW_KEY_ENTER)
         m_loggerMessage->endTyping();
      else if (key == GLFW_KEY_DEL || key == GLFW_KEY_BACKSPACE)
         m_loggerMessage->backwardCharToNarrowString();
      /* return true to disable other key functioning while typing */
      ret = true;
   }

   return ret;
}

/* MMDAgent::procCharMessage: process char message */
bool MMDAgent::procCharMessage(char c)
{
   bool ret = false;

   if (m_enable == false)
      return false;

   if (m_loggerLog && m_loggerLog->isTypingActive()) {
      /* typing for narrowing */
      m_loggerLog->addCharToNarrowString(c);
      /* return false to disable other key functioning while typing */
      ret = true;
   }

   if (m_loggerMessage && m_loggerMessage->isTypingActive()) {
      /* typing for narrowing */
      m_loggerMessage->addCharToNarrowString(c);
      /* return false to disable other key functioning while typing */
      ret = true;
   }

   sendMessage(m_moduleId, MMDAGENT_EVENT_KEY, "%C", c);

   return ret;
}

/* MMDAgent::procReceivedMessage: process received message */
void MMDAgent::procReceivedMessage(const char *type, const char *value)
{
   static char buff[MMDAGENT_MAXBUFLEN];    /* static buffer */
   static char *argv[MMDAGENT_MAXNCOMMAND];
   int num = 0;

   char *str1, *str2, *str3;
   bool bool1, bool2, bool3, bool4;
   float f;
   btVector3 pos;
   btQuaternion rot;
   float fvec[3];

   if(m_enable == false)
      return;

   if(MMDAgent_strlen(type) <= 0)
      return;

   if (m_option->getUseStdInOut() == true) {
      /* output message to stdout */
      if (MMDAgent_strlen(type) > 0) {
         if (MMDAgent_strlen(value) > 0) {
            printf("%s|%s\n", type, value);
         } else {
            printf("%s\n", type);
         }
         fflush(stdout);
      }
   }

   if (m_plugin)
      m_plugin->execProcMessage(this, type, value);

   /* divide string into arguments */
   if (MMDAgent_strlen(value) > 0) {
      strncpy(buff, value, MMDAGENT_MAXBUFLEN - 1);
      buff[MMDAGENT_MAXBUFLEN - 1] = '\0';
      for (str1 = MMDAgent_strtok(buff, "|", &str2); str1; str1 = MMDAgent_strtok(NULL, "|", &str2)) {
         if (num >= MMDAGENT_MAXNCOMMAND) {
            sendLogString(m_moduleId, MLOG_ERROR, "%s: number of arguments exceed the limit.", type);
            break;
         }
         argv[num] = str1;
         num++;
      }
   }

   if (MMDAgent_strequal(type, MMDAGENT_COMMAND_MODELADD)) {
      sendLogString(m_moduleId, MLOG_MESSAGE_CAPTURED, "%s|%s", type, value);
      bool1 = true;
      str1 = NULL;
      str2 = NULL;
      if (num < 2 || num > 7) {
         sendLogString(m_moduleId, MLOG_ERROR, "%s: number of arguments should be 2-7.", type);
         return;
      }
      if (num >= 3) {
         if (MMDAgent_str2pos(argv[2], &pos) == false) {
            sendLogString(m_moduleId, MLOG_ERROR, "%s: %s is not a position string.", type, argv[2]);
            return;
         }
      } else {
         pos = btVector3(btScalar(0.0f), btScalar(0.0f), btScalar(0.0f));
      }
      if (num >= 4) {
         if (MMDAgent_str2rot(argv[3], &rot) == false) {
            sendLogString(m_moduleId, MLOG_ERROR, "%s: %s is not a rotation string.", type, argv[3]);
            return;
         }
      } else {
         rot.setEulerZYX(btScalar(0.0f), btScalar(0.0f), btScalar(0.0f));
      }
      if(num >= 5) {
         if(MMDAgent_strequal(argv[4], "ON")) {
            bool1 = true;
         } else if(MMDAgent_strequal(argv[4], "OFF")) {
            bool1 = false;
         } else {
            sendLogString(m_moduleId, MLOG_ERROR, "%s: 5th argument should be \"ON\" or \"OFF\".", type);
            return;
         }
      }
      if (num >= 6) {
         str1 = argv[5];
      }
      if (num >= 7) {
         str2 = argv[6];
      }
      addModel(argv[0], argv[1], &pos, &rot, bool1, str1, str2);
   } else if (MMDAgent_strequal(type, MMDAGENT_COMMAND_MODELCHANGE)) {
      /* change model */
      sendLogString(m_moduleId, MLOG_MESSAGE_CAPTURED, "%s|%s", type, value);
      if (num != 2) {
         sendLogString(m_moduleId, MLOG_ERROR, "%s: number of arguments should be 2.", type);
         return;
      }
      int id = findModelAlias(argv[0]);
      if (id >= 0)
         m_model[id].lock();
      changeModel(argv[0], argv[1]);
      if (id >= 0)
         m_model[id].unlock();
   } else if (MMDAgent_strequal(type, MMDAGENT_COMMAND_MODELCHANGEASYNC)) {
      /* change model async */
      sendLogString(m_moduleId, MLOG_MESSAGE_CAPTURED, "%s|%s", type, value);
      if (num != 2) {
         sendLogString(m_moduleId, MLOG_ERROR, "%s: number of arguments should be 2.", type);
         return;
      }
      if (m_threadedLoading)
         m_threadedLoading->startModelChangeThread(argv[1], argv[0]);
   } else if (MMDAgent_strequal(type, MMDAGENT_COMMAND_MODELDELETE)) {
      /* delete model */
      sendLogString(m_moduleId, MLOG_MESSAGE_CAPTURED, "%s|%s", type, value);
      if (num != 1) {
         sendLogString(m_moduleId, MLOG_ERROR, "%s: number of arguments should be 1.", type);
         return;
      }
      deleteModel(argv[0]);
   } else if (MMDAgent_strequal(type, MMDAGENT_COMMAND_MOTIONADD)) {
      /* add motion */
      sendLogString(m_moduleId, MLOG_MESSAGE_CAPTURED, "%s|%s", type, value);
      bool1 = true; /* full */
      bool2 = true; /* once */
      bool3 = true; /* enableSmooth */
      bool4 = false; /* enableRePos */
      f = MOTIONMANAGER_DEFAULTPRIORITY; /* priority */
      if (num < 3 || num > 8) {
         sendLogString(m_moduleId, MLOG_ERROR, "%s: number of arguments should be 4-7.", type);
         return;
      }
      if (num >= 4) {
         if (MMDAgent_strequal(argv[3], "FULL")) {
            bool1 = true;
         } else if (MMDAgent_strequal(argv[3], "PART")) {
            bool1 = false;
         } else {
            sendLogString(m_moduleId, MLOG_ERROR, "%s: 4th argument should be \"FULL\" or \"PART\".", type);
            return;
         }
      }
      if (num >= 5) {
         if (MMDAgent_strequal(argv[4], "ONCE")) {
            bool2 = true;
         } else if (MMDAgent_strequal(argv[4], "LOOP")) {
            bool2 = false;
         } else {
            sendLogString(m_moduleId, MLOG_ERROR, "%s: 5th argument should be \"ONCE\" or \"LOOP\".", type);
            return;
         }
      }
      if (num >= 6) {
         if (MMDAgent_strequal(argv[5], "ON")) {
            bool3 = true;
         } else if (MMDAgent_strequal(argv[5], "OFF")) {
            bool3 = false;
         } else {
            sendLogString(m_moduleId, MLOG_ERROR, "%s: 6th argument should be \"ON\" or \"OFF\".", type);
            return;
         }
      }
      if (num >= 7) {
         if (MMDAgent_strequal(argv[6], "ON")) {
            bool4 = true;
         } else if (MMDAgent_strequal(argv[6], "OFF")) {
            bool4 = false;
         } else {
            sendLogString(m_moduleId, MLOG_ERROR, "%s: 7th argument should be \"ON\" or \"OFF\".", type);
            return;
         }
      }
      if (num >= 8) {
         f = MMDAgent_str2float(argv[7]);
      }
      addMotion(argv[0], argv[1], argv[2], bool1, bool2, bool3, bool4, f);
   } else if (MMDAgent_strequal(type, MMDAGENT_COMMAND_MOTIONCHANGE)) {
      /* change motion */
      sendLogString(m_moduleId, MLOG_MESSAGE_CAPTURED, "%s|%s", type, value);
      if (num != 3) {
         sendLogString(m_moduleId, MLOG_ERROR, "%s: number of arguments should be 3.", type);
         return;
      }
      changeMotion(argv[0], argv[1], argv[2]);
   } else if (MMDAgent_strequal(type, MMDAGENT_COMMAND_MOTIONRESET)) {
      /* reset motion */
      sendLogString(m_moduleId, MLOG_MESSAGE_CAPTURED, "%s|%s", type, value);
      if (num != 2) {
         sendLogString(m_moduleId, MLOG_ERROR, "%s: number of arguments should be 2.", type);
         return;
      }
      resetMotion(argv[0], argv[1]);
   } else if (MMDAgent_strequal(type, MMDAGENT_COMMAND_MOTIONACCELERATE)) {
      /* accelerate motion */
      sendLogString(m_moduleId, MLOG_MESSAGE_CAPTURED, "%s|%s", type, value);
      fvec[0] = 0.0f;  /* speed */
      fvec[1] = 0.0f;  /* duration time in sec */
      fvec[2] = -1.0f; /* specified frame index for end of acceleration */
      if (num < 3 || num > 5) {
         sendLogString(m_moduleId, MLOG_ERROR, "%s: number of arguments should be 3-5.", type);
         return;
      }
      if(num >= 3)
         fvec[0] = MMDAgent_str2float(argv[2]);
      if (num >= 4)
         fvec[1] = MMDAgent_str2float(argv[3]);
      if (num >= 5)
         fvec[2] = MMDAgent_str2float(argv[4]);
      accelerateMotion(argv[0], argv[1], fvec[0], fvec[1], fvec[2]);
   } else if (MMDAgent_strequal(type, MMDAGENT_COMMAND_MOTIONDELETE)) {
      /* delete motion */
      sendLogString(m_moduleId, MLOG_MESSAGE_CAPTURED, "%s|%s", type, value);
      if (num != 2) {
         sendLogString(m_moduleId, MLOG_ERROR, "%s: number of arguments should be 2.", type);
         return;
      }
      deleteMotion(argv[0], argv[1]);
   }
   else if (MMDAgent_strequal(type, MMDAGENT_COMMAND_MOTIONCONFIGURE)) {
      /* configure motion */
      sendLogString(m_moduleId, MLOG_MESSAGE_CAPTURED, "%s|%s", type, value);
      if (num < 3 || num > 4) {
         sendLogString(m_moduleId, MLOG_ERROR, "%s: number of arguments should be 3 or 4.", type);
         return;
      }
      configureMotion(argv[0], argv[1], argv[2], num >= 4 ? argv[3] : NULL);
   }
   else if (MMDAgent_strequal(type, MMDAGENT_COMMAND_MOVESTART)) {
      /* start moving */
      sendLogString(m_moduleId, MLOG_MESSAGE_CAPTURED, "%s|%s", type, value);
      bool1 = false;
      f = -1.0;
      if (num < 2 || num > 4) {
         sendLogString(m_moduleId, MLOG_ERROR, "%s: number of arguments should be 2-4.", type);
         return;
      }
      if (MMDAgent_str2pos(argv[1], &pos) == false) {
         sendLogString(m_moduleId, MLOG_ERROR, "%s: %s is not a position string.", type, argv[1]);
         return;
      }
      if (num >= 3) {
         if (MMDAgent_strequal(argv[2], "LOCAL")) {
            bool1 = true;
         } else if (MMDAgent_strequal(argv[2], "GLOBAL")) {
            bool1 = false;
         } else {
            sendLogString(m_moduleId, MLOG_ERROR, "%s: 3rd argument should be \"GLOBAL\" or \"LOCAL\".", type);
            return;
         }
      }
      if (num >= 4)
         f = MMDAgent_str2float(argv[3]);
      startMove(argv[0], &pos, bool1, f);
   } else if (MMDAgent_strequal(type, MMDAGENT_COMMAND_MOVESTOP)) {
      /* stop moving */
      sendLogString(m_moduleId, MLOG_MESSAGE_CAPTURED, "%s|%s", type, value);
      if (num != 1) {
         sendLogString(m_moduleId, MLOG_ERROR, "%s: number of arguments should be 1.", type);
         return;
      }
      stopMove(argv[0]);
   } else if (MMDAgent_strequal(type, MMDAGENT_COMMAND_ROTATESTART)) {
      /* start rotation */
      sendLogString(m_moduleId, MLOG_MESSAGE_CAPTURED, "%s|%s", type, value);
      bool1 = false;
      f = -1.0;
      if (num < 2 || num > 4) {
         sendLogString(m_moduleId, MLOG_ERROR, "%s: number of arguments should be 2-4.", type);
         return;
      }
      if (MMDAgent_str2rot(argv[1], &rot) == false) {
         sendLogString(m_moduleId, MLOG_ERROR, "%s: %s is not a rotation string.", type, argv[1]);
         return;
      }
      if (num >= 3) {
         if (MMDAgent_strequal(argv[2], "LOCAL")) {
            bool1 = true;
         } else if (MMDAgent_strequal(argv[2], "GLOBAL")) {
            bool1 = false;
         } else {
            sendLogString(m_moduleId, MLOG_ERROR, "%s: 3rd argument should be \"GLOBAL\" or \"LOCAL\".", type);
            return;
         }
      }
      if (num >= 4)
         f = MMDAgent_str2float(argv[3]);
      startRotation(argv[0], &rot, bool1, f);
   } else if (MMDAgent_strequal(type, MMDAGENT_COMMAND_ROTATESTOP)) {
      /* stop rotation */
      sendLogString(m_moduleId, MLOG_MESSAGE_CAPTURED, "%s|%s", type, value);
      if (num != 1) {
         sendLogString(m_moduleId, MLOG_ERROR, "%s: number of arguments should be 1.", type);
         return;
      }
      stopRotation(argv[0]);
   } else if (MMDAgent_strequal(type, MMDAGENT_COMMAND_TURNSTART)) {
      /* turn start */
      sendLogString(m_moduleId, MLOG_MESSAGE_CAPTURED, "%s|%s", type, value);
      bool1 = false;
      f = -1.0;
      if (num < 2 || num > 4) {
         sendLogString(m_moduleId, MLOG_ERROR, "%s: number of arguments should be 2-4.", type);
         return;
      }
      if (MMDAgent_str2pos(argv[1], &pos) == false) {
         sendLogString(m_moduleId, MLOG_ERROR, "%s: %s is not a position string.", type, argv[1]);
         return;
      }
      if (num >= 3) {
         if (MMDAgent_strequal(argv[2], "LOCAL")) {
            bool1 = true;
         } else if (MMDAgent_strequal(argv[2], "GLOBAL")) {
            bool1 = false;
         } else {
            sendLogString(m_moduleId, MLOG_ERROR, "%s: 3rd argument should be \"GLOBAL\" or \"LOCAL\".", type);
            return;
         }
      }
      if (num >= 4)
         f = MMDAgent_str2float(argv[3]);
      startTurn(argv[0], &pos, bool1, f);
   } else if (MMDAgent_strequal(type, MMDAGENT_COMMAND_TURNSTOP)) {
      /* stop turn */
      sendLogString(m_moduleId, MLOG_MESSAGE_CAPTURED, "%s|%s", type, value);
      if (num != 1) {
         sendLogString(m_moduleId, MLOG_ERROR, "%s: number of arguments should be 1.", type);
         return;
      }
      stopTurn(argv[0]);
   } else if (MMDAgent_strequal(type, MMDAGENT_COMMAND_STAGE)) {
      /* change stage */
      sendLogString(m_moduleId, MLOG_MESSAGE_CAPTURED, "%s|%s", type, value);
      if (num != 1) {
         sendLogString(m_moduleId, MLOG_ERROR, "%s: number of arguments should be 1.", type);
         return;
      }
      /* pmd or bitmap */
      char *buf = MMDAgent_strdup(argv[0]);
      str1 = MMDAgent_strtok(buf, ",", &str3);
      str2 = MMDAgent_strtok(NULL, ",", &str3);
      if (str2 == NULL) {
         setStage(str1);
      } else {
         setFloor(str1);
         setBackground(str2);
      }
      free(buf);
   } else if (MMDAgent_strequal(type, MMDAGENT_COMMAND_WINDOWFRAME)) {
      /* change window frame (old) */
      sendLogString(m_moduleId, MLOG_MESSAGE_CAPTURED, "%s|%s", type, value);
      if (num != 1) {
         sendLogString(m_moduleId, MLOG_ERROR, "%s: number of arguments should be 1.", type);
         return;
      }
      setWindowFrame(value);
   } else if (MMDAgent_strequal(type, MMDAGENT_COMMAND_WINDOWFRAME_ADD)) {
      /* add or swap window frame */
      sendLogString(m_moduleId, MLOG_MESSAGE_CAPTURED, "%s|%s", type, value);
      if (num != 2) {
         sendLogString(m_moduleId, MLOG_ERROR, "%s: number of arguments should be 2.", type);
         return;
      }
      addWindowFrame(argv[0], argv[1]);
   } else if (MMDAgent_strequal(type, MMDAGENT_COMMAND_WINDOWFRAME_DELETE)) {
      /* delete window frame */
      sendLogString(m_moduleId, MLOG_MESSAGE_CAPTURED, "%s|%s", type, value);
      if (num != 1) {
         sendLogString(m_moduleId, MLOG_ERROR, "%s: number of arguments should be 1.", type);
         return;
      }
      deleteWindowFrame(argv[0]);
   } else if (MMDAgent_strequal(type, MMDAGENT_COMMAND_WINDOWFRAME_DELETEALL)) {
      /* delete window frame */
      sendLogString(m_moduleId, MLOG_MESSAGE_CAPTURED, "%s|%s", type, value);
      if (num != 0) {
         sendLogString(m_moduleId, MLOG_ERROR, "%s: number of arguments should be 0.", type);
         return;
      }
      deleteAllWindowFrame();
   } else if (MMDAgent_strequal(type, MMDAGENT_COMMAND_CAMERA)) {
      /* camera */
      sendLogString(m_moduleId, MLOG_MESSAGE_CAPTURED, "%s|%s", type, value);
      if((num < 4 || num > 7) && num != 1) {
         sendLogString(m_moduleId, MLOG_ERROR, "%s: number of arguments should be 1 or 4-7.", type);
         return;
      }
      if (num == 1) {
         changeCamera(argv[0], NULL, NULL, NULL, NULL, NULL, NULL);
      } else {
         changeCamera(argv[0], argv[1], argv[2], argv[3], (num >= 5) ? argv[4] : NULL, (num >= 6) ? argv[5] : NULL, (num >= 7) ? argv[6] : NULL);
      }
   } else if (MMDAgent_strequal(type, MMDAGENT_COMMAND_LIGHTCOLOR)) {
      /* change light color */
      sendLogString(m_moduleId, MLOG_MESSAGE_CAPTURED, "%s|%s", type, value);
      if (num != 1) {
         sendLogString(m_moduleId, MLOG_ERROR, "%s: number of arguments should be 1.", type);
         return;
      }
      if (MMDAgent_str2fvec(argv[0], fvec, 3) == false) {
         sendLogString(m_moduleId, MLOG_ERROR, "%s: \"%s\" is not RGB value.", type, argv[0]);
         return;
      }
      changeLightColor(fvec[0], fvec[1], fvec[2]);
   } else if (MMDAgent_strequal(type, MMDAGENT_COMMAND_LIGHTDIRECTION)) {
      /* change light direction */
      sendLogString(m_moduleId, MLOG_MESSAGE_CAPTURED, "%s|%s", type, value);
      if (num != 1) {
         sendLogString(m_moduleId, MLOG_ERROR, "%s: number of arguments should be 1.", type);
         return;
      }
      if (MMDAgent_str2fvec(argv[0], fvec, 3) == false) {
         sendLogString(m_moduleId, MLOG_ERROR, "%s: \"%s\" is not XYZ value.", type, argv[0]);
         return;
      }
      changeLightDirection(fvec[0], fvec[1], fvec[2]);
   } else if (MMDAgent_strequal(type, MMDAGENT_COMMAND_LIPSYNCSTART)) {
      /* start lip sync */
      sendLogString(m_moduleId, MLOG_MESSAGE_CAPTURED, "%s|%s", type, value);
      if (num != 2) {
         sendLogString(m_moduleId, MLOG_ERROR, "%s: number of arguments should be 2.", type);
         return;
      }
      startLipSync(argv[0], argv[1]);
   } else if (MMDAgent_strequal(type, MMDAGENT_COMMAND_LIPSYNCSTOP)) {
      /* stop lip sync */
      sendLogString(m_moduleId, MLOG_MESSAGE_CAPTURED, "%s|%s", type, value);
      if (num != 1) {
         sendLogString(m_moduleId, MLOG_ERROR, "%s: number of arguments should be 1.", type);
         return;
      }
      stopLipSync(argv[0]);
   } else if (MMDAgent_strequal(type, MENU_COMMAND_TYPE)) {
      if (m_menu->processMessage(type, value) == false)
         return;
   } else if (MMDAgent_strequal(type, PROMPT_COMMAND_SHOW)) {
      if (m_prompt->processMessage(type, value) == false)
         return;
   } else if (MMDAgent_strequal(type, NOTIFY_COMMAND_SHOW)) {
      if (m_notify->processMessage(type, value) == false)
         return;
   } else if (MMDAgent_strequal(type, MMDAGENT_COMMAND_MODELBINDBONE)) {
      // MODEL_BINDBONE|(key name)|(min)|(max)|(model alias)|(bone name)|x1,y1,z1|rx1,ry1,rz1|x2,y2,z2|rx2,ry2,rz2
      // MODEL_BINDBONE|(model alias)|(bone name)|x,y,z|rx,ry,rz
      btVector3 pos2;
      btQuaternion rot2;
      sendLogString(m_moduleId, MLOG_MESSAGE_CAPTURED, "%s|%s", type, value);
      if (num != 4 && num != 9) {
         sendLogString(m_moduleId, MLOG_ERROR, "%s: number of arguments should be 4 or 9.", type);
         return;
      }
      if (num == 4) {
         if (MMDAgent_str2pos(argv[2], &pos) == false) {
            sendLogString(m_moduleId, MLOG_ERROR, "%s: %s is not a position string.", type, argv[5]);
            return;
         }
         if (MMDAgent_str2rot(argv[3], &rot) == false) {
            sendLogString(m_moduleId, MLOG_ERROR, "%s: %s is not a rotation string.", type, argv[6]);
            return;
         }
         if (addBoneControl(NULL, 0.0f, 0.0f, argv[0], argv[1], &pos, &rot, &pos, &rot)) {
            /* send message */
            sendMessage(m_moduleId, MMDAGENT_EVENT_MODELBINDBONE, "%s|%s", argv[0], argv[1]);
         }
      } else {
         if (MMDAgent_str2pos(argv[5], &pos) == false) {
            sendLogString(m_moduleId, MLOG_ERROR, "%s: %s is not a position string.", type, argv[5]);
            return;
         }
         if (MMDAgent_str2rot(argv[6], &rot) == false) {
            sendLogString(m_moduleId, MLOG_ERROR, "%s: %s is not a rotation string.", type, argv[6]);
            return;
         }
         if (MMDAgent_str2pos(argv[7], &pos2) == false) {
            sendLogString(m_moduleId, MLOG_ERROR, "%s: %s is not a position string.", type, argv[7]);
            return;
         }
         if (MMDAgent_str2rot(argv[8], &rot2) == false) {
            sendLogString(m_moduleId, MLOG_ERROR, "%s: %s is not a rotation string.", type, argv[8]);
            return;
         }
         if (addBoneControl(argv[0], MMDAgent_str2float(argv[1]), MMDAgent_str2float(argv[2]), argv[3], argv[4], &pos, &rot, &pos2, &rot2)) {
            /* send message */
            sendMessage(m_moduleId, MMDAGENT_EVENT_MODELBINDBONE, "%s|%s", argv[3], argv[4]);
         }
      }
   } else if (MMDAgent_strequal(type, MMDAGENT_COMMAND_MODELBINDFACE)) {
      // MODEL_BINDFACE|(key name)|(min)|(max)|(model alias)|(face name)|rate1|rate2
      // MODEL_BINDFACE|(model alias)|(face name)|rate[|duration]
      sendLogString(m_moduleId, MLOG_MESSAGE_CAPTURED, "%s|%s", type, value);
      if (num != 3 && num != 4 && num != 7) {
         sendLogString(m_moduleId, MLOG_ERROR, "%s: number of arguments should be 3 or 4 or 7.", type);
         return;
      }
      if (num == 3 || num == 4) {
         if (addMorphControl(NULL, 0.0f, 0.0f, argv[0], argv[1], MMDAgent_str2float(argv[2]), (num == 4) ? MMDAgent_str2float(argv[3]) : 0.0f)) {
            sendMessage(m_moduleId, MMDAGENT_EVENT_MODELBINDFACE, "%s|%s", argv[0], argv[1]);
         }
      } else {
         if (addMorphControl(argv[0], MMDAgent_str2float(argv[1]), MMDAgent_str2float(argv[2]), argv[3], argv[4], MMDAgent_str2float(argv[5]), MMDAgent_str2float(argv[6]))) {
            sendMessage(m_moduleId, MMDAGENT_EVENT_MODELBINDFACE, "%s|%s|%s", argv[0], argv[3], argv[4]);
         }
      }
   } else if (MMDAgent_strequal(type, MMDAGENT_COMMAND_MODELUNBINDBONE)) {
      // MODEL_UNBINDBONE|(model alias)|(bone name)
      sendLogString(m_moduleId, MLOG_MESSAGE_CAPTURED, "%s|%s", type, value);
      if (num != 2) {
         sendLogString(m_moduleId, MLOG_ERROR, "%s: number of arguments should be 2.", type);
         return;
      }
      if (removeBoneControl(argv[0], argv[1])) {
         /* send message */
         sendMessage(m_moduleId, MMDAGENT_EVENT_MODELUNBINDBONE, "%s|%s", argv[0], argv[1]);
      }
   } else if (MMDAgent_strequal(type, MMDAGENT_COMMAND_MODELUNBINDFACE)) {
      // MODEL_UNBINDFACE|(model alias)|(face name)
      sendLogString(m_moduleId, MLOG_MESSAGE_CAPTURED, "%s|%s", type, value);
      if (num != 2) {
         sendLogString(m_moduleId, MLOG_ERROR, "%s: number of arguments should be 2.", type);
         return;
      }
      if (removeMorphControl(argv[0], argv[1])) {
         sendMessage(m_moduleId, MMDAGENT_EVENT_MODELUNBINDFACE, "%s|%s", argv[0], argv[1]);
      }
   } else if (MMDAgent_strequal(type, MMDAGENT_COMMAND_KEYVALUESET)) {
      sendLogString(m_moduleId, MLOG_MESSAGE_CAPTURED, "%s|%s", type, value);
      if (num != 2) {
         sendLogString(m_moduleId, MLOG_ERROR, "%s: number of arguments should be 2.", type);
         return;
      }
      m_keyvalue->setString(argv[0], "%s", argv[1]);
   } else if (MMDAgent_strequal(type, MMDAGENT_COMMAND_LOG_START)) {
      sendLogString(m_moduleId, MLOG_MESSAGE_CAPTURED, "%s|%s", type, value);
      if (m_logToFile->isLogging()) {
         sendLogString(m_moduleId, MLOG_WARNING, "%s: already logging now", type);
      } else {
         char *p = m_content->getContentInfoDup(m_configDirName, "LogIdentifier");
         if (m_logToFile->open(m_configDirName, p ? p : "noId") == false)
            sendLogString(m_moduleId, MLOG_ERROR, "%s: failed to open file for log saving, logging disabled", type);
         sendLogString(m_moduleId, MLOG_STATUS, "start logging in %s...", m_logToFile->getDirName());
         if (p) free(p);
         p = m_content->getContentInfoDup(m_configDirName, "LogSpeechInput");
         if (p && MMDAgent_strequal(p, "true"))
            sendMessage(m_moduleId, MMDAGENT_COMMAND_RECOGRECORDSTART, "%s", m_logToFile->getDirName());
         if (p) free(p);
      }
   } else if (MMDAgent_strequal(type, MMDAGENT_COMMAND_LOG_FINISH)) {
      sendLogString(m_moduleId, MLOG_MESSAGE_CAPTURED, "%s|%s", type, value);
      if (m_logToFile->isLogging() == false) {
         sendLogString(m_moduleId, MLOG_STATUS, "%s: logging is not running", type);
      } else {
         m_logToFile->close();
         sendLogString(m_moduleId, MLOG_STATUS, "stop logging in %s", m_logToFile->getDirName());
         char *p = m_content->getContentInfoDup(m_configDirName, "LogSpeechInput");
         if (p && MMDAgent_strequal(p, "true"))
            sendMessage(m_moduleId, MMDAGENT_COMMAND_RECOGRECORDSTOP, NULL);
         if (p) free(p);
      }
   } else if (MMDAgent_strequal(type, MMDAGENT_COMMAND_LOG_UPLOAD)) {
      sendLogString(m_moduleId, MLOG_MESSAGE_CAPTURED, "%s|%s", type, value);
      if (m_logToFile->isLogging()) {
         sendLogString(m_moduleId, MLOG_WARNING, "%s: you cannot upload while logging is in progress, skipped", type);
      } else {
         if (m_logUploader) {
            /* still running, require refresh */
            m_logUploader->requestRefresh();
         } else {
            char *url = m_content->getContentInfoDup(m_configDirName, "LogUploadURL");
            if (url == NULL) {
               sendLogString(m_moduleId, MLOG_WARNING, "%s: no upload URL specified", type);
            } else {
               char *protover = m_content->getContentInfoDup(m_configDirName, "LogUploadHTTPVersion");
               m_logUploader = new ContentUpload();
               m_logUploader->setupAndStart(this, m_moduleId, url, m_configDirName, protover);
               if (protover)
                  free(protover);
               free(url);
            }
         }
      }
   } else if (MMDAgent_strequal(type, MMDAGENT_COMMAND_RECOGRECORDSTART)) {
      m_logToFile->setRecordingFlag(true);
   } else if (MMDAgent_strequal(type, MMDAGENT_COMMAND_RECOGRECORDSTOP)) {
      m_logToFile->setRecordingFlag(false);
   } else if (MMDAgent_strequal(type, MMDAGENT_COMMAND_BUTTONADD)) {
      /* BUTTON_ADD|alias|scale|x,y|image path|action|(ON or OFF for autoclose) */
      float coord[2];
      sendLogString(m_moduleId, MLOG_MESSAGE_CAPTURED, "%s|%s", type, value);
      if (num != 6) {
         sendLogString(m_moduleId, MLOG_ERROR, "%s: number of arguments should be 6.", type);
         return;
      }
      f = MMDAgent_str2float(argv[1]);
      if (MMDAgent_str2fvec(argv[2], coord, 2) == false) {
         sendLogString(m_moduleId, MLOG_ERROR, "%s: \"%s\" is not in a format: x,y", type, argv[2]);
         return;
      }
      if (MMDAgent_strequal(argv[5], "ON")) {
         bool1 = true;
      } else if (MMDAgent_strequal(argv[5], "OFF")) {
         bool1 = false;
      } else {
         sendLogString(m_moduleId, MLOG_ERROR, "%s: 6th argument should be \"ON\" or \"OFF\".", type);
         return;
      }
      int idx = -1;
      for (int i = 0; i < MMDAGENT_MAXNUMCONTENTBUTTONS; i++) {
         if (idx == -1 && m_contentButtons[i] == NULL)
            idx = i;
         if (m_contentButtons[i] && MMDAgent_strequal(m_contentButtons[i]->getName(), argv[0])) {
            sendLogString(m_moduleId, MLOG_WARNING, "%s: \"%s\" already exists, ignored", type, argv[0]);
            return;
         }
      }
      if (idx == -1) {
         sendLogString(m_moduleId, MLOG_ERROR, "%s: too many content buttons (%d), ignored", type, MMDAGENT_MAXNUMCONTENTBUTTONS);
         return;
      }
      Button *b = new Button;
      if (b->load(this, m_moduleId, argv[0], f, coord) == false) {
         sendLogString(m_moduleId, MLOG_ERROR, "%s: wrong format: %s", type, value);
         return;
      }
      b->setContent(argv[3], argv[4]);
      b->setAutoClose(bool1);
      b->show();
      m_contentButtons[idx] = b;
      sendMessage(m_moduleId, MMDAGENT_EVENT_BUTTONADD, "%s", argv[0]);
   } else if (MMDAgent_strequal(type, MMDAGENT_COMMAND_BUTTONDELETE)) {
      /* BUTTON_DELETE|alias*/
      int i;
      sendLogString(m_moduleId, MLOG_MESSAGE_CAPTURED, "%s|%s", type, value);
      if (num != 1) {
         sendLogString(m_moduleId, MLOG_ERROR, "%s: number of arguments should be 1.", type);
         return;
      }
      for (i = 0; i < MMDAGENT_MAXNUMCONTENTBUTTONS; i++) {
         if (m_contentButtons[i] && MMDAgent_strequal(m_contentButtons[i]->getName(), argv[0])) {
            m_contentButtons[i]->wantDelete();
            break;
         }
      }
      if (i >= MMDAGENT_MAXNUMCONTENTBUTTONS) {
         sendLogString(m_moduleId, MLOG_ERROR, "%s: \"%s\" not found.", type, argv[0]);
         return;
      }
   } else if (MMDAgent_strequal(type, MMDAGENT_COMMAND_MOTIONCAPTURESTART)) {
      /* MOTIONCAPTURE_START|alias|filename.vmd */
      sendLogString(m_moduleId, MLOG_MESSAGE_CAPTURED, "%s|%s", type, value);
      if (num != 2) {
         sendLogString(m_moduleId, MLOG_ERROR, "%s: number of arguments should be 2.", type);
         return;
      }
      startMotionCapture(argv[0], argv[1]);
   } else if (MMDAgent_strequal(type, MMDAGENT_COMMAND_MOTIONCAPTURESTOP)) {
      /* MOTIONCAPTURE_STOP|alias */
      sendLogString(m_moduleId, MLOG_MESSAGE_CAPTURED, "%s|%s", type, value);
      if (num != 1) {
         sendLogString(m_moduleId, MLOG_ERROR, "%s: number of arguments should be 1.", type);
         return;
      }
      stopMotionCapture(argv[0]);
   } else if (MMDAgent_strequal(type, MMDAGENT_COMMAND_SETANIMATIONSPEEDRATE)) {
      /* TEXTURE_SETANIMATIONRATE|alias|textureFileName|rate */
      sendLogString(m_moduleId, MLOG_MESSAGE_CAPTURED, "%s|%s", type, value);
      if (num != 3) {
         sendLogString(m_moduleId, MLOG_ERROR, "%s: number of arguments should be 3.", type);
         return;
      }
      setTexureAnimationSpeedRate(argv[0], argv[1], MMDAgent_str2double(argv[2]));
   } else if (MMDAgent_strequal(type, MMDAGENT_COMMAND_OPENCONTENT)) {
      /* OPEN_CONTENT|url_or_file_path */
      sendLogString(m_moduleId, MLOG_MESSAGE_CAPTURED, "%s|%s", type, value);
      if (num != 1) {
         sendLogString(m_moduleId, MLOG_ERROR, "%s: number of arguments should be 1.", type);
         return;
      }
      char *buf = MMDAgent_fullpathname(value);
      setResetFlag(buf);
      free(buf);
   } else if (MMDAgent_strequal(type, MMDAGENT_COMMAND_SETPARALLELSKINNINGTHREADS)) {
      /* CONFIG_PARALLELSKINNING_THREADS|nThread */
      sendLogString(m_moduleId, MLOG_MESSAGE_CAPTURED, "%s|%s", type, value);
      if (num != 1) {
         sendLogString(m_moduleId, MLOG_ERROR, "%s: number of arguments should be 1.", type);
         return;
      }
      int currentValue = m_option->getParallelSkinningNumthreads();
      m_option->setParallelSkinningNumthreads(MMDAgent_str2int(value));
      if (currentValue != m_option->getParallelSkinningNumthreads()) {
         omp_set_num_threads(m_option->getParallelSkinningNumthreads());
         sendLogString(m_moduleId, MLOG_STATUS, "numthreads for skinning = %d", m_option->getParallelSkinningNumthreads());
      }
   } else if (MMDAgent_strequal(type, MMDAGENT_COMMAND_REOMTEKEYCHAR)) {
      /* REMOTEKEY_CHAR|char */
      sendLogString(m_moduleId, MLOG_MESSAGE_CAPTURED, "%s|%s", type, value);
      if (num != 1) {
         sendLogString(m_moduleId, MLOG_ERROR, "%s: number of arguments should be 1.", type);
         return;
      }
      m_keyHandler.processRemoteChar(value);
   } else if (MMDAgent_strequal(type, MMDAGENT_COMMAND_REOMTEKEYDOWN)) {
      /* REMOTEKEY_DOWN|codestr */
      sendLogString(m_moduleId, MLOG_MESSAGE_CAPTURED, "%s|%s", type, value);
      if (num != 1) {
         sendLogString(m_moduleId, MLOG_ERROR, "%s: number of arguments should be 1.", type);
         return;
      }
      m_keyHandler.processRemoteKeyDown(value);
   } else if (MMDAgent_strequal(type, MMDAGENT_COMMAND_REOMTEKEYUP)) {
      /* REMOTEKEY_UP|codestr */
      sendLogString(m_moduleId, MLOG_MESSAGE_CAPTURED, "%s|%s", type, value);
      if (num != 1) {
         sendLogString(m_moduleId, MLOG_ERROR, "%s: number of arguments should be 1.", type);
         return;
      }
      m_keyHandler.processRemoteKeyUp(value);
   } else if (MMDAgent_strequal(type, MMDAGENT_COMMAND_CAPTION_SETSTYLE)) {
      /* CAPTION_SETSTYLE|style_alias|font|color */
      /* CAPTION_SETSTYLE|style_alias|font|color|edge1|edge2|basecolor */
      float col[4], edge1[5], edge2[5], bscol[4];
      sendLogString(m_moduleId, MLOG_MESSAGE_CAPTURED, "%s|%s", type, value);
      if (num != 3 && num != 6) {
         sendLogString(m_moduleId, MLOG_ERROR, "%s: number of arguments should be 3 or 6.", type);
         return;
      }
      char *fontPath = MMDAgent_strequal(argv[1], "default") ? NULL : argv[1];
      if (MMDAgent_str2fvec(argv[2], col, 4) == false) {
         sendLogString(m_moduleId, MLOG_ERROR, "%s: \"%s\" is not in a format \"r,g,b,a\"", type, argv[2]);
         return;
      }
      if (num == 3) {
         if (m_caption->setStyle(argv[0], fontPath, col, NULL, NULL, NULL) == false) {
            sendLogString(m_moduleId, MLOG_ERROR, "%s: failed to set style: %s", type, value);
            return;
         }
      } else {
         if (MMDAgent_str2fvec(argv[3], edge1, 5) == false) {
            sendLogString(m_moduleId, MLOG_ERROR, "%s: \"%s\" is not in a format \"r,g,b,a,thickness\"", type, argv[3]);
            return;
         }
         if (MMDAgent_str2fvec(argv[4], edge2, 5) == false) {
            sendLogString(m_moduleId, MLOG_ERROR, "%s: \"%s\" is not in a format \"r,g,b,a,thickness\"", type, argv[4]);
            return;
         }
         if (MMDAgent_str2fvec(argv[5], bscol, 4) == false) {
            sendLogString(m_moduleId, MLOG_ERROR, "%s: \"%s\" is not in a format \"r,g,b,a\"", type, argv[5]);
            return;
         }
         if (m_caption->setStyle(argv[0], fontPath, col, edge1, edge2, bscol) == false) {
            sendLogString(m_moduleId, MLOG_ERROR, "%s: failed to set style: %s", type, value);
            return;
         }
      }
      sendMessage(m_moduleId, MMDAGENT_EVENT_CAPTION_SETSTYLE, "%s", argv[0]);
   } else if (MMDAgent_strequal(type, MMDAGENT_COMMAND_CAPTION_START)) {
      /* CAPTION_START|alias|style_alias|text|size|align|height|duration */
      sendLogString(m_moduleId, MLOG_MESSAGE_CAPTURED, "%s|%s", type, value);
      if (num != 7) {
         sendLogString(m_moduleId, MLOG_ERROR, "%s: number of arguments should be 7.", type);
         return;
      }
      CaptionElementConfig config;
      if (MMDAgent_strequal(argv[4], "CENTER")) {
         config.position = CAPTION_POSITION_CENTER;
      } else if (MMDAgent_strequal(argv[4], "LEFT")) {
         config.position = CAPTION_POSITION_SLIDELEFT;
      } else if (MMDAgent_strequal(argv[4], "RIGHT")) {
         config.position = CAPTION_POSITION_SLIDERIGHT;
      } else {
         sendLogString(m_moduleId, MLOG_ERROR, "%s: align parameter should be CENTER, LEFT or RIGHT", type);
         return;
      }
      config.size = MMDAgent_str2float(argv[3]);
      config.height = MMDAgent_str2float(argv[5]);
      config.duration = MMDAgent_str2double(argv[6]);
      if (m_caption->start(argv[0], argv[2], argv[1], config) == false) {
         sendLogString(m_moduleId, MLOG_ERROR, "%s: failed to start: %s", type, value);
         return;
      }
      sendMessage(m_moduleId, MMDAGENT_EVENT_CAPTION_START, "%s", argv[0]);
   } else if (MMDAgent_strequal(type, MMDAGENT_COMMAND_CAPTION_STOP)) {
      /* CAPTION_STOP|alias */
      sendLogString(m_moduleId, MLOG_MESSAGE_CAPTURED, "%s|%s", type, value);
      if (num != 1) {
         sendLogString(m_moduleId, MLOG_ERROR, "%s: number of arguments should be 1.", type);
         return;
      }
      if (m_caption->stop(argv[0]) == false) {
         sendLogString(m_moduleId, MLOG_WARNING, "%s: nothing to stop: %s", type, argv[0]);
      } else {
         sendMessage(m_moduleId, MMDAGENT_EVENT_CAPTION_STOP, "%s", argv[0]);
      }
   } else if (MMDAgent_strequal(type, MMDAGENT_COMMAND_HOME_SET)) {
      /* HOME_SET */
      sendLogString(m_moduleId, MLOG_MESSAGE_CAPTURED, "%s|%s", type, value);
      if (num != 0) {
         sendLogString(m_moduleId, MLOG_ERROR, "%s: should have no argument.", type);
         return;
      }
      m_content->setHomeFile(m_configFileName);
   } else if (MMDAgent_strequal(type, MMDAGENT_COMMAND_HOME_CLEAR)) {
      /* HOME_CLEAR */
      sendLogString(m_moduleId, MLOG_MESSAGE_CAPTURED, "%s|%s", type, value);
      if (num != 0) {
         sendLogString(m_moduleId, MLOG_ERROR, "%s: should have no argument.", type);
         return;
      }
      m_content->clearHome();
   } else if (MMDAgent_strequal(type, MMDAGENT_COMMAND_TRANSPARENT_START)) {
      sendLogString(m_moduleId, MLOG_MESSAGE_CAPTURED, "%s|%s", type, value);
      if (num < 0 || num > 1) {
         sendLogString(m_moduleId, MLOG_ERROR, "%s: number of arguments should be 0 or 1.", type);
         return;
      }
      const float *tcol;
      if (num == 0) {
         tcol = m_option->getTransparentColor();
      } else {
         if (MMDAgent_str2fvec(argv[0], fvec, 3) == false) {
            sendLogString(m_moduleId, MLOG_ERROR, "%s: \"%s\" is not RGB color.", type, argv[0]);
            return;
         }
         tcol = fvec;
      }
      /* enable */
      if (m_offscreen && m_offscreen->enableTransparentWindow(tcol, m_option->getTransparentPixmap()) == false) {
         sendLogString(m_moduleId, MLOG_ERROR, "%s: failed to make screen transparent", type);
         return;
      }
      sendLogString(m_moduleId, MLOG_STATUS, "transparent screen enabled");
   } else if (MMDAgent_strequal(type, MMDAGENT_COMMAND_TRANSPARENT_STOP)) {
      sendLogString(m_moduleId, MLOG_MESSAGE_CAPTURED, "%s", type);
      if (num != 0) {
         sendLogString(m_moduleId, MLOG_ERROR, "%s: should have no argument.", type);
         return;
      }
      if (m_offscreen && m_offscreen->disableTransparentWindow() == false) {
         sendLogString(m_moduleId, MLOG_ERROR, "%s: failed to make screen non-transparent", type);
         return;
      }
      sendLogString(m_moduleId, MLOG_STATUS, "transparent screen disabled");
   }
   if (m_infotext)
      m_infotext->processMessage(type, argv, num);
}

/* MMDAgent::procReceivedLogString: process log string */
void MMDAgent::procReceivedLogString(int id, unsigned int flag, const char *log, const char *timestamp)
{
   const char *modulename;
   const char *flagString;
   char buf[MMDAGENT_MAXBUFLEN];

   if (MMDAgent_strlen(log) <= 0)
      return;

   // compose log string
   modulename = getModuleName(id);
   if (modulename == NULL) {
      sendLogString(m_moduleId, MLOG_ERROR, "InternalError: unknown module id = %d", id);
      return;
   }
   flagString = m_message->getFlagString(flag);
   if (flagString == NULL) {
      sendLogString(m_moduleId, MLOG_ERROR, "InternalError: unknown flag id = %d", flag);
      return;
   }

   MMDAgent_snprintf(buf, MMDAGENT_MAXBUFLEN - 1, "[%s] [%lu] %s: %s: %s\n", timestamp, m_tickCount, modulename, flagString, log);
   buf[MMDAGENT_MAXBUFLEN - 1] = '\0';

#ifdef __ANDROID__
   // write to android log
   switch(flag) {
   case MLOG_ERROR:
      __android_log_print(ANDROID_LOG_ERROR, "MMDAgent", "%s: %s", modulename, log);
      break;
   case MLOG_WARNING:
      __android_log_print(ANDROID_LOG_WARN, "MMDAgent", "%s: %s", modulename, log);
      break;
   default:
      __android_log_print(ANDROID_LOG_VERBOSE, "MMDAgent", "%s: %s", modulename, log);
   }
#else
   if (m_option->getUseStdInOut() == false) {
      // write to stdout
      printf("%s", buf);
   }
#endif /* __ANDROID__ */

   // write to log file
   if (m_fpLog)
      fprintf(m_fpLog, "%s", buf);

   // return here if not enabled
   if(m_enable == false)
      return;

   // add to log window within MMDAgent
   switch (flag) {
   case MLOG_ERROR:
   case MLOG_WARNING:
   case MLOG_STATUS:
      if (m_loggerLog)
         m_loggerLog->log(flag, "%s: %s: %s\n", modulename, flagString, log);
      break;
   case MLOG_MESSAGE_SENT:
   case MLOG_MESSAGE_CAPTURED:
   default:
      if (m_loggerMessage)
         m_loggerMessage->log(flag, "%s: %s: %s\n", modulename, flagString, log);
      break;
   }

   // process in plugin
   if (m_plugin)
      m_plugin->execLog(this, id, flag, log, buf);

   // save log to file
   m_logToFile->save(buf);

   // hold error messages to m_errorMessages
   if (flag == MLOG_ERROR) {
      char* buf = MMDAgent_strdup(m_errorMessages);
      char addstr[MMDAGENT_MAXBUFLEN];
      MMDAgent_snprintf(addstr, MMDAGENT_MAXBUFLEN, "%s: %s\n", modulename, log);
      char *retstr = MMDAgent_strWrapDup(addstr, 76);
      MMDAgent_snprintf(m_errorMessages, MMDAGENT_MAXBUFLEN, "%s%s", buf, retstr);
      m_errorMessages[MMDAGENT_MAXBUFLEN - 1] = '\0';
      free(buf);
      free(retstr);
   }

}

/* MMDAgent::procScrollLogMessage: process log scroll message */
void MMDAgent::procScrollLogMessage(bool up)
{
   int *size = m_option->getLogSize();

   if (m_loggerLog) m_loggerLog->scroll((int) (size[1] * (up == true ? 0.5 : -0.5)));
   if (m_loggerMessage) m_loggerMessage->scroll((int)(size[1] * (up == true ? 0.5 : -0.5)));
}

/* MMDAgent::procDropFileMessage: process file drops message */
void MMDAgent::procDropFileMessage(const char * file, int x, int y)
{
   int i;
   int dropAllowedModelID;
   int targetModelID;

   /* for motion */
   MotionPlayer *motionPlayer;

   if(m_enable == false)
      return;

   if(MMDAgent_strlen(file) <= 0)
      return;

   sendMessage(m_moduleId, MMDAGENT_EVENT_DRAGANDDROP, "%s|%d|%d", file, x, y);

   if (MMDAgent_strtailmatch(file, ".vmd") || MMDAgent_strtailmatch(file, ".VMD")) {
      dropAllowedModelID = -1;
      targetModelID = -1;
      if (m_keyCtrl) {
         /* if Ctrl-key, start motion on all models */
         targetModelID = m_option->getMaxNumModel();
      } else if (m_doubleClicked && m_selectedModel != -1 && m_model[m_selectedModel].allowMotionFileDrop()) {
         targetModelID = m_selectedModel;
      } else {
         targetModelID = m_render->pickModel(m_model, m_numModel, x, y, &dropAllowedModelID); /* model ID in curpor position */
         if (targetModelID == -1)
            targetModelID = dropAllowedModelID;
      }
      if (targetModelID == -1) {
         sendLogString(m_moduleId, MLOG_WARNING, "procDropFileMessage: there is no model at the point.");
      } else {
         if (m_keyShift) { /* if Shift-key, insert motion */
            if (targetModelID == m_option->getMaxNumModel()) {
               /* all model */
               for (i = 0; i < m_numModel; i++) {
                  if (m_model[i].isEnable() && m_model[i].allowMotionFileDrop())
                     addMotion(m_model[i].getAlias(), NULL, file, false, true, true, true, MOTIONMANAGER_DEFAULTPRIORITY);
               }
            } else {
               /* target model */
               if (m_model[targetModelID].isEnable() && m_model[targetModelID].allowMotionFileDrop())
                  addMotion(m_model[targetModelID].getAlias(), NULL, file, false, true, true, true, MOTIONMANAGER_DEFAULTPRIORITY);
               else
                  sendLogString(m_moduleId, MLOG_WARNING, "procDropFileMessage: there is no model at the point.");
            }
         } else {
            /* change base motion */
            if (targetModelID == m_option->getMaxNumModel()) {
               /* all model */
               for (i = 0; i < m_numModel; i++) {
                  if (m_model[i].isEnable() && m_model[i].allowMotionFileDrop()) {
                     for (motionPlayer = m_model[i].getMotionManager()->getMotionPlayerList(); motionPlayer; motionPlayer = motionPlayer->next) {
                        if (motionPlayer->active && MMDAgent_strequal(motionPlayer->name, "base")) {
                           changeMotion(m_model[i].getAlias(), "base", file); /* if 'base' motion is already used, change motion */
                           break;
                        }
                     }
                     if (!motionPlayer)
                        addMotion(m_model[i].getAlias(), "base", file, true, false, true, true, MOTIONMANAGER_DEFAULTPRIORITY);
                  }
               }
            } else {
               /* target model */
               if(m_model[targetModelID].isEnable() && m_model[targetModelID].allowMotionFileDrop()) {
                  if (m_model[targetModelID].getMotionManager()->getRunning("base"))
                     changeMotion(m_model[targetModelID].getAlias(), "base", file); /* if 'base' motion is already used, change motion */
                  else
                     addMotion(m_model[targetModelID].getAlias(), "base", file, true, false, true, true, MOTIONMANAGER_DEFAULTPRIORITY);
               } else {
                  sendLogString(m_moduleId, MLOG_WARNING, "procDropFileMessage: there is no model at the point.");
               }
            }
         }
      }
   } else if (MMDAgent_strtailmatch(file, ".xpmd") || MMDAgent_strtailmatch(file, ".XPMD")) {
      /* load stage */
      setStage(file);
   } else if (MMDAgent_strtailmatch(file, ".pmd") || MMDAgent_strtailmatch(file, ".PMD")) {
      /* drop model */
      if (m_keyCtrl) {
         /* if Ctrl-key, add model */
         addModel(NULL, file, NULL, NULL, true, NULL, NULL);
      } else {
         /* change model */
         if (m_doubleClicked && m_selectedModel != -1) /* already selected */
            targetModelID = m_selectedModel;
         else
            targetModelID = m_render->pickModel(m_model, m_numModel, x, y, &dropAllowedModelID);
         if (targetModelID == -1)
            sendLogString(m_moduleId, MLOG_WARNING, "procDropFileMessage: there is no model at the point.");
         else
            if (m_threadedLoading)
               m_threadedLoading->startModelChangeThread(file, m_model[targetModelID].getAlias());
      }
   } else if (MMDAgent_strtailmatch(file, ".bmp") || MMDAgent_strtailmatch(file, ".tga") || MMDAgent_strtailmatch(file, ".png") || MMDAgent_strtailmatch(file, ".jpg") || MMDAgent_strtailmatch(file, ".jpeg") ||
              MMDAgent_strtailmatch(file, ".BMP") || MMDAgent_strtailmatch(file, ".TGA") || MMDAgent_strtailmatch(file, ".PNG") || MMDAgent_strtailmatch(file, ".JPG") || MMDAgent_strtailmatch(file, ".JPEG")) {
      if (m_keyCtrl)
         setFloor(file); /* change floor with Ctrl-key */
      else
         setBackground(file); /* change background without Ctrl-key */
   } else if (MMDAgent_strtailmatch(file, ".mdf") || MMDAgent_strtailmatch(file, ".MDF")) {
      /* restart */
      setResetFlag(file);
   }
}

struct saveSnapData {
   char *filename;
   GLubyte *pixels;
   int width;
   int height;
};

/* thread function for saving snapshot */
static void saveSnapShotMain(void *param)
{
   saveSnapData *data = (saveSnapData *)param;
   PMDTexture *tex;

   tex = new PMDTexture();
   tex->savePNG(data->pixels, data->width, data->height, data->filename);
   delete tex;
   delete[] data->pixels;
   free(data->filename);
   free(data);
}

/* MMDAgent::saveScreenShot: save screen shot */
void MMDAgent::saveScreenShot(const char *filename)
{
   saveSnapData *data;
   GLFWthread id;

   if (filename == NULL)
      return;
   data = (saveSnapData *)malloc(sizeof(saveSnapData));
   data->filename = MMDAgent_strdup(filename);
   getWindowSize(&(data->width), &(data->height));
   data->pixels = new GLubyte[3 * data->width * data->height];
   glReadPixels(0, 0, data->width, data->height, GL_RGB, GL_UNSIGNED_BYTE, data->pixels);
   id = glfwCreateThread(saveSnapShotMain, data);
   if (id == -1) {
      sendLogString(m_moduleId, MLOG_WARNING, "failed to create thread for taking snapshot");
      delete[] data->pixels;
      free(data->filename);
      free(data);
   }
}

/* historyMenuHandler: history menu handler callback */
static void historyMenuHandler(int id, int item, void *data)
{
   char buff[MMDAGENT_MAXBUFLEN];
   MMDAgent *mmdagent = (MMDAgent *)data;
   char *historyFile;
   KeyValue *v;
   const char *mdfpath;

   /* restart with the content mdf file */
   historyFile = getHistoryFilePathDup();
   if (historyFile) {
      v = new KeyValue;
      v->setup();
      v->load(historyFile, NULL);
      MMDAgent_snprintf(buff, MMDAGENT_MAXBUFLEN, "mdf%d", item);
      mdfpath = v->getString(buff, NULL);
      if (mdfpath)
         mmdagent->setResetFlag(mdfpath);
      free(historyFile);
   }
}

/* MMDAgent::callHistory: call history */
void MMDAgent::callHistory()
{
   char buff[MMDAGENT_MAXBUFLEN];
   char buff2[MMDAGENT_MAXBUFLEN];
   char *historyFile;
   const char *contentDir;
   KeyValue *vold, *v;
   bool updated = false;
   int id;
   int i, k;
   const char *pp, *ip, *timestr;
   KeyValue *desc;

   if (m_menu == NULL)
      return;

   /* prepare a temporal stem to menu (will disapper when closed) */
   id = m_menu->find("<History>");
   if (id == -1)
      id = m_menu->add("<History>", MENUPRIORITY_TEMPORAL, historyMenuHandler, this);

   historyFile = getHistoryFilePathDup();

   if (historyFile) {
      /* load history */
      vold = new KeyValue;
      vold->setup();
      vold->load(historyFile, NULL);

      /* delete non-existing item */
      v = new KeyValue;
      v->setup();
      k = 0;
      for (i = 0; i < HISTORYMAXNUM; i++) {
         MMDAgent_snprintf(buff, MMDAGENT_MAXBUFLEN, "dir%d", i);
         pp = vold->getString(buff, NULL);
         if (pp) {
            if (MMDAgent_stat(pp) == MMDAGENT_STAT_DIRECTORY) {
               MMDAgent_snprintf(buff2, MMDAGENT_MAXBUFLEN, "dir%d", k);
               v->setString(buff2, "%s", pp);
               MMDAgent_snprintf(buff, MMDAGENT_MAXBUFLEN, "mdf%d", i);
               pp = vold->getString(buff, NULL);
               MMDAgent_snprintf(buff2, MMDAGENT_MAXBUFLEN, "mdf%d", k);
               v->setString(buff2, "%s", pp);
               MMDAgent_snprintf(buff, MMDAGENT_MAXBUFLEN, "time%d", i);
               pp = vold->getString(buff, NULL);
               if (pp) {
                  MMDAgent_snprintf(buff2, MMDAGENT_MAXBUFLEN, "time%d", k);
                  v->setString(buff2, "%s", pp);
               }
               k++;
            } else {
               updated = true;
            }
         }
      }
      if (updated) {
         v->save(historyFile);
      }
      delete vold;

      /* add history entries as item */
      for (i = 0; i < HISTORYMAXNUM; i++) {
         MMDAgent_snprintf(buff, MMDAGENT_MAXBUFLEN, "mdf%d", i);
         pp = v->getString(buff, NULL);
         MMDAgent_snprintf(buff, MMDAGENT_MAXBUFLEN, "dir%d", i);
         contentDir = v->getString(buff, NULL);
         MMDAgent_snprintf(buff, MMDAGENT_MAXBUFLEN, "time%d", i);
         timestr = v->getString(buff, NULL);
         /* construct menu item from content information */
         if (contentDir) {
            desc = m_content->getContentInfoNew(contentDir);
            if (desc) {
               /* the directory has saved content info, follow it */
               if (desc->exist("ContentName"))
                  MMDAgent_snprintf(buff, MMDAGENT_MAXBUFLEN, "%s", desc->getString("ContentName", ""));
               else
                  buff[0] = '\0';
               ip = desc->getString("ImageFile", NULL);
               if (ip) {
                  MMDAgent_snprintf(buff2, MMDAGENT_MAXBUFLEN, "%s%c%s", contentDir, MMDAGENT_DIRSEPARATOR, ip);
               } else {
                  MMDAgent_snprintf(buff2, MMDAGENT_MAXBUFLEN, "%s%c%s", contentDir, MMDAGENT_DIRSEPARATOR, CONTENTMANAGER_DEFAULTBANNERIMAGEFILE);
               }
               m_menu->setItem(id, i, buff, buff2, NULL, NULL, timestr);
               delete desc;
            } else {
               desc = new KeyValue;
               desc->setup();
               MMDAgent_snprintf(buff, MMDAGENT_MAXBUFLEN, "%s%c%s", contentDir, MMDAGENT_DIRSEPARATOR, CONTENTMANAGER_PACKAGEFILE);
               if (desc->load(buff, g_enckey) == true) {
                  /* the directory has PACKAGE_DESC.txt, follow it */
                  if (desc->exist("label"))
                     MMDAgent_snprintf(buff, MMDAGENT_MAXBUFLEN, "%s", desc->getString("label", ""));
                  else
                     buff[0] = '\0';
                  ip = desc->getString("image", NULL);
                  if (ip) {
                     MMDAgent_snprintf(buff2, MMDAGENT_MAXBUFLEN, "%s%c%s", contentDir, MMDAGENT_DIRSEPARATOR, ip);
                  } else {
                     MMDAgent_snprintf(buff2, MMDAGENT_MAXBUFLEN, "%s%c%s", contentDir, MMDAGENT_DIRSEPARATOR, CONTENTMANAGER_DEFAULTBANNERIMAGEFILE);
                  }
                  m_menu->setItem(id, i, buff, buff2, NULL, NULL, timestr);
               } else {
                  /* if no PACKAGE_DESC.txt, apply default */
                  MMDAgent_snprintf(buff2, MMDAGENT_MAXBUFLEN, "%s%c%s", contentDir, MMDAGENT_DIRSEPARATOR, CONTENTMANAGER_DEFAULTBANNERIMAGEFILE);
                  m_menu->setItem(id, i, contentDir, buff2, NULL, NULL, timestr);
               }
               delete desc;
            }
         }
      }
      free(historyFile);
      delete v;
   }

   /* set the stem as current, in full screen, and disabling stem flipping */
   m_menu->jump(id);
   m_menu->disableForwardBackwardTillHide();
   m_menu->show();

}

/* MMDAgent::getUserContext: get user context */
void MMDAgent::getUserContext(const char *mdffile)
{
   char *lastPlayedFile;
   KeyValue *v;
   const char *p;
   char buff[MMDAGENT_MAXBUFLEN];

   lastPlayedFile = getLastPlayedDataPathDup();
   if (lastPlayedFile == NULL)
      return;

   /* load existing db */
   v = new KeyValue;
   v->setup();
   v->loadText(lastPlayedFile, NULL);

   /* get last played time of the content */
   if (v->exist(mdffile)) {
      int year, month, day, hour, minute, sec, msec;
      MMDAgent_gettimeinfo(&year, &month, &day, &hour, &minute, &sec, &msec);
      p = v->getString(mdffile, NULL);
      MMDAgent_snprintf(buff, MMDAGENT_MAXBUFLEN, "%s", p);
      buff[4] = buff[7] = buff[10] = buff[13] = '\0';
      int dyear  = MMDAgent_str2int(&(buff[0]));
      int dmonth = MMDAgent_str2int(&(buff[5]));
      int dday   = MMDAgent_str2int(&(buff[8]));
      int dhour  = MMDAgent_str2int(&(buff[11]));
      if (dyear == year && dmonth == month && dday == day) {
         m_keyvalue->setString("UserContext_FirstLaunchOfToday", "false");
      } else {
         m_keyvalue->setString("UserContext_FirstLaunchOfToday", "true");
      }
      m_keyvalue->setString("UserContext_InitialLaunch", "false");
   } else {
      m_keyvalue->setString("UserContext_FirstLaunchOfToday", "true");
      m_keyvalue->setString("UserContext_InitialLaunch", "true");
   }

   free(lastPlayedFile);
   delete v;
}

/* MMDAgent::procOpenContentDirMessage: process open content dir message */
void MMDAgent::procOpenContentDirMessage()
{
   if (m_configDirName == NULL)
      return;

   if (MMDAgent_openExternal(m_configDirName, NULL) == false) {
      sendLogString(m_moduleId, MLOG_ERROR, "failed to open directory with external application");
   }
}

/* MMDAgent::procOpenContentFileMessage: process open content filedir message */
void MMDAgent::procOpenContentFileMessage()
{
   char *contentFile = NULL;
   int len;

   if (m_configDirName == NULL || m_configFileName == NULL)
      return;

   contentFile = MMDAgent_strdup(m_keyvalue->getString("ContentFile", NULL));
   if (contentFile == NULL) {
      len = MMDAgent_strlen(m_configFileName);
      if (len > 4) {
         contentFile = MMDAgent_strdup(m_configFileName);
         contentFile[len - 4] = '.';
         contentFile[len - 3] = 'f';
         contentFile[len - 2] = 's';
         contentFile[len - 1] = 't';
      }
   }
   if (contentFile) {
      if (MMDAgent_openExternal(contentFile, NULL) == false)
         sendLogString(m_moduleId, MLOG_ERROR, "failed to open scenario file with external application");
      free(contentFile);
   } else {
      sendLogString(m_moduleId, MLOG_ERROR, "failed to find current scenario to open in external application");
   }
}

/* MMDAgent::procSaveWindowPlacementMessage: process save window placement message */
void MMDAgent::procSaveWindowPlacementMessage()
{
   if (m_enable == false)
      return;

   m_screen->saveWindowPosition();
}

/* MMDAgent::procLoadWindowPlacementMessage: process load window placement message */
void MMDAgent::procLoadWindowPlacementMessage()
{
   bool fullscreen;

   if (m_enable == false)
      return;

   if (m_screen->loadWindowPosition(&fullscreen))
      m_option->setFullScreen(fullscreen);
}

/* MMDAgent::procToggleTitleBarMessage: process toggle title bar message */
void MMDAgent::procToggleTitleBarMessage()
{
   if (m_enable == false)
      return;

   m_screen->toggleTitleBar();
}


/* MMDAgent::setLoadHomeFlag: set load home flag */
void MMDAgent::setLoadHomeFlag(bool flag)
{
   m_loadHome = flag;
}

/* MMDAgent::getLoadHomeFlag: get load home flag */
bool MMDAgent::getLoadHomeFlag()
{
   return m_loadHome;
}

/* MMDAgent::getHomeDup: get home */
char *MMDAgent::getHomeDup()
{
   return m_content->getHomeURLdup(NULL);
}

/* MMDAgent::procToggleDoppelShadowMessage: process toggle doppel shadow message */
void MMDAgent::procToggleDoppelShadowMessage()
{
   if (m_enable == false)
      return;
   m_option->setUseDoppelShadow(!m_option->getUseDoppelShadow());
   m_render->setDoppelShadowFlag(m_option->getUseDoppelShadow());
}

/* MMDAgent::setLog2DFlag: set log 2d flag */
void MMDAgent::setLog2DFlag(bool flag)
{
   if (m_loggerLog)
      m_loggerLog->set2dflag(flag);
   if (m_loggerMessage)
      m_loggerMessage->set2dflag(flag);
}
/* MMDAgent::getLogBaseHeight: set log base height */
float MMDAgent::getLogBaseHeight()
{
   float ret = -1.0f;
   float h;

   if (m_loggerLog) {
      h = m_loggerLog->getHeightBase();
      if (ret == -1.0f || ret > h)
         ret = h;
   }
   if (m_loggerMessage) {
      h = m_loggerMessage->getHeightBase();
      if (ret == -1.0f || ret > h)
         ret = h;
   }
   return ret;
}

/* MMDAgent::getBulletPhysics: get bullet physics instance */
BulletPhysics *MMDAgent::getBulletPhysics()
{
   return m_bullet;
}

/* MMDAgent::getSystemTexture: get system texture */
SystemTexture *MMDAgent::getSystemTexture()
{
   return m_systex;
}

/* MMDAgent::getStartingFrameLeft: get starting animation frame left */
double MMDAgent::getStartingFrameLeft()
{
   return m_startingFrame;
}

/* MMDAgent::setOptionalStatusString: set optional status string */
void MMDAgent::setOptionalStatusString(const char *str)
{
   MMDAgent_snprintf(m_optionalStatusString, MMDAGENT_MAXBUFLEN, "%s", str);
}

/* MMDAgent::getKeyHandler: get key handler */
KeyHandler *MMDAgent::getKeyHandler()
{
   return &m_keyHandler;
}

/* MMDAgent::procToggleTransparent: process toggle transparent window */
void MMDAgent::procToggleTransparent()
{
   if (m_enable == false)
      return;

   if (m_offscreen == NULL)
      return;

   if (m_option == NULL)
      return;

   if (m_offscreen->isTransparentWindow()) {
      if (m_offscreen->disableTransparentWindow() == false) {
         sendLogString(m_moduleId, MLOG_ERROR, "failed to make screen non-transparent");
         return;
      }
      sendLogString(m_moduleId, MLOG_STATUS, "transparent screen disabled");
   } else {
      if (m_offscreen->enableTransparentWindow(m_option->getTransparentColor(), m_option->getTransparentPixmap()) == false) {
         sendLogString(m_moduleId, MLOG_ERROR, "failed to make screen transparent");
         return;
      }
      sendLogString(m_moduleId, MLOG_STATUS, "transparent screen enabled");
   }
}

/* LogToFile::initialize: initialize LogToFile */
void LogToFile::initialize()
{
   m_fp = NULL;
   m_dirName = NULL;
   m_totalSize = 0;
   m_maxSize = 10 * 1024 * 1024;  // limit maximum log size to 10MB (approx. 260k lines)
   m_recording = false;
}

/* LogToFile::clear: free LogToFile */
void LogToFile::clear()
{
   if (m_dirName)
      free(m_dirName);
   if (m_fp)
      close();
   initialize();
}

/* LogToFile::LogToFile: constructor */
LogToFile::LogToFile()
{
   initialize();
}

/* LogToFile::~LogToFile: destructor */
LogToFile::~LogToFile()
{
   clear();
}

/* LogToFile::open: open log file for writing */
bool LogToFile::open(const char *contentDirName, const char *logIdentifier)
{
   char buf[MMDAGENT_MAXBUFLEN];
   char buf2[MMDAGENT_MAXBUFLEN];
   FILE *fp;

   clear();

   /* prepare top log directory  */
   MMDAgent_snprintf(buf, MMDAGENT_MAXBUFLEN, "%s%c%s", contentDirName, MMDAGENT_DIRSEPARATOR, MMDAGENT_LOGFILEDIRNAME);
   if (MMDAgent_existdir(buf) == false) {
      if (MMDAgent_mkdir(buf) == false)
         return false;
   }

   /* prepare log-identifier named log dir under it */
   MMDAgent_snprintf(buf, MMDAGENT_MAXBUFLEN, "%s%c%s%c%s", contentDirName, MMDAGENT_DIRSEPARATOR, MMDAGENT_LOGFILEDIRNAME, MMDAGENT_DIRSEPARATOR, logIdentifier);
   if (MMDAgent_existdir(buf) == false) {
      if (MMDAgent_mkdir(buf) == false)
         return false;
   }

   if (m_dirName)
      free(m_dirName);
   m_dirName = MMDAgent_strdup(buf);

   /* open log file */
   MMDAgent_gettimestampstr(buf2, MMDAGENT_MAXBUFLEN, "%4d_%02d_%02d__%02d_%02d_%02d__%03d");
   MMDAgent_snprintf(buf, MMDAGENT_MAXBUFLEN, "%s%clog_%s.txt", m_dirName, MMDAGENT_DIRSEPARATOR, buf2);
   fp = MMDAgent_fopen(buf, "w");
   if (fp == NULL)
      return false;

   m_fp = fp;

   m_totalSize = 0;

   return true;
}

/* LogToFile::save: save log message to file */
void LogToFile::save(const char *string)
{
   char buf[MMDAGENT_MAXBUFLEN];

   if (m_fp == NULL)
      return;

   MMDAgent_snprintf(buf, MMDAGENT_MAXBUFLEN, "%s", string);
   if (fputs(buf, m_fp) < 0) {
      close();
   }

   m_totalSize += MMDAgent_strlen(buf);
   if (m_totalSize > m_maxSize) {
      MMDAgent_snprintf(buf, MMDAGENT_MAXBUFLEN, "ERROR: log size reached limit (%.2f MB), terminate logging now!\n", (float)m_maxSize / (1024.0 * 1024.0));
      fputs(buf, m_fp);
      close();
   }
}

/* LogToFile::close: close log file */
void LogToFile::close()
{
   if (m_fp == NULL)
      return;

   fclose(m_fp);

   m_fp = NULL;
}

/* LogToFile::getDirName: return dir name */
const char *LogToFile::getDirName()
{
   return m_dirName;
}

/* LogToFile::isLogging: return true while logging */
bool LogToFile::isLogging()
{
   if (m_fp)
      return true;
   return false;
}

/* LogToFile::setRecordingFlag: set recording flag */
void LogToFile::setRecordingFlag(bool flag)
{
   m_recording = flag;
}

/* LogToFile::getRecordingFlag: get recording flag */
bool LogToFile::getRecordingFlag()
{
   return m_recording;
}
