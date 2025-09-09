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
#include <ctype.h>

#include "re2/re2.h"
#include "MMDAgent.h"
#include "VIManager.h"

/* findAsciiString: find ascii string */
static const char* findAsciiString(const char *str, const char *c)
{
   if (MMDAgent_getcharsize(c) != 1)
      return NULL;
   return strchr(str, (int) * c);
}

/* checkVariableName: check variable name */
static bool checkVariableName(const char *name)
{
   size_t i, len;
   const char *c;
   unsigned char size;

   len = MMDAgent_strlen(name);
   if (len <= 0)
      return false;

   c = name;
   for(i = 0; i < len; i += size) {
      size = MMDAgent_getcharsize(c);
      if(size == 0) /* fail safe */
         return false;
      if (i == 0 && *c == '%') {
         c += size;
         continue;
      }
      if (size != 1 || ! (isalnum(*c) || *c == '_'))
         return false;
      c += size;
   }

   return true;
}

/* getTokenFromStringWithQuoters: a general string separator with quote handling */
static int getTokenFromStringWithQuoters(const char *str, int *index, char *buff, const char *separators, const char *quoters, bool erasequote)
{
   int i, j = 0, len;
   const char *c;
   unsigned char size;
   bool inQuote = false;
   char currentQuote = '\0';

   if (str == NULL) {
      buff[0] = '\0';
      return 0;
   }

   len = (int)MMDAgent_strlen(str);
   if (len <= 0) {
      buff[0] = '\0';
      return 0;
   }

   /* skip the first separator sequence to move forward to the head of the next token */
   c = str + (*index);
   for (i = 0; i < len && (*c == '\0' || findAsciiString(separators, c) != NULL); i += size) {
      if (*c == '\0') {
         buff[0] = '\0';
         return 0;
      }
      size = MMDAgent_getcharsize(c);
      if (size == 0) {
         buff[0] = '\0';
         return -1;
      }
      if (erasequote == false) {
         memcpy(&buff[j], c, sizeof(char) * size);
         j += size;
      }
      (*index) += size;
      c += size;
   }

   /* copy the token to buff till the end of the token */
   for (i = 0; i < len && *c != '\0' && (inQuote == true || findAsciiString(separators, c) == NULL); i += size) {
      if (quoters != NULL && findAsciiString(quoters, c) != NULL && ! (inQuote == true && *c != currentQuote)) {
         /* toggle quote mode */
         if (inQuote)
            inQuote = false;
         else {
            inQuote = true;
            currentQuote = *c;
         }
         /* skip quote */
         size = MMDAgent_getcharsize(c);
         if (size == 0) {
            buff[0] = '\0';
            return -1;
         }
         if (erasequote == false) {
            memcpy(&buff[j], c, sizeof(char) * size);
            j += size;
         }
         (*index) += size;
         c += size;
         continue;
      }
      size = MMDAgent_getcharsize(c);
      if (size == 0) {
         buff[0] = '\0';
         return -1;
      }
      memcpy(&buff[j], c, sizeof(char) * size);
      j += size;
      (*index) += size;
      c += size;
   }

   buff[j] = '\0';

   /* move index forward to skip the last separator sequence */
   for (; i < len && *c != '\0' && findAsciiString(separators, c) != NULL; i += size) {
      size = MMDAgent_getcharsize(c);
      if (size == 0) {
         buff[0] = '\0';
         return -1;
      }
      if (erasequote == false) {
         memcpy(&buff[j], c, sizeof(char) * size);
         j += size;
      }
      (*index) += size;
      c += size;
   }

   return j;
}

/* getTokenFromString: get token from string for reading FST definition fields */
static int getTokenFromString(const char *str, int *index, char *buff, bool erasequote)
{
   return getTokenFromStringWithQuoters(str, index, buff, " \t\n\r", "\"'", erasequote);
}

/* getArgFromString: get argument from string for parsing argment string */
static int getArgFromString(const char *str, int *index, char *buff, char separator)
{
   char separatorList[2];
   separatorList[0] = separator;
   separatorList[1] = '\0';
   return getTokenFromStringWithQuoters(str, index, buff, separatorList, NULL, true);
}

/* countArgs: count arguments */
static int countArgs(const char *str, char separator)
{
   int i, len, num;
   const char *c;
   unsigned char size;

   len = (int)MMDAgent_strlen(str);
   if(len <= 0)
      return 0;

   num = 1;
   c = str;
   for(i = 0; i < len; i += size) {
      size = MMDAgent_getcharsize(c);
      if(size == 0) { /* fail safe */
         return 0;
      }
      if (size == 1 && *c == separator)
         num++;
      c += size;
   }

   return num;
}

/* InputArguments_initialize: initialize input arguments */
void InputArguments_initialize(InputArguments *ia, const char *str)
{
   int i, j, idx1, idx2;
   char buff1[MMDAGENT_MAXBUFLEN];
   char buff2[MMDAGENT_MAXBUFLEN];

   /* get number of arguments */
   ia->size = countArgs(str, VIMANAGER_SEPARATOR1);
   if(ia->size <= 0) {
      ia->size = 0;
      ia->args = NULL;
      ia->argc = NULL;
      ia->str = NULL;
      return;
   }

   ia->args = (char ***) malloc(ia->size * sizeof(char **));
   ia->argc = (int *) malloc(ia->size * sizeof(int));
   ia->str = MMDAgent_strdup(str);

   /* get event arguments */
   idx1 = 0;
   for(i = 0; i < ia->size; i++) {
      getArgFromString(str, &idx1, buff1, VIMANAGER_SEPARATOR1);

      ia->argc[i] = countArgs(buff1, VIMANAGER_SEPARATOR2);
      if(ia->argc[i] <= 0) {
         ia->args[i] = NULL;
      } else {
         ia->args[i] = (char **) malloc(ia->argc[i] * sizeof(char *));
         idx2 = 0;
         for(j = 0; j < ia->argc[i]; j++) {
            getArgFromString(buff1, &idx2, buff2, VIMANAGER_SEPARATOR2);
            ia->args[i][j] = MMDAgent_strdup(buff2);
         }
      }
   }
}

/* InputArguments_clear: free input arguments */
void InputArguments_clear(InputArguments *ia)
{
   int i, j;

   if(ia->args != NULL) {
      for(i = 0; i < ia->size; i++) {
         for(j = 0; j < ia->argc[i]; j++)
            free(ia->args[i][j]);
         if(ia->args[i] != NULL)
            free(ia->args[i]);
      }
      free(ia->args);
      free(ia->argc);
      if (ia->str != NULL)
         free(ia->str);
      ia->size = 0;
      ia->args = NULL;
      ia->argc = NULL;
      ia->str = NULL;
   }
}

/* VIManager_Arc_initialize: initialize arc */
static void VIManager_Arc_initialize(VIManager_Arc *a, char *input_event_type, char *input_event_args, char *output_command_type, char *output_command_args, char *variable_action, VIManager_State *next_state)
{
   a->input_event_type = MMDAgent_strdup(input_event_type);
   InputArguments_initialize(&a->input_event_args, input_event_args);
   a->output_command_type = MMDAgent_strdup(output_command_type);
   a->output_command_args = MMDAgent_strdup(output_command_args);
   a->variable_action = MMDAgent_strdup(variable_action);
   a->next_state = next_state;
   a->block_id = 0;
   a->label = NULL;
   a->next = NULL;
}

/* VIManager_Arc_clear: free arc */
static void VIManager_Arc_clear(VIManager_Arc * a)
{
   if (a->input_event_type != NULL)
      free(a->input_event_type);
   InputArguments_clear(&a->input_event_args);
   if (a->output_command_type != NULL)
      free(a->output_command_type);
   if (a->output_command_args != NULL)
      free(a->output_command_args);
   if (a->variable_action != NULL)
      free(a->variable_action);
   if (a->label != NULL)
      free(a->label);
   VIManager_Arc_initialize(a, NULL, NULL, NULL, NULL, NULL, NULL);
}

/* VIManager_AList_initialize: initialize arc list */
static void VIManager_AList_initialize(VIManager_AList *l)
{
   l->head = NULL;
}

/* VIManager_AList_clear: free arc list */
static void VIManager_AList_clear(VIManager_AList *l)
{
   VIManager_Arc *tmp1, *tmp2;

   for (tmp1 = l->head; tmp1 != NULL; tmp1 = tmp2) {
      tmp2 = tmp1->next;
      VIManager_Arc_clear(tmp1);
      free(tmp1);
   }
   l->head = NULL;
}

/* VIManager_State_initialize: initialize state */
static void VIManager_State_initialize(VIManager_State *s, const char *label)
{
   if (label)
      MMDAgent_snprintf(s->label, VIMANAGER_STATE_LABEL_MAXLEN, "%s", label);
   VIManager_AList_initialize(&s->arc_list);
   s->virtual_fromState = NULL;
   s->virtual_toState = NULL;
}

/* VIManager_State_clear: free state */
static void VIManager_State_clear(VIManager_State *s)
{
   VIManager_AList_clear(&s->arc_list);
   VIManager_State_initialize(s, NULL);
}

/* VIManager_SList_initialize: initialize state list */
static void VIManager_SList_initialize(VIManager_SList *l)
{
   l->index.release();
}

/* VIManager_SList_clear: free state list */
static void VIManager_SList_clear(VIManager_SList *l)
{
   VIManager_State *tmp;
   void *save;

   for (tmp = (VIManager_State *)l->index.firstData(&save); tmp; tmp = (VIManager_State *)l->index.nextData(&save)) {
      VIManager_State_clear(tmp);
      free(tmp);
   }
   l->index.release();
}

/* VIManager_SList_count: count state list */
static unsigned int VIManager_SList_count(VIManager_SList *l)
{
   VIManager_State *tmp;
   void *save;
   unsigned int count = 0;

   for (tmp = (VIManager_State *)l->index.firstData(&save); tmp; tmp = (VIManager_State *)l->index.nextData(&save)) {
      count++;
   }

   return count;
}

/* VIManager_SList_searchStateAndCreate: search state pointer, and create if not exist */
static VIManager_State *VIManager_SList_searchStateAndCreate(VIManager_SList *l, const char *label)
{
   VIManager_State *s;
   int len = (int)MMDAgent_strlen(label);

   if (l->index.search(label, len, (void **)&s) == false) {
      s = (VIManager_State *)malloc(sizeof(VIManager_State));
      VIManager_State_initialize(s, label);
      l->index.add(label, len, s);
   }

   return s;
}

/* VIManager_SList_findState: search state pointer */
static VIManager_State *VIManager_SList_findState(VIManager_SList *l, const char *label)
{
   VIManager_State *s;

   if (l->index.search(label, (int)MMDAgent_strlen(label), (void **)&s) == false)
      return NULL;
   return s;
}

/* VIManager_SList_addArc: add arc */
static void VIManager_State_rewriteArc(VIManager_SList *l1, const char *label1, VIManager_SList *l2, const char *label2, VIManager_State *s, unsigned int block_id)
{
   VIManager_State *s1, *s2;

   s1 = VIManager_SList_searchStateAndCreate(l1, label1);
   s2 = VIManager_SList_searchStateAndCreate(l2, label2);

   for (VIManager_Arc *arc = s1->arc_list.head; arc; arc = arc->next) {
      if (arc->next_state && MMDAgent_strequal(arc->next_state->label, s2->label) && arc->block_id == block_id) {
         arc->next_state = s;
      }
   }
}


/* VIManager_SList_addArc: add arc */
static VIManager_Arc *VIManager_SList_addArc(VIManager_SList *l1, const char *label1, VIManager_SList *l2, const char *label2, char *isymbol, char *osymbol, char *vsymbol, unsigned int block_id, unsigned int line_number, const char *label = NULL)
{
   int i, idx;
   VIManager_State *s1, *s2;
   VIManager_Arc *a1, *a2;
   VIManager_AList *arc_list;

   char itype[MMDAGENT_MAXBUFLEN];
   char iargs[MMDAGENT_MAXBUFLEN];
   char otype[MMDAGENT_MAXBUFLEN];
   char oargs[MMDAGENT_MAXBUFLEN];

   s1 = VIManager_SList_searchStateAndCreate(l1, label1);
   s2 = VIManager_SList_searchStateAndCreate(l2, label2);
   arc_list = &s1->arc_list;

   /* analyze input symbol */
   if (MMDAgent_strlen(isymbol) > 1 && isymbol[0] == VIMANAGER_REGEXP_BRACE && isymbol[MMDAgent_strlen(isymbol) - 1] == VIMANAGER_REGEXP_BRACE) {
      /* regular expression, save all isymbol to itype */
      strcpy(itype, isymbol);
      iargs[0] = '\0';
   } else if (MMDAgent_strlen(isymbol) > 1 && isymbol[0] == '$') {
      /* variable test, save all isymbol to itype */
      strcpy(itype, isymbol);
      iargs[0] = '\0';
   } else {
      idx = 0;
      i = getArgFromString(isymbol, &idx, itype, VIMANAGER_SEPARATOR1);
      if (i <= 0)
         return NULL;
      getArgFromString(isymbol, &idx, iargs, '\0');
   }

   /* analyze output symbol */
   idx = 0;
   i = getArgFromString(osymbol, &idx, otype, VIMANAGER_SEPARATOR1);
   if (i <= 0)
      return NULL;
   getArgFromString(osymbol, &idx, oargs, '\0');

   /* create */
   a1 = (VIManager_Arc *) malloc(sizeof(VIManager_Arc));
   VIManager_Arc_initialize(a1, itype, iargs, otype, oargs, vsymbol, s2);
   a1->line_number = line_number;
   a1->block_id = block_id;
   if (label != NULL)
      a1->label = MMDAgent_strdup(label);

   /* set */
   if (arc_list->head == NULL) {
      arc_list->head = a1;
   } else {
      for (a2 = arc_list->head; a2->next != NULL; a2 = a2->next);
      a2->next = a1;
   }

   return a1;
}

/* VIManager_Variable_initialize: initialize variable */
static void VIManager_Variable_initialize(VIManager_Variable *v, const char *name, const char *value)
{
   v->name = MMDAgent_strdup(name);
   v->value = MMDAgent_strdup(value);
   v->next = NULL;
}

/* VIManager_Variable_clear: free variable */
static void VIManager_Variable_clear(VIManager_Variable * v)
{
   if (v->name != NULL)
      free(v->name);
   if (v->value != NULL)
      free(v->value);
}

/* VIManager_VList_initialize: initialize variable list */
static void VIManager_VList_initialize(VIManager_VList *l)
{
   l->index.release();
}

/* VIManager_VList_clear: free variable list */
static void VIManager_VList_clear(VIManager_VList *l)
{
   VIManager_Variable *tmp;
   void *save;

   for (tmp = (VIManager_Variable *)l->index.firstData(&save); tmp; tmp = (VIManager_Variable *)l->index.nextData(&save)) {
      VIManager_Variable_clear(tmp);
      free(tmp);
   }
   l->index.release();
}

/* VIManager_VList_search: search for a variable */
static VIManager_Variable *VIManager_VList_search(VIManager_VList *vlist, const char *name)
{
   VIManager_Variable *v;
   int len;

   len = (int)MMDAgent_strlen(name);

   if (len <= 0)
      return NULL;

   if (vlist->index.search(name, len, (void **)&v) == false)
      return NULL;
   return v;
}

/* VIManager_VList_set: set variable to list */
static void VIManager_VList_set(VIManager_VList *vlist, const char *name, const char *value)
{
   VIManager_Variable *v;

   /* search */
   v = VIManager_VList_search(vlist, name);
   if (v != NULL) {
      /* replace */
      if (v->value != NULL)
         free(v->value);
      v->value = MMDAgent_strdup(value);
   } else {
      /* create */
      v = (VIManager_Variable *)malloc(sizeof(VIManager_Variable));
      VIManager_Variable_initialize(v, name, value);
      /* set */
      vlist->index.add(name, (int)MMDAgent_strlen(name), v);
   }
}

/* VIManager::initialize: initialize VIManager */
void VIManager::initialize()
{
   m_mmdagent = NULL;
   m_id = 0;
   m_fileName = NULL;
   m_name = NULL;
   VIManager_SList_initialize(&m_stateList);
   VIManager_SList_initialize(&m_stateListAppend);
   VIManager_VList_initialize(&m_variableList);
   m_currentState = NULL;
   m_append_num = 0;
   m_block_id = 0;
   for (int i = 0; i < VIMANAGER_HISTORY_LEN; i++)
      m_history[i] = NULL;
   m_historyPoint = 0;
   m_end = false;
}

/* VIManager:clear: free VIManager */
void VIManager::clear()
{
   if (m_name != NULL)
      free(m_name);
   if (m_fileName != NULL)
      free(m_fileName);
   VIManager_SList_clear(&m_stateList);
   VIManager_SList_clear(&m_stateListAppend);
   VIManager_VList_clear(&m_variableList);
   initialize();
}

/* VIManager::substituteVariableAndCopy: substitute variables with their values in a string */
void VIManager::substituteVariableAndCopy(const char *input, char *output)
{
   int i, len, vlen;
   const char *c;
   char *out, *cv;
   unsigned char size;
   int braced_counter;
   VIManager_Variable *v;

   len = (int)MMDAgent_strlen(input);
   if (len <= 0) {
      *output = '\0';
      return;
   }

   c = input;
   out = output;
   for (i = 0; i < len; i += size) {
      size = MMDAgent_getcharsize(c);
      if(size == 0) { /* fail safe */
         *out = '\0';
         return;
      }
      memcpy(out, c, size);
      if (size == 1 && *c == '$') {
         /* variable start, hold this place to cv and read variable name */
         cv = out;
         c += size;
         out += size;
         i += size;
         if (MMDAgent_getcharsize(c) == 1 && *c == '$') {
            /* "$$" -> "$" */
            c += 1;
            size = 1;
            continue;
         }
         braced_counter = 0;
         if (MMDAgent_getcharsize(c) == 1 && *c == '{') {
            braced_counter++;
            c += 1;
            i += 1;
         }
         for (; i < len; i += size) {
            size = MMDAgent_getcharsize(c);
            if(size == 0) { /* fail safe */
               *out = '\0';
               return;
            }
            if (MMDAgent_getcharsize(c) == 1 && *c == '{') {
               braced_counter++;
            } else if (MMDAgent_getcharsize(c) == 1 && *c == '}') {
               braced_counter--;
               if (braced_counter == 0) {
                  c += 1;
                  i += 1;
                  break;
               }
            } else {
               if (size != 1 || ! (isalnum(*c) || *c == '_' || *c == '%')) break;
            }
            memcpy(out, c, size);
            c += size;
            out += size;
         }
         *out = '\0';
         /* search for variable */
         if (MMDAgent_strstr(cv + 1, "%ENV")) {
            /* consult environmental variables */
            v = NULL;
            out = cv;
            char replaced_str[MMDAGENT_MAXBUFLEN];
            m_mmdagent->sendLogString(m_id, MLOG_STATUS, "%s", cv + 1);
            if (MMDAgent_replaceEnvDup(cv + 1, replaced_str) >= 0) {
               m_mmdagent->sendLogString(m_id, MLOG_STATUS, "%s: get \"%s\": \"%s\"", m_name, cv + 1, replaced_str);
               vlen = (int)MMDAgent_strlen(replaced_str);
               memcpy(out, replaced_str, vlen);
               out += vlen;
            } else {
               m_mmdagent->sendLogString(m_id, MLOG_STATUS, "%s: \"%s\": not found", m_name, cv + 1);
            }
         } else if (*(cv + 1) == '%') {
            /* consult global variable in KeyValue */
            v = NULL;
            /* rewind out buf to the start of variable name */
            out = cv;
            const char *p = m_mmdagent->getKeyValue()->getString(cv + 2, NULL);
            if (p) {
               m_mmdagent->sendLogString(m_id, MLOG_STATUS, "%s: get KeyValue of \"%s\": \"%s\"", m_name, cv + 2, p);
               vlen = (int)MMDAgent_strlen(p);
               memcpy(out, p, vlen);
               out += vlen;
            } else {
               m_mmdagent->sendLogString(m_id, MLOG_STATUS, "%s: KeyValue \"%s\": not found", m_name, cv + 2);
            }
         } else {
            v = VIManager_VList_search(&m_variableList, cv + 1);
            /* rewind out buf to the start of variable name */
            out = cv;
            /* overwrite with its value */
            if (v != NULL) {
               vlen = (int)MMDAgent_strlen(v->value);
               memcpy(out, v->value, vlen);
               out += vlen;
            }
         }
         size = 0;
      } else {
         c += size;
         out += size;
      }
   }
   *out = '\0';
}

/* VIManager::checkStringMatch: check if vstr with variables matches the string */
bool VIManager::checkStringMatch(const char *vstr, const char *str)
{
   char buf1[MMDAGENT_MAXBUFLEN];

   if (vstr == NULL || str == NULL)
      return false;

   /* substitute variables in pattern */
   substituteVariableAndCopy(vstr, buf1);

   return MMDAgent_strequal(str, buf1);
}

/* VIManager::checkStringMatchRegExp: check if vstr with variables matches the string as regular expression */
bool VIManager::checkStringMatchRegExp(const char *vstr, const char *str1, const char *str2)
{
   char buf1[MMDAGENT_MAXBUFLEN];
   char buf2[MMDAGENT_MAXBUFLEN];
   int i, n;
   bool match;

   if (vstr == NULL || str1 == NULL)
      return false;

   /* substitute variables in pattern */
   substituteVariableAndCopy(vstr, buf1);

   /* set target pattern */
   if (str2 != NULL)
      MMDAgent_snprintf(buf2, MMDAGENT_MAXBUFLEN, "%s%c%s", str1, VIMANAGER_SEPARATOR1, str2);
   else
      strcpy(buf2, str1);

   /* create re2 instance with given pattern */
   RE2 re(buf1);

   if (re.ok() == false)
      return MMDAgent_strequal(buf2, buf1);

   /* get number of match argument to be taken */
   n = re.NumberOfCapturingGroups();
   if (n == 0)
      return RE2::FullMatch(buf2, re);

   std::string *result = new std::string[n];
   RE2::Arg *arg = new RE2::Arg[n];
   RE2::Arg **argp = new RE2::Arg*[n];
   for (i = 0; i < n; i++) {
      arg[i] = &result[i];
      argp[i] = &arg[i];
   }
   match = RE2::FullMatchN(buf2, re, &(argp[0]), n);
   if (match == true) {
      /* set numeric variables */
      for (i = 0; i < n; i++) {
         MMDAgent_snprintf(buf2, MMDAGENT_MAXBUFLEN, "%d", i + 1);
         VIManager_VList_set(&m_variableList, buf2, result[i].c_str());
         m_mmdagent->sendLogString(m_id, MLOG_STATUS, "%s: $%s=%s", m_name, buf2, result[i].c_str());
      }
   }
   delete[] result;
   delete[] arg;
   delete[] argp;

   return match;
}

/* VIManager::checkVariableTest: check if variable expression is true */
bool VIManager::checkVariableTest(const char *vstr, bool *result)
{
   char buff[MMDAGENT_MAXBUFLEN];
   int len;

   if (vstr == NULL)
      return false;

   /* substitute variables in pattern */
   substituteVariableAndCopy(vstr, buff);

   len = (int)MMDAgent_strlen(buff);
   for (int i = 0; i < len - 1; i++) {
      if (buff[i] == '!' && buff[i + 1] == '=') {
         buff[i] = buff[i + 1] = '\0';
         if (result)
            *result = ! MMDAgent_strequal(&(buff[0]), &(buff[i + 2]));
         return true;
      } else if (buff[i] == '=' && buff[i + 1] == '=') {
         buff[i] = buff[i + 1] = '\0';
         if (result)
            *result = MMDAgent_strequal(&(buff[0]), &(buff[i + 2]));
         return true;
      }
   }

   if (result)
      *result = false;
   return false;
}

/* VIManager::checkArcMatch: check if an arc matches an input */
bool VIManager::checkArcMatch(const char *arc_type, const char *input_type, const InputArguments *arc_arg, const InputArguments *input_arg)
{
   char buf[MMDAGENT_MAXBUFLEN];
   int i, j, k;
   bool match;

   if (MMDAgent_strlen(arc_type) > 1 && arc_type[0] == VIMANAGER_REGEXP_BRACE && arc_type[MMDAgent_strlen(arc_type) - 1] == VIMANAGER_REGEXP_BRACE) {
      /* regular expression match */
      strcpy(buf, &(arc_type[1]));
      buf[MMDAgent_strlen(buf) - 1] = '\0';
      if (input_arg != NULL)
         return checkStringMatchRegExp(buf, input_type, input_arg->str);
      else
         return checkStringMatchRegExp(buf, input_type, NULL);
   }

   if (MMDAgent_strlen(arc_type) > 1 && arc_type[0] == '$') {
      /* variable test */
      checkVariableTest(arc_type, &match);
      m_mmdagent->sendLogString(m_id, MLOG_STATUS, "%s: test: %s: result = %s", m_name, arc_type, match ? "true" : "false");
      return match;
   }

   if (checkStringMatch(arc_type, input_type) == false)
      return false;

   if (arc_arg == NULL)
      return false;
   if (input_arg == NULL) {
      if (arc_arg->size == 0)
         return true;
      else
         return false;
   }
   if (arc_arg->size != input_arg->size)
      return false;

   for (i = 0; i < arc_arg->size; i++) {
      for (j = 0; j < arc_arg->argc[i]; j++) {
         match = false;
         for (k = 0; k < input_arg->argc[i]; k++) {
            if (checkStringMatch(arc_arg->args[i][j], input_arg->args[i][k]) == true) {
               match = true;
               break;
            }
         }
         if (match == false)
            return false;
      }
   }
   return true;
}

/* VIManager::assignVariableByEquation: assign variable by equation */
bool VIManager::assignVariableByEquation(const char *va)
{
   int idx1, idx2;
   int s, len;
   char tok[2];
   char buff[MMDAGENT_MAXBUFLEN];
   char buffn[MMDAGENT_MAXBUFLEN];
   char buffv[MMDAGENT_MAXBUFLEN];
   char buffvr[MMDAGENT_MAXBUFLEN];

   if (MMDAgent_strlen(va) == 0)
      return true;

   tok[0] = VIMANAGER_SEPARATOR2;
   tok[1] = '\0';

   idx1 = 0;
   while (getTokenFromStringWithQuoters(va, &idx1, buff, tok, "\"'", true) > 0) {
      /* separate by equal */
      if (countArgs(buff, '=') != 2)
         return false;
      idx2 = 0;
      getArgFromString(buff, &idx2, buffn, '=');
      if (buffn[0] != '$')
         return false;
      s = 1;
      if (buffn[1] == '{') {
         len = (int)MMDAgent_strlen(buffn);
         if (buffn[len - 1] != '}')
            return false;
         buffn[len - 1] = '\0';
         /* check if variable name is valid */
         if (checkVariableName(&(buffn[2])) == false)
            return false;
         s = 2;
      }
      getArgFromString(buff, &idx2, buffv, '=');
      substituteVariableAndCopy(buffv, buffvr);
      if (buffn[s] == '%') {
         m_mmdagent->getKeyValue()->setString(&(buffn[s + 1]), "%s", buffvr);
         m_mmdagent->sendLogString(m_id, MLOG_STATUS, "%s: set KeyValue \"%s\" to \"%s\"", m_name, &(buffn[s + 1]), buffvr);
      } else {
         VIManager_VList_set(&m_variableList, &(buffn[s]), buffvr);
      }
      m_mmdagent->sendLogString(m_id, MLOG_STATUS, "%s: $%s=%s", m_name, &(buffn[s]), buffvr);
   }
   return true;
}

/* VIManager::loadFSTFile: load fst file */
bool VIManager::loadFSTFile(ZFileKey *key, const char *file, unsigned int *arc_count_ret, int depth, const char *label)
{
   ZFile *zf;
   int line;
   unsigned int arc_count = 0;
   bool top;
   bool within_block;

   char buff[MMDAGENT_MAXBUFLEN];
   char buff_s1[MMDAGENT_MAXBUFLEN];
   char buff_s2[MMDAGENT_MAXBUFLEN];
   char buff_is[MMDAGENT_MAXBUFLEN];
   char buff_os[MMDAGENT_MAXBUFLEN];
   char buff_vs[MMDAGENT_MAXBUFLEN];
   char buff_er[MMDAGENT_MAXBUFLEN];
   int size_s1;
   int size_s2;
   int size_is;
   int size_os;
   int size_vs;
   int size_er;

   VIManager_Arc *arc;
   VIManager_SList *last_state_list;
   char buff_last_os[MMDAGENT_MAXBUFLEN];
   char last_state_label[VIMANAGER_STATE_LABEL_MAXLEN];
   char label1[VIMANAGER_STATE_LABEL_MAXLEN];
   char label2[VIMANAGER_STATE_LABEL_MAXLEN];
   char appendlabel[VIMANAGER_STATE_LABEL_MAXLEN];
   bool err;
   int len;
   int idx;
   bool ret = true;

   for (idx = 0; idx < depth; idx++) {
      buff[idx] = '>';
   }
   buff[depth] = '\0';
   m_mmdagent->sendLogString(m_id, MLOG_STATUS, "%sloading FST \"%s\"", buff, file);

   /* open */
   zf = new ZFile(key);
   if (zf->openAndLoad(file) == false) {
      m_mmdagent->sendLogString(m_id, MLOG_ERROR, "cannot open \"%s\"", file);
      delete zf;
      *arc_count_ret = 0;
      return false;
   }

   /* prepare */
   line = 1;
   top = true;
   within_block = false;
   last_state_list = NULL;
   last_state_label[0] = '\0';
   while (zf->gets(buff, MMDAGENT_MAXBUFLEN - 3) != NULL) { /* string + \r + \n + \0 */
      /* remove final \n and \r */
      len = (int)MMDAgent_strlen(buff);
      while (len > 0 && (buff[len - 1] == '\n' || buff[len - 1] == '\r'))
         buff[--len] = '\0';
      /* skip head spaces and tabs */
      idx = 0;
      while (idx < len && (buff[idx] == ' ' || buff[idx] == '\t'))
         idx++;

      /* check and load arc */
      err = false;
      if (idx >= len) {
         /* blank line */
         arc = NULL;
         within_block = false;
      } else {
         if (buff[idx] == '$' && top == true) {
            /* variable definitions at the top */
            if (assignVariableByEquation(&(buff[idx])) == false)
               m_mmdagent->sendLogString(m_id, MLOG_ERROR, "%s: line %d: failed to set variable: \"%s\"", file, line, buff_s1);
            arc = NULL;
            within_block = false;
         } else if (buff[idx] == '%') {
            /* possibly meta directive */
            if (MMDAgent_strheadmatch(buff + idx, "%INCLUDE")) {
               /* recursive call to include other fst file at the line */
               unsigned int arc_count_sub = 0;
               idx += 8;
               size_s1 = getTokenFromStringWithQuoters(buff, &idx, buff_s1, "() \t\n\r", "\"'", true);
               if (size_s1 > 0 && loadFSTFile(key, buff_s1, &arc_count_sub, depth + 1, buff_s1) == false)
                  err = true;
               arc_count += arc_count_sub;
               /* break history and blocks */
               arc = NULL;
               within_block = false;
               last_state_list = NULL;
               last_state_label[0] = '\0';
            }
         } else if (buff[idx] == VIMANAGER_COMMENT) {
            /* comment: do nothing */
         } else {
            top = false;
            if (idx == 0) {
               /* non-indented line: a state definition */
               m_block_id++;
               size_s1 = getTokenFromString(buff, &idx, buff_s1, true);
               size_s2 = getTokenFromString(buff, &idx, buff_s2, true);
               size_is = getTokenFromString(buff, &idx, buff_is, true);
               size_os = getTokenFromString(buff, &idx, buff_os, true);
               size_vs = getTokenFromString(buff, &idx, buff_vs, false);
               size_er = getTokenFromString(buff, &idx, buff_er, true);
               if (size_s1 > 0 && size_s2 > 0 && size_is == 0) {
                  /* only state numbers: beginning of a block */
                  if (buff_s2[size_s2 - 1] != ':') {
                     m_mmdagent->sendLogString(m_id, MLOG_ERROR, "%s: line %d: number line has no colon", file, line);
                     err = true;
                  } else {
                     buff_s2[size_s2 - 1] = '\0';
                     MMDAgent_snprintf(label1, VIMANAGER_STATE_LABEL_MAXLEN, "%s", buff_s1);
                     MMDAgent_snprintf(label2, VIMANAGER_STATE_LABEL_MAXLEN, "%s", buff_s2);
                  }
                  arc = NULL;
                  within_block = true;
                  last_state_list = NULL;
                  last_state_label[0] = '\0';
               } else if (size_s1 > 0 && size_s2 > 0 && size_is > 0 && size_os > 0 && size_er == 0) {
                  /* full arc definition in a line */
                  MMDAgent_snprintf(label1, VIMANAGER_STATE_LABEL_MAXLEN, "%s", buff_s1);
                  MMDAgent_snprintf(label2, VIMANAGER_STATE_LABEL_MAXLEN, "%s", buff_s2);
                  if (buff_is[0] == '$') {
                     if (checkVariableTest(buff_is, NULL) == false) {
                        m_mmdagent->sendLogString(m_id, MLOG_ERROR, "%s: line %d: format error in variable test: %s", file, line, buff_is);
                        err = true;
                     }
                  }
                  arc = VIManager_SList_addArc(&m_stateList, label1, &m_stateList, label2, buff_is, buff_os, buff_vs, m_block_id, line, label);
                  if (arc == NULL)
                     err = true;
                  else
                     arc_count++;
                  last_state_list = &m_stateList;
                  strcpy(last_state_label, label1);
                  strcpy(buff_last_os, buff_os);
                  within_block = false;
               } else {
                  /* error */
                  err = true;
                  arc = NULL;
                  within_block = false;
               }
            } else {
               /* indented line: successive state definition in a block */
               if (within_block == false) {
                  m_mmdagent->sendLogString(m_id, MLOG_ERROR, "%s: line %d: not in a block definition", file, line);
                  err = true;
               } else {
                  size_is = getTokenFromString(buff, &idx, buff_is, true);
                  size_os = getTokenFromString(buff, &idx, buff_os, true);
                  size_vs = getTokenFromString(buff, &idx, buff_vs, false);
                  size_er = getTokenFromString(buff, &idx, buff_er, true);
                  if (buff_is[0] == '+') {
                     /* append line in block */
                     if (buff_is[1] == '$') {
                        if (checkVariableTest(&(buff_is[1]), NULL) == false) {
                           m_mmdagent->sendLogString(m_id, MLOG_ERROR, "%s: line %d: format error in variable test: %s", file, line, buff_is);
                           err = true;
                        }
                     }
                     if (arc == NULL) {
                        m_mmdagent->sendLogString(m_id, MLOG_ERROR, "%s: line %d: '+' should not be at the top of block: %s", file, line, buff_is);
                        err = true;
                     } else {
                        if (last_state_list == NULL) {
                           m_mmdagent->sendLogString(m_id, MLOG_ERROR, "%s: line %d: internal error: %s", file, line, buff_is);
                           err = true;
                        } else {
                           if (size_is > 0 && size_os > 0 && size_er == 0) {
                              arc = VIManager_SList_addArc(last_state_list, last_state_label, &m_stateList, label2, &(buff_is[1]), buff_os, buff_vs, m_block_id, line, label);
                           } else if (size_is > 0 && size_er == 0) {
                              arc = VIManager_SList_addArc(last_state_list, last_state_label, &m_stateList, label2, &(buff_is[1]), buff_last_os, buff_vs, m_block_id, line, label);
                           }
                           if (arc == NULL)
                              err = true;
                           else
                              arc_count++;
                        }
                     }
                  } else {
                     /* definition line in block */
                     if (size_is > 0 && size_os > 0 && size_er == 0) {
                        if (buff_is[0] == '$') {
                           if (checkVariableTest(buff_is, NULL) == false) {
                              m_mmdagent->sendLogString(m_id, MLOG_ERROR, "%s: line %d: format error in variable test: %s", file, line, buff_is);
                              err = true;
                           }
                        }
                        if (arc == NULL) {
                           arc = VIManager_SList_addArc(&m_stateList, label1, &m_stateList, label2, buff_is, buff_os, buff_vs, m_block_id, line, label);
                           last_state_list = &m_stateList;
                           strcpy(last_state_label, label1);
                           strcpy(buff_last_os, buff_os);
                        } else {
                           MMDAgent_snprintf(appendlabel, VIMANAGER_STATE_LABEL_MAXLEN, "%09u", m_append_num);
                           VIManager_State *s = VIManager_SList_searchStateAndCreate(&m_stateListAppend, appendlabel);
                           s->virtual_fromState = VIManager_SList_findState(&m_stateList, label1);
                           s->virtual_toState = VIManager_SList_findState(&m_stateList, label2);
                           VIManager_State_rewriteArc(last_state_list, last_state_label, &m_stateList, label2, s, m_block_id);
                           arc = VIManager_SList_addArc(&m_stateListAppend, appendlabel, &m_stateList, label2, buff_is, buff_os, buff_vs, m_block_id, line, label);
                           last_state_list = &m_stateListAppend;
                           strcpy(last_state_label, appendlabel);
                           strcpy(buff_last_os, buff_os);
                           m_append_num++;
                        }
                     }
                     if (arc == NULL)
                        err = true;
                     else
                        arc_count++;
                  }
               }
            }
         }
      }
      if (err == true) {
         m_mmdagent->sendLogString(m_id, MLOG_ERROR, "%s: wrong format at line %d", file, line);
         ret = false;
      }
      if (zf->eof() || zf->error())
         break;
      line++;
   }

   zf->close();
   delete zf;

   *arc_count_ret = arc_count;

   if (m_fileName)
      free(m_fileName);
   m_fileName = MMDAgent_strdup(file);

   return ret;
}


/* VIManager::VIManager: constructor */
VIManager::VIManager()
{
   initialize();
}

/* VIManager::~VIManager: destructor */
VIManager::~VIManager()
{
   clear();
}

/* VIManager::load: load FST */
bool VIManager::load(MMDAgent *mmdagent, int id, ZFileKey *key, const char *file, const char *name)
{
   unsigned int arc_count;
   unsigned int arc_count_total;
   bool ret = true;

   m_mmdagent = mmdagent;
   m_id = id;
   if (m_name != NULL)
      free(m_name);
   m_name = MMDAgent_strdup(name);

   /* unload */
   VIManager_SList_clear(&m_stateList);
   VIManager_SList_initialize(&m_stateList);
   VIManager_SList_clear(&m_stateListAppend);
   VIManager_SList_initialize(&m_stateListAppend);
   m_append_num = 0;

   /* load FST */
   arc_count_total = 0;
   m_block_id = 0;
   if (loadFSTFile(key, file, &arc_count, 0, NULL) == true) {
      m_mmdagent->sendLogString(m_id, MLOG_STATUS, "FST %s \"%s\"", name, file);
      arc_count_total += arc_count;
   } else {
      ret = false;
   }

   if (ret == true) {
      unsigned int c1, c2;
      c1 = VIManager_SList_count(&m_stateList);
      c2 = VIManager_SList_count(&m_stateListAppend);
      m_mmdagent->sendLogString(m_id, MLOG_STATUS, "%u arcs, %u states", arc_count_total, c1 + c2);
   }

   m_end = false;

   return ret;
}

static void dispNum(char *buff, int bufflen, VIManager_State *s1, VIManager_State *s2)
{
   if (s1->virtual_fromState && s2->virtual_fromState)
      MMDAgent_snprintf(buff, bufflen, "[%s -> %s]", s1->virtual_fromState->label, s2->virtual_toState->label);
   else if (s1->virtual_fromState)
      MMDAgent_snprintf(buff, bufflen, "[%s ->] %s", s1->virtual_fromState->label, s2->label);
   else if (s2->virtual_fromState)
      MMDAgent_snprintf(buff, bufflen, "%s [-> %s]", s1->label, s2->virtual_toState->label);
   else
      MMDAgent_snprintf(buff, bufflen, "%s %s", s1->label, s2->label);
}

/* VIManager::transition: state transition (if jumped, return arc) */
bool VIManager::transition(const char *itype, const InputArguments *iargs, char *otype, char *oargs)
{
   VIManager_Arc *arc;
   VIManager_AList *arc_list;
   char buff[128];

   strcpy(otype, VIMANAGER_EPSILON);
   strcpy(oargs, "");

   /* FST isn't loaded yet */
   if (m_currentState == NULL)
      return false;

   /* state don't have arc list */
   arc_list = &m_currentState->arc_list;
   if (arc_list->head == NULL) {
      m_end = true;
      return false;
   }

   /* match */
   for (arc = arc_list->head; arc != NULL; arc = arc->next) {
      /* check if input matches this arc, consulting variables and wildcards */
      if (checkArcMatch(arc->input_event_type, itype, &arc->input_event_args, iargs)) {
         dispNum(buff, 128, m_currentState, arc->next_state);
         if (MMDAgent_strequal(itype, VIMANAGER_EPSILON) == false) {
            if (iargs == NULL || iargs->str == NULL)
               m_mmdagent->sendLogString(m_id, MLOG_MESSAGE_CAPTURED, "%s", itype);
            else
               m_mmdagent->sendLogString(m_id, MLOG_MESSAGE_CAPTURED, "%s|%s", itype, iargs->str);
         }
         if (arc->input_event_args.str == NULL) {
            if (arc->output_command_args == NULL) {
               m_mmdagent->sendLogString(m_id, MLOG_STATUS, "[%s:%u] %s %s %s %s", arc->label ? arc->label : m_name, arc->line_number, buff, arc->input_event_type, arc->output_command_type, arc->variable_action);
            } else {
               m_mmdagent->sendLogString(m_id, MLOG_STATUS, "[%s:%u] %s %s %s|%s %s", arc->label ? arc->label : m_name, arc->line_number, buff, arc->input_event_type, arc->output_command_type, arc->output_command_args, arc->variable_action);
            }
         } else {
            if (arc->output_command_args == NULL) {
               m_mmdagent->sendLogString(m_id, MLOG_STATUS, "[%s:%u] %s %s|%s %s %s", arc->label ? arc->label : m_name, arc->line_number, buff, arc->input_event_type, arc->input_event_args.str, arc->output_command_type, arc->variable_action);
            } else {
               m_mmdagent->sendLogString(m_id, MLOG_STATUS, "[%s:%u] %s %s|%s %s|%s %s", arc->label ? arc->label : m_name, arc->line_number, buff, arc->input_event_type, arc->input_event_args.str, arc->output_command_type, arc->output_command_args, arc->variable_action);
            }
         }
         /* set output string, consulting variables if any */
         substituteVariableAndCopy(arc->output_command_type, otype);
         substituteVariableAndCopy(arc->output_command_args, oargs);
         /* if this arc has variable action, execute it */
         assignVariableByEquation(arc->variable_action);
         /* move to next state */
         m_currentState = arc->next_state;
         /* check if the next state has arc */
         if (m_currentState->arc_list.head == NULL) {
            if (m_currentState->virtual_fromState)
               m_mmdagent->sendLogString(m_id, MLOG_ERROR, "[%s] a state between [%s - %s]: no next arc?", arc->label ? arc->label : m_name, m_currentState->virtual_fromState->label, m_currentState->virtual_toState->label);
            else
               m_mmdagent->sendLogString(m_id, MLOG_STATUS, "[%s] now reached state #%s which has no arc", arc->label ? arc->label : m_name, m_currentState->label);
         }
         /* save to history */
         {
            std::lock_guard<std::mutex> lock(m_mutexHistory);
            m_history[m_historyPoint] = arc;
            m_historyPoint++;
            if (m_historyPoint >= VIMANAGER_HISTORY_LEN)
               m_historyPoint -= VIMANAGER_HISTORY_LEN;
         }
         return true;
      }
   }

   return false;
}

/* VIManager::setCurrentState: set current state */
bool VIManager::setCurrentState(const char *label)
{
   VIManager_State *state;

   state = VIManager_SList_findState(&m_stateList, label);
   if (state == NULL) {
      m_mmdagent->sendLogString(m_id, MLOG_ERROR, "[%s] state \"%s\" does not exist in FST", m_name, label);
      return false;
   }
   m_currentState = state;
   m_mmdagent->sendLogString(m_id, MLOG_STATUS, "[%s] current state set to \"%s\"", m_name, label);
   return true;
}

/* VIManager::getCurrentState: get current state */
VIManager_State *VIManager::getCurrentState()
{
   return m_currentState;
}

/* VIManager::getCurrentVariableList: get current variable list */
VIManager_VList *VIManager::getCurrentVariableList()
{
   return &m_variableList;
}

/* VIManager::getName: get name */
const char *VIManager::getName()
{
   return m_name;
}

/* VIManager::getFileName: get file name */
const char *VIManager::getFileName()
{
   return m_fileName;
}


/* VIManager::getTransitionHistory: get allocated list of transition history */
int VIManager::getTransitionHistory(VIManager_Arc **list, int maxlen)
{
   int src;
   int len;
   int ret;

   {
      std::lock_guard<std::mutex> lock(m_mutexHistory);
      if (m_history[m_historyPoint] == NULL) {
         len = m_historyPoint;
         src = 0;
      } else {
         len = VIMANAGER_HISTORY_LEN;
         src = m_historyPoint;
      }
      if (maxlen < len) {
         ret = -1;
      } else {
         for (int i = 0; i < len; i++) {
            list[i] = m_history[src];
            src++;
            if (src >= VIMANAGER_HISTORY_LEN)
               src -= VIMANAGER_HISTORY_LEN;
         }
         ret = len;
      }
   }

   return ret;
}

/* VIManager::getEndFlag: get end flag */
bool VIManager::getEndFlag()
{
   return m_end;
}

/* VIManager::jumpToState: jump to the state if exist */
bool VIManager::jumpToState(const char *state_label)
{
   VIManager_State *state;

   state = VIManager_SList_findState(&m_stateList, state_label);
   if (state == NULL)
      return false;
   m_currentState = state;
   return true;
}
