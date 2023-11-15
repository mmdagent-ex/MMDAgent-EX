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

/* maximum number of history */
#define MMDAGENT_HTTPSERVER_MAX_HISTORY_NUM 30

/* HttpServer: simple http server */
class HttpServer
{
private:
   int m_portNum;             /* port number to listen */
   char *m_history[MMDAGENT_HTTPSERVER_MAX_HISTORY_NUM];/* history */
   int m_historyLen;
   int m_historyCurrent;
   void *m_server;            /* server instance */
   MMDAgent *m_mmdagent;      /* mmdagent instance */
   GLFWthread m_thread; // thread ID

   /* initialize: initialize data */
   void initialize();

   /* clear: free data */
   void clear();

   /* exec: execute the message */
   void exec(const char * message);

   /* getHtmlString: get new html page string */
   char *getHtmlString();

public:

   /* HttpServer: constructor */
   HttpServer(MMDAgent *mmdagent, int portnum);

   /* HttpServer: destructor */
   ~HttpServer();

   /* run: thread run */
   void run();

   /* start: start */
   void start();
};
