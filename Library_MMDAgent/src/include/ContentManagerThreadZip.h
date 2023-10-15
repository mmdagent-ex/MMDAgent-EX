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

/* ContentManagerThreadZip: content manager for extracting zip archive */
class ContentManagerThreadZip
{
private:

   MMDAgent *m_mmdagent;      /* mmdagent whose member function may be called */
   int m_id;                  /* mmdagent module id */

   GLFWmutex m_mutex;              /* mutex */
   GLFWthread m_thread;            /* thread */
   bool m_kill;

   char *m_srcFile;
   char *m_dstDir;
   bool m_finished;
   bool m_error;
   char *m_mdfFile;

   unsigned long m_totalSize;
   unsigned long m_currentSize;

   /* initialize: initialize */
   void initialize();

   /* clear: free */
   void clear();

   /* unzipAndFindConfig: unzip archive and find config file */
   bool unzipAndFindConfig(const char *zip, const char *tempDirName, char **configFileNameRet, bool convertZipPathEncoding);

public:

   /* ContentManagerThreadZip: constructor */
   ContentManagerThreadZip();

   /* ~ContentManagerThreadZip: destructor */
   ~ContentManagerThreadZip();

   /* setupAndStart: setup manager and start thread */
   void setupAndStart(MMDAgent *mmdagent, int id, const char *source, const char *savedir);

   /* run: main thread loop */
   void run();

   /* stopAndRelease: stop thread and free */
   void stopAndRelease();

   /* isFinished: return true when thread has been finished */
   bool isFinished();

   /* hasError: return true when an error occured */
   bool hasError();

   /* getContentMDFFile: get content mdf file */
   const char *getContentMDFFile();

   /* getProgress: get progress information */
   void getProgress(char *buff_ret, float *rate_ret);
};
