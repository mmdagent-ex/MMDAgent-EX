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
extern "C" {
#if defined(_WIN32) || defined(__APPLE__)
#include <rabbitmq-c/amqp.h>
#include <rabbitmq-c/tcp_socket.h>
#else
#include <amqp.h>
#include <amqp_tcp_socket.h>
#endif
}

/* definitions */
#define RABBITMQ_CONSUMER_MODE 0
#define RABBITMQ_PRODUCER_MODE 1
#define PLUGINRABBITMQ_COMMAND_SEND   "RABBITMQ_SEND"
#define PLUGINRABBITMQ_EVENT_RECEIVED "RABBITMQ_EVENT_RECV"

class RabbitMQ
{
private:

   MMDAgent* m_mmdagent;   // MMDAgent instance
   int m_id;               // module id
   char *m_name;           // name
   int m_mode;             // RABBITMQ_{CONSUMER|PRODUCER}_MODE
   char *m_host;           // RabbitMQ host to connect
   int m_port;             // RabbitMQ port number to connect
   char *m_exchangename;   // exchange name
   char *m_queuename;      // queue name or routing key
   Thread *m_thread;       // thread instance
   bool m_active;          // true while active

   /* on_amqp_error: check return code of amqp functions, and when error, issue error message and return true */
   bool on_amqp_error(amqp_rpc_reply_t x, char const *context);

public:

   /* constructor */
   RabbitMQ(MMDAgent *mmdagent, int id, const char *name, int mode, const char *host, int port, const char *exchangename, const char *queuename);

   /* destructor */
   ~RabbitMQ();

   /* getMode: get mode */
   int getMode();

   /* start: start processing thread */
   void start();

   /* run: thread main loop function */
   void run();

   /* enqueueMessage: enqueue message to be sent to server */
   void enqueueMessage(const char *str);

};
