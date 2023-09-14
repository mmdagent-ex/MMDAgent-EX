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
