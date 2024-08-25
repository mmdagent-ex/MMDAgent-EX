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
/* ----------------------------------------------------------------- */
/*           Toolkit for Building Voice Interaction Systems          */
/*           MMDAgent developed by MMDAgent Project Team             */
/*           http://www.mmdagent.jp/                                 */
/* ----------------------------------------------------------------- */
/*                                                                   */
/*  Copyright (c) 2009-2016  Nagoya Institute of Technology          */
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

#include <time.h>
#if defined(__APPLE__)
#include <sys/time.h>
#endif
#include "MMDAgent.h"

/* MessageLink_initialize: initialize message */
static void MessageLink_initialize(MessageLink *l, int id, unsigned int flag, const char *type, const char *value)
{
   l->id = id;
   l->flag = flag;
   l->type = MMDAgent_strdup(type);
   l->value = MMDAgent_strdup(value);
   l->next = NULL;
}

/* MessageLink_clear: free message */
static void MessageLink_clear(MessageLink *l)
{
   if(l->type != NULL)
      free(l->type);
   if(l->value != NULL)
      free(l->value);
}

/* MessageQueue_initialize: initialize message list */
static void MessageQueue_initialize(MessageQueue *q)
{
   q->head = NULL;
   q->tail = NULL;
}

/* MessageQueue_clear: free message list */
static void MessageQueue_clear(MessageQueue *q)
{
   MessageLink *tmp1, *tmp2;

   for(tmp1 = q->head; tmp1 ; tmp1 = tmp2) {
      tmp2 = tmp1->next;
      MessageLink_clear(tmp1);
      free(tmp1);
   }
}

/* IdentLink_initialize: initialize identifications */
static void IdentLink_initialize(IdentLink *l, int id, const char *name)
{
   l->id = id;
   l->name = MMDAgent_strdup(name);
   l->next = NULL;
}

/* IdentLink_clear: free identifications */
static void IdentLink_clear(IdentLink *l)
{
   if(l->name != NULL)
      free(l->name);
}

/* IdentList_initialize: initialize identification list */
static void IdentList_initialize(IdentList *q)
{
   q->root = NULL;
   q->num = 0;
}

/* IdentList_clear: free identification list */
static void IdentList_clear(IdentList *q)
{
   IdentLink *tmp1, *tmp2;

   for(tmp1 = q->root; tmp1 ; tmp1 = tmp2) {
      tmp2 = tmp1->next;
      IdentLink_clear(tmp1);
      free(tmp1);
   }
}

/* MessageQueue_enqueue: enqueue */
static void MessageQueue_enqueue(MessageQueue *q, int id, unsigned int flag, const char *type, const char *value)
{
   if(MMDAgent_strlen(type) <= 0)
      return;

   if(q->tail == NULL) {
      q->tail = (MessageLink *) malloc(sizeof(MessageLink));
      MessageLink_initialize(q->tail, id, flag, type, value);
      q->head = q->tail;
   } else {
      q->tail->next = (MessageLink *) malloc(sizeof(MessageLink));
      MessageLink_initialize(q->tail->next, id, flag, type, value);
      q->tail = q->tail->next;
   }
}

/* MessageQueue_dequeue: dequeue */
static bool MessageQueue_dequeue(MessageQueue *q, int *id, unsigned int *flag, char *type, char *value)
{
   MessageLink *tmp;

   if(q->head == NULL) {
      *id = -1;
      *flag = 0;
      strcpy(type, "");
      if(value != NULL)
         strcpy(value, "");
      return false;
   }

   *id = q->head->id;
   *flag = q->head->flag;
   strcpy(type, q->head->type);
   if(q->head->value != NULL && value != NULL)
      strcpy(value, q->head->value);
   else
      strcpy(value, "");

   tmp = q->head->next;
   MessageLink_clear(q->head);
   free(q->head);
   q->head = tmp;
   if(tmp == NULL)
      q->tail = NULL;

   return true;
}

/* IdentList_find: find identification */
static int IdentList_find(IdentList *q, const char *name)
{
   IdentLink *l;

   for (l = q->root; l; l = l->next) {
      if (MMDAgent_strequal(l->name, name)) {
         return l->id;
      }
   }
   return -1;
}

/* IdentList_findbyid: find identification by id */
static const char *IdentList_findById(IdentList *q, int id)
{
   IdentLink *l;

   if (id < 0 || id >= q->num)
      return NULL;

   for (l = q->root; l; l = l->next) {
      if (l->id == id)
         return l->name;
   }
   return NULL;
}

/* IdentList_add: add identification */
static int IdentList_add(IdentList *q, const char *name)
{
   IdentLink *l;
   int id;

   if(MMDAgent_strlen(name) <= 0)
      return -1;

   id = q->num;
   q->num++;

   l = (IdentLink *) malloc(sizeof(IdentLink));
   IdentLink_initialize(l, id, name);
   l->next = q->root;
   q->root = l;

   return id;
}

/* Message::initialize: initialize message queue */
void Message::initialize()
{
   m_messageMutex = NULL;
   m_logStringMutex = NULL;

   MessageQueue_initialize(&m_messageQueue);
   MessageQueue_initialize(&m_logStringQueue);
   IdentList_initialize(&m_identList);

	m_skipFlag = false;
}

/* Message::clear: free message queue */
void Message::clear()
{
   if(m_messageMutex)
      glfwDestroyMutex(m_messageMutex);
   if(m_logStringMutex)
      glfwDestroyMutex(m_logStringMutex);

   MessageQueue_clear(&m_messageQueue);
   MessageQueue_clear(&m_logStringQueue);
   IdentList_clear(&m_identList);

   initialize();
}

/* Message::Message: constructor */
Message::Message()
{
   initialize();
}

/* Message::~Message: destructor */
Message::~Message()
{
   clear();
}

/* Message::setup: setup message queue */
bool Message::setup()
{
   clear();

   m_messageMutex = glfwCreateMutex();
   m_logStringMutex = glfwCreateMutex();

   if(m_messageMutex == NULL || m_logStringMutex == NULL) {
      clear();
      return false;
   }

   return true;
}

/* Message::getId: get id */
int Message::getId(const char *ident)
{
   int id;

   if ( (id = IdentList_find(&m_identList, ident)) == -1)
      id = IdentList_add(&m_identList, ident);

   return id;
}

/* Message::getIdString: get id string */
const char *Message::getIdString(int id)
{
   return IdentList_findById(&m_identList, id);
}

/* Message::getFlagString: get flag string */
const char *Message::getFlagString(unsigned int flag)
{
   static const char *flagString[] = {
      "",
      "Error",
      "Warning",
      "Status",
      "Debug",
      "Sent",
      "Captured"
   };

   if (flag >= MLOG_MAX)
      return flagString[0];

   return flagString[flag];
}

/* Message::enqueueMessage: enqueue message */
void Message::enqueueMessage(int id, const char *type, const char *value)
{
   glfwLockMutex(m_messageMutex);
   MessageQueue_enqueue(&m_messageQueue, id, 0, type, value);
   glfwUnlockMutex(m_messageMutex);
}

/* Message::enqueueLogString: enqueue log string */
void Message::enqueueLogString(int id, unsigned int flag, const char *log)
{
   char buf[MMDAGENT_MAXBUFLEN];

   if (m_skipFlag == true)
      return;

   MMDAgent_gettimestampstr(buf, MMDAGENT_MAXBUFLEN, "%4d/%02d/%02d %02d:%02d:%02d.%03d");
   glfwLockMutex(m_logStringMutex);
   MessageQueue_enqueue(&m_logStringQueue, id, flag, log, buf);
   glfwUnlockMutex(m_logStringMutex);
}

/* Message::dequeueMessage: dequeue message */
bool Message::dequeueMessage(int *id, char *type, char *value)
{
   bool result;
   unsigned int flag;
   glfwLockMutex(m_messageMutex);
   result = MessageQueue_dequeue(&m_messageQueue, id, &flag, type, value);
   glfwUnlockMutex(m_messageMutex);
   return result;
}

/* Message::dequeueLogString: dequeue log string */
bool Message::dequeueLogString(int *id, unsigned int *flag, char *log, char *timestamp)
{
   bool result;
   glfwLockMutex(m_logStringMutex);
   result = MessageQueue_dequeue(&m_logStringQueue, id, flag, log, timestamp);
   glfwUnlockMutex(m_logStringMutex);
   return result;
}

/* Message::setLogSkipFlag: set log skip flag */
void Message::setLogSkipFlag(bool flag)
{
   m_skipFlag = flag;
}
