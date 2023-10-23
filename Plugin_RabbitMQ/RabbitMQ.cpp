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

#include "MMDAgent.h"
#include "Thread.h"
#include "AudioLipSync.h"
#include "RabbitMQ.h"

#define POCO_NO_AUTOMATIC_LIBS
#ifdef _WIN32
#define POCO_STATIC
#endif
#include <Poco/JSON/Parser.h>
#include <Poco/Dynamic/Var.h>
#include <Poco/Base64Decoder.h>

#include <sstream>
#include <iostream>
#include <fstream>
#include <vector>

#define ACTION_MOTION_MAXNUM 10

/* constructor */
RabbitMQMotionConfig::RabbitMQMotionConfig(std::string jsonstr)
{
   try {

      Poco::JSON::Parser parser;
      Poco::Dynamic::Var result = parser.parse(jsonstr);
      Poco::JSON::Object::Ptr object = result.extract<Poco::JSON::Object::Ptr>();
      std::string s;

      m_param = new KeyValue();
      m_param->setup();
      s = object->getValue<std::string>("target_model_alias");
      m_param->setString("target_model_alias", s.c_str());
      s = object->getValue<std::string>("motion_expression_alias");
      m_param->setString("motion_expression_alias", s.c_str());
      s = object->getValue<std::string>("motion_action_alias");
      m_param->setString("motion_action_alias", s.c_str());

      m_expression2motion = new KeyValue();
      m_expression2motion->setup();
      Poco::JSON::Array::Ptr expressionList = object->getArray("expression_list");
      for (int i = 0; i < expressionList->size(); i++) {
         Poco::JSON::Object::Ptr item = expressionList->getObject(i);
         std::string key = item->getValue<std::string>("name");
         std::string val = item->getValue<std::string>("motion");
         m_expression2motion->setString(key.c_str(), "%s", val.c_str());
      }
      m_action2motion = new KeyValue();
      m_action2motion->setup();
      Poco::JSON::Array::Ptr actionList = object->getArray("action_list");
      for (int i = 0; i < actionList->size(); i++) {
         Poco::JSON::Object::Ptr item = actionList->getObject(i);
         std::string key = item->getValue<std::string>("name");
         std::string val = item->getValue<std::string>("motion");
         m_action2motion->setString(key.c_str(), "%s", val.c_str());
      }
   }
   catch (const Poco::Exception &ex) {
      return;
   }
}

/* destructor */
RabbitMQMotionConfig::~RabbitMQMotionConfig()
{
   if (m_action2motion)
      delete m_action2motion;
   if (m_expression2motion)
      delete m_expression2motion;
   if (m_param)
      delete m_param;
}

/* RabbitMQMotionConfig::getParam: get parameter string */
const char *RabbitMQMotionConfig::getParam(const char *name)
{
   if (m_param == NULL)
      return NULL;
   return m_param->getString(name, NULL);
}

/* RabbitMQMotionConfig::getExpressionPath: get expression motion path */
const char *RabbitMQMotionConfig::getExpressionPath(const char *name)
{
   if (m_expression2motion == NULL)
      return NULL;
   return m_expression2motion->getString(name, NULL);

}

/* RabbitMQMotionConfig::getActionPath: get action motion path */
const char *RabbitMQMotionConfig::getActionPath(const char *name)
{
   if (m_action2motion == NULL)
      return NULL;
   return m_action2motion->getString(name, NULL);
}

/************************************************************************/

/* main thread function, just call RemotePlugin::run() */
static void mainThread(void *param)
{
   RabbitMQ *r = (RabbitMQ *)param;
   r->run();
}

/* RabbitMQ::on_amqp_error: check return code of amqp functions, and when error, issue error message and return true */
bool RabbitMQ::on_amqp_error(amqp_rpc_reply_t x, char const *context)
{
   switch (x.reply_type) {
   case AMQP_RESPONSE_NORMAL:
      return false;

   case AMQP_RESPONSE_NONE:
      m_mmdagent->sendLogString(m_id, MLOG_ERROR, "%s: %s: missing RPC reply type!", m_name, context);
      break;

   case AMQP_RESPONSE_LIBRARY_EXCEPTION:
      m_mmdagent->sendLogString(m_id, MLOG_ERROR, "%s: %s: %s", m_name, context, amqp_error_string2(x.library_error));
      break;

   case AMQP_RESPONSE_SERVER_EXCEPTION:
      switch (x.reply.id) {
      case AMQP_CONNECTION_CLOSE_METHOD: {
         amqp_connection_close_t *m = (amqp_connection_close_t *)x.reply.decoded;
         m_mmdagent->sendLogString(m_id, MLOG_ERROR, "%s: %s: server connection error %uh, message: %.*s", m_name, context, m->reply_code, (int)m->reply_text.len, (char *)m->reply_text.bytes);
         break;
      }
      case AMQP_CHANNEL_CLOSE_METHOD: {
         amqp_channel_close_t *m = (amqp_channel_close_t *)x.reply.decoded;
         m_mmdagent->sendLogString(m_id, MLOG_ERROR, "%s: %s: server channel error %uh, message: %.*s", m_name, context, m->reply_code, (int)m->reply_text.len, (char *)m->reply_text.bytes);
         break;
      }
      default:
         m_mmdagent->sendLogString(m_id, MLOG_ERROR, "%s: %s: unknown server error, method id 0x%08X", m_name, context, x.reply.id);
         break;
      }
      break;
   }
   m_active = false;
   return true;
}

/* constructor */
RabbitMQ::RabbitMQ(MMDAgent *mmdagent, int id, const char *name, int mode, const char *host, int port, const char *exchangename, const char *type, const char *queuename, AudioLipSync *sync, RabbitMQMotionConfig *motion_config)
{
   m_mmdagent = mmdagent;
   m_id = id;
   m_type = MMDAgent_strdup(type);
   m_name = MMDAgent_strdup(name);
   m_mode = mode;
   m_host = MMDAgent_strdup(host);
   m_port = port;
   m_exchangename = MMDAgent_strdup(exchangename);
   m_queuename = MMDAgent_strdup(queuename);
   m_active = false;
   m_sync = sync;
   m_motion_config = motion_config;
   m_thread = new Thread;
   m_thread->setup();
}

/* destructor */
RabbitMQ::~RabbitMQ()
{
   m_active = false;
   m_thread->terminate();
   delete m_thread;
   if (m_queuename) free(m_queuename);
   if (m_exchangename) free(m_exchangename);
   if (m_host) free(m_host);
   if (m_name) free(m_name);
}

/* RabbitMQ::getMode: get mode */
int RabbitMQ::getMode()
{
   return m_mode;
}

/* RabbitMQ::start: start processing thread */
void RabbitMQ::start()
{
   if (m_thread->isRunning() == false) {
      m_active = true;
      m_thread->addThread(glfwCreateThread(mainThread, this));
   }
}

/* RabbitMQ::run: thread main loop function */
void RabbitMQ::run()
{
   char buff[MMDAGENT_MAXBUFLEN];
   amqp_bytes_t queuename;

   /* make connection to rabbitmq server */
   amqp_connection_state_t conn = amqp_new_connection();
   amqp_socket_t *socket = amqp_tcp_socket_new(conn);
   if (socket == NULL) {
      m_mmdagent->sendLogString(m_id, MLOG_ERROR, "%s: failed to create TCP socket", m_name);
      m_active = false;
      return;
   }
   int status = amqp_socket_open(socket, m_host, m_port);
   if (status) {
      m_mmdagent->sendLogString(m_id, MLOG_ERROR, "%s: failed to connect to rabbitmq server %s:%d", m_name, m_host, m_port);
      m_active = false;
      return;
   }

   if (on_amqp_error(amqp_login(conn, "/", 0, 131072, 0, AMQP_SASL_METHOD_PLAIN, "guest", "guest"), "Logging in")) return;
   amqp_channel_open(conn, 1);
   if (on_amqp_error(amqp_get_rpc_reply(conn), "Opening channel")) return;

   if (m_mode == RABBITMQ_CONSUMER_MODE) {
      if (MMDAgent_strlen(m_exchangename) == 0) {
         // start listening to a queue as a consumer using default exchange
         amqp_queue_declare_ok_t *r = amqp_queue_declare(conn, 1, amqp_cstring_bytes(m_queuename), 0, 0, 0, 1, amqp_empty_table);
         if (on_amqp_error(amqp_get_rpc_reply(conn), "Declaring queue")) return;
         amqp_basic_consume(conn, 1, amqp_cstring_bytes(m_queuename), amqp_empty_bytes, 0, 0, 0, amqp_empty_table);
         if (on_amqp_error(amqp_get_rpc_reply(conn), "Consuming")) return;
         m_mmdagent->sendLogString(m_id, MLOG_STATUS, "%s: %s:%d: listen queue=%s", m_name, m_host, m_port, m_queuename);
      } else {
         // declare exchange on server
         amqp_exchange_declare(conn, 1, amqp_cstring_bytes(m_exchangename), amqp_cstring_bytes(m_type), 0, 0, 0, 0, amqp_empty_table);
         if (on_amqp_error(amqp_get_rpc_reply(conn), "Declaring exchange")) return;
         // declare a queue for given exchange and binding key and connect to it as a consumer
         amqp_queue_declare_ok_t *r = amqp_queue_declare(conn, 1, amqp_empty_bytes, 0, 0, 0, 1, amqp_empty_table);
         if (on_amqp_error(amqp_get_rpc_reply(conn), "Declaring queue")) return;
         queuename = amqp_bytes_malloc_dup(r->queue);
         if (queuename.bytes == NULL) {
            m_mmdagent->sendLogString(m_id, MLOG_ERROR, "out of memory while copying queue name");
            m_active = false;
            return;
         }
         amqp_queue_bind(conn, 1, queuename, amqp_cstring_bytes(m_exchangename),
            (MMDAgent_strlen(m_queuename) == 0) ? amqp_empty_bytes : amqp_cstring_bytes(m_queuename),
            amqp_empty_table);
         if (on_amqp_error(amqp_get_rpc_reply(conn), "Binding queue")) return;
         amqp_basic_consume(conn, 1, queuename, amqp_empty_bytes, 0, 1, 0, amqp_empty_table);
         if (on_amqp_error(amqp_get_rpc_reply(conn), "Consuming")) return;
         if (MMDAgent_strlen(m_queuename) == 0) {
            m_mmdagent->sendLogString(m_id, MLOG_STATUS, "%s: %s:%d: listen exchange=%s (%s)", m_name, m_host, m_port, m_exchangename, m_type);
         } else {
            m_mmdagent->sendLogString(m_id, MLOG_STATUS, "%s: %s:%d: listen exchange=%s (%s), bindingkey=%s", m_name, m_host, m_port, m_exchangename, m_type, m_queuename);
         }
      }
   }

   if (m_mode == RABBITMQ_PRODUCER_MODE) {
      if (MMDAgent_strlen(m_exchangename) > 0) {
         // declare exchange on server
         amqp_exchange_declare(conn, 1, amqp_cstring_bytes(m_exchangename), amqp_cstring_bytes(m_type), 0, 0, 0, 0, amqp_empty_table);
         if (on_amqp_error(amqp_get_rpc_reply(conn), "Declaring exchante")) return;
      }
      m_mmdagent->sendLogString(m_id, MLOG_STATUS, "%s: %s:%d: send exchange=%s (%s), bindingkey=%s", m_name, m_host, m_port, m_exchangename, m_type, m_queuename);
   }

   /* main loop */
   while (m_active == true && m_thread && m_thread->isRunning()) {
      amqp_rpc_reply_t res;
      amqp_envelope_t envelope;

      if (m_mode == RABBITMQ_CONSUMER_MODE) {
         /* wait for message to arrive in the queue and process it */
         amqp_maybe_release_buffers(conn);
         res = amqp_consume_message(conn, &envelope, NULL, 0);
         if (AMQP_RESPONSE_NORMAL != res.reply_type)
            break;
         parse_received_data((char *)envelope.message.body.bytes, (int)envelope.message.body.len);
         amqp_destroy_envelope(&envelope);
      }
      if (m_mode == RABBITMQ_PRODUCER_MODE) {
         /* send stored message to server */
         while (m_thread->dequeueBuffer(1, buff, NULL) > 0) {
            amqp_bytes_t message_bytes;
            message_bytes.bytes = buff;
            message_bytes.len = MMDAgent_strlen(buff);
            amqp_basic_publish(conn, 1, amqp_cstring_bytes(m_exchangename), amqp_cstring_bytes(m_queuename), 0, 0, NULL, message_bytes);
         }
         /* busy wait */
         MMDAgent_sleep(0.05);
      }
   }

   /* disconnect */
   if (MMDAgent_strlen(m_exchangename) > 0)
      amqp_bytes_free(queuename);
   if (on_amqp_error(amqp_channel_close(conn, 1, AMQP_REPLY_SUCCESS), "Closing channel")) return;
   if (on_amqp_error(amqp_connection_close(conn, AMQP_REPLY_SUCCESS), "Closing connection")) return;
   int ret = amqp_destroy_connection(conn);
   if (ret < 0) {
      m_mmdagent->sendLogString(m_id, MLOG_ERROR, "%s: ending connection: %s", m_name, amqp_error_string2(ret));
   }
   m_mmdagent->sendLogString(m_id, MLOG_STATUS, "%s: channel closed", m_name);
   m_active = false;
}

/* RabbitMQ::enqueueMessage: enqueue message to be sent to server */
void RabbitMQ::enqueueMessage(const char *str)
{
   if (m_active == true && m_thread->isRunning() && m_mode == RABBITMQ_PRODUCER_MODE)
      m_thread->enqueueBuffer(1, str, NULL);
}

/* RabbitMQ::play_motion: play motion */
void RabbitMQ::play_motion(const char *motionAlias, const char *motionFileName)
{
   char buff[MMDAGENT_MAXBUFLEN];
   const char *modelAlias = m_motion_config->getParam("target_model_alias");

   // find target model
   int id = m_mmdagent->findModelAlias(modelAlias);
   if (id < 0)
      return;

   // search for unused motion alias slot
   int i;
   for (i = 0; i < ACTION_MOTION_MAXNUM; i++) {
      MMDAgent_snprintf(buff, MMDAGENT_MAXBUFLEN, "%s%d", motionAlias, i);
      if (m_mmdagent->getModelList()[id].getMotionManager()->getRunning(buff) == NULL)
         break;
   }
   if (i >= ACTION_MOTION_MAXNUM)
      return;

   // issue MOTION_ADD for the slot
   m_mmdagent->sendMessage(m_id, MMDAGENT_COMMAND_MOTIONADD, "%s|%s|%s|PART|ONCE|ON|OFF", modelAlias, buff, motionFileName);

   // issue MOTION_DELETE for already running motions other than above
   for (int k = 0; k < ACTION_MOTION_MAXNUM; k++) {
      if (i == k)
         continue;
      MMDAgent_snprintf(buff, MMDAGENT_MAXBUFLEN, "%s%d", motionAlias, k);
      if (m_mmdagent->getModelList()[id].getMotionManager()->getRunning(buff))
         m_mmdagent->sendMessage(m_id, MMDAGENT_COMMAND_MOTIONDELETE, "%s|%s", modelAlias, buff);
   }
}

/* RabbitMQ::parse_received_data: parse received data */
void RabbitMQ::parse_received_data(char *buf, int len)
{
   char *buff;

   buff = (char *)malloc(len + 1);
   strncpy(buff, buf, len);
   buff[len] = '\0';

   /* parse json string with Poco::JSON parser */
   Poco::JSON::Parser parser;
   Poco::Dynamic::Var result = parser.parse(buff);
   Poco::JSON::Object::Ptr object = result.extract<Poco::JSON::Object::Ptr>();

   std::string timestamp = object->getValue<std::string>("timestamp");
   std::string id = object->getValue<std::string>("id");
   std::string producer = object->getValue<std::string>("producer");
   std::string update_type = object->getValue<std::string>("update_type");
   std::string data_type = object->getValue<std::string>("data_type");
   std::string body = object->getValue<std::string>("body");

   m_mmdagent->sendLogString(m_id, MLOG_STATUS, "%s: %s, %s, %s", m_name, id.c_str(), producer.c_str(), data_type.c_str());

   if (m_sync) {
      if (data_type == "audio") {
         /* decode audio and send to Julius for lip sync and play */
         std::istringstream istr(body);
         Poco::Base64Decoder decoder(istr);
         std::vector<char> decodedData;

         char ch;
         while (decoder.get(ch)) {
            decodedData.push_back(ch);
         }

         int len = decodedData.size();
         char *buf = (char *)malloc(len);
         if (buf) {
            for (int i = 0; i < len; i++)
               buf[i] = decodedData[i];
            m_sync->processSoundData(buf, len);
            m_sync->segmentSoundData();
            m_mmdagent->sendLogString(m_id, MLOG_STATUS, "%s: audio len = %d", m_name, len);
            free(buf);
         }
      }
   }

   // execute motions
   if (data_type == "expression_and_action") {
      Poco::JSON::Parser parser;
      Poco::Dynamic::Var result = parser.parse(body);
      Poco::JSON::Object::Ptr obj = result.extract<Poco::JSON::Object::Ptr>();

      if (obj->has("expression")) {
         std::string s = obj->getValue<std::string>("expression");
         const char *motionFile = m_motion_config->getExpressionPath(s.c_str());
         if (motionFile)
            play_motion(m_motion_config->getParam("motion_expression_alias"), motionFile);
      }
      if (obj->has("action")) {
         std::string s = obj->getValue<std::string>("action");
         const char *motionFile = m_motion_config->getActionPath(s.c_str());
         if (motionFile)
            play_motion(m_motion_config->getParam("motion_action_alias"), motionFile);
      }

   }

   free(buff);
}
