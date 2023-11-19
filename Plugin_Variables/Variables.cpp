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
/*           The Toolkit for Building Voice Interaction Systems      */
/*           "MMDAgent" developed by MMDAgent Project Team           */
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

/* headers */

#include <time.h>
#include "MMDAgent.h"
#include "Variables.h"

/* Variables::initialize: initialize */
void Variables::initialize()
{
   m_head = NULL;
   m_tail = NULL;

   m_mmdagent = NULL;

   srand((unsigned) time(NULL));
}

/* Variables::clear: free */
void Variables::clear()
{
   Value *tmp1, *tmp2;

   for(tmp1 = m_head; tmp1 ; tmp1 = tmp2) {
      tmp2 = tmp1->next;
      free(tmp1->name);
      free(tmp1->sval);
      free(tmp1);
   }

   initialize();
}

/* Variables::Variables: thread constructor */
Variables::Variables()
{
   initialize();
}

/* Variables::~Variables: thread destructor */
Variables::~Variables()
{
   clear();
}

/* Variables::setup: setup variables */
void Variables::setup(MMDAgent *mmdagent, int id)
{
   if(mmdagent == NULL)
      return;

   m_mmdagent = mmdagent;
   m_id = id;
}

/* Variables::set: set value */
void Variables::set(const char *alias, const char *str1, const char *str2)
{
   Value *val;

   float max, min, tmp;

   if(MMDAgent_strlen(alias) <= 0) {
      m_mmdagent->sendLogString(m_id, MLOG_ERROR, "set: null alias string?");
      return;
   }
   /* check the same alias */
   for(val = m_head; val; val = val->next) {
      if(MMDAgent_strequal(val->name, alias))
         break;
   }

   /* push */
   if(val == NULL) {
      val = (Value *) malloc(sizeof(Value));
      val->name = MMDAgent_strdup(alias);
      val->next = NULL;
      if(m_tail == NULL) {
         m_head = val;
         val->prev = NULL;
      } else {
         m_tail->next = val;
         val->prev = m_tail;
      }
      m_tail = val;
   } else {
      free(val->sval);
   }

   /* set string */
   if(str2 == NULL) {
      val->sval = MMDAgent_strdup(str1);
   } else {
      int plen = sizeof(char) * (MMDAgent_strlen(str1) + 1 + MMDAgent_strlen(str2) + 1);
      val->sval = (char *) malloc(plen);
      MMDAgent_snprintf(val->sval, plen, "%s|%s", str1, str2);
   }

   /* set float */
   if(str2 == NULL) {
      val->fval = MMDAgent_str2float(str1);
   } else {
      min = MMDAgent_str2float(str1);
      max = MMDAgent_str2float(str2);
      if(max < min) {
         tmp = max;
         max = min;
         min = tmp;
      }
      val->fval = min + (max - min) * (rand() - 0.0f) * (1.0f / (RAND_MAX - 0.0f)); /* 0.0f is RAND_MIN */
   }

   m_mmdagent->sendMessage(m_id, VARIABLES_VALUESETEVENT, "%s", alias); /* send message */
}

/* Variables::unset: unset value */
void Variables::unset(const char *alias)
{
   Value *tmp1, *tmp2;

   for(tmp1 = m_head; tmp1; tmp1 = tmp2) {
      tmp2 = tmp1->next;
      if(MMDAgent_strequal(tmp1->name, alias)) {
         if(tmp1 == m_head) {
            if(tmp1 == m_tail) {
               m_head = NULL;
               m_tail = NULL;
            } else {
               m_head = tmp1->next;
               tmp1->next->prev = NULL;
            }
         } else {
            if(tmp1 == m_tail) {
               m_tail = tmp1->prev;
               tmp1->prev->next = NULL;
            } else {
               tmp1->next->prev = tmp1->prev;
               tmp1->prev->next = tmp1->prev;
            }
         }
         m_mmdagent->sendMessage(m_id, VARIABLES_VALUEUNSETEVENT, "%s", tmp1->name); /* send message */
         free(tmp1->name);
         free(tmp1->sval);
         free(tmp1);
         break;
      }
   }
   if (tmp1 == NULL)
      m_mmdagent->sendLogString(m_id, MLOG_WARNING, "unset: alias \"%s\" not exist", alias);
}

/* Variables::evaluate: evaluate value */
void Variables::evaluate(const char *alias, const char *mode, const char *str)
{
   Value *val;
   float f1, f2;
   bool ret;

   /* get value 1 */
   for(val = m_head; val; val = val->next) {
      if(MMDAgent_strequal(val->name, alias) == true) {
         f1 = val->fval;
         break;
      }
   }
   if(val == NULL) {
      m_mmdagent->sendLogString(m_id, MLOG_ERROR, "evaluate: alias \"%s\" not exist", alias);
      return;
   }

   /* get value 2 */
   f2 = MMDAgent_str2float(str);

   /* evaluate */
   if(MMDAgent_strequal(mode, VARIABLES_EQ) == true) {
      if(f1 == f2)
         ret = true;
      else
         ret = false;
   } else if(MMDAgent_strequal(mode, VARIABLES_NE) == true) {
      if(f1 != f2)
         ret = true;
      else
         ret = false;
   } else if(MMDAgent_strequal(mode, VARIABLES_LE) == true) {
      if(f1 <= f2)
         ret = true;
      else
         ret = false;
   } else if(MMDAgent_strequal(mode, VARIABLES_LT) == true) {
      if(f1 < f2)
         ret = true;
      else
         ret = false;
   } else if(MMDAgent_strequal(mode, VARIABLES_GE) == true) {
      if(f1 >= f2)
         ret = true;
      else
         ret = false;
   } else if(MMDAgent_strequal(mode, VARIABLES_GT) == true) {
      if(f1 > f2)
         ret = true;
      else
         ret = false;
   } else {
      /* unknown mode */
      m_mmdagent->sendLogString(m_id, MLOG_ERROR, "evaluate: unknown mode \"%s\"", mode);
      return;
   }

   if(ret == true)
      m_mmdagent->sendMessage(m_id, VARIABLES_VALUEEVALEVENT, "%s|%s|%s|%s", alias, mode, str, VARIABLES_TRUE);
   else
      m_mmdagent->sendMessage(m_id, VARIABLES_VALUEEVALEVENT, "%s|%s|%s|%s", alias, mode, str, VARIABLES_FALSE);
}

/* Variables::get: get value */
void Variables::get(const char *alias)
{
   Value *val;

   for(val = m_head; val; val = val->next) {
      if(MMDAgent_strequal(val->name, alias) == true) {
         m_mmdagent->sendMessage(m_id, VARIABLES_VALUEGETEVENT, "%s|%s", alias, val->sval);
         return;
      }
   }
   m_mmdagent->sendLogString(m_id, MLOG_WARNING, "get: alias \"%s\" not exist", alias);
}
