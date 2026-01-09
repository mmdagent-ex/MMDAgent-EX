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

#if __has_include(<rdkafka.h>)
#include <rdkafka.h>
#elif __has_include(<librdkafka/rdkafka.h>)
#include <librdkafka/rdkafka.h>
#else
#error "rdkafka.h not found"
#endif

// Server-client connection handling class
class Kafka
{
private:
   char *m_broker;            // broker address (host:port)
   char *m_codec;             // compression codec
   char *m_topic;             // topic
   char *m_key;               // unique key identifier for producer
   size_t m_keylen;           // length of the key
   int m_partition;           // partition for consumer

   int m_mode;                // producer / consumer mode switch

   char m_errstr[MMDAGENT_MAXBUFLEN];  // error string holder
   rd_kafka_conf_t *m_conf;            // rdkafka configuration
   rd_kafka_topic_conf_t *m_topicConf; // rdkafka topic configuration
   rd_kafka_t *m_rk;                   // rdkafka handler
   rd_kafka_topic_t *m_rkt;            // rdkafka topic handler

   // initialize
   void initialize();

   // clear
   void clear();

public:
   // constructor as producer
   Kafka(const char *broker, const char *codec, const char *topic, const char *key, int partition);

   // constructor as consumer
   Kafka(const char *broker, const char *codec, const char *topic, int partition);

   // destructor
   ~Kafka();

   // connect: make connection
   bool connect();

   // disconnect: disconnect socket
   void disconnect();

   // isConnected: return true when connected
   bool isConnected();

   // send: send data
   int send(void *buf, size_t len);

   // receive: receive data
   int receive(void *buf, size_t maxlen);

   // getMode: get mode
   int getMode();

   // getErrorString: get error string
   const char *getErrorString();
};
