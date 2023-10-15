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

#define CONTENTUPLOAD_TIMEOUTSEC 20    /* timeout seconds for upload */
#define CONTENTUPLOAD_MAXFILENUM 2000  /* maximum number of files in a directory to upload */

/* UpFiles: file upload sub-class */
class UpFiles {

private:
   const char *m_uuid;         /* unique id string */
   char *m_idstr;
   char *m_uri;          /* URI */
   char *m_filename;     /* file name */
   const char *m_protoVer;     /* HTTP version string */

   /* initialize: initialize data */
   void initialize();

   /* clear: clear data */
   void clear();

public:

   /* Constructor */
   UpFiles(const char *uri, const char *filename, const char *identifier, const char *uuid, const char *protoVer);

   /* Destructor */
   ~UpFiles();

   /* upload: do upload */
   bool upload();

   /* getFileName: get filename */
   const char *getFileName();

};

/* ContentUpload: upload manager */
class ContentUpload
{
private:

   MMDAgent *m_mmdagent;      /* mmdagent whose member function may be called */
   int m_id;                  /* mmdagent module id */

   GLFWmutex m_mutex;              /* mutex */
   GLFWthread m_thread;            /* thread */
   bool m_kill;

   char *m_url;
   char *m_dir;
   char *m_uuid;
   char *m_versionString;
   UpFiles *m_files[CONTENTUPLOAD_MAXFILENUM];    /* list of files to be downloaded*/
   int m_num;                  /* number of above */

   bool m_refresh;
   bool m_finished;
   bool m_error;

   /* initialize: initialize */
   void initialize();

   /* clear: free */
   void clear();

   /* clearMemoryFromMainThread: clear memory from main thread */
   void clearMemoryFromMainThread();

public:

   /* ContentUpload: constructor */
   ContentUpload();

   /* ~ContentUpload: destructor */
   ~ContentUpload();

   /* setupAndStart: setup manager and start thread */
   void setupAndStart(MMDAgent *mmdagent, int id, const char *destUrl, const char *contentDir, const char *HTTPVersionString);

   /* run: main thread loop */
   void run();

   /* stopAndRelease: stop thread and free */
   void stopAndRelease();

   /* isFinished: return true when thread has been finished */
   bool isFinished();

   /* hasError: return true when an error occured */
   bool hasError();

   /* requestRefresh: request to run again */
   void requestRefresh();
};
