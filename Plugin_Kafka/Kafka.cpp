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

// Kafka::initialize
void Kafka::initialize()
{
   m_broker = NULL;
   m_codec = NULL;
   m_topic = NULL;
   m_key = NULL;
   m_keylen = 0;
   m_partition = RD_KAFKA_PARTITION_UA;

   m_mode = 'C';

   m_conf = NULL;
   m_topicConf = NULL;
   m_rk = NULL;
   m_rkt = NULL;
}

// Kafka::clear
void Kafka::clear()
{
   int count;

   if (m_rk && m_mode == 'P') {
      /* wait for messages to be delivered */
      while (rd_kafka_outq_len(m_rk) > 0)
         rd_kafka_poll(m_rk, 100);
   }

   if (m_rkt && m_mode == 'C') {
      /* Stop consuming */
      rd_kafka_consume_stop(m_rkt, m_partition);
      while (rd_kafka_outq_len(m_rk) > 0)
         rd_kafka_poll(m_rk, 10);
   }

   if (m_topicConf)
      rd_kafka_topic_conf_destroy(m_topicConf);
//   if (m_conf)
//      rd_kafka_conf_destroy(m_conf);
   if (m_rkt)
      rd_kafka_topic_destroy(m_rkt);
   if (m_rk)
      rd_kafka_destroy(m_rk);

   // wait background threads to clean up
   count = 5;
   while (count-- > 0 && rd_kafka_wait_destroyed(500) == -1) {}

   if (m_broker)
      free(m_broker);
   if (m_codec)
      free(m_codec);
   if (m_topic)
      free(m_topic);
   if (m_key)
      free(m_key);

   initialize();
}

// constructor as producer
Kafka::Kafka(const char *broker, const char *codec, const char *topic, const char *key, int partition)
{
   initialize();
   m_broker = MMDAgent_strdup(broker);
   m_codec = MMDAgent_strdup(codec);
   m_topic = MMDAgent_strdup(topic);
   m_key = MMDAgent_strdup(key);
   m_keylen = m_key ? strlen(m_key) : 0;
   m_partition = (partition >= 0) ? partition : RD_KAFKA_PARTITION_UA;
   m_mode = 'P';
}

// constructor as consumer
Kafka::Kafka(const char *broker, const char *codec, const char *topic, int partition)
{
   initialize();
   m_broker = MMDAgent_strdup(broker);
   m_codec = MMDAgent_strdup(codec);
   m_topic = MMDAgent_strdup(topic);
   m_partition = (partition >= 0) ? partition : RD_KAFKA_PARTITION_UA;
   m_mode = 'C';
}

// destructor
Kafka::~Kafka()
{
   clear();
}

// Kafka::connect: make connection
bool Kafka::connect()
{
   /* Kafka configuration */
   m_conf = rd_kafka_conf_new();

   /* Topic Configuration */
   m_topicConf = rd_kafka_topic_conf_new();

   /* set compression mode */
   if (m_codec) {
      if (rd_kafka_conf_set(m_conf, "compression.codec", m_codec, m_errstr, MMDAGENT_MAXBUFLEN) != RD_KAFKA_CONF_OK) {
         clear();
         return false;
      }
   }

   if (m_broker == NULL || m_topic == NULL)
      return false;

   /* create handle */
   m_rk = rd_kafka_new((m_mode == 'P') ? RD_KAFKA_PRODUCER : RD_KAFKA_CONSUMER, m_conf, m_errstr, MMDAGENT_MAXBUFLEN);
   if (m_rk == NULL) {
      clear();
      return false;
   }

   /* add broker */
   if (rd_kafka_brokers_add(m_rk, m_broker) == 0) {
      strcpy(m_errstr, "No valid brokers specified\n");
      clear();
      return false;
   }

   /* create topic */
   m_rkt = rd_kafka_topic_new(m_rk, m_topic, m_topicConf);
   if (m_rkt == NULL) {
      strcpy(m_errstr, "failed to create topic\n");
      clear();
      return false;
   }
   /* now m_topicConf is owned by m_topic */
   m_topicConf = NULL;

   if (m_mode == 'C') {
      /* start consuming from the last */
      if (rd_kafka_consume_start(m_rkt, m_partition, RD_KAFKA_OFFSET_END) == -1) {
         rd_kafka_resp_err_t err = rd_kafka_last_error();
         MMDAgent_snprintf(m_errstr, MMDAGENT_MAXBUFLEN, "failed to start consuming: %s\n", rd_kafka_err2str(err));
         clear();
         return false;
      }
   }

   return true;
}

// Kafka::diconnect: disconnect socket
void Kafka::disconnect()
{
   clear();
}

// Kafka::isConnected: return true when connected
bool Kafka::isConnected()
{
   if (m_rkt)
      return true;
   return false;
}

// Kafka::send: send data (producer)
int Kafka::send(void *buf, size_t len)
{
   rd_kafka_resp_err_t err;
   int ret;

   if (m_rkt == NULL || m_mode != 'P')
      return -1;

   if (rd_kafka_produce(m_rkt, m_partition, RD_KAFKA_MSG_F_COPY, buf, len, m_key, m_keylen, NULL) == -1) {
      err = rd_kafka_last_error();
      MMDAgent_snprintf(m_errstr, MMDAGENT_MAXBUFLEN, "failed to produce to topic %s partition %i: %s", rd_kafka_topic_name(m_rkt), m_partition, rd_kafka_err2str(err));
      ret = -1;
   } else {
      ret = (int)len;
   }

   rd_kafka_poll(m_rk, 0);

   return ret;
}

// Kafka::receive: receive data (consumer), would block
int Kafka::receive(void *buf, size_t maxlen)
{
   rd_kafka_message_t *rkmessage;
   char buff[MMDAGENT_MAXBUFLEN];

   if (m_rkt == NULL || m_mode != 'C')
      return -1;

   rd_kafka_poll(m_rk, 0);

   /* consume single message */
   rkmessage = rd_kafka_consume(m_rkt, m_partition, 300);
   if (!rkmessage) /* timeout */
      return 0;

   if (rkmessage->err) {
      if (rkmessage->err == RD_KAFKA_RESP_ERR__PARTITION_EOF) {
         /* consumer reached end of the topic */
         return 0;
      } else {
         MMDAgent_snprintf(m_errstr, MMDAGENT_MAXBUFLEN, "consume error for topic %s: %s", rd_kafka_topic_name(rkmessage->rkt), rd_kafka_message_errstr(rkmessage));
         if (rkmessage->err == RD_KAFKA_RESP_ERR__UNKNOWN_PARTITION || rkmessage->err == RD_KAFKA_RESP_ERR__UNKNOWN_TOPIC) {
            clear();
         }
         return -1;
      }
   }

   MMDAgent_snprintf(buff, MMDAGENT_MAXBUFLEN, "%.*s", (int)rkmessage->len, (char*)rkmessage->payload);
   if (strlen(buff) + 1 > maxlen)
      buff[maxlen] = '\0';
   strcpy((char *)buf, buff);

   /* return message to rdkafka */
   rd_kafka_message_destroy(rkmessage);

   return (int)strlen(buff);
}

// Kafka::getMode: get mode
int Kafka::getMode()
{
   return m_mode;
}

// Kafka::getErrorString: get error string
const char *Kafka::getErrorString()
{
   return m_errstr;
}
