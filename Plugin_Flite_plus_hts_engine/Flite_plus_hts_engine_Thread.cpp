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

#include <locale.h>

#include "MMDAgent.h"

#include "HTS_engine.h"

#include "flite_hts_engine.h"

#include "Flite_plus_hts_engine.h"
#include "Flite_plus_hts_engine_Thread.h"

/* mainThread: main thread */
static void mainThread(void *param)
{
   Flite_plus_hts_engine_Thread *flite_hts_engine_thread = (Flite_plus_hts_engine_Thread *) param;
   flite_hts_engine_thread->run();
}

/* Flite_plus_hts_engine_Thread::initialize: initialize thread */
void Flite_plus_hts_engine_Thread::initialize()
{
   m_mmdagent = NULL;
   m_id = 0;

   m_mutex = NULL;
   m_cond = NULL;
   m_thread = -1;

   m_count = 0;

   m_speaking = false;
   m_kill = false;

   m_charaBuff = NULL;
   m_styleBuff = NULL;
   m_textBuff = NULL;

   m_numModels = 0;
   m_modelNames = NULL;
   m_numStyles = 0;
   m_styleNames = NULL;
}

/* Flite_plus_hts_engine_Thread::clear: free thread */
void Flite_plus_hts_engine_Thread::clear()
{
   int i;

   stop();
   m_kill = true;

   /* wait */
   if (m_cond != NULL)
      glfwSignalCond(m_cond);

   if(m_mutex != NULL || m_cond != NULL || m_thread >= 0) {
      if(m_thread >= 0) {
         glfwWaitThread(m_thread, GLFW_WAIT);
         glfwDestroyThread(m_thread);
      }
      if(m_cond != NULL)
         glfwDestroyCond(m_cond);
      if(m_mutex != NULL)
         glfwDestroyMutex(m_mutex);
   }

   if(m_charaBuff)
      free(m_charaBuff);
   if(m_styleBuff)
      free(m_styleBuff);
   if(m_textBuff)
      free(m_textBuff);

   /* free model names */
   if (m_numModels > 0) {
      for (i = 0; i < m_numModels; i++)
         free(m_modelNames[i]);
      free(m_modelNames);
   }

   /* free style names */
   if (m_numStyles > 0) {
      for (i = 0; i < m_numStyles; i++)
         free(m_styleNames[i]);
      free(m_styleNames);
   }

   initialize();
}

/* Flite_plus_hts_engine_Thread::Flite_plus_hts_engine_Thread: thread constructor */
Flite_plus_hts_engine_Thread::Flite_plus_hts_engine_Thread()
{
   initialize();
}

/* Flite_plus_hts_engine_Thread::~Flite_plus_hts_engine_Thread: thread destructor */
Flite_plus_hts_engine_Thread::~Flite_plus_hts_engine_Thread()
{
   clear();
}

/* Flite_plus_hts_engine_Thread::loadAndStart: load models and start thread */
bool Flite_plus_hts_engine_Thread::loadAndStart(MMDAgent *mmdagent, int id, const char *config)
{
   int i, j, k;
   char buff[MMDAGENT_MAXBUFLEN];
   FILE *fp;
   bool err = false;
   double *weights;

   /* load config file */
   fp = MMDAgent_fopen(config, "r");
   if (fp == NULL)
      return false;

   /* number of speakers */
   i = MMDAgent_fgettoken(fp, buff);
   if (i <= 0) {
      fclose(fp);
      clear();
      return false;
   }
   m_numModels = MMDAgent_str2int(buff);
   if (m_numModels <= 0) {
      fclose(fp);
      clear();
      return false;
   }

   /* model names */
   m_modelNames = (char **) malloc(sizeof(char *) * m_numModels);
   for (i = 0; i < m_numModels; i++) {
      j = MMDAgent_fgettoken(fp, buff);
      if (j <= 0)
         err = true;
      m_modelNames[i] = MMDAgent_strdup(buff);
   }
   if (err) {
      fclose(fp);
      clear();
      return false;
   }

   /* number of speaking styles */
   i = MMDAgent_fgettoken(fp, buff);
   if (i <= 0) {
      fclose(fp);
      clear();
      return false;
   }
   m_numStyles = MMDAgent_str2int(buff);
   if (m_numStyles <= 0) {
      m_numStyles = 0;
      fclose(fp);
      clear();
      return false;
   }

   /* style names and weights */
   m_styleNames = (char **) calloc(m_numStyles, sizeof(char *));
   weights = (double *) calloc((3 * m_numModels + 4) * m_numStyles, sizeof(double));
   for (i = 0; i < m_numStyles; i++) {
      j = MMDAgent_fgettoken(fp, buff);
      if(j <= 0)
         err = true;
      m_styleNames[i] = MMDAgent_strdup(buff);
      for (j = 0; j < 3 * m_numModels + 4; j++) {
         k = MMDAgent_fgettoken(fp, buff);
         if (k <= 0)
            err = true;
         weights[(3 * m_numModels + 4) * i + j] = MMDAgent_str2float(buff);
      }
   }
   fclose(fp);
   if(err) {
      free(weights);
      clear();
      return false;
   }

   /* load models for TTS */
   if (m_flite_plus_hts_engine.load(m_modelNames, m_numModels, weights, m_numStyles) != true) {
      free(weights);
      clear();
      return false;
   }
   free(weights);

   m_mmdagent = mmdagent;
   m_id = id;

   /* start thread */
   glfwInit();
   m_mutex = glfwCreateMutex();
   m_cond = glfwCreateCond();
   m_thread = glfwCreateThread(mainThread, this);
   if(m_mutex == NULL || m_cond == NULL || m_thread < 0) {
      clear();
      return false;
   }

   return true;
}

/* Flite_plus_hts_engine_Thread::stopAndRelease: stop thread and free Flite_HTS_Engine */
void Flite_plus_hts_engine_Thread::stopAndRelease()
{
   clear();
}

/* Flite_plus_hts_engine_Thread::run: main thread loop for TTS */
void Flite_plus_hts_engine_Thread::run()
{
   char lip[MMDAGENT_MAXBUFLEN];
   char *chara, *style, *text;
   int index;

   while (m_kill == false) {
      /* wait text */
      glfwLockMutex(m_mutex);
      while(m_count <= 0) {
         glfwWaitCond(m_cond, m_mutex, GLFW_INFINITY);
         if(m_kill == true)
            return;
      }
      chara = MMDAgent_strdup(m_charaBuff);
      style = MMDAgent_strdup(m_styleBuff);
      text = MMDAgent_strdup(m_textBuff);
      m_count--;
      glfwUnlockMutex(m_mutex);

      m_speaking = true;

      /* search style index */
      for (index = 0; index < m_numStyles; index++)
         if (MMDAgent_strequal(m_styleNames[index], style))
            break;
      if (index >= m_numStyles) { /* unknown style */
         m_speaking = false;
         continue;
      }

      /* send SYNTH_EVENT_START */
      m_mmdagent->sendMessage(m_id, FLITEPLUSHTSENGINETHREAD_EVENTSTART, "%s", chara);

      /* synthesize */
      m_flite_plus_hts_engine.setStyle(index);
      m_flite_plus_hts_engine.prepare(text);
      m_flite_plus_hts_engine.getPhonemeSequence(lip);
      if (MMDAgent_strlen(lip) > 0) {
         m_mmdagent->sendMessage(m_id, FLITEPLUSHTSENGINETHREAD_COMMANDSTARTLIP, "%s|%s", chara, lip);
         m_flite_plus_hts_engine.synthesis();
      }

      /* send SYNTH_EVENT_STOP */
      m_mmdagent->sendMessage(m_id, FLITEPLUSHTSENGINETHREAD_EVENTSTOP, "%s", chara);

      if(chara)
         free(chara);
      if(style)
         free(style);
      if(text)
         free(text);
      m_speaking = false;
   }
}

/* Flite_plus_hts_engine_Thread::isRunning: check running */
bool Flite_plus_hts_engine_Thread::isRunning()
{
   if(m_kill == true || m_mutex == NULL || m_cond == NULL || m_thread < 0)
      return false;
   else
      return true;
}

/* Flite_plus_hts_engine_Thread::isSpeaking: check speaking */
bool Flite_plus_hts_engine_Thread::isSpeaking()
{
   return m_speaking;
}

/* checkCharacter: check speaking character */
bool Flite_plus_hts_engine_Thread::checkCharacter(const char *chara)
{
   bool ret;

   /* check */
   if(isRunning() == false)
      return false;

   /* wait buffer mutex */
   glfwLockMutex(m_mutex);

   /* save character name, speaking style, and text */
   ret = MMDAgent_strequal(m_charaBuff, chara);

   /* release buffer mutex */
   glfwUnlockMutex(m_mutex);

   return ret;
}

/* Flite_plus_hts_engine_Thread::synthesis: start synthesis */
void Flite_plus_hts_engine_Thread::synthesis(const char *chara, const char *style, const char *text)
{
   /* check */
   if(isRunning() == false)
      return;
   if(MMDAgent_strlen(chara) <= 0 || MMDAgent_strlen(style) <= 0 || MMDAgent_strlen(text) <= 0)
      return;

   /* wait buffer mutex */
   glfwLockMutex(m_mutex);

   /* save character name, speaking style, and text */
   if(m_charaBuff)
      free(m_charaBuff);
   if(m_styleBuff)
      free(m_styleBuff);
   if(m_textBuff)
      free(m_textBuff);
   m_charaBuff = MMDAgent_strdup(chara);
   m_styleBuff = MMDAgent_strdup(style);
   m_textBuff = MMDAgent_strdup(text);
   m_count++;

   /* start synthesis thread */
   if(m_count <= 1)
      glfwSignalCond(m_cond);

   /* release buffer mutex */
   glfwUnlockMutex(m_mutex);
}

/* Flite_plus_hts_engine_Thread::stop: barge-in function */
void Flite_plus_hts_engine_Thread::stop()
{
   if(isRunning() == true) {
      m_flite_plus_hts_engine.stop();

      /* wait buffer mutex */
      glfwLockMutex(m_mutex);

      /* stop lip sync */
      m_mmdagent->sendMessage(m_id, FLITEPLUSHTSENGINETHREAD_COMMANDSTOPLIP, "%s", m_charaBuff);

      /* release buffer mutex */
      glfwUnlockMutex(m_mutex);

   }
}
