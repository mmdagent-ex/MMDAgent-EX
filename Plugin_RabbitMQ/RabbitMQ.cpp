/* ----------------------------------------------------------------- */
/*           The Toolkit for Building Voice Interaction Systems      */
/*           "MMDAgent" developed by MMDAgent Project Team           */
/*           http://www.mmdagent.jp/                                 */
/* ----------------------------------------------------------------- */
/*                                                                   */
/*  Copyright (c) 2009-2023  Nagoya Institute of Technology          */
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
#include "Thread.h"
#include "RabbitMQ.h"

#include <iostream>
#include <fstream>

/*
*
* This module currently supports only basic usage.
*
* In consumer mode, connect to the queue of the name [queuename]
*
* In producer mode, connect to the exchange of the name [exchangename] with routing key [queuename]
*
*/

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
      m_mmdagent->sendLogString(m_id, MLOG_ERROR, "%s: missing RPC reply type!", context);
      break;

   case AMQP_RESPONSE_LIBRARY_EXCEPTION:
      m_mmdagent->sendLogString(m_id, MLOG_ERROR, "%s: %s", context, amqp_error_string2(x.library_error));
      break;

   case AMQP_RESPONSE_SERVER_EXCEPTION:
      switch (x.reply.id) {
      case AMQP_CONNECTION_CLOSE_METHOD: {
         amqp_connection_close_t *m = (amqp_connection_close_t *)x.reply.decoded;
         m_mmdagent->sendLogString(m_id, MLOG_ERROR, "%s: server connection error %uh, message: %.*s", context, m->reply_code, (int)m->reply_text.len, (char *)m->reply_text.bytes);
         break;
      }
      case AMQP_CHANNEL_CLOSE_METHOD: {
         amqp_channel_close_t *m = (amqp_channel_close_t *)x.reply.decoded;
         m_mmdagent->sendLogString(m_id, MLOG_ERROR, "%s: server channel error %uh, message: %.*s", context, m->reply_code, (int)m->reply_text.len, (char *)m->reply_text.bytes);
         break;
      }
      default:
         m_mmdagent->sendLogString(m_id, MLOG_ERROR, "%s: unknown server error, method id 0x%08X", context, x.reply.id);
         break;
      }
      break;
   }
   m_active = false;
   return true;
}

/* constructor */
RabbitMQ::RabbitMQ(MMDAgent *mmdagent, int id, int mode, const char *host, int port, const char *exchangename, const char *queuename)
{
   m_mmdagent = mmdagent;
   m_id = id;
   m_thread = NULL;
   m_active = false;
   m_host = MMDAgent_strdup(host);
   m_port = port;
   m_exchangename = MMDAgent_strdup(exchangename);
   m_queuename = MMDAgent_strdup(queuename);
   m_mode = mode;
   m_thread = new Thread;
   m_thread->setup();
}

/* destructor */
RabbitMQ::~RabbitMQ()
{
   m_active = false;
   delete m_thread;
   free(m_queuename);
   free(m_exchangename);
   free(m_host);
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

   /* make connection to rabbitmq server */
   amqp_connection_state_t conn = amqp_new_connection();
   amqp_socket_t *socket = amqp_tcp_socket_new(conn);
   if (socket == NULL) {
      m_mmdagent->sendLogString(m_id, MLOG_ERROR, "failed to create TCP socket");
      return;
   }
   int status = amqp_socket_open(socket, m_host, m_port);
   if (status) {
      m_mmdagent->sendLogString(m_id, MLOG_ERROR, "failed to connect to rabbitmq server %s:%d", m_host, m_port);
      return;
   }

   if (on_amqp_error(amqp_login(conn, "/", 0, 131072, 0, AMQP_SASL_METHOD_PLAIN, "guest", "guest"), "Logging in")) return;
   amqp_channel_open(conn, 1);
   if (on_amqp_error(amqp_get_rpc_reply(conn), "Opening channel")) return;

   if (m_mode == RABBITMQ_CONSUMER_MODE) {
#if 1
      // start listening to a queue as a consumer
      amqp_basic_consume(conn, 1, amqp_cstring_bytes(m_queuename), amqp_empty_bytes, 0, 0, 0, amqp_empty_table);
      if (on_amqp_error(amqp_get_rpc_reply(conn), "Consuming")) return;
      m_mmdagent->sendLogString(m_id, MLOG_STATUS, "listening %s:%d: queue=%s", m_host, m_port, m_queuename);
#else
      // declare a new queue for given exchange and binding key and connect to it as a consumer
      amqp_queue_declare_ok_t *r = amqp_queue_declare(conn, 1, amqp_empty_bytes, 0, 0, 0, 1, amqp_empty_table);
      if (on_amqp_error(amqp_get_rpc_reply(conn), "Declaring queue")) return;
      amqp_bytes_t queuename = amqp_bytes_malloc_dup(r->queue);
      if (queuename.bytes == NULL) {
         m_mmdagent->sendLogString(m_id, MLOG_ERROR, "out of memory while copying queue name");
         m_active = false;
         return;
      }
      amqp_queue_bind(conn, 1, queuename, amqp_cstring_bytes(exchange), amqp_cstring_bytes(bindingkey), amqp_empty_table);
      if (on_amqp_error(amqp_get_rpc_reply(conn), "Binding queue")) return;
      amqp_basic_consume(conn, 1, queuename, amqp_empty_bytes, 0, 1, 0, amqp_empty_table);
      if (on_amqp_error(amqp_get_rpc_reply(conn), "Consuming")) return;
      m_mmdagent->sendLogString(m_id, MLOG_STATUS, "listening %s:%d: exchange=%s, bindingkey=%s", m_host, m_port, m_exchangename, m_queuename);
#endif
   }

   if (RABBITMQ_PRODUCER_MODE) {
      m_mmdagent->sendLogString(m_id, MLOG_STATUS, "publishing to %s:%d: exchange=%s, bindingkey=%s", m_host, m_port, m_exchangename, m_queuename);
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
         MMDAgent_snprintf(buff, MMDAGENT_MAXBUFLEN, "%*s", (int)envelope.message.body.len, (char *)envelope.message.body.bytes);
         buff[(int)envelope.message.body.len] = '\0';
         m_mmdagent->sendMessage(m_id, PLUGINRABBITMQ_EVENT_RECEIVED, "%s", buff);
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
      }
   }

   /* disconnect */
#if 0
   amqp_bytes_free(queuename);
#endif
   if (on_amqp_error(amqp_channel_close(conn, 1, AMQP_REPLY_SUCCESS), "Closing channel")) return;
   if (on_amqp_error(amqp_connection_close(conn, AMQP_REPLY_SUCCESS), "Closing connection")) return;
   int ret = amqp_destroy_connection(conn);
   if (ret < 0) {
      m_mmdagent->sendLogString(m_id, MLOG_ERROR, "Ending connection: %s", amqp_error_string2(ret));
   }
   m_mmdagent->sendLogString(m_id, MLOG_STATUS, "channel closed");
   m_active = false;
}

/* RabbitMQ::enqueueMessage: enqueue message to be sent to server */
void RabbitMQ::enqueueMessage(const char *str)
{
   if (m_active == true && m_thread->isRunning() && m_mode == RABBITMQ_PRODUCER_MODE)
      m_thread->enqueueBuffer(1, str, NULL);
}
