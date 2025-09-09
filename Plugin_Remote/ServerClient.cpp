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

// A small container to hold hostent and its dynamically allocated members
struct hostent_box {
   struct hostent h;
   char **addr_list;        // array of pointers for h.h_addr_list
   struct in_addr *addrs;   // contiguous array of IPv4 addresses
   char *name_copy;         // copy of the hostname
};

/*
 * Thread-safe replacement for gethostbyname (IPv4 only).
 * On success, returns 0 and stores a pointer to a heap-allocated hostent in *out.
 * The returned object must be freed with my_freehostent().
 */
static int my_gethostbyname_threadsafe(const char *name, struct hostent **out)
{
   if (!name || !out) return -1;
   *out = NULL;

   struct addrinfo hints = { 0 }, *res = NULL, *rp = NULL;
   hints.ai_family = AF_INET;     // IPv4 only
   hints.ai_socktype = SOCK_STREAM; // filter by stream sockets

   int rc = getaddrinfo(name, NULL, &hints, &res);
   if (rc != 0 || !res) return -2;

   // Count the number of IPv4 addresses
   size_t naddr = 0;
   for (rp = res; rp; rp = rp->ai_next)
      if (rp->ai_family == AF_INET) ++naddr;
   if (naddr == 0) { freeaddrinfo(res); return -3; }

   // Allocate the container
   struct hostent_box *box = (struct hostent_box *)malloc(sizeof(*box));
   if (!box) { freeaddrinfo(res); return -4; }
   memset(box, 0, sizeof(*box));

   box->addrs = (struct in_addr *)malloc(naddr * sizeof(struct in_addr));
   box->addr_list = (char **)malloc((naddr + 1) * sizeof(char *));
   box->name_copy = MMDAgent_strdup(name);

   if (!box->addrs || !box->addr_list || !box->name_copy) {
      free(box->name_copy);
      free(box->addr_list);
      free(box->addrs);
      free(box);
      freeaddrinfo(res);
      return -5;
   }

   // Copy IPv4 addresses into the array
   size_t i = 0;
   for (rp = res; rp; rp = rp->ai_next) {
      if (rp->ai_family != AF_INET) continue;
      struct sockaddr_in *sin = (struct sockaddr_in *)rp->ai_addr;
      box->addrs[i] = sin->sin_addr;           // copy value
      box->addr_list[i] = (char *)&box->addrs[i]; // h_addr_list points here
      ++i;
   }
   box->addr_list[i] = NULL; // terminate with NULL

   // Fill hostent fields to mimic gethostbyname()
   box->h.h_name = box->name_copy;
   box->h.h_aliases = NULL;
   box->h.h_addrtype = AF_INET;
   box->h.h_length = (int)sizeof(struct in_addr);
   box->h.h_addr_list = box->addr_list;

   freeaddrinfo(res);

   *out = &box->h; // return as hostent*
   return 0;
}

/*
 * Frees the hostent returned by my_gethostbyname_threadsafe().
 * Safe to call with NULL.
 */
static void my_freehostent(struct hostent *h)
{
   if (!h) return;
   // Convert back from hostent* to hostent_box*
   struct hostent_box *box = (struct hostent_box *)((char *)h - offsetof(struct hostent_box, h));
   free(box->name_copy);
   free(box->addr_list);
   free(box->addrs);
   free(box);
}

void ServerClient::initialize()
{
   m_socket_initialized = false;
   m_server_sd = SOCKET_INVALID;
   m_hostname = NULL;
}

void ServerClient::reset()
{
   if (m_server_sd != SOCKET_INVALID) {
      closeSocket(m_server_sd);
   }
   if (m_hostname != NULL)
      free(m_hostname);
   m_server_sd = SOCKET_INVALID;
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
      if (WSAStartup(MAKEWORD(2, 0), &data) != 0) {
         reset();
         return false;
      }
      m_socket_initialized = true;
   }
#endif
  /* create socket */
#ifdef WINSOCK
   if((m_server_sd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP) == INVALID_SOCKET)) {
      m_server_sd = SOCKET_INVALID;
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
   if (m_server_sd == SOCKET_INVALID)
      return false;
   return true;
}

// server, accept from socket and return new connected socket, SOCKET_INVALID on error
socket_t ServerClient::acceptFrom()
{
   static struct sockaddr_in from;
#ifdef HAVE_SOCKLEN_T
   static socklen_t nbyte;
#else
   static int nbyte;
#endif // HAVE_SOCKLEN_T
   socket_t asd;

   if (m_server_sd == SOCKET_INVALID)
      return SOCKET_INVALID;

   nbyte = sizeof(struct sockaddr_in);
   asd = accept(m_server_sd, (struct sockaddr *)&from, &nbyte);
   if (asd == SOCKET_INVALID) {               /* error */
      return SOCKET_INVALID;
   }

   char buf[INET_ADDRSTRLEN];
   if (m_hostname != NULL)
      free(m_hostname);
   if (inet_ntop(AF_INET, &from.sin_addr, buf, sizeof(buf))) {
      m_hostname = MMDAgent_strdup(buf);
   } else {
      m_hostname = NULL;
   }

   return asd;
}

// client, make connection and return new socket, SOCKET_INVALID on error
socket_t ServerClient::makeConnection(const char *hostname, int port_num)
{
   struct hostent *hp;
   struct sockaddr_in sin;
   socket_t sd;

#ifdef WINSOCK
   /* init winsock */
   if (m_socket_initialized == false) {
      WSADATA data;
      if (WSAStartup(MAKEWORD(2, 0), &data) != 0) {
         reset();
         return SOCKET_INVALID;
      }
      m_socket_initialized = true;
   }
#endif

   /* host existence check */
   if (my_gethostbyname_threadsafe(hostname, &hp) != 0) {
      return SOCKET_INVALID;
   }

   /* create socket */
#ifdef WINSOCK
   if((sd = socket(PF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET)
      return SOCKET_INVALID;
#else  /* ~WINSOCK */
   if((sd = socket(PF_INET, SOCK_STREAM, 0)) < 0)
      return SOCKET_INVALID;
#endif /* ~WINSOCK */

   /* try to connect */
   memset((char *)&sin, 0, sizeof(sin));
   memcpy(&sin.sin_addr, hp->h_addr_list[0], hp->h_length);
   sin.sin_family = hp->h_addrtype;
   sin.sin_port = htons((unsigned short)port_num);
#ifdef WINSOCK
   if (connect(sd, (struct sockaddr *)&sin, sizeof(sin)) == SOCKET_ERROR) {
      /* fail */
      my_freehostent(hp);
      return SOCKET_INVALID;
   }
#else
   if (connect(sd, (struct sockaddr *)&sin, sizeof(sin)) == -1) {
      /* fail */
      my_freehostent(hp);
      return SOCKET_INVALID;
   }
#endif

   my_freehostent(hp);

   return sd;
}

// close socket, return 0 on success, -1 on error
int ServerClient::closeSocket(socket_t sd)
{
  int ret;
#ifdef WINSOCK
  ret = closesocket(sd);
  if (ret != 0)
     ret = -1;
#else
  ret = close(sd);
#endif
  return(ret);
}

char *ServerClient::getClientHostName()
{
   return m_hostname;
}

ServerClient::kStatus ServerClient::waitData(socket_t *sd, int num, socket_t *sd_ret)
{
   fd_set rfds, wfds;
   int ret;
   int i;
   socket_t sdmax;

   FD_ZERO(&rfds);
   FD_ZERO(&wfds);
   if (m_server_sd != SOCKET_INVALID)
      FD_SET(m_server_sd, &rfds);
   sdmax = m_server_sd;
   for (i = 0; i < num; i++) {
      FD_SET(sd[i], &rfds);
      FD_SET(sd[i], &wfds);
      if (sdmax < sd[i])
         sdmax = sd[i];
   }
   ret = select(sdmax + 1, &rfds, &wfds, NULL, NULL);
#ifdef WINSOCK
   if (ret == SOCKET_ERROR)
      return ServerClient::SOCKET_HASERROR;
#else
   if (ret <= 0)
      return ServerClient::SOCKET_HASERROR;
#endif
   if (m_server_sd >= 0 && FD_ISSET(m_server_sd, &rfds))
      return ServerClient::SOCKET_CONNECT;
   for (i = 0; i < num; i++) {
      if (FD_ISSET(sd[i], &rfds)) {
         *sd_ret = sd[i];
         return ServerClient::SOCKET_READABLE;
      }
   }
   return ServerClient::SOCKET_WRITABLE;
}

int ServerClient::send(socket_t sd, const void *buf, size_t len)
{
   int ret;
#ifdef WINSOCK
   ret = ::send(sd, (const char *)buf, (int)len, 0);
   if (ret == SOCKET_ERROR)
      ret = -1;
#else
   ret = (int)::send(sd, buf, len, 0);
#endif
   return ret;
}

int ServerClient::recv(socket_t sd, void *buf, size_t maxlen)
{
   int ret;
#ifdef WINSOCK
   ret = ::recv(sd, (char *)buf, (int)maxlen, 0);
   if (ret == SOCKET_ERROR)
      ret = -1;
#else
   ret = (int)::recv(sd, buf, maxlen, 0);
#endif
   return ret;
}

// shutdown socket
void ServerClient::shutdown(socket_t sd)
{
#ifdef WINSOCK
   ::shutdown(sd, SD_BOTH);
#else
   ::shutdown(sd, SHUT_RDWR);
#endif
}

