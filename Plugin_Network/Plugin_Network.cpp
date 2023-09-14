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

/*
NETWORK_GET|alias|"URI"|"localfilepath"
NETWORK_EVENT_GET|alias|status_code
*/

/* headers */
#include "MMDAgent.h"
#include "Downloader.h"

/* definitions */

#ifdef _WIN32
#define EXPORT extern "C" __declspec(dllexport)
#else
#ifdef MMDAGENT_PLUGIN_STATIC
#define EXPORT
#define extAppInit Plugin_Network_extAppInit
#define extAppStart Plugin_Network_extAppStart
#define extProcMessage Plugin_Network_extProcMessage
#define extRender Plugin_Network_extRender
#define extAppEnd Plugin_Network_extAppEnd
#else
#define EXPORT extern "C"
#endif
#endif /* _WIN32 */

#define PLUGIN_NAME "Network"     /* plugin name */
#define MAXNUMTHREAD 10           /* maximum number of download threads */

/* thread function, just call Downloader::run() */
static void downloaderThreadMain(void *param)
{
   Downloader *d = (Downloader *)param;
   d->run();
}

/* NetworkPlugin class */
class NetworkPlugin {

private:

   MMDAgent *m_mmdagent;                /* MMDAgent class */
   int m_id;                            /* Module ID */
   Downloader *m_loader[MAXNUMTHREAD];  /* downloaders */

   /* initialize: initialize data */
   void initialize(MMDAgent *mmdagent, int id)
   {
      int i;

      m_mmdagent = mmdagent;
      m_id = id;
      for (i = 0; i < MAXNUMTHREAD; i++)
         m_loader[i] = NULL;
   }

   /* clear: clear data */
   void clear()
   {
      int i;
      for (i = 0; i < MAXNUMTHREAD; i++) {
         if (m_loader[i] != NULL)
            delete m_loader[i];
         m_loader[i] = NULL;
      }
   }

public:

   /* constructor */
   NetworkPlugin(MMDAgent *mmdagent, int id)
   {
      initialize(mmdagent, id);
   }

   /* destructor */
   ~NetworkPlugin()
   {
      clear();
   }

   /* addDownloader: add a new file downloader thread */
   bool addDownloader(const char *uri, const char *filepath, const char *alias)
   {
      int i;
      GLFWthread id;

      for (i = 0; i < MAXNUMTHREAD; i++) {
         if (m_loader[i] != NULL && m_loader[i]->isFinished()) {
            delete m_loader[i];
            m_loader[i] = NULL;
         }
      }
      for (i = 0; i < MAXNUMTHREAD; i++) {
         if (m_loader[i] == NULL)
            break;
      }
      if (i >= MAXNUMTHREAD) {
         m_mmdagent->sendLogString(m_id, MLOG_ERROR, "%s: number of threads exceed maximum (%d)", alias, MAXNUMTHREAD);
         return false;
      }

      Downloader *d = new Downloader(m_mmdagent, m_id, uri, filepath, alias);
      id = glfwCreateThread(downloaderThreadMain, d);
      if (id == -1) {
         delete d;
         m_mmdagent->sendLogString(m_id, MLOG_ERROR, "%s: failed to create thread", alias);
         return false;
      }
      d->setId(id);

      m_loader[i] = d;

      return true;
   }
};

/* variables */
static NetworkPlugin *plugin = NULL;
int mid;
static bool enabled = false;

/* extAppStart: start */
EXPORT void extAppStart(MMDAgent *mmdagent)
{
   mid = mmdagent->getModuleId(PLUGIN_NAME);
   glfwInit();
#ifdef POCO_STATIC
   /* static linking requires initialization at each shared instances */
   MMDAgent_enablepoco();
#endif
   plugin = new NetworkPlugin(mmdagent, mid);
   enabled = true;
}

/* extProcMessage: process message */
EXPORT void extProcMessage(MMDAgent *mmdagent, const char *type, const char *args)
{
   char buf[MMDAGENT_MAXBUFLEN];
   char *s1, *s2, *s3, *save;

   if(MMDAgent_strequal(type, MMDAGENT_COMMAND_PLUGINENABLE) && MMDAgent_strequal(args, PLUGIN_NAME)) {
      mmdagent->sendLogString(mid, MLOG_MESSAGE_CAPTURED, "%s|%s", type, args);
      if (enabled == false) {
         /* start plugin if not started yet */
         plugin = new NetworkPlugin(mmdagent, mid);
         enabled = true;
      }
      mmdagent->sendMessage(mid, MMDAGENT_EVENT_PLUGINENABLE, "%s", PLUGIN_NAME);
   } else if(MMDAgent_strequal(type, MMDAGENT_COMMAND_PLUGINDISABLE) && MMDAgent_strequal(args, PLUGIN_NAME)) {
      mmdagent->sendLogString(mid, MLOG_MESSAGE_CAPTURED, "%s|%s", type, args);
      if (enabled == true) {
         /* end plugin if not endeded yet */
         delete plugin;
         enabled = false;
      }
      mmdagent->sendMessage(mid, MMDAGENT_EVENT_PLUGINDISABLE, "%s", PLUGIN_NAME);
   } else if (MMDAgent_strequal(type, PLUGIN_COMMAND_GET)) {
      mmdagent->sendLogString(mid, MLOG_MESSAGE_CAPTURED, "%s|%s", type, args);
      /* divide string into arguments */
      if (MMDAgent_strlen(args) == 0) {
         mmdagent->sendLogString(mid, MLOG_ERROR, "no argument");
      } else {
         strncpy(buf, args, MMDAGENT_MAXBUFLEN - 1);
         buf[MMDAGENT_MAXBUFLEN - 1] = '\0';
         if ((s1 = MMDAgent_strtok(buf, "|", &save)) == NULL)
            mmdagent->sendLogString(mid, MLOG_ERROR, "no argument");
         else if ((s2 = MMDAgent_strtok(NULL, "|", &save)) == NULL)
            mmdagent->sendLogString(mid, MLOG_ERROR, "too few argument");
         else if ((s3 = MMDAgent_strtok(NULL, "|", &save)) == NULL)
            mmdagent->sendLogString(mid, MLOG_ERROR, "too few argument");
         else
            plugin->addDownloader(s2, s3, s1);
      }
   }
}

/* extAppEnd: end of application */
EXPORT void extAppEnd(MMDAgent *mmdagent)
{
   enabled = false;
   delete plugin;
}
