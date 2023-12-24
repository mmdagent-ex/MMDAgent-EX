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
#include "ServerClient.h"

#if defined(_WIN32) && !defined(__CYGWIN32__)
#include <winsock2.h>
#define WINSOCK
#else
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#endif

void ServerClient::initialize()
{
   m_socket_initialized = false;
   m_server_sd = -1;
   m_hostname = NULL;
}

void ServerClient::reset()
{
   if (m_server_sd != -1) {
      closeSocket(m_server_sd);
   }
   if (m_hostname != NULL)
      free(m_hostname);
   m_server_sd = -1;
   m_hostname = NULL;
}

void ServerClient::clear()
{
#ifdef WINSOCK
   if (m_socket_initialized == true)
      WSACleanup();
#endif
   reset();
   initialize();
}

ServerClient::ServerClient()
{
   initialize();
}

ServerClient::~ServerClient()
{
   clear();
}

bool ServerClient::readyAsServer(int portnum)
{
   struct sockaddr_in sin;
   int optval;
   int optlen;

   reset();

#ifdef WINSOCK
   /* init winsock */
   if (m_socket_initialized == false) {
      WSADATA data;
      WSAStartup(MAKEWORD(2,0), &data);
      m_socket_initialized = true;
   }
#endif
  /* create socket */
#ifdef WINSOCK
   if((m_server_sd = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, 0)) == INVALID_SOCKET) {
      reset();
      return false;
   }
#else  /* ~WINSOCK */
   if((m_server_sd = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
      reset();
      return false;
   }
#endif /* ~WINSOCK */

   /* set socket to allow reuse of local address at bind() */
   /* this option prevent from "error: Address already in use" */
   optval = 1;
   optlen = sizeof(int);
   if (setsockopt(m_server_sd, SOL_SOCKET, SO_REUSEADDR, (char *)&optval, optlen) != 0) {
      reset();
      return false;
   }

   /* assign name(address) to socket */
   memset((char *)&sin, 0, sizeof(sin));
   sin.sin_family = AF_INET;
   sin.sin_port = htons((unsigned short)portnum);
   if (bind(m_server_sd, (struct sockaddr *)&sin, sizeof(sin)) < 0) {
      int errcode = WSAGetLastError();
      reset();
      return false;
   }
   /* begin to listen */
   if (listen(m_server_sd, 5) < 0) {
      reset();
      return false;
   }

   return true;
}

// ServerClient::return true when server has been started
bool ServerClient::isServerStarted()
{
   if (m_server_sd == -1)
      return false;
   return true;
}

int ServerClient::acceptFrom()
{
   static struct sockaddr_in from;
#ifdef HAVE_SOCKLEN_T
   static socklen_t nbyte;
#else
   static int nbyte;
#endif // HAVE_SOCKLEN_T
   int asd;

   if (m_server_sd == -1)
      return -1;

   nbyte = sizeof(struct sockaddr_in);
   asd = accept(m_server_sd, (struct sockaddr *)&from, &nbyte);
   if (asd < 0) {               /* error */
      return -1;
   }

   if (m_hostname != NULL)
      free(m_hostname);
   m_hostname = MMDAgent_strdup(inet_ntoa(from.sin_addr));

   return asd;
}

int ServerClient::makeConnection(const char *hostname, int port_num)
{
   static struct hostent *hp;
   static struct sockaddr_in	sin;
   int sd;

#ifdef WINSOCK
   /* init winsock */
   if (m_socket_initialized == false) {
      WSADATA data;
      WSAStartup(0x1010, &data);
      m_socket_initialized = true;
   }
#endif

   /* host existence check */
   if ((hp  = gethostbyname(hostname)) == NULL)
      return -1;
   /* create socket */
#ifdef WINSOCK
   if((sd = socket(PF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET)
      return -1;
#else  /* ~WINSOCK */
   if((sd = socket(PF_INET, SOCK_STREAM, 0)) < 0)
      return -1;
#endif /* ~WINSOCK */

   /* try to connect */
   memset((char *)&sin, 0, sizeof(sin));
   memcpy(&sin.sin_addr, hp->h_addr, hp->h_length);
   sin.sin_family = hp->h_addrtype;
   sin.sin_port = htons((unsigned short)port_num);
   if (connect(sd, (struct sockaddr *)&sin, sizeof(sin)) < 0) {
      /* fail */
      return -1;
   }

   return sd;
}

int ServerClient::closeSocket(int sd)
{
  int ret;
#ifdef WINSOCK
  ret = closesocket(sd);
#else
  ret = close(sd);
#endif
  return(ret);
}

char *ServerClient::getClientHostName()
{
   return m_hostname;
}

ServerClient::kStatus ServerClient::waitData(int *sd, int num, int *sd_ret)
{
   fd_set rfds, wfds;
   int ret;
   int i;
   int sdmax;

   FD_ZERO(&rfds);
   FD_ZERO(&wfds);
   if (m_server_sd >= 0)
      FD_SET(m_server_sd, &rfds);
   sdmax = m_server_sd;
   for (i = 0; i < num; i++) {
      FD_SET(sd[i], &rfds);
      FD_SET(sd[i], &wfds);
      if (sdmax < sd[i])
         sdmax = sd[i];
   }
   ret = select(sdmax + 1, &rfds, &wfds, NULL, NULL);
   if (ret <= 0)
      return ServerClient::SOCKET_HASERROR;
   if (FD_ISSET(m_server_sd, &rfds))
      return ServerClient::SOCKET_CONNECT;
   for (i = 0; i < num; i++) {
      if (FD_ISSET(sd[i], &rfds)) {
         *sd_ret = sd[i];
         return ServerClient::SOCKET_READABLE;
      }
   }
   return ServerClient::SOCKET_WRITABLE;
}

int ServerClient::send(int sd, const void *buf, size_t len)
{
   int ret;
#ifdef WINSOCK
   ret = ::send(sd, (const char *)buf, len, 0);
#else
   ret = (int)::send(sd, buf, len, 0);
#endif
   return ret;
}

int ServerClient::recv(int sd, void *buf, size_t maxlen)
{
   int ret;
#ifdef WINSOCK
   ret = ::recv(sd, (char *)buf, maxlen, 0);
#else
   ret = (int)::recv(sd, buf, maxlen, 0);
#endif
   return ret;
}

void ServerClient::shutdown(int sd)
{
#ifdef WINSOCK
   ::shutdown(sd, SD_BOTH);
#else
   ::shutdown(sd, SHUT_RDWR);
#endif
}

