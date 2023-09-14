/* ----------------------------------------------------------------- */
/*           The Toolkit for Building Voice Interaction Systems      */
/*           "MMDAgent" developed by MMDAgent Project Team           */
/*           http://www.mmdagent.jp/                                 */
/* ----------------------------------------------------------------- */
/*                                                                   */
/*  Copyright (c) 2009-2015  Nagoya Institute of Technology          */
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
#include "MMDAgent.h"
#include "Downloader.h"

/* Downloader::clear: clear data */
void Downloader::clear()
{
   if (m_uristring)
      free(m_uristring);
   if (m_filepath)
      free(m_filepath);
   if (m_alias)
      free(m_alias);
   m_uristring = NULL;
   m_filepath = NULL;
   m_alias = NULL;
   m_threadId = GLFWTHREAD_UNDEF;
   m_finished = false;
}

/* Downloader::constructor */
Downloader::Downloader(MMDAgent *mmdagent, int id, const char *uri, const char *path, const char *alias)
{
   m_mmdagent = mmdagent;
   m_id = id;
   m_uristring = MMDAgent_strdup(uri);
   m_filepath = MMDAgent_strdup(path);
   m_alias = MMDAgent_strdup(alias);
   m_threadId = GLFWTHREAD_UNDEF;
   m_finished = false;
   m_session = NULL;
   m_kill = false;
}

/* Downloader::destructor */
Downloader::~Downloader()
{
   m_kill = true;
   if (m_threadId != GLFWTHREAD_UNDEF) {
      Poco::Net::HTTPSClientSession *session = (Poco::Net::HTTPSClientSession *)m_session;
      if (session) {
         m_session = NULL;
         try {
            session->abort();
         } catch (Poco::Exception&) {
            /* do nothing */
         }
      }
      glfwWaitThread(m_threadId, GLFW_WAIT);
   }
   clear();
}

/* Downloader::setId: set thread id */
void Downloader::setId(GLFWthread id)
{
   m_threadId = id;
}

/* Downloader::run: main thread function */
void Downloader::run()
{
   char *path;

   /* fetch Contents list from the url */
   path = MMDFiles_pathdup_from_application_to_system_locale(m_filepath);
   if (path == NULL) {
      m_mmdagent->sendLogString(m_id, MLOG_ERROR, "unable to convert saving path to utf8: %s", m_filepath);
      return;
   }



   Poco::Net::HTTPClientSession *pSession = NULL;
   Poco::Net::HTTPResponse res;
   Poco::URI uri(m_uristring);

   try {
      bool retry = false;
      bool authorize = false;
      int redirects = 0;
      std::string username;
      std::string password;

      do {
         if (!pSession) {
            if (uri.getScheme() != "http")
               pSession = new Poco::Net::HTTPSClientSession(uri.getHost(), uri.getPort());
            else
               pSession = new Poco::Net::HTTPClientSession(uri.getHost(), uri.getPort());
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
         m_session = (void *)pSession;
         pSession->sendRequest(req);
         std::istream& rs = pSession->receiveResponse(res);
         if (m_kill)
            return;
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
            m_session = NULL;
            delete pSession;
            pSession = NULL;
            m_mmdagent->sendMessage(m_id, PLUGIN_EVENT_GET, "%s", m_alias);
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
      m_mmdagent->sendLogString(m_id, MLOG_ERROR, "%s: failed to save %s", m_alias, m_uristring);
   }
   free(path);
   m_finished = true;
   return;
}

/* Downloader::isFinished: return TRUE when thread has been finished */
bool Downloader::isFinished()
{
   return m_finished;
}

/* Downloader::getAlias: get alias */
const char *Downloader::getAlias()
{
   return m_alias;
}
