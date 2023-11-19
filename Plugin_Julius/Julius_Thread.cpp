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

#include "MMDAgent.h"
#include "julius/juliuslib.h"
#include "Julius_Logger.h"
#include "Julius_Record.h"
#include "Julius_Thread.h"

/* callbackRecogBegin: callback for beginning of recognition */
static void callbackRecogBegin(Recog *recog, void *data)
{
   Julius_Thread *j = (Julius_Thread *) data;
   j->sendMessage(JULIUSTHREAD_EVENTSTART, "");
}

/* callbackRecogResult: callback for recognition result */
static void callbackRecogResult(Recog *recog, void *data)
{
   Julius_Thread *j = (Julius_Thread *) data;
   j->procResult();
}

/* callbackRecogResult: callback for GMM result */
static void callbackGMMResult(Recog *recog, void *data)
{
   Julius_Thread *j = (Julius_Thread *)data;
   j->procGMMResult();
}

/* callbackPoll: callback called per about 0.1 sec during audio input */
static void callbackPoll(Recog *recog, void *data)
{
   Julius_Thread *j = (Julius_Thread *) data;
   j->procCommand();
}

/* callbackOnLine: callback for start of processing */
static void callbackOnLine(Recog *recog, void *data)
{
   Julius_Thread *j = (Julius_Thread *) data;
   j->sendMessage(MMDAGENT_EVENT_PLUGINENABLE, JULIUSTHREAD_PLUGINNAME);
}

/* callbackOnLine: callback for input parameter status */
static void callbackParam(Recog *recog, void *data)
{
   Julius_Thread *j = (Julius_Thread *)data;
   j->procParam();
}

#ifdef ENABLE_RAPID
/* callbackRapid: callback for rapid determination */
static void callbackRapid(Recog *recog, void *data)
{
   Julius_Thread *j = (Julius_Thread *)data;
   j->procRapid();
}
#endif

/* mainThread: main thread */
static void mainThread(void *param)
{
   Julius_Thread *julius_thread = (Julius_Thread *) param;
   julius_thread->run();
}

/* Julius_Thread::initialize: initialize thread */
void Julius_Thread::initialize()
{
   m_jconf = NULL;
   m_recog = NULL;

   m_mmdagent = NULL;
   m_id = 0;

   m_mutex = NULL;
   m_thread = -1;

   m_configFile = NULL;
   m_userConfigFile = NULL;
   m_userDictionary = NULL;
#ifdef ENABLE_RAPID
   m_userRapidWordDictionary = NULL;
#endif
   m_zf = NULL;

   m_status = JULIUSTHREAD_STATUSWAIT;
   m_command = NULL;
   m_logger = NULL;
   m_recorder = NULL;

   m_boostFactor = JULIUSTHREAD_PREDICTWORD_BOOSTFACTOR_DEFAULT;

   m_currentGain = 1.0f;
   m_pausing = false;
   m_wordspacing[0] = '\0';
}

/* Julius_Thread::clear: free thread */
void Julius_Thread::clear()
{
   JuliusModificationCommand *c, *next;

   if(m_thread >= 0) {
      if(m_recog)
         j_close_stream(m_recog);
      glfwWaitThread(m_thread, GLFW_WAIT);
      glfwDestroyThread(m_thread);
   }
   if(m_mutex != NULL)
      glfwDestroyMutex(m_mutex);
   if (m_recog)
      j_recog_free(m_recog); /* jconf is also released in j_recog_free */
   else if (m_jconf)
      j_jconf_free(m_jconf);

#ifdef ENABLE_RAPID
   if (m_userRapidWordDictionary != NULL)
      free(m_userRapidWordDictionary);
#endif
   if(m_userDictionary != NULL)
      free(m_userDictionary);
   if (m_userConfigFile != NULL)
      free(m_userConfigFile);
   if (m_configFile != NULL)
      free(m_configFile);
   if (m_zf)
      delete m_zf;

   for(c = m_command; c != NULL; c = next) {
      next = c->next;
      free(c->str);
      free(c);
   }

   if (m_logger)
      delete m_logger;
   if (m_recorder)
      delete m_recorder;

   initialize();
}

/* Julius_Thread::Julius_Thread: thread constructor */
Julius_Thread::Julius_Thread()
{
   initialize();
}

/* Julius_Thread::~Julius_Thread: thread destructor */
Julius_Thread::~Julius_Thread()
{
   clear();
}

/* Julius_Thread::loadAndStart: load models and start thread */
void Julius_Thread::loadAndStart(MMDAgent *mmdagent, int id, const char *configFile, const char *userConfigFile, const char *userDictionary
#ifdef ENABLE_RAPID
                                 , const char *userRapidWordDictionary
#endif
)
{
   /* reset */
   clear();

   m_mmdagent = mmdagent;
   m_id = id;

   m_configFile = MMDAgent_strdup(configFile);
   if (userConfigFile)
      m_userConfigFile = MMDAgent_strdup(userConfigFile);
   if (userDictionary)
      m_userDictionary = MMDAgent_strdup(userDictionary);
#ifdef ENABLE_RAPID
   m_userRapidWordDictionary = MMDAgent_strdup(userRapidWordDictionary);
#endif

   if(m_mmdagent == NULL || m_configFile == NULL) {
      clear();
      return;
   }

   /* create recognition thread */
   glfwInit();
   m_mutex = glfwCreateMutex();
   m_thread = glfwCreateThread(mainThread, this);
   if(m_mutex == NULL || m_thread < 0) {
      clear();
      return;
   }
}

/* Julius_Thread::stopAndRelease: stop thread and release julius */
void Julius_Thread::stopAndRelease()
{
   clear();
}

/* Julius_Thread::run: main loop */
void Julius_Thread::run()
{
   char *tmp;
   char buff[MMDAGENT_MAXBUFLEN];
   ZFileKey *key;
   ZFile *zf;
   char *s;

   if(m_jconf != NULL || m_recog != NULL || m_mmdagent == NULL || m_thread < 0 || m_configFile == NULL)
      return;

   /* set latency */
#if !defined(__APPLE__) && !defined(__ANDROID__)
   MMDAgent_snprintf(buff, MMDAGENT_MAXBUFLEN, "PA_MIN_LATENCY_MSEC=%d", JULIUSTHREAD_LATENCY);
   putenv(buff);
   MMDAgent_snprintf(buff, MMDAGENT_MAXBUFLEN, "LATENCY_MSEC=%d", JULIUSTHREAD_LATENCY);
   putenv(buff);
#endif

   /* log output */
   FILE *logfp = NULL;
   if (m_mmdagent->getKeyValue()->exist(PLUGINJULIUS_LOG_FILE)) {
      const char *filename = m_mmdagent->getKeyValue()->getString(PLUGINJULIUS_LOG_FILE, NULL);
      if (MMDAgent_strlen(filename) > 0) {
         m_mmdagent->sendLogString(m_id, MLOG_STATUS, "saving Julius log to %s", filename);
         logfp = MMDAgent_fopen(filename, "w");
      }
   }
   jlog_set_output(logfp);

   /* check SIMD availability */
   get_builtin_simd_string(buff);
   m_mmdagent->sendLogString(m_id, MLOG_STATUS, "built-in SIMD instruction set: %s", buff);
   switch (check_avail_simd()) {
   case USE_SIMD_SSE:
      m_mmdagent->sendLogString(m_id, MLOG_STATUS, "use SSE");
      break;
   case USE_SIMD_AVX:
      m_mmdagent->sendLogString(m_id, MLOG_STATUS, "use AVX");
      break;
   case USE_SIMD_FMA:
      m_mmdagent->sendLogString(m_id, MLOG_STATUS, "use FMA");
      break;
   case USE_SIMD_NEONV2:
      m_mmdagent->sendLogString(m_id, MLOG_STATUS, "use NEONv2");
      break;
   case USE_SIMD_NEON:
      m_mmdagent->sendLogString(m_id, MLOG_STATUS, "use NEON");
      break;
   case USE_SIMD_NONE:
      m_mmdagent->sendLogString(m_id, MLOG_STATUS, "NONE AVAILABLE, DNN computation may be too slow");
      break;
   }

   /* load config file */
   tmp = MMDAgent_pathdup_from_application_to_system_locale(m_configFile);
   m_jconf = j_config_load_file_new(tmp);
   if (m_jconf == NULL) {
      m_mmdagent->sendLogString(m_id, MLOG_ERROR, "failed to load config file \"%s\"", m_configFile);
      free(tmp);
      return;
   }
   free(tmp);

   /* load encryption key */
   key = new ZFileKey();
   if (key->loadKeyDir(m_mmdagent->getConfigDirName()) == false) {
      delete key;
      key = NULL;
   }
   zf = NULL;
   if (m_zf)
      delete m_zf;
   m_zf = NULL;

   /* load additional user config file if exists */
   if (MMDAgent_strlen(m_userConfigFile) > 0) {
      zf = NULL;
      if (key) {
         zf = new ZFile(key);
         s = MMDAgent_strdup(zf->decryptAndGetFilePath(m_userConfigFile, NULL));
         if (s == NULL) {
            delete zf;
            delete key;
            return;
         }
         tmp = MMDAgent_pathdup_from_application_to_system_locale(s);
      } else {
         tmp = MMDAgent_pathdup_from_application_to_system_locale(m_userConfigFile);
      }
      j_config_load_file(m_jconf, tmp);
      free(tmp);
      if (zf)
         delete zf;
   }

   /* force some settings: -progout */
   m_jconf->searchnow->output.progout_flag = true;
   /* -fvad 2 */
   m_jconf->detect.fvad_mode = 2;

   /* load user dictionary */
   if (MMDAgent_strlen(m_userDictionary) > 0) {
      if (MMDAgent_exist(m_userDictionary)) {
         m_mmdagent->sendLogString(m_id, MLOG_STATUS, "going to load user dictionary \"%s\"", m_userDictionary);
         m_zf = NULL;
         if (key) {
            m_zf = new ZFile(key);
            s = MMDAgent_strdup(m_zf->decryptAndGetFilePath(m_userDictionary, NULL));
            if (s == NULL) {
               delete m_zf;
               m_zf = NULL;
               delete key;
               return;
            }
            tmp = MMDAgent_pathdup_from_application_to_system_locale(s);
         } else {
            tmp = MMDAgent_pathdup_from_application_to_system_locale(m_userDictionary);
         }
         j_add_dict(m_jconf->lm_root, tmp);
         free(tmp);
      }
   }

   /* create instance */
   m_recog = j_create_instance_from_jconf(m_jconf);
   if (m_recog == NULL) {
      m_mmdagent->sendLogString(m_id, MLOG_ERROR, "failed to startup engine");
      j_jconf_free(m_jconf);
      m_jconf = NULL;
      if (key) delete key;
      if (m_zf) delete m_zf;
      m_zf = NULL;
      return;
   }
   m_mmdagent->sendLogString(m_id, MLOG_STATUS, "instance ok");

#ifdef ENABLE_RAPID
   /* append user rapid words */
   if (MMDAgent_strlen(m_userRapidWordDictionary) > 0 && MMDAgent_exist(m_userRapidWordDictionary)) {
      PROCESS_LM *lm;
      for (lm = m_recog->lmlist; lm; lm = lm->next) {
         if (MMDAgent_strequal(lm->config->name, "keyword")) {
            WORD_INFO *new_winfo = word_info_new();
            zf = NULL;
            if (key) {
               zf = new ZFile(key);
               s = MMDAgent_strdup(zf->decryptAndGetFilePath(m_userRapidWordDictionary, NULL));
               if (s == NULL) {
                  delete zf;
                  delete key;
                  return;
               }
               tmp = MMDAgent_pathdup_from_application_to_system_locale(s);
            } else {
               tmp = MMDAgent_pathdup_from_application_to_system_locale(m_userRapidWordDictionary);
            }
            if (!init_voca(new_winfo, tmp, lm->am->hmminfo, FALSE, lm->config->forcedict_flag)) {
               m_mmdagent->sendLogString(m_id, MLOG_WARNING, "cannot load user word dic, skipped: %s", m_userRapidWordDictionary);
            } else {
               multigram_add_words_to_grammar_by_id(lm, 0, new_winfo);
               m_mmdagent->sendLogString(m_id, MLOG_STATUS, "added %d words to keyword list: %s", new_winfo->num, m_userRapidWordDictionary);
            }
            free(tmp);
            word_info_free(new_winfo);
            if (zf)
               delete zf;
            break;
         }
      }
   }
   m_mmdagent->sendLogString(m_id, MLOG_STATUS, "instance ok2");
#endif /* ENABLE_RAPID */

   /* register callback functions */
   callback_add(m_recog, CALLBACK_EVENT_RECOGNITION_BEGIN, callbackRecogBegin, this);
   callback_add(m_recog, CALLBACK_RESULT, callbackRecogResult, this);
   callback_add(m_recog, CALLBACK_RESULT_GMM, callbackGMMResult, this);
   callback_add(m_recog, CALLBACK_POLL, callbackPoll, this);
   callback_add(m_recog, CALLBACK_EVENT_PROCESS_ONLINE, callbackOnLine, this);
   callback_add(m_recog, CALLBACK_STATUS_PARAM, callbackParam, this);
#ifdef ENABLE_RAPID
   callback_add(m_recog, CALLBACK_RESULT_PASS1_DETERMINED, callbackRapid, this);
#endif

   /* setup recorder */
   m_recorder = new Julius_Record(m_mmdagent, m_id, m_recog);

   /* open audio device */
   if (j_adin_init(m_recog) == false) {
      m_mmdagent->sendLogString(m_id, MLOG_ERROR, "failed to initialize audio device");
      j_recog_free(m_recog); /* jconf is also released in j_recog_free */
      m_recog = NULL;
      m_jconf = NULL;
      if (key) delete key;
      if (m_zf) delete m_zf;
      m_zf = NULL;
      return;
   }
   m_mmdagent->sendLogString(m_id, MLOG_STATUS, "audio init ok");

   if (j_open_stream(m_recog, NULL) != 0) {
      m_mmdagent->sendLogString(m_id, MLOG_ERROR, "failed to open audio stream");
      j_recog_free(m_recog); /* jconf is also released in j_recog_free */
      m_recog = NULL;
      m_jconf = NULL;
      if (key) delete key;
      if (m_zf) delete m_zf;
      m_zf = NULL;
      return;
   }
   m_mmdagent->sendLogString(m_id, MLOG_STATUS, "openstream ok");

   /* setup logger */
   m_logger = new Julius_Logger();
   m_logger->setup(m_mmdagent, m_id, m_recog);

   /* start logger */
   m_logger->setActiveFlag(true);

   /* start recognize */
   j_recognize_stream(m_recog);
}

/* Julius_Thread::pause: pause recognition process */
void Julius_Thread::pause()
{
   if(m_recog == NULL)
      return;
   if (m_pausing == true)
      return;
   m_pausing = true;
   j_adin_change_input_scaling_factor(m_recog, 0.0f);
}

/* Julius_Thread::resume: resume recognition process */
void Julius_Thread::resume()
{
   if(m_recog == NULL)
      return;
   if (m_pausing == false)
      return;
   m_pausing = false;
   j_adin_change_input_scaling_factor(m_recog, m_currentGain);
}

/* Julius_Thread::isPausing: return true if pausing */
bool Julius_Thread::isPausing()
{
   return m_pausing;
}

/* Julius_Thread::procResult: process recognition result */
void Julius_Thread::procResult()
{
   int i;
   int first;
   Sentence *sentence;
   RecogProcess *process;
   char pname[MMDAGENT_MAXBUFLEN];
   char result[MMDAGENT_MAXBUFLEN];
   float maxScore;
   RecogProcess *processWithMaxScore;

   maxScore = LOG_ZERO;
   processWithMaxScore = NULL;
   for (process = m_recog->process_list; process; process = process->next) {
      if (!process->live)
         continue;
      if (MMDAgent_strlen(process->config->name) > 0)
         strncpy(pname, process->config->name, MMDAGENT_MAXBUFLEN);
      else
         pname[0] = '\0';
      if (process->result.status < 0) {
         switch (process->result.status) {
         case J_RESULT_STATUS_REJECT_POWER:
            m_mmdagent->sendLogString(m_id, MLOG_STATUS, "[#%d %s] input rejected by power", process->config->id, pname);
            break;
         case J_RESULT_STATUS_TERMINATE:
            m_mmdagent->sendLogString(m_id, MLOG_STATUS, "[#%d %s] input terminated by application", process->config->id, pname);
            break;
         case J_RESULT_STATUS_ONLY_SILENCE:
            m_mmdagent->sendLogString(m_id, MLOG_STATUS, "[#%d %s] input recognized as silence, ignored", process->config->id, pname);
            break;
         case J_RESULT_STATUS_REJECT_GMM:
            m_mmdagent->sendLogString(m_id, MLOG_STATUS, "[#%d %s] input rejected by GMM", process->config->id, pname);
            break;
         case J_RESULT_STATUS_REJECT_SHORT:
            m_mmdagent->sendLogString(m_id, MLOG_STATUS, "[#%d %s] input rejected by length (too short)", process->config->id, pname);
            break;
         case J_RESULT_STATUS_REJECT_LONG:
            m_mmdagent->sendLogString(m_id, MLOG_STATUS, "[#%d %s] input rejected by length (too long)", process->config->id, pname);
            break;
         case J_RESULT_STATUS_FAIL:
            m_mmdagent->sendLogString(m_id, MLOG_STATUS, "[#%d %s] search failed", process->config->id, pname);
            break;
         }
         continue;
      }
      sentence = &(process->result.sent[0]);
      strcpy(result, "");
      for (i = 0; i < sentence->word_num; i++) {
         if (MMDAgent_strlen(process->lm->winfo->woutput[sentence->word[i]]) > 0) {
            strcat(result, " ");
            strcat(result, process->lm->winfo->woutput[sentence->word[i]]);
         }
      }
      m_mmdagent->sendLogString(m_id, MLOG_STATUS, "[#%d %s] %f %s", process->config->id, pname, sentence->score_am, result);
      if (maxScore < sentence->score_am) {
         maxScore = sentence->score_am;
         processWithMaxScore = process;
      }
   }

   if (processWithMaxScore != NULL) {
      process = processWithMaxScore;
      sentence = &(process->result.sent[0]);
      strcpy(result, "");
      first = 1;
      for (i = 0; i < sentence->word_num; i++) {
         if (MMDAgent_strlen(process->lm->winfo->woutput[sentence->word[i]]) > 0) {
            if (first == 1)
               first = 0;
            else
               if (MMDAgent_strlen(m_wordspacing) > 0) strcat(result, m_wordspacing);
            strcat(result, process->lm->winfo->woutput[sentence->word[i]]);
         }
      }
      if (first == 0)
         sendMessage(JULIUSTHREAD_EVENTSTOP, result);
   }
}

/* Julius_Thread::procGMMResult: process GMM result */
void Julius_Thread::procGMMResult()
{
   if (m_recog && m_recog->gc && m_recog->gc->max_d && m_recog->gc->max_d->name)
      m_mmdagent->sendMessage(m_id, JULIUSTHREAD_EVENTGMM, "%s", m_recog->gc->max_d->name);
}

/* Julius_Thread::procCommand: process command message to modify recognition condition */
void Julius_Thread::procCommand()
{
   JuliusModificationCommand *c;
   char *p1, *p2, *save;
   float f;
   PROCESS_LM *lm;
   JCONF_LM_NAMELIST *dict, *next;

   /* pause running while main thread is inactive */
   m_mmdagent->waitWhenPaused();

   if(m_recog != NULL && m_recog->process_online) {
      while(1) {
         /* dequeue command message */
         glfwLockMutex(m_mutex);
         c = m_command;
         if(c != NULL)
            m_command = c->next;
         glfwUnlockMutex(m_mutex);
         if(c == NULL)
            return;
         /* change status */
         while(1) {
            glfwLockMutex(m_mutex);
            if(m_status == JULIUSTHREAD_STATUSWAIT) {
               m_status = JULIUSTHREAD_STATUSMODIFY;
               glfwUnlockMutex(m_mutex);
               break;
            }
            glfwUnlockMutex(m_mutex);
            MMDAgent_sleep(0.1);
         }
         /* process command message */
         p1 = MMDAgent_strtok(c->str, "|", &save);
         p2 = MMDAgent_strtok(NULL, "|", &save);
         if(MMDAgent_strequal(p1, JULIUSTHREAD_GAIN)) {
            if (m_pausing == false) {
               f = (float) atof(p2);
               if(f < 0.0)
                  f = 0.0;
               m_currentGain = f;
               j_adin_change_input_scaling_factor(m_recog, f);
               this->sendMessage(JULIUSTHREAD_EVENTMODIFY, JULIUSTHREAD_GAIN);
            }
         } else if(MMDAgent_strequal(p1, JULIUSTHREAD_USERDICTSET)) {
            if(MMDAgent_strlen(p2) > 0) {
               for(lm = m_recog->lmlist; lm; lm = lm->next) {
                  if (lm->lmtype != LM_PROB) continue;
                  /* free all additional dictionary names */
                  for(dict = lm->config->additional_dict_files; dict; dict = next) {
                     next = dict->next;
                     if(dict->name)
                        free(dict->name);
                     free(dict);
                  }
                  /* set additional dictionary name */
                  lm->config->additional_dict_files = (JCONF_LM_NAMELIST *) malloc(sizeof(JCONF_LM_NAMELIST));
                  lm->config->additional_dict_files->name = MMDAgent_pathdup_from_application_to_system_locale(p2);
                  lm->config->additional_dict_files->next = NULL;
                  /* reload */
                  j_reload_adddict(m_recog, lm);
                  m_mmdagent->sendLogString(m_id, MLOG_STATUS, "user dictionary changed to \"%s\"", p2);
               }
               this->sendMessage(JULIUSTHREAD_EVENTMODIFY, JULIUSTHREAD_USERDICTSET);
            } else {
               m_mmdagent->sendLogString(m_id, MLOG_ERROR, "%s: missing argument", JULIUSTHREAD_USERDICTSET);
            }
         } else if(MMDAgent_strequal(p1, JULIUSTHREAD_USERDICTUNSET)) {
            for(lm = m_recog->lmlist; lm; lm = lm->next) {
               if (lm->lmtype != LM_PROB) continue;
               /* free all additional dictionary names */
               for(dict = lm->config->additional_dict_files; dict; dict = next) {
                  next = dict->next;
                  if(dict->name)
                     free(dict->name);
                  free(dict);
               }
               /* set additional dictionary name */
               lm->config->additional_dict_files = NULL;
               /* reload */
               j_reload_adddict(m_recog, lm);
            }
            this->sendMessage(JULIUSTHREAD_EVENTMODIFY, JULIUSTHREAD_USERDICTUNSET);
         } else if (MMDAgent_strequal(p1, JULIUSTHREAD_PREDICTWORDFACTOR)) {
            m_boostFactor = MMDAgent_str2float(p2);
         } else if (MMDAgent_strequal(p1, JULIUSTHREAD_PREDICTWORD)) {
            /* receive comma-separated predicted word list and boost them by setting high word probabilities */
            char *p, *save2;
            char *buf;
            int c1, c2;
            WORD_ID w;
            buf = MMDAgent_strdup(p2);
            c1 = c2 = 0;

            /* find word and boost its probability */
            for (p = MMDAgent_strtok(buf, ",", &save2); p; p = MMDAgent_strtok(NULL, ",", &save2)) {
               c1++;
               for (lm = m_recog->lmlist; lm; lm = lm->next) {
                  if (lm->lmtype != LM_PROB) continue;
                  if (lm->winfo == NULL) continue;
                  for (w = 0; w < lm->winfo->num; w++) {
                     if (MMDAgent_strequal(lm->winfo->woutput[w], p)) {
                        lm->winfo->cprob[w] = m_boostFactor;
                        c2++;
                     }
                     else {
                        lm->winfo->cprob[w] = -m_boostFactor;
                     }
                  }
               }
            }
            /* parse tree lexcon to update embedded cprob */
            {
               int i;
               int node;
               LOGPROB tmpprob;
               for (RecogProcess *process = m_recog->process_list; process; process = process->next) {
                  if (!process->live)
                     continue;
                  if (process->lm->lmtype != LM_PROB)
                     continue;
                  for (i = 0; i < process->wchmm->fsnum; i++) {
                     process->wchmm->fscore[i] = LOG_ZERO;
                  }
                  for (WORD_ID w = 0; w < process->wchmm->winfo->num; w++) {
                     for (i = 0; i < process->wchmm->winfo->wlen[w] + 1; i++) {
                        if (i < process->wchmm->winfo->wlen[w]) {
                           node = process->wchmm->offset[w][i];
                        }
                        else {
                           node = process->wchmm->wordend[w];
                        }
                        if (process->wchmm->state[node].scid < 0) {
                           if (process->wchmm->ngram) {
                              tmpprob = uni_prob(process->wchmm->ngram, process->wchmm->winfo->wton[w])
#ifdef CLASS_NGRAM
                                 + process->wchmm->winfo->cprob[w]
#endif
                                 ;
                           } else {
                              tmpprob = LOG_ZERO;
                           }
                           if (process->wchmm->lmvar == LM_NGRAM_USER) {
                              tmpprob = (*(process->wchmm->uni_prob_user))(process->wchmm->winfo, w, tmpprob);
                           }
                           int n = - process->wchmm->state[node].scid;
                           if (process->wchmm->fscore[n] < tmpprob) {
                              process->wchmm->fscore[n] = tmpprob;
                           }
                        }
                     }
                  }
               }
            }

            m_mmdagent->sendLogString(m_id, MLOG_STATUS, "%d words, %d entry boosted by %.3f", c1, c2, m_boostFactor);
            free(buf);
         }
         free(c->str);
         free(c);
         /* change status */
         glfwLockMutex(m_mutex);
         m_status = JULIUSTHREAD_STATUSWAIT;
         glfwUnlockMutex(m_mutex);
      }
   }
}

/* Julius_Thread::procParam: process parameter status */
void Julius_Thread::procParam()
{
   int frames;
   int msec;

   if (m_recog && m_recog->mfcclist && m_recog->mfcclist->param) {
      frames = m_recog->mfcclist->param->samplenum;
      msec = (int)((float)frames * (float)m_recog->jconf->input.period * (float)m_recog->jconf->input.frameshift / 10000.0f);
      m_mmdagent->sendLogString(m_id, MLOG_STATUS, "input length: %d msec", msec);
   }
}

#ifdef ENABLE_RAPID
/* Julius_Thread::procRapid: process rapid determination */
void Julius_Thread::procRapid()
{
   RecogProcess *process;
   FSBeam *d;
   char buff[MMDAGENT_MAXBUFLEN];
   int i;

   if (m_recog == NULL)
      return;
   for (process = m_recog->process_list; process; process = process->next) {
      if (!process->live)
         continue;
      if (process->config->rapid.enabled == FALSE)
         continue;
      d = &(process->pass1);
      if (d->rapid.newly_determined_len > 0) {
         strcpy(buff, process->lm->winfo->woutput[d->rapid.newly_determined_wid[0]]);
         for (i = 1; i < d->rapid.newly_determined_len; i++) {
            strcat(buff, ",");
            strcat(buff, process->lm->winfo->woutput[d->rapid.newly_determined_wid[i]]);
	      }
	      this->sendMessage(JULIUSTHREAD_EVENTRAPID, buff);
      }
   }
}
#endif /* ENABLE_RAPID */

/* Julius_Thread::storeCommand: store command message to modify recognition condition */
void Julius_Thread::storeCommand(const char *args)
{
   JuliusModificationCommand *c1, *c2;

   if(m_recog != NULL && MMDAgent_strlen(args) > 0) {
      /* create command message */
      c1 = (JuliusModificationCommand *) malloc(sizeof(JuliusModificationCommand));
      c1->str = MMDAgent_strdup(args);
      c1->next = NULL;
      /* store command message */
      glfwLockMutex(m_mutex);
      if(m_command == NULL)
         m_command = c1;
      else {
         for(c2 = m_command; c2->next != NULL; c2 = c2->next);
         c2->next = c1;
      }
      glfwUnlockMutex(m_mutex);
   }
}

/* Julius_Thread::moveThreshold: move level threshold */
void Julius_Thread::moveThreshold(int step)
{
   if (m_recog == NULL)
      return;

   m_recog->adin->thres += step;
   if (m_recog->adin->thres < JULIUSLOGGER_ADINUNDERFLOWTHRES)
      m_recog->adin->thres = JULIUSLOGGER_ADINUNDERFLOWTHRES;
   if (m_recog->adin->thres > JULIUSLOGGER_ADINOVERFLOWTHRES)
      m_recog->adin->thres = JULIUSLOGGER_ADINOVERFLOWTHRES;
   m_recog->jconf->detect.level_thres = m_recog->adin->thres;
   m_recog->adin->zc.trigger = m_recog->adin->thres;
}

/* Julius_Thread::sendMessage: send message to MMDAgent */
void Julius_Thread::sendMessage(const char *str1, const char *str2)
{
   m_mmdagent->sendMessage(m_id, str1, "%s", str2);
}

/* Julius_Thread::getLogActiveFlag: get active flag of logger */
bool Julius_Thread::getLogActiveFlag()
{
   return (m_logger) ? m_logger->getActiveFlag() : false;
}

/* Julius_Thread::setLogActiveFlag: set active flag of logger */
void Julius_Thread::setLogActiveFlag(bool b)
{
   if (m_logger)
      m_logger->setActiveFlag(b);
}

/* Julius_Thread::setLogCaption: set closed caption string of logger */
void Julius_Thread::setLogCaption(const char *s, int id, float x, float y)
{
   if (m_logger)
      m_logger->setCaption(s, id, x, y);
}

/* Julius_Thread::setLogCaptionDuration: set caption duration of logger */
void Julius_Thread::setLogCaptionDuration(int id, double frame)
{
   if (m_logger)
      m_logger->setCaptionDuration(id, frame);
}

/* Julius_Thread::updateLog: update log view per step */
void Julius_Thread::updateLog(double frame)
{
   bool updateFlag = false;

   glfwLockMutex(m_mutex);
   if(m_status == JULIUSTHREAD_STATUSWAIT) {
      updateFlag = true;
      m_status = JULIUSTHREAD_STATUSUPDATE;
   }
   glfwUnlockMutex(m_mutex);

   /* if modifying, skip update */
   if(updateFlag == false)
      return;
   if (m_logger)
      m_logger->update(frame);

   glfwLockMutex(m_mutex);
   m_status = JULIUSTHREAD_STATUSWAIT;
   glfwUnlockMutex(m_mutex);
}

/* Julius_Thread::renderLog: render log view */
void Julius_Thread::renderLog(float width, float height)
{
   bool renderFlag = false;

   glfwLockMutex(m_mutex);
   if(m_status == JULIUSTHREAD_STATUSWAIT) {
      renderFlag = true;
      m_status = JULIUSTHREAD_STATUSRENDER;
   }
   glfwUnlockMutex(m_mutex);

   /* if modifying, skip render */
   if(renderFlag == false)
      return;
   if (m_logger)
      m_logger->render2D(width, height);

   glfwLockMutex(m_mutex);
   m_status = JULIUSTHREAD_STATUSWAIT;
   glfwUnlockMutex(m_mutex);
}

/* Julius_Thread::enableRecording: enable recording */
void Julius_Thread::enableRecording(const char *dirname)
{
   m_mmdagent->sendLogString(m_id, MLOG_STATUS, "recording triggered audio inputs to %s ...", dirname);
   m_recorder->setDir(dirname);
   // internally limit recording amount at one session to approx. 10 minutes (20MB)
   m_recorder->setLimit(20 * 1024 * 1024);
   m_recorder->enable();
}

/* Julius_Thread::disableRecording: disable recording */
void Julius_Thread::disableRecording()
{
   m_mmdagent->sendLogString(m_id, MLOG_STATUS, "stopped recording");
   m_recorder->disable();
}

/* Julius_Thread::setWordSpacing: set word spacing */
void Julius_Thread::setWordSpacing(const char *str)
{
   MMDAgent_snprintf(m_wordspacing, 10, "%s", str);
}
