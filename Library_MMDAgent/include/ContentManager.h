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

/* definitions */

#define CONTENTMANAGER_CONTENTFILE ".mmdagent-content-files"
#define CONTENTMANAGER_PACKAGEFILE "PACKAGE_DESC.txt"
#define CONTENTMANAGER_DEFAULTBANNERIMAGEFILE  "banner.png"
#define CONTENTMANAGER_NONDESKTOPMESSAGE "Sorry, This content is for smartphone only."
#define CONTENTMANAGER_BOOKMARKFILE "_bookmark"
#define CONTENTMANAGER_HOMEFILE "_home"

struct CMList {
   char *url;
   char *dir;
   struct CMList *next;
   CMList() { url = NULL; dir = NULL; next = NULL; }
   ~CMList() {if (url) free(url); if (dir) free(dir); url = NULL; dir = NULL; next = NULL;}
};

struct BanList {
   char *urlex;
   struct BanList *next;
};

/* ContentManager: content manager */
class ContentManager
{
public:
   struct CacheDir {
      char *dir;
      long long time;
      char *name;
      char *subtext;
      char *file;
      char *imagefile;
      char *source;
      char *docfile;
      bool preserved;
      bool isHome;
      bool nowPlaying;
      bool needsUpdate;
      bool logEnabled;
      bool logSpeechInput;
      bool disabled;
   };

private:

   MMDAgent *m_mmdagent;      /* mmdagent whose member function may be called */
   int m_id;                  /* mmdagent module id */

   CacheDir *m_cacheDirList;
   char *m_currentContentDir;
   char *m_currentMdfFile;
   char *m_currentSourceURL;
   int m_currentBookmarkAddItemId;
   Menu *m_menu;

   ContentManagerThreadZip *m_zip;
   ContentManagerThreadWeb *m_web;
   bool m_hasFinished;
   bool m_hasError;
   bool m_needsReset;
   char *m_mdfFile;
   char *m_contentMessage;

   ContentManagerThreadWeb *m_contentUpdater;
   CMList *m_list;
   BanList *m_banList;

   FTGLTextureFont *m_font;      /* text font */
   FTGLTextDrawElements m_elem;  /* text drawing element holder for progress text rendering */
   bool m_fontError;             /* TRUE when has error in font rendering */

   /* initialize: initialize content manager */
   void initialize();

   /* clear: free content manager */
   void clear();

   /* renderBegin: beginning part of render */
   void renderBegin();

   /* render: render a bar */
   void render(const char *label, float rate);

   /* renderEnd: ending part of render */
   void renderEnd();

public:

   /* ContentManager: constructor */
   ContentManager();

   /* ~ContentManager: destructor */
   ~ContentManager();

   /* setup: initialize and setup content manager */
   void setup(MMDAgent *mmdagent, int id);

   /* checkContentComplete: check content complete */
   bool checkContentComplete(const char *savedir);

   /* startExtractContent: start extracting content from uri and save */
   bool startExtractContent(const char *uri_or_path, const char *savedir, bool resetAfterExtraction, bool fetchContentListOnly, bool preserve);

   /* setupLocalPackageInfo: setup local packge info into content info  */
   void setupLocalPackageInfo(const char *dir);

   /* getContentInfoNew: get newly allocated content info */
   KeyValue *getContentInfoNew(const char *dir);

   /* outputContentInfoToLog: output content information to log */
   void outputContentInfoToLog(const char *dir);

   /* getContentInfoDup: return allocated string for specified keyname */
   char *getContentInfoDup(const char *dir, const char *keyname);

   /* showDocOnFirstTime: show document if this is the first launch */
   bool showDocOnFirstTime(const char *dir);

   /* saveLastPlayedTime: save last played time */
   void saveLastPlayedTime(const char *dir);

   /* updateAndRender: update and render */
   void updateAndRender();

   /* isRunning: return true when running */
   bool isRunning();

   /* hasFinished: return true when finished at last update */
   bool hasFinished();

   /* wasError: return true when last content expansion was failed */
   bool wasError();

   /* needReset: return true when last content expansion requires reset */
   bool needReset();

   /* getContentMDFFile: get content mdf file */
   const char *getContentMDFFile();

   /* menuHandler: menu handler callback */
   void menuHandler(int id, int item);

   /* popupHandler: popup handler callback */
   void popupHandler(int id, int item, int choice);

   /* popupAddHandler: popup add handler callback */
   void popupAddHandler(int id, int item, int choice);

   /* setMenu: set menu to handle bookmark */
   void setMenu(Menu *menu);

   /* updateMenu: update bookmark menu */
   void updateMenu(const char *currentContentDir, const char *currentMdfFile);

   /* setBookmark: set bookmark */
   void setBookmark(int id, const char *currentContentDir, const char *currentMdfFile, const char *currentSourceURL);

   /* setContentCursorToCurrent: set content cursor to current */
   void setContentCursorToCurrent();

   /* startCheckUpdate: start checking source update */
   void startCheckUpdate(const char *systemDirName);

   /* processCheckUpdate: process checking source update */
   bool processCheckUpdate();

   /* getSourceInContentDir: get source in content dir */
   char *getSourceInContentDir(const char* contentDir);

   /* currentContentRequireUpdate: check if current content has update on server */
   bool currentContentRequireUpdate(const char *currentContentDir);

   /* restartCurrentUpdate: restart current content from URLto update */
   void restartCurrentUpdate(const char *currentContentDir);

   /* loadBanList: read Ban List */
   void loadBanList(const char *systemDirName);

   /* isBanned: return true if the given URL is banned */
   bool isBanned(const char *url);

   /* getHomeURLdup: return allocated buffer that holds home url */
   char *getHomeURLdup(int *idx);

   /* setHomeURL: set home to the given url */
   void setHomeURL(const char *url);

   /* clearHome: clear home */
   void clearHome();
};
