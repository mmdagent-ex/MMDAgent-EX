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
#include "Poco/Net/HTTPSClientSession.h"
#include "Poco/Net/HTTPClientSession.h"
#include "Poco/Net/HTTPIOStream.h"
#include "Poco/Net/HTTPRequest.h"
#include "Poco/Net/HTTPResponse.h"
#include "Poco/Net/HTTPCredentials.h"
#include "Poco/Net/NetException.h"
#include "Poco/URI.h"
#include "Poco/UnbufferedStreamBuf.h"
#include "Poco/NullStream.h"
#include "Poco/StreamCopier.h"
#include "Poco/Version.h"
#define MAX_REDIRECTS 10

#include <memory>
#include <iostream>
#include <fstream>

#include <openssl/sha.h>

#include "MMDAgent.h"

/* definitions */
#define SHA256_FILE_READ_BUFFER_SIZE (1024 * 1024 * 30)

#define byte2mb(A) (float)(((A) + 52429.0f) / 1048576.0f)

enum ReturnCode {
   RC_UNDEFINED,       /* undefined */
   RC_SUCCESS,         /* generic success flag */
   RC_SKIP,            /* same file exists, not fetch */
   RC_NEW,             /* new file, fetch */
   RC_RENEW,           /* file size changes, fetch */
   RC_FAIL_SYS,        /* failed to write */
   RC_FAIL_NET         /* failed to download */
};

/* makedirlist: get files list under dir */
void makedirlist(const char *dir, KeyValue *k)
{
   DIRECTORY *d;
   char buff[MMDAGENT_MAXBUFLEN];
   char buff2[MMDAGENT_MAXBUFLEN];

   d = MMDAgent_opendir(dir);
   if (d == NULL)
      return;
   while (MMDAgent_readdir(d, buff) == true) {
      if (buff[0] == '.' || buff[0] == '_' || MMDAgent_strequal(buff, MMDAGENT_LOGFILEDIRNAME) == true)
         continue;
      MMDAgent_snprintf(buff2, MMDAGENT_MAXBUFLEN, "%s%c%s", dir, MMDAGENT_DIRSEPARATOR, buff);
      if (MMDAgent_stat(buff2) == MMDAGENT_STAT_DIRECTORY)
         makedirlist(buff2, k);
      else
         k->setString(buff2, "exist");
   }
   MMDAgent_closedir(d);
}


/* convert string-represented SHA256 digest to binary */
void string_to_sha256(const char* string, unsigned char output[SHA256_DIGEST_LENGTH]) {
   for (size_t i = 0; i < SHA256_DIGEST_LENGTH; i++) {
      sscanf(string + i * 2, "%2hhx", &output[i]);
   }
}


/* calculate SHA256 digest of a file */
bool calculate_sha256(FILE *file, unsigned char output[SHA256_DIGEST_LENGTH]) {
   SHA256_CTX sha256;
   char *buffer;
   size_t bufsize = SHA256_FILE_READ_BUFFER_SIZE;
   size_t bytes_read;

   buffer = (char *)malloc(bufsize);
   if (!buffer)
      return false;

   if (!SHA256_Init(&sha256))
      return false;

   while ((bytes_read = fread(buffer, 1, bufsize, file))) {
      if (!SHA256_Update(&sha256, buffer, bytes_read))
         return false;
   }

   if (ferror(file))
      return false;

   if (!SHA256_Final(output, &sha256))
      return false;

   free(buffer);
   return true;
}

/* DownFiles::initialize: initialize data */
void DownFiles::initialize()
{
   m_uri = NULL;
   m_size = 0;
   m_dirname = NULL;
   m_savefile = NULL;
   m_status = RC_UNDEFINED;
   m_fid = 0;
   m_session = NULL;
   m_isssl = false;
   m_kill = false;
   m_preserve = false;
}

/* DownFiles::clear: clear data */
void DownFiles::clear()
{
   if (m_uri) free(m_uri);
   if (m_savefile) free(m_savefile);
   if (m_dirname) free(m_dirname);
   if (m_session) abort();
   initialize();
}

/* DownFiles::Constructor */
DownFiles::DownFiles(char *uri, size_t size, char *savefile, char *dirname, bool preserve, unsigned char *digest)
{
   initialize();
   m_uri = MMDAgent_strdup(uri);
   m_size = size;
   m_currentSize = 0;
   m_dirname = MMDAgent_strdup(dirname);
   m_savefile = MMDAgent_strdup(savefile);
   m_preserve = preserve;
   memcpy(m_digest, digest, SHA256_DIGEST_LENGTH);
}

/* DownFiles::Destructor */
DownFiles::~DownFiles()
{
   clear();
}

/* DownFiles::getSize: get size */
size_t DownFiles::getSize()
{
   return m_size;
}

/* DownFiles::getCurrentSize: get current size */
size_t DownFiles::getCurrentSize()
{
   return m_currentSize;
}

/* DownFiles::getFileName: get filename */
const char *DownFiles::getFileName()
{
   return m_savefile;
}

/* DownFiles::setId: set id */
void DownFiles::setId(int id)
{
   m_fid = id;
}

/* DownFiles::getId: get id */
int DownFiles::getId()
{
   return m_fid;
}

/* DownFiles::check: check if need download */
int DownFiles::check()
{
   int ret;
   char buff[MMDAGENT_MAXBUFLEN];
   unsigned char digest[SHA256_DIGEST_LENGTH];

   MMDAgent_snprintf(buff, MMDAGENT_MAXBUFLEN, "%s%c%s", m_dirname, MMDAGENT_DIRSEPARATOR, m_savefile);
   if (MMDAgent_exist(buff) == false) {
      /* not exist */
      ret = RC_NEW;
   } else {
      FILE *fp;
      if ((fp = MMDAgent_fopen(buff, "rb")) != NULL && calculate_sha256(fp, digest)) {
         /* has sha256 digest, compare it with existing file */
         if (memcmp(digest, m_digest, SHA256_DIGEST_LENGTH) == 0) {
            /* exactly the same */
            ret = RC_SKIP;
         } else {
            ret = RC_RENEW;
         }
         fclose(fp);
      } else {
         /* no sha256 digest, judge only by the file size */
         if (MMDFiles_getfsize(buff) == m_size) /* exist, same size */
            ret = RC_SKIP;
         else /* exist, different size */
            ret = RC_RENEW;
      }
   }

   m_status = ret;

   return ret;
}

/* DownFiles::download: do download */
int DownFiles::download()
{
   FILE *fp;
   char buff[MMDAGENT_MAXBUFLEN];
   char buff2[MMDAGENT_MAXBUFLEN];
   char *path;
   size_t i, len;

   if (m_status != RC_NEW && m_status != RC_RENEW) return RC_SUCCESS;

   /* testing if output file can be opened for writing */
   MMDAgent_snprintf(buff, MMDAGENT_MAXBUFLEN, "%s%c%s", m_dirname, MMDAGENT_DIRSEPARATOR, m_savefile);
   fp = MMDAgent_fopen(buff, "wb");
   if (fp == NULL) {
      /* when write failed, make that directory and retry */
      len = MMDAgent_strlen(buff);
      for (i = 0; i < len; i++) {
         if (buff[i] == MMDAGENT_DIRSEPARATOR) {
            strcpy(buff2, buff);
            buff2[i] = '\0';
            MMDAgent_mkdir(buff2);
         }
      }
      fp = MMDAgent_fopen(buff, "wb");
      if (fp == NULL) /* file open failed */ {
         if (m_preserve) {
            /* save with another file prefix */
            MMDAgent_snprintf(buff2, MMDAGENT_MAXBUFLEN, "%s_updating", buff);
            strcpy(buff, buff2);
            fp = MMDAgent_fopen(buff, "wb");
         }
      }
      if (fp == NULL)
         return RC_FAIL_SYS;
   }
   fclose(fp);

   if (m_size == 0) /* file of 0 size has been created, exit here */
      return RC_SUCCESS;

   m_currentSize = 0;

   /* do download, saving current size to m_currentSize */
   path = MMDFiles_pathdup_from_application_to_system_locale(buff);
   if (path == NULL)
      return RC_FAIL_SYS;

   m_session = NULL;

   Poco::Net::HTTPClientSession *pSession = NULL;
   Poco::Net::HTTPResponse res;
   std::string encoded;
   Poco::URI::encode(m_savefile, "", encoded);
   std::string base(m_uri);
   Poco::URI uri(base + "/" + encoded);

   try {
      bool retry = false;
      bool authorize = false;
      int redirects = 0;
      std::string username;
      std::string password;

      do {
         if (!pSession) {
            if (uri.getScheme() != "http") {
               pSession = new Poco::Net::HTTPSClientSession(uri.getHost(), uri.getPort());
               m_isssl = true;
            } else {
               pSession = new Poco::Net::HTTPClientSession(uri.getHost(), uri.getPort());
               m_isssl = false;
            }
         }
         std::string qpath = uri.getPathAndQuery();
         if (qpath.empty()) qpath = "/";
         Poco::Net::HTTPRequest req(Poco::Net::HTTPRequest::HTTP_GET, qpath, Poco::Net::HTTPMessage::HTTP_1_1);
         if (authorize) {
            Poco::Net::HTTPCredentials::extractCredentials(uri, username, password);
            Poco::Net::HTTPCredentials cred(username, password);
            cred.authenticate(req, res);
         }
         req.set("User-Agent", Poco::format("poco/%d.%d.%d", (POCO_VERSION >> 24) & 0xFF, (POCO_VERSION >> 16) & 0xFF, (POCO_VERSION >> 8) & 0xFF));
         req.set("Accept", "*/*");
         Poco::Timespan time;
         time.assign(CONTENTDOWNLOAD_TIMEOUT_SEC, 0);
         pSession->setTimeout(time);
         if (m_kill) {
            if (pSession)
               delete pSession;
            free(path);
            return RC_FAIL_NET;
         }
         m_session = (void *)pSession;
         pSession->sendRequest(req);
         m_session = NULL;
         if (m_kill) {
            if (pSession)
               delete pSession;
            free(path);
            return RC_FAIL_NET;
         }
         m_session = (void *)pSession;
         std::istream& rs = pSession->receiveResponse(res);
         m_session = NULL;
         bool moved = (res.getStatus() == Poco::Net::HTTPResponse::HTTP_MOVED_PERMANENTLY ||
            res.getStatus() == Poco::Net::HTTPResponse::HTTP_FOUND ||
            res.getStatus() == Poco::Net::HTTPResponse::HTTP_SEE_OTHER ||
            res.getStatus() == Poco::Net::HTTPResponse::HTTP_TEMPORARY_REDIRECT);
         if (moved) {
            uri.resolve(res.get("Location"));
            if (!username.empty()) {
               uri.setUserInfo(username + ":" + password);
               authorize = false;
            }
            delete pSession;
            pSession = NULL;
            ++redirects;
            retry = true;
         } else if (res.getStatus() == Poco::Net::HTTPResponse::HTTP_OK) {
            std::ofstream ofs(path, std::ios_base::out | std::ios_base::binary);
            {
               std::istream &istr = rs;
               std::ostream &ostr = ofs;

               Poco::Buffer<char> buffer(8192);
               std::streamsize len = 0;
               istr.read(buffer.begin(), 8192);
               std::streamsize n = istr.gcount();
               while (n > 0) {
                  len += n;
                  m_currentSize = (size_t)len;
                  ostr.write(buffer.begin(), n);
                  if (istr && ostr) {
                     istr.read(buffer.begin(), 8192);
                     n = istr.gcount();
                  } else {
                     n = 0;
                  }
                  if (m_kill) {
                     if (pSession)
                        delete pSession;
                     free(path);
                     return RC_FAIL_NET;
                  }
               }
            }
            ofs.close();
            delete pSession;
            pSession = NULL;
            retry = false;
         } else if (res.getStatus() == Poco::Net::HTTPResponse::HTTP_UNAUTHORIZED && !authorize) {
            authorize = true;
            retry = true;
            Poco::NullOutputStream null;
            Poco::StreamCopier::copyStream(rs, null);
         } else {
            throw Poco::Net::HTTPException(res.getReason(), uri.toString());
         }
      } while (retry && redirects < MAX_REDIRECTS);

      if (redirects >= MAX_REDIRECTS)
         throw Poco::Net::HTTPException("Too many redirects", uri.toString());
   } catch (...) {
      if (pSession)
         delete pSession;
      m_session = NULL;
      return RC_FAIL_NET;
   }

   free(path);
   m_currentSize = m_size;

   return RC_SUCCESS;
}

/* DownFiles::abort: abort download */
void DownFiles::abort()
{
   m_kill = true;

   if (m_isssl) {
      Poco::Net::HTTPSClientSession *session = (Poco::Net::HTTPSClientSession *)m_session;
      if (session) {
         m_session = NULL;
         try {
            session->abort();
         }
         catch (Poco::Exception&) {
            /* do nothing */
         }
      }
   } else {
      Poco::Net::HTTPClientSession *session = (Poco::Net::HTTPClientSession *)m_session;
      if (session) {
         m_session = NULL;
         try {
            session->abort();
         }
         catch (Poco::Exception&) {
            /* do nothing */
         }
      }
   }
}

/**********************************************************/

/* mainThread: main thread for ContentManagerThreadWeb */
static void mainThread(void *param)
{
   ContentManagerThreadWeb *c = (ContentManagerThreadWeb *)param;
   c->run();
}

/* ContentManagerThreadWeb::initialize: initialize */
void ContentManagerThreadWeb::initialize()
{
   m_mmdagent = NULL;
   m_id = 0;

   m_mutex = NULL;
   m_thread = -1;
   m_kill = false;

   m_url = NULL;
   m_saveDir = NULL;
   for (int i = 0; i < CONTENTDOWNLOAD_MAXFILENUM; i++)
      m_files[i] = NULL;
   m_num = 0;
   m_fileIndex = NULL;
   m_finished = false;
   m_error = false;
   m_fileUpdated = false;
   m_mdfFile = NULL;

   m_totalSize = 0;
   m_currentId = 0;
   m_currentSize = 0;
   m_fetchNum = 0;
   m_fetchSize = 0;
   m_currentFetchedSize = 0;
   m_fetchCount = 0;
   m_fetchStarted = false;
   m_fetchContentListOnly = false;
   m_contentMessage = NULL;
   m_session = NULL;
   m_isssl = false;
   m_preserve = false;

   MMDAgent_enablepoco();
}

/* ContentManagerThreadWeb::clear: free */
void ContentManagerThreadWeb::clear()
{
   m_kill = true;

   if (m_mutex != NULL || m_thread >= 0) {
      if (m_thread >= 0) {
         abort();
         glfwWaitThread(m_thread, GLFW_WAIT);
      }
      if (m_mutex != NULL)
         glfwDestroyMutex(m_mutex);
   }

   if (m_url)
      free(m_url);
   if (m_saveDir)
      free(m_saveDir);
   for (int i = 0; i < CONTENTDOWNLOAD_MAXFILENUM; i++)
      if (m_files[i])
         delete m_files[i];
   if (m_fileIndex)
      delete m_fileIndex;
   if (m_mdfFile)
      free(m_mdfFile);
   if (m_contentMessage)
      free(m_contentMessage);

   initialize();
}

/* ContentManagerThreadWeb::clearMemoryFromMainThread: clear memory from main thread */
void ContentManagerThreadWeb::clearMemoryFromMainThread()
{
   if (m_url)
      free(m_url);
   m_url = NULL;
   if (m_saveDir)
      free(m_saveDir);
   m_saveDir = NULL;
}

/* ContentManagerThreadWeb::fetchFile: fetch a single file */
bool ContentManagerThreadWeb::fetchFile(const char *urlString, const char *savePath)
{
   char *path;

   /* fetch Contents list from the url */
   path = MMDFiles_pathdup_from_application_to_system_locale(savePath);
   if (path == NULL) {
      m_mmdagent->sendLogString(m_id, MLOG_ERROR, "unable to convert saving path to utf8: %s", savePath);
      return false;
   }

   Poco::Net::HTTPClientSession *pSession = NULL;
   Poco::Net::HTTPResponse res;
   Poco::URI uri(urlString);

   try {
      bool retry = false;
      bool authorize = false;
      int redirects = 0;
      std::string username;
      std::string password;

      do {
         if (!pSession) {
            if (uri.getScheme() != "http") {
               pSession = new Poco::Net::HTTPSClientSession(uri.getHost(), uri.getPort());
               m_isssl = true;
            } else {
               pSession = new Poco::Net::HTTPClientSession(uri.getHost(), uri.getPort());
               m_isssl = false;
            }
         }
         std::string qpath = uri.getPathAndQuery();
         if (qpath.empty()) qpath = "/";
         Poco::Net::HTTPRequest req(Poco::Net::HTTPRequest::HTTP_GET, qpath, Poco::Net::HTTPMessage::HTTP_1_1);
         if (authorize) {
            Poco::Net::HTTPCredentials::extractCredentials(uri, username, password);
            Poco::Net::HTTPCredentials cred(username, password);
            cred.authenticate(req, res);
         }
         req.set("User-Agent", Poco::format("poco/%d.%d.%d", (POCO_VERSION >> 24) & 0xFF, (POCO_VERSION >> 16) & 0xFF, (POCO_VERSION >> 8) & 0xFF));
         req.set("Accept", "*/*");
         Poco::Timespan time;
         time.assign(CONTENTDOWNLOAD_TIMEOUT_SEC, 0);
         pSession->setTimeout(time);
         if (m_kill) {
            if (pSession)
               delete pSession;
            free(path);
            return false;
         }
         m_session = (void *)pSession;
         pSession->sendRequest(req);
         m_session = NULL;
         if (m_kill)
            return false;
         m_session = (void *)pSession;
         std::istream& rs = pSession->receiveResponse(res);
         m_session = NULL;
         if (m_kill)
            return false;
         bool moved = (res.getStatus() == Poco::Net::HTTPResponse::HTTP_MOVED_PERMANENTLY ||
            res.getStatus() == Poco::Net::HTTPResponse::HTTP_FOUND ||
            res.getStatus() == Poco::Net::HTTPResponse::HTTP_SEE_OTHER ||
            res.getStatus() == Poco::Net::HTTPResponse::HTTP_TEMPORARY_REDIRECT);
         if (moved) {
            uri.resolve(res.get("Location"));
            if (!username.empty()) {
               uri.setUserInfo(username + ":" + password);
               authorize = false;
            }
            delete pSession;
            pSession = 0;
            ++redirects;
            retry = true;
         } else if (res.getStatus() == Poco::Net::HTTPResponse::HTTP_OK) {
            std::ofstream ofs(path, std::ios_base::out | std::ios_base::binary);
            Poco::StreamCopier::copyStream(rs, ofs);
            ofs.close();
            delete pSession;
            pSession = NULL;
            retry = false;
         } else if (res.getStatus() == Poco::Net::HTTPResponse::HTTP_UNAUTHORIZED && !authorize) {
            authorize = true;
            retry = true;
            Poco::NullOutputStream null;
            Poco::StreamCopier::copyStream(rs, null);
         } else {
            throw Poco::Net::HTTPException(res.getReason(), uri.toString());
         }
      } while (retry && redirects < MAX_REDIRECTS);

      if (redirects >= MAX_REDIRECTS)
         throw Poco::Net::HTTPException("Too many redirects", uri.toString());
   } catch (...) {
      if (pSession)
         delete pSession;
      m_session = NULL;
      free(path);
      return false;
   }
   free(path);
   return true;
}

/* ContentManagerThreadWeb::abort: abort */
void ContentManagerThreadWeb::abort()
{
   /* close fetch file socket on another thread */
   if (m_isssl) {
      Poco::Net::HTTPSClientSession *session = (Poco::Net::HTTPSClientSession *)m_session;
      if (session) {
         m_session = NULL;
         try {
            session->abort();
         } catch (Poco::Exception&) {
            /* do nothing */
         }
      }
   } else {
      Poco::Net::HTTPClientSession *session = (Poco::Net::HTTPClientSession *)m_session;
      if (session) {
         m_session = NULL;
         try {
            session->abort();
         } catch (Poco::Exception&) {
            /* do nothing */
         }
      }
   }

   /* close file downloading socket on another thread */
   if (m_fetchStarted)
      m_files[m_currentId]->abort();
}

/* ContentManagerThreadWeb::ContentManagerThreadWeb: constructor */
ContentManagerThreadWeb::ContentManagerThreadWeb()
{
   initialize();
}

/* ContentManagerThreadWeb::~ContentManagerThreadWeb: destructor */
ContentManagerThreadWeb::~ContentManagerThreadWeb()
{
   clear();
}

/* ContentManagerThreadWeb::setupAndStart: setup manager and start thread */
void ContentManagerThreadWeb::setupAndStart(MMDAgent *mmdagent, int id, const char *sourceUrl, const char *savedir, bool fetchContentListOnly, bool preserve)
{
   m_mmdagent = mmdagent;
   m_id = id;
   m_url = MMDAgent_strdup(sourceUrl);
   m_saveDir = MMDAgent_strdup(savedir);

   m_finished = false;
   m_error = false;
   m_fileUpdated = false;
   m_mdfFile = NULL;
   m_totalSize = 0;
   m_fetchStarted = false;
   m_fetchContentListOnly = fetchContentListOnly;
   m_preserve = preserve;

   m_mutex = glfwCreateMutex();
   m_thread = glfwCreateThread(mainThread, this);
   if (m_mutex == NULL || m_thread < 0) {
      clear();
      return;
   }
}

/* ContentManagerThreadWeb::run: main thread loop */
void ContentManagerThreadWeb::run()
{
   char buff[MMDAGENT_MAXBUFLEN];
   char buff2[MMDAGENT_MAXBUFLEN];
   char buf[1024];
   char *configFileNameInContent;
   char *base;
   ZFile *zf;
   char *p, *save;
   int dircount1, dircount2;
   size_t size, len, j;
   int ret;
   KeyValue *desc, *prop;
   int i;
   time_t epochtime_lastmodified = 0, epochtime_extracted;
   unsigned char digest[SHA256_DIGEST_LENGTH];

   /* fetch Contents list from the url */
   if (m_url[MMDAgent_strlen(m_url) - 1] == '/') {
      MMDAgent_snprintf(buff, MMDAGENT_MAXBUFLEN, "%s%s", m_url, CONTENTMANAGER_CONTENTFILE);
   } else {
      MMDAgent_snprintf(buff, MMDAGENT_MAXBUFLEN, "%s/%s", m_url, CONTENTMANAGER_CONTENTFILE);
   }
   MMDAgent_snprintf(buff2, MMDAGENT_MAXBUFLEN, "%s%c%s", m_saveDir, MMDAGENT_DIRSEPARATOR, CONTENTMANAGER_CONTENTFILE);
   if (fetchFile(buff, buff2)) {
      m_mmdagent->sendLogString(m_id, MLOG_STATUS, "fetched %s", buff);
   } else {
      m_mmdagent->sendLogString(m_id, MLOG_WARNING, "unable to download or failed to save %s", buff);
      clearMemoryFromMainThread();
      m_error = true;
      m_finished = true;
      return;
   }

   if (m_kill == true)
      return;

   /* parse obtained content file list to get number of files, total size and mdf exec file */
   m_num = 0;
   m_totalSize = 0;
   dircount1 = -1;
   configFileNameInContent = NULL;
   /* assume the content file list is NOT encrypted */
   zf = new ZFile(NULL);
   if (zf->openAndLoad(buff2) == false) {
      m_mmdagent->sendLogString(m_id, MLOG_ERROR, "unable to open or failed to save %s", buff);
      clearMemoryFromMainThread();
      m_error = true;
      m_finished = true;
      delete zf;
      return;
   }

   m_fileIndex = new KeyValue();
   m_fileIndex->setup();

   if (zf->gets(buf, 1024))
      epochtime_lastmodified = (time_t)atoll(buf);

   while (zf->gets(buf, 1024) != NULL) {
      if (buf[0] == '#') continue;
      /* file size */
      p = MMDAgent_strtok(buf, " \r\n", &save);
      if (p == NULL) continue;
      size = (size_t)MMDAgent_str2int(p);
      /* hash */
      p = MMDAgent_strtok(NULL, " \r\n", &save);
      if (p == NULL) continue;
      string_to_sha256(p, digest);
      /* file name */
      p = MMDAgent_strtok(NULL, "\r\n", &save);
      if (p == NULL) continue;
      if (MMDAgent_strtailmatch(p, ".mdf") || MMDAgent_strtailmatch(p, ".MDF")) {
         dircount2 = 0;
         len = MMDAgent_strlen(p);
         for (j = 0; j < len; j++)
            if (p[j] == MMDAGENT_DIRSEPARATOR)
               dircount2++;
         if (dircount1 < dircount2) {
            if (configFileNameInContent)
               free(configFileNameInContent);
            configFileNameInContent = MMDAgent_strdup(p);
            dircount1 = dircount2;
         }
      }
      if (m_num >= CONTENTDOWNLOAD_MAXFILENUM) {
         m_mmdagent->sendLogString(m_id, MLOG_ERROR, "number of files exceeds limit (%d)", CONTENTDOWNLOAD_MAXFILENUM);
         if (configFileNameInContent)
            free(configFileNameInContent);
         clearMemoryFromMainThread();
         m_error = true;
         m_finished = true;
         delete zf;
         return;
      }
      m_files[m_num++] = new DownFiles(m_url, size, p, m_saveDir, m_preserve, digest);
      m_totalSize += size;
      MMDAgent_snprintf(buff2, MMDAGENT_MAXBUFLEN, "%s%c%s", m_saveDir, MMDAGENT_DIRSEPARATOR, p);
      m_fileIndex->setString(buff2, "exist");
   }
   zf->close();
   delete zf;

   if (m_fetchContentListOnly) {
      /* do not download, just update time stampcheck */
      KeyValue *prop;

      sprintf(buff, "%s%c%s", m_saveDir, MMDAGENT_DIRSEPARATOR, MMDAGENT_CONTENTINFOFILE);
      prop = new KeyValue;
      prop->setup();
      if (prop->load(buff, NULL)) {
         prop->setString("LastModifiedEpochTime", "%u", epochtime_lastmodified);
         time(&epochtime_extracted);
      }
      prop->save(buff);
      delete prop;

      clearMemoryFromMainThread();
      m_finished = true;
      return;
   }

   if (m_kill == true)
      return;

   /* fetch or renew package description from the url */
   if (m_url[MMDAgent_strlen(m_url) - 1] == '/') {
      MMDAgent_snprintf(buff, MMDAGENT_MAXBUFLEN, "%s%s", m_url, CONTENTMANAGER_PACKAGEFILE);
   } else {
      MMDAgent_snprintf(buff, MMDAGENT_MAXBUFLEN, "%s/%s", m_url, CONTENTMANAGER_PACKAGEFILE);
   }
   MMDAgent_snprintf(buff2, MMDAGENT_MAXBUFLEN, "%s%c%s", m_saveDir, MMDAGENT_DIRSEPARATOR, CONTENTMANAGER_PACKAGEFILE);
   fetchFile(buff, buff2);

   if (m_kill == true)
      return;

   /* extract package description */
   desc = new KeyValue;
   desc->setup();
   desc->load(buff2, g_enckey);

   /* finally set mdf file name */
   if (desc->exist("execMDFFile")){
      if (configFileNameInContent)
         free(configFileNameInContent);
      configFileNameInContent = MMDAgent_strdup(desc->getString("execMDFFile", ""));
   }

   /* make MDF file to absolute */
   if (configFileNameInContent) {
      sprintf(buff, "%s%c%s", m_saveDir, MMDAGENT_DIRSEPARATOR, configFileNameInContent);
      if (m_mdfFile)
         free(m_mdfFile);
      m_mdfFile = MMDAgent_strdup(buff);
   }

   /* check if running on this platform is allowed BEFORE DOWNLOADING CONTENTS */
   if (MMDAgent_inDesktopOS() && desc->exist("nonDesktop")){
      if (MMDAgent_strequal(desc->getString("nonDesktop", ""), "true") || MMDAgent_strequal(desc->getString("nonDesktop", ""), "True") || MMDAgent_strequal(desc->getString("nonDesktop", ""), "TRUE")) {
         m_contentMessage = MMDAgent_strdup(CONTENTMANAGER_NONDESKTOPMESSAGE);
         if (configFileNameInContent)
            free(configFileNameInContent);
         clearMemoryFromMainThread();
         delete desc;
         m_finished = true;
         return;
      }
   }

   /* delete files not in the file index */
   KeyValue *v = new KeyValue();
   v->setup();
   makedirlist(m_saveDir, v);
   void *psave;
   for (const char *p = v->firstKey(&psave); p; p = v->nextKey(&psave)) {
      if (m_fileIndex->exist(p) == false) {
         m_mmdagent->sendLogString(m_id, MLOG_STATUS, "delete: %s", p);
         MMDAgent_removefile(p);
      }
   }
   delete v;

   /* check local file status */
   m_fetchSize = 0;
   m_fetchNum = 0;
   for (int i = 0; i < m_num; i++) {
      switch (m_files[i]->check()) {
      case RC_SKIP:
         m_files[i]->setId(-1);
         break;
      case RC_NEW:
         m_mmdagent->sendLogString(m_id, MLOG_STATUS, "new: %s (%d bytes)", m_files[i]->getFileName(), m_files[i]->getSize());
         m_files[i]->setId(m_fetchNum);
         m_fetchSize += m_files[i]->getSize();
         m_fetchNum++;
         break;
      case RC_RENEW:
         m_mmdagent->sendLogString(m_id, MLOG_STATUS, "update: %s (%d bytes)", m_files[i]->getFileName(), m_files[i]->getSize());
         m_files[i]->setId(m_fetchNum);
         m_fetchSize += m_files[i]->getSize();
         m_fetchNum++;
         break;
      default:
         break;
      }
   }

   if (m_fetchNum == 0) {
      /* nothing new, finish */
      m_mmdagent->sendLogString(m_id, MLOG_STATUS, "clean, up-to-date");
      m_fileUpdated = false;
   } else {
      /* fetch content diffs */
      m_currentId = 0;
      m_currentSize = 0;
      m_currentFetchedSize = 0;
      m_fetchCount = 0;
      m_fetchStarted = true;
      for (int i = 0; i < m_num; i++) {
         m_currentId = i;
         m_currentSize += m_files[i]->getSize();
         if (m_files[i]->getId() != -1) {
            m_fetchCount++;
            ret = m_files[i]->download();
            if (m_kill == true)
               return;
            switch (ret) {
            case RC_SUCCESS:
               m_mmdagent->sendLogString(m_id, MLOG_STATUS, "fetched %s (%d bytes)", m_files[i]->getFileName(), m_files[i]->getSize());
               break;
            case RC_FAIL_NET:
               m_mmdagent->sendLogString(m_id, MLOG_ERROR, "failed to fetch %s (%d bytes)", m_files[i]->getFileName(), m_files[i]->getSize());
               break;
            case RC_FAIL_SYS:
               m_mmdagent->sendLogString(m_id, MLOG_ERROR, "failed to write %s (%d bytes)", m_files[i]->getFileName(), m_files[i]->getSize());
               break;
            default:
               break;
            }
            if (ret != RC_SUCCESS) {
               m_error = true;
               break;
            }
            m_currentFetchedSize += m_files[i]->getSize();
         }
         if (m_kill == true)
            return;
      }
      m_fetchStarted = false;
      if (m_error)
         m_mmdagent->sendLogString(m_id, MLOG_STATUS, "exit with error");
      else
         m_mmdagent->sendLogString(m_id, MLOG_STATUS, "fetched %d files, %.1f MBytes", m_fetchNum, byte2mb(m_fetchSize));
      m_fileUpdated = true;
   }
   /* release file lists */
   for (i = 0; i < m_num; i++) {
      delete m_files[i];
      m_files[i] = NULL;
   }

   /* store content information */
   prop = new KeyValue;
   prop->setup();
   prop->setString("ContentType", "%s", "http");
   prop->setString("SourceURL", "%s", m_url);
   if (configFileNameInContent)
      prop->setString("ExecMDFFile", "%s", configFileNameInContent);
   else
      prop->setString("ExecMDFFile", "");
   if (desc->exist("label")) {
      prop->setString("ContentName", "%s", desc->getString("label", ""));
   } else if (configFileNameInContent) {
      base = MMDAgent_basename(configFileNameInContent);
      if (MMDFiles_strtailmatch(base, ".mdf") || MMDFiles_strtailmatch(base, ".MDF")) {
         len = MMDFiles_strlen(base);
         base[len - 4] = '\0';
      }
      prop->setString("ContentName", "%s", base);
      free(base);
   } else {
      prop->setString("ContentName", "");
   }
   if (desc->exist("image"))
      prop->setString("ImageFile", desc->getString("image", ""));
   prop->setString("LastModifiedEpochTime", "%u", epochtime_lastmodified);
   time(&epochtime_extracted);
   prop->setString("ExtractedEpochTime", "%u", epochtime_extracted);
   prop->setString("DownloadCompleted", "%s", m_fetchNum == m_fetchCount ? "true" : "false");
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
   if (desc->exist("autoUpdateFiles"))
      prop->setString("AutoUpdateFiles", desc->getString("autoUpdateFiles", ""));
   if (desc->exist("autoUpdatePeriod"))
      prop->setString("AutoUpdatePeriod", desc->getString("autoUpdatePeriod", ""));
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
   sprintf(buff, "%s%c%s", m_saveDir, MMDAGENT_DIRSEPARATOR, MMDAGENT_CONTENTINFOFILE);
   prop->save(buff);

   delete prop;
   delete desc;

   if (configFileNameInContent)
      free(configFileNameInContent);

   clearMemoryFromMainThread();

   m_finished = true;

   return;
}

/* ContentManagerThreadWeb::stopAndRelease: stop thread and free */
void ContentManagerThreadWeb::stopAndRelease()
{
   clear();
}

/* ContentManagerThreadWeb::isFinished: return true when thread has been finished */
bool ContentManagerThreadWeb::isFinished()
{
   return m_finished;
}

/* ContentManagerThreadWeb::hasError: return true when an error occured */
bool ContentManagerThreadWeb::hasError()
{
   return m_error;
}

/* ContentManagerThreadWeb::isUpdated: return true when part of the content has been updated */
bool ContentManagerThreadWeb::isUpdated()
{
   return m_fileUpdated;
}

/* ContentManagerThreadWeb::getContentMDFFile: get content mdf file */
const char *ContentManagerThreadWeb::getContentMDFFile()
{
   return m_mdfFile;
}

/* ContentManagerThreadWeb::getProgress: get progress information */
void ContentManagerThreadWeb::getProgress(char *buff_ret, float *rate_ret)
{
   float rate;

   if (m_fetchStarted == false) {
      buff_ret[0] = '\0';
      *rate_ret = 0.0f;
      return;
   }

   if (m_fetchSize == 0)
      rate = 0.0f;
   else
      rate = (float)(m_currentFetchedSize + m_files[m_currentId]->getCurrentSize()) / (float)m_fetchSize;

   sprintf(buff_ret, "total %4.1fMB: downloading %4.1fMB - %3.1f%% done\n(%d/%d)[%4.1f/%4.1fMB] %s", byte2mb(m_totalSize), byte2mb(m_fetchSize), rate * 100.0f, m_fetchCount, m_fetchNum, byte2mb(m_files[m_currentId]->getCurrentSize()), byte2mb(m_files[m_currentId]->getSize()), m_files[m_currentId]->getFileName());

   *rate_ret = rate;
}

/* ContentManagerThreadWeb::getContentMessage: get content message */
char *ContentManagerThreadWeb::getContentMessage()
{
   return m_contentMessage;
}

/**********************************************************/

/* mainThread: main thread for TinyDownload */
static void mainThreadTinyDownload(void *param)
{
   TinyDownload *t = (TinyDownload *)param;
   t->run();
}

/* TinyDownload::initialize: initialize */
void TinyDownload::initialize()
{
   m_mmdagent = NULL;
   m_id = 0;

   m_mutex = NULL;
   m_thread = -1;

   m_baseurl = NULL;
   m_files = NULL;

   m_finished = false;
   m_error = false;
   m_kill = false;
   m_session = NULL;
   m_isssl = false;

   MMDAgent_enablepoco();
}

/* TinyDownload::clear: free */
void TinyDownload::clear()
{
   stopThread();

   if (m_baseurl)
      free(m_baseurl);
   if (m_files)
      free(m_files);

   initialize();
}

/* TinyDownload::startThread: start thread */
bool TinyDownload::startThread()
{
   m_kill = false;
   m_mutex = glfwCreateMutex();
   m_thread = glfwCreateThread(mainThreadTinyDownload, this);
   if (m_mutex == NULL || m_thread < 0) {
      m_mutex = NULL;
      m_thread = -1;
      return false;
   }
   return true;
}

/* TinyDownload::stopThread: stop thread */
void TinyDownload::stopThread()
{
   m_kill = true;
   if (m_mutex != NULL || m_thread >= 0) {
      if (m_thread >= 0) {
         abort();
         glfwWaitThread(m_thread, GLFW_WAIT);
      }
      if (m_mutex != NULL)
         glfwDestroyMutex(m_mutex);
   }
   m_mutex = NULL;
   m_thread = -1;
}

/* TinyDownload::fetchFile: fetch a single file */
bool TinyDownload::fetchFile(const char *urlString, const char *savePath)
{
   char *path;

   path = MMDFiles_pathdup_from_application_to_system_locale(savePath);
   if (path == NULL) {
      m_mmdagent->sendLogString(m_id, MLOG_ERROR, "unable to convert saving path to utf8: %s", savePath);
      return false;
   }


   Poco::Net::HTTPClientSession *pSession = NULL;
   Poco::Net::HTTPResponse res;
   Poco::URI uri(urlString);

   try {
      bool retry = false;
      bool authorize = false;
      int redirects = 0;
      std::string username;
      std::string password;

      do {
         if (!pSession) {
            if (uri.getScheme() != "http") {
               pSession = new Poco::Net::HTTPSClientSession(uri.getHost(), uri.getPort());
               m_isssl = true;
            } else {
               pSession = new Poco::Net::HTTPClientSession(uri.getHost(), uri.getPort());
               m_isssl = false;
            }
         }
         std::string qpath = uri.getPathAndQuery();
         if (qpath.empty()) qpath = "/";
         Poco::Net::HTTPRequest req(Poco::Net::HTTPRequest::HTTP_GET, qpath, Poco::Net::HTTPMessage::HTTP_1_1);
         if (authorize) {
            Poco::Net::HTTPCredentials::extractCredentials(uri, username, password);
            Poco::Net::HTTPCredentials cred(username, password);
            cred.authenticate(req, res);
         }
         req.set("User-Agent", Poco::format("poco/%d.%d.%d", (POCO_VERSION >> 24) & 0xFF, (POCO_VERSION >> 16) & 0xFF, (POCO_VERSION >> 8) & 0xFF));
         req.set("Accept", "*/*");
         Poco::Timespan time;
         time.assign(CONTENTDOWNLOAD_TIMEOUT_SEC, 0);
         pSession->setTimeout(time);
         if (m_kill) {
            if (pSession)
               delete pSession;
            free(path);
            return false;
         }
         m_session = (void *)pSession;
         pSession->sendRequest(req);
         m_session = NULL;
         if (m_kill)
            return false;
         m_session = (void *)pSession;
         std::istream& rs = pSession->receiveResponse(res);
         m_session = NULL;
         if (m_kill)
            return false;
         bool moved = (res.getStatus() == Poco::Net::HTTPResponse::HTTP_MOVED_PERMANENTLY ||
            res.getStatus() == Poco::Net::HTTPResponse::HTTP_FOUND ||
            res.getStatus() == Poco::Net::HTTPResponse::HTTP_SEE_OTHER ||
            res.getStatus() == Poco::Net::HTTPResponse::HTTP_TEMPORARY_REDIRECT);
         if (moved) {
            uri.resolve(res.get("Location"));
            if (!username.empty()) {
               uri.setUserInfo(username + ":" + password);
               authorize = false;
            }
            delete pSession;
            pSession = NULL;
            ++redirects;
            retry = true;
         } else if (res.getStatus() == Poco::Net::HTTPResponse::HTTP_OK) {
            std::ofstream ofs(path, std::ios_base::out | std::ios_base::binary);
            Poco::StreamCopier::copyStream(rs, ofs);
            ofs.close();
            delete pSession;
            pSession = NULL;
            retry = false;
         } else if (res.getStatus() == Poco::Net::HTTPResponse::HTTP_UNAUTHORIZED && !authorize) {
            authorize = true;
            retry = true;
            Poco::NullOutputStream null;
            Poco::StreamCopier::copyStream(rs, null);
         } else {
            throw Poco::Net::HTTPException(res.getReason(), uri.toString());
         }
      } while (retry && redirects < MAX_REDIRECTS);

      if (redirects >= MAX_REDIRECTS)
         throw Poco::Net::HTTPException("Too many redirects", uri.toString());
   }
   catch (...) {
      m_session = NULL;
      m_mmdagent->sendLogString(m_id, MLOG_WARNING, "unable to download or failed to save %s", urlString);
      free(path);
      return false;
   }
   free(path);
   m_mmdagent->sendLogString(m_id, MLOG_STATUS, "fetched %s", urlString);
   return true;
}

/* TinyDownload::TinyDownload: constructor */
TinyDownload::TinyDownload()
{
   initialize();
}

/* TinyDownload::~TinyDownload: destructor */
TinyDownload::~TinyDownload()
{
   clear();
}

/* TinyDownload::setupAndStart: setup manager and start thread */
void TinyDownload::setupAndStart(MMDAgent *mmdagent, int id, const char *baseUrl, const char *files)
{
   if (baseUrl == NULL || files == NULL)
      return;

   m_mmdagent = mmdagent;
   m_id = id;
   m_baseurl = MMDAgent_strdup(baseUrl);
   m_files = MMDAgent_strdup(files);

   m_finished = false;
   m_error = false;

   if (startThread() == false) {
      clear();
      return;
   }
}

/* TinyDownload::run: main thread loop */
void TinyDownload::run()
{
   char *p, *save;
   char *filelist;
   char buff1[MMDAGENT_MAXBUFLEN];
   char buff2[MMDAGENT_MAXBUFLEN];

   filelist = MMDAgent_strdup(m_files);
   for (p = MMDAgent_strtok(filelist, ",", &save); p; p = MMDAgent_strtok(NULL, ",", &save)) {
      if (m_kill)
         break;
      if (m_baseurl[MMDAgent_strlen(m_baseurl) - 1] == '/') {
         MMDAgent_snprintf(buff1, MMDAGENT_MAXBUFLEN, "%s%s", m_baseurl, p);
      } else {
         MMDAgent_snprintf(buff1, MMDAGENT_MAXBUFLEN, "%s/%s", m_baseurl, p);
      }
      MMDAgent_snprintf(buff2, MMDAGENT_MAXBUFLEN, "%s.tmp", p);
      MMDAgent_removefile(buff2);
      if (fetchFile(buff1, buff2) == false) {
         MMDAgent_removefile(buff2);
         m_error = true;
         m_finished = true;
         m_mmdagent->sendLogString(m_id, MLOG_WARNING, "failed to fetch %s", p);
         free(filelist);
         return;
      }
      if (m_kill)
         break;
      MMDAgent_removefile(p);
      if (MMDAgent_rename(buff2, p) == false) {
         MMDAgent_removefile(buff2);
         m_error = true;
         m_finished = true;
         m_mmdagent->sendLogString(m_id, MLOG_WARNING, "failed to rename %s to %s", buff2, p);
         free(filelist);
         return;
      }
   }

   m_error = false;
   m_finished = true;
   free(filelist);
}

/* TinyDownload::stopAndRelease: stop thread and free */
void TinyDownload::stopAndRelease()
{
   clear();
}

/* TinyDownload::restart: restart */
void TinyDownload::restart()
{
   stopThread();
   startThread();
}

/* TinyDownload::isFinished: return true when thread has been finished */
bool TinyDownload::isFinished()
{
   return m_finished;
}

/* TinyDownload::hasError: return true when an error occured */
bool TinyDownload::hasError()
{
   return m_error;
}

/* TinyDownload::abort: abort */
void TinyDownload::abort()
{
   /* close fetch file socket on another thread */
   if (m_isssl) {
      Poco::Net::HTTPSClientSession *session = (Poco::Net::HTTPSClientSession *)m_session;
      if (session) {
         m_session = NULL;
         try {
            session->abort();
         }
         catch (Poco::Exception&) {
            /* do nothing */
         }
      }
   } else {
      Poco::Net::HTTPClientSession *session = (Poco::Net::HTTPClientSession *)m_session;
      if (session) {
         m_session = NULL;
         try {
            session->abort();
         }
         catch (Poco::Exception&) {
            /* do nothing */
         }
      }
   }
}
