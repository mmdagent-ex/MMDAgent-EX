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

/* definitions */

#define OPENJTALK_MINLF0VAL log(20.0)

#define OPENJTALK_AUDIOBUFFSIZE 3200

#define OPENJTALK_MAXSPEED    10.0
#define OPENJTALK_MINSPEED    0.1
#define OPENJTALK_MAXHALFTONE 24.0
#define OPENJTALK_MINHALFTONE -24.0
#define OPENJTALK_MAXALPHA    1.0
#define OPENJTALK_MINALPHA    0.0
#define OPENJTALK_MAXVOLUME   10.0
#define OPENJTALK_MINVOLUME   0.0

/* Open_JTalk: Japanese TTS system */
class Open_JTalk
{
private:

   MMDAgent *m_mmdagent;
   int m_id;
   Mecab m_mecab;       /* text analyzer */
   NJD m_njd;           /* container for Naist Japanese Dictionary */
   JPCommon m_jpcommon; /* dictionary-independent container */
   HTS_Engine m_engine; /* speech synthesizer */

   int m_numModels;        /* number of models */
   double *m_styleWeights; /* weights of speaking styles */
   int m_numStyles;        /* number of speaking styles */

   /* initialize: initialize system */
   void initialize();

   /* clear: free system */
   void clear();

public:

   /* Open_JTalk: constructor */
   Open_JTalk();

   /* ~Open_JTalk: destructor */
   ~Open_JTalk();

   /* load: load dictionary and models */
   bool load(MMDAgent *mmdagent, int id, const char *dicDir, char **modelFiles, int numModels, double *styleWeights, int numStyles);

   /* prepare: text analysis, decision of state durations, and parameter generation */
   void prepare(const char *str);

   /* getPhonemeSequence: get phoneme sequence */
   void getPhonemeSequence(char *str, int strlen);

   /* synthesis: speech synthesis */
   void synthesis();

   /* stop: stop speech synthesis */
   void stop();

   /* setStyle: set style interpolation weight */
   bool setStyle(int val);
};
