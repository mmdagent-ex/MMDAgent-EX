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
class ServerClient
{
private:
   bool m_socket_initialized;   // flag for socket initialization
   int m_server_sd;              // server socket
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

   // server, accept from socket and return new connected socket, -1 on error
   int acceptFrom();

   // client, make connection and return new socket, -1 on error
   int makeConnection(const char *hostname, int port_num);

   // close socket
   int closeSocket(int sd);

   // shutdown socket
   void shutdown(int sd);

   // get client host name
   char *getClientHostName();

   // wait data
   kStatus waitData(int *sd, int num, int *sd_ret);

   // send data
   int send(int sd, const void *buf, size_t len);

   // receive data
   int recv(int sd, void *buf, size_t maxlen);

};
