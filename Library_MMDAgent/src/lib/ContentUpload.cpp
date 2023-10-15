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
#include <time.h>
#include "Poco/StreamCopier.h"
#include "Poco/URI.h"
#include "Poco/Exception.h"
#include "Poco/UUID.h"
#include "Poco/Net/HTTPRequest.h"
#include "Poco/Path.h"
#include "Poco/Net/HTMLForm.h"
#include "Poco/Net/FilePartSource.h"
#include "Poco/Net/StringPartSource.h"
#include "Poco/Net/HTTPSClientSession.h"
#include "Poco/Net/HTTPResponse.h"
#include <memory>
#include <iostream>
#include <fstream>

#include "MMDAgent.h"

/* UpFiles::initialize: initialize data */
void UpFiles::initialize()
{
   m_uuid = NULL;
   m_idstr = NULL;
   m_uri = NULL;
   m_filename = NULL;
   m_protoVer = NULL;
}

/* UpFiles::clear: clear data */
void UpFiles::clear()
{
   if (m_idstr) free(m_idstr);
   if (m_uri) free(m_uri);
   if (m_filename) free(m_filename);
   initialize();
}

/* UpFiles::Constructor */
UpFiles::UpFiles(const char *uri, const char *filename, const char *identifier, const char *uuid, const char *protoVer)
{
   initialize();
   m_uri = MMDAgent_strdup(uri);
   m_filename = MMDAgent_strdup(filename);
   m_idstr = MMDAgent_strdup(identifier);
   m_uuid = uuid;
   m_protoVer = protoVer;
}

/* UpFiles::Destructor */
UpFiles::~UpFiles()
{
   clear();
}

/* UpFiles::upload: do upload */
bool UpFiles::upload()
{
   if (m_uri == NULL || m_filename == NULL || m_idstr == NULL || m_uuid == NULL)
      return false;

   try {
      // make HTTP request instance with POST and path
      Poco::URI uri(m_uri);
      std::string path(uri.getPathAndQuery());
      if (path.empty())
         path = "/";

      // make request
      Poco::Net::HTTPRequest request(Poco::Net::HTTPRequest::HTTP_POST, path, m_protoVer ? std::string(m_protoVer) : Poco::Net::HTTPMessage::HTTP_1_1);

      // sending as multipart message
      Poco::Net::HTMLForm form;

      // set encoding scheme
      form.setEncoding(Poco::Net::HTMLForm::ENCODING_MULTIPART);

      // add the contents to form
      Poco::Path fileName(m_filename, Poco::Path::PATH_UNIX);
#if defined(_WIN32)
      form.addPart("data", new Poco::Net::FilePartSource(fileName.toString(Poco::Path::PATH_WINDOWS)));
#else
      form.addPart("data", new Poco::Net::FilePartSource(fileName.toString(Poco::Path::PATH_NATIVE)));
#endif
      form.set("identifier", m_idstr);
      form.set("uuid", m_uuid);

      // prepare for submission
      form.prepareSubmit(request);

      // connect to server
      // create HTTP client session instance for the uri
      Poco::Net::HTTPSClientSession *httpSession = new Poco::Net::HTTPSClientSession(uri.getHost(), uri.getPort());
      httpSession->setTimeout(Poco::Timespan(CONTENTUPLOAD_TIMEOUTSEC, 0));
      // write the multipart
      form.write(httpSession->sendRequest(request));

      // get response
      Poco::Net::HTTPResponse res;
      std::istream &is = httpSession->receiveResponse(res);
      int status = (int)res.getStatus();
      std::string response;
      while (is)
         response.push_back(char(is.get()));

      // status check
      if (status < 200 && status >= 300)
         return false;
   }
   catch (Poco::Exception& exc) {
      std::cerr << exc.displayText() << std::endl;
      return false;
   }

   return true;
}

/* UpFiles::getFileName: get filename */
const char *UpFiles::getFileName()
{
   return m_filename;
}

/**********************************************************/

/* mainThread: main thread */
static void mainThread(void *param)
{
   ContentUpload *c = (ContentUpload *)param;
   c->run();
}

/* ContentUpload::initialize: initialize */
void ContentUpload::initialize()
{
   m_mmdagent = NULL;
   m_id = 0;

   m_mutex = NULL;
   m_thread = -1;
   m_kill = false;

   m_url = NULL;
   m_dir = NULL;
   m_uuid = NULL;
   m_versionString = NULL;
   for (int i = 0; i < CONTENTDOWNLOAD_MAXFILENUM; i++)
      m_files[i] = NULL;
   m_num = 0;
   m_refresh = false;
   m_finished = false;
   m_error = false;

   MMDAgent_enablepoco();
}

/* ContentUpload::clear: free */
void ContentUpload::clear()
{
   m_kill = true;

   if (m_mutex != NULL || m_thread >= 0) {
      if (m_thread >= 0) {
         //glfwWaitThread(m_thread, GLFW_WAIT);
         glfwDestroyThread(m_thread);
      }
      if (m_mutex != NULL)
         glfwDestroyMutex(m_mutex);
   }

   if (m_url)
      free(m_url);
   if (m_dir)
      free(m_dir);
   if (m_uuid)
      free(m_uuid);
   if (m_versionString)
      free(m_versionString);
   for (int i = 0; i < CONTENTDOWNLOAD_MAXFILENUM; i++)
      if (m_files[i])
         delete m_files[i];

   initialize();
}

/* ContentUpload::clearMemoryFromMainThread: clear memory from main thread */
void ContentUpload::clearMemoryFromMainThread()
{
   if (m_url)
      free(m_url);
   m_url = NULL;
   if (m_dir)
      free(m_dir);
   m_dir = NULL;
}

/* ContentUpload::ContentUpload: constructor */
ContentUpload::ContentUpload()
{
   initialize();
}

/* ContentUpload::~ContentUpload: destructor */
ContentUpload::~ContentUpload()
{
   clear();
}

/* ContentUpload::setupAndStart: setup manager and start thread */
void ContentUpload::setupAndStart(MMDAgent *mmdagent, int id, const char *destUrl, const char *contentDir, const char *HTTPVersionString)
{
   m_mmdagent = mmdagent;
   m_id = id;
   m_url = MMDAgent_strdup(destUrl);
   m_dir = MMDAgent_strdup(contentDir);
   m_versionString = MMDAgent_strdup(HTTPVersionString);
   m_finished = false;
   m_error = false;

   /* get UUID */
   m_uuid = MMDAgent_strdup(MMDAgent_getUUID());

   m_mutex = glfwCreateMutex();
   m_thread = glfwCreateThread(mainThread, this);
   if (m_mutex == NULL || m_thread < 0) {
      clear();
      return;
   }
}

/* ContentUpload::run: main thread loop */
void ContentUpload::run()
{
   char buff[MMDAGENT_MAXBUFLEN];
   char idstr[MMDAGENT_MAXBUFLEN];
   char path1[MMDAGENT_MAXBUFLEN];
   char path2[MMDAGENT_MAXBUFLEN];
   char path3[MMDAGENT_MAXBUFLEN];
   DIRECTORY *d1, *d2;
   int i, n;
   int sentNum;

   m_mmdagent->sendLogString(m_id, MLOG_STATUS, "uploading log files...", m_url);

   do {
      m_refresh = false;
      /* list up files to upload */
      n = 0;
      MMDAgent_snprintf(path1, MMDAGENT_MAXBUFLEN, "%s%c%s", m_dir, MMDAGENT_DIRSEPARATOR, MMDAGENT_LOGFILEDIRNAME);
      d1 = MMDAgent_opendir(path1);
      if (d1 == NULL)
         return;
      while (MMDAgent_readdir(d1, idstr) == true) {
         if (idstr[0] == '.') continue;
         MMDAgent_snprintf(path2, MMDAGENT_MAXBUFLEN, "%s%c%s", path1, MMDAGENT_DIRSEPARATOR, idstr);
         d2 = MMDAgent_opendir(path2);
         if (d2 == NULL) continue;
         while (MMDAgent_readdir(d2, buff) == true) {
            if (buff[0] == '.') continue;
            MMDAgent_snprintf(path3, MMDAGENT_MAXBUFLEN, "%s%c%s", path2, MMDAGENT_DIRSEPARATOR, buff);
            m_files[n++] = new UpFiles(m_url, path3, idstr, m_uuid, m_versionString);
         }
         MMDAgent_closedir(d2);
      }
      MMDAgent_closedir(d1);
      m_num = n;

      sentNum = 0;
      for (i = 0; i < m_num; i++) {
         if (m_files[i]->upload() == false) {
            m_mmdagent->sendLogString(m_id, MLOG_ERROR, "[%d/%d] failed %s", i + 1, m_num, m_files[i]->getFileName());
            m_error = true;
         } else {
            m_mmdagent->sendLogString(m_id, MLOG_STATUS, "[%d/%d] %s", i + 1, m_num, m_files[i]->getFileName());
            MMDAgent_removefile(m_files[i]->getFileName());
            sentNum++;
         }
      }

      if (m_num == 0)
         m_mmdagent->sendLogString(m_id, MLOG_STATUS, "no saved log, clean");
      else
         m_mmdagent->sendLogString(m_id, MLOG_STATUS, "%d files, %d uploaded and removed, %d remains", m_num, sentNum, m_num - sentNum);

      /* release file lists */
      for (i = 0; i < m_num; i++) {
         delete m_files[i];
         m_files[i] = NULL;
      }

      // if refresh is called while downloading, loop the whole
   } while (m_refresh == true);

   clearMemoryFromMainThread();

   m_finished = true;

   return;
}

/* ContentUpload::stopAndRelease: stop thread and free */
void ContentUpload::stopAndRelease()
{
   clear();
}

/* ContentUpload::isFinished: return true when thread has been finished */
bool ContentUpload::isFinished()
{
   return m_finished;
}

/* ContentUpload::hasError: return true when an error occured */
bool ContentUpload::hasError()
{
   return m_error;
}

/* ContentUpload::requestRefresh: request to run again */
void ContentUpload::requestRefresh()
{
   if (m_mutex == NULL || m_finished == true)
      return;
   m_refresh = true;
}
