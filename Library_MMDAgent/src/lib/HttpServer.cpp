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

#define WIN32_LEAN_AND_MEAN
#include "MMDAgent.h"
#include "httplib.h"


/* html elements */
static const std::string html_text_head =
"<html><head>"
"<meta charset = \"UTF-8\"><meta http-equiv=\"Cache-Control\" content=\"no-store\">"
"<title>MMDAgent-EX internal message post page</title>"
"<div>"
"<style>textarea{font-size:24px;}.div1{background-color:#EBEBEB;width:100%;</style>"
"</head><body>"
"<h2>Message:</h2>"
"<form action=\"/post\" method=\"post\">"
"<textarea name=\"com\" style=\"width: 1024px; max-width: 100%;\" rows=\"3\">"
;

static const std::string html_text_tail =
"</textarea>"
"<p></p><button>Send</button>"
"</form>"
"</div>"
"<h3>History:</h3>"
"<div class=\"div1\">"
;

static const std::string html_history_head =
"<p>";

static const std::string html_history_tail =
"</p>";

static const std::string html_footer =
"</div></body></html>";


/* HttpServer::initialize: initialize data */
void HttpServer::initialize()
{
   m_portNum = 0;
   for (int i = 0; i < MMDAGENT_HTTPSERVER_MAX_HISTORY_NUM; i++)
      m_history[i] = NULL;
   m_historyLen = 0;
   m_historyCurrent = 0;
   m_last = NULL;
   m_server = NULL;
   m_mmdagent = NULL;
   m_thread = -1;
}

/* HttpServer::clear: free data */
void HttpServer::clear()
{
   httplib::Server *svr = (httplib::Server *)m_server;

   if (svr) {
      svr->wait_until_ready();
      svr->stop();
      delete svr;
   }

   for (int i = 0; i < m_historyLen; i++) {
      if (m_history[i])
         free(m_history[i]);
   }
   if (m_last)
      free(m_last);

   initialize();
}

/* HttpServer::HttpServer: constructor */
HttpServer::HttpServer(MMDAgent *mmdagent, int portnum)
{
   initialize();
   m_mmdagent = mmdagent;
   m_portNum = portnum;
}

/* HttpServer::HttpServer: destructor */
HttpServer::~HttpServer()
{
   clear();
}

/* HttpServer::exec: execute the message */
void HttpServer::exec(const char *message)
{
   char buff[MMDAGENT_MAXBUFLEN];
   char *p, *q, *psave;

   MMDAgent_snprintf(buff, MMDAGENT_MAXBUFLEN, "%s", message);
   p = MMDAgent_strtok(buff, "|\r\n", &psave);
   q = MMDAgent_strtok(NULL, "\r\n", &psave);
   m_mmdagent->sendMessage(0, p, q);

   if (m_last != NULL && MMDAgent_strequal(m_last, message) == true)
      return;
   if (m_last)
      free(m_last);
   m_last = MMDAgent_strdup(message);

   /* add to history */
   if (m_historyLen < MMDAGENT_HTTPSERVER_MAX_HISTORY_NUM) {
      m_history[m_historyLen] = MMDAgent_strdup(message);
      m_historyLen++;
   } else {
      if (m_history[m_historyCurrent])
         free(m_history[m_historyCurrent]);
      m_history[m_historyCurrent] = MMDAgent_strdup(message);
      m_historyCurrent++;
      if (m_historyCurrent >= MMDAGENT_HTTPSERVER_MAX_HISTORY_NUM)
         m_historyCurrent -= MMDAGENT_HTTPSERVER_MAX_HISTORY_NUM;
   }
}

/* HttpServer::getHtmlString: get new html page string */
char* HttpServer::getHtmlString(const char *text)
{
   std::string page = html_text_head + text + html_text_tail;

   if (m_historyLen < MMDAGENT_HTTPSERVER_MAX_HISTORY_NUM) {
      for (int i = m_historyLen - 1; i >= 0; i--) {
         page += html_history_head + m_history[i] + html_history_tail;
      }
   } else {
      for (int i = m_historyCurrent - 1; i >= 0; i--) {
         page += html_history_head + m_history[i] + html_history_tail;
      }
      for (int i = m_historyLen - 1; i >= m_historyCurrent; i++) {
         page += html_history_head + m_history[i] + html_history_tail;
      }
   }

   page += html_footer;

   char *retstr = MMDAgent_strdup(page.c_str());

   return retstr;
}

/* HttpServer::run: thread run */
void HttpServer::run()
{
   httplib::Server *svr = new httplib::Server();

   // access to "/": return form
   svr->Get("/", [this](const httplib::Request &, httplib::Response &res) {
      char *s = getHtmlString("");
      res.set_content(s, "text/html");
      free(s);
      });

   // accecss to "/req": get message as query "com=..."
   svr->Get("/req", [this](const httplib::Request &req, httplib::Response &res) {
      std::string val = "";
      if (req.has_param("com")) {
         val = req.get_param_value("com");
         exec(val.c_str());
      }
      char *s = getHtmlString(val.c_str());
      res.set_content(s, "text/html");
      free(s);
      });

   svr->Post("/post", [this](const httplib::Request &req, httplib::Response &res) {
      std::string val = "";
      if (req.has_param("com")) {
         val = req.get_param_value("com");
         exec(val.c_str());
      }
      char *s = getHtmlString(val.c_str());
      res.set_content(s, "text/html");
      free(s);
      });

   m_server = svr;

   m_mmdagent->sendLogString(0, MLOG_STATUS, "starting http server on port %d", m_portNum);

   svr->listen("0.0.0.0", m_portNum);
}

/* main thread function, just call HttpServer::run() */
static void mainThread(void *param)
{
   HttpServer *s = (HttpServer *)param;
   s->run();
}

/* HttpServer::start: start */
void HttpServer::start()
{
   m_thread = glfwCreateThread(mainThread, this);
}
