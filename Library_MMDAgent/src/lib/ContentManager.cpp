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

/* headers */
#define POCO_NO_AUTOMATIC_LIBS
#ifdef _WIN32
/* use static library built with MMDAgent-EX */
#define POCO_STATIC
#endif
#include "Poco/RegularExpression.h"
#include "Poco/Exception.h"

#include <time.h>
#include "MMDAgent.h"

#define CONTENTMANAGER_BOOKMARKNUM MENUMAXITEM

/* ContentManager::initialize: initialize content manager */
void ContentManager::initialize()
{
   m_mmdagent = NULL;
   m_id = 0;
   m_cacheDirList = NULL;
   m_currentContentDir = NULL;
   m_currentMdfFile = NULL;
   m_currentSourceURL = NULL;
   m_currentBookmarkAddItemId = 0;
   m_menu = NULL;
   m_zip = NULL;
   m_web = NULL;
   m_contentUpdater = NULL;
   m_hasFinished = false;
   m_hasError = false;
   m_needsReset = false;
   m_mdfFile = NULL;
   m_contentMessage = NULL;
   m_list = NULL;
   m_banList = NULL;
   m_font = NULL;
   memset(&m_elem, 0, sizeof(FTGLTextDrawElements));
   m_fontError = false;
}

/* ContentManager::clear: free content manager */
void ContentManager::clear()
{
   int i;
   CMList *l, *tmp;
   BanList *b, *btmp;

   if (m_cacheDirList) {
      for (i = 0; i < CONTENTMANAGER_BOOKMARKNUM; i++) {
         if (m_cacheDirList[i].dir)
            free(m_cacheDirList[i].dir);
         if (m_cacheDirList[i].name)
            free(m_cacheDirList[i].name);
         if (m_cacheDirList[i].file)
            free(m_cacheDirList[i].file);
         if (m_cacheDirList[i].imagefile)
            free(m_cacheDirList[i].imagefile);
         if (m_cacheDirList[i].source)
            free(m_cacheDirList[i].source);
         if (m_cacheDirList[i].docfile)
            free(m_cacheDirList[i].docfile);
      }
      free(m_cacheDirList);
   }
   if (m_currentSourceURL)
      free(m_currentSourceURL);
   if (m_currentMdfFile)
      free(m_currentMdfFile);
   if (m_currentContentDir)
      free(m_currentContentDir);
   if (m_zip)
      delete m_zip;
   if (m_web)
      delete m_web;
   if (m_contentUpdater)
      delete m_contentUpdater;
   if (m_mdfFile)
      free(m_mdfFile);
   if (m_contentMessage)
      free(m_contentMessage);
   l = m_list;
   while (l) {
      tmp = l->next;
      delete l;
      l = tmp;
   }
   b = m_banList;
   while (b) {
      btmp = b->next;
      if (b->urlex)
         free(b->urlex);
      delete b;
      b = btmp;
   }
   if (m_elem.vertices) free(m_elem.vertices);
   if (m_elem.texcoords) free(m_elem.texcoords);
   if (m_elem.indices) free(m_elem.indices);

   initialize();
}

/* ContentManager::ContentManager: constructor */
ContentManager::ContentManager()
{
   initialize();
}

/* ContentManager::~ContentManager: destructor */
ContentManager::~ContentManager()
{
   clear();
}

/* ContentManager::setup: initialize and setup content manager */
void ContentManager::setup(MMDAgent *mmdagent, int id)
{
   clear();
   m_mmdagent = mmdagent;
   m_id = id;
   m_font = mmdagent->getTextureFont();
}

/* ContentManager::checkContentComplete: check content complete */
bool ContentManager::checkContentComplete(const char *savedir)
{
   char buff[MMDAGENT_MAXBUFLEN];
   KeyValue *prop;
   bool ret = false;

   MMDAgent_snprintf(buff, MMDAGENT_MAXBUFLEN, "%s%c%s", savedir, MMDAGENT_DIRSEPARATOR, MMDAGENT_CONTENTINFOFILE);
   prop = new KeyValue;
   prop->setup();
   if (prop->load(buff, NULL))
      if (MMDAgent_strequal(prop->getString("DownloadCompleted", "false"), "true"))
         ret = true;
   delete prop;
   return ret;
}

/* ContentManager::startExtractContent: start extracting content from uri and save */
bool ContentManager::startExtractContent(const char *uri_or_path, const char *savedir, bool resetAfterExtraction, bool fetchContentListOnly, bool preserve)
{
   bool started = false;

   if (MMDAgent_strtailmatch(uri_or_path, ".mmda")) {
      /* start unpacking thread for the path, saving to savedir */
      if (m_zip) {
         /* not allow more than one content */
         m_mmdagent->sendLogString(m_id, MLOG_ERROR, "duplicate thread, failed to load %s", uri_or_path);
         return false;
      }
      m_hasError = false;
      m_needsReset = resetAfterExtraction;
      m_hasFinished = false;
      m_zip = new ContentManagerThreadZip;
      m_zip->setupAndStart(m_mmdagent, m_id, uri_or_path, savedir);
      started = true;
   } else if (MMDAgent_strheadmatch(uri_or_path, "http://") || MMDAgent_strheadmatch(uri_or_path, "https://") || MMDAgent_strheadmatch(uri_or_path, "mmdagent://")) {
      /* start http downloading thread, saving to savedir */
      if (m_web) {
         /* not allow more than one content */
         m_mmdagent->sendLogString(m_id, MLOG_ERROR, "duplicate thread, failed to load %s", uri_or_path);
         return false;
      }
      m_hasError = false;
      m_needsReset = resetAfterExtraction;
      m_hasFinished = false;
      m_web = new ContentManagerThreadWeb;
      if (MMDAgent_strheadmatch(uri_or_path, "mmdagent://")) {
         char *uri = MMDAgent_strdup(uri_or_path);
         uri[4] = 'h';
         uri[5] = 't';
         uri[6] = 't';
         uri[7] = 'p';
         m_web->setupAndStart(m_mmdagent, m_id, uri + 4, savedir, fetchContentListOnly, preserve);
         free(uri);
      } else {
         m_web->setupAndStart(m_mmdagent, m_id, uri_or_path, savedir, fetchContentListOnly, preserve);
      }
      started = true;
   }

   return(started);
}

/* ContentManager::setupLocalPackageInfo: setup local packge info into content info  */
void ContentManager::setupLocalPackageInfo(const char *dir)
{
   char buff[MMDAGENT_MAXBUFLEN];
   KeyValue *desc, *prop;

   /* extract package description */
   desc = new KeyValue;
   desc->setup();
   MMDAgent_snprintf(buff, MMDAGENT_MAXBUFLEN, "%s%c%s", dir, MMDAGENT_DIRSEPARATOR, CONTENTMANAGER_PACKAGEFILE);
   if (desc->load(buff, g_enckey) == false) {
      delete desc;
      return;
   }

   /* load already existing content information if any */
   MMDAgent_snprintf(buff, MMDAGENT_MAXBUFLEN, "%s%c%s", dir, MMDAGENT_DIRSEPARATOR, MMDAGENT_CONTENTINFOFILE);
   prop = new KeyValue;
   prop->setup();

   /* skip if content info file does not exist (in case this is local content, not a cache of web content */
   if (prop->load(buff, NULL) == false) {
      delete desc;
      delete prop;
      return;
   }

   /* store/update content information */
   prop->setString("ContentType", "%s", "local");
   if (desc->exist("label"))
      prop->setString("ContentName", "%s", desc->getString("label", ""));
   if (desc->exist("readme"))
      prop->setString("Readme", desc->getString("readme", ""));
   if (desc->exist("nonBrowse"))
      prop->setString("NonBrowse", desc->getString("nonBrowse", "false"));
   if (desc->exist("logUploadURL"))
      prop->setString("LogUploadURL", desc->getString("logUploadURL", ""));
   if (desc->exist("logUploadHTTPVersion"))
      prop->setString("LogUploadHTTPVersion", desc->getString("logUploadHTTPVersion", ""));
   if (desc->exist("logIdentifier"))
      prop->setString("LogIdentifier", desc->getString("logIdentifier", ""));
   if (desc->exist("logSpeechInput"))
      prop->setString("LogSpeechInput", desc->getString("logSpeechInput", "false"));
   if (desc->exist("readmeForceAgreement"))
      prop->setString("ReadmeForceAgreement", desc->getString("readmeForceAgreement", "false"));
   if (desc->exist("kafkaBroker"))
      prop->setString("KafkaBroker", desc->getString("kafkaBroker", ""));
   if (desc->exist("kafkaCodec"))
      prop->setString("KafkaCodec", desc->getString("kafkaCodec", ""));
   if (desc->exist("kafkaProducerTopic"))
      prop->setString("KafkaProducerTopic", desc->getString("kafkaProducerTopic", ""));
   if (desc->exist("kafkaConsumerTopic"))
      prop->setString("KafkaConsumerTopic", desc->getString("kafkaConsumerTopic", ""));
   if (desc->exist("kafkaPartition"))
      prop->setString("KafkaPartition", desc->getString("kafkaPartition", ""));

   prop->save(buff);
   delete prop;
   delete desc;
}


/* ContentManager::getContentInfoNew: get newly allocated content info */
KeyValue *ContentManager::getContentInfoNew(const char *dir)
{
   char buff1[MMDAGENT_MAXBUFLEN];
   char buff2[MMDAGENT_MAXBUFLEN];
   KeyValue *prop;
   int i, len, index = 0;
   char size;

   prop = new KeyValue;
   prop->setup();
   strcpy(buff1, dir);

   do {
      MMDAgent_snprintf(buff2, MMDAGENT_MAXBUFLEN, "%s%c%s", buff1, MMDAGENT_DIRSEPARATOR, MMDAGENT_CONTENTINFOFILE);
      if (prop->load(buff2, NULL))
         break;
      index = -1;
      len = MMDFiles_strlen(buff1);
      for (i = 0; i < len; i += size) {
         size = MMDFiles_getcharsize(&buff1[i]);
         if (size == 1 && MMDFiles_dirseparator(buff1[i]) == true)
            index = i;
      }
      if (index >= 0)
         buff1[index] = '\0';
   } while (index >= 0);

   if (index < 0) {
      delete prop;
      return NULL;
   }

   return prop;
}

/* ContentManager::outputContentInfoToLog: output content information to log */
void ContentManager::outputContentInfoToLog(const char *dir)
{
   KeyValue *prop;
   void *save;
   const char *key;

   prop = getContentInfoNew(dir);
   if (prop) {
      m_mmdagent->sendLogString(m_id, MLOG_STATUS, "Content Info Begin\n");
      for (key = prop->firstKey(&save); key; key = prop->nextKey(&save)) {
         m_mmdagent->sendLogString(m_id, MLOG_STATUS, "  %s=%s\n", key, prop->getString(key, ""));
      }
      m_mmdagent->sendLogString(m_id, MLOG_STATUS, "Content Info End\n");
      delete prop;
   }
}

/* ContentManager::getContentInfoDup: return allocated string for specified keyname */
char *ContentManager::getContentInfoDup(const char *dir, const char *keyname)
{
   KeyValue *prop;
   char *p;

   prop = getContentInfoNew(dir);
   if (prop == NULL)
      return NULL;
   p = MMDAgent_strdup(prop->getString(keyname, NULL));
   delete prop;
   return p;
}

/* ContentManager::showDocOnFirstTime: show document if this is the first launch */
bool ContentManager::showDocOnFirstTime(const char *dir)
{
   KeyValue *prop, *prop_user;
   char buff[MMDAGENT_MAXBUFLEN];
   char buff2[MMDAGENT_MAXBUFLEN];
   time_t extracted, lastPlayed;
   bool ret;

   if (m_contentMessage) {
      m_mmdagent->getInfoText()->setText("Info", m_contentMessage, "OK");
      m_mmdagent->getInfoText()->show();
      return true;
   }

   prop = new KeyValue;
   prop->setup();
   MMDAgent_snprintf(buff, MMDAGENT_MAXBUFLEN, "%s%c%s", dir, MMDAGENT_DIRSEPARATOR, MMDAGENT_CONTENTINFOFILE);
   if (prop->load(buff, NULL) == false) {
      delete prop;
      return false;
   }
   prop_user = new KeyValue;
   prop_user->setup();
   MMDAgent_snprintf(buff, MMDAGENT_MAXBUFLEN, "%s%c%s", dir, MMDAGENT_DIRSEPARATOR, MMDAGENT_CONTENTUSERINFOFILE);
   if (prop_user->load(buff, NULL) == false) {
      lastPlayed = 0;
   } else {
      lastPlayed = (time_t)atoll(prop_user->getString("LastPlayEpochTime", "0"));
   }
   delete prop_user;

   ret = false;
   if (prop->exist("Readme") && prop->exist("LastModifiedEpochTime")) {
      extracted = (time_t)atoll(prop->getString("LastModifiedEpochTime", "0"));
      if (lastPlayed < extracted) {
         MMDAgent_snprintf(buff2, MMDAGENT_MAXBUFLEN, "%s%c%s", dir, MMDAGENT_DIRSEPARATOR, prop->getString("Readme", ""));
         if (MMDAgent_strequal(prop->getString("ReadmeForceAgreement", "false"), "true")) {
            m_mmdagent->getInfoText()->load(buff2, "ReadMe", "Accept|Decline");
            m_mmdagent->getInfoText()->setAgreementFlag(true);
         } else {
            m_mmdagent->getInfoText()->load(buff2, "ReadMe", "OK");
            m_mmdagent->getInfoText()->setAgreementFlag(false);
         }
         m_mmdagent->getInfoText()->show();
         ret = true;
      }
   }
   if (ret == false)
      saveLastPlayedTime(dir);

   delete prop;

   return ret;
}

/* ContentManager::saveLastPlayedTime: save last played time */
void ContentManager::saveLastPlayedTime(const char *dir)
{
   KeyValue *prop;
   char buff[MMDAGENT_MAXBUFLEN];
   time_t current_time;

   prop = new KeyValue;
   prop->setup();
   MMDAgent_snprintf(buff, MMDAGENT_MAXBUFLEN, "%s%c%s", dir, MMDAGENT_DIRSEPARATOR, MMDAGENT_CONTENTUSERINFOFILE);
   prop->load(buff, NULL);
   time(&current_time);
   prop->setString("LastPlayEpochTime", "%u", current_time);
   prop->save(buff);
   delete prop;
}

/* ContentManager::updateAndRender: update and render */
void ContentManager::updateAndRender()
{
   char buff[MMDAGENT_MAXBUFLEN];
   float rate;

   /* return immediately if no thread exists */
   if (m_zip == NULL && m_web == NULL)
      return;

   /* check and delete threads that has been finished running since last call */
   if (m_zip && m_zip->isFinished()) {
      if (m_zip->hasError())
         m_hasError = true;
      m_mdfFile = MMDAgent_strdup(m_zip->getContentMDFFile());
      delete m_zip;
      m_zip = NULL;
      m_hasFinished = true;
   }
   if (m_web && m_web->isFinished()) {
      if (m_web->hasError())
         m_hasError = true;
      m_mdfFile = MMDAgent_strdup(m_web->getContentMDFFile());
      m_contentMessage = MMDAgent_strdup(m_web->getContentMessage());
      delete m_web;
      m_web = NULL;
      m_hasFinished = true;
   }

   if (m_zip == NULL && m_web == NULL)
      return;

   /* call rendering functions for existing threads */
   renderBegin();
   if (m_zip) {
      m_zip->getProgress(buff, MMDAGENT_MAXBUFLEN, &rate);
      render(buff, rate);
   }
   if (m_web) {
      m_web->getProgress(buff, MMDAGENT_MAXBUFLEN, &rate);
      render(buff, rate);
   }
   renderEnd();
}

/* ContentManager::isRunning: return true when running */
bool ContentManager::isRunning()
{
   bool ret = false;

   if (m_zip || m_web)
      ret = true;

   return ret;
}

/* ContentManager::hasFinished: return true when finished at last update */
bool ContentManager::hasFinished()
{
   return m_hasFinished;
}

/* ContentManager::wasError: return true when last content expansion was failed */
bool ContentManager::wasError()
{
   return m_hasError;
}

/* ContentManager::needReset: return true when last content expansion requires reset */
bool ContentManager::needReset()
{
   return m_needsReset;
}

/* ContentManager::getContentMDFFile: get content mdf file */
const char *ContentManager::getContentMDFFile()
{
   return m_mdfFile;
}


/* localMenuHandler: local menu handler callback */
static void localMenuHandler(int id, int item, void *data)
{
   ContentManager *cm = (ContentManager *)data;
   cm->menuHandler(id, item);
}

/* localPopupHandler: local popup handler callback */
static void localPopupHandler(int id, int item, int choice, void *data)
{
   ContentManager *cm = (ContentManager *)data;
   cm->popupHandler(id, item, choice);
}

/* localPopupAddHandler: local popup add handler callback */
static void localPopupAddHandler(int id, int item, int choice, void *data)
{
   ContentManager *cm = (ContentManager *)data;
   cm->popupAddHandler(id, item, choice);
}

/* ContentManager::menuHandler: menu handler callback */
void ContentManager::menuHandler(int id, int item)
{
   if (m_cacheDirList == NULL)
      return;

   if (item == m_currentBookmarkAddItemId) {
      /* add bookmark */
      setBookmark(item, m_currentContentDir, m_currentMdfFile, m_currentSourceURL);
      /* re-create menu now */
      updateMenu(NULL, NULL);
      return;
   }

   if (m_cacheDirList[item].needsUpdate && m_cacheDirList[item].source)
      /* when content update was detected, restart with the content URL, which invokes content download */
      m_mmdagent->setResetFlag(m_cacheDirList[item].source);
   else
      /* else, restart with the content mdf file, which just starts cached content */
      m_mmdagent->setResetFlag(m_cacheDirList[item].file);
}

/* ContentManager::popupHandler: popup handler callback */
void ContentManager::popupHandler(int id, int item, int choice)
{
   char *contentDirName;
   char buff[MMDAGENT_MAXBUFLEN];
   char buff2[MMDAGENT_MAXBUFLEN];
   KeyValue *bm, *bmnew;
   int i, n;
   const char *mdf, *dir, *src;

   if (m_cacheDirList == NULL)
      return;

   bm = NULL;

   switch (choice) {
   case 0: /* view readme */
      if (m_cacheDirList[item].docfile) {
         m_mmdagent->getInfoText()->load(m_cacheDirList[item].docfile);
         m_mmdagent->getInfoText()->show();
      }
      break;
   case 1: /* set as home */
      if (m_cacheDirList[item].dir) {
         if (m_cacheDirList[item].source) {
            setHomeURL(m_cacheDirList[item].source);
            /* re-create menu now */
            updateMenu(NULL, NULL);
            m_menu->setPopupFlag(id, item, false);
         }
      }
      break;
   case 2: /* delete bookmark */
      contentDirName = MMDAgent_contentdirdup();
      if (contentDirName == NULL)
         break;
      MMDAgent_snprintf(buff, MMDAGENT_MAXBUFLEN, "%s%c%s", contentDirName, MMDAGENT_DIRSEPARATOR, CONTENTMANAGER_BOOKMARKFILE);
      free(contentDirName);
      bm = new KeyValue;
      bm->setup();
      if (bm->load(buff, NULL) == false)
         break;
      bmnew = new KeyValue;
      bmnew->setup();
      n = 0;
      for (i = 0; i < CONTENTMANAGER_BOOKMARKNUM; i++) {
         if (i == item) continue;
         MMDAgent_snprintf(buff2, MMDAGENT_MAXBUFLEN, "dir%d", i);
         dir = bm->getString(buff2, NULL);
         if (dir) {
            MMDAgent_snprintf(buff2, MMDAGENT_MAXBUFLEN, "dir%d", n);
            bmnew->setString(buff2, dir);
            MMDAgent_snprintf(buff2, MMDAGENT_MAXBUFLEN, "mdf%d", i);
            mdf = bm->getString(buff2, NULL);
            if (mdf) {
               MMDAgent_snprintf(buff2, MMDAGENT_MAXBUFLEN, "mdf%d", n);
               bmnew->setString(buff2, mdf);
            }
            MMDAgent_snprintf(buff2, MMDAGENT_MAXBUFLEN, "src%d", i);
            src = bm->getString(buff2, NULL);
            if (src) {
               MMDAgent_snprintf(buff2, MMDAGENT_MAXBUFLEN, "src%d", n);
               bmnew->setString(buff2, src);
            }
            n++;
         }
      }
      bmnew->save(buff);
      delete bmnew;
      /* re-create menu now */
      updateMenu(NULL, NULL);
      break;
   }

   if (bm)
      delete bm;
}

/* ContentManager::popupAddHandler: popup add handler callback */
void ContentManager::popupAddHandler(int id, int item, int choice)
{
   switch (choice) {
   case 0: /* add bookmark */
      setBookmark(item, m_currentContentDir, m_currentMdfFile, m_currentSourceURL);
      /* re-create menu now */
      updateMenu(NULL, NULL);
      break;
   }
}

/* ContentManager::setMenu: set menu to handle bookmark */
void ContentManager::setMenu(Menu *menu)
{
   m_menu = menu;
}

/* ContentManager::updateMenu: update bookmark menu */
void ContentManager::updateMenu(const char *currentContentDir, const char *currentMdfFile)
{
   char *contentDirName;
   char buff[MMDAGENT_MAXBUFLEN];
   char buff2[MMDAGENT_MAXBUFLEN];
   KeyValue *bm;
   KeyValue *prop;
   int id;
   int i, n;
   const char *popupCommands[] = { "View", "SetHome", "Delete" };

   if (m_menu == NULL)
      return;

   /* store current content dir */
   if (currentContentDir) {
      if (m_currentContentDir)
         free(m_currentContentDir);
      m_currentContentDir = MMDAgent_strdup(currentContentDir);
   }

   /* store current mdf file to be used when adding bookmark */
   if (currentMdfFile) {
      if (m_currentMdfFile)
         free(m_currentMdfFile);
      m_currentMdfFile = MMDAgent_strdup(currentMdfFile);
   }

   if (m_currentSourceURL)
      free(m_currentSourceURL);
   m_currentSourceURL = NULL;
   prop = new KeyValue;
   prop->setup();
   MMDAgent_snprintf(buff, MMDAGENT_MAXBUFLEN, "%s%c%s", m_currentContentDir, MMDAGENT_DIRSEPARATOR, MMDAGENT_CONTENTINFOFILE);
   if (prop->load(buff, NULL)) {
      if (prop->exist("SourceURL")) {
         m_currentSourceURL = MMDAgent_strdup(prop->getString("SourceURL", ""));
      }
   }
   delete prop;

   /* clear cache dir list */
   if (m_cacheDirList) {
      for (i = 0; i < CONTENTMANAGER_BOOKMARKNUM; i++) {
         if (m_cacheDirList[i].dir)
            free(m_cacheDirList[i].dir);
         if (m_cacheDirList[i].name)
            free(m_cacheDirList[i].name);
         if (m_cacheDirList[i].subtext)
            free(m_cacheDirList[i].subtext);
         if (m_cacheDirList[i].file)
            free(m_cacheDirList[i].file);
         if (m_cacheDirList[i].imagefile)
            free(m_cacheDirList[i].imagefile);
         if (m_cacheDirList[i].source)
            free(m_cacheDirList[i].source);
         if (m_cacheDirList[i].docfile)
            free(m_cacheDirList[i].docfile);
      }
      free(m_cacheDirList);
   }
   m_cacheDirList = (CacheDir *)malloc(sizeof(CacheDir) * CONTENTMANAGER_BOOKMARKNUM);
   for (i = 0; i < CONTENTMANAGER_BOOKMARKNUM; i++) {
      m_cacheDirList[i].dir = NULL;
      m_cacheDirList[i].name = NULL;
      m_cacheDirList[i].subtext = NULL;
      m_cacheDirList[i].file = NULL;
      m_cacheDirList[i].imagefile = NULL;
      m_cacheDirList[i].source = NULL;
      m_cacheDirList[i].docfile = NULL;
      m_cacheDirList[i].preserved = false;
      m_cacheDirList[i].isHome = false;
      m_cacheDirList[i].nowPlaying = false;
      m_cacheDirList[i].needsUpdate = false;
      m_cacheDirList[i].logEnabled = false;
      m_cacheDirList[i].logSpeechInput = false;
      m_cacheDirList[i].disabled = false;
   }

   /* create menu if not */
   id = m_menu->find("[Bookmark]");
   if (id == -1) {
      id = m_menu->add("[Bookmark]", MENUPRIORITY_SYSTEM, localMenuHandler, this);
      /* make it skip */
      m_menu->setSkipFlag(id, true);
   }

   /* load bookmark */
   contentDirName = MMDAgent_contentdirdup();
   if (contentDirName == NULL)
      return;
   MMDAgent_snprintf(buff, MMDAGENT_MAXBUFLEN, "%s%c%s", contentDirName, MMDAGENT_DIRSEPARATOR, CONTENTMANAGER_BOOKMARKFILE);
   bm = new KeyValue;
   bm->setup();
   n = 0;
   if (bm->load(buff, NULL)) {
      /* load content information per directory */
      const char *mdf, *dir, *src;
      char *home = getHomeURLdup(NULL);
      for (i = 0; i < CONTENTMANAGER_BOOKMARKNUM; i++) {
         MMDAgent_snprintf(buff, MMDAGENT_MAXBUFLEN, "dir%d", i);
         dir = bm->getString(buff, NULL);
         if (dir == NULL)
            break;
         MMDAgent_snprintf(buff, MMDAGENT_MAXBUFLEN, "mdf%d", i);
         mdf = bm->getString(buff, NULL);
         MMDAgent_snprintf(buff, MMDAGENT_MAXBUFLEN, "src%d", i);
         src = bm->getString(buff, NULL);

         prop = new KeyValue;
         prop->setup();
         MMDAgent_snprintf(buff2, MMDAGENT_MAXBUFLEN, "%s%c%s", dir, MMDAGENT_DIRSEPARATOR, MMDAGENT_CONTENTINFOFILE);
         if (prop->load(buff2, NULL)) {
            /* has directory content information file in the directory (web cache etc) */
            m_cacheDirList[n].dir = MMDAgent_strdup(dir);
            m_cacheDirList[n].name = MMDAgent_strdup(prop->getString("ContentName", ""));
            m_cacheDirList[n].time = atoll(prop->getString("ExtractedEpochTime", "0"));
            MMDAgent_snprintf(buff2, MMDAGENT_MAXBUFLEN, "%s%c%s", dir, MMDAGENT_DIRSEPARATOR, prop->getString("ExecMDFFile", ""));
            m_cacheDirList[n].file = MMDAgent_strdup(buff2);
            if (prop->exist("ImageFile")) {
               MMDAgent_snprintf(buff2, MMDAGENT_MAXBUFLEN, "%s%c%s", dir, MMDAGENT_DIRSEPARATOR, prop->getString("ImageFile", ""));
               m_cacheDirList[n].imagefile = MMDAgent_strdup(buff2);
            } else {
               MMDAgent_snprintf(buff2, MMDAGENT_MAXBUFLEN, "%s%c%s", dir, MMDAGENT_DIRSEPARATOR, CONTENTMANAGER_DEFAULTBANNERIMAGEFILE);
               m_cacheDirList[n].imagefile = MMDAgent_strdup(buff2);
            }
            if (prop->exist("SourceURL"))
               m_cacheDirList[n].source = MMDAgent_strdup(prop->getString("SourceURL", ""));
            if (prop->exist("Readme")) {
               MMDAgent_snprintf(buff2, MMDAGENT_MAXBUFLEN, "%s%c%s", dir, MMDAGENT_DIRSEPARATOR, prop->getString("Readme", ""));
               m_cacheDirList[n].docfile = MMDAgent_strdup(buff2);
            } else {
               m_cacheDirList[n].docfile = NULL;
            }
            if (prop->exist("LastModifiedEpochTime"))
               if (atoll(prop->getString("LastModifiedEpochTime", "0")) > m_cacheDirList[n].time)
                  m_cacheDirList[n].needsUpdate = true;
            if (prop->exist("DownloadCompleted"))
               if (MMDAgent_strequal(prop->getString("DownloadCompleted", "true"), "false"))
                  m_cacheDirList[n].needsUpdate = true;
            if (prop->exist("LogUploadURL"))
               m_cacheDirList[n].logEnabled = true;
            if (prop->exist("LogSpeechInput"))
               m_cacheDirList[n].logSpeechInput = MMDAgent_strequal(prop->getString("LogSpeechInput", "false"), "true") ? true : false;
         } else if (MMDAgent_strequal(src, "none") == false) {
            /* web content but cache is missing */
            m_cacheDirList[n].name = MMDAgent_strdup(src);
            m_cacheDirList[n].imagefile = NULL;
            m_cacheDirList[n].source = MMDAgent_strdup(src);
            m_cacheDirList[n].needsUpdate = true;
            m_cacheDirList[n].subtext = MMDAgent_strdup("tap to download");
         } else {
            /* local file content */
            KeyValue *desc;
            const char *ip;
            desc = new KeyValue;
            desc->setup();
            MMDAgent_snprintf(buff, MMDAGENT_MAXBUFLEN, "%s%c%s", dir, MMDAGENT_DIRSEPARATOR, CONTENTMANAGER_PACKAGEFILE);
            if (desc->load(buff, g_enckey) == true) {
               /* the directory has PACKAGE_DESC.txt, follow it */
               if (desc->exist("label")) {
                  m_cacheDirList[n].name = MMDAgent_strdup(desc->getString("label", ""));
               } else {
                  m_cacheDirList[n].name = MMDAgent_tailpath(mdf, 40);
               }
               ip = desc->getString("image", NULL);
               if (ip) {
                  MMDAgent_snprintf(buff2, MMDAGENT_MAXBUFLEN, "%s%c%s", dir, MMDAGENT_DIRSEPARATOR, ip);
               } else {
                  MMDAgent_snprintf(buff2, MMDAGENT_MAXBUFLEN, "%s%c%s", dir, MMDAGENT_DIRSEPARATOR, CONTENTMANAGER_DEFAULTBANNERIMAGEFILE);
               }
               m_cacheDirList[n].imagefile = MMDAgent_strdup(buff2);
               if (desc->exist("readme")) {
                  MMDAgent_snprintf(buff2, MMDAGENT_MAXBUFLEN, "%s%c%s", dir, MMDAGENT_DIRSEPARATOR, desc->getString("readme", ""));
                  m_cacheDirList[n].docfile = MMDAgent_strdup(buff2);
               }
            } else {
               /* no PACKAGE_DESC.txt, apply default */
               m_cacheDirList[n].name = MMDAgent_tailpath(mdf, 40);
               MMDAgent_snprintf(buff2, MMDAGENT_MAXBUFLEN, "%s%c%s", dir, MMDAGENT_DIRSEPARATOR, CONTENTMANAGER_DEFAULTBANNERIMAGEFILE);
               m_cacheDirList[n].imagefile = MMDAgent_strdup(buff2);
            }
            delete desc;
            m_cacheDirList[n].dir = MMDAgent_strdup(dir);
            m_cacheDirList[n].file = MMDAgent_strdup(mdf);
            m_cacheDirList[n].subtext = MMDAgent_strdup("local content");
            if (MMDAgent_stat(m_cacheDirList[n].dir) != MMDAGENT_STAT_DIRECTORY || MMDAgent_stat(m_cacheDirList[n].file) != MMDAGENT_STAT_NORMAL) {
               m_cacheDirList[n].disabled = true;
               m_cacheDirList[n].subtext = MMDAgent_strdup("local content (not exist)");
            } else {
               m_cacheDirList[n].subtext = MMDAgent_strdup("local content");
            }
         }
         if (m_cacheDirList[n].source && home && MMDAgent_strequal(m_cacheDirList[n].source, home)) {
            m_cacheDirList[n].isHome = true;
         }
         delete prop;
         n++;
      }
      if (home)
         free(home);
   }
   delete bm;

   for (i = 0; i < n && i < MENUMAXITEM; i++) {
      m_menu->setItem(id, i, m_cacheDirList[i].name, m_cacheDirList[i].imagefile, NULL, NULL, m_cacheDirList[i].subtext);
      m_menu->setPopup(id, i, popupCommands, 3, localPopupHandler, this);
      if (m_cacheDirList[i].preserved)
         m_menu->setIconFlag(id, i, MENU_ICON_LOCK, true);
      if (m_cacheDirList[i].isHome)
         m_menu->setIconFlag(id, i, MENU_ICON_HOME, true);
      if (m_cacheDirList[i].needsUpdate)
         m_menu->setIconFlag(id, i, MENU_ICON_DOWNLOAD, true);
      if (m_currentContentDir && MMDAgent_strequal(m_cacheDirList[i].dir, m_currentContentDir)) {
         m_cacheDirList[i].nowPlaying = true;
         m_menu->setIconFlag(id, i, MENU_ICON_PLAY, true);
      }
      if (m_cacheDirList[i].logEnabled)
         m_menu->setIconFlag(id, i, MENU_ICON_EXPORT, true);
      if (m_cacheDirList[i].logSpeechInput)
         m_menu->setIconFlag(id, i, MENU_ICON_MIC, true);
      if (m_cacheDirList[i].disabled)
         m_menu->setItemStatus(id, i, MENUITEM_STATUS_DISABLED);
      else
         m_menu->setItemStatus(id, i, MENUITEM_STATUS_NORMAL);
      /* keep popup status */
      if (i == m_menu->getPoppingRow())
         m_menu->setPopupFlag(id, i, true);
   }
   const char *popupAddCommands[] = { "Add bookmark" };

   if (i < MENUMAXITEM) {
      m_menu->setItem(id, i, "[+ add current]", NULL, NULL, NULL);
      m_menu->setPopup(id, i, popupAddCommands, 1, localPopupAddHandler, this);
      m_menu->setItemStatus(id, i, MENUITEM_STATUS_NORMAL);
      m_currentBookmarkAddItemId = i;
      i++;
   }
   for (; i < MENUMAXITEM; i++)
      m_menu->removeItem(id, i);

   free(contentDirName);
}

/* ContentManager::setBookmark: set bookmark */
void ContentManager::setBookmark(int id, const char *currentContentDir, const char *currentMdfFile, const char *currentSourceURL)
{
   char *contentDirName;
   char buff[MMDAGENT_MAXBUFLEN];
   char buff2[MMDAGENT_MAXBUFLEN];
   KeyValue *bm;
   const char *key, *p;
   void *save;
   char mid;

   contentDirName = MMDAgent_contentdirdup();
   if (contentDirName == NULL)
      return;
   MMDAgent_snprintf(buff, MMDAGENT_MAXBUFLEN, "%s%c%s", contentDirName, MMDAGENT_DIRSEPARATOR, CONTENTMANAGER_BOOKMARKFILE);
   bm = new KeyValue;
   bm->setup();
   bm->load(buff, NULL);

   for (key = bm->firstKey(&save); key; key = bm->nextKey(&save)) {
      if (MMDAgent_strheadmatch(key, "dir")) {
         if (MMDAgent_strequal(bm->getString(key, NULL), currentContentDir)) {
            mid = key[3];
            MMDAgent_snprintf(buff2, MMDAGENT_MAXBUFLEN, "mdf%c", mid);
            if ((p = bm->getString(buff2, NULL)) != NULL && MMDAgent_strequal(p, currentMdfFile)) {
               // same entry already exist
               delete bm;
               free(contentDirName);
               return;
            }
         }
      }
   }

   MMDAgent_snprintf(buff2, MMDAGENT_MAXBUFLEN, "dir%d", id);
   bm->setString(buff2, currentContentDir);
   MMDAgent_snprintf(buff2, MMDAGENT_MAXBUFLEN, "mdf%d", id);
   bm->setString(buff2, currentMdfFile);
   MMDAgent_snprintf(buff2, MMDAGENT_MAXBUFLEN, "src%d", id);
   if (currentSourceURL) {
      bm->setString(buff2, currentSourceURL);
   } else {
      bm->setString(buff2, "none");
   }
   bm->save(buff);
   delete bm;
   free(contentDirName);
}


/* ContentManager::setContentCursorToCurrent: set content cursor to current */
void ContentManager::setContentCursorToCurrent()
{
   int i;
   int id;

   if (m_currentContentDir == NULL)
      return;
   if (m_cacheDirList == NULL)
      return;
   if (m_menu == NULL)
      return;
   id = m_menu->find("[Bookmark]");
   if (id == -1)
      return;

   for (i = 0; i < MENUMAXITEM; i++) {
      if (m_cacheDirList[i].dir && MMDAgent_strequal(m_cacheDirList[i].dir, m_currentContentDir)) {
         m_menu->moveCursorAt(id, i);
         break;
      }
   }

}


/* ContentManager::renderBegin: beginning part of render */
void ContentManager::renderBegin()
{
   glDisable(GL_LIGHTING);
   glDisable(GL_DEPTH_TEST);
   glMatrixMode(GL_PROJECTION);
   glPushMatrix();
   glLoadIdentity();
   MMDAgent_setOrtho(-1.0f, 1.0f, -1.0f, 1.0f, -1, 1);
   glMatrixMode(GL_MODELVIEW);
   glPushMatrix();
   glLoadIdentity();
   glEnableClientState(GL_VERTEX_ARRAY);
   glTranslatef(0.0f, -0.6f, 0.0f);
}

/* ContentManager::render: render a bar */
void ContentManager::render(const char *label, float rate)
{
   float r;
   int ridx;
   GLfloat colbar[3] = { 0.30f, 0.30f, 0.30f };
   GLfloat col[5][3] = { { 0.96f, 0.23f, 0.10f }, { 0.96f, 0.57f, 0.10f }, { 0.96f, 0.77f, 0.14f }, { 0.86f, 0.84f, 0.13f }, { 0.68f, 0.96f, 0.12f } };
   GLfloat vertices0[12] = { -0.9f, 0.0f, 0.0f, 0.9f, 0.0f, 0.0f, -0.9f, 0.2f, 0.0f, 0.9f, 0.2f, 0.0f };
   GLfloat vertices1[12] = { -0.9f, 0.0f, 0.01f, 0.9f, 0.0f, 0.01f, -0.9f, 0.2f, 0.01f, 0.9f, 0.2f, 0.01f };

   m_elem.textLen = 0;
   m_elem.numIndices = 0;
   if (m_font && m_fontError == false) {
      /* text indicator at above */
      if (m_font->getTextDrawElements(label, &m_elem, m_elem.textLen, 0.0f, 0.0f, 0.0f) == false) {
         m_elem.textLen = 0;
         m_elem.numIndices = 0;
         m_fontError = true;
      }
   }

   glPushMatrix();

   /* base bar */
   glColor4f(colbar[0], colbar[1], colbar[2], 1.0f);
   glVertexPointer(3, GL_FLOAT, 0, vertices0);
   glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

   /* progress bar */
   vertices1[3] = vertices1[9] = rate * 1.8f - 0.9f;
   ridx = (int)(rate * 4.0f);
   if (ridx > 3)
      ridx = 3;
   r = rate * 4.0f - ridx;
   glColor4f(col[ridx][0] * (1.0f - r) + col[ridx + 1][0] * r, col[ridx][1] * (1.0f - r) + col[ridx + 1][1] * r, col[ridx][2] * (1.0f - r) + col[ridx + 1][2] * r, 1.0f);
   glVertexPointer(3, GL_FLOAT, 0, vertices1);
   glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

   if (m_font && m_fontError == false && m_elem.numIndices > 0) {
      glTranslatef(-0.88f, 0.32f, 0.05f);
      glScalef(0.07f, 0.07f, 0.07f);
      glEnable(GL_TEXTURE_2D);
      glActiveTexture(GL_TEXTURE0);
      glClientActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_2D, m_font->getTextureID());
      glEnableClientState(GL_TEXTURE_COORD_ARRAY);
      if (m_elem.numIndices > 0) {
         glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
         glVertexPointer(3, GL_FLOAT, 0, m_elem.vertices);
         glTexCoordPointer(2, GL_FLOAT, 0, m_elem.texcoords);
         glDrawElements(GL_TRIANGLES, m_elem.numIndices, GL_INDICES, (const GLvoid *)m_elem.indices);
      }
      glDisableClientState(GL_TEXTURE_COORD_ARRAY);
      glDisable(GL_TEXTURE_2D);
   }

   glPopMatrix();

   /* slide down for next content downloader */
   glTranslatef(0.0f, 0.45f, 0.0f);
}

/* ContentManager::renderEnd: ending part of render */
void ContentManager::renderEnd()
{
   glDisableClientState(GL_VERTEX_ARRAY);
   glPopMatrix();
   glMatrixMode(GL_PROJECTION);
   glPopMatrix();
   glMatrixMode(GL_MODELVIEW);
   glEnable(GL_DEPTH_TEST);
   glEnable(GL_LIGHTING);
}

/* ContentManager::startCheckUpdate: start checking source update */
void ContentManager::startCheckUpdate(const char *systemDirName)
{
   char buff[MMDAGENT_MAXBUFLEN];
   char buff2[MMDAGENT_MAXBUFLEN];
   DIRECTORY *d;
   char *contentDirName;
   KeyValue *prop;
   CMList *list, *l;
   int n;

   if (m_contentUpdater) {
      /* not allow more than one content */
      m_mmdagent->sendLogString(m_id, MLOG_ERROR, "duplicate thread, failed to check updates");
      return;
   }
   contentDirName = MMDAgent_contentdirdup();
   if (contentDirName == NULL)
      return;
   d = MMDAgent_opendir(contentDirName);
   if (d == NULL) {
      // no content cache
      // m_mmdagent->sendLogString(m_id, MLOG_ERROR, "failed to open content cache dir %s", contentDirName);
      free(contentDirName);
      return;
   }
   list = NULL;

   /* system */
   MMDAgent_snprintf(buff2, MMDAGENT_MAXBUFLEN, "%s%c%s", systemDirName, MMDAGENT_DIRSEPARATOR, MMDAGENT_CONTENTINFOFILE);
   prop = new KeyValue;
   prop->setup();
   if (prop->load(buff2, NULL) && prop->exist("SourceURL")) {
      l = new CMList();
      l->dir = MMDAgent_strdup(systemDirName);
      if (MMDAgent_strheadmatch(prop->getString("SourceURL", ""), "mmdagent://")) {
         MMDAgent_snprintf(buff2, MMDAGENT_MAXBUFLEN, "%s", prop->getString("SourceURL", ""));
         buff2[4] = 'h';
         buff2[5] = 't';
         buff2[6] = 't';
         buff2[7] = 'p';
         l->url = MMDAgent_strdup(&(buff2[4]));
      } else {
         l->url = MMDAgent_strdup(prop->getString("SourceURL", ""));
      }
      l->next = list;
      list = l;
   }
   delete prop;

   /* contents */
   n = 0;
   while (MMDAgent_readdir(d, buff) == true) {
      if (buff[0] == '.')
         continue;
      if (buff[0] == '_')
         continue;
      MMDAgent_snprintf(buff2, MMDAGENT_MAXBUFLEN, "%s%c%s%c%s", contentDirName, MMDAGENT_DIRSEPARATOR, buff, MMDAGENT_DIRSEPARATOR, MMDAGENT_CONTENTINFOFILE);
      prop = new KeyValue;
      prop->setup();
      if (prop->load(buff2, NULL) == false) {
         delete prop;
         continue;
      }
      if (prop->exist("SourceURL")) {
         l = new CMList();
        MMDAgent_snprintf(buff2, MMDAGENT_MAXBUFLEN, "%s%c%s", contentDirName, MMDAGENT_DIRSEPARATOR, buff);
         l->dir = MMDAgent_strdup(buff2);
         if (MMDAgent_strheadmatch(prop->getString("SourceURL", ""), "mmdagent://")) {
            MMDAgent_snprintf(buff2, MMDAGENT_MAXBUFLEN, "%s", prop->getString("SourceURL", ""));
            buff2[4] = 'h';
            buff2[5] = 't';
            buff2[6] = 't';
            buff2[7] = 'p';
            l->url = MMDAgent_strdup(&(buff2[4]));
         } else {
            l->url = MMDAgent_strdup(prop->getString("SourceURL", ""));
         }
         l->next = list;
         list = l;
      }
      delete prop;
      n++;
   }
   MMDAgent_closedir(d);

   m_mmdagent->sendLogString(m_id, MLOG_STATUS, "start checking updates for existing %d web contents\n", n);

   if (m_list == NULL) {
      m_list = list;
   } else {
      l = m_list;
      while (l->next != NULL)
         l = l->next;
      l->next = list;
   }

   free(contentDirName);
}

/* ContentManager::processCheckUpdate: process checking source update */
bool ContentManager::processCheckUpdate()
{
   CMList *l;

   if (m_list == NULL && m_contentUpdater == NULL)
      return true;

   if (m_list == NULL && m_contentUpdater != NULL && m_contentUpdater->isFinished()) {
      m_mmdagent->sendLogString(m_id, MLOG_STATUS, "finished checking content updates, rehash menu now\n");
      updateMenu(NULL, NULL);
      delete m_contentUpdater;
      m_contentUpdater = NULL;
   }
   if (m_list != NULL && (m_contentUpdater == NULL || m_contentUpdater->isFinished())) {
      l = m_list;
      m_list = m_list->next;
      if (m_contentUpdater)
         delete m_contentUpdater;
      m_contentUpdater = new ContentManagerThreadWeb;
      m_contentUpdater->setupAndStart(m_mmdagent, m_id, l->url, l->dir, true, false);
      delete l;
   }
   return false;
}

/* ContentManager::getSourceInContentDir: get source in content dir */
char *ContentManager::getSourceInContentDir(const char* contentDir)
{
   char buff[MMDAGENT_MAXBUFLEN];
   KeyValue* prop;
   char* source = NULL;
   bool needUpdate = false;

   MMDAgent_snprintf(buff, MMDAGENT_MAXBUFLEN, "%s%c%s", contentDir, MMDAGENT_DIRSEPARATOR, MMDAGENT_CONTENTINFOFILE);
   prop = new KeyValue;
   prop->setup();
   if (prop->load(buff, NULL) && prop->exist("SourceURL")) {
      source = MMDAgent_strdup(prop->getString("SourceURL", ""));
      if (prop->exist("LastModifiedEpochTime") && prop->exist("ExtractedEpochTime"))
         if (atoll(prop->getString("LastModifiedEpochTime", "0")) > atoll(prop->getString("ExtractedEpochTime", "0")))
            needUpdate = true;
      if (prop->exist("DownloadCompleted"))
         if (MMDAgent_strequal(prop->getString("DownloadCompleted", "true"), "false"))
            needUpdate = true;
   }
   delete prop;
   if (needUpdate)
      return source;
   return NULL;
}

/* ContentManager::currentContentRequireUpdate: check if current content has update on server */
bool ContentManager::currentContentRequireUpdate(const char *currentContentDir)
{
   char* contentDirName;
   char* source;

   contentDirName = MMDAgent_contentdirdup();
   if (contentDirName) {
      /* current content is under content dir, check in recursive manner */
      char *dir = MMDAgent_strdup(currentContentDir);
      while (MMDAgent_strheadmatch(dir, contentDirName)) {
         source = getSourceInContentDir(dir);
         if (source)
            return true;
         char *p = MMDAgent_dirname(dir);
         free(dir);
         dir = p;
      }
      free(dir);
   }
   source = getSourceInContentDir(currentContentDir);
   if (source)
      return true;
   return false;
}

/* ContentManager::restartCurrentUpdate: restart current content from URLto update */
void ContentManager::restartCurrentUpdate(const char *currentContentDir)
{
   char* contentDirName;
   char* source;

   contentDirName = MMDAgent_contentdirdup();
   if (contentDirName) {
      /* current content is under content dir, check in recursive manner */
      char* dir = MMDAgent_strdup(currentContentDir);
      while (MMDAgent_strheadmatch(dir, contentDirName)) {
         source = getSourceInContentDir(dir);
         if (source) {
            m_mmdagent->setResetFlag(source);
            return;
         }
         char* p = MMDAgent_dirname(dir);
         free(dir);
         dir = p;
      }
      free(dir);
   }
   source = getSourceInContentDir(currentContentDir);
   if (source)
      m_mmdagent->setResetFlag(source);
}

/* ContentManager::loadBanList: read Ban List */
void ContentManager::loadBanList(const char *systemDirName)
{
   char buff[MMDAGENT_MAXBUFLEN];
   FILE *fp;
   int len;
   char *p1;
   BanList *b, *btmp;

   if (systemDirName == NULL)
      return;

   MMDAgent_snprintf(buff, MMDAGENT_MAXBUFLEN, "%s%c%s", systemDirName, MMDAGENT_DIRSEPARATOR, "banlist");
   fp = MMDAgent_fopen(buff, "r");
   if (fp == NULL)
      return;

   b = m_banList;
   while (b) {
      btmp = b->next;
      if (b->urlex)
         free(b->urlex);
      delete b;
      b = btmp;
   }
   m_banList = NULL;

   while (fgets(buff, MMDAGENT_MAXBUFLEN, fp)) {
      len = MMDAgent_strlen(buff);
      if (len <= 0)
         continue;
      p1 = &(buff[len - 1]);
      while (p1 >= &(buff[0]) && (*p1 == '\n' || *p1 == '\r' || *p1 == '\t' || *p1 == ' ')) {
         *p1 = L'\0';
         p1--;
      }
      if (buff[0] == '#')
         continue;
      b = new BanList();
      b->urlex = MMDAgent_strdup(buff);
      b->next = m_banList;
      m_banList = b;
   }
   fclose(fp);

}

using Poco::RegularExpression;

/* ContentManager::isBanned: return true if the given URL is banned */
bool ContentManager::isBanned(const char *url)
{
   BanList *b;
   bool matched;

   b = m_banList;

   while (b) {
      try {
	matched = Poco::RegularExpression(b->urlex).match(url);
      }
      catch (const Poco::Exception&) {
         b = b->next;
         continue;
      }
      if (matched)
         return true;
      b = b->next;
   }
   return false;
}

/* ContentManager::getHomeURLdup: return allocated buffer that holds home url */
char *ContentManager::getHomeURLdup(int *idx)
{
   char *contentDirName;
   char buff[MMDAGENT_MAXBUFLEN];
   KeyValue *bm;
   char *ret;
   bool is_file = false;

   contentDirName = MMDAgent_contentdirdup();
   if (contentDirName == NULL)
      return NULL;
   MMDAgent_snprintf(buff, MMDAGENT_MAXBUFLEN, "%s%c%s", contentDirName, MMDAGENT_DIRSEPARATOR, CONTENTMANAGER_HOMEFILE);
   free(contentDirName);
   bm = new KeyValue;
   bm->setup();
   if (bm->load(buff, NULL) && bm->exist("home")) {
      ret = MMDAgent_strdup(bm->getString("home", NULL));
   } else {
      ret = NULL;
   }
   delete bm;
   if (ret) {
      if (MMDAgent_strheadmatch(ret, "http://")) {
         if (idx) *idx = 7;
      } else if (MMDAgent_strheadmatch(ret, "https://")) {
         if (idx) *idx = 8;
      } else if (MMDAgent_strheadmatch(ret, "mmdagent://")) {
         if (idx) *idx = 11;
      } else {
         if (idx) *idx = 0;
         is_file = true;
      }
   }
   /* return NULL when a home is a file and does not exist */
   if (ret && is_file && MMDAgent_exist(ret) == false) {
      m_mmdagent->sendLogString(m_id, MLOG_WARNING, "home is set but not exist: %s", ret);

      ret = NULL;
   }

   return ret;
}

/* ContentManager::setHomeURL: set home to the given url */
void ContentManager::setHomeURL(const char *url)
{
   char *contentDirName;
   char buff[MMDAGENT_MAXBUFLEN];
   KeyValue *bm;

   contentDirName = MMDAgent_contentdirdup();
   if (contentDirName == NULL)
      return;
   MMDAgent_snprintf(buff, MMDAGENT_MAXBUFLEN, "%s%c%s", contentDirName, MMDAGENT_DIRSEPARATOR, CONTENTMANAGER_HOMEFILE);
   bm = new KeyValue;
   bm->setup();
   bm->setString("home", url);
   bm->save(buff);
   delete bm;
   free(contentDirName);
}

/* ContentManager::setHomeFile: set home to the given path */
void ContentManager::setHomeFile(const char *path)
{
   char *fullpath = MMDAgent_fullpathname(path);
   if (fullpath == NULL)
      return;
   setHomeURL(fullpath);
   free(fullpath);
}

/* ContentManager::clearHome: clear home */
void ContentManager::clearHome()
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
