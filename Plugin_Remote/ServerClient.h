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

// Server-client connection handling class

#if defined(_WIN32) && !defined(__CYGWIN32__)
#include <winsock2.h>
#include <ws2tcpip.h>
#define WINSOCK
#else
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#endif

#ifdef WINSOCK
typedef SOCKET socket_t;
#define SOCKET_INVALID INVALID_SOCKET
#else
typedef int socket_t;
#define SOCKET_INVALID -1
#endif

class ServerClient
{
private:
   bool m_socket_initialized;   // flag for socket initialization
   socket_t m_server_sd;              // server socket
   char *m_hostname;             // client hostname

   // initialize
   void initialize();

   // clear
   void clear();

public:
   enum kStatus { SOCKET_CONNECT, SOCKET_READABLE, SOCKET_WRITABLE, SOCKET_HASERROR };

   // constructor
   ServerClient();

   // destructor
   ~ServerClient();

   // reset sockets
   void reset();

   // server, ready as server
   bool readyAsServer(int portnum);

   // return true when server has been started
   bool isServerStarted();

   // server, accept from socket and return new connected socket, SOCKET_INVALID on error
   socket_t acceptFrom();

   // client, make connection and return new socket, SOCKET_INVALID on error
   socket_t makeConnection(const char *hostname, int port_num);

   // close socket, return 0 on success, -1 on error
   int closeSocket(socket_t sd);

   // shutdown socket
   void shutdown(socket_t sd);

   // get client host name
   char *getClientHostName();

   // wait data
   kStatus waitData(socket_t *sd, int num, socket_t *sd_ret);

   // send data, -1 on error
   int send(socket_t sd, const void *buf, size_t len);

   // receive data, -1 on error
   int recv(socket_t sd, void *buf, size_t maxlen);

};
