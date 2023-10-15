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

#define MLOG_UNDEFINED        0
#define MLOG_ERROR            1
#define MLOG_WARNING          2
#define MLOG_STATUS           3
#define MLOG_DEBUG            4
#define MLOG_MESSAGE_SENT     5
#define MLOG_MESSAGE_CAPTURED 6
#define MLOG_MAX              7

/* MessageLink: message */
typedef struct _MessageLink {
   int id;
   unsigned int flag;
   char *type;
   char *value;
   struct _MessageLink *next;
} MessageLink;

/* MessageQueue: message list */
typedef struct _MessageQueue {
   MessageLink *head;
   MessageLink *tail;
} MessageQueue;

/* IdentLink: identification */
typedef struct _IdentLink {
   int id;
   char *name;
   struct _IdentLink *next;
} IdentLink;

/* IdentList: identification list */
typedef struct _IdentList {
   IdentLink *root;
   int num;
} IdentList;

/* Message: message queue */
class Message
{
private:

   GLFWmutex m_messageMutex;         /* message mutex */
   GLFWmutex m_logStringMutex;       /* log string mutex */

   MessageQueue m_messageQueue;      /* message queue */
   MessageQueue m_logStringQueue;    /* log string queue */

   IdentList m_identList;            /* module id list */

	bool m_skipFlag;                  /* skip saving message when true */

   /* initialize: initialize message queue */
   void initialize();

   /* clear: free message queue */
   void clear();

public:

   /* Message: constructor */
   Message();

   /* Message: destructor */
   ~Message();

   /* setup: setup message queue */
   bool setup();

   /* getId: get id */
   int getId(const char *ident);

   /* getIdString: get id string */
   const char *getIdString(int id);

   /* getFlagString: get flag string */
   const char *getFlagString(unsigned int flag);

   /* enqueueMessage: enqueue received message */
   void enqueueMessage(int id, const char *type, const char *value);

   /* enqueueLogString: enqueue log string */
   void enqueueLogString(int id, unsigned int flag, const char *log);

   /* dequeueMessage: dequeue received message */
   bool dequeueMessage(int *id, char *type, char *value);

   /* dequeueLogString: dequeue log string */
   bool dequeueLogString(int *id, unsigned int *flag, char *log, char *timestamp);

	/* setLogSkipFlag: set log skip flag */
   void setLogSkipFlag(bool flag);
};
