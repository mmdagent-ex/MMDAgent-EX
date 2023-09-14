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
#include "Kafka.h"
#include "Thread.h"

/* definitions */

#ifdef _WIN32
#define EXPORT extern "C" __declspec(dllexport)
#else
#ifdef MMDAGENT_PLUGIN_STATIC
#define EXPORT
#define extAppStart Plugin_Kafka_extAppStart
#define extProcMessage Plugin_Kafka_extProcMessage
#define extUpdate Plugin_Kafka_extUpdate
#define extRender2D Plugin_Kafka_extRender2D
#define extLog Plugin_Kafka_extLog
#define extAppEnd Plugin_Kafka_extAppEnd
#else
#define EXPORT extern "C"
#endif
#endif /* _WIN32 */

// plugin name
#define PLUGIN_NAME "Kafka"

// events
#define PLUGIN_EVENT_CONNECTED "KAFKA_EVENT_CONNECTED"
#define PLUGIN_EVENT_DISCONNECTED "KAFKA_EVENT_DISCONNECTED"

/* static function for thread main */
static void mainThread(void *param);

/* KafkaPlugin class */
class KafkaPlugin {

private:

   MMDAgent *m_mmdagent;      // MMDAgent class
   int m_id;                  // Module ID for messaging/logging
   Kafka *m_kafka;            // network connection handling function class
   Thread *m_thread;          // server thread
   bool m_active;             // true while the main thread is active
   bool m_kill;

   // initialize
   void initialize()
   {
      m_mmdagent = NULL;
      m_id = 0;
      m_kafka = NULL;
      m_thread = NULL;
      m_active = false;
   }

   // clear
   void clear()
   {
      m_active = false;
      if (m_thread != NULL)
         delete m_thread;

      initialize();
   }

public:

   KafkaPlugin()
   {
      initialize();
   }

   ~KafkaPlugin()
   {
      clear();
   }

   // setup
   void setup(MMDAgent *mmdagent, int id)
   {
      clear();

      m_mmdagent = mmdagent;
      m_id = id;
   }

   // start producer thread
   void startProducer(const char *broker, const char *codec, const char *topic, const char *key, int partition)
   {
      if (m_kafka)
         delete m_kafka;
      m_kafka = new Kafka(broker, codec, topic, key, partition);
      if (m_thread == NULL) {
         m_thread = new Thread;
         m_thread->setup();
      }
      if (m_thread->isRunning() == false)
         m_thread->addThread(glfwCreateThread(mainThread, this));
   }

   // start consumer thread
   void startConsumer(const char *broker, const char *codec, const char *topic, int partition)
   {
      if (m_kafka)
         delete m_kafka;
      m_kafka = new Kafka(broker, codec, topic, partition);
      if (m_thread == NULL) {
         m_thread = new Thread;
         m_thread->setup();
      }
      if (m_thread->isRunning() == false)
         m_thread->addThread(glfwCreateThread(mainThread, this));
   }

   // stop main thread and disconnect client if any
   void stop()
   {
      if (is_active() == false)
         return;

      m_thread->lock();
      m_active = false;
      m_thread->unlock();
      m_thread->stop();
   }

   // return true when main thread is active
   bool is_active()
   {
      if (m_active && m_thread && m_thread->isRunning())
         return true;
      return false;
   }

   // get messages queued by the other end
   int dequeueMessage(char *type, char *args)
   {
      if (is_active())
         return m_thread->dequeueBuffer(0, type, args);

      return 0;
   }

   // enqueue log strings to be sent to the other end
   void enqueueLogString(const char *str)
   {
      if (is_active() && m_kafka->isConnected())
         m_thread->enqueueBuffer(1, str, NULL);
   }

   // main thread
   void run()
   {
      char buff[MMDAGENT_MAXBUFLEN];
      int len;
      char *psave, *p, *q;

      m_thread->lock();

      if (m_kafka->connect() == false)
         return;

      if (m_kafka->getMode() == 'P')
         m_mmdagent->sendLogString(m_id, MLOG_STATUS, "connected as producer, sending logs");
      else
         m_mmdagent->sendLogString(m_id, MLOG_STATUS, "connected as consumer, receiving messages");

      // set running flag
      m_active = true;

      m_thread->unlock();

      // socket processing loop
      while (is_active()) {

         if (m_kafka->getMode() == 'P') {
            m_thread->waitQueue();
            // dequeue the log strings and send them to the server
            while (m_thread->dequeueBuffer(1, buff, NULL) > 0) {
               if (m_kafka->send(buff, MMDAgent_strlen(buff)) < 0) {
                  // client error, close it
                  m_kafka->disconnect();
                  break;
               }
            }
         } else {
            // receive messages from server and enqueue it to mmdagent
            // received message from server, enqueue it
            if ((len = m_kafka->receive(buff, MMDAGENT_MAXBUFLEN - 1)) < 0)
               break;
            if (len > 0) {
               buff[len] = '\0';
               p = MMDAgent_strtok(buff, "|", &psave);
               q = MMDAgent_strtok(NULL, "\r\n", &psave);
               m_thread->enqueueBuffer(0, p, q);
            }
         }
      }
      m_thread->lock();
      delete m_kafka;
      m_kafka = NULL;
      m_thread->unlock();
      m_active = false;
      return;
   }
};

/* main thread function, just call RemotePlugin::run() */
static void mainThread(void *param)
{
   KafkaPlugin *r = (KafkaPlugin *) param;
   r->run();
}

/////////////////////////////////////////////////////////////////////////////////

/* variables */
static int mid;
static KafkaPlugin consumer;
static KafkaPlugin producer;
static bool enabled = false;
static bool status = false;
static bool activated = false;

static void startPluginWithKeyValue(MMDAgent *mmdagent)
{
   char buff[MMDAGENT_MAXBUFLEN];
   KeyValue *v;
   const char *broker, *topic, *codec;
   int partition;

   activated = false;
   v = new KeyValue;
   v->setup();
   MMDAgent_snprintf(buff, MMDAGENT_MAXBUFLEN, "%s%c%s", mmdagent->getConfigDirName(), MMDAGENT_DIRSEPARATOR, MMDAGENT_CONTENTINFOFILE);
   if (v->load(buff, NULL) == false) {
      delete v;
      return;
   }
   if (v->exist("KafkaBroker") && v->exist("KafkaProducerTopic")) {
      broker = v->getString("KafkaBroker", "");
      codec = v->getString("KafkaCodec", NULL);
      partition = (int)atoi(v->getString("KafkaPartition", "-1"));
      topic = v->getString("KafkaProducerTopic", "");
      producer.startProducer(broker, codec, topic, MMDAgent_getUUID(), partition);
      activated = true;
   }
   if (v->exist("KafkaBroker") && v->exist("KafkaConsumerTopic")) {
      broker = v->getString("KafkaBroker", "");
      codec = v->getString("KafkaCodec", NULL);
      partition = (int)atoi(v->getString("KafkaPartition", "-1"));
      topic = v->getString("KafkaConsumerTopic", "");
      consumer.startConsumer(broker, codec, topic, partition);
      activated = true;
   }
   delete v;
}

/* extAppStart: initialize controller */
EXPORT void extAppStart(MMDAgent *mmdagent)
{
   glewInit();
   mid = mmdagent->getModuleId(PLUGIN_NAME);
   producer.setup(mmdagent, mid);
   consumer.setup(mmdagent, mid);
   startPluginWithKeyValue(mmdagent);
   enabled = true;
}

/* extProcMessage: process message */
EXPORT void extProcMessage(MMDAgent *mmdagent, const char *type, const char *args)
{
   if(MMDAgent_strequal(type, MMDAGENT_COMMAND_PLUGINENABLE) && MMDAgent_strequal(args, PLUGIN_NAME)) {
      mmdagent->sendLogString(mid, MLOG_MESSAGE_CAPTURED, "%s|%s", type, args);
      if (enabled == false) {
         /* start plugin if not started yet */
         enabled = true;
         startPluginWithKeyValue(mmdagent);
      } else if (status == true )
         mmdagent->sendMessage(mid, MMDAGENT_EVENT_PLUGINENABLE, "%s", PLUGIN_NAME);
   } else if(MMDAgent_strequal(type, MMDAGENT_COMMAND_PLUGINDISABLE) && MMDAgent_strequal(args, PLUGIN_NAME)) {
      mmdagent->sendLogString(mid, MLOG_MESSAGE_CAPTURED, "%s|%s", type, args);
      if (enabled == true) {
         /* end plugin if not endeded yet */
         enabled = false;
         producer.stop();
         consumer.stop();
      } else if (status == false)
         mmdagent->sendMessage(mid, MMDAGENT_EVENT_PLUGINDISABLE, "%s", PLUGIN_NAME);
   }
}

/* extUpdate: update */
EXPORT void extUpdate(MMDAgent *mmdagent, double deltaFrame)
{
   if (enabled == true && (producer.is_active() == true || consumer.is_active() == true)) {
      if (status == false) {
         mmdagent->sendMessage(mid, MMDAGENT_EVENT_PLUGINENABLE, "%s", PLUGIN_NAME);
         status = true;
      }
   } else if (enabled == false && (producer.is_active() == false && consumer.is_active() == false)) {
      if (status == true) {
         mmdagent->sendMessage(mid, MMDAGENT_EVENT_PLUGINDISABLE, "%s", PLUGIN_NAME);
         status = false;
      }
   }
   // if queue has received message, send them to MMDAgent
   char type[MMDAGENT_MAXBUFLEN];
   char args[MMDAGENT_MAXBUFLEN];
   if (status == true && activated == true) {
      while (consumer.dequeueMessage(type, args) > 0) {
         mmdagent->sendMessage(mid, type, "%s", args);
      }
   }
}

/* extRender2D: render in 2D screen */
EXPORT void extRender2D(MMDAgent *mmdagent, float screenWidth, float screenHeight)
{
   if (enabled == true && activated == true) {
      glColor4f(0.0f, 1.0f, 0.4f, 1.0f);
      MMDFiles_drawedge(0.0f, 0.0f, screenWidth, screenHeight, 0.2f);
   }
}

/* extLog: process log string */
EXPORT void extLog(MMDAgent *mmdagent, int id, unsigned int flag, const char *text, const char *fulltext)
{
   if (enabled == true && activated == true) {
      producer.enqueueLogString(fulltext);
   }
}

/* extAppEnd: end of application */
EXPORT void extAppEnd(MMDAgent *mmdagent)
{
   enabled = false;
   producer.stop();
   consumer.stop();
}
