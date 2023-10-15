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

#define JULIUSLOGGER_SIZE                      4.0f
#define JULIUSLOGGER_ADINMAXVOLUMEUPDATEFRAME  4.0
#define JULIUSLOGGER_ADINUNDERFLOWTHRES        50
#define JULIUSLOGGER_ADINOVERFLOWTHRES         32000
#define JULIUSLOGGER_HASRESULTFRAME            6.0
#define JULIUSLOGGER_TRIGGERFACETRANSFRAME     3.0
#define JULIUSLOGGER_MAXVOLUMEKEYNAME          "Julius_MaxVol"
#define JULIUSLOGGER_LOGTABLE_STEP_SHIFT       4
#define JULIUSLOGGER_VOLUMEFACENAME            "volume"
#define JULIUSLOGGER_TRIGGERFACENAME           "trigger"
#define JULIUSLOGGER_EVENTOVERFLOW             "RECOG_EVENT_OVERFLOW"
#define JULIUSLOGGER_CCDURATIONFRAME           120.0f
#define JULIUSLOGGER_CCUSER                    0
#define JULIUSLOGGER_CCSYSTEM                  1
#define JULIUSLOGGER_CCFONTSCALE               3.0f

/* Julius_Logger: display debug information with OpenGL */
class Julius_Logger
{
private :

   MMDAgent *m_mmdagent;
   int m_id;

   bool m_active;                       /* draw the log only when true */
   float m_size;                        /* size */

   double m_adInFrameStep;      /* current ellapsed frame for input period */
   int m_currentMaxAdIn;        /* current input maximum value (0-32767) */
   bool m_adinUpdated;          /* TRUE when m_currentMaxAdIn was updated after the last update() call */
   float m_levelMin;            /* minimum level in dB (= JULIUSLOGGER_ADINUNDERFLOWTHRES)*/
   float m_levelMax;            /* maximum level in dB (= JULIUSLOGGER_ADINOVERFLOWTHRES) */
   float m_maxAdIn;             /* normalized input level at last input period (0.0-1.0) */
   float m_lastMaxAdin;         /* normalized input level at previous input period for smoothing (0.0-1.0) */
   float m_currentSize;         /* current input indicator value, interpolated by m_maxAdIn and m_lastMaxAdin (0.0-1.0) */
   float m_width;               /* screen width */
   float m_height;              /* screen height */

   int m_levelThres;            /* audio trigger level threshold, obtained from Julius */
   float m_levelSize;           /* current level indicator value (0.0-1.0) */

   bool m_recognizing;          /* true when recognition is running */
   bool m_hasResult;            /* true when this input will have some valid result (not a non-speech input) */
   double m_recognizingFrame;   /* frame counter for rolling */
   double m_recogTransFrame;    /* rest frame for recognition switch transition */

   bool m_hasCaption;                 /* true when any caption is to be displayed */
   double m_ccFrame[2];               /* frame counter for captions */
   float m_ccPos[2][2];               /* position of captions */
   FTGLTextDrawElements m_ccElem[2];  /* work area for text drawing for captions */
   GLfloat m_ccVertices[2][12];       /* vertices for caption drawing */

   float *m_logtable;             /* log table for fast dB calculation */

   GLfloat m_triggerLevelVers[12];
   GLfloat m_circleVers[96];

   /* initialize: initialize data */
   void initialize();

   /* clear: free data */
   void clear();

   /* mklogtable: make log table*/
   void mklogtable();

   /* logval: return log value */
   float logval(int v);

   /* updateVers: update vertices */
   void updateVers();

public :

   /* Julius_Logger: constructor */
   Julius_Logger();

   /* ~Julius_Logger: destructor  */
   ~Julius_Logger();

   /* setup: setup for logging */
   void setup(MMDAgent *mmdagent, int id, Recog *recog);

   /* checkRecogProgress: check recognition progress */
   void checkRecogProgress(Recog *recog, bool flag);

   /* setRecognitionFlag: mark recognition start and end */
   void setRecognitionFlag(bool flag);

   /* updateMaxVol: update maximum volume */
   void updateMaxVol(SP16 *buf, int len);

   /* updateLevelThres: update level threshold */
   void updateLevelThres(int thres);

   /* setActiveFlag: set active flag */
   void setActiveFlag(bool flag);

   /* getActiveFlag: get active flag */
   bool getActiveFlag();

   /* setSize: set size */
   void setSize(float size);

   /* setCaption: set caption */
   void setCaption(const char *s, int id, float x, float y);

   /* setCaptionDuration: set caption duration */
   void setCaptionDuration(int id, double frame);

   /* update: update log view per step */
   void update(double frame);

   /* render: render log view */
   void render2D(float width, float height);
};
