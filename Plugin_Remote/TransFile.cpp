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

#define POCO_NO_AUTOMATIC_LIBS
#ifdef _WIN32
/* use static library built with MMDAgent-EX */
#define POCO_STATIC
#endif
#include <iostream>
#include <Poco/Net/WebSocket.h>
#include <Poco/Net/HTTPClientSession.h>
#include <Poco/Net/HTTPSClientSession.h>
#include <Poco/Net/HTTPRequest.h>
#include <Poco/Net/HTTPResponse.h>
#include <Poco/Net/SSLManager.h>
#include <Poco/Net/Context.h>
#include <Poco/Net/SecureStreamSocket.h>
#include <Poco/Net/AcceptCertificateHandler.h>
#include <Poco/Net/Socket.h>
#include <Poco/URI.h>
#include <Poco/Net/NetException.h>
#include <Poco/Base64Encoder.h>
#include <Poco/RandomStream.h>
#include <Poco/StreamCopier.h>
#include <Poco/Path.h>
#include <Poco/FileStream.h>

#include "TransFile.h"

// socket buffer maximum length
#define SOCKET_MAXBUFLEN 8192

// maximum size of a file that can be transferred: 30MB
#define MAXFILESIZE 1024*1024*30

/* generate 16 bytes of random binary data and encode to Base64 */
static std::string makeRandomKey() {
   unsigned char buffer[16];
   Poco::RandomInputStream().read((char *)buffer, sizeof(buffer));

   std::stringstream ss;
   Poco::Base64Encoder encoder(ss);
   encoder.write((char *)buffer, sizeof(buffer));
   encoder.close();

   return ss.str();
}

/* TransFile::initialize: initialize */
void TransFile::initialize()
{
   m_mmdagent = NULL;
   m_id = 0;
   m_host = NULL;
   m_port = 0;
   m_dir = NULL;
   m_saveFilePath = NULL;
   m_threadId = -1;
   m_running = false;
}

/* TransFile::clear: clear */
void TransFile::clear()
{
   stop();
   if (m_threadId >= 0) {
      glfwWaitThread(m_threadId, GLFW_WAIT);
      glfwDestroyThread(m_threadId);
   }
   if (m_saveFilePath)
      free(m_saveFilePath);
   if (m_dir)
      free(m_dir);
   if (m_host)
      free(m_host);
   initialize();
}

/* constructor */
TransFile::TransFile()
{
   initialize();
}

/* destructor */
TransFile::~TransFile()
{
   clear();
}

/* TransFile::run */
void TransFile::run()
{
   Poco::Net::WebSocket *ws;

   /* connect to channel */
   try {
      Poco::Net::HTTPRequest request(Poco::Net::HTTPRequest::HTTP_GET, m_dir, Poco::Net::HTTPRequest::HTTP_1_1);
      request.set("Upgrade", "websocket");
      request.set("Connection", "Upgrade");
      request.set("Sec-WebSocket-Version", Poco::Net::WebSocket::WEBSOCKET_VERSION);
      request.set("Sec-WebSocket-Key", makeRandomKey());
      request.set("X-App-Type", "MMDAgent");
      Poco::Net::HTTPResponse response;
      if (m_port == 443) {
         Poco::Net::HTTPSClientSession session(m_host, m_port);
         ws = new Poco::Net::WebSocket(session, request, response);
      } else {
         Poco::Net::HTTPClientSession session(m_host, m_port);
         ws = new Poco::Net::WebSocket(session, request, response);
      }
      ws->setReceiveTimeout(Poco::Timespan(0, 10));
   }
   catch (const Poco::Net::ConnectionRefusedException& e) {
      // unable to find server or service unavailable
      m_mmdagent->sendLogString(m_id, MLOG_ERROR, "%s", e.what());
      m_mmdagent->sendMessage(m_id, "REMOTE_TRANSFILE_INCOMPLETE", "%s|%s", m_dir, m_saveFilePath);
      return;
   }
   catch (const Poco::Net::WebSocketException& e) {
      // connection refused by server, perhaps maximum connection limit
      m_mmdagent->sendLogString(m_id, MLOG_ERROR, "%s", e.what());
      m_mmdagent->sendMessage(m_id, "REMOTE_TRANSFILE_INCOMPLETE", "%s|%s", m_dir, m_saveFilePath);
      return;
   }
   catch (const std::exception& e) {
      // any other error
      m_mmdagent->sendLogString(m_id, MLOG_ERROR, "%s", e.what());
      m_mmdagent->sendMessage(m_id, "REMOTE_TRANSFILE_INCOMPLETE", "%s|%s", m_dir, m_saveFilePath);
      return;
   }

   if (m_running == false) {
      ws->close();
      m_mmdagent->sendMessage(m_id, "REMOTE_TRANSFILE_INCOMPLETE", "%s|%s", m_dir, m_saveFilePath);
      return;
   }

   /* set max payload size to avoid buffer overrun */
   ws->setMaxPayloadSize(SOCKET_MAXBUFLEN - 1);

   /* send message to tell prepared */
   m_mmdagent->sendMessage(m_id, "REMOTE_TRANSFILE_PREPARED", "%s|%s", m_dir, m_saveFilePath);

   /* receive file */
   try
   {
      char buffer[SOCKET_MAXBUFLEN];
      int flags = 0;
      int n;
      int file_length = 0;

      std::string filename = m_saveFilePath;
      Poco::FileOutputStream fos(filename, std::ios::binary);
      do {
         n = ws->receiveFrame(buffer, SOCKET_MAXBUFLEN - 1, flags);
         buffer[n] = '\0';
         if (MMDAgent_strheadmatch(buffer, "__peer_disconnected__"))
            break;
         file_length += n;
         if (file_length >= MAXFILESIZE) {
            m_mmdagent->sendLogString(m_id, MLOG_ERROR, "sent file too large (>%d)", MAXFILESIZE);
            m_running = false;
         }
         fos.write(buffer, n);
         if (m_running == false)
            break;
      } while (n > 0 || (flags & Poco::Net::WebSocket::FRAME_OP_BITMASK) != Poco::Net::WebSocket::FRAME_OP_CLOSE);
      fos.close();
   }
   catch (Poco::Net::WebSocketException& e)
   {
      const char *p = e.what();
      m_mmdagent->sendLogString(m_id, MLOG_ERROR, "%s", e.what());
      m_mmdagent->sendMessage(m_id, "REMOTE_TRANSFILE_INCOMPLETE", "%s|%s", m_dir, m_saveFilePath);
      return;
   }

   if (m_running == false) {
      ws->close();
      m_mmdagent->sendMessage(m_id, "REMOTE_TRANSFILE_INCOMPLETE", "%s|%s", m_dir, m_saveFilePath);
      return;
   }

   ws->close();

   /* send message to tell finished */
   m_mmdagent->sendMessage(m_id, "REMOTE_TRANSFILE_FINISHED", "%s|%s", m_dir, m_saveFilePath);


   m_running = false;
}

/* threadMain: main thread */
static void threadMain(void *param)
{
   TransFile *tf = (TransFile *)param;
   tf->run();
}

/* TransFile::start: start */
bool TransFile::start(MMDAgent *mmdagent, int mid, const char *host, int port, const char *dir, const char *saveFilePath)
{
   clear();

   m_mmdagent = mmdagent;
   m_id = mid;
   m_host = MMDAgent_strdup(host);
   m_port = port;
   m_dir = MMDAgent_strdup(dir);
   m_saveFilePath = MMDAgent_strdup(saveFilePath);

   m_running = true;
   m_threadId = glfwCreateThread(threadMain, this);
   if (m_threadId == -1) {
      m_mmdagent->sendLogString(m_id, MLOG_ERROR, "failed to create thread for file transfer");
      m_running = false;
      return false;
   }

   return true;
}

/* TransFile::stop: stop */
void TransFile::stop()
{
   m_running = false;
}

/* TransFile::isRunning: return true when thread is running */
bool TransFile::isRunning()
{
   return m_running;
}
