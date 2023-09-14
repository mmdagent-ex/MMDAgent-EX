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

#include "mecab.h"
#include "njd.h"
#include "jpcommon.h"
#include "HTS_engine.h"

#include "text2mecab.h"
#include "mecab2njd.h"
#include "njd2jpcommon.h"

#include "njd_set_pronunciation.h"
#include "njd_set_digit.h"
#include "njd_set_accent_phrase.h"
#include "njd_set_accent_type.h"
#include "njd_set_unvoiced_vowel.h"
#include "njd_set_long_vowel.h"

#include "Open_JTalk.h"

/* Open_JTalk::initialize: initialize system */
void Open_JTalk::initialize()
{
   m_mmdagent = NULL;
   m_id = 0;

   Mecab_initialize(&m_mecab);
   NJD_initialize(&m_njd);
   JPCommon_initialize(&m_jpcommon);
   HTS_Engine_initialize(&m_engine);

   m_numModels = 0;
   m_styleWeights = NULL;
   m_numStyles = 0;
}

/* Open_JTalk::clear: free system */
void Open_JTalk::clear()
{
   Mecab_clear(&m_mecab);
   NJD_clear(&m_njd);
   JPCommon_clear(&m_jpcommon);
   HTS_Engine_clear(&m_engine);

   m_numModels = 0;
   if (m_styleWeights != NULL)
      free(m_styleWeights);
   m_styleWeights = NULL;
   m_numStyles = 0;
}

/* Open_JTalk::Open_JTalk: constructor */
Open_JTalk::Open_JTalk()
{
   initialize();
}

/* Open_JTalk::~Open_JTalk: destructor */
Open_JTalk::~Open_JTalk()
{
   clear();
}

/* Open_JTalk::load: load dictionary and models */
bool Open_JTalk::load(MMDAgent *mmdagent, int id, const char *dicDir, char **modelFiles, int numModels, double *styleWeights, int numStyles)
{
   int i, j;

   char *dn_mecab;
   ZFileKey *key;
   ZFile *zf;
   unsigned char **fn_models_mem;
   size_t *sizes;

   m_mmdagent = mmdagent;
   m_id = id;

   if (dicDir == NULL || modelFiles == NULL || numModels <= 0 || styleWeights == NULL || numStyles <= 0) {
      m_mmdagent->sendLogString(m_id, MLOG_ERROR, "wrong parameter to load OpenJTalk");
      return false;
   }
   m_numModels = numModels;
   m_numStyles = numStyles;

   /* load dictionary */
   dn_mecab = MMDAgent_pathdup_from_application_to_system_locale(dicDir);
   if(Mecab_load(&m_mecab, dn_mecab) != TRUE) {
      m_mmdagent->sendLogString(m_id, MLOG_ERROR, "failed to load mecab dictionary in \"%s\"", dicDir);
      free(dn_mecab);
      return false;
   }
   free(dn_mecab);

   /* load encryption key */
   key = new ZFileKey();
   if (key->loadKeyDir(m_mmdagent->getConfigDirName()) == false) {
      delete key;
      key = NULL;
   }

   /* load acoustic models */
   fn_models_mem = (unsigned char **) calloc(m_numModels, sizeof(unsigned char *));
   sizes = (size_t *)calloc(m_numModels, sizeof(size_t));
   zf = new ZFile(key);
   for (i = 0; i < m_numModels; i++) {
      if (zf->openAndLoad(modelFiles[i]) == false)
         break;
      sizes[i] = zf->getSize();
      fn_models_mem[i] = (unsigned char *)malloc(sizes[i]);
      memcpy(fn_models_mem[i], zf->getData(), sizes[i]);
      zf->close();
   }
   delete zf;
   if (i < m_numModels) {
      m_mmdagent->sendLogString(m_id, MLOG_ERROR, "failed to load one of voice models");
      for (j = 0; j < i; j++)
         free(fn_models_mem[j]);
      free(fn_models_mem);
      free(sizes);
      if (key) delete key;
      return false;

   }
   if(HTS_Engine_load_mem(&m_engine, (void **)fn_models_mem, sizes, m_numModels) != TRUE) {
      m_mmdagent->sendLogString(m_id, MLOG_ERROR, "failed to load one of voice models");
      for(i = 0; i < m_numModels; i++)
         free(fn_models_mem[i]);
      free(fn_models_mem);
      free(sizes);
      if (key) delete key;
      return false;
   }
   for (i = 0; i < m_numModels; i++)
      free(fn_models_mem[i]);
   free(fn_models_mem);
   free(sizes);
   if (key) delete key;

   /* set style interpolation weight */
   m_styleWeights = (double *) calloc(m_numStyles * (m_numModels * 3 + 4), sizeof(double));
   for (j = 0; j < m_numStyles * (m_numModels * 3 + 4); j++)
      m_styleWeights[j] = styleWeights[j];

   HTS_Engine_set_audio_buff_size(&m_engine, OPENJTALK_AUDIOBUFFSIZE);

   setStyle(0);
   return true;
}

/* Open_JTalk::prepare: text analysis, decision of state durations, and parameter generation */
void Open_JTalk::prepare(const char *str)
{
   char *buff;
   char **label_feature = NULL;
   int label_size = 0;

   if(m_numStyles <= 0)
      return;

   /* text analysis */
   HTS_Engine_set_stop_flag(&m_engine, false);
   buff = (char *) calloc(2 * MMDAgent_strlen(str) + 1, sizeof(char));
   text2mecab(buff, str);
   Mecab_analysis(&m_mecab, buff);
   free(buff);
   mecab2njd(&m_njd, Mecab_get_feature(&m_mecab), Mecab_get_size(&m_mecab));
   njd_set_pronunciation(&m_njd);
   njd_set_digit(&m_njd);
   njd_set_accent_phrase(&m_njd);
   njd_set_accent_type(&m_njd);
   njd_set_unvoiced_vowel(&m_njd);
   njd_set_long_vowel(&m_njd);
   njd2jpcommon(&m_jpcommon, &m_njd);
   JPCommon_make_label(&m_jpcommon);
   if (JPCommon_get_label_size(&m_jpcommon) > 2) {
      /* decision of state durations */
      label_feature = JPCommon_get_label_feature(&m_jpcommon);
      label_size = JPCommon_get_label_size(&m_jpcommon);
      if(HTS_Engine_generate_state_sequence_from_strings(&m_engine, &label_feature[1], label_size - 1) != TRUE) { /* skip first silence */
         HTS_Engine_refresh(&m_engine);
         return;
      }
      if(HTS_Engine_generate_parameter_sequence(&m_engine) != TRUE) {
         HTS_Engine_refresh(&m_engine);
         return;
      }
   }
}

/* Open_JTalk::getPhonemeSequence: get phoneme sequence */
void Open_JTalk::getPhonemeSequence(char *str)
{
   int i, j, k;
   int size;
   char **feature;
   int nstate;
   int fperiod;
   int sampling_frequency;
   char *ch, *start, *end;

   strcpy(str, "");

   if(m_numStyles <= 0)
      return;

   size = JPCommon_get_label_size(&m_jpcommon);
   feature = JPCommon_get_label_feature(&m_jpcommon);
   nstate = HTS_Engine_get_nstate(&m_engine);
   fperiod = HTS_Engine_get_fperiod(&m_engine);
   sampling_frequency = HTS_Engine_get_sampling_frequency(&m_engine);

   if (size <= 2)
      return;

   /* skip first and final silence */
   size -= 2;
   feature = &feature[1];

   for (i = 0; i < size; i++) {
      if (i > 0)
         strcat(str, ",");
      /* get phoneme from full-context label */
      start = strchr(feature[i], '-');
      end = strchr(feature[i], '+');
      if (start != NULL && end != NULL) {
         for (ch = start + 1; ch != end; ch++)
            sprintf(str, "%s%c", str, *ch);
      } else {
         strcat(str, feature[i]);
      }
      /* get ms */
      for (j = 0, k = 0; j < nstate; j++)
         k += (HTS_Engine_get_state_duration(&m_engine, i * nstate + j) * fperiod * 1000) / sampling_frequency;
      sprintf(str, "%s,%d", str, k);
   }
}

/* Open_JTalk::synthesis: speech synthesis */
void Open_JTalk::synthesis()
{
   if(m_numStyles <= 0)
      return;
   if (JPCommon_get_label_size(&m_jpcommon) > 2)
      HTS_Engine_generate_sample_sequence(&m_engine);
   HTS_Engine_refresh(&m_engine);
   JPCommon_refresh(&m_jpcommon);
   NJD_refresh(&m_njd);
   Mecab_refresh(&m_mecab);
}

/* Open_JTalk::stop: stop speech synthesis */
void Open_JTalk::stop()
{
   if(m_numStyles <= 0)
      return;
   HTS_Engine_set_stop_flag(&m_engine, TRUE);
}

/* Open_JTalk::setStyle: set style interpolation weight */
bool Open_JTalk::setStyle(int val)
{
   int i, index;
   double f;

   if (m_numStyles <= 0 || m_styleWeights == NULL || val < 0 || val >= m_numStyles)
      return false;

   /* interpolation weight */
   index = val * (m_numModels * 3 + 4);
   for (i = 0; i < m_numModels; i++)
      HTS_Engine_set_parameter_interpolation_weight(&m_engine, i, 0, m_styleWeights[index + m_numModels * 0 + i]);
   for (i = 0; i < m_numModels; i++)
      HTS_Engine_set_parameter_interpolation_weight(&m_engine, i, 1, m_styleWeights[index + m_numModels * 1 + i]);
   for (i = 0; i < m_numModels; i++)
      HTS_Engine_set_duration_interpolation_weight(&m_engine, i, m_styleWeights[index + m_numModels * 2 + i]);

   /* speed */
   f = m_styleWeights[index + m_numModels * 3 + 0];
   if(f > OPENJTALK_MAXSPEED)
      HTS_Engine_set_speed(&m_engine, OPENJTALK_MAXSPEED);
   else if(f < OPENJTALK_MINSPEED)
      HTS_Engine_set_speed(&m_engine, OPENJTALK_MINSPEED);
   else
      HTS_Engine_set_speed(&m_engine, f);

   /* pitch */
   f = m_styleWeights[index + m_numModels * 3 + 1];
   if(f > OPENJTALK_MAXHALFTONE)
      HTS_Engine_add_half_tone(&m_engine, OPENJTALK_MAXHALFTONE);
   else if(f < OPENJTALK_MINHALFTONE)
      HTS_Engine_add_half_tone(&m_engine, OPENJTALK_MINHALFTONE);
   else
      HTS_Engine_add_half_tone(&m_engine, f);

   /* alpha */
   f = m_styleWeights[index + m_numModels * 3 + 2];
   if(f > OPENJTALK_MAXALPHA)
      HTS_Engine_set_alpha(&m_engine, OPENJTALK_MAXALPHA);
   else if(f < OPENJTALK_MINALPHA)
      HTS_Engine_set_alpha(&m_engine, OPENJTALK_MINALPHA);
   else
      HTS_Engine_set_alpha(&m_engine, f);

   /* volume */
   f = m_styleWeights[index + m_numModels * 3 + 3];
   if(f > OPENJTALK_MAXVOLUME)
      HTS_Engine_set_volume(&m_engine, OPENJTALK_MAXVOLUME);
   else if(f < OPENJTALK_MINVOLUME)
      HTS_Engine_set_volume(&m_engine, OPENJTALK_MINVOLUME);
   else
      HTS_Engine_set_volume(&m_engine, f);

   return true;
}
