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

#define CONTENTDOWNLOAD_TIMEOUT_SEC  5   /* timeout of download connection in seconds */
#define CONTENTDOWNLOAD_MAXFILENUM 2000  /* maximum number of files in a content */

/* DownFiles: file download sub-class */
class DownFiles {

private:
   char *m_uri;          /* URI */
   size_t m_size;        /* total size in bytes */
   size_t m_currentSize; /* current size in bytes */
   char *m_dirname;      /* directory name */
   char *m_savefile;     /* save file name */
   int m_status;  /* status */
   int m_fid;            /* fetch id */
   bool m_kill;
   void *m_session;      /* current session */
   bool m_isssl;
   bool m_preserve;      /* true when new files does not override but save with special prefix */
   unsigned char m_digest[32];

   /* initialize: initialize data */
   void initialize();

   /* clear: clear data */
   void clear();

public:

   /* Constructor */
   DownFiles(char *uri, size_t size, char *savefile, char *dirname, bool preserve, unsigned char *digest);

   /* Destructor */
   ~DownFiles();

   /* getSize: get size */
   size_t getSize();

   /* getCurrentSize: get current size */
   size_t getCurrentSize();

   /* getFileName: get filename */
   const char *getFileName();

   /* check: check if need download */
   int check();

   /* setId: set id */
   void setId(int id);

   /* getId: get id */
   int getId();

   /* download: do download */
   int download();

   /* abort: abort download */
   void abort();
};

/* ContentManagerThreadWeb: content manager for fetching http archive */
class ContentManagerThreadWeb
{
private:

   MMDAgent *m_mmdagent;      /* mmdagent whose member function may be called */
   int m_id;                  /* mmdagent module id */

   GLFWmutex m_mutex;              /* mutex */
   GLFWthread m_thread;            /* thread */
   bool m_kill;

   char *m_url;
   char *m_saveDir;
   DownFiles *m_files[CONTENTDOWNLOAD_MAXFILENUM];    /* list of files to be downloaded*/
   int m_num;                  /* number of above */
   KeyValue *m_fileIndex;

   bool m_finished;
   bool m_error;
   bool m_fileUpdated;     /* true when any file was fetched and updated */
   char *m_mdfFile;

   size_t m_totalSize;     /* total byte size to be downloaded */
   int m_currentId;            /* current downloading file id */
   size_t m_currentSize;       /* current amount of prepared content data */
   int m_fetchNum;             /* number of new/updated files to be fetched */
   size_t m_fetchSize;         /* current amount of downloaded data */
   size_t m_currentFetchedSize;
   int m_fetchCount;
   bool m_fetchStarted;
   bool m_fetchContentListOnly;
   char *m_contentMessage;
   void *m_session;      /* current session */
   bool m_isssl;
   bool m_preserve;      /* true when new files does not override but save with special prefix */

   /* initialize: initialize */
   void initialize();

   /* clear: free */
   void clear();

   /* clearMemoryFromMainThread: clear memory from main thread */
   void clearMemoryFromMainThread();

   /* fetchFile: fetch a single file */
   bool fetchFile(const char *urlString, const char *savePath);

   /* abort: abort */
   void abort();

public:

   /* ContentManagerThreadWeb: constructor */
   ContentManagerThreadWeb();

   /* ~ContentManagerThreadWeb: destructor */
   ~ContentManagerThreadWeb();

   /* setupAndStart: setup manager and start thread */
   void setupAndStart(MMDAgent *mmdagent, int id, const char *sourceUrl, const char *savedir, bool fetchContentListOnly, bool preserve);

   /* run: main thread loop */
   void run();

   /* stopAndRelease: stop thread and free */
   void stopAndRelease();

   /* isFinished: return true when thread has been finished */
   bool isFinished();

   /* hasError: return true when an error occured */
   bool hasError();

   /* isUpdated: return true when part of the content has been updated */
   bool isUpdated();

   /* getContentMDFFile: get content mdf file */
   const char *getContentMDFFile();

   /* getProgress: get progress information */
   void getProgress(char *buff_ret, float *rate_ret);

   /* getContentMessage: get content message */
   char *getContentMessage();
};

/* TinyDownload: download one file */
class TinyDownload {
private:

   MMDAgent *m_mmdagent;      /* mmdagent whose member function may be called */
   int m_id;                  /* mmdagent module id */
   GLFWmutex m_mutex;         /* mutex */
   GLFWthread m_thread;       /* thread */
   char *m_baseurl;           /* download base url */
   char *m_files;             /* save file paths */
   bool m_finished;           /* true when finished downloading */
   bool m_error;              /* true when an error occurs */
   bool m_kill;
   void *m_session;           /* current session */
   bool m_isssl;

   /* initialize: initialize */
   void initialize();

   /* clear: free */
   void clear();

   /* startThread: start thread */
   bool startThread();

   /* stopThread: stop thread */
   void stopThread();

   /* fetchFile: fetch a single file */
   bool fetchFile(const char *urlString, const char *savePath);

   /* abort: abort */
   void abort();

public:

   /* TinyDownload: constructor */
   TinyDownload();

   /* ~TinyDownload: destructor */
   ~TinyDownload();

   /* setupAndStart: setup manager and start thread */
   void setupAndStart(MMDAgent *mmdagent, int id, const char *baseUrl, const char *files);

   /* run: main thread loop */
   void run();

   /* stopAndRelease: stop thread and free */
   void stopAndRelease();

   /* restart: restart */
   void restart();

   /* isFinished: return true when thread has been finished */
   bool isFinished();

   /* hasError: return true when an error occured */
   bool hasError();
};
