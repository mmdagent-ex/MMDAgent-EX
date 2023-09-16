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
   Thread *m_thread;       // thread instance
   char *m_host;           // RabbitMQ host to connect
   int m_port;             // RabbitMQ port number to connect
   char *m_exchangename;   // exchange name
   char *m_queuename;      // queue name or routing key
   int m_mode;             // RABBITMQ_{CONSUMER|PRODUCER}_MODE
   bool m_active;          // true while active

   /* on_amqp_error: check return code of amqp functions, and when error, issue error message and return true */
   bool on_amqp_error(amqp_rpc_reply_t x, char const *context);

public:

   /* constructor */
   RabbitMQ(MMDAgent *mmdagent, int id, int mode, const char *host, int port, const char *exchangename, const char *queuename);

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
