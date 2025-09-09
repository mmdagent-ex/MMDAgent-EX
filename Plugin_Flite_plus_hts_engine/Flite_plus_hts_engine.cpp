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

#include "HTS_engine.h"

#include "flite_hts_engine.h"

#include "Flite_plus_hts_engine.h"

/* Flite_plus_hts_engine::initialize: initialize system */
void Flite_plus_hts_engine::initialize()
{
   m_label_size = 0;
   m_label_data = NULL;

   HTS_Engine_initialize(&m_engine);

   m_numModels = 0;
   m_styleWeights = NULL;
   m_numStyles = 0;
}

/* Flite_plus_hts_engine::clear: free system */
void Flite_plus_hts_engine::clear()
{
   int i;

   if(m_label_data) {
      for(i = 0; i < m_label_size; i++)
         free(m_label_data[i]);
      free(m_label_data);
      m_label_size = 0;
      m_label_data = NULL;
   }

   HTS_Engine_clear(&m_engine);

   m_numModels = 0;
   if (m_styleWeights != NULL)
      free(m_styleWeights);
   m_styleWeights = NULL;
   m_numStyles = 0;
}

/* Flite_plus_hts_engine::Flite_hts_engine: constructor */
Flite_plus_hts_engine::Flite_plus_hts_engine()
{
   initialize();
}

/* Flite_plus_hts_engine::~Flite_hts_engine: destructor */
Flite_plus_hts_engine::~Flite_plus_hts_engine()
{
   clear();
}

/* Flite_plus_hts_engine::load: load dictionary and models */
bool Flite_plus_hts_engine::load(char **modelFiles, int numModels, double *styleWeights, int numStyles)
{
   int i, j;

   char **fn_models;

   if (modelFiles == NULL || numModels <= 0 || styleWeights == NULL || numStyles <= 0) {
      return false;
   }
   m_numModels = numModels;
   m_numStyles = numStyles;

   /* load acoustic models */
   fn_models = (char **) calloc(m_numModels, sizeof(char *));
   for (i = 0; i < m_numModels; i++)
      fn_models[i] = MMDAgent_pathdup_from_application_to_system_locale(modelFiles[i]);
   if(HTS_Engine_load(&m_engine, fn_models, m_numModels) != TRUE) {
      for(i = 0; i < m_numModels; i++)
         free(fn_models[i]);
      free(fn_models);
      return false;
   }
   for (i = 0; i < m_numModels; i++)
      free(fn_models[i]);
   free(fn_models);

   /* set style interpolation weight */
   m_styleWeights = (double *) calloc(m_numStyles * (m_numModels * 3 + 4), sizeof(double));
   for (j = 0; j < m_numStyles * (m_numModels * 3 + 4); j++)
      m_styleWeights[j] = styleWeights[j];

   HTS_Engine_set_audio_buff_size(&m_engine, FLITEHTSENGINE_AUDIOBUFFSIZE);
   setStyle(0);
   return true;
}

/* Flite_plus_hts_engine::prepare: text analysis, decision of state durations, and parameter generation */
void Flite_plus_hts_engine::prepare(const char *str)
{
   int i;
   Flite_Text_Analyzer analyzer;

   if(m_numStyles <= 0)
      return;

   /* text analysis */
   HTS_Engine_set_stop_flag(&m_engine, false);
   Flite_Text_Analyzer_initialize(&analyzer);
   Flite_Text_Analyzer_analysis(&analyzer, str);
   if(Flite_Text_Analyzer_get_label_data(&analyzer, &m_label_data, &m_label_size) != TRUE) {
      Flite_Text_Analyzer_clear(&analyzer);
      return;
   }
   Flite_Text_Analyzer_clear(&analyzer);

   if (m_label_size <= 2 ||
         HTS_Engine_generate_state_sequence_from_strings(&m_engine, &m_label_data[1], m_label_size - 1) != TRUE ||
         HTS_Engine_generate_parameter_sequence(&m_engine) != TRUE) {
      HTS_Engine_refresh(&m_engine);
      for(i = 0; i < m_label_size; i++)
         free(m_label_data[i]);
      free(m_label_data);
      m_label_size = 0;
      return;
   }
}

/* Flite_plus_hts_engine::getPhonemeSequence: get phoneme sequence */
void Flite_plus_hts_engine::getPhonemeSequence(char *str, int strlen)
{
   int i, j, k;
   int size;
   char **feature;
   int nstate;
   int fperiod;
   int sampling_frequency;
   char *ch, *start, *end;
   int len;
   int maxlen;
   int sublen;

   strcpy(str, "");
   len = 0;
   maxlen = strlen - 1;
   if (maxlen < 1)
      return;

   if(m_numStyles <= 0)
      return;

   size = m_label_size;
   feature = m_label_data;
   nstate = (int)HTS_Engine_get_nstate(&m_engine);
   fperiod = (int)HTS_Engine_get_fperiod(&m_engine);
   sampling_frequency = (int)HTS_Engine_get_sampling_frequency(&m_engine);

   if (size <= 2)
      return;

   /* skip first and final silence */
   size -= 2;
   feature = &feature[1];

   for (i = 0; i < size; i++) {
      if (i > 0) {
         if (len >= maxlen) return;
         str[len++] = ',';
      }
      /* get phoneme from full-context label */
      start = strchr(feature[i], '-');
      end = strchr(feature[i], '+');
      if (start != NULL && end != NULL) {
         for (ch = start + 1; ch != end; ch++) {
            if (len >= maxlen) return;
            str[len++] = *ch;
         }
      } else {
         sublen = (int)MMDAgent_strlen(feature[i]);
         for (j = 0; j < sublen; j++) {
            if (len >= maxlen) return;
            str[len++] = feature[i][j];
         }
      }
      /* get ms */
      for (j = 0, k = 0; j < nstate; j++)
         k += ((int)HTS_Engine_get_state_duration(&m_engine, i * nstate + j) * fperiod * 1000) / sampling_frequency;
      if (strlen - len - 1 <= 0) return;
      MMDAgent_snprintf(&str[len], strlen - len - 1, ",%d", k);
      len = (int)MMDAgent_strlen(str);
   }
}

/* Flite_plus_hts_engine::synthesis: speech synthesis */
void Flite_plus_hts_engine::synthesis()
{
   int i;

   if(m_numStyles <= 0)
      return;

   if (m_label_size > 2)
      HTS_Engine_generate_sample_sequence(&m_engine);
   HTS_Engine_refresh(&m_engine);
   if(m_label_data) {
      for(i = 0; i < m_label_size; i++) {
         free(m_label_data[i]);
      }
      free(m_label_data);
   }
   m_label_data = NULL;
   m_label_size = 0;
}

/* Flite_plus_hts_engine::stop: stop speech synthesis */
void Flite_plus_hts_engine::stop()
{
   if(m_numStyles <= 0)
      return;
   HTS_Engine_set_stop_flag(&m_engine, TRUE);
}

/* Flite_plus_hts_engine::setStyle: set style interpolation weight */
bool Flite_plus_hts_engine::setStyle(int val)
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
   if(f > FLITEHTSENGINE_MAXSPEED)
      HTS_Engine_set_speed(&m_engine, FLITEHTSENGINE_MAXSPEED);
   else if(f < FLITEHTSENGINE_MINSPEED)
      HTS_Engine_set_speed(&m_engine, FLITEHTSENGINE_MINSPEED);
   else
      HTS_Engine_set_speed(&m_engine, f);

   /* pitch */
   f = m_styleWeights[index + m_numModels * 3 + 1];
   if(f > FLITEHTSENGINE_MAXHALFTONE)
      HTS_Engine_add_half_tone(&m_engine, FLITEHTSENGINE_MAXHALFTONE);
   else if(f < FLITEHTSENGINE_MINHALFTONE)
      HTS_Engine_add_half_tone(&m_engine, FLITEHTSENGINE_MINHALFTONE);
   else
      HTS_Engine_add_half_tone(&m_engine, f);

   /* alpha */
   f = m_styleWeights[index + m_numModels * 3 + 2];
   if(f > FLITEHTSENGINE_MAXALPHA)
      HTS_Engine_set_alpha(&m_engine, FLITEHTSENGINE_MAXALPHA);
   else if(f < FLITEHTSENGINE_MINALPHA)
      HTS_Engine_set_alpha(&m_engine, FLITEHTSENGINE_MINALPHA);
   else
      HTS_Engine_set_alpha(&m_engine, f);

   /* volume */
   f = m_styleWeights[index + m_numModels * 3 + 3];
   if(f > FLITEHTSENGINE_MAXVOLUME)
      HTS_Engine_set_volume(&m_engine, FLITEHTSENGINE_MAXVOLUME);
   else if(f < FLITEHTSENGINE_MINVOLUME)
      HTS_Engine_set_volume(&m_engine, FLITEHTSENGINE_MINVOLUME);
   else
      HTS_Engine_set_volume(&m_engine, f);

   return true;
}
